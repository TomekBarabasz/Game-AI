from console_progressbar import ProgressBar
from itertools import count
from datetime import datetime

class EndCondition:
	@staticmethod
	def createSimpleGameLimit(limit):
		class SimpleGameLimit:
			def __init__(self, limit):
				self.limit = limit
				self.numGames = 0
			def end(self, score):
				self.numGames += 1
				return self.numGames >= self.limit
			def getConfidenceInterval(self):
				return None
		return SimpleGameLimit(limit)

	@staticmethod
	def createConfidenceIntervalBasedEndCondition(conf_interval_limit, confidence=0.95, minLimit=30, maxLimit=200):
		import numpy as np
		import scipy.stats as st
		class ConfidenceIntervalBasedEndCondition:
			def __init__(self, conf_interval_limit, confidence, minLimit, maxLimit):
				self.minLimit = minLimit
				self.maxLimit = maxLimit
				self.conf_interval_limit = conf_interval_limit
				self.confidence = confidence
				self.scores = []
				self.ci = []
				self.calcCI = self.calcCI_ndist if minLimit > 100 else self.calcCI_tdist

			def calcCI_ndist(self):
				a = 1.0 * np.array(self.scores)
				interval = st.norm.interval(self.confidence, loc=np.mean(a), scale=st.sem(a))
				return (interval[1]-interval[0])*0.5

			def calcCI_tdist(self):
				a = 1.0 * np.array(self.scores)
				interval = st.t.interval(self.confidence, df = len(a)-1, loc=np.mean(a), scale=st.sem(a))
				return (interval[1]-interval[0])*0.5
			
			def end(self, score):
				self.scores.append( score[0] )
				numGames = len(self.scores)
				if numGames < 2 : 
					return numGames > self.maxLimit
				
				#ci = self.calcCI()
				ci = self.calcCI_tdist() if numGames < 100 else self.calcCI_ndist()
				self.ci.append(ci)
				if numGames < self.minLimit: 
					return False
				else:
					return ci < self.conf_interval_limit or numGames > self.maxLimit

			def getConfidenceInterval(self):
				return self.ci[-1]
			
		return ConfidenceIntervalBasedEndCondition(conf_interval_limit, confidence, minLimit, maxLimit)

class GameController:
	FinishedByWin 			= 0
	FinishedByRoundLimit = 1
	FinishedByStateLoop  = 2
	EndCodeNames = { FinishedByWin:'win', FinishedByRoundLimit:'rlim',FinishedByStateLoop:'loop'}
	def __init__(self, rulez, players):
		self.rulez = rulez
		self.players = players
	
	def runSingleGame(self, logger, progressBar, roundLimit):
		NumPlayers = len(self.players)
		self.rulez.start(NumPlayers)	
		for player in self.players.values():
			player.start(NumPlayers)
		visitedStates = {}
		MovesCnt = [0]*NumPlayers
		TotalMoveCalcTime = [0.0]*NumPlayers
		
		for CntRounds in range(1,roundLimit+1):
			endCode = GameController.FinishedByRoundLimit
			logger.log('round {0}\n'.format(CntRounds))
			logger.logState(self.rulez.state)
			for name, player in self.players.items():
				t0 = datetime.now()
				move = player.calcMove(self.rulez)
				if move[0] != 'noop':
					MovesCnt[name] += 1
					TotalMoveCalcTime[name] += (datetime.now() - t0).total_seconds()
					logger.log('player {0} does {1}\n'.format(name, move))
				self.rulez.submitMove(name, move)
			progressBar.print_progress_bar(CntRounds)

			if not self.rulez.nextState():
				endCode = GameController.FinishedByWin
				score = [self.rulez.score(pi) for pi in range(0,NumPlayers)]
				logger.log('score {0}, rounds {1}\n'.format(score,CntRounds))
				break
				
			h = self.rulez.state.hash()
			if h in visitedStates:
				logger.log( "\nnext state already visited -> play loop detected, game aborted\n")
				logger.logState(self.rulez.state)
				break
			else:	
				visitedStates[h] = True
		
		if endCode != GameController.FinishedByWin:
			score = [100 // NumPlayers]*NumPlayers
		if endCode == GameController.FinishedByRoundLimit:
			logger.log('rounds limit reached, game aborted\n')
		
		return score, CntRounds, endCode, [a/b for a,b in zip(TotalMoveCalcTime, MovesCnt)]
	
	def run(self, endCondition, logger, showProgress=True,roundLimit=1000):
		class dummyProgressBar:
			def print_progress_bar(self, val): pass
		NumPlayers = len(self.players)
		TotalScore = [0] * NumPlayers
		TotalRounds = 0
		EndCodes = {}
		AvgMoveTime = [0.0]*NumPlayers
		
		for gn in count(1):
			logger.log('\nstarting game {0}\n'.format(gn))
			pb = ProgressBar(total=roundLimit,prefix='game_{0}'.format(gn), suffix='', decimals=0, length=roundLimit, fill='#') if showProgress else dummyProgressBar()
			score,roundsDone,endCode,avgMoveTime = self.runSingleGame(logger, pb, roundLimit)
			AvgMoveTime = [a+b for a,b in zip(AvgMoveTime, avgMoveTime)]
			EndCodes[endCode] = EndCodes.get(endCode,0)+1
			TotalScore = [ a+b for a,b in zip(TotalScore, score) ]
			TotalRounds += roundsDone
			if endCondition.end(score):
				break

		AvgScore    = [ round(s / gn,2) for s in TotalScore ]
		avgRounds   = round(TotalRounds / gn, 2)
		AvgMoveTime = [ round(a / gn, 5) for a in AvgMoveTime]

		result = { 	'total_games'	: gn,
						'end_codes' 	: EndCodes,
						'avg_score' 	: AvgScore,
						'total_rounds' : TotalRounds,
						'avg_rounds'	: avgRounds,
						'avg_move_time': AvgMoveTime,
						}
		ci = 	endCondition.getConfidenceInterval()
		if ci is not None:
			result['conf_interval'] = ci
		return result
		
		
