# !/usr/local/bin/python
import matplotlib.pyplot as plt
import numpy as np


# set starttime and endtime to show in plot here

start = 0
end = 19


time1, ted, tsd, tsdhwr, tcsd, thfc, tl2nh, tl1nh, tl2h, tl1h = np.loadtxt('thresholds.csv', delimiter=',', unpack=True, skiprows=1)
time2, ed, sd, sdhwr, csd, hfc, l2nh, l1nh, l2h, l1h = np.loadtxt('spectral_flux.csv', delimiter=',', unpack=True, skiprows=1)

fig, axs = plt.subplots(3, 3)

# PLOT THRESHOLDS
axs[0, 0].plot(time1, ted, 'r')
axs[0, 1].plot(time1, tsd, 'r')
axs[0, 2].plot(time1, tsdhwr, 'r')
axs[1, 0].plot(time1, tcsd, 'r')
axs[1, 1].plot(time1, thfc, 'r')
axs[1, 2].plot(time1, tl2nh, 'r')
axs[2, 0].plot(time1, tl1nh, 'r')
axs[2, 1].plot(time1, tl2h, 'r')
axs[2, 2].plot(time1, tl1h, 'r')

# PLOT SPECTRAL FLUXES
axs[0, 0].plot(time2, ed, 'b')
axs[0, 1].plot(time2, sd, 'b')
axs[0, 2].plot(time2, sdhwr, 'b')
axs[1, 0].plot(time2, csd, 'b')
axs[1, 1].plot(time2, hfc, 'b')
axs[1, 2].plot(time2, l2nh, 'b')
axs[2, 0].plot(time2, l1nh, 'b')
axs[2, 1].plot(time2, l2h, 'b')
axs[2, 2].plot(time2, l1h, 'b')

# SET TITLES
axs[0, 0].set_title('Energy Difference (ed)')
axs[0, 1].set_title('Spectral Difference (sd)')
axs[0, 2].set_title('Spectral Difference HWR (sdhwr)')
axs[1, 0].set_title('Complex Spectral Difference (csd)')
axs[1, 1].set_title('High Frequency Content (hfc)')
axs[1, 2].set_title('L2 without HWR')
axs[2, 0].set_title('L1 without HWR')
axs[2, 1].set_title('L2 with HWR')
axs[2, 2].set_title('L1 with HWR')

axs[0, 0].set_xlim([start,end])
axs[0, 1].set_xlim([start,end])
axs[0, 2].set_xlim([start,end])
axs[1, 0].set_xlim([start,end])
axs[1, 1].set_xlim([start,end])
axs[1, 2].set_xlim([start,end])
axs[2, 0].set_xlim([start,end])
axs[2, 1].set_xlim([start,end])
axs[2, 2].set_xlim([start,end])

axs[0, 0].autoscale(enable=True, axis='y')
axs[0, 1].autoscale(enable=True, axis='y')
axs[0, 2].autoscale(enable=True, axis='y')
axs[1, 0].autoscale(enable=True, axis='y')
axs[1, 1].autoscale(enable=True, axis='y')
axs[1, 2].autoscale(enable=True, axis='y')
axs[2, 0].autoscale(enable=True, axis='y')
axs[2, 1].autoscale(enable=True, axis='y')
axs[2, 2].autoscale(enable=True, axis='y')

plt.show()