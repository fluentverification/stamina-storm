#!/usr/bin/env python3

import sys

def testToNumber(n):
    hashMap = {}
    for i in range(n):
        hashMap[i] = True

if __name__=='__main__':
    numToTest = int(sys.argv[1])
    testToNumber(numToTest)
