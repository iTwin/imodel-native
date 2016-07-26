import sys,re

def getAssetsXml (assetsFilePath):
    file = open (assetsFilePath, 'r')
    lines = file.readlines ()
    file.close ()
    
    xml = ''
    for line in lines:
        assets = line.split ()
        for asset in assets:
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
