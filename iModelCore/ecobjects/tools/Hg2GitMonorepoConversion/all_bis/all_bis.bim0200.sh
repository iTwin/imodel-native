#!/bin/bash
# If running on *nix, you may need to add root as a trusted group in your .hgrc
#```
#[trusted]
#users = root
#```
set -e
start=$SECONDS

# OVERIDE
DIR_BIM0200="${DIR_BIM0200:-/mnt/c/dev/Bim0200Dev}"
DIR_GIT="${DIR_GIT:-/mnt/c/dev/Bim0200dev_bisMerge/bis-schemas}"
# !OVERIDE

DIR_ROOT=$(pwd)
DIR_BIM0200_SRC="${DIR_BIM0200}/src"
DIR_ALL_BIS="${DIR_ROOT}/all_bis"
DIR_ALL_BIS_FILEMAPS="${DIR_ALL_BIS}/filemaps"
DIRS_HG=(
    "${DIR_BIM0200_SRC}/DgnDomains/AecUnits"
    "${DIR_BIM0200_SRC}/DgnDomains/BridgePhysical"
    "${DIR_BIM0200_SRC}/DgnDomains/Building"
    "${DIR_BIM0200_SRC}/DgnDomains/BuildingSpacePlanning"
    "${DIR_BIM0200_SRC}/DgnDomains/ConstraintSystem"
    "${DIR_BIM0200_SRC}/DgnDomains/Costing"
    "${DIR_BIM0200_SRC}/DgnDomains/Electrical"
    "${DIR_BIM0200_SRC}/DgnDomains/Forms"
    "${DIR_BIM0200_SRC}/DgnDomains/Grids"
    "${DIR_BIM0200_SRC}/DgnDomains/LinearReferencing"
    "${DIR_BIM0200_SRC}/DgnDomains/Planning"
    "${DIR_BIM0200_SRC}/DgnDomains/Plant"
    "${DIR_BIM0200_SRC}/DgnDomains/Profiles"
    "${DIR_BIM0200_SRC}/DgnDomains/RealityModeling"
    "${DIR_BIM0200_SRC}/DgnDomains/RoadRailAlignment"
    "${DIR_BIM0200_SRC}/DgnDomains/RoadRailPhysical"
    "${DIR_BIM0200_SRC}/DgnDomains/Site"
    "${DIR_BIM0200_SRC}/DgnDomains/Structural"

    "${DIR_BIM0200_SRC}/DgnPlatform"

    # TODO: iModelSchemaEditor is a git repo, so the hg2git.py script obviously
    # won't work on it.
    # "${DIR_BIM0200_SRC}/iModelSchemaEditor"
)
MAPS_HG=(
    "${DIR_ALL_BIS_FILEMAPS}/DgnDomains/AecUnits.filemap"
    "${DIR_ALL_BIS_FILEMAPS}/DgnDomains/BridgePhysical.filemap"
    "${DIR_ALL_BIS_FILEMAPS}/DgnDomains/Building.filemap"
    "${DIR_ALL_BIS_FILEMAPS}/DgnDomains/BuildingSpacePlanning.filemap"
    "${DIR_ALL_BIS_FILEMAPS}/DgnDomains/ConstraintSystem.filemap"
    "${DIR_ALL_BIS_FILEMAPS}/DgnDomains/Costing.filemap"
    "${DIR_ALL_BIS_FILEMAPS}/DgnDomains/Electrical.filemap"
    "${DIR_ALL_BIS_FILEMAPS}/DgnDomains/Forms.filemap"
    "${DIR_ALL_BIS_FILEMAPS}/DgnDomains/Grids.filemap"
    "${DIR_ALL_BIS_FILEMAPS}/DgnDomains/LinearReferencing.filemap"
    "${DIR_ALL_BIS_FILEMAPS}/DgnDomains/Planning.filemap"
    "${DIR_ALL_BIS_FILEMAPS}/DgnDomains/Plant.filemap"
    "${DIR_ALL_BIS_FILEMAPS}/DgnDomains/Profiles.filemap"
    "${DIR_ALL_BIS_FILEMAPS}/DgnDomains/RealityModeling.filemap"
    "${DIR_ALL_BIS_FILEMAPS}/DgnDomains/RoadRailAlignment.filemap"
    "${DIR_ALL_BIS_FILEMAPS}/DgnDomains/RoadRailPhysical.filemap"
    "${DIR_ALL_BIS_FILEMAPS}/DgnDomains/Site.filemap"
    "${DIR_ALL_BIS_FILEMAPS}/DgnDomains/Structural.filemap"

    "${DIR_ALL_BIS_FILEMAPS}/DgnPlatform.filemap"

    # TODO: iModelSchemaEditor is a git repo, so the hg2git.py script obviously
    # won't work on it.
    # "${DIR_ALL_BIS_FILEMAPS}/iModelSchemaEditor.filemap"
)

