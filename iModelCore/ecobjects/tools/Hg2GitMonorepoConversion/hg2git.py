#!/usr/bin/python2
"""
Author: Victor Cushman
Date: 01/2018
Description:
    This script is designed to convert mercurial repositories containing Bis
    schemas over to a git mono-repository.
Setup:
    1. install hg-git
        $ sudo apt-get install python-setuptools
        $ sudo easy_install hg-git
    2. enable hg-git extension in your .hgrc under the extensions section
        ```
        # other .hgrc stuff...
        [extensions]
        hgext.bookmarks =
        hgext.convert =
        hggit =
        ```
"""

import os
import subprocess
import shutil
import sys
import tempfile
import time

SUCCESS = 0
FAILURE = 1
MODE_CONVERT = "convert"
MODE_MERGE   = "merge"

class ansiterm:
    DEFAULT       = "\033[0m"
    COLOR_BLACK   = "\033[1;30m"
    COLOR_RED     = "\033[1;31m"
    COLOR_GREEN   = "\033[1;32m"
    COLOR_YELLOW  = "\033[1;33m"
    COLOR_BLUE    = "\033[1;34m"
    COLOR_MAGENTA = "\033[1;35m"
    COLOR_CYAN    = "\033[1;36m"
    COLOR_WHITE   = "\033[1;37m"
    @staticmethod
    def to_color_str(string, color_esc_sequence):
        if sys.stdout.isatty():
            return "{}{}{}".format(color_esc_sequence, string, ansiterm.DEFAULT)
        else:
            return string

COLOR_HG  = ansiterm.COLOR_CYAN
COLOR_GIT = ansiterm.COLOR_BLUE

################################################################################
def print_usage():
    color_hgrepo = ansiterm.to_color_str("<hgrepo>", COLOR_HG)
    color_gitrepo = ansiterm.to_color_str("<gitrepo>", COLOR_GIT)
    color_filemap = ansiterm.to_color_str("<filemap>", ansiterm.COLOR_YELLOW)
    color_gitbranch = ansiterm.to_color_str("<gitbranch>", ansiterm.COLOR_CYAN)
    print "usage:\n\
  <script> {mode_convert} {hgrepo} {gitrepo} {filemap}     Convert mercurial repository {hgrepo} to a branch on {gitrepo}\n\
                                                    using filemap {filemap} as a guide for what files to include.\n\
  <script> {mode_merge}   {hgrepo} {gitrepo} {gitbranch}   Merge branches previously created with {mode_convert} into branch\n\
                                                    {gitbranch} of {gitrepo}.".format(\
                                                                                mode_convert = MODE_CONVERT,\
                                                                                mode_merge = MODE_MERGE,\
                                                                                hgrepo = color_hgrepo,\
                                                                                gitrepo = color_gitrepo,\
                                                                                filemap = color_filemap,\
                                                                                gitbranch = color_gitbranch)

################################################################################
def print_error(errormsg):
    print "{} {}".format(ansiterm.to_color_str("error:", ansiterm.COLOR_RED), errormsg)

################################################################################
def print_faux_cmd(workingdir, cmd):
    print "{}{} {}".format(workingdir, ansiterm.to_color_str("$", ansiterm.COLOR_GREEN), cmd)

################################################################################
def run_cmd(workingdir, cmd):
    proc = subprocess.Popen(cwd=workingdir, args=cmd.split())
    output, error = proc.communicate()
    proc.wait()
    if proc.returncode != SUCCESS:
        print_error("non-SUCCESS status code was returned from subprocess")
    return proc.returncode

################################################################################
def run_convert(hgpath, gitpath, filemappath):
    hgbasename = os.path.basename(hgpath)
    print "CONVERTING {}".format(ansiterm.to_color_str(hgbasename, COLOR_HG))

    # Create temporary directory for pruned repository.
    tmpdir = tempfile.mkdtemp()
    # Map files from old mercurial repository into temporary repository.
    workingdir = os.path.dirname(os.path.realpath(__file__))
    cmd = "hg convert --filemap {} {} {}".format(filemappath, hgpath, tmpdir)
    print_faux_cmd(workingdir, cmd)
    returncode = run_cmd(workingdir, cmd)
    if returncode != SUCCESS:
        shutil.rmtree(tmpdir)
        return returncode
    workingdir = tmpdir
    cmd = "hg update"
    print_faux_cmd(workingdir, cmd)
    returncode = run_cmd(workingdir, cmd)
    if returncode != SUCCESS:
        shutil.rmtree(tmpdir)
        return returncode
    # Bookmark mercurial repo in prep for exporting to git.
    workingdir = tmpdir
    cmd = "hg bookmarks {}".format(hgbasename)
    print_faux_cmd(workingdir, cmd)
    returncode = run_cmd(workingdir, cmd)
    if returncode != SUCCESS:
        shutil.rmtree(tmpdir)
        return returncode
    # Push bookmarked repo as a branch of the git repository.
    # Mercurial may return exit code 1 if there is nothing to be pushed. So
    # instead of checking for an error status, we'll just eat the error if comes
    # up.
    workingdir = tmpdir
    cmd = "hg push {}".format(gitpath)
    print_faux_cmd(workingdir, cmd)
    run_cmd(workingdir, cmd)

    shutil.rmtree(tmpdir)
    return SUCCESS

