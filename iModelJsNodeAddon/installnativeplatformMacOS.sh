#!/bin/zsh

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

cp $OutRoot/MacOSX64/imodeljsnodeaddon_pkgs/imodeljs-native/* $tempDir/core/backend/node_modules/@bentley/imodeljs-native
cp -r $OutRoot/MacOSX64/imodeljsnodeaddon_pkgs/imodeljs-darwin-x64 $tempDir/core/backend/node_modules/@bentley/imodeljs-native
cp $OutRoot/MacOSX64/imodeljsnodeaddon_pkgs/imodeljs-native/* $tempDir/full-stack-tests/backend/node_modules/@bentley/imodeljs-native
cp -r $OutRoot/MacOSX64/imodeljsnodeaddon_pkgs/imodeljs-darwin-x64 $tempDir/full-stack-tests/backend/node_modules/@bentley/imodeljs-native
