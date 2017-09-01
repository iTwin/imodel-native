#--------------------------------------------------------------------------------------
#
#     $Source: imodeljs/nodejs/makeImodelJsNativePackages.py $
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

def setMacros(packagefile, versionCode, platform):
    str = ''
    with open(packagefile, 'r') as pf:
        str = pf.read()

    with open(packagefile, 'w') as pf:
        str = str.replace(r'${VERSION_CODE}', versionCode.lower())
        str = str.replace(r'${PLATFORM}', platform.lower())
        pf.write(str)

# Tell copytree to ignore binary files that should not be in the addon
def filterOutUnwantedFiles(dirname, files):
    if dirname.endswith('Assets'):
        return ['DgnGeoCoord'];
    return ['v8B02.dll', 'v8_libbaseB02.dll', 'v8_libplatformB02.dll']

# Copy a version-specific addon into place
# @param indir The path to the Product.  E.g.,
# D:\bim0200dev\out\Winx64\product\iModelJsNodeAddon-Windows
# @param versionCode The node version that this addon is for.  E.g., N_8_2
# @param platformandarch The target platform and achitecture. E.g., WinX64 or LinuxX64
def doCopy(productdir, localpackagedir, versionCode, platformandarch):
    # The product's directory structure should look like this:
    # D:\bim0200dev\out\Winx64\product\iModelJsNodeAddon-Windows\Addon\N_8_2\imodeljs.node
    # D:\bim0200dev\out\Winx64\product\iModelJsNodeAddon-Windows\Support

    srcsupportdir = os.path.join(productdir, "Support");
    srcnodefile = os.path.join(os.path.join(os.path.join(productdir, "Addon"), versionCode), "imodeljs.node")
    srcpackagefile = os.path.join(os.path.join(os.path.join(productdir, "Addon"), versionCode), "package.json")

    if not os.path.exists(srcnodefile) or not os.path.exists(srcsupportdir) or not os.path.exists(srcpackagefile):
        print '***'
        print('*** ' + productdir + ' -- invalid or incomplete iModelJsNodeAddon Product.')
        if not os.path.exists(srcnodefile):
            print ' ***   not found: ' + srcnodefile 
        if not os.path.exists(srcsupportdir):
            print ' ***   not found: ' + srcsupportdir 
        if not os.path.exists(srcpackagefile):
            print ' ***   not found: ' + srcpackagefile 
        print('***')
        exit(1)

    dstpackagefile = os.path.join(localpackagedir, 'package.json')
    dstaddondir = os.path.join(localpackagedir, 'addon')
    dstnodefile = os.path.join(dstaddondir, 'imodeljs.node')

    # NB: shutil.copytree insists on creating dstaddondir and will throw an exception if it already exists. That is why we don't call makedirs(dest...) here.
    shutil.copytree(srcsupportdir, dstaddondir, False, filterOutUnwantedFiles)

    shutil.copyfile(srcnodefile, dstnodefile)

    shutil.copyfile(srcpackagefile, dstpackagefile)

    setMacros(dstpackagefile, versionCode, platformandarch)

#
#   main
#
if __name__ == '__main__':
    if len(sys.argv) < 4:
        print "Syntax: ", sys.argv[0], " inputproductdir outputpackageparentdir platformandarch"
        exit(1)

    productdir = sys.argv[1]
    outdirParent = sys.argv[2]
    platformandarch = sys.argv[3]

    if outdirParent.endswith ('/') or outdirParent.endswith ('\\'):
        outdirParent = outdirParent[0:len(outdirParent)-1]

    if productdir.endswith ('/') or productdir.endswith ('\\'):
        productdir = productdir[0:len(productdir)-1]

    addonDir = os.path.join(productdir, 'Addon')
    if not os.path.basename(productdir).startswith('iModelJsNodeAddon') or not os.path.exists(addonDir):
        print '*** ' + productdir + ' does not appear to be the path to an iModelJsNodeAddon product directory';
        exit(1)

    for versionsubdir in os.listdir(addonDir):    

        # We are looking for the version-specific addon subdirectories. They tell us the names of the addons
        if (re.match(r'^([A-Z])_(\d+)_(\d+)_(\d+)$', versionsubdir) is None and re.match(r'^([A-Z])_(\d+)_(\d+)$', versionsubdir) is None):
            print '*** ' + versionsubdir + ' is unexpected. Only version-specific subdirectories should appear under addon.';
            continue

        # Compute the name of a directory that we can use to stage this package. This is just a temporary name.
        # The real name of the package is inside the package.json file.
        localpackagename = 'imodeljs-' + versionsubdir.replace('_', '-') + '-' + platformandarch

        packagedir = os.path.join(outdirParent, localpackagename)

        print 'Creating ' + packagedir + ' ...'

        if os.path.exists(packagedir):
            shutil.rmtree(packagedir)

        doCopy(productdir, packagedir, versionsubdir, platformandarch)

    exit(0)