ANSI_ESC="\033["
ANSI_ESC_DEFAULT="${ANSI_ESC}0m"
ANSI_ESC_BOLD="${ANSI_ESC}1m"
ANSI_ESC_UNDERLINE="${ANSI_ESC}4m"
ANSI_ESC_FG_RED="${ANSI_ESC}1;31m"
ANSI_ESC_FG_YELLOW="${ANSI_ESC}1;33m"
ANSI_ESC_FG_MAGENTA="${ANSI_ESC}1;35m"
ANSI_ESC_FG_CYAN="${ANSI_ESC}1;36m"

HEADER1="${ANSI_ESC_FG_YELLOW}${ANSI_ESC_BOLD}${ANSI_ESC_UNDERLINE}"
HEADER2="${ANSI_ESC_FG_MAGENTA}${ANSI_ESC_UNDERLINE}"

function cecho {
    if [ -t 1 ]; then # test isatty
        echo -e "${1}${2}${ANSI_ESC_DEFAULT}"
    else
        echo "${2}"
    fi
}

function advanced_engineering_strategy_to_prevent_random_failures {
    sleep 1
}

################################################################################

cecho $ANSI_ESC_FG_CYAN "GIT REPO:"
echo -e ">> $DIR_GIT"
cecho $ANSI_ESC_FG_CYAN "HG REPOS:"
for r in ${DIRS_HG}; do
    echo -e ">> ${r}"
done


## Setup example git repo.
## Uncomment this section for testing. Be sure to override DIR_GIT.
#rm -rf $DIR_GIT
#mkdir -p $DIR_GIT
#cecho ${HEADER1} "SETTING UP GIT REPO"
#cd $DIR_GIT
#git init
#echo "This git repo is #monolit." > README.txt
#git add .
#git commit -m "init commit"
#cecho ${HEADER2} "GIT REPO AFTER SETUP"
#tree $DIR_GIT -n

## Create new branch
GIT_BRANCH="bim0200dev_merge_$(date +%Y-%m-%d)"
cd $DIR_GIT
git checkout -b $GIT_BRANCH

## Conversion/Merge.
i=0
while [ $i -lt ${#DIRS_HG[@]} ]; do
    ${DIR_ROOT}/hg2git.py convert ${DIRS_HG[$i]} $DIR_GIT ${MAPS_HG[$i]} || {
        cecho ${ANSI_ESC_FG_RED} "FAILURE DURING CONVERT"
        exit 1
    }
    echo "================================================================"
    advanced_engineering_strategy_to_prevent_random_failures

    ${DIR_ROOT}/hg2git.py merge ${DIRS_HG[$i]} $DIR_GIT $GIT_BRANCH || {
        cecho ${ANSI_ESC_FG_RED} "FAILURE DURING MERGE"
        exit 1
    }
    echo "================================================================"
    advanced_engineering_strategy_to_prevent_random_failures

    i=$(expr ${i} + 1)
done

duration=$(( $SECONDS - start ))
echo "TOTAL ELAPSED TIME: ${duration} seconds"
exit 0
