import sys
from msvcrt import getch
from itertools import count
from tracer import num2Suites, move2str

def wait():
	print('press any key to continue...')
	c = getch()
	if c == b'\x03':
		raise KeyboardInterrupt

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
