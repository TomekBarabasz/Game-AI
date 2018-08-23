class GameRulez:
	def start(self, numPlayers): pass
	def submitMove(self, player, move):pass
	def nextState(self):pass
	def score(self, player):return None
	@staticmethod
	def multiscore(state, player):return None
	def getLegalMoves(self, state, player):return None
	def getNextState(self, state, moves):return None
