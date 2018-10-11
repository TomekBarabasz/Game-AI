from copy import copy
import random

class HookeJeevesPatternSearch:
	def __init__(self):
		pass
	
	def run(self, valueFcn, x0, bounds, P0, T, a, verbose=False):
		P = copy(P0)
		fbest = valueFcn(x0)
		while True:
			result, fbest,x1 = self.exploratorySearch(x0,fbest,valueFcn, P,T, bounds)
			if verbose: print('exp {0} fbest {1:.3f}'.format('S' if result else 'F', fbest))
			if result == True:
				P = copy(P0)
				x0, x1, fbest = self.patternMove(x0, x1, fbest, valueFcn, P, T, a, bounds)
				if verbose: print('ptrn fbest {0:.3f}'.format(fbest))
			else:
				P = [p*0.5 for p in P]
				if any( map( lambda x,y: x < y, P, T ) ):
					break
		return ((fbest, x0),),P
	
	def exploratorySearch(self, x0, fbest, valueFcn, P, T, bounds):
		x = list(copy(x0))
		indices = list(range(len(x)))
		result = False
		while len(indices) != 0:
			i = random.choice(indices)
			indices.remove(i)
			xi = x[i]
			for p in (P[i], -P[i]):
				xip = xi+p
				if xip < bounds[i][0] or xip >= bounds[i][1]: continue
				x[i] = xip
				f = valueFcn(x)
				if f < fbest:
					fbest = f
					result = True
					break
				else:
					x[i] = xi
		
		return result, fbest, x
	
	def patternMove(self, x0, x1, fbest, valueFcn, P, T, a, bounds):
		while True:
			x2 = [x0_ + a*(x1_-x0_) for x0_,x1_ in zip(x0,x1)]
			result, fbest, x2 = self.exploratorySearch(x2, fbest, valueFcn, P, T, bounds)
			if result == True:
				x0 = x1
				x1 = x2
			else:
				x0 = x1
				break
		return x0, x1, fbest

def test1():
	print('test1: function x[0]*x[0] + x[1]*x[1] - x[0]*x[1]')
	print('expected bestf = -28.0, best x = (2.0, 4.0)')
	ps = HookeJeevesPatternSearch()
	def val(x): return 3*x[0]*x[0] + x[1]*x[1] - 12*x[0] - 8*x[1]
	x0=[1.,1.]
	P0 = (0.5,0.5)
	T = (0.1, 0.1)
	bounds = [(-100,100)]*2
	print ('starting fval=',val((1,1)))
	s,p = ps.run(val, [1,1], bounds, P0, T, 2)
	fbest, x = s[0]
	print ('final fval=',fbest)
	print ('best x=',x)
	
def test2():
	print('test2: function x[0]*x[0] + x[1]*x[1] - x[0]*x[1]')
	print('expected bestf = 0.0, best x = (0.0, 0.0)')
	def val(x): return x[0]*x[0] + x[1]*x[1] - x[0]*x[1]
	x0=(2.,2.)
	P0 = (0.5,0.5)
	T = (0.01, 0.01)
	print ('starting fval=',val(x0))
	ps = HookeJeevesPatternSearch()
	s,p = ps.run(val, x0, None, P0, T, 2)
	fbest, x = s[0]
	print ('final fval=',fbest)
	print ('best x=',x)

if __name__ == '__main__':
	print('HookeJeevesPatternSearch tests')
	test1()
	print('\n')
	test2()
	
	