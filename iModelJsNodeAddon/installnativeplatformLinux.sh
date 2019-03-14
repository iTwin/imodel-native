#!/bin/bash

function invalidiModelJsDir {
  echo Set the environment variable imodeljsDir, or pass an argument, that points to an imodeljs directory.
  exit
}

if [[ -z "${OutRoot}" ]]; then
  echo The OutRoot environment variable is not set.
  exit
fi

if [[ "" == $1 ]]; then
  if [[ -z "${imodeljsDir}" ]]; then invalidiModelJsDir; fi

  export tempDir=$imodeljsDir
else
  export tempDir=$1
fi

if [ ! -d "$tempDir" ]; then invalidiModelJsDir; fi

cp $OutRoot/LinuxX64/imodeljsnodeaddon_pkgs/imodeljs-native/* $tempDir/core/backend/node_modules/@bentley/imodeljs-native
cp -r $OutRoot/LinuxX64/imodeljsnodeaddon_pkgs/imodeljs-linux-x64 $tempDir/core/backend/node_modules/@bentley/imodeljs-native
