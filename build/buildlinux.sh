#!/bin/bash
SrcRoot="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )/../../"
export BMAKE_OPT="-I"$SrcRoot"imodel-native/build/PublicSDK"
export ToolCache="$SrcRoot"imodel-native/build/toolcache/
export NDEBUG=1
python3 "$SrcRoot"imodel-native/build/BentleyBuild/BentleyBuild.py -s  "$SrcRoot"imodel-native/build/strategies/iModelCoreOpen.BuildStrategy.xml+"$SrcRoot"imodel-native/build/strategies/iModelJsNodeAddonOpen.BuildStrategy.xml -a linuxx64 --srcroot="$SrcRoot" --outputroot="$SrcRoot"../out build ${@:1}
