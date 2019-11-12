#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See COPYRIGHT.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import os
import json
import unittest
from GitCommands import GitCommand

class GitCommandTests(unittest.TestCase):
    def setUp(self):
        os.chdir(os.path.dirname(os.path.realpath(__file__)))

    def test_execute(self):
        git = GitCommand()
        res = git.execute('git status')
        self.assertTrue(res)
        self.assertGreater(len(git.res), 0)

    def test_switch_branch(self):
        git = GitCommand()
        res = git.switch_branch('pushmaps')
        self.assertTrue(res)
        res = git.switch_branch('master')
        self.assertTrue(res)        
    def test_stats(self):
        git = GitCommand()
        res = git.status()
        self.assertTrue(res)
        self.assertIsNotNone(git.stats)

if __name__ == '__main__':
    unittest.main()