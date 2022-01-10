#!/usr/bin/python3

import unittest
import os
import sys

'''
This file automatically tests several things from stamina-cplusplus
'''

files = {}

EXECUTABLE_PATH = "../build/stamina-cplusplus"
JAVA_EXECUTABLE_PATH = ""

class TestStaminaCpp(unittest.TestCase):

	def test_exitSuccess(self):
		global files
		for model, prop in files.items():
			cmd = f"{EXECUTABLE_PATH} {model} {prop}"
			exitCode = os.system(cmd)
			if exitCode == 0:
				print(f"[INFO] Successfull exit", file=sys.stderr)
			else:
				print(f"[ERROR] Bad exit!\n\tCode: {exitCode}\n\tModel: {model}\n\tProperty: {prop}", file=sys.stderr)
			self.assertEqual(exitCode, 0)

if __name__=='__main__':
	unittest.main()
