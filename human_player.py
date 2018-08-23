import sys
from msvcrt import getch
from itertools import count

def wait():
	print('press any key to continue...')
	c = getch()
	if c == b'\x03':
		raise KeyboardInterrupt

def num2Suites(cards):
	suits = '♥♠♣♦'
	values=['9','10','W','D','K','A']
	return ' '.join([ values[x//10-9]+suits[x%10-1] for x in cards])

def move2str(move):
	if move[0] == 'play':
		return move[0] + ' ' + num2Suites(move[1])
	elif move[0] == 'take':
		return move[0] + ' ' + str(move[1])
	elif move[0] == 'noop':
		return move[0]

class HumanGamePlayer:
	def __init__(self, name, *a):
		self.name = name
		
	def start(self,numPlayers):
		print('you are player',self.name)
	def calcMove(self, rulez):
		Moves = rulez.getLegalMoves(rulez.state, self.name)
		if len(Moves) == 1:
			#print('Auto move selection',Moves[0])
			return Moves[0]
		print('State is:')
		print('Stack',num2Suites(rulez.state.stack))
		for kh,n in zip(rulez.state.known,count(0)):
			if n != self.name:
				print('player',n,'known hand',num2Suites(kh))
		print('your cards',num2Suites(rulez.state.hand[self.name]))
		print ('Your move options:')
		for m,i in zip(Moves, count(1)):
			print (i,':',move2str(m))
		#print(Moves)
		choice = int( sys.stdin.readline() )
		return Moves[choice-1]
