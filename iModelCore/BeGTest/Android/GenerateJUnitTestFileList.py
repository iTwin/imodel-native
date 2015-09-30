import sys, os, re, string
import stat

def convertToJniName (nm):
    return nm.replace ('_', '_1')

def GenJUnitTestFilesForDir (rootDir, dirNameIn, listfilename, javaTemplate, jniTemplate):
    dirName = dirNameIn.replace ('\\', '/') # <outputdir>/build/UnitTests/<reponame>/Published<reponame>UnitTests/
    
    if rootDir[-1] != '/':
        rootDir += '/'

    print '#rootDir='+rootDir

    utPackageName           = dirName[len(rootDir):]
    print '#utPackageName='+utPackageName
    utPackageName = utPackageName.replace('/', '');
    print '#utPackageName='+utPackageName
    unitTestsListHfileName  = os.path.join (dirName, listfilename)
    unitTestsListFileName   = utPackageName + '.list.h'
    unitTestsJniFileName    = utPackageName + 'JniTest.cpp'
    unitTestsJniObjName     = utPackageName + 'JniTest$(oext)'
    unitTestsJniDefines     = ''
    
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
                
            junitTestFileName    = utClassName + '.java'
            junitTestFileDir     = '$(androidTestDir)java/com/bentley/unittest/' + utPackageName + '/'
            junitTestFilePath    = junitTestFileDir + junitTestFileName
            junitTestDefine      = ' -DBE_TEST_CLASS_' + utClassName + ' '
            unitTestsJniDefines += junitTestDefine
            
            print junitTestFilePath + ' : ' + unitTestsListHfileName + " " + javaTemplate
            print '     $(msg)'
            print '     !~@mkdir    ' + junitTestFileDir
            print '     @$(BENTLEY_ANDROID_TOOLCHAIN_preprocess) -I . -I $(outputDir) -I ' + dirName + junitTestDefine + javaTemplate + ' > ' + junitTestFilePath
            print '     @$(baseDir)SearchAndReplace.py  ' + junitTestFilePath + ' __REPO__ ' + utPackageName
            print '     @$(baseDir)SearchAndReplace.py  ' + junitTestFilePath + ' __FIXTURE__ ' + utClassName
            print '     @$(baseDir)SearchAndReplace.py  ' + junitTestFilePath + ' __ANDROIDJUNITTEST_SHARED_LIBRARIES__ "$(__ANDROIDJUNITTEST_SHARED_LIBRARIES__)"'
            print '     ~time'
            print '\n'
            
    print '$(outputDir)' + unitTestsJniFileName + ' $(outputDir)' + unitTestsListFileName + ' : ' + unitTestsListHfileName + " " + jniTemplate
    print '     $(msg)'
    print '     $(copyCmd) '                         + jniTemplate            + ' $(outputDir)' + unitTestsJniFileName
    print '     @$(baseDir)SearchAndReplace.py $(outputDir)' + unitTestsJniFileName   + ' __REPO__ ' + utPackageName
    print '     @$(baseDir)SearchAndReplace.py $(outputDir)' + unitTestsJniFileName   + ' __FIXTURE__ ' + convertToJniName (utPackageName)
    print '     $(copyCmd) '                         + unitTestsListHfileName + ' $(outputDir)' + unitTestsListFileName
    print '     ~time'
    print '\n'

    if len (unitTestsJniDefines) > 0:
        print 'oldCDefs =% $(cDefs)'
        print 'cDefs + ' + unitTestsJniDefines + '\n'
    print '$(outputDir)' + unitTestsJniObjName + ' :  $(outputDir)' + unitTestsJniFileName + ' $(outputDir)' + unitTestsListFileName + '\n'
    if len (unitTestsJniDefines) > 0:
        print 'cDefs = $(oldCDefs)'
    print '\n'

def main():

    if len (sys.argv) != 4:
        print 'Syntax: GenerateJUnitTestFileList.py rootdir pathToJavaCFile pathToJniCppFile'
        return

    dir = sys.argv[1]

    if not dir[-1] in '/\\':
        dir = dir + os.sep

    dir = dir.replace('\\', '/')

    java = sys.argv[2].replace  ('\\', '/')
    jni  = sys.argv[3].replace  ('\\', '/')

    for root,dirs,files in os.walk (dir, topdown=True, onerror=None, followlinks=True):
        if root.find ('.hg') != -1:
            continue
        for file in files:
            if file == 'UnitTests.list.h':
                GenJUnitTestFilesForDir (dir, root, file, java, jni)

if __name__ == '__main__':
    main()
