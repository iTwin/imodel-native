#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import sys, os, os.path, stat, time
from os.path import join
from datetime import date, timedelta
import re

# Try to add the filename to the list of 
if 0:
    try:
        self.map[root].append (filename)
    except:
        self.map[root] = [filename]
baseDir = ''
def GetRootLocation ():
    global baseDir
    return baseDir  

#-----------------------------------------------------------------------------------------------
# @bsimethod
#---------------+---------------+---------------+---------------+---------------+---------------
class Tree: 
    #-----------------------------------------------------------------------------------------------
    # @bsimethod
    #---------------+---------------+---------------+---------------+---------------+---------------
    def __init__ (self, name):
        self.children = []
        self.name = name

    #-----------------------------------------------------------------------------------------------
    # @bsimethod
    #---------------+---------------+---------------+---------------+---------------+---------------
    def Add (self, name):
        # see if it exists
        for child in self.children:
            if child.name == name:
                return child
        newNode = Tree (name)
        self.children.append (newNode)
        return newNode

    #-----------------------------------------------------------------------------------------------
    # @bsimethod
    #---------------+---------------+---------------+---------------+---------------+---------------
    def ToString (self, depth):
        return self.ToStringPathStyle (0, "")

    #-----------------------------------------------------------------------------------------------
    # @bsimethod
    #---------------+---------------+---------------+---------------+---------------+---------------
    def ToStringTreeStyle (self, depth):
        result = '  '*depth + "[" + self.name + "]\n"
        for child in self.children:
            result += child.ToString (depth+1)
        return result

    #-----------------------------------------------------------------------------------------------
    # @bsimethod
    #---------------+---------------+---------------+---------------+---------------+---------------
    def MakePCHPart (self):
        return "$(PCHHeaderDependency) ${MultiCompileDepends}"

    #-----------------------------------------------------------------------------------------------
    # @bsimethod
    #---------------+---------------+---------------+---------------+---------------+---------------
    def MakeObjPart (self, name):
        return "$(o)" + name[0: name.rfind('.')] + "$(oext):"
  
    #-----------------------------------------------------------------------------------------------
    # @bsimethod
    #---------------+---------------+---------------+---------------+---------------+---------------
    def ToStringPathStyle (self, depth, pathsofar):
        result = ""
        i = 0
        l = len(self.children)
        for i in range(0, l):
            result += self.children[i].ToStringPathStyle (depth+1, pathsofar + self.name + "\\")
        if 0 == l:
            #NOTE: this is coupled with the bmake multicompile pch process.
            #pathsofar starts with a slash - get rid of it.
            result = self.MakeObjPart (self.name) + " " + GetRootLocation () + pathsofar[1:len(pathsofar)] + self.name + " " + self.MakePCHPart () + "\n\n"
        return result

#-----------------------------------------------------------------------------------------------
# @bsimethod
#---------------+---------------+---------------+---------------+---------------+---------------
class FileRelationships:
    #-----------------------------------------------------------------------------------------------
    # @bsimethod
    #---------------+---------------+---------------+---------------+---------------+---------------
    def __init__(self):
        self.map = {}
        self.tree = Tree('')

    #-----------------------------------------------------------------------------------------------
    # @bsimethod
    #---------------+---------------+---------------+---------------+---------------+---------------
    def AddFile (self, baseDir, path):
        # remove basedir from path
        path = path [len (baseDir): len(path)]
        chunks = path.split ("\\")
        currentNode = self.tree
        for chunk in chunks:
            if '' != chunk:
                currentNode = currentNode.Add (chunk)

    #-----------------------------------------------------------------------------------------------
    # @bsimethod
    #---------------+---------------+---------------+---------------+---------------+---------------
    def BuildRelationships(self, baseDir, excludeDirs, excludeFiles, ext):

        for root, dirs, files in os.walk (baseDir): 
            # There are some dirs we don't want to look in, remove them from what we are looking at.
            for excludeDir in excludeDirs:
                if excludeDir in dirs:
                    dirs.remove (excludeDir)

            for filename in files:
                for exc in excludeFiles:
                    if re.match (exc, filename) == None:
                        if filename.lower().rfind (ext.lower()) == len(filename)-len(ext):
                            self.AddFile (baseDir, os.path.join (root, filename))

#-----------------------------------------------------------------------------------------------
# @bsimethod
#---------------+---------------+---------------+---------------+---------------+---------------
if __name__ == '__main__':
    if len(sys.argv) < 3:
        print 'This generates a list of files and their paths.'
        print 'Usage: GenerateTestFileList.py <RootTestDir> <extension> ' 
        print '  ie.  GenerateTestFileList.py d:\\src\\beijing\\DgnPlatformTest\\src\\Tests\\Published\\ cpp'
        exit (1)
    baseDir = sys.argv[1]
    extension = sys.argv[2]
    f = FileRelationships ()
    f.BuildRelationships (baseDir, ['CVS', '.hg'], ['\.*', '.*\.swp'], extension)
    print f.tree.ToString (0)

