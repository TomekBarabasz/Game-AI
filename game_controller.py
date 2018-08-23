import sys,argparse
from itertools import count
from console_progressbar import ProgressBar
	
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
		

