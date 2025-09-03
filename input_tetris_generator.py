# 2025-04-29  input_tetris_generator.py

import random
import sys

if __name__ == '__main__':
    script, seed, length = sys.argv
    seed = int(seed)
    length = int(length)

    random.seed(seed)

    TETRIS = "IOLJZST"

    if length <= 0:
        exit(0)

    print(random.choice(TETRIS), end='')

    for _ in range(length - 1):
        print(random.choice(TETRIS))

    print('X')

