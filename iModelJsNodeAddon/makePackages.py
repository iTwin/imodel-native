#--------------------------------------------------------------------------------------
#
#     $Source: makePackages.py $
#
#  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------

# This program creates a directory that looks like an npm package, naming it
# with the specified name, and copying into it the imodeljs node addons that are defined in the
# specified product.

import os
import sys
import re
import shutil
import subprocess

# publish a package
def publishPackage(packagedir, doPublish, tag):

    if not doPublish:
        print packagedir
        return

    pubcmd = 'npm publish '
    pubcmd += '--@bentley:registry=https://bentley.jfrog.io/bentley/api/npm/staging/ '
    pubcmd += packagedir

    if tag != None:
        pubcmd = pubcmd + ' --tag ' + tag

    if 0 != os.system(pubcmd):
        exit(1)

# Replace ${macros} with values in specified file
def writePackageJson(packagefile, NODE_OS = None, NODE_CPU = None, PACKAGE_VERSION = None, COMPATIBLE_API_PACKAGE_VERSIONS = None, NODE_ENGINES = None):
    str = ''
    with open(packagefile, 'r') as pf:
        str = pf.read()

    with open(packagefile, 'w') as pf:
        if (NODE_OS):
            str = str.replace(r'${NODE_OS}', NODE_OS.lower())
        if (NODE_CPU):
            str = str.replace(r'${NODE_CPU}', NODE_CPU.lower())
        if (NODE_ENGINES):
            str = str.replace(r'${NODE_ENGINES}', NODE_ENGINES.lower())
        if (PACKAGE_VERSION):
            str = str.replace(r'${PACKAGE_VERSION}', PACKAGE_VERSION.lower())
        if (COMPATIBLE_API_PACKAGE_VERSIONS):
            str = str.replace(r'${COMPATIBLE_API_PACKAGE_VERSIONS}', COMPATIBLE_API_PACKAGE_VERSIONS.lower())
        pf.write(str)

# Compute the range of versions of addon apis that are compatible with this addon.
# They are apis with the same or lower minor and/or patch version, within the same major version.
def compute_compatible_api_version_range(packageVersion):
    return "<=" + packageVersion + "  >=" + packageVersion.split('.')[0] + ".0.0"

# Copy a version-specific addon into place
# @param outdirParent The path to the output package's parent directory
# @param inputProductdir The path to the Product that contains the ingredients, e.g., D:\bim0200dev\out\Winx64\product\iModelJsNodeAddon-Windows
# @param nodeOS The target platform (using Node terminology)
# @param nodeCPU The target CPU (using Node terminology)
# @param packageVersion The semantic version number for the generated package
# @param sourceDir The source directory, i.e., %SrcRoot%iModelJsNodeAddon
# @return the full path to the generated package directory
def generate_package_for_platform(outdirParent, inputProductdir, nodeOS, nodeCPU, packageVersion, sourceDir):

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

    writePackageJson(dstpackagefile, NODE_OS = nodeOS, NODE_CPU = nodeCPU, PACKAGE_VERSION = packageVersion, NODE_ENGINES = ' ')

    return outputpackagedir

# Generate imodeljs-nodeaddonapi
# @param outdirParent The path to the output package's parent directory
# @param parentSourceDir The iModelJsNodeAddon source directory, i.e., %SrcRoot%iModelJsNodeAddon
# @param packageVersion The semantic version number for the generated package
# @return the full path to the generated package directory
def generate_imodeljs_native_platform_api(outdirParent, parentSourceDir, packageVersion):

    outputpackagedir = os.path.join(outdirParent, 'imodeljs-native-platform-api')

    apiSourceDir = os.path.join(parentSourceDir, 'api_package')

    os.makedirs(outputpackagedir)

    packageTemplateFileName = 'package.json.template'

    # Copy some files into place without modifying them.
    filesToCopy = ['installImodelJsNative.js', 'formatPackageName.js', 'loadNativePlatform.js','README.md', 'LICENSE.md']

    for fileToCopy in filesToCopy:
        shutil.copyfile(os.path.join(apiSourceDir, fileToCopy), os.path.join(outputpackagedir, fileToCopy))

    # Generate the package.json file
    dstpackagefile = os.path.join(outputpackagedir, 'package.json')
    shutil.copyfile(os.path.join(apiSourceDir, packageTemplateFileName), dstpackagefile)

    writePackageJson(dstpackagefile, PACKAGE_VERSION = packageVersion)

    return outputpackagedir

#
#   main
#
if __name__ == '__main__':
    if len(sys.argv) < 8:
        print "Syntax: ", sys.argv[0], " inputproductdir outputpackageparentdir nodeOS nodeCPU packageversionfilename sourceDir {publish|print} [tag]"
        exit(1)

    productdir = sys.argv[1]
    outdirParent = sys.argv[2]
    nodeOS = sys.argv[3].lower()
    nodeCPU = sys.argv[4].lower()
    packageVersionFileName = sys.argv[5]
    sourceDir = sys.argv[6]
    doPublish = (sys.argv[7].lower() == 'publish')

    # TBD: Pass a tag in or read it from a special file? How to prevent stale tags values?
    tag = None

    if outdirParent.endswith ('/') or outdirParent.endswith ('\\'):
        outdirParent = outdirParent[0:len(outdirParent)-1]

    if productdir.endswith ('/') or productdir.endswith ('\\'):
        productdir = productdir[0:len(productdir)-1]

    # The package semantic version number (n.m.p) is stored in a file. Inject this version number into all of the package files that we generate.
    packageVersion = ""
    with open(packageVersionFileName, 'r') as pvf:
        packageVersion = pvf.read()
    packageVersion = packageVersion.strip()

    addonDir = os.path.join(productdir, 'Addon')

    if os.path.exists(outdirParent):
        print '*** ' + outdirParent + ' already exists. Remove output directory before calling this script'
        exit(1)

    os.makedirs(outdirParent)

    publishPackage(generate_package_for_platform(outdirParent, productdir, nodeOS, nodeCPU, packageVersion, sourceDir), doPublish, tag)

    # Generate the api package - PUBLISH ONLY ON WINDOWS - Builds for all other platforms only publish their platform-specific addon packages.
    if nodeOS != 'win32':
        doPublish = False

    publishPackage(generate_imodeljs_native_platform_api(outdirParent, sourceDir, packageVersion), doPublish, tag)

    exit(0)
