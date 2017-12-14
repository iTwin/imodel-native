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
import subprocess

# publish a package
def publishPackage(packagedir, doPublish, tag):

    if not doPublish:
        print packagedir;
        return;

    pubcmd = 'npm publish ' + packagedir;

    if tag != None:
        pubcmd = pubcmd + ' --tag ' + tag;

    if 0 != os.system(pubcmd):
        exit(1);

# Replace ${macros} with values in specified file
def setMacros(packagefile, NODE_VERSION_CODE = None, NODE_OS = None, NODE_CPU = None, PACKAGE_VERSION = None, DECL_FILE_NAME = None, NODE_ENGINES = None):
    str = ''
    with open(packagefile, 'r') as pf:
        str = pf.read()

    with open(packagefile, 'w') as pf:
        if (NODE_VERSION_CODE):
            str = str.replace(r'${NODE_VERSION_CODE}', NODE_VERSION_CODE.lower())
        if (NODE_OS):
            str = str.replace(r'${NODE_OS}', NODE_OS.lower())
        if (NODE_CPU):
            str = str.replace(r'${NODE_CPU}', NODE_CPU.lower())
        if (NODE_ENGINES):
            str = str.replace(r'${NODE_ENGINES}', NODE_ENGINES.lower())
        if (PACKAGE_VERSION):
            str = str.replace(r'${PACKAGE_VERSION}', PACKAGE_VERSION.lower())
        if (DECL_FILE_NAME):
            str = str.replace(r'${DECL_FILE_NAME}', DECL_FILE_NAME)
        pf.write(str)

# Tell copytree to ignore binary files that should not be in the addon
def filterOutUnwantedFiles(dirname, files):
    if dirname.endswith('Assets'):
        return ['DgnGeoCoord'];
    return ['v8B02.dll', 'v8_libbaseB02.dll', 'v8_libplatformB02.dll']

# Copy a version-specific addon into place
# @param outdirParent The path to the output package's parent directory
# @param inputProductdir The path to the Product that contains the ingredients, e.g., D:\bim0200dev\out\Winx64\product\iModelJsNodeAddon-Windows
# @param versionsubdir The name of the subdirectory that contains the addon. The name of the subdir encodes the version of node that this addon is for.  E.g., N_8_2
# @param nodeOS The target platform (using Node terminology)
# @param nodeCPU The target CPU (using Node terminology)
# @param packageVersion The semantic version number for the generated package
# @param sourceDir The source directory, i.e., %SrcRoot%iModelJsNodeAddon
# @return the full path to the generated package directory
def generate_addon_for_platform(outdirParent, inputProductdir, versionsubdir, nodeOS, nodeCPU, packageVersion, sourceDir):

    # Generate the name of the addon package
    versionsubdir = versionsubdir.lower();

    # Compute the name of a directory that we can use to stage this package. This is just a temporary name.
    # The real name of the package is inside the package.json file.
    outputpackagename = 'imodeljs-' + versionsubdir + '-' + nodeOS + '-' + nodeCPU

    outputpackagedir = os.path.join(outdirParent, outputpackagename)

    if os.path.exists(outputpackagedir):
        shutil.rmtree(outputpackagedir)

    # The input product's directory structure should look like this:
    # D:\bim0200dev\out\Winx64\product\iModelJsNodeAddon-Windows\Addon\N_8_2\imodeljs.node
    # D:\bim0200dev\out\Winx64\product\iModelJsNodeAddon-Windows\Support

    srcsupportdir = os.path.join(inputProductdir, "Support");
    srcnodefile = os.path.join(os.path.join(os.path.join(inputProductdir, "Addon"), versionsubdir), "imodeljs.node")
    srcpackagefile = os.path.join(sourceDir, "package.json.template")

    if not os.path.exists(srcnodefile) or not os.path.exists(srcsupportdir) or not os.path.exists(srcpackagefile):
        print '***'
        print('*** ' + inputProductdir + ' -- invalid or incomplete iModelJsNodeAddon Product.')
        if not os.path.exists(srcnodefile):
            print ' ***   not found: ' + srcnodefile 
        if not os.path.exists(srcsupportdir):
            print ' ***   not found: ' + srcsupportdir 
        print('***')
        exit(1)

    dstpackagefile = os.path.join(outputpackagedir, 'package.json')
    dstaddondir = os.path.join(outputpackagedir, 'addon')
    dstnodefile = os.path.join(dstaddondir, 'imodeljs.node')

    # NB: shutil.copytree insists on creating dstaddondir and will throw an exception if it already exists. That is why we don't call os.makedirs(dest...) here.
    shutil.copytree(srcsupportdir, dstaddondir, False, filterOutUnwantedFiles)

    shutil.copyfile(srcnodefile, dstnodefile)

    shutil.copyfile(srcpackagefile, dstpackagefile)

    # The node addon is specific to a version of node.
    # *** NEEDS WORK: We don't inject this constraint for Electron addons -- not sure how to specify electron engines.
    nodeEngines = ' '
    if (versionsubdir.lower().startswith('n_')):
        nodeEngineVersion = versionsubdir[2:].replace('_', '.')
        nodeEngines = '"engines": "' + nodeEngineVersion + '",'

    setMacros(dstpackagefile, NODE_VERSION_CODE = versionsubdir, NODE_OS = nodeOS, NODE_CPU = nodeCPU, PACKAGE_VERSION = packageVersion, NODE_ENGINES = nodeEngines)

    return outputpackagedir

