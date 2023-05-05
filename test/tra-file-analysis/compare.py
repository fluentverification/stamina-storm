#!/usr/bin/env python3

import sys

class Transitions():
	def __init__(self, filename):
		self.numberStates = 0
		self.numberTrans = 0
		self.__readFromFilename(filename)
		self.__transitions = {}

	def __readFromFilename(self, filename):
		maxStIdx = 0
		with open(filename, "r") as f:
			line = f.readline()
				while line != "":
					frm, to, tRate = (None, None, None)
					# Parse the data from the line
					try:
						frm, to, tRate = line.split(',')
						frm = int(frm)
						to = int(to)
						maxStIdx = max(maxStIdx, frm, to)
						tRate = float(tRate)
					except Exception as e:
						print(f"Error: Line is malformed\n\"{e}\"\nGot exception {e}")
					# Use the values for frm, to, to populate the hashmap
					if not frm in self.__transitions:
						self.__transitions[frm] = []
					self.__transitions[frm].append((to, tRate))
					self.numberTrans += 1
					line = f.readline()
		self.numberStates = maxStIdx

	def removeTransition(self, frm, to, tRate):
		if frm in self.__transitions:
			self.__transitions[frm].remove((to, tRate))

	def hasTransition(self, frm, to, tRate):
		return frm in self.__transitions and (to, Rate) in self.__transitions[frm]

	def compare(self, other):
		other = other.copy()
		isEqual = self.numberTrans == other.numberTrans
		for stateIndex, destinationAndRates in self.__transitions.items():
			dest, rate = destinationAndRates
			if not other.hasTransition(stateIndex, dest, rate):
				self.isEqual = False
				print(f"[INFO] Both .tra files do not have index {stateIndex} to {dest} with rate {rate}")
			else:
				other.removeTransition(stateIndex, dest, rate)
		isEqual = isEqual and other.compare(self)
		return isEqual

	def __eq__(self, other):
		return self.compare(other)

if __name__=="__main__":
	if "--help" in sys.argv or "-h" in sys.argv:
		print("Compares two transition files to see if they have identical sets of transitions. May be memory intensive")
		print("Usage:\n\tcompare.py [FILE 1] [FILE 2]")
		sys.exit(0)
	if len(sys.argv) != 3:
		print("Malformed args length!")
		sys.exit(1)
	f1 = sys.argv[1]
	f2 = sys.argv[2]
	t1 = Transitions(f1)
	t2 = Transitions(f2)
	if t1 == t2:
		print("===================================================")
		print("TRANSITION FILES CONTAIN SAME TRANSITIONS")
		print("===================================================")
