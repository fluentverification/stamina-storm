#!/usr/bin/env python3

import sys

def testTraFile(fl):
    line = "hey it's an easter egg"
    trs = {}
    duplicates = 0
    while line != "":
        line = fl.readline()
        if line == "\n":
            continue
        try:
            frm, to, trRate = line.split(' ')
        except Exception:
            print(f"[ERROR] Ill formatted line: {line}")
            continue
        key = f"{frm}{to}"
        if key in trs:
            print(f"[INFO] Duplicate transition found!\n\tFrom: {frm}, To {to}, Rate1 (Ours): {trRate}, Rate2: {trs[key]}")
            duplicates += 1
        else:
            trs[key] = trRate
    if duplicates == 0:
        print("No duplicates found!")
    else:
        print(f"Found {duplicates} duplicates")

if __name__=="__main__":
    if len(sys.argv) == 1:
        print("ERROR! Requires at least one input file")
        exit(1)
    for f in sys.argv[1:]:
        print(f"TESTING FILE: {f}")
        print("=======================================")
        with open(f, 'r') as fl:
            testTraFile(fl)
        print("=======================================")
