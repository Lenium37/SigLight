# !/usr/local/bin/python
import matplotlib.pyplot as plt
import numpy as np


# set starttime and endtime to show in plot here

start = 30
end = 35


time1, av_t = np.loadtxt('average_threshold.csv', delimiter=',', unpack=True, skiprows=1)
time2, av_f = np.loadtxt('average_flux.csv', delimiter=',', unpack=True, skiprows=1)


# PLOT THRESHOLDS
plt.plot(time1, av_t, 'r')
plt.plot(time2, av_f, 'b')

#plt.set_title('Weighted Average Onset Detection Algorithm')
#plt.set_xlim([start, end])
#plt.autoscale(enable=True, axis='y')
plt.show()
