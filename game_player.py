from collections import deque
from msvcrt import getch
from subprocess import run
from itertools import count
from random import choice
import sys
from operator import lt,gt

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

def eval_numcardsonly(state):
	#winning score is 100, so this must be up to 100, where higher is better
	return [24 - len(h) for h in state.hand]

# num cards  0,1,2, 3, 4
weights = [ [0,6,12,18,6],#9
				[0,5,10,15,5],#10
				[0,4,8, 12,4],#W
				[0,3,6, 9, 3],#D
				[0,2,4, 8, 2],#k
				[0,1,2, 4, 1]]#A

# num cards  0,1,2, 3, 4
weights1 =  [[0,10,20,30,10],#9
				[0,5, 10,15,5],#10
				[0,4, 8, 12,4],#W
				[0,3, 6, 9, 3],#D
				[0,2, 4, 8, 2],#k
				[0,1, 2, 4, 1]]#A

"""def eval_weightedHand(state):
	def evhand(hand):
		cc = {}
		for c in hand:
			cn = c // 10
			cc[cn] = cc.get(cn,0) + 1
		return 100-sum( [weights[k-9][c] for k,c in cc.items()] )
	return [ evhand(h) for h in state.hand ]"""

def create_eval_weightand(weights):
	def eval_weightedHand(state):
		def evhand(hand):
			cc = {}
			for c in hand:
				cn = c // 10
				cc[cn] = cc.get(cn,0) + 1
			return 100-sum( [weights[k-9][c] for k,c in cc.items()] )
		return [ evhand(h) for h in state.hand ]
	return eval_weightedHand

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

class HumanGamePlayer:
	def __init__(self, name,*a):
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

class Tracer:
	def __init__(self):pass
	def traceNode(self,node):pass
	def traceEdge(self, from_, to_, label):pass
	def trace(self, *txt):pass
	def restart(self,idxToHighlight):pass
	def close(self):pass

class ConsoleTracer(Tracer):
	def __init__(self,lvl):
		self.lvl = lvl
	def traceNode(self, n):
		print('player',n.state.current)
		print('bestv=',n.bestValue)
		print('stack=',n.state.stack)
		print('p0 hand=',n.state.hand[0])
		print('p1 hand=',n.state.hand[1])
		if hasattr(n,'moves'):
			print('valid moves=',n.moves)
	def trace(self, *args):
		print(args)

class FileTracer(Tracer):
	def __init__(self,lvl,logfile):
		self.lvl = lvl
		print('opening trace file',logfile)
		self.fn = open(logfile,'w')
	def traceNode(self, n):
		text = 'backtracking node {0} bv {1}'.format(n.name, n.bestValue)
		text += ' bm {0}\n'.format(n.bestMove) if hasattr(n, 'bestMove') else '\n'
		self.fn.write(text)
	def trace(self, *args):
		self.fn.write(' '.join(map(str,args)) + '\n')
	def traceEdge(self, from_, to_, label):
		self.fn.write('node {0} trying move {1}\n'.format(from_,label))

class GraphTracer(Tracer):
	def __init__(self, path, name_, exe):
		self.path = path	
		self.calcnum = 1
		self.name = name_
		self.exe = exe
		self.nodedesc = GraphTracer.nodeDescHtml if exe is not None else GraphTracer.nodeDesc

	def restart(self,idxToHighlight):
		self.calcnum += 1
		self.fn = fn = self.path + '\\' + '{0}-{1}.gv'.format(self.name, self.calcnum)
		self.gf = open(fn,'w')
		self.gf.write('digraph g {\n')
		self.idxToHighlight = idxToHighlight

	@staticmethod
	def nodeName2Str(name):
		return '.'.join(map(str,name))

	@staticmethod
	def nodeDescHtml(node,idxToHighlight):
		if node.state.current == idxToHighlight:
			pn = '<tr><td align="left" bgcolor="black"><font color="white">player {0}</font></td></tr>'.format(node.state.current)
		else:
			pn = '<tr><td align="left">player {0}</td></tr>'.format(node.state.current)
		
		beg = '"node{0}" [color="red" penwidth=4.0 ' if node.name==[1] else '"node{0}" [ '
		nd = beg.format(GraphTracer.nodeName2Str(node.name))
		nd += 'shape="box" fontname="Courier New" label=<<table  border="0" cellborder="0">' + pn
		nd += '<tr><td align="left">best={0}</td></tr>'.format(node.bestValue)
		nd += '<tr><td align="left">stack={0}</td></tr>'.format(node.state.stack)
		nd += '<tr><td align="left">p0 hand={0}</td></tr>'.format(node.state.hand[0])
		nd += '<tr><td align="left">p1 hand={0}</td></tr>'.format(node.state.hand[1])
		if hasattr(node,'numMoves'):
			nd +='<tr><td align="left">number of moves={0}</td></tr>'.format(node.numMoves)
		if hasattr(node,'bestMove'):
			nd +='<tr><td align="left">best move={0}</td></tr>'.format(node.bestMove)
		if hasattr(node,'alpha'):
			nd +='<tr><td align="left">alpha={0} beta={1}</td></tr>'.format(node.alpha, node.beta)
		nd += '</table>>];\n'
		return nd
	
	@staticmethod
	def nodeDesc(node,idxToHighlight):
		return '"node{0}" [label="player{1}\\nbest={2}\\nstack={3}\\np0 hand={4}\\np1 hand={5}\\nbest move={6}"];\n'.format(
			GraphTracer.nodeName2Str(node.name), node.state.current, node.bestValue,node.state.stack, node.state.hand[0], node.state.hand[1], node.bestMove if hasattr(node,'bestMove') else '')
	def traceNode(self, node):
		self.gf.write(self.nodedesc(node, self.idxToHighlight))	
	def traceEdge(self, from_, to_, label):
		self.gf.write('"node{0}" -> "node{1}" [label="{2} {3}"]\n'.format(GraphTracer.nodeName2Str(from_), GraphTracer.nodeName2Str(to_), *label))
	def close(self):
		self.gf.write('}\n')
		self.gf.close()
		self.gf = None
		if self.exe is not None:
			svg = self.fn[ 0 : self.fn.rfind('.')] + '.svg'
			run([self.exe,'-Tsvg', self.fn, '-o', svg,'-Goverlap=prism'])

