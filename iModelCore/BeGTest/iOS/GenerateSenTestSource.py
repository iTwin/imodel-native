import sys, os, re, string
import stat

def processDir (rootDir, dirNameIn, listfilename, senTestTemplate, XCodeProject_BeTestiOSName):
    dirName = dirNameIn.replace ('\\', '/')
    
    if rootDir[-1] != '/':
        rootDir += '/'

    repoUT                  = dirName[len(rootDir):]
    repoUT = repoUT.replace('/', '');
    testFilePath            = '$('+XCodeProject_BeTestiOSName+')BeTestiOSTests/UnitTests.mm'
    tmpTestFileDir          = '$(o)tmp/' + repoUT + '/'
    unitTestsListHfileName  = os.path.join (dirName, listfilename)
    
    tcFilePath = os.path.join (dirName, 'UnitTests.classes.txt')
    if os.path.exists (tcFilePath):
        tcFile = open (tcFilePath, 'r')
        for utClassName in tcFile.readlines ():         # for each unit test class (e.g., WStringTest)
            # remove '\n'
            if len (utClassName) > 0:
                utClassName = utClassName[:-1]
            # skip empty lines
            if utClassName == '':
                continue
            
            tmpTestFileName = utClassName + '.cpp'
            tmpTestFilePath = tmpTestFileDir + tmpTestFileName
            testClassDefine = ' -DBE_TEST_CLASS_' + utClassName + ' '
                                                        # Use the specified template to create an Objective-C wrapper.
                                                        # The template is a .mm-like file.
                                                        # The template file begins by including <UnitTests.list.h>
                                                        # The UnitTests.list.h file for this directory contains a list of macros corresponding to the unit tests in this suite.
                                                        # Each macro takes the name of an individual unit test as an argument.
                                                        # The template #defines those macros to generate Objective-C methods that call the tests.
                                                        # We use the C preprocessor to process the include file and apply the macro definitions.
                                                        # The template file's Objective-C code looks like a class whose name is '__FIXTURE__'
                                                        # We replace '__FIXTURE__' with the test class name
            print tmpTestFilePath + ' : ' + unitTestsListHfileName + ' ' + senTestTemplate
            print '     $(msg)'
            print '     !~@mkdir ' + tmpTestFileDir
            print '     @$(CPreprocCmd) -I ' + dirName + testClassDefine + ' ' + senTestTemplate + ' > ' + tmpTestFilePath
            print '     @$(baseDir)SearchAndReplace.py ' + tmpTestFilePath + ' __FIXTURE__ ' + repoUT + '_' + utClassName
            print '     ~time'
            print ''
            print 'always:'
            print '     @cat ' + tmpTestFilePath + ' >> ' + testFilePath
            print ''

def main():

    if len (sys.argv) != 4:
        print 'Syntax: GenerateSenTestSource.py rootdir pathToDotMTemplateFile XCodeProject_BeTestiOSName'
        return

    dir = sys.argv[1]
    XCodeProject_BeTestiOSName = sys.argv[3]

    if not dir[-1] in '/\\':
        dir = dir + os.sep

    dir = dir.replace('\\', '/')

    dotmm = sys.argv[2].replace  ('\\', '/')

    for root,dirs,files in os.walk (dir, topdown=True, onerror=None, followlinks=True):
        if root.find ('.hg') != -1:
            continue
        for file in files:
            if file == 'UnitTests.list.h':
                processDir (dir, root, file, dotmm, XCodeProject_BeTestiOSName)

if __name__ == '__main__':
    main()
