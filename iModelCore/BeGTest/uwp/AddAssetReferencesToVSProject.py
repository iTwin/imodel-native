#--------------------------------------------------------------------------------------
#
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#
#--------------------------------------------------------------------------------------
import sys,re,shlex

def getAssetsXml (assetsFilePath):
    file = open (assetsFilePath, 'r')
    lines = file.readlines ()
    file.close ()
    
    xml = ''
    for line in lines:
        assets = shlex.split(line, posix=False)
        for asset in assets:
            asset = asset.strip('"')
            xml = xml + '<None Include="' + asset + '"><DeploymentContent>true</DeploymentContent></None>\n'
    return xml

if __name__ == '__main__':
    project = sys.argv[1]
    toMatch  = '__ASSETS__'
    replacement = getAssetsXml (sys.argv[2])

    oFile = open (project, 'r')
    lines = oFile.readlines ()
    oFile.close ()

    oFile = open (project, 'w')
    for line in lines:
        oFile.write (line.replace (toMatch, replacement))
    oFile.close ()