class CompositeTracer(Tracer):
	def __init__(self, tracers):
		self.tracers = tracers
	def traceNode(self,n):	
		for t in self.tracers: t.traceNode(n)
	def traceEdge(self,f,to,l):
		for t in self.tracers: t.traceEdge(f,to,l)
	def trace(self,*txt):			
		for t in self.tracers: t.trace(*txt)
	def restart(self,idxToHighlight):			
		for t in self.tracers: t.restart(idxToHighlight)
	def close(self):				
		for t in self.tracers: t.close()

class MinMaxGamePlayer_2P:
	def __init__(self, name, maxDepth=10):
		self.name = name
		self.calcnum = 0
		self.tracer = Tracer()
		self.maxDepth = maxDepth
		self.moveNbr = 1
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
	def __init__(self, name, maxDepth):
		self.name = name
		self.calcnum = 0
		self.tracer = Tracer()
		self.maxDepth = maxDepth
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

class MinMaxAlphaBetaGamePlayer_2P:
	def __init__(self, name, maxDepth=10):
		self.name = name
		self.calcnum = 0
		self.tracer = Tracer()
		self.maxDepth = maxDepth
		self.moveNbr = 1
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

class MCGamePlayer_3P:
	def __init__(self, name):
		self.name = name
	def start(self,numPlayers):
		pass
	def calcMove(self, rulez):
		return None

def createEvalFunction(type):
	if type=='nco':
		return eval_numcardsonly
	elif type=='wh':
		return create_eval_weightand(weights)
	elif type=='wh1':
		return create_eval_weightand(weights1)

def createMinimaxPlayer(name,Type,args):
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
		P = MCGamePlayer_3P
	print('creating player',name,'of type',P)
	depth = int(Type[1][1:]) if len(Type) > 1 and Type[1].startswith('d') else 10
	player =  P(name,depth)
	
	ef = createEvalFunction(Type[2] if len(Type)>3 else 'nco')
	
	player.evalFcn = ef
	return player

def createTracer(args,type,pIdx):
	name = '{0}-p{1}'.format(type, pIdx)
	Trace = args['trace']
	if Trace is None: Trace=[]
	trace = (0,None)
	for t in Trace:
		tt = t.split(' ')
		pi = int(tt[0][1])-1
		if pi == pIdx:
			trace = ( int(tt[1]),tt[2] if len(tt)==3 else None )
			break

	Graph = args.get('graph',[])
	if Graph is None: Graph=[]
	graph = None
	for g in Graph:
		gg = g.split(' ')
		pi = int( gg[0][1] )-1
		if pi == pIdx:
			graph = gg[1]

	graphExe = args.get('graphexe',None)
	if trace[0] > 0:
		logger = ConsoleTracer(trace[0]) if trace[1] is None else FileTracer(trace[0], trace[1]+'\\'+name+'.log')
	if graph is not None:
		grapher = GraphTracer(graph,name,graphExe)
	if trace[0] > 0 and graph is not None:
		tracer = CompositeTracer( [logger, grapher] )
	elif trace[0] > 0:
		tracer = logger
	elif graph is not None:
		tracer = grapher
	else:
		tracer = Tracer()
	
	return tracer

def createPlayer(type_,name,kwargs):
	Type = type_.split(' ')
	PlayerFactory = {'random':RandomGamePlayer, 'human':HumanGamePlayer, 'lowcard':LowCardGamePlayer, 'minimax':createMinimaxPlayer}
	tracer = createTracer(kwargs,type_,name)
	p = PlayerFactory[Type[0]](name,Type,kwargs)
	p.tracer=tracer
	return p
