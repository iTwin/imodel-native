#!/usr/bin/python

import os

srcRoot     = os.environ['SrcRoot']
outRoot     = os.environ['OutRoot']
assetRoot   = os.environ['AssetRoot']

iosarch = "arm64"

configPaths = [
    srcRoot + "iModelJs/nonport/iOS/iModelJsApp/iModelJsApp.xcconfig",
]

print(configPaths)

for configPath in configPaths:
    with open(configPath, 'w') as configFile:
        configFile.write('SrcRoot = {0}\n'.format(srcRoot))
        configFile.write('OutRoot = {0}\n'.format(outRoot))
        configFile.write('AssetRoot = {0}\n'.format(assetRoot))
        configFile.write('BuildArchitecture = iOSARM64\n')
        configFile.write('IOSARCH = {0}\n'.format(iosarch))

v       = os.getenv('VersionNumber','10.02.01.999').split('.')
version = "%02d.%02d.%02d" % (int(v[0]),int(v[1]),int(v[2]))
build   = "%02d.%02d.%02d.%02d" % (int(v[0]),int(v[1]),int(v[2]),int(v[3]))

with open(configPaths[0], 'a') as appConfigFile:
    appConfigFile.write('Version = {0}\n'.format(version))
    appConfigFile.write('Build = {0}\n'.format(build))


