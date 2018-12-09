import sys, argparse
from main import createPlayer
from tracer import createLogger
from game_controller import GameController
from main import createPlayer, createEndCondition, printResult
from rules_python import GameRules
from HookeJeevesPatternSearch import HookeJeevesPatternSearch
from itertools import chain

def main(args):
	parser = argparse.ArgumentParser()
	parser.add_argument('-numgames','-ng',		type=str, help='Number of games to play')
	parser.add_argument('-confint','-ci',		type=float, help='Confidence interval limit')
	parser.add_argument('-conf',					type=float, default=0.95, help='Confidence coefficient')
	parser.add_argument('-student', type=str, help='learning player type')	#choices=['random','minimax','human','lowcard']
	parser.add_argument('-opponent', type=str, help='opponent player type')
	parser.add_argument('-verbose','-v',action='store_true')
	parser.add_argument('-roundlimit','-rlim', type=int, default=100,help="round limit per game")
	parser.add_argument('-trace','-t',	type=str, help="filename for trace info")
	Args = parser.parse_args(args)

	def hjps_eval(weights):
		def eval_hand(state):
			def evhand(hand):
				cc = {}
				for c in hand:
					cn = c // 10
					cc[cn] = cc.get(cn,0) + 1
				return 100-sum( [weights[(k-9)*5 + c] for k,c in cc.items()] )
			return [ evhand(h) for h in state.hand ]

		players = { 0 : createPlayer(Args.student,  0, numplayers=2),
						1 : createPlayer(Args.opponent, 1, numplayers=2)
					}
		logger =  createLogger(None)
		rules = GameRules()
		gc = GameController(rules, players)
		endC = createEndCondition(Args)
		result = gc.run(endC, logger, showProgress=False, roundLimit=Args.roundlimit)
		return result['avg_score'][0]

	hjps = HookeJeevesPatternSearch ();
	iw = [ [0,6,12,18,6],#9
				[0,5,10,15,5],#10
				[0,4,8, 12,4],#W
				[0,3,6, 9, 3],#D
				[0,2,4, 8, 2],#k
				[0,1,2, 4, 1]]#A
	x0 = list(chain(*iw))

	def eval(x):
		
if __name__ == '__main__':
	main( sys.argv[1:])
