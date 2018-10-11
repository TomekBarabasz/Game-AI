from itertools import count

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

def create_eval_weightand_old(weights):
	def eval_weightedHand(state):
		def evhand(hand):
			cc = {}
			for c in hand:
				cn = c // 10
				cc[cn] = cc.get(cn,0) + 1
			return 100-sum( [weights[k-9][c] for k,c in cc.items()] )
		return [ evhand(h) for h in state.hand ]
	return eval_weightedHand

def create_eval_weightand(weights):
	def eval_weightedHand(state):
		def evhand(hand):
			cc = [0]*6
			for c in hand:
				cn = c // 10
				cc[cn-9] += 1
			acc = 0
			#for k,c in zip(count(0), cc): acc += weights[k][c]
			#return 100-acc
			return 100-sum( [ weights[k][c] for k,c in zip(count(0), cc) ] )
		return [ evhand(h) for h in state.hand ]
	return eval_weightedHand


def createEvalFunction(type):
	if type=='nco':
		return eval_numcardsonly
	elif type=='wh':
		return create_eval_weightand(weights)
	elif type=='wh1':
		return create_eval_weightand(weights1)
