import sys,argparse
from itertools import count
from rules_python import GameRules
from human_player import HumanGamePlayer
from random_player import RandomGamePlayer, LowCardGamePlayer
from minmax_player import createMinmaxPlayer
from game_controller import GameController, EndCondition
from tracer import createTracer, createLogger
from datetime import datetime
import multiprocessing
import cProfile, pstats

def createPlayer(type_,name,**kwargs):
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

def mergeResults(mpResults):
	num_results = len(mpResults)
	num_games = sum( [r['total_games'] for r in mpResults] )
	avg_score = list(map(lambda x: sum(x)/num_results, zip(*[ r['avg_score'] for r in mpResults ])))
	total_rounds = sum( [r['total_rounds'] for r in mpResults] )
	avg_rounds = sum( [r['avg_rounds'] for r in mpResults] ) / num_results
	avg_move_time = list(map(lambda x : sum(x)/num_results, zip(*[ r['avg_move_time'] for r in mpResults ])))
	
	result = { 	'total_games'	: num_games,
					#'end_codes' 	: EndCodes,
					'avg_score' 	: avg_score,
					'total_rounds' : total_rounds,
					'avg_rounds'	: avg_rounds,
					'avg_move_time': avg_move_time
				}
	return result

def updateNumgamesForMp(numgames, mp):
	ng_i = map( lambda x: int(x)//mp, numgames.split(' '))
	return ' '.join( map(str, ng_i) )

def printResult(result):
	print("Run Stats :")
	print('Num games	: total {0}'.format(result['total_games']))
	if 'end_codes' in result:
		print('End codes	: {0}'.format(','.join([ '{0} {1}'.format(GameController.EndCodeNames[k],v) for k,v in result['end_codes'].items()] ) ))
	print('Avg Score	: {0}'.format(' : '.join(map(str,result['avg_score']))))
	print('Num rounds	: total {0} average rounds per game {1}'.format(result['total_rounds'], result['avg_rounds']))
	print('Avg move time	:',result['avg_move_time'])
	if 'exec_time' in result:
		print('Total exec time	: {0} sec'.format( result['exec_time'] ))
	if 'conf_interval' in result:
		print('Final Conf int	:', round(result['conf_interval'],2))

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
	parser.add_argument('-mp',type=int, help="number of processess to spawn")
	Args = parser.parse_args(args)

	if Args.mp is None:
		t0 = datetime.now()
		result = Play(Args)
		t1 = datetime.now()
	else:
		Results = []
		t0 = datetime.now()
		Args.numgames = updateNumgamesForMp(Args.numgames, Args.mp)
		with multiprocessing.Pool(Args.mp) as p:
			Results = p.map( Play, [Args]*Args.mp)
			result = mergeResults(Results)
		t1 = datetime.now()
	result['exec_time'] = t1-t0
	printResult(result)

def Play(Args):
	players = {}
	verbose = Args.verbose
	types = []
	for pn in range(0,4):
		type = Args.__dict__['p{0}'.format(pn+1)]
		if type is None: break
		if type=='human' : verbose = True
		types.append(type)

	for type,pn in zip(types,count(0)):
		players[pn] = createPlayer(type,pn,trace=Args.trace, graph=Args.graph, graphexe=Args.graphexe, numplayers=len(types))

	for p in players.values() :
		p.verbose = verbose
	
	typesDesc = 'player types are ' + '/'.join(types)
	#print(typesDesc)
	rules = GameRules()
	gc = GameController(rules, players)
	logger = createLogger(Args.gamelog)
	logger.log(typesDesc+'\n')
	endC = createEndCondition(Args)
	pr = cProfile.Profile()
	pr.enable()
	result = gc.run(endC, logger, Args.progress, Args.roundlimit)
	pr.disable()
	sortby = 'tottime'
	ps = pstats.Stats(pr).sort_stats(sortby)
	ps.print_stats()

	if Args.confplot:
		#TODO: move to mesurements object
		import matplotlib.pyplot as plt
		plt.plot(endC.ci)
		plt.xlabel('game number')
		plt.ylabel('confidence interval length')
		plt.suptitle("{0}% confidence interval".format(int(Args.conf*100)))
		plt.show()
	return result

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