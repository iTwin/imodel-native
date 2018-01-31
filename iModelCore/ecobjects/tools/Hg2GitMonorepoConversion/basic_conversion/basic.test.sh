#!/bin/bash
set -e

DIR_ROOT=$(pwd)
DIR_BASIC_CONVERSION=$DIR_ROOT/basic_conversion
DIR_TEST=$DIR_ROOT/test
DIR_GIT=$DIR_TEST/git
DIR_HG1=$DIR_TEST/hg/hg1
DIR_HG2=$DIR_TEST/hg/hg2

source ${DIR_BASIC_CONVERSION}/basic.interface.sh

do_setup=do_setup_impl
function do_setup_impl {
    ## Clean.
    rm -rf $DIR_TEST
    mkdir -p $DIR_GIT
    mkdir -p $DIR_HG1
    mkdir -p $DIR_HG2

    ## Setup git repo.
    cecho ${HEADER2} "SETTING UP GIT REPO"
    cd $DIR_GIT
    git init
    # Dummy file added in order to create initial commit.
    echo "This git repo is #monolit." > README.txt
    git add .
    git commit -m "init commit"
    sleep_commit_git

    ## Setup mercurial repos.
    # The mercurial repos will have the following history:
    # (1) Add file A to hg1.
    # (2) Add file B to hg2.
    # (3) Update file A in hg1.
    # (4) Update file B in hg2.
    # (5) Add file C in hg1.
    # (6) Update file C in hg1.
    # (7) Add two files that should be ignored in hg1.
    cecho ${HEADER2} "SETTING UP MERCURIAL REPOS"
    cd $DIR_HG1
    hg init
    echo -e "${ANSI_ESC_FG_GREEN}CREATE FILE \"A\" IN REPO hg1${ANSI_ESC_DEFAULT}"
    echo "file A" > A
    hg addremove > /dev/null
    hg commit -m "create file A"
    sleep_commit_hg

    cd $DIR_HG2
    hg init
    echo -e "${ANSI_ESC_FG_GREEN}CREATE FILE \"B\" IN REPO hg1${ANSI_ESC_DEFAULT}"
    echo "file B" > B
    hg addremove > /dev/null
    hg commit -m "create file B"
    sleep_commit_hg

    cd $DIR_HG1
    echo -e "${ANSI_ESC_FG_GREEN}UPDATE FILE \"A\" IN REPO hg2${ANSI_ESC_DEFAULT}"
    echo "updated file A" > A
    hg commit -m "update file A"
    sleep_commit_hg

    cd $DIR_HG2
    echo -e "${ANSI_ESC_FG_GREEN}UPDATE FILE \"B\" IN REPO hg2${ANSI_ESC_DEFAULT}"
    echo "updated file B" > B
    hg commit -m "update file B"
    sleep_commit_hg

    cd $DIR_HG1
    echo -e "${ANSI_ESC_FG_GREEN}CREATE FILE \"C\" IN REPO hg1${ANSI_ESC_DEFAULT}"
    echo "file C" > C
    hg addremove > /dev/null
    hg commit -m "create file C"
    sleep_commit_hg

    cd $DIR_HG1
    echo -e "${ANSI_ESC_FG_GREEN}UPDATE FILE \"C\" IN REPO hg1${ANSI_ESC_DEFAULT}"
    echo "updated file C" > C
    hg commit -m "update file C"
    sleep_commit_hg

    cd $DIR_HG1
    echo -e "${ANSI_ESC_FG_GREEN}ADD FILES TO BE IGNORED IN REPO hg1${ANSI_ESC_DEFAULT}"
    echo "ignore me!" > IGNOREME1
    echo "ignore me!" > IGNOREME2
    hg addremove > /dev/null
    hg commit -m "add files to be ignored"
    sleep_commit_hg

    cecho ${HEADER2} "TEST DIRECTORY STRUCTURE AFTER SETUP"
    tree $DIR_TEST -n
}

do_log=do_log_impl
function do_log_impl {
    cecho ${HEADER2} "git LOG"
    cd $DIR_GIT
    git log

    cecho ${HEADER2} "hg1 LOG"
    cd $DIR_HG1
    hg log

    cecho ${HEADER2} "hg2 LOG"
    cd $DIR_HG2
    hg log
}

do_convert=do_convert_impl
function do_convert_impl {
    ## Conversion.
    ${DIR_ROOT}/hg2git.py convert $DIR_HG1 $DIR_GIT ${DIR_BASIC_CONVERSION}/hg1.basic.test.filemap
    ${DIR_ROOT}/hg2git.py convert $DIR_HG2 $DIR_GIT ${DIR_BASIC_CONVERSION}/hg2.basic.test.filemap

    cecho ${HEADER2} "GIT BRANCHES AFTER CONVERT"
    cd $DIR_GIT
    git branch
}

do_merge=do_merge_impl
function do_merge_impl {
    ${DIR_ROOT}/hg2git.py merge $DIR_HG1 $DIR_GIT master
    ${DIR_ROOT}/hg2git.py merge $DIR_HG2 $DIR_GIT master

    cd $DIR_ROOT
    cecho ${HEADER2} "TEST DIRECTORY STRUCTURE AFTER MERGE"
    tree $DIR_TEST -n
}

run
exit 0
