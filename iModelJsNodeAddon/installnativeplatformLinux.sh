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

cp $OutRoot/LinuxX64/packages/imodeljs-native-platform-api/*          $tempDir/common/temp/node_modules/@bentley/imodeljs-native-platform-api
cp -r $OutRoot/LinuxX64/packages/imodeljs-n_10-linux-x64               $tempDir/common/temp/node_modules/@bentley
