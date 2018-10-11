from subprocess import run
from itertools import count

def num2Suites(cards, separator=' '):
	suits = '♥♠♣♦'
	values=['9','10','W','D','K','A']
	return separator.join([ values[x//10-9]+suits[x%10-1] for x in cards])

def move2str(move):
	if move[0] == 'play':
		return move[0] + ' ' + num2Suites(move[1])
	elif move[0] == 'take':
		return move[0] + ' ' + str(move[1])
	elif move[0] == 'noop':
		return move[0]

class GameLogger:
	class DummyLogger:
		def log(self,*args):pass
		def logState(self, state):pass
	@classmethod
	def create(cls, filename):
		return GameLogger(filename) if filename is not None else GameLogger.DummyLogger()
	def __init__(self, filename):
		self.fp = open(filename,'w')
	def log(self, *args):
		self.fp.write( *args )	
	def logState(self, state):
		self.fp.write( 'state hash is {0}\n'.format(state.hash()))
		self.fp.write( 'stack is {0}\n'.format(state.stack))
		for hand,pi in zip(state.hand, count(0)):
			self.fp.write( 'p{0} hand is {1}\n'.format(pi,hand))

class Tracer:
	def __init__(self):pass
	def enabled(self):return False
	def traceNode(self,node):pass
	def traceEdge(self, from_, to_, label):pass
	def trace(self, *txt):pass
	def restart(self,idxToHighlight):pass
	def close(self):pass

class ConsoleTracer(Tracer):
	def __init__(self,lvl):
		self.lvl = lvl
	def enabled(self):return True
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
	def enabled(self):return True
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
	def enabled(self):return True
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
	def enabled(self):return True
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

def createTracer(args,type,pIdx):
	name = '{0}-p{1}'.format(type, pIdx)
	Trace = args.get('trace',None)
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
	#else: print("Graph is",Graph)
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

def createLogger(filename):
	return GameLogger.create(filename)
