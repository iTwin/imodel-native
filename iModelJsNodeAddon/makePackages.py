#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------

# This program creates a directory that looks like an npm package, naming it
# with the specified name, and copying into it the imodeljs node addons that are defined in the
# specified product.

import os
import sys
import re
import shutil
import subprocess
import glob

SUPPORTED_PLATFORMS = {
    'x64' : {
        'Platform': 'Windows',
        'NodeOS': 'win32',
        'NodeCPU': 'x64'
    },
    'linuxx64' : {
        'Platform': 'Linux',
        'NodeOS': 'linux',
        'NodeCPU': 'x64'
    },
    'macosx64' : {
        'Platform': 'Darwin',
        'NodeOS': 'darwin',
        'NodeCPU': 'x64'
    },
    'macosarm64' : {
        'Platform': 'Darwin',
        'NodeOS': 'darwin',
        'NodeCPU': 'arm64'
    }
}

# Replace ${macros} with values in specified file
def writePackageJson(packagefile, NODE_OS = None, NODE_CPU = None, PACKAGE_VERSION = None):
    str = ''
    with open(packagefile, 'r') as pf:
        str = pf.read()

    with open(packagefile, 'w') as pf:
        if (NODE_OS):
            str = str.replace(r'${NODE_OS}', NODE_OS.lower())
        if (NODE_CPU):
            str = str.replace(r'${NODE_CPU}', NODE_CPU.lower())
        if (PACKAGE_VERSION):
            str = str.replace(r'${PACKAGE_VERSION}', PACKAGE_VERSION.lower())
        pf.write(str)

# Copy a version-specific addon into place
# @param outdirParent The path to the output package's parent directory
# @param inputProductdir The path to the Product that contains the ingredients, e.g., D:\imodel02\out\Winx64\product\iModelJsNodeAddon-Windows
# @param packageVersion The semantic version number for the generated package
# @param sourceDir The source directory, i.e., %SrcRoot%iModelJsNodeAddon
# @param platform The platform of the generated package, see SUPPORTED_PLATFORMS
# @return the full path to the generated package directory
def generate_package_for_platform(outdirParent, inputProductdir, packageVersion, sourceDir, platform):

    nodeOS = SUPPORTED_PLATFORMS[platform]['NodeOS']
    nodeCPU = SUPPORTED_PLATFORMS[platform]['NodeCPU']

    # Compute the name of a directory that we can use to stage this package. This is just a temporary name.
    # The real name of the package is inside the package.json file.
    outputpackagename = 'imodeljs-' + nodeOS + '-' + nodeCPU

    outputpackagedir = os.path.join(outdirParent, outputpackagename)

    if os.path.exists(outputpackagedir):
        shutil.rmtree(outputpackagedir)

    srcpackagefile = os.path.join(sourceDir, "package.json.template")
    dstpackagefile = os.path.join(outputpackagedir, 'package.json')
    dstaddondir = os.path.join(outputpackagedir, 'addon')

    # NB: shutil.copytree insists on creating dstaddondir and will throw an exception if it already exists. That is why we don't call os.makedirs(dest...) here.
    shutil.copytree(inputProductdir, outputpackagedir, False)

    shutil.copyfile(srcpackagefile, dstpackagefile)

    shutil.copyfile(os.path.join(sourceDir, 'README-Public.md'), os.path.join(outputpackagedir, 'README.md'))

    shutil.copyfile(os.path.join(sourceDir, 'api_package', 'LICENSE.md'), os.path.join(outputpackagedir, 'LICENSE.md'))

    writePackageJson(dstpackagefile, NODE_OS = nodeOS, NODE_CPU = nodeCPU,  PACKAGE_VERSION = packageVersion)

    return outputpackagedir

# Generate @bentley/imodeljs-native
# @param outdirParent The path to the output package's parent directory
# @param parentSourceDir The iModelJsNodeAddon source directory, i.e., %SrcRoot%iModelJsNodeAddon
# @param packageVersion The semantic version number for the generated package
# @return the full path to the generated package directory
def generate_imodeljs_native_platform_api(outdirParent, parentSourceDir, packageVersion):

    outputpackagedir = os.path.join(outdirParent, 'imodeljs-native')

    apiSourceDir = os.path.join(parentSourceDir, 'api_package')

    os.makedirs(outputpackagedir)

    packageTemplateFileName = 'package.json.template'

    # Copy some files into place without modifying them.
    filesToCopy = ['installNativePlatform.js', 'README.md', 'LICENSE.md']

    for fileToCopy in filesToCopy:
        shutil.copyfile(os.path.join(apiSourceDir, fileToCopy), os.path.join(outputpackagedir, fileToCopy))

    for fileToCopy in glob.glob(os.path.join(os.environ['BuildContext'], 'Delivery', 'lib', '*.d.ts*')):
        shutil.copy(fileToCopy, outputpackagedir)

    for fileToCopy in glob.glob(os.path.join(os.environ['BuildContext'], 'Delivery', 'lib', '*.js*')):
        shutil.copy(fileToCopy, outputpackagedir)

    # Generate the package.json file
    dstpackagefile = os.path.join(outputpackagedir, 'package.json')
    shutil.copyfile(os.path.join(apiSourceDir, packageTemplateFileName), dstpackagefile)

    writePackageJson(dstpackagefile, PACKAGE_VERSION = packageVersion)

    return outputpackagedir

#
# main
#
if __name__ == '__main__':
    if len(sys.argv) < 6:
        print ("Syntax: " + sys.argv[0] + " inputproductdir outputpackageparentdir packageversionfilename sourceDir platform")
        exit(1)

    platform = sys.argv[5].lower()

    productDirParent = os.path.normpath(sys.argv[1])
    productDir = os.path.join(productDirParent, 'iModelJsNative-' + SUPPORTED_PLATFORMS[platform]['Platform'])

    outdirParent = os.path.normpath(sys.argv[2])
    packageVersionFileName = sys.argv[3]
    sourceDir = sys.argv[4]

    # The package semantic version number (n.m.p) is stored in a file. Inject this version number into all of the package files that we generate.
    packageVersion = ""
    with open(packageVersionFileName, 'r') as pvf:
        packageVersion = pvf.read()
    packageVersion = packageVersion.strip()

    if os.path.exists(outdirParent):
        print ('*** ' + outdirParent + ' already exists. Remove output directory before calling this script')
        exit(1)

    os.makedirs(outdirParent)

    generate_package_for_platform(outdirParent, productDir, packageVersion, sourceDir, platform)

    generate_imodeljs_native_platform_api(outdirParent, sourceDir, packageVersion)

    exit(0)
