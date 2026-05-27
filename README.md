# Distributed Tetris AI

A custom C++ engine to allow for the development and training of a pytorch model with pybind11

---
## How it works

machine_learning_model.py:
- Trains the model using a DQN algorithm
- Every 100 training iterations, it sends the weights at the time to eval_server.py using a background thread to not stop training
- When receiving results from eval_server.py, it determines which is the best model and saves it
- uses Port 9000 when communicating with eval_server.py

eval_server.py:
- Receives the model weights and distributes them out to all evaluating machines (running eval_client.py)
- Receives the results from the machines and scores the performance to determine the best model
- Sends results to machine_learning_model.py
- uses Port 9000 when communicating with machine_learning_model.py
- uses Port 9001 when communicating with eval_client.py

eval_client.py:
- Receives the model weights and simulates games based on what the model thinks is optimal
- Sends the results back to eval_server.py
- Uses Port 9001 when communicating with eval_server.py
---

## Config files

**`server.txt`** - located on the training machine, this tells the trainer where `eval_server.py` will be run:
```
192.168.1.1 # the machine's ip
# all lines beginning with '#' are comments and ignored
```

**`machines.txt`** - located on the machine that runs `eval_sever.py`, it has all addresses of all evaluating machines:
```
192.168.1.2
192.168.1.3
# all lines beginning with '#' are comments and ignored
```
---

## Running it
**1. On each eval machine:**
- Ensure engine is built by using "python setup.py build_ext --inplace"
```bash
python eval_client.py
```

**2. On the distributing machine:**
```bash
python eval_server.py
```

**3. On the training machine:**
- Ensure engine is built by using "python setup.py build_ext --inplace"
```bash
python machine_learning_model.py
```

> **Note:** Order matters, you must start eval_server.py before running machine_learning_model, and eval_client.py before eval_server.py sends the first weights out.
---

## Building the engine
```bash
pip install torch pybind11 setuptools
python setup.py build_ext --inplace
```

This creates 2 .pyd files, `engine.pyd` which is what we want, and another file with a very long name. This needs to be in the project root and built on all machines (it's a compiled C++ extension)

## Checkpoints

- `checkpoint.pth`: saved every 50 iterations
- `model_to_evaluate.pth`: saved every 100 iterations, it's the snapshot that will be evaluated
- `best_model.pth`: when a new highest avg lines when evaluated

If you stopped training, using `load_full_checkpoint('checkpoint.pth')` will load all context when it was last saved (last 50 iterations)

## Timeouts
all client timeouts have been setto 1800 seconds (30 min). When an eval machine stops responding the server will just skip it and continue evaluating on the rest of the machines.

## Prerequisites
- pytorch
- pybind11
- setuptools

just run:

```bash
pip install torch pybind11 setuptools
```