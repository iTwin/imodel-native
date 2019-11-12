#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See COPYRIGHT.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------

import os, sys, shutil

#--------------------------------------------------------------------------------------------------
# Replace ${macros} with values in specified file
def writePackageJson(packagefile, NODE_OS = None, NODE_CPU = None, PACKAGE_VERSION = None, COMPATIBLE_API_PACKAGE_VERSIONS = None, NODE_ENGINES = None):
    str = ''
    with open(packagefile, 'r') as pf:
        str = pf.read()

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
        
    with open(packagefile, 'w') as pf:
        pf.write(str)

#--------------------------------------------------------------------------------------------------
def generatePackageForPlatform(packageDir, productDir, nodeOS, nodeCPU, packageVersion, sourceDir):
    # Compute the name of a directory that we can use to stage this package. This is just a temporary name.
    # The real name of the package is inside the package.json file.
    outputpackagename = 'imodeljs-' + nodeOS + '-' + nodeCPU
    outputpackagedir = os.path.join(packageDir, outputpackagename)
    pkgtemplatedir = os.path.dirname(os.path.abspath(__file__))
    srcpackagefile = os.path.join(pkgtemplatedir, "package.json.template")
    dstpackagefile = os.path.join(outputpackagedir, 'package.json')
    dstaddondir = os.path.join(outputpackagedir, 'addon')

    # NB: shutil.copytree insists on creating dstaddondir and will throw an exception if it already exists. That is why we don't call os.makedirs(dest...) here.
    shutil.copytree(productDir, outputpackagedir, False)
    shutil.copyfile(srcpackagefile, dstpackagefile)
    shutil.copyfile(os.path.join(sourceDir, 'README-Public.md'), os.path.join(outputpackagedir, 'README.md'))
    shutil.copyfile(os.path.join(sourceDir, 'api_package', 'LICENSE.md'), os.path.join(outputpackagedir, 'LICENSE.md'))

    writePackageJson(dstpackagefile, NODE_OS = nodeOS, NODE_CPU = nodeCPU, PACKAGE_VERSION = packageVersion, NODE_ENGINES = ' ')

#--------------------------------------------------------------------------------------------------
def main():
    if len(sys.argv) != 6:
        print "Syntax: ", sys.argv[0], " productDir packageDir nodeOS nodeCPU sourceDir"
        return 1

    productDir = sys.argv[1].rstrip('/\\')
    packageDir = sys.argv[2].rstrip('/\\')
    nodeOS = sys.argv[3].lower()
    nodeCPU = sys.argv[4].lower()
    sourceDir = sys.argv[5]
    
    # The package semantic version number (n.m.p) is stored in a file. Inject this version number into all of the package files that we generate.
    packageVersion = ""
    with open(os.path.join(sourceDir, 'package_version.txt'), 'r') as pvf:
        packageVersion = pvf.read().strip()
    
    if os.path.exists(packageDir):
        shutil.rmtree(pacakgeDir)

    os.makedirs(packageDir)

    generatePackageForPlatform(packageDir, productDir, nodeOS, nodeCPU, packageVersion, sourceDir)

    return 0

#--------------------------------------------------------------------------------------------------
if __name__ == '__main__':
    sys.exit(main())
