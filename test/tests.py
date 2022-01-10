#!/usr/bin/python3

#import unittest
import os
import sys

'''
This file automatically tests several things from stamina-cplusplus
'''

files = {}

EXECUTABLE_PATH = "../build/stamina-cplusplus"
JAVA_EXECUTABLE_PATH = ""

def getFiles(argv1):
	global files
	with open(argv1) as csvFile:
		for ln in csvFile:
			model, prop = ln.split(',')
			print(f"Got model {model} and property {prop}", file=sys.stderr)
			files[model] = prop

class TestStaminaCpp: #(unittest.TestCase):
	def test_exitSuccess(self):
		print("Unit Test: Exit Success")
		for model, prop in files.items():
			cmd = f"{EXECUTABLE_PATH} {model} {prop}"
			print(f"Running command:\n\t{cmd}")
			exitCode = os.system(cmd)
			if exitCode == 0:
				print(f"[TEST::INFO] Successfull exit", file=sys.stderr)
			else:
				print(f"[TEST::ERROR] Bad exit!\n\tCode: {exitCode}\n\tModel: {model}\n\tProperty: {prop}", file=sys.stderr)
			#self.assertEqual(exitCode, 0)

if __name__=='__main__':
	getFiles(sys.argv[1])
	print("Starting unit tests")
	#unittest.main()
	a = TestStaminaCpp()
	a.test_exitSuccess()
