import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import math

simlength=50
N = np.arange(simlength)
V = np.arange(simlength)
UCT = np.zeros((simlength,simlength))
for n in N:
    for v in V:
        UCT[n,v] = math.sqrt( math.log(v+1)/(n+1) )
X,Y=np.meshgrid(N,V)
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
ax.plot_surface(X,Y,UCT)
ax.set_xlabel('num parent node visits')
ax.set_ylabel('num move node visits')
plt.show()
