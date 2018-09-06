from itertools import count
from random import choice
from eval_func import createEvalFunction

class Node:
	def __init__(self, **kwargs):
		for k,v  in kwargs.items():
			setattr(self,k,v)
	
	def updateBest(self, bv, bm):
		i = self.state.current
		if bv[i] > self.bestValue[i]:
			#print('found new best value ',bv,'was',self.bestValue,'for player',i)
			self.bestValue = bv
			self.bestMove = bm

class MinMaxGamePlayer_2P:
	def __init__(self, name, maxDepth, tracer, evalFunc):
		self.name = name
		self.calcnum = 0
		self.tracer = tracer
		self.maxDepth = maxDepth
		self.moveNbr = 1
		self.evalFcn = evalFunc
	def start(self,numPlayers):
		pass
	def calcMove(self, rules):	
		self.rules = rules
		self.tracer.restart(self.name)
		Moves = self.rules.getLegalMoves(rules.state, self.name)
		if len(Moves)==1: return Moves[0]
		self.tracer.trace('---start calc move #{0}---'.format(self.moveNbr))
		self.nodesVisited = 0
		mv,_ = self.calcMoveRec(rules.state, self.name, [1])
		self.tracer.trace('visited {0} nodes'.format(self.nodesVisited))
		self.tracer.close()
		self.moveNbr+=1
		#if len(mv)>1:
		#	print('found equaly valued moves:',mv)
		return mv[0] if len(mv)==1 else choice(mv)

	def calcMoveRec(self, state, pi, stateName):
		self.nodesVisited += 1
		if self.rules.isTerminal(state):
			value = self.rules.multiscore(state)
			self.tracer.trace('terminal node {0} d{1} score {2}'.format(stateName,len(stateName),value))
			self.tracer.traceNode( Node(name=stateName, bestValue=value, state=state) )
			return None,value
		if len(stateName) >= self.maxDepth:
			value = self.evalFcn(state)
			self.tracer.trace('max depth node {0} d{1} eval {2}'.format(stateName, len(stateName), value))
			self.tracer.traceNode( Node(name=stateName, bestValue=value, state=state) )
			return None,value
		Moves = self.rules.getLegalMoves(state, pi)
		bestV = [-100] * 2
		bestM = []
		self.tracer.trace('new node {0} d{1} {2} moves'.format(stateName, len(stateName), len(Moves)))
		self.tracer.trace('stack={0}\np0 hand={1}\np1 hand={2}'.format(state.stack, state.hand[0], state.hand[1]))
		for mv,i in zip(Moves,count(1)):
			ns = self.rules.getNextState(state,pi,mv)
			nname = stateName + [i]
			self.tracer.traceEdge(stateName, nname, mv)
			_,value = self.calcMoveRec(ns, self.rules.nextPlayer(pi), nname)
			if value[pi] == bestV[pi]:
				bestM.append(mv)
			elif value[pi] > bestV[pi]:
				self.tracer.trace('node {0} best value updated to {1}'.format(stateName, value))
				bestV = value
				bestM = [mv]
		self.tracer.traceNode( Node(name=stateName, bestValue=bestV, bestMove=bestM, state = state))
		return bestM, bestV

	def calcMoveList(self, rulez):
		Np = len(rulez.state.hand)
		Moves = rulez.getLegalMoves(rulez.state,self.name)
		if len(Moves) == 1: return Moves[0]
		self.tracer.trace('calcMove for player',self.name)
		nodes = [ Node(name='1',bestValue=[-100]*Np,bestMove=None,state=rulez.state,moves=Moves) ]
		self.tracer.restart(self.name)
		while len(nodes):
			n = nodes[ -1 ]
			if len(nodes) > self.maxDepth :
				value = self.evalFcn(n.state)
				self.tracer.trace('too deep, escape, eval is',value)
				n.bestValue = value
				self.tracer.traceNode(n)
				del nodes[-1]
				n = nodes[-1]
				n.updateBest(value,n.moves[-1])
				del n.moves[-1]
			else:
				self.tracer.trace('--- depth',len(nodes),'---player',n.state.current,' ---')
				self.tracer.trace('bestv=',n.bestValue)
				self.tracer.trace('stack=',n.state.stack)
				self.tracer.trace('p0 hand=',n.state.hand[0])
				self.tracer.trace('p1 hand=',n.state.hand[1])
				self.tracer.trace('valid moves=',n.moves)
				if len(n.moves) > 0:
					mv = n.moves[-1]
					self.tracer.trace('trying move',mv,)
					ns = rulez.getNextState(n.state,n.state.current,mv)
					nname = n.name + str(len(n.moves))
					self.tracer.traceEdge(n.name, nname, mv)
					if rulez.isTerminal(ns):
						value = rulez.multiscore(ns)
						self.tracer.traceNode( Node(name=nname, bestValue=value, state=ns) )
						self.tracer.trace('terminal state reached, score is',value)
						n.updateBest(value, mv)
						del n.moves[-1]
					else:
						nodes.append( Node(name=nname, bestValue =[-100]*Np, bestMove=None, state=ns, moves=rulez.getLegalMoves(ns,ns.current)) )
						self.tracer.trace('descending')
				else:
					value = n.bestValue
					self.tracer.traceNode(n)
					mv = n.bestMove
					del nodes[-1]
					self.tracer.trace('no more moves, ascending')
					if len(nodes):
						n = nodes[-1]
						n.updateBest(value,n.moves[-1])
						del n.moves[-1]
						
			#wait()
		#}end of while	
		self.tracer.close()
		return mv

