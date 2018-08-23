import sys,argparse
from itertools import count
from gra_w_pana import GameRulezInPython
from game_player import createPlayer
from console_progressbar import ProgressBar

class GameRulez:
	def start(self, numPlayers): pass
	def submitMove(self, player, move):pass
	def nextState(self):pass
	def score(self, player):return None
	@staticmethod
	def multiscore(state, player):return None
	def getLegalMoves(self, state, player):return None
	def getNextState(self, state, moves):return None

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
		self.fp.write( 'stack is {0}\n'.format(state.stack))
		for hand,pi in zip(state.hand, count(0)):
			self.fp.write( 'p{0} hand is {1}\n'.format(pi,hand))
		
class GameController:
	def __init__(self, rulez, players):
		self.rulez = rulez
		self.players = players

	def run(self, numGames, logger, roundLimit=None):
		NumPlayers = len(self.players)
		TotalScore = [0] * NumPlayers
		TotalRounds = 0
		TotalRoundsAborted = 0

		for gn in range(1,numGames+1):
			logger.log('starting game {0}\n'.format(gn))
			aborted=False
			self.rulez.start(NumPlayers)
			CntRounds = 0
			pb = ProgressBar(total=roundLimit,prefix='game_{0}'.format(gn), suffix='', decimals=0, length=roundLimit, fill='#')
			for player in self.players.values():
				player.start(NumPlayers)
			while True:
				logger.log('round {0}\n'.format(CntRounds+1))
				logger.logState(self.rulez.state)
				for name, player in self.players.items():
					move = player.calcMove(self.rulez)
					if move[0] != 'noop': logger.log('player {0} does {1}\n'.format(name, move))
					self.rulez.submitMove(name, move)
				CntRounds += 1
				pb.print_progress_bar(CntRounds)
				if roundLimit is not None and CntRounds > roundLimit:
					TotalRoundsAborted += 1
					aborted=True
					break
				if not self.rulez.nextState():
					break
			if not aborted:
				score = [self.rulez.score(pi) for pi in range(0,NumPlayers)]
				logger.log('score {0}, rounds {1}\n'.format(score,CntRounds))
				TotalScore = [ a+b for a,b in zip(TotalScore, score) ]
				TotalRounds += CntRounds
			else:
				logger.log('rounds limit reached, game aborted\n')
			
		avgScore = [ str(round(s / numGames,2)) for s in TotalScore ]
		avgRounds = round(TotalRounds / numGames, 2)
		print('')
		print('Avg num rounds {0} Avg Score {1}'.format(avgRounds, '/'.join(avgScore) ))
		print('Aborted',TotalRoundsAborted,'rounds')
		
def play(args):
	parser = argparse.ArgumentParser()
	parser.add_argument('-numgames','-ng',		type=int, default=1, help='Number of games to play')
	parser.add_argument('-p1', type=str, help='player 1 type')	#choices=['random','minimax','human','lowcard']
	parser.add_argument('-p2', type=str, help='player 2 type')
	parser.add_argument('-p3', type=str, help='player 3 type')
	parser.add_argument('-p4', type=str, help='player 4 type')
	parser.add_argument('-verbose','-v',action='store_true')
	parser.add_argument('-roundlimit','-rlim', type=int, default=100,help="round limit per game")
	parser.add_argument('-trace','-t',	type=str, nargs='+', help="enable console tracing")
	parser.add_argument('-graph','-g',	type=str, nargs='+', help="filename for search graph")
	parser.add_argument('-graphexe','-ge',	type=str, help="graphvix executable for graph creation, like dot or twopi")
	parser.add_argument('-gamelog',type=str, help="filename for game logging")
	Args = parser.parse_args(args)

	players = {}
	verbose = Args.verbose
	types = []
	options = {}
	for pn in range(0,4):
		type = Args.__dict__['p{0}'.format(pn+1)]
		if type is None: break
		if type=='human' : verbose = True
		types.append(type)

	options = { 'trace' : Args.trace, 'graph' : Args.graph, 'graphexe' : Args.graphexe, 'numplayers' : len(types) }
	for type,pn in zip(types,count(0)):
		players[pn] = createPlayer(type,pn,options)

	for p in players.values() :
		p.verbose = verbose
	
	typesDesc = 'player types are ' + '/'.join(types)
	print(typesDesc)
	rules = GameRulezInPython()
	gc = GameController(rules, players)
	logger = GameLogger.create(Args.gamelog)
	logger.log(typesDesc+'\n')
	gc.run(Args.numgames, logger, Args.roundlimit)

def test(args):
	parser = argparse.ArgumentParser()
	parser.add_argument('-trace','-t',	type=int, default=0,help="enable console tracing")
	parser.add_argument('-graph','-g',	type=str, help="filename for search graph")
	parser.add_argument('-depth','-d',	type=int, default=3,help="search depth")
	parser.add_argument('-moves','-m',	type=int, help="search depth")
	Args = parser.parse_args(args)

	rules = GameRulezInPython()
	NumPlayers = 2
	p0 = createPlayer('minimax2P',0, Args)
	p1 = createPlayer('minimax2P',1, Args)
	rules.start(NumPlayers,[91,92,101,102,111,112,121,122])
	p0.start(NumPlayers)
	p1.start(NumPlayers)
	current = rules.state.current
	movesCnt=0
	while True:
		print('-----------')
		cpi = rules.state.current
		print('stack is',rules.state.stack)
		mv = p0.calcMove(rules)
		rules.submitMove(0,mv)
		if cpi==0:
			print('p0 hand is',rules.state.hand[0])
			print('p0 legal moves are',rules.getLegalMoves(rules.state,0))
			print('player0 selected',mv)
		mv = p1.calcMove(rules)
		rules.submitMove(1,mv)
		if cpi==1:
			print('p1 hand is',rules.state.hand[1])
			print('p1 legal moves are',rules.getLegalMoves(rules.state,1))
			print('player1 selected',mv)
		movesCnt += 1
		if Args.moves is not None and movesCnt >= Args.moves:
			print('move limit reached')
			break
		if not rules.nextState(): 
			break

	print('score is',rules.multiscore(rules.state))

if __name__ == '__main__':
	cmd = sys.argv[1]
	Cmds = {'play' : play, 'test' : test }
	if cmd in Cmds:
		Cmds[cmd]( sys.argv[2:]  )
	else:
		print('Valid commands are:', ', '.join(Cmds.keys()))