################################################################################
def run_merge(hgpath, gitpath, gitbranch):
    hgbasename = os.path.basename(hgpath)
    print "MERGING {}".format(ansiterm.to_color_str(hgbasename, COLOR_HG))
    
    # Merge Bis branch into git repo without committing changes.
    cmd = "git merge {} {} --no-commit --no-ff".format(hgbasename, gitbranch)
    workingdir = gitpath
    print_faux_cmd(workingdir, cmd)
    returncode = run_cmd(workingdir, cmd)
    if returncode != SUCCESS:
        return returncode

    # Commit changes, completing the merge.
    cmd = "git add ."
    workingdir = gitpath
    print_faux_cmd(workingdir, cmd)
    returncode = run_cmd(workingdir, cmd)
    if returncode != SUCCESS:
        return returncode
    cmd = "git commit -i . -m merge_{}".format(hgbasename)
    workingdir = gitpath
    print_faux_cmd(workingdir, cmd)
    returncode = run_cmd(workingdir, cmd)
    if returncode != SUCCESS:
        return returncode

    return SUCCESS

################################################################################'
def parse_arguments(args):
    config = {"isready": True, "mode": None, "hgpath": None, "gitpath": None}

    if len(args) == 1:
        config["isready"] = False
        return config
    required_args = 5
    if len(args) != required_args:
        config["isready"] = False
        print_error("script takes exactly {} arguments ({} provided)".format(required_args, len(args)))
        return config

    config["mode"] = args[1]
    if not (config["mode"] == MODE_CONVERT or config["mode"] == MODE_MERGE):
        config["isready"] = False
        print_error("unknown mode '{}'".format(mode))

    config["hgpath"] = os.path.realpath(args[2])
    if not os.path.isdir(config["hgpath"]):
        config["isready"] = False
        print_error("'{}' is not a valid directory".format(config["hgpath"]))

    config["gitpath"] = os.path.realpath(args[3])
    if not os.path.isdir(config["gitpath"]):
        config["isready"] = False
        print_error("'{}' is not a valid directory".format(config["gitpath"]))

    if config["mode"] == MODE_CONVERT:
        config["filemap"] = os.path.realpath(args[4])
        if not os.path.isfile(config["filemap"]):
            config["isready"] = False
            print_error("filemap '{}' is not a valid file".format(config["filemap"]))
    elif config["mode"] == MODE_MERGE:
        config["gitbranch"] = args[4]

    return config

################################################################################
def main():
    config = parse_arguments(sys.argv)
    if not config["isready"]:
        print_usage()
        sys.exit(FAILURE)
    print "{}".format(ansiterm.to_color_str(os.path.basename(__file__), ansiterm.COLOR_YELLOW))
    print "TARGET GIT REPO: {}".format(ansiterm.to_color_str(config["gitpath"], COLOR_GIT))
    print "TARGET MERCURIAL REPO: {}".format(ansiterm.to_color_str(config["hgpath"], COLOR_HG))

    status = SUCCESS
    start_time = time.time()
    if config["mode"] == MODE_CONVERT:
        status = run_convert(config["hgpath"], config["gitpath"], config["filemap"])
    elif config["mode"] == MODE_MERGE:
        status = run_merge(config["hgpath"], config["gitpath"], config["gitbranch"])
    end_time = time.time()
    print "ELAPSED TIME: {}s".format(str(end_time - start_time))

    if status != SUCCESS:
        print_error("non-SUCCESS status code was returned within script")
        sys.exit(status)
    sys.exit(SUCCESS)

################################################################################
if __name__ == '__main__':
    main()