class MinMaxFullSearchGamePlayer_2P:
	def __init__(self, name, maxDepth, tracer, evalFunc):
		self.name = name
		self.calcnum = 0
		self.tracer = tracer
		self.maxDepth = maxDepth
		self.evalFcn = evalFunc
		print('maxDepth',maxDepth)
	def start(self,numPlayers):
		pass

	def calcMove(self, rules):	
		self.rules = rules
		self.tracer.restart(self.name)
		Moves = self.rules.getLegalMoves(rules.state, self.name)
		if len(Moves)==1: return Moves[0]
		mv,v = self.calcMoveRec(rules.state, self.name, '1')
		self.tracer.close()
		if v[self.name] != 100:
			print('suboptimal move',mv,'eval',v)
		#if len(mv)>1:
		#	print('found equaly valued moves:',mv)
		return mv[0] if len(mv)==1 else choice(mv)

	def calcMoveRec(self, state, pi, stateName):
		if self.rules.isTerminal(state):
			value = self.rules.multiscore(state)
			self.tracer.trace('terminal state reached, score is',value)
			self.tracer.traceNode( Node(name=stateName, bestValue=value, state=state) )
			return None,value
		if len(stateName) >= self.maxDepth:
			value = self.evalFcn(state)
			self.tracer.trace('too deep, escape, eval is',value)
			self.tracer.traceNode( Node(name=stateName, bestValue=value, state=state) )
			return None,value
		Moves = self.rules.getLegalMoves(state, pi)
		bestV = [-100] * 2
		bestM = []
		for mv,i in zip(Moves,count(1)):
			ns = self.rules.getNextState(state,pi,mv)
			nname = stateName + str(i)
			self.tracer.traceEdge(stateName, nname, mv)
			_,value = self.calcMoveRec(ns, self.rules.nextPlayer(pi), nname)
			if value[pi] == bestV[pi]:
				bestM.append(mv)
			elif value[pi] > bestV[pi]:
				bestV = value
				bestM = [mv]
				if value[pi] == 100 and stateName == '1':
					break
		self.tracer.traceNode( Node(name=stateName, bestValue=bestV, bestMove=bestM, state = state))
		return bestM, bestV

def createMinmaxPlayer(name,Type,tracer,args):
	from minmax_ab_player import MinMaxAlphaBetaGamePlayer_2P
	#Type[0] : basic type
	#Type[1] : dx #depth
	#Type[2] : eval fcn : 
	#Type[3] : subtype ab
	if args['numplayers'] == 2:
		if len(Type) < 4:
			P = MinMaxGamePlayer_2P
		else:
			Subtypes = { 'ab' : MinMaxAlphaBetaGamePlayer_2P, 'exact': MinMaxFullSearchGamePlayer_2P}
			P = Subtypes[ Type[3] ]
	else:
		from mc_player import MCGamePlayer_3P
		P = MCGamePlayer_3P
	print('creating player',name,'of type',P)
	depth = int(Type[1][1:]) if len(Type) > 1 and Type[1].startswith('d') else 10
	
	ef = createEvalFunction(Type[2] if len(Type)>3 else 'nco')
	player =  P(name,depth, tracer, ef)
	return player

