import sys
sys.path.append('C:\\Users\\atb003\\Documents\\Prolog')

class GameRules:
	def __init__(self, prologFilename):
		from prolog import YAProlog,Term, CompoundTerm
		self.prolog = YAProlog(file=prologFilename)
	def start(self, numPlayers):
		g = CompoundTerm('start_rnd_game',[numPlayers,'State'])
		assert( self.prolog.runGoalOnce(g) )
		self.state = g.State
		self.actions = []
	def submitMove(self, player, move):
		self.actions.append( CompoundTerm('does',[player, move]) )
		print('Player', player, 'submitted',move)
	def nextState(self):
		g = CompoundTerm('get_next_state',[self.state, self.actions, 'NextState'])
		assert( self.prolog.runGoalOnce(g) )
		self.state = g.NextState
		g = CompoundTerm('terminal',[self.state])
		self.actions=[]
		return self.prolog.runGoalOnce(g)
	def score(self, player):
		g = CompoundTerm('goal',[player, 'Score'])
		assert( self.prolog.runGoalOnce(g) )
		return g.Score
	def getLegalMoves(self, state, player):
		g = CompoundTerm('legal',[state, player, 'Moves'])
		assert( self.prolog.runGoalOnce(g) )
		return g.Moves
	def getNextState(self, state, moves):
		actions = [ CompoundTerm('does',[player,move]) for player,move in moves.items() ]
		g = compoundTerm('get_next_state',[state, actions, 'NextState'])
		assert( self.prolog.runGoalOnce(g) )
		return g.NextState
	def test(self):
		g = CompoundTerm('start_rnd_game',[2,'State'])
		assert( self.prolog.runGoalOnce(g) )
		state = g.State
		print('typeof State is',type(state))
		for f in state:
			print('type of fluent is',type(f))
			print(f)
