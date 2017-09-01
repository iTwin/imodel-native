import os
import sys

#----------------------------------------------------------------------------------------------------------------------------------------------------
def main():
    if len(sys.argv) < 3:
        print("WARN: not enough arguments")
        exit(0)

    pdbName = sys.argv[1]
    libNames = sys.argv[2:]

    newestLibTime = 0
    newestLibName = ""
    for libName in libNames:
        libTime = os.path.getmtime(libName)
        if libTime > newestLibTime:
            newestLibTime = libTime
            newestLibName = libName

    print("INFO: Setting '{0}' mtime to '{1}' due to '{2}'.".format(pdbName, newestLibTime, newestLibName))

    os.utime(pdbName, (os.path.getatime(pdbName), newestLibTime))

#----------------------------------------------------------------------------------------------------------------------------------------------------
if __name__ == '__main__':
    sys.exit(main())
