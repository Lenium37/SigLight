# !/usr/local/bin/python
import matplotlib.pyplot as plt
import csv
import numpy as np


# set starttime and endtime to show in plot here

result = np.genfromtxt('cb_kernel.csv', delimiter=',')
plt.imshow(result, vmin=-1.0, vmax=1.0, cmap='inferno')
plt.colorbar()
# PLOT MATRIX
plt.show()