import sys
from subprocess import Popen, PIPE, STDOUT, DEVNULL
import datetime
import numpy
import re
import time
import argparse
from scipy import stats
import re

parser = argparse.ArgumentParser()
parser.add_argument('--reverse', action='store_true')
parser.add_argument('--verbose', action='store_true')
parser.add_argument('--bench', action='store_true')
args, _ = parser.parse_known_args()

VERBOSE = True if args.verbose else False

INITIAL_ROUND = 1
STARTING_TIMEBANK = 20000
TIME_PER_MOVE = 100
CONSOLE_ENCODING = 'ascii'

class GameProgram:
		def __init__(self, process):
				self.process = process

		def writeLines(self, lines):
				lines_str = '\n'.join(lines) + '\n'
				if VERBOSE:
						print(lines_str, flush=True)
				self.process.stdin.write(bytes(lines_str, CONSOLE_ENCODING))
				self.process.stdin.flush()

		def readLine(self):
				line_str = self.process.stdout.readline().decode(CONSOLE_ENCODING)
				if VERBOSE:
						print(line_str, flush=True)
				return line_str

NONE = '.'
PLAYER0 = '0'
PLAYER1 = '1'
DRAW = '#' # board is full, but no winner

def other(player):
		if player == NONE:
				return NONE
		if player == PLAYER0:
				return PLAYER1
		if player == PLAYER1:
				return PLAYER0
		if player == DRAW:
				return DRAW

class TicTacToe:
		def __init__(self, matrix):
				self.matrix = matrix

		def winner(self):
				for a, b, c in [
						[(0,0), (0,1), (0,2)],
						[(1,0), (1,1), (1,2)],
						[(2,0), (2,1), (2,2)],
						[(0,0), (1,0), (2,0)],
						[(0,1), (1,1), (2,1)],
						[(0,2), (1,2), (2,2)],
						[(0,0), (1,1), (2,2)],
						[(2,0), (1,1), (0,2)],
				]:
						for player in [PLAYER0, PLAYER1]:
								if self.matrix[a] == player and self.matrix[b] == player and self.matrix[c] == player:
										return player
				
				# no winner and no empty slots so it is a draw
				if numpy.all(self.matrix != '.'):
					return DRAW

				return NONE

		def is_full_or_won(self):
				return numpy.all(self.matrix != '.') or self.winner() != NONE

class Board:
		def __init__(self):
				self.matrix = numpy.full((9,9), NONE, dtype=str)

		def __str__(self):
				return re.sub("[\[\]'\n ]", '', numpy.array2string(self.matrix, separator=',', prefix=''))

		def get_matrix(self):
				return self.matrix

		def at(self, Y, X):
				return TicTacToe(self.matrix[0+3*Y:0+3*(Y+1), 0+3*X:0+3*(X+1)])

		def iterate(self):
				for Y in range(3):
						for X in range(3):
								yield self.at(Y, X)

		def winner(self):
				winner = TicTacToe(numpy.array([ttt.winner() for ttt in self.iterate()]).reshape(3,3)).winner()
				return winner

POSSIBLE_MOVE = '-1'

class GameOver(Exception):
		def __init__(self, winner, reason):
				self.winner = winner
				self.reason = reason
		def get_winner(self):
				return self.winner

