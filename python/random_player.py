from random import choice

class RandomGamePlayer:
	def __init__(self, name,*a):
		self.name = name
		self.verbose = False
	def start(self,numPlayers):
		pass
	def calcMove(self, rulez):
		Moves = rulez.getLegalMoves(rulez.state, self.name)
		m = choice(Moves)
		if (self.verbose) : print('player',self.name,'selected',m)
		return m

class LowCardGamePlayer:
	def __init__(self, name,*a):
		self.name = name
		self.verbose = False
	def start(self,numPlayers):
		pass
	def calcMove(self, rulez):
		Moves = rulez.getLegalMoves(rulez.state, self.name)
		m = Moves[0]
		if (self.verbose) : print('player',self.name,'selected',m)
		return m
