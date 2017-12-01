#--------------------------------------------------------------------------------------
#
#     $Source: makePackages.py $
#
#  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------

# This program creates a directory that looks like an npm package, naming it
# with the specified name, and copying into it the imodeljs node addons that are defined in the
# specified product.

import os
import sys
import re
import shutil

def setMacros(packagefile, NODE_VERSION_CODE = None, PLATFORM = None, PACKAGE_VERSION = None, DECL_FILE_NAME = None):
    str = ''
    with open(packagefile, 'r') as pf:
        str = pf.read()

    with open(packagefile, 'w') as pf:
        if (NODE_VERSION_CODE):
            str = str.replace(r'${NODE_VERSION_CODE}', NODE_VERSION_CODE.lower())
        if (PLATFORM):
            str = str.replace(r'${PLATFORM}', PLATFORM.lower())
        if (PACKAGE_VERSION):
            str = str.replace(r'${PACKAGE_VERSION}', PACKAGE_VERSION.lower())
        if (DECL_FILE_NAME):
            str = str.replace(r'${DECL_FILE_NAME}', DECL_FILE_NAME.lower())
        pf.write(str)

# Tell copytree to ignore binary files that should not be in the addon
def filterOutUnwantedFiles(dirname, files):
    if dirname.endswith('Assets'):
        return ['DgnGeoCoord'];
    return ['v8B02.dll', 'v8_libbaseB02.dll', 'v8_libplatformB02.dll']

# Copy a version-specific addon into place
# @param outputpackagedir The path to the output directory in which the package should be generated
# @param inputProductdir The path to the Product that contains the ingredients, e.g., D:\bim0200dev\out\Winx64\product\iModelJsNodeAddon-Windows
# @param nodeVersionCode The node version that this addon is for.  E.g., N_8_2
# @param platformandarch The target platform and achitecture. E.g., WinX64 or LinuxX64
# @param packageVersion The semantic version number for the generated package
def doCopy(outputpackagedir, inputProductdir, nodeVersionCode, platformandarch, packageVersion):
    # The input product's directory structure should look like this:
    # D:\bim0200dev\out\Winx64\product\iModelJsNodeAddon-Windows\Addon\N_8_2\imodeljs.node
    # D:\bim0200dev\out\Winx64\product\iModelJsNodeAddon-Windows\Support

    srcsupportdir = os.path.join(inputProductdir, "Support");
    srcnodefile = os.path.join(os.path.join(os.path.join(inputProductdir, "Addon"), nodeVersionCode), "imodeljs.node")
    srcpackagefile = os.path.join(os.path.join(os.path.join(inputProductdir, "Addon"), nodeVersionCode), "package.json")

    if not os.path.exists(srcnodefile) or not os.path.exists(srcsupportdir) or not os.path.exists(srcpackagefile):
        print '***'
        print('*** ' + inputProductdir + ' -- invalid or incomplete iModelJsNodeAddon Product.')
        if not os.path.exists(srcnodefile):
            print ' ***   not found: ' + srcnodefile 
        if not os.path.exists(srcsupportdir):
            print ' ***   not found: ' + srcsupportdir 
        if not os.path.exists(srcpackagefile):
            print ' ***   not found: ' + srcpackagefile
        print('***')
        exit(1)

    dstpackagefile = os.path.join(outputpackagedir, 'package.json')
    dstaddondir = os.path.join(outputpackagedir, 'addon')
    dstnodefile = os.path.join(dstaddondir, 'imodeljs.node')

    # NB: shutil.copytree insists on creating dstaddondir and will throw an exception if it already exists. That is why we don't call os.makedirs(dest...) here.
    shutil.copytree(srcsupportdir, dstaddondir, False, filterOutUnwantedFiles)

    shutil.copyfile(srcnodefile, dstnodefile)

    shutil.copyfile(srcpackagefile, dstpackagefile)

    setMacros(dstpackagefile, nodeVersionCode, platformandarch, packageVersion)