# Generate imodeljs-nodeaddonapi
# @param outdirParent The path to the output package's parent directory
# @param parentSourceDir The iModelJsNodeAddon source directory, i.e., %SrcRoot%iModelJsNodeAddon
# @param packageVersion The semantic version number for the generated package
# @return the full path to the generated package directory
def generate_imodeljs_nodeaddonapi(outdirParent, parentSourceDir, packageVersion):

    outputpackagedir = os.path.join(outdirParent, 'imodeljs-nodeaddonapi')

    apiSourceDir = os.path.join(parentSourceDir, 'api_package');

    os.makedirs(outputpackagedir);

    declFileName = 'imodeljs-nodeaddonapi.d.ts'
    packageTemplateFileName = 'package.json.template'

    # Copy some files into place without modifying them.
    filesToCopy = [declFileName, 'README.md']

    for fileToCopy in filesToCopy:
        shutil.copyfile(os.path.join(apiSourceDir, fileToCopy), os.path.join(outputpackagedir, fileToCopy))

    # Generate the package.json file
    dstpackagefile = os.path.join(outputpackagedir, 'package.json')
    shutil.copyfile(os.path.join(apiSourceDir, packageTemplateFileName), dstpackagefile);

    setMacros(dstpackagefile, PACKAGE_VERSION = packageVersion, DECL_FILE_NAME = declFileName)

    return outputpackagedir;


# Generate imodeljs-nodeaddon
# @param outdirParent The path to the output package's parent directory
# @param parentSourceDir The iModelJsNodeAddon source directory, i.e., %SrcRoot%iModelJsNodeAddon
# @param packageVersion The semantic version number for the generated package
# @return the full path to the generated package directory
def generate_imodeljs_nodeaddon(outputpackagedir, parentSourceDir, packageVersion):

    outputpackagedir = os.path.join(outdirParent, 'imodeljs-nodeaddon')

    addonSourceDir = os.path.join(parentSourceDir, 'addon_package');

    os.makedirs(outputpackagedir);

    declFileName = 'NodeAddonLoader.d.ts'
    packageTemplateFileName = 'package.json.template'

    # Copy some files into place without modifying them.
    filesToCopy = [declFileName, 'NodeAddonLoader.js', 'README.md']

    for fileToCopy in filesToCopy:
        shutil.copyfile(os.path.join(addonSourceDir, fileToCopy), os.path.join(outputpackagedir, fileToCopy))

    # Generate the package.json file
    dstpackagefile = os.path.join(outputpackagedir, 'package.json')
    shutil.copyfile(os.path.join(addonSourceDir, packageTemplateFileName), dstpackagefile);

    setMacros(dstpackagefile, PACKAGE_VERSION = packageVersion, DECL_FILE_NAME = declFileName)
    
    return outputpackagedir;

#
#   main
#
if __name__ == '__main__':
    if len(sys.argv) < 8:
        print "Syntax: ", sys.argv[0], " inputproductdir outputpackageparentdir nodeOS nodeCPU packageversionfilename sourceDir {publish|print} [tag]"
        exit(1)

    productdir = sys.argv[1]
    outdirParent = sys.argv[2]
    nodeOS = sys.argv[3].lower();
    nodeCPU = sys.argv[4].lower();
    packageVersionFileName = sys.argv[5]
    sourceDir = sys.argv[6]
    doPublish = (sys.argv[7].lower() == 'publish');

    # TBD: Pass a tag in or read it from a special file? How to prevent stale tags values?
    tag = None;

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
        if (re.match(r'^([a-z])_(\d+)_(\d+)_(\d+)$', versionsubdir) is None and re.match(r'^([a-z])_(\d+)_(\d+)$', versionsubdir) is None):
            print '*** ' + versionsubdir + ' is unexpected. Only version-specific subdirectories should appear under addon.';
            continue
        
        publishPackage(generate_addon_for_platform(outdirParent, productdir, versionsubdir, nodeOS, nodeCPU, packageVersion, sourceDir), doPublish, tag);

    # Generate the other packages that describe or re-deliver the platform-specific addon packages
    publishPackage(generate_imodeljs_nodeaddonapi(outdirParent, sourceDir, packageVersion), doPublish, tag);
    publishPackage(generate_imodeljs_nodeaddon(outdirParent, sourceDir, packageVersion), doPublish, tag);

    exit(0)
