"""
this runs on whatever communicates with the distributed system, it can be a load balancer or the training machine

it will listen for evaluation requests from machine_learning_model.py,
send them to all machines listed in machines.txt,
collects the results, combines them and returns the results

machines.txt format:
    192.168.1.10
    192.168.1.11
    # lines beginning with '#' are ignored

Ports:
    9000: from the trainer, listens and receives models to evaluate
    9001: what the distributed machines listen to
"""

import socket
import struct
import io
import threading
import time

LISTEN_PORT = 9000
CLIENT_PORT = 9001
MACHINES_FILE = 'machines.txt'
CLIENT_TIMEOUT = 1800 

def load_machines(path = MACHINES_FILE):
    machines = []
    with open(path, 'r') as f:
        for line in f:
            line = line.strip()
            if line and not line.startswith('#'):
                machines.append(line)
    
    if not machines:
        raise ValueError(f"No machines were given in {path}")
    return machines

def recv_exactly(sock, n):
    data = b''
    while len(data) < n:
        chunk = sock.recv(n - len(data))
        if not chunk:
            return None
        data += chunk
    return data

def send_to_client(host, payload, results, idx):
    # connects to one eval computer, sends weights, waits for response, stores it into results[idx]
    print(f"Sending weights to client {host}:{CLIENT_PORT} ...")
    try:
        with socket.create_connection((host, CLIENT_PORT), timeout = CLIENT_TIMEOUT) as sock:
            sock.sendall(struct.pack('>I', len(payload)))
            sock.sendall(payload)

            raw_count = recv_exactly(sock, 4)
            if raw_count is None:
                print(f" No response from {host}")
                results[idx] = None
                return
            count = struct.unpack('>I', raw_count)[0]

            raw_scores = recv_exactly(sock, count * 4)
            if raw_scores is None:
                print(f"Incomplete score data from {host}")
                results[idx] = None
                return
            
            scores = list(struct.unpack(f'>{count}f', raw_scores))
            avg = sum(scores) / len(scores)
            std = (sum((s- avg)** 2 for s in scores) / len(scores)) ** 0.5
            cv = (std / avg * 100) if avg > 0 else 0.0
            print(f"{host}: {count} games | avg = {avg:.2f} | cv = {cv:.2f} %")
            results[idx] = scores
    except (ConnectionRefusedError, OSError, socket.timeout) as e:
        print(f"Failed to contact {host}: {e}")
        results[idx] = None

def aggregate(results_per_machine):
    all_scores = []
    for r in results_per_machine:
        if r is not None:
            all_scores.extend(r)
    
    if not all_scores:
        return 0.0, 0.0
    
    avg = sum(all_scores) / len(all_scores)
    std = (sum((s - avg) ** 2 for s in all_scores) / len(all_scores)) ** 0.5
    cv = (std / avg * 100) if avg > 0 else 0.0
    return avg, cv

def handle_trainer(conn, addr):
    print(f"\n Trainer connected from {addr}")

    raw_len = recv_exactly(conn, 4)
    if raw_len is None:
        print("Trainer disconnected before sending data.")
        conn.close()
        return
    payload_len = struct.unpack('>I', raw_len)[0]
    print(f"Expecting {payload_len} bytes of weights")

    payload = recv_exactly(conn, payload_len)
    if payload_len is None or len(payload) != payload_len:
        print("Incomplete weights received")
        conn.close()
        return
    print(f"Weights received ({payload_len} bytes)")

    machines = load_machines()
    print(f"Distributed to {len(machines)} machine(s): {machines}")

    results = [None] * len(machines)
    threads = []

    for i, host in enumerate(machines):
        t = threading.Thread(target = send_to_client, args = (host, payload, results, i), daemon = True)
        t.start()
        threads.append(t)
    
    for t in threads:
        t.join(timeout = CLIENT_TIMEOUT + 10)
    
    avg, cv = aggregate(results)
    successful = sum(1 for r in results if r is not None)
    total_games = sum(len(r) for r in results if r is not None)

    print(f"\n Aggregated results from {successful}/{len(machines)} machines")
    print(f"Total Games: {total_games} | Overall avg lines: {avg:.4f} | CV: {cv:.4f}%")

    conn.sendall(struct.pack('>ff', avg, cv))
    conn.close()
    print("Result sent to trainer. Waiting for next request.")

def main():
    machines = load_machines()
    print(f"Loaded {len(machines)} eval machine(s) from {MACHINES_FILE}: {machines}")
    print(f"Listenint for trainer on port {LISTEN_PORT}")

    server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_sock.bind(('', LISTEN_PORT))
    server_sock.listen(1)

    while True:
        conn, addr = server_sock.accept()
        t = threading.Thread(target = handle_trainer, args = (conn, addr), daemon = True)
        t.start()

if __name__ == '__main__':
    main()