# Generate the package that defines the API implemented by all platform-specific packages
# @param outputpackagedir The path to the output directory in which the package should be generated
# @param productdir The path to the Product.  E.g., D:\bim0200dev\out\Winx64\product\iModelJsNodeAddon-Windows
# @param packageVersion The semantic version number for the generated package
def generateApiPackage(outputpackagedir, sourceDir, packageVersion):

    declFileName = 'imodeljs-nodeaddonapi.d.ts'

    filesToCopy = [declFileName, 'README.md']

    os.makedirs(outputpackagedir);
    for fileToCopy in filesToCopy:
        shutil.copyfile(os.path.join(sourceDir, fileToCopy), os.path.join(outputpackagedir, fileToCopy))

    dstpackagefile = os.path.join(outputpackagedir, 'package.json')
    shutil.copyfile(os.path.join(sourceDir, 'imodeljs-nodeaddonapi.package.json'), dstpackagefile);

    setMacros(dstpackagefile, PACKAGE_VERSION = packageVersion, DECL_FILE_NAME = declFileName)

#
#   main
#
if __name__ == '__main__':
    if len(sys.argv) < 6:
        print "Syntax: ", sys.argv[0], " inputproductdir outputpackageparentdir platformandarch packageversionfilename sourceDir"
        exit(1)

    productdir = sys.argv[1]
    outdirParent = sys.argv[2]
    platformandarch = sys.argv[3]
    packageVersionFileName = sys.argv[4]
    sourceDir = sys.argv[5]

    if outdirParent.endswith ('/') or outdirParent.endswith ('\\'):
        outdirParent = outdirParent[0:len(outdirParent)-1]

    if productdir.endswith ('/') or productdir.endswith ('\\'):
        productdir = productdir[0:len(productdir)-1]

    # The package semantic version number (n.m.p) is stored in a file. Inject this version number into all of the package files that we generate.
    packageVersion = "";
    with open(packageVersionFileName, 'r') as pvf:
        packageVersion = pvf.read()
    packageVersion = packageVersion.strip()

    addonDir = os.path.join(productdir, 'Addon')
    if not os.path.basename(productdir).startswith('iModelJsNodeAddon') or not os.path.exists(addonDir):
        print '*** ' + productdir + ' does not appear to be the path to an iModelJsNodeAddon product directory';
        exit(1)

    if os.path.exists(outdirParent):
        print '*** ' + outdirParent + ' already exists. Remove output directory before calling this script';
        exit(1)

    os.makedirs(outdirParent)

    # Generate a platform-specific package fo each platform that was built

    for versionsubdir in os.listdir(addonDir):    

        # We are looking for the version-specific addon subdirectories. They tell us the names of the addons
        if (re.match(r'^([A-Z])_(\d+)_(\d+)_(\d+)$', versionsubdir) is None and re.match(r'^([A-Z])_(\d+)_(\d+)$', versionsubdir) is None):
            print '*** ' + versionsubdir + ' is unexpected. Only version-specific subdirectories should appear under addon.';
            continue

        # Compute the name of a directory that we can use to stage this package. This is just a temporary name.
        # The real name of the package is inside the package.json file.
        outputpackagename = 'imodeljs-' + versionsubdir + '-' + platformandarch

        outputpackagedir = os.path.join(outdirParent, outputpackagename)

        if os.path.exists(outputpackagedir):
            shutil.rmtree(outputpackagedir)

        doCopy(outputpackagedir, productdir, versionsubdir, platformandarch, packageVersion)

        print 'npm publish ' + outputpackagedir;

    # Generate the package that defines the API implemented by all platform-specific packages
    outputpackagedir = os.path.join(outdirParent, 'imodeljs-nodeaddonapi')
    generateApiPackage(outputpackagedir, sourceDir, packageVersion);
    print 'npm publish ' + outputpackagedir;

    exit(0)
