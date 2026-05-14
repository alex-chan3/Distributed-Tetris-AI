import torch
import torch.nn as nn
import torch.optim as optim
import random
import numpy as np
from collections import deque

import engine

FEATURES = 23
GAMMA = 0.995
EPSILON = 1.0
EPSILON_DECAY = 0.995
EPSILON_MIN = 0.05
LR = 5e-4
ITERATIONS = 5000
BATCH_SIZE = 256
BUFFER_SIZE = 100000
TARGET_UPDATE = 100

def save_checkpoint(path = 'checkpoint.pth'):
    torch.save({
        'model': model.state_dict(),
        'target_model': target_model.state_dict(),
        'optimizer': optimizer.state_dict(),
        'epsilon': EPSILON
    }, path)
    print(f"Model saved at {path}")

def load_weights_only(path = 'checkpoint.pth'):
    checkpoint = torch.load(path)
    model.load_state_dict(checkpoint['model'])
    target_model.load_state_dict(checkpoint['target_model'])
    print(f"Model weights loaded from {path}")

# unlike load_weights_only, this can be used to continue training
def load_full_checkpoint(path = 'checkpoint.pth'):
    global EPSILON
    checkpoint = torch.load(path)
    model.load_state_dict(checkpoint['model'])
    target_model.load_state_dict(checkpoint['target_model'])
    optimizer.load_state_dict(checkpoint['optimizer'])
    EPSILON = checkpoint['epsilon']
    print(f"Loaded everything from {path}")

# use to make features between 0 and 1
def normalize_states(states):
    states = states[:, :23]

    states = states.clone()

    # from the engine, state is: [lines, col0 height, ..., col9 height, col0 holes, ..., col9 holes, total smoothness, max height]

    states[:, 0] /= 4.0        # lines cleared: 0-4
    states[:, 1:11] /= 20.0    # column heights: 0-20
    states[:, 11:21] /= 20.0   # column holes: 0-20
    states[:, 21] /= 100.0     # smoothness: 0-100
    states[:, 22] /= 20.0      # max height: 0-20
    return states

# ml network definition
class TetrisModel(nn.Module):
    def __init__(self):
        super().__init__()
        self.net = nn.Sequential(
            nn.Linear(FEATURES, 64),
            nn.ReLU(),
            nn.Linear(64, 32),
            nn.ReLU(),
            nn.Linear(32, 1)
        )

    def forward(self, x):
        return self.net(x)

# allows for epsilon-greedy exploration
def selectMove(states, epsilon):
    if random.random() < epsilon:
        return random.randrange(len(states))
    
    with torch.no_grad():
        q_vals = model(states).squeeze(-1)
    return torch.argmax(q_vals).item()

# update weights/teaching model
def trainStep():
    if len(replay_buffer) < BATCH_SIZE:
        return
    
    batch = random.sample(replay_buffer, BATCH_SIZE)

    states = torch.stack([b[0] for b in batch])
    rewards = torch.tensor([b[1] for b in batch], dtype = torch.float32)
    dones = torch.tensor([b[3] for b in batch], dtype = torch.float32)

    preds = model(states).squeeze(-1)

    next_states = [b[2] for b in batch]
    next_q = torch.zeros(BATCH_SIZE)

    with torch.no_grad():
        for i, ns in enumerate(next_states):
            if ns is None:
                continue
            q_vals = target_model(ns).squeeze(-1)
            next_q[i] =torch.max(q_vals)
    
    targets = rewards + GAMMA * next_q * (1 - dones)

    loss = nn.MSELoss()(preds, targets)

    optimizer.zero_grad()
    loss.backward()
    optimizer.step()

# used to evaluate models to find/compare them
def evaluate(model, env, num_games=10):
    global EPSILON
    saved_epsilon = EPSILON
    EPSILON = 0.0

    results = []

    for i in range(num_games):
        env.initialize(1)
        moves = 0

        raw = env.getNextStep()
        states = normalize_states(torch.tensor(raw, dtype = torch.float32))

        while not env.gameLost():
            with torch.no_grad():
                q_vals = model(states).squeeze(-1)
            move = torch.argmax(q_vals).item()

            env.applyMove(move)

            if not env.gameLost():
                raw = env.getNextStep()
                states = normalize_states(torch.tensor(raw, dtype = torch.float32))
            
            moves += 1
        
        lines = env.getLinesCleared()
        results.append(lines)
        print(f"  Eval Game {i + 1:2d} | Lines Cleared: {lines:5d} | Pieces Dropped: {moves:5d}")
    
    EPSILON = saved_epsilon

    avg = np.mean(results)
    cv = (np.std(results) / avg * 100) if avg > 0 else 0.0

    print(f"  Average: {avg:.4f} | CV: {cv:.4f}%")
    return avg, cv

# main training method
def train(env, iterations):
    global EPSILON
    curr_avg_lines = 0
    max_avg_lines = 0
    curr_cv = 0
    max_cv = 0

    for it in range(iterations):
        env.initialize(1)

        total_score = 0
        moves = 0

        raw = env.getNextStep()
        states = normalize_states(torch.tensor(raw, dtype = torch.float32))

        while not env.gameLost():
            move = selectMove(states, EPSILON)
            chosen_state = states[move]

            reward = env.applyMove(move)
            total_score += reward

            done = env.gameLost()

            if done:
                next_states = None
            else:
                raw_next = env.getNextStep()
                next_states = normalize_states(torch.tensor(raw_next, dtype = torch.float32))

            replay_buffer.append((
                chosen_state,
                reward,
                next_states,
                float(done)
            ))

            trainStep()

            if not done:
                states = next_states
            
            moves += 1
        
        EPSILON = max(EPSILON_MIN, EPSILON * EPSILON_DECAY)

        if it % TARGET_UPDATE == 0:
            target_model.load_state_dict(model.state_dict())
            print("--- target model updated ---")

        lines = env.getLinesCleared()

        print(
            f"Iteration {it:4d} | "
            f"Score {total_score:8.2f} | "
            f"Lines {lines:4d} | "
            f"Moves {moves:4d} | "
            f"Eps {EPSILON:.3f} | "
            f"Max Average Lines: {max_avg_lines:8.1f} | "
            f"Max model's cv: {max_cv:6.1f}"
        )

        if it % 100 == 0 and it != 0:
            print("Evaluating now")
            save_checkpoint('model_to_evaluate.pth')
            curr_avg_lines, curr_cv = evaluate(model, env, 30)

            if curr_avg_lines > max_avg_lines:
                save_checkpoint('best_model.pth')
                max_avg_lines = curr_avg_lines
                max_cv = curr_cv
        
        if it % 50 == 0:
            save_checkpoint('checkpoint.pth')

model = TetrisModel()
target_model = TetrisModel()
target_model.load_state_dict(model.state_dict())
target_model.eval()

optimizer = optim.Adam(model.parameters(), lr = LR)
replay_buffer = deque(maxlen=BUFFER_SIZE)

if __name__ == '__main__':
    env = engine.Engine()
    train(env, ITERATIONS)