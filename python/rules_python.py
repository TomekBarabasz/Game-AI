from random import shuffle
from bisect import bisect_left,bisect_right
from itertools import count
from copy import copy,deepcopy
from functools import reduce

class GameRules:
	
	class State:
		def __init__(self):
			self.stack=[]
			self.hand=[]
			self.known=[]
			self.current = 0

		def hash(self):
			def subhash(cardlist):
				return reduce(lambda x,y : x | GameRules.HashMask[y], cardlist, 0)
			
			h0 = str( subhash(self.stack) )
			h1 = ''.join( [str(subhash(ph)) for ph in self.hand] )
			h2 = str(self.current)
			return h0+h1+h2
		
	def __init__(self):
		self.ActionMap = {'noop' : GameRules.actionNoop, 'play' : GameRules.actionPlay, 'take' : GameRules.actionTake }
		allcards = [kn*10+ks for kn in range(9,15) for ks in range(1,5)]
		GameRules.HashMask = { v:1<<i for v,i in zip(allcards,count(0)) }

	def start(self, numPlayers,allcards=None):
		#fluents : stack : list, hand : list, known : list
		state = GameRules.State()
		self.numPlayers = numPlayers
		if allcards is None:
			allcards = [kn*10+ks for kn in range(9,15) for ks in range(1,5)]
		shuffle(allcards)
		numCardsPerPlayer = len(allcards) // numPlayers
		for pn in range(0,numPlayers):
			h = sorted(allcards[ pn*numCardsPerPlayer : (pn + 1)*numCardsPerPlayer ])
			state.hand.append( h )
			state.known.append( list(h) if numPlayers==2 else [0]*numCardsPerPlayer)
			if h[0] == 91: 
				state.current = pn
		self.state = state
		self.submittedMoves = [None] * numPlayers
	
	def submitMove(self, pIdx, move):
		self.submittedMoves[pIdx] = move
	
	def nextState(self):
		state = self.state
		cont = True
		nextcpi = (state.current + 1 ) % self.numPlayers
		for m,pi in zip(self.submittedMoves,count(0)):
			state = self.getNextState(state, pi, self.submittedMoves[pi])
			if self.isTerminal(state) : 
				cont = False
				break
		state.current = nextcpi
		self.state = state
		self.submittedMoves = [None] * self.numPlayers
		return cont
	
	def nextPlayer(self, pIdx):
		return (pIdx + 1 ) % self.numPlayers

	def score(self, pIdx):
		isPlayerDone = [False] * self.numPlayers
		tos = ( self.state.stack[0] // 10 ) * 10
		playersDone = 0
		for pi in range(0,self.numPlayers):
			hand = self.state.hand[pi]
			done = (sum([ 1 for c in hand if (c // 10) == 14]) == 4 and len(hand) == 4) or len(hand)==0
			isPlayerDone[pi] = done
			if done: playersDone += 1
		if playersDone == self.numPlayers: return 50
		return 100 if isPlayerDone[pIdx] else 0

	@staticmethod
	def multiscore(state):
		Np = len(state.hand)
		isPlayerDone = [False] * Np
		tos = ( state.stack[0] // 10 ) * 10
		playersDone = 0
		for pi in range(0,Np):
			hand = state.hand[pi]
			done = (sum([ 1 for c in hand if (c // 10) == 14]) == 4 and len(hand) == 4) or len(hand)==0
			isPlayerDone[pi] = done
			if done: playersDone += 1
		if playersDone == Np: return [50]*Np
		return [100 if isPlayerDone[i] else 0 for i in range(0,Np)]

	@staticmethod
	def getLegalMoves(state, playerIdx):
		stack = state.stack
		hand = state.hand[playerIdx]
		moves=[]
		if state.current != playerIdx or len(hand) == 0:
			return [('noop',0)]
		
		ss = len(stack)
		if ss == 0:
			#beginning of game
			if hand[0] == 91:
				moves.append( ('play',[91]))
				if hand[1:4] == [92,93,94]:
					moves.append( ('play',[91,92,93,94]))
			else:
				moves.append( ('noop',0) )
			
		else:
			if ss==1 and hand[0:3] == [92,93,94]:
				moves.append( ('play',[92,93,94]))
			tos = (stack[0] // 10) * 10
			imin = bisect_left(hand, tos)
			if imin < len(hand):
				moves.extend( [('play',[hand[i]]) for i in range(imin,len(hand))] )
				quads = GameRules.findColorQuadsToPlay(hand, tos)
				moves.extend( [('play',q) for q in quads] )

			if   1==ss: pass
			elif 2==ss: moves.append( ('take',1))
			elif 3==ss: moves.append( ('take',2))
			else:       moves.append( ('take',3))
		
		if len(moves)==0:
			print('state is',state.__dict__)
			print('pIdx is',playerIdx)
			assert(len(moves))
		return moves
	
	@staticmethod
	def isTerminal(state):
		numPlayersWithEmptyHand = sum( [1 for h in state.hand if not h] )
		return numPlayersWithEmptyHand == len(state.hand)-1
	
	@staticmethod
	def replaceZeros(origin, new):
		firstNonZero = numZeros = bisect_right(origin,1)
		torem = len(new)
		zerosLeft = max(0,numZeros - torem)
		result = [0]*zerosLeft + origin[firstNonZero:] + new
		return sorted(result)

	@staticmethod
	def findColorQuadsToPlay(hand, tos):
		imin = bisect_left(hand, tos)
		cc = {}
		for i in range(imin,len(hand)):
			c = hand[i] // 10
			cc[c] = cc.get(c,0) + 1
		return [ [c*10+i for i in range(1,5)] for c,n in cc.items() if n==4 ]

	@staticmethod
	def actionNoop(state, pIdx, arg):
		return deepcopy(state)
	
	@staticmethod
	def actionTake(state, pIdx, cnt):
		nextState = GameRules.State()
		nextState.current = state.current
		cards = state.stack[0:cnt]
		np = len(state.hand)
		nextState.hand  = [None] * np
		nextState.known = [None] * np
		nextState.stack = state.stack[cnt:]
		nextState.current = state.current
		for pi in range(0,len(state.hand)):
			if pi != pIdx:
				nextState.hand[pi]  = state.hand[pi]
				nextState.known[pi] = state.known[pi]
			else:
				nextState.hand[pi] = sorted(state.hand[pi] + cards)
				nextState.known[pi] = sorted(GameRules.replaceZeros(state.known[pi],cards))
		return nextState

	@staticmethod
	def actionPlay(state,pIdx,cards):
		#print('action play, cards type is',type(cards))
		nextState = GameRules.State()
		nextState.stack = cards + state.stack
		np = len(state.hand)
		nextState.hand  = [None] * np
		nextState.known = [None] * np
		nextState.current = state.current
		for hand,known,pi in zip(state.hand, state.known, count(0)):
			#print('action play, hand type is',type(hand))
			if pi == pIdx:
				nextState.hand[pi]  = [ c for c in hand  if c not in cards ]
				nextState.known[pi] = [ c for c in known if c not in cards ]
			else:
				nextState.hand[pi] = hand
				nextState.known[pi] = known
		return nextState

	def getNextState(self, state, pIdx, move):
		Np = self.numPlayers
		ns =  self.ActionMap[move[0]](state,pIdx,move[1])
		ns.current = (state.current + 1 ) % Np
		return ns

def test():
	gr = GameRules()
	assert(  GameRules.replaceZeros([0,0,0,0,0,91,92,93],[101,102]) == [0,0,0,91,92,93,101,102] )
	assert(  GameRules.replaceZeros([0,91,92,93],[101,102]) == [91,92,93,101,102])
	assert(  GameRules.replaceZeros(  [91,92,93],[101,102]) == [91,92,93,101,102])

	gr.start(3)
	m0 = gr.getLegalMoves(gr.state, 0)
	m1 = gr.getLegalMoves(gr.state, 1)
	m2 = gr.getLegalMoves(gr.state, 2)

	gr.submitMove(0,m0[0])
	gr.submitMove(1,m1[0])
	gr.submitMove(2,m2[0])

	print(gr.submittedMoves)

	gr.nextState()
	print(gr.state.__dict__)

	gr.state.stack=[101,102,94,93,91]
	gr.state.hand = [ [121], [131], [141]]
	gr.state.known= [[0]*4,[0]*5,[0]*6]
	print('\nstate before take3',gr.state.__dict__)
	ns = gr.getNextState(gr.state, 0, ('take',3))
	print('state after take3',ns.__dict__)

if __name__ == '__main__':
	test()
