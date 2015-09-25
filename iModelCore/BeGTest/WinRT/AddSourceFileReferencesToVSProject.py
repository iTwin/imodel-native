import sys,re

def getCppsXml (sourceFilesFilePath):
    file = open (sourceFilesFilePath, 'r')
    lines = file.readlines ()
    file.close ()
    
    xml = ''
    for line in lines:
        sourceFiles = line.split ()
        for sourceFile in sourceFiles:
            xml = xml + '<ClCompile Include="' + sourceFile + '" />\n'
    return xml

if __name__ == '__main__':
    project = sys.argv[1]
    toMatch  = '__GENERATED_SOURCE_FILES__'
    replacement = getCppsXml (sys.argv[2])

    oFile = open (project, 'r')
    lines = oFile.readlines ()
    oFile.close ()

    oFile = open (project, 'w')
    for line in lines:
        oFile.write (line.replace (toMatch, replacement))
    oFile.close ()
