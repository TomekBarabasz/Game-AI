import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from matplotlib import cm
import math

def plot1():
	C  = 2.0
	visits = np.arange(1,50,1)
	#value_0 = math.sqrt( math.log(  
	value_0 = C * np.sqrt( np.log( visits ) / visits )
	value_50 = 0.5 + C * np.sqrt( np.log( visits ) / visits )
	value_100 = 1.0 + C * np.sqrt( np.log( visits ) / visits )

	fig = plt.figure()
	ax = fig.add_subplot(111)
	ax.plot(visits, value_0, color='b', marker='.', label='value=0')
	ax.plot(visits, value_50, color='m', marker='.', label='value=50')
	ax.plot(visits, value_100, color='g', marker='.', label='value=100')
	ax.legend()
	plt.show()

def plot2():
	time   = np.linspace(1,50)
	visits = np.linspace(1,50)
	X,Y = np.meshgrid(time,visits)
	UCB = np.zeros((50,50),float)
	for t in range(1,50):
		for v in range(1,50):
			UCB[v][t] = np.sqrt( np.log( t ) / v )
	fig = plt.figure()
	#ax = fig.add_subplot(111)
	#cmap=plt.get_cmap(Args.cmap)
	#c=ax.pcolor(X,Y,Z, cmap=cmap)
	ax = fig.add_subplot(111, projection='3d')
	#ax.scatter(time,visits,UCB)
	ax.plot_surface(X,Y,UCB)
	ax.set_xlabel('parent node visit count')
	ax.set_ylabel('action node visi count')
	ax.set_zlabel('upper confidence bound')
	plt.show()
	
if __name__ == '__main__':
	plot2()

