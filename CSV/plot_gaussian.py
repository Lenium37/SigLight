# !/usr/local/bin/python
import matplotlib.pyplot as plt
import csv
import numpy as np


# set starttime and endtime to show in plot here

result = np.genfromtxt('gaussian_kernel.csv', delimiter=',')
plt.imshow(result, vmin=-0.008, vmax=0.008, cmap='inferno')
plt.colorbar()
# PLOT MATRIX
plt.show()