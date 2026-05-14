import torch
import engine
from machine_learning_model import TetrisModel, normalize_states, FEATURES

def load_weights(path = 'best_model.pth'):
    checkpoint = torch.load(path)
    model = TetrisModel()
    model.load_state_dict(checkpoint['model'])
    model.eval()
    print(f"Weights loaded from {path}")
    return model

def run(model, env):
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
    print(f"Lines: {lines} | Moves: {moves}")
    return lines, moves

if __name__ == '__main__':
    model = load_weights('best_model.pth')
    env = engine.Engine()

    allLines = []
    allMoves = []
    num_runs = 50

    for i in range(num_runs):
        print(f"Run {i + 1}", end = ' | ')
        lines, moves = run(model, env)
        allLines.append(lines)
        allMoves.append(moves)

    meanLines = sum(allLines) / num_runs
    meanMoves = sum(allMoves) / num_runs

    varLines = sum((x - meanLines) ** 2 for x in allLines) / num_runs
    varMoves = sum((x - meanMoves) ** 2 for x in allMoves) / num_runs

    stdLines = varLines ** 0.5
    stdMoves = varMoves ** 0.5

    cvLines = stdLines / meanLines
    cvMoves = stdMoves / meanMoves

    print(f"\nAverage lines over {num_runs} runs: {meanLines:.1f}")
    print(f"Average moves over {num_runs} runs: {meanMoves:.1f}")
    print(f"Lines cv: {cvLines:.1f}")
    print(f"Moves cv: {cvMoves:.1f}")
