#!/usr/bin/python3

import unittest

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
			os.system(cmd)
