"""
this is run on all machines you want to evaluate the ai

it connects with eval_server to receive the model weights, runs the AI with epsilon = 0 (fully greedy and what it believes is optimal) for the requested number of games,
and returns individual game socres to the server

formate of server.txt:
    192.168.1.1                        <- (ip of individual machine(s))
    192.168.1.2

    # anything beginning with '#' is ignored

Port:
    9001: used when communicated with eval_server.py
"""

import socket
import struct
import io
import torch
import torch.nn as nn

try:
    import engine
except ImportError:
    raise ImportError("Engine module should be on each eval machine")

FEATURES = 23
LISTEN_PORT = 9001

# this must match from machine_learning_model.py exactly
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

# must match exactly what is in machine_learning_model.py
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

def recv_exactly(sock, n):
    data = b''
    while len(data) < n:
        chunk = sock.recv(n - len(data))
        if not chunk:
            return None
        data += chunk
    return data

def load_model_from_bytes(payload_bytes):
    buf = io.BytesIO(payload_bytes)
    checkpoint = torch.load(buf, map_location = 'cpu')
    model = TetrisModel()
    model.load_state_dict(checkpoint['model'])
    model.eval()
    num_games = checkpoint.get('num_games', 10)
    return model, num_games

def run_game(model, env):
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
    return lines, moves

def evaluate(model, env, num_games):
    scores = []
    for i in range(num_games):
        lines, moves = run_game(model, env)
        print(f"Game {i + 1:3d} | Lines: {lines:5d} | Moves: {moves:5d}")
        scores.append(float(lines))
    
    avg = sum(scores) / len(scores)
    std = (sum((s - avg) ** 2 for s in scores) / len(scores)) ** 0.5
    cv = (std / avg * 100) if avg > 0 else 0.0
    print(f" Local avg: {avg:.2f} | CV: {cv:.2f}%")

    return scores

def handle_connection(conn, addr, env):
    print(f"\nConnection from {addr}")

    raw_len = recv_exactly(conn, 4)
    if raw_len is None:
        print("Server disconnected before sending data.")
        conn.close()
        return
    payload_len = struct.unpack('>I', raw_len)[0]
    print(f"Expecting {payload_len} bytes of weights")

    payload = recv_exactly(conn, payload_len)
    if payload is None or len(payload) != payload_len:
        print("Incomplete payload received")
        conn.close()
        return
    
    print(f"Weights received. Loading model")
    model, num_games = load_model_from_bytes(payload)
    print(f"Running {num_games} games")

    scores = evaluate(model, env, num_games)

    count = len(scores)
    conn.sendall(struct.pack('>I', count))
    conn.sendall(struct.pack(f'>{count}f', *scores))
    print(f"Sent {count} scores back to server.")
    conn.close()

def main():
    env = engine.Engine()
    print(f"Listening for eval server on port {LISTEN_PORT}")

    server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_sock.bind(('', LISTEN_PORT))
    server_sock.listen(5)

    while True:
        conn, addr = server_sock.accept()
        handle_connection(conn, addr, env)

if __name__ == '__main__':
    main()