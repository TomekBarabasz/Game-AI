from minmax_player import Node
from itertools import count

class MinMaxAlphaBetaGamePlayer_2P:
	def __init__(self, name, maxDepth, tracer, evalFunc):
		self.name = name
		self.calcnum = 0
		self.tracer = tracer
		self.maxDepth = maxDepth
		self.moveNbr = 1
		self.evalFcn = evalFunc
	def start(self,numPlayers):
		pass
	@staticmethod
	def zeroSumValue(pi, utility):
		return (utility[pi]-utility[1-pi]) #*0.7071067

	def calcMove(self, rules):	
		self.rules = rules
		Moves = self.rules.getLegalMoves(rules.state, self.name)
		if len(Moves)==1: return Moves[0]
		self.tracer.restart(self.name)		
		self.tracer.trace('---start calc move #{0}---'.format(self.moveNbr))
		self.nodesVisited = 0
		mv,_ = self.calcMoveRec(rules.state, self.name, [1], -101, 101)
		self.tracer.trace('visited {0} nodes'.format(self.nodesVisited))
		self.tracer.close()
		self.moveNbr+=1
		assert(mv is not None)
		return mv
	
	#alpha - best value for maximizing player, i.e. == self.name
	#beta  - best value for minimizing player, i.e. != self.name
	def calcMoveRec(self, state, pi, stateName, alpha, beta):
		self.nodesVisited += 1
		if self.rules.isTerminal(state):
			value = self.rules.multiscore(state)
			zsv = MinMaxAlphaBetaGamePlayer_2P.zeroSumValue(self.name,value)
			self.tracer.trace('terminal node {0} d{1} score {2} zero-sum value {3}'.format(stateName,len(stateName),value,zsv))
			self.tracer.traceNode( Node(name=stateName, bestValue='{0}={1}'.format(value,zsv), state=state) )
			return None,zsv
		if len(stateName) >= self.maxDepth:
			value = self.evalFcn(state)
			zsv = MinMaxAlphaBetaGamePlayer_2P.zeroSumValue(self.name,value)
			self.tracer.trace('max depth node {0} d{1} eval {2} zero-sum value {3}'.format(stateName, len(stateName), value,zsv))
			self.tracer.traceNode( Node(name=stateName, bestValue='{0}={1}'.format(value,zsv), zsv=zsv, state=state) )
			return None,zsv
		Moves = self.rules.getLegalMoves(state, pi)
		bestV = -101 if pi==self.name else 101
		bestM = None
		self.tracer.trace('new {0} node {1} d{2} {3} moves'.format('MAX' if pi==self.name else 'MIN', stateName, len(stateName), len(Moves)))
		self.tracer.trace('stack={0}\np0 hand={1}\np1 hand={2}'.format(state.stack, state.hand[0], state.hand[1]))
		self.tracer.trace('alpha={0} beta={1}'.format(alpha,beta))
		for mv,i in zip(Moves,count(1)):
			ns = self.rules.getNextState(state,pi,mv)
			nname = stateName + [i]
			self.tracer.traceEdge(stateName, nname, mv)
			_, v = self.calcMoveRec(ns, self.rules.nextPlayer(pi), nname, alpha, beta)
			if pi == self.name:
				#maximizer
				if v > bestV:
					bestV = v
					bestM = mv
					self.tracer.trace('node {0} update best {1}'.format(stateName, v))
				if bestV >= beta:
					self.tracer.trace('beta pruning node {0} {1}>={2}'.format(stateName, bestV, beta))
					break
				#alpha = max(alpha,bestV)
				if bestV > alpha:
					alpha = bestV
					self.tracer.trace('update alpha : node {0} alpha {1}'.format(stateName, alpha))
			else:
				#minimizer					
				bestV = min(bestV,v)
				if bestV <= alpha:
					self.tracer.trace('alpha pruning node {0} {1}<={2}'.format(stateName, bestV, alpha))
					break
				#beta = min(beta, bestV)
				if bestV < beta:
					beta = bestV
					self.tracer.trace('update beta : node {0} beta {1}'.format(stateName, beta))
		self.tracer.traceNode( Node(name=stateName, bestValue=bestV, bestMove=bestM, state = state, alpha=alpha, beta=beta, numMoves=len(Moves)))
		return bestM, bestV