def turn(players, player, board, possible_moves, round_number):
		lines = []

		if round_number == INITIAL_ROUND:
				lines.append(f"settings player_names {players[0]['name']},{players[1]['name']}")
				lines.append(f"settings your_bot {player['name']}")
				lines.append(f"settings timebank 10000")
				lines.append(f"settings time_per_move 100")
				lines.append('settings your_botid 0')

		# start of round for this player
		lines.append(f'update game round {round_number}')
		lines.append(f"update game field {str(board)}")
		lines.append(f"update game macroboard {','.join(possible_moves)}")
		lines.append(f"action move {player['timebank']}")

		time_before = datetime.datetime.now()
		player['program'].writeLines(lines)
		choice = player['program'].readLine()
		time_after = datetime.datetime.now()

		player['timebank'] -= (time_after - time_before).microseconds / 1000.

		if player['timebank'] < 0:
				raise GameOver(winner=other(player['id']), reason='Exceeded time limit !')

		if choice.startswith('place_move'):
				_, i, j = choice.split(' ')
				i, j = int(i), int(j)
				X = i//3
				x = i%3
				Y = j//3
				y = j%3

				# do the move if possible
				if board.get_matrix()[j,i] != NONE:
						raise GameOver(winner=other(player['id']), reason='Invalid move !')
				board.get_matrix()[j,i] = player['id']

				# compute the possible moves for the next player
				if board.at(y, x).winner() == NONE:
					possible_moves = [ttt.winner() for ttt in board.iterate()]
					possible_moves[3*y + x] = '-1'
				else:
					possible_moves = ['-1' if ttt.winner() == NONE else ttt.winner() for ttt in board.iterate()]

		# player wants to skip turn (only allowed if board is full or won)
		elif choice.startswith('no_moves'):
				if board.winner() == NONE:
						raise GameOver(winner=other(player['id']), reason='Abusive no_moves !')

		# not a valid move neither a 'no_moves'
		else:
				raise GameOver(winner=other(player['id']), reason=f'Invalid output from bot : "{choice}"')

		if VERBOSE:
				print(re.sub("[\[\]' ,]", '', numpy.array2string(board.get_matrix(), separator=',', prefix='')))

		# check for winner or draw
		winner = board.winner()
		if winner != NONE:
				raise GameOver(winner=winner, reason='Fair victory')

		return possible_moves

def initialized_player(path, playerName, id, process):
	player = {}
	player['path'] = path
	player['timebank'] = STARTING_TIMEBANK
	player['name'] = playerName
	player['id'] = id
	player['program'] = GameProgram(process)
	return player

def play_until_throw_gameover(players):
	board = Board()
	round_number = INITIAL_ROUND
	possible_moves = 9*[POSSIBLE_MOVE]
	while True:
		possible_moves = turn(players, players[0], board, possible_moves, round_number)
		possible_moves = turn(players, players[1], board, possible_moves, round_number)
		round_number += 1

def one_game(statistics, path0, path1):
		players = []
		with Popen([path0], stdout=PIPE, stdin=PIPE, stderr=None if VERBOSE else DEVNULL) as process0:
				with Popen([path1], stdout=PIPE, stdin=PIPE, stderr=None if VERBOSE else DEVNULL) as process1:
						try:
							players = [
								initialized_player(path0, "player_0", PLAYER0, process0),
								initialized_player(path1, "player_1", PLAYER1, process1)
							]

							play_until_throw_gameover(players)

						except GameOver as game_over:
							winner = game_over.get_winner()
							statistics[winner] += 1
							statistics = f'{statistics[PLAYER0]}, {statistics[PLAYER1]}, draw: {statistics[DRAW]}, '
							if winner == DRAW:
									print(statistics + 'game is draw')
							else:
									winner_name = players[0]['name'] if winner == PLAYER0 else players[1]['name']
									print(statistics + 'winner is : ' + winner_name + ' reason : ' + game_over.reason)
						finally:
								players[0]['program'].writeLines(['exit'])
								players[1]['program'].writeLines(['exit'])

def benchmark(statistics, path0):
		players = []
		with Popen([path0], stdout=PIPE, stdin=PIPE, stderr=PIPE) as process0:
			try:
				players = [
					initialized_player(path0, "player_0", PLAYER0, process0),
					initialized_player(path0, "player_1", PLAYER1, None)
				]
				board = Board()
				round_number = INITIAL_ROUND
				possible_moves = 9*[POSSIBLE_MOVE]
				possible_moves = turn(players, players[0], board, possible_moves, round_number)
			finally:
					positions__per_s = 0.
					while True:
						line_str = process0.stderr.readline().decode(CONSOLE_ENCODING)
						if "positions/s" in line_str:
							positions__per_s = float(re.search("positions/s: ([0-9]+)", line_str).group().split(" ")[1])
							break
					players[0]['program'].writeLines(['exit'])
					return positions__per_s

try:
		statistics = {PLAYER0: 0, PLAYER1: 0, DRAW: 0}
		if not args.bench:
			while True:
					one_game(statistics, sys.argv[1 if not args.reverse else 2], sys.argv[2 if not args.reverse else 1])
		else:
			results = []
			for _ in range(20):
				positions__per_s = benchmark(statistics, sys.argv[1 if not args.reverse else 2])
				print(positions__per_s)
				results.append(positions__per_s)
			print(stats.describe(results))

except KeyboardInterrupt:
		pass
