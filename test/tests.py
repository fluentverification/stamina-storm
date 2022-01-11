#!/usr/bin/python3

#import unittest
import os
import sys

'''
This file automatically tests several things from stamina-cplusplus
'''

HEADER = '\033[95m'
OKBLUE = '\033[94m'
OKCYAN = '\033[96m'
OKGREEN = '\033[92m'
WARNING = '\033[93m'
FAIL = '\033[91m'
ENDC = '\033[0m'
BOLD = '\033[1m'
UNDERLINE = '\033[4m'


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
		passes = 0
		fails = 0
		for model, prop in files.items():
			model = model.replace(' ', '\\ ')
			prop = prop.replace(' ', '\\ ')
			cmd = f"{EXECUTABLE_PATH} {model} {prop}"
			print(f"Running command:\n\t{cmd}")
			exitCode = os.system(cmd)
			if exitCode == 0:
				print(f"{BOLD}{OKBLUE}{UNDERLINE}[TEST::INFO]{ENDC} Successfull exit", file=sys.stderr)
				passes += 1
			else:
				print(f"{BOLD}{FAIL}{UNDERLINE}[TEST::ERROR]{ENDC} Bad exit!\n\tCode: {exitCode}\n\tModel: {model}\n\tProperty: {prop}", file=sys.stderr)
				fails += 1
			#self.assertEqual(exitCode, 0)
		print(f"{BOLD}{UNDERLINE}{OKGREEN}[TEST::PASSED TESTS]{ENDC} Total number of passed tests {passes}")
		print(f"{BOLD}{UNDERLINE}{FAIL}[TEST::FAILED TESTS]{ENDC} Total number of failed tests {fails}")

if __name__=='__main__':
	if len(sys.argv) < 2:
		print("Requires path to list of module and properties files (in CSV format)!", file=sys.stderr)
	getFiles(sys.argv[1])
	print("Starting unit tests")
	#unittest.main()
	a = TestStaminaCpp()
	a.test_exitSuccess()
