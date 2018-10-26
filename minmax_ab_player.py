from minmax_player import Node
from itertools import count
#from copy import deepcopy

class MinMaxAlphaBetaTracer:
	def __init__(self, tracer, name):
		self.tracer = tracer
		self.name = name

	def restart(self, name):
		self.name = name
		self.tracer.restart(name)

	def traceTerminalNode(self, stateName, state, value, zsv):
		self.tracer.trace('terminal node {0} d{1} score {2} zero-sum value {3}'.format(stateName,len(stateName),value,zsv))
		self.tracer.traceNode( Node(name=stateName, bestValue='{0}={1}'.format(value,zsv), state=state) )
	
	def traceMaxDepthNode(self, stateName, state, value, zsv):
		self.tracer.trace('max depth node {0} d{1} eval {2} zero-sum value {3}'.format(stateName, len(stateName), value,zsv))
		self.tracer.traceNode( Node(name=stateName, bestValue='{0}={1}'.format(value,zsv), zsv=zsv, state=state) )
	
	def traceNodeAB(self,stateName, state, Moves, alpha, beta, pi):
		self.tracer.trace('new {0} node {1} d{2} {3} moves'.format('MAX' if pi==self.name else 'MIN', stateName, len(stateName), len(Moves)))
		self.tracer.trace('stack={0}\np0 hand={1}\np1 hand={2}'.format(state.stack, state.hand[0], state.hand[1]))
		self.tracer.trace('alpha={0} beta={1}'.format(alpha,beta))
	
	def traceAlphaBeta(self, stateName, bestV, v, alpha, beta, pi):
		if pi == self.name:
			#maximizer
			if v > bestV:
				bestV = v
				self.tracer.trace('node {0} update best {1}'.format(stateName, v))
			if bestV >= beta:
				self.tracer.trace('beta pruning node {0} {1}>={2}'.format(stateName, bestV, beta))
			#alpha = max(alpha,bestV)
			elif bestV > alpha:
				alpha = bestV
				self.tracer.trace('update alpha : node {0} alpha {1}'.format(stateName, alpha))
		else:
			#minimizer					
			bestV = min(bestV,v)
			if bestV <= alpha:
				self.tracer.trace('alpha pruning node {0} {1}<={2}'.format(stateName, bestV, alpha))
			#beta = min(beta, bestV)
			elif bestV < beta:
				beta = bestV
				self.tracer.trace('update beta : node {0} beta {1}'.format(stateName, beta))
	
	def traceEdge(self, stateName, nname, mv):
		self.tracer.traceEdge(stateName, nname, mv)
	def traceNode(self,node):
		self.tracer.traceNode(node)
	def trace(self, text):
		self.tracer.trace(text)
	def close(self):
			self.tracer.close()

class MinMaxAlphaBetaTracer_disabled:
	def restart(self, name):
		pass
	def trace(self, str):
		pass
	def traceTerminalNode(self, stateName, state, value, zsv):
		pass	
	def traceMaxDepthNode(self, stateName, state, value, zsv):
		pass
	def traceNodeAB(self, stateName, state, Moves, alpha, beta, pi):
		pass
	def traceAlphaBeta(self, stateName, bestV, v, alpha, beta, pi):
		pass
	def traceEdge(self, stateName, nname, mv):
		pass
	def traceNode(self,node):
		pass
	def close(self):
		pass

class MinMaxAlphaBetaGamePlayer_2P:
	def __init__(self, name, maxDepth, tracer, evalFunc):
		self.name = name
		self.calcnum = 0
		self.tracer = MinMaxAlphaBetaTracer(tracer,name) if tracer.enabled() else MinMaxAlphaBetaTracer_disabled()
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
		#state = deepcopy(rules.state)
		#state.name = [1]
		#state.alpha=-101
		#state.beta=101
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
			self.tracer.traceTerminalNode(stateName, state, value, zsv)
			return None,zsv
		if len(stateName) >= self.maxDepth:
			value = self.evalFcn(state)
			zsv = MinMaxAlphaBetaGamePlayer_2P.zeroSumValue(self.name,value)
			self.tracer.traceMaxDepthNode(stateName, state, value, zsv)
			return None,zsv
		Moves = self.rules.getLegalMoves(state, pi)
		bestV = -101 if pi==self.name else 101
		bestM = None
		self.tracer.traceNodeAB(stateName, state, Moves, alpha, beta, pi)
		
		for mv,i in zip(Moves,count(1)):
			ns = self.rules.getNextState(state,pi,mv)
			nname = stateName + [i]
			self.tracer.traceEdge(stateName, nname, mv)
			_, v = self.calcMoveRec(ns, self.rules.nextPlayer(pi), nname, alpha, beta)
			self.tracer.traceAlphaBeta(stateName, bestV, v, alpha, beta, pi)
			if pi == self.name:
				#maximizer
				if v > bestV:
					bestV = v
					bestM = mv
				if bestV >= beta:
					break
				#alpha = max(alpha,bestV)
				if bestV > alpha:
					alpha = bestV
			else:
				#minimizer					
				bestV = min(bestV,v)
				if bestV <= alpha:
					break
				#beta = min(beta, bestV)
				if bestV < beta:
					beta = bestV
		self.tracer.traceNode( Node(name=stateName, bestValue=bestV, bestMove=bestM, state = state, alpha=alpha, beta=beta, numMoves=len(Moves)))
		return bestM, bestV
