# Ultimate Tic Tac Toe

Computer players for Ultimate Tic Tac Toe from riddles.io, see [playground](playground.riddles.io/competitions/ultimate-tic-tac-toe)

## Minmax exploration algorithm

### Features

+ [Minmax algorithm](https://www.chessprogramming.org/Minimax)
+ [Alpha-Beta pruning](https://www.chessprogramming.org/Alpha-Beta)
+ [Iterative deepening](https://www.chessprogramming.org/Iterative_Deepening)
+ (with [PV-move explored first](https://www.chessprogramming.org/PV-Move))
+ [Transposition table](https://www.chessprogramming.org/Transposition_Table)
+ Double hasing in Transposition Table to reduce collisions.
+ [Zobrist Hasing](https://www.chessprogramming.org/Zobrist_Hashing)
+ Normalization of equivalent boards before access to Transposition Table
+ [Backtracking](https://www.chessprogramming.org/Backtracking)
+ [Bitboards](https://www.chessprogramming.org/Bitboards)
+ [Bit-twiddling computations](https://www.chessprogramming.org/Bit-Twiddling)
+ Precomputations
+ Time budget management
+ Score computation
+ Farthest defeat / closest win chosen

#### Score computation

The score takes into account how likely is the victory in a small tic tac toe
(it properly detects when a sub-board can't be won)
and uses this to compute the score of the complete board.

### Performance

The minmax algorithm can evaluate **13M positions/s** on a single CPU core.
