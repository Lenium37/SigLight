# !/usr/local/bin/python
import matplotlib.pyplot as plt
import numpy as np
import argparse as ap

parser = ap.ArgumentParser()
parser.add_argument('--mfcc', action="store_true")
parser.add_argument('--chroma', action="store_true")
parser.add_argument('--stft', action="store_true")
parser.add_argument('--rhythm', action="store_true")
parser.add_argument('--kernel', action="store_true")
args = parser.parse_args()

print01 = False #kernel
print02 = False #ssm_mfcc
print03 = False #ssm_chroma
print04 = False #ssm_stft
print05 = False #ssm_rhythm
print06 = True #novelty_combined
print07 = False #novelty_mfcc
print08 = False #novelty_chroma
print09 = False #novelty_stft
print10 = False #novelty_rhythm
print11 = True #all extrema
print12 = True #filtered extrema aka segments

if (args.mfcc):
    print02 = True
    print07 = True
if (args.chroma):
    print03 = True
    print08 = True
if (args.stft):
    print04 = True
    print09 = True
if (args.rhythm):
    print05 = True
    print10 = True
if (args.kernel):
    print01 = True


# set starttime and endtime to show in plot here
if print01:
    plt.figure(1)
    result1 = np.genfromtxt('/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/kernel.csv', delimiter=',');
    plt.imshow(result1, cmap='inferno')
    plt.colorbar()

if print02:
    plt.figure(2)
    result2 = np.genfromtxt('/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/ssm_mfcc.csv', delimiter=',')
    plt.imshow(result2, cmap='inferno')
    plt.colorbar()

if print03:
    plt.figure(3)
    result2 = np.genfromtxt('/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/ssm_chroma.csv', delimiter=',')
    plt.imshow(result2, cmap='inferno')
    plt.colorbar()

if print04:
    plt.figure(4)
    result2 = np.genfromtxt('/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/ssm_stft.csv', delimiter=',')
    plt.imshow(result2, cmap='inferno')
    plt.colorbar()

if print05:
    plt.figure(5)
    result2 = np.genfromtxt('/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/ssm_rhythm.csv', delimiter=',')
    plt.imshow(result2, cmap='inferno')
    plt.colorbar()

plt.figure(6)
if print06:
    time1, value1 = np.loadtxt('/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/novelty_function_combined.csv', delimiter=',', unpack=True, skiprows=0)
    plt.plot(time1, value1, 'r', color='blue') #combined
if print07:
    time2, value2 = np.loadtxt('/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/novelty_function_mfcc.csv', delimiter=',', unpack=True, skiprows=0)
    plt.plot(time2, value2, 'r', color='yellow') #mfcc
if print08:
    time3, value3 = np.loadtxt('/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/novelty_function_chroma.csv', delimiter=',', unpack=True, skiprows=0)
    plt.plot(time3, value3, 'r', color='purple') #chroma
if print09:
    time4, value4 = np.loadtxt('/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/novelty_function_stft.csv', delimiter=',', unpack=True, skiprows=0)
    plt.plot(time4, value4, 'r', color='orange') #stft
if print10:
    time5, value5 = np.loadtxt('/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/novelty_function_rhythm.csv', delimiter=',', unpack=True, skiprows=0)
    plt.plot(time5, value5, 'r', color='cyan') #rhythm
if print11:
    time6, value6 = np.loadtxt('/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/extrema.csv', delimiter=',', unpack=True, skiprows=0)
    plt.plot(time6, value6, 'o', color='green') #extrema
if print12:
    time7, value7 = np.loadtxt('/Users/stevendrewers/CLionProjects/Sound-to-Light-2.0/CSV/segments.csv', delimiter=',', unpack=True, skiprows=0)
    plt.plot(time7, value7, 'o', color='red') #filtered_extrema
plt.show()
