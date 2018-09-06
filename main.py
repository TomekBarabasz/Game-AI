import sys,argparse
from itertools import count
from rules_python import GameRules
from human_player import HumanGamePlayer
from random_player import RandomGamePlayer, LowCardGamePlayer
from minmax_player import createMinmaxPlayer
from game_controller import GameController, EndCondition
from tracer import createTracer, createLogger

def createPlayer(type_,name,kwargs):
	Type = type_.split(' ')
	PlayerFactory = {'random':RandomGamePlayer, 'human':HumanGamePlayer, 'lowcard':LowCardGamePlayer, 'minmax':createMinmaxPlayer}
	tracer = createTracer(kwargs,type_,name)
	p = PlayerFactory[Type[0]](name,Type,tracer, kwargs)
	return p

def createEndCondition(Args):
	if Args.confint is not None:
		if Args.numgames.find(' ') > -1:
			mingames,maxgames = map(int,Args.numgames.split(' '))
		else:
			maxgames = int(Args.numgames)
		return EndCondition.createConfidenceIntervalBasedEndCondition(Args.confint, confidence=Args.conf, minLimit=1, maxLimit=maxgames)
	else:
		return EndCondition.createSimpleGameLimit(int(Args.numgames))

def play(args):
	parser = argparse.ArgumentParser()
	parser.add_argument('-numgames','-ng',		type=str, default="1", help='Number of games to play')
	parser.add_argument('-confint','-ci',		type=float, help='Confidence interval limit')
	parser.add_argument('-conf',					type=float, default=0.95, help='Confidence coefficient')
	parser.add_argument('-confplot',	action='store_true')
	parser.add_argument('-p1', type=str, help='player 1 type')	#choices=['random','minimax','human','lowcard']
	parser.add_argument('-p2', type=str, help='player 2 type')
	parser.add_argument('-p3', type=str, help='player 3 type')
	parser.add_argument('-p4', type=str, help='player 4 type')
	parser.add_argument('-verbose','-v',action='store_true')
	parser.add_argument('-roundlimit','-rlim', type=int, default=100,help="round limit per game")
	parser.add_argument('-trace','-t',	type=str, nargs='+', help="enable console tracing")
	parser.add_argument('-graph','-g',	type=str, nargs='+', help="filename for search graph")
	parser.add_argument('-graphexe','-ge',	type=str, help="graphviz executable for graph creation, like dot or twopi")
	parser.add_argument('-gamelog',type=str, help="filename for game logging")
	parser.add_argument('-progress','-p',action='store_true')
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
	rules = GameRules()
	gc = GameController(rules, players)
	logger = createLogger(Args.gamelog)
	logger.log(typesDesc+'\n')
	endC = createEndCondition(Args)
	gc.run(endC.end, logger, Args.progress, Args.roundlimit)
	if Args.confplot:
		#TODO: move to mesurements object
		import matplotlib.pyplot as plt
		plt.plot(endC.ci)
		plt.xlabel('game number')
		plt.ylabel('confidence interval length')
		plt.suptitle("{0}% confidence interval".format(int(Args.conf*100)))
		plt.show()
	if Args.confint is not None:
		print('Final Conf int	:', round(endC.ci[-1],2))
	
def test(args):
	parser = argparse.ArgumentParser()
	parser.add_argument('-trace','-t',	type=int, default=0,help="enable console tracing")
	parser.add_argument('-graph','-g',	type=str, help="filename for search graph")
	parser.add_argument('-depth','-d',	type=int, default=3,help="search depth")
	parser.add_argument('-moves','-m',	type=int, help="search depth")
	Args = parser.parse_args(args)

	rules = GameRules()
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

def help(args):
	print("""how to create players:
  p1 human
  p1 random
  p1 minimax d<depth> eval_function subtype, where eval_function=[nco,wh,wh1] subtype=[exact|ab]
  p1 lowcard
  
how to create graph trace
  -graph "p1 folder" "p2 folder"
  -graphexe "C:\Program Files (x86)\Graphviz2.38\bin\twopi.exe"
  -graphexe "C:\Program Files (x86)\Graphviz2.38\bin\dot.exe"
""")

if __name__ == '__main__':
	cmd = sys.argv[1]
	Cmds = {'play' : play, 'test' : test, 'help' : help }
	if cmd in Cmds:
		Cmds[cmd]( sys.argv[2:]  )
	else:
		print('Valid commands are:', ', '.join(Cmds.keys()))