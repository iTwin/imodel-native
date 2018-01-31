#!/bin/bash
set -e

ANSI_ESC="\033["
ANSI_ESC_DEFAULT="${ANSI_ESC}0m"
ANSI_ESC_BOLD="${ANSI_ESC}1m"
ANSI_ESC_UNDERLINE="${ANSI_ESC}4m"
ANSI_ESC_FG_BLACK="${ANSI_ESC}1;30m"
ANSI_ESC_FG_RED="${ANSI_ESC}1;31m"
ANSI_ESC_FG_GREEN="${ANSI_ESC}1;32m"
ANSI_ESC_FG_YELLOW="${ANSI_ESC}1;33m"
ANSI_ESC_FG_BLUE="${ANSI_ESC}1;34m"
ANSI_ESC_FG_MAGENTA="${ANSI_ESC}1;35m"
ANSI_ESC_FG_CYAN="${ANSI_ESC}1;36m"
ANSI_ESC_BG_BLACK="${ANSI_ESC}1;40m"
ANSI_ESC_BG_RED="${ANSI_ESC}1;41m"
ANSI_ESC_BG_GREEN="${ANSI_ESC}1;42m"
ANSI_ESC_BG_YELLOW="${ANSI_ESC}1;43m"
ANSI_ESC_BG_BLUE="${ANSI_ESC}1;44m"
ANSI_ESC_BG_MAGENTA="${ANSI_ESC}1;45m"
ANSI_ESC_BG_CYAN="${ANSI_ESC}1;46m"

HEADER1="${ANSI_ESC_FG_YELLOW}${ANSI_ESC_BOLD}${ANSI_ESC_UNDERLINE}"
HEADER2="${ANSI_ESC_FG_MAGENTA}${ANSI_ESC_UNDERLINE}"

# function cecho($1=color, $2=string) -> void 
function cecho {
    if [ -t 1 ]; then # test isatty
        echo -e "${1}${2}${ANSI_ESC_DEFAULT}"
    else
        echo "${2}"
    fi
}

# function print_error($1=errorstr) -> void
# Echos a message in the form "error: ${errorstr}"
function print_error {
    echo -e "${ANSI_ESC_FG_RED}error: ${ANSI_ESC_DEFAULT}${1}"
}

# function sleep_commit_git(void) -> void
# Pause for a brief period of time after making a commit in git. Commits within
# the same second can become messed up in git history. This pause buffer is used
# to avoid the issue.
function sleep_commit_git {
    sleep 1
}

# function sleep_commit_hg(void) -> void
# Pause for a brief period of time after making a commit in hg. Commits within
# the same second can become messed up in hg history. This pause buffer is used
# to avoid the issue.
function sleep_commit_hg {
    sleep 1
}

# function print_usage(void) -> void
function print_usage {
    cat << END
USAGE: <script> [--setup | --log | --convert | --merge]
OPTIONS:
    --setup     Setup test repos only.
    --log       Display repository logs (implimentation defined).
    --convert   Run script to convert test mercurial repos to git branches.
    --merge     Merge all mercurial git branches into the git master branch.
END
}

# Parse command line parameters.
SETTING_DO_SETUP=0
SETTING_DO_LOG=0
SETTING_DO_CONVERT=0
SETTING_DO_MERGE=0
if [ $# -eq 0 ]; then
    ./${0} --setup
    ./${0} --log
    ./${0} --convert
    ./${0} --merge
    ./${0} --log
    exit 0
elif [ $# -eq 1 ]; then
    if [ "$1" == "--setup" ]; then
        SETTING_DO_SETUP=1
    elif [ "$1" == "--log" ]; then
        SETTING_DO_LOG=1
    elif [ "$1" == "--convert" ]; then
        SETTING_DO_CONVERT=1
    elif [ "$1" == "--merge" ]; then
        SETTING_DO_MERGE=1
    else
        print_usage
        exit 1
    fi
else
    print_error "script takes 0 or 1 arguments"
    print_usage
    exit 1
fi

# Overwrite these in the implimentation files.
do_setup=_do_setup
do_log=_do_log
do_convert=_do_convert
do_merge=_do_merge
function _do_setup {
    print_error "do_setup not overwritten"
}
function _do_log {
    print_error "do_log not overwritten"
}
function _do_convert {
    print_error "do_covert not overwritten"
}
function _do_merge {
    print_error "do_merge not overwritten"
}

# function run(void) -> void
# Call this function in the implimentation script.
function run {
    if [ $SETTING_DO_SETUP -eq 1 ]; then
        cecho ${HEADER1} "SETUP REPOS                                                                     "
        $do_setup
    fi

    if [ $SETTING_DO_LOG -eq 1 ]; then
        cecho ${HEADER1} "REPOSITORY LOGS                                                                 "
        $do_log
    fi

    if [ $SETTING_DO_CONVERT -eq 1 ]; then
        cecho ${HEADER1} "PERFORM CONVERSION                                                              "
        $do_convert
    fi

    if [ $SETTING_DO_MERGE -eq 1 ]; then
        cecho ${HEADER1} "PERFORM MERGE                                                                   "
        $do_merge
    fi
}
