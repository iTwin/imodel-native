#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See COPYRIGHT.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
#!/usr/bin/python

import os
import sys

srcRoot     = os.environ['SrcRoot']
outRoot     = os.environ['OutRoot']
iosarch     = sys.argv[1]
libDir      = sys.argv[2]
fwkDir      = sys.argv[3]

configPaths = [
    srcRoot + "imodel02/iModelJsMobile/nonport/Framework/iModelJsNative/iModelJsNative/Config.xcconfig",
]

print(configPaths)

for configPath in configPaths:
    with open(configPath, 'w') as configFile:
        configFile.write('OutRoot = {0}\n'.format(outRoot))
        configFile.write('BuildArchitecture = {0}\n'.format(iosarch))
        configFile.write('ENABLE_BITCODE = No\n')
        configFile.write('BsiIncludeDir = $(OutRoot)$(BuildArchitecture)/BuildContexts/iModelJsMobile/\n')
        configFile.write('BsiLibDir = {0}\n'.format(libDir))
        configFile.write('CONFIGURATION_BUILD_DIR= {0}\n'.format(fwkDir))

