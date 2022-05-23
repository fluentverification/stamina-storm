#!/usr/bin/env python3

import os
import sys
import time

import matplotlib.pyplot as plt

def timeCommand(command):
    start = time.time()
    os.system(command)
    end = time.time()
    return end - start

def mainTest():
    try:
        numToInsert = 2
        while True:
            print(f"Testing insertion with size {numToInsert}")
            cppTime = timeCommand(f"./a.out {numToInsert}")
            javaTime = timeCommand(f"java Test {numToInsert}")
            pythonTime = timeCommand(f"./hashMap.py {numToInsert}")
            with open("times.txt", 'a') as f:
                f.write(f"{numToInsert},{cppTime},{javaTime},{pythonTime}\n")
            numToInsert *= 2
    except KeyboardInterrupt:
        print("Exiting")
        sys.exit(0)

def createGraph():
    cppTimes = []
    javaTimes = []
    pyTimes = []
    nums = []
    with open('times.txt', 'r') as f:
        a = f.readlines()
        for line in a:
            n, cpp, java, py = line.split(',')
            nums.append(int(n))
            cppTimes.append(float(cpp))
            javaTimes.append(float(java))
            pyTimes.append(float(py))
    plt.plot(nums, cppTimes, label="C++ std::unordered_set Times")
    plt.plot(nums, javaTimes, label="Java HashSet Times")
    plt.plot(nums, pyTimes, label="Python 'set' (built-in) Times")
    plt.legend()
    plt.semilogx()
    plt.xlabel("Number of states to insert")
    plt.ylabel("Time taken (s)")
    plt.show()

if __name__=='__main__':
    if "graph" in sys.argv:
        createGraph()
    else:
        print("[NOTE]: This program will run infinitely if left unchecked. Please use ^C to exit when you have enough data.")
        mainTest()
