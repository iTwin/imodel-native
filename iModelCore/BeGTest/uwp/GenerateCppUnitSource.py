import sys, os, re, string
import stat

def processDir (dirNameIn, rootDir, listfilename, templateFile):
    dirName = dirNameIn.replace ('\\', '/') # <outputdir>/build/UnitTests/<reponame>/Published<reponame>UnitTests/

    if rootDir[-1] != '/':
        rootDir += '/'

    repoUT                  = dirName[len(rootDir):]
    repoUT = repoUT.replace('/', '');
    testFileDir             = '$(o)Project/source/'
    testFileName            = repoUT + '.cpp'
    testFilePath            = testFileDir + testFileName
    tmpTestFileDir          = '$(o)tmp/' + repoUT + '/'
    unitTestsListHfileName  = os.path.join (dirName, listfilename)
    
    print testFilePath + ' : ' + unitTestsListHfileName + ' ' + templateFile
    print '     $(msg)'
    print '     !~@mkdir ' + testFileDir
    print '     $(SrcRoot)bsicommon/sharedmki/Cat.py ' + templateFile + '.prepend > $@'
    print '     ~time'
    print ''
    
    tcFilePath = os.path.join (dirName, 'UnitTests.classes.txt')
    if os.path.exists (tcFilePath):
        tcFile = open (tcFilePath, 'r')
        for utClassName in tcFile.readlines ():
            # remove '\n'
            if len (utClassName) > 0:
                utClassName = utClassName[:-1]
            # skip empty lines
            if utClassName == '':
                continue
            
            tmpTestFileName = utClassName + '.cpp'
            tmpTestFilePath = tmpTestFileDir + tmpTestFileName
            testClassDefine = ' -DBE_TEST_CLASS_' + utClassName + ' '
            
            print tmpTestFilePath + ' : ' + unitTestsListHfileName + ' ' + templateFile
            print '     $(msg)'
            print '     !~@mkdir ' + tmpTestFileDir
            print '     $(CPreprocCmd) -I ' + dirName + testClassDefine + templateFile + ' > $@'
            print '     $(baseDir)SearchAndReplace.py $@ __REPO__ ' + repoUT
            print '     $(baseDir)SearchAndReplace.py $@ __FIXTURE__ ' + utClassName
            print '     $(SrcRoot)bsicommon/sharedmki/Cat.py $@ >> ' + testFilePath
            print '     ~time'
            print ''

def main():

    if len (sys.argv) != 3:
        print 'Syntax: GenerateSenTestSource.py rootdir pathToTemplateFile'
        return

    dir = sys.argv[1]

    if not dir[-1] in '/\\':
        dir = dir + os.sep

    dir = dir.replace('\\', '/')

    templateFile = sys.argv[2].replace  ('\\', '/')

    for root,dirs,files in os.walk (dir, topdown=True, onerror=None, followlinks=True):
        if root.find ('.hg') != -1:
            continue
        for file in files:
            if file == 'UnitTests.list.h':
                processDir (root, dir, file, templateFile)

if __name__ == '__main__':
    main()
