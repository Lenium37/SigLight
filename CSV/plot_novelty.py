# !/usr/local/bin/python
import matplotlib.pyplot as plt
import numpy as np


# set starttime and endtime to show in plot here
plt.figure(1)
result1 = np.genfromtxt('/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/kernel.csv', delimiter=',')
plt.imshow(result1, cmap='inferno')
plt.colorbar()
plt.figure(2)
result2 = np.genfromtxt('/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/ssm_mfcc.csv', delimiter=',')
plt.imshow(result2, cmap='inferno')
plt.colorbar()
plt.figure(3)
result2 = np.genfromtxt('/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/ssm_chroma.csv', delimiter=',')
plt.imshow(result2, cmap='inferno')
plt.colorbar()
plt.figure(4)
time1, value1 = np.loadtxt('/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/novelty_function_combined.csv', delimiter=',', unpack=True, skiprows=0)
time2, value2 = np.loadtxt('/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/novelty_function_mfcc.csv', delimiter=',', unpack=True, skiprows=0)
time3, value3 = np.loadtxt('/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/novelty_function_chroma.csv', delimiter=',', unpack=True, skiprows=0)
time4, value4 = np.loadtxt('/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/extrema.csv', delimiter=',', unpack=True, skiprows=0)
time5, value5 = np.loadtxt('/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/segments.csv', delimiter=',', unpack=True, skiprows=0)
plt.plot(time1, value1, 'r', color='blue')
plt.plot(time2, value2, 'r', color='yellow')
plt.plot(time3, value3, 'r', color='purple')
plt.plot(time4, value4, 'o', color='green')
plt.plot(time5, value5, 'o', color='red')

#plt.set_title('Weighted Average Onset Detection Algorithm')
#plt.set_xlim([start, end])
#plt.autoscale(enable=True, axis='y')
plt.show()
