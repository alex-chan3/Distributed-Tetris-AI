import torch
import torch.nn as nn
import torch.optim as optim
import random
import numpy as np
from collections import deque
import socket
import io
import struct
import threading

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
EVAL_SERVER_PORT = 9000 # ensure this matches with eval_server.py or else it will not work

def load_server_address(path = 'server.txt'):
    with open(path, 'r') as f:
        for line in f:
            line = line.strip()
            if line and not line.startswith('#'):
                return line
    raise ValueError(f"No valid address in {path}")

# this collects evaluation results without blocking this training program
# each result is (avg_lines_cleared, iteration)
_eval_result_queue = []
_eval_result_lock = threading.Lock()

def _eval_worker(payload, iteration):
    """
    This runs in the background and sends it to the evaluation server and waits for the results,
    without blocking training allowing for training to continue. Results are stored in the queue
    """
    host = load_server_address()
    print(f"Evaluation Thread started, sending weights to {host}:{EVAL_SERVER_PORT} (iteration {iteration})")
    try:
        with socket.create_connection((host, EVAL_SERVER_PORT), timeout = 1800) as sock: # timeout is 30 minutes as evaluating takes a long time
            sock.sendall(struct.pack('>I', len(payload)))
            sock.sendall(payload)

            # blocking part to wait for results from server (thread only)
            result = _recv_exactly(sock, 8)
            if result is None:
                print(f"Server did not respond (iteration {iteration})")
                return
            avg_lines, _ = struct.unpack('>ff', result)
            avg_lines = float(avg_lines)
            print(f"\n Result received for iteration {iteration}: avg = {avg_lines:.2f}")

            with _eval_result_lock:
                _eval_result_queue.append((avg_lines, iteration))
    except (ConnectionRefusedError, OSError) as e:
        print(f"Connection to server for iteration {iteration} has failed: {e}")

def send_weights_async(num_games = 30, iteration = 0):
    buf = io.BytesIO()
    torch.save({'model': model.state_dict(), 'num_games': num_games}, buf)
    payload = buf.getvalue()

    t = threading.Thread(target=_eval_worker, args = (payload, iteration), daemon = True)
    t.start()

def _recv_exactly(sock, n):
    data = b''
    while len(data) < n:
        chunk = sock.recv(n - len(data))
        if not chunk:
            return None
        data += chunk
    return data

def save_checkpoint(path = 'checkpoint.pth'):
    torch.save({
        'model': model.state_dict(),
        'target_model': target_model.state_dict(),
        'optimizer': optimizer.state_dict(),
        'epsilon': EPSILON
    }, path)
    print(f"Model saved to {path}")

def load_weights_only(path = 'checkpoint.pth'):
    checkpoint = torch.load(path)
    model.load_state_dict(checkpoint['model'])
    target_model.load_state_dict(checkpoint['target_model'])
    print(f"Model loaded from {path}")

def load_full_checkpoint(path = 'checkpoint.pth'):
    global EPSILON
    checkpoint = torch.load(path)
    model.load_state_dict(checkpoint['model'])
    target_model.load_state_dict(checkpoint['target_model'])
    optimizer.load_state_dict(checkpoint['optimizer'])
    EPSILON = checkpoint['epsilon']
    print(f"Loaded everything in {path}")

def normalize_states(states):
    states = states[:, :23]
    states = states.clone()
    # from the engine: [lines, col0, ..., col9 heights, col0,..., col9 holes, smoothness, max_height]
    states[:, 0] /= 4.0
    states[:, 1:11] /= 20.0
    states[:, 11:21] /= 20.0
    states[:, 21] /= 100.0
    states[:, 22] /= 20.0
    return states

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

def selectMove(states, epsilon):
    if random.random() < epsilon:
        return random.randrange(len(states))
    with torch.no_grad():
        q_vals = model(states).squeeze(-1)
    return torch.argmax(q_vals).item()    

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
            next_q[i] = torch.max(q_vals)
    
    targets = rewards + GAMMA * next_q * (1 - dones)

    loss = nn.MSELoss()(preds, targets)
    optimizer.zero_grad()
    loss.backward()
    optimizer.step()

def train(env, iterations):
    global EPSILON
    max_avg_lines = 0

    for it in range(iterations):
        with _eval_result_lock:
            while _eval_result_queue:
                avg_lines, from_iter = _eval_result_queue.pop(0)
                if avg_lines > max_avg_lines:
                    max_avg_lines = avg_lines
                    save_checkpoint('best_model.pth')
                    print(f"New best model saved (avg = {avg_lines:.2f} from iteration {from_iter})")
        
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
            
            replay_buffer.append((chosen_state, reward, next_states, float(done)))

            trainStep()

            if not done:
                states = next_states
            
            moves += 1
        EPSILON = max (EPSILON_MIN, EPSILON * EPSILON_DECAY)

        if it % TARGET_UPDATE == 0:
            target_model.load_state_dict(model.state_dict())
            print("---- target model updated ----")
        
        lines = env.getLinesCleared()
        print(
            f"Iteration {it:4d} | "
            f"Score {total_score:8.2f} | "
            f"Lines {lines:4d} | "
            f"Moves {moves:4d} | "
            f"Eps {EPSILON:.3f} | "
            f"Max Avg Lines: {max_avg_lines:8.1f}"
        )

        if it % 100 == 0 and it != 0:
            print("Sending model to evaluator server")
            save_checkpoint('model_to_evaluate.pth')
            send_weights_async(num_games = 30, iteration = it)
        
        if it % 50 == 0:
            save_checkpoint('checkpoint.pth')

model = TetrisModel()
target_model = TetrisModel()
target_model.load_state_dict(model.state_dict())
target_model.eval()

optimizer = optim.Adam(model.parameters(), lr = LR)
replay_buffer = deque(maxlen = BUFFER_SIZE)

if __name__ == '__main__':
    env = engine.Engine()
    train(env, ITERATIONS)