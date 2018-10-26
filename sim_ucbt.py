import numpy as np
import matplotlib.pyplot as plt
import math

C  = 1.0
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

