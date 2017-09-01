#--------------------------------------------------------------------------------------
#
#     $Source: imodeljs/makeImodelJsNativePackage.py $
#
#  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------

# This program creates a directory that looks like an npm package, naming it
# with the specified name, and copying into it the imodeljs node addons that are defined in the
# specified product.

import os
import fnmatch
import sys
import re
import errno
import shutil

def mkdir_if_necesssary(outdir):
    if not os.path.exists(outdir):
        os.mkdir(outdir)

def makedirs_if_necessary(outdir):
    try:
        os.makedirs(outdir)
    except OSError:
        pass

# Generate the contents of the package.json file
def generatePackageJsonStr(packagename, packagever):
    packagejson = "{"
    packagejson = packagejson + '"name": "@bentley/' + packagename + '",'
    packagejson = packagejson + '"version": "' + packagever + '",'
    packagejson = packagejson + '"author": {'
    packagejson = packagejson + '   "name": "Bentley Systems, Inc.",'
    packagejson = packagejson + '   "url": "http://www.bentley.com"'
    packagejson = packagejson + '}'
    packagejson = packagejson + '}'
    return packagejson

# Create the package.json file
def generatePackageJson(packagedir, packagename, packagever):
    packagefilename = os.path.join(packagedir, 'package.json')
    with open(packagefilename, "w") as packagefile:
        packagefile.write(generatePackageJsonStr(packagename, packagever));

# Tell copytree to ignore binary files that should not be in the addon
def filterOutUnwantedFiles(dirname, files):
    if dirname.endswith('Assets'):
        return ['DgnGeoCoord'];
    return ['v8B02.dll', 'v8_libbaseB02.dll', 'v8_libplatformB02.dll']

# Copy a version-specific addon into place
# @param indir The path to the Product.  E.g.,
# D:\bim0200dev\out\Winx64\product\iModelJsNodeAddon-Windows
# @param versionCode The node version that this addon is for.  E.g., N_8_2
def doCopy(productdir, outdir, versionCode):
    # The product's directory structure should look like this:
    # D:\bim0200dev\out\Winx64\product\iModelJsNodeAddon-Windows\Addon\N_8_2\imodeljs.node
    # D:\bim0200dev\out\Winx64\product\iModelJsNodeAddon-Windows\Support

    srcsupportdir = os.path.join(productdir, "Support");
    srcnodefile = os.path.join(os.path.join(os.path.join(productdir, "Addon"), versionCode), "imodeljs.node")

    if not os.path.exists(srcnodefile) or not os.path.exists(srcsupportdir):
        print '***'
        print('*** ERROR: "' + srcnodefile + '" was not found or ' + srcsupportdir + ' was not found.')
        print('*** ' + productdir + ' is not a valid iModelJsNodeAddon delivery Product.')
        print('***')
        exit(1)

    destPath = os.path.join(outdir, 'addon')

    dstnodefile = os.path.join(destPath, 'imodeljs.node')

    # NB: shutil.copytree insists on creating destPath and will throw an exception if it already exists. That is why we don't call makedirs(destPath) here.
    print 'Copying ' + srcsupportdir + ' to ' + destPath + ' ...'
    shutil.copytree(srcsupportdir, destPath, False, filterOutUnwantedFiles)

    print 'Copying ' + srcnodefile + ' to ' + dstnodefile + ' ...'
    shutil.copyfile(srcnodefile, dstnodefile)

#
#   main
#
if __name__ == '__main__':
    if len(sys.argv) < 6:
        print "Syntax: ", sys.argv[0], " inputproductdir outputpackageparentdir versionsubdir platform packageversion "
        exit(1)

    productdir = sys.argv[1]
    outdirParent = sys.argv[2]
    versionsubdir = sys.argv[3]
    platform = sys.argv[4]
    packageversion = sys.argv[5]

    if outdirParent.endswith ('/') or outdirParent.endswith ('\\'):
        outdirParent = outdirParent[0:len(outdirParent)-1]

    if productdir.endswith ('/') or productdir.endswith ('\\'):
        productdir = productdir[0:len(productdir)-1]

    addonDir = os.path.join(productdir, 'Addon')
    if not os.path.basename(productdir).startswith('iModelJsNodeAddon') or not os.path.exists(addonDir):
        print '*** ' + productdir + ' does not appear to be the path to an iModelJsNodeAddon product directory';
        exit(1)

    packagename = 'imodeljs-' + versionsubdir.replace('_', '-') + '-' + platform

    packagedir = os.path.join(outdirParent, packagename)
    if os.path.exists(packagedir):
        shutil.rmtree(packagedir)
    mkdir_if_necesssary(packagedir)

    generatePackageJson(packagedir, packagename, packageversion)

    for filename in os.listdir(addonDir):    

        # We are looking for the version-specific addon subdirectories. They tell us the names of the addons
        if versionsubdir != filename:
            continue

        doCopy(productdir, packagedir, filename)

    exit(0)
