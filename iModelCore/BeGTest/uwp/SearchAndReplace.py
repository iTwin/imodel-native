import sys,re

if __name__ == '__main__':
    fileName = sys.argv[1]
    toMatch  = sys.argv[2]
    replacement = sys.argv[3]

    oFile = open(fileName, 'r')
    lines = oFile.readlines()
    oFile.close()

    oFile = open(fileName, 'w')
    for line in lines:
        oFile.write (re.sub(toMatch, replacement, line))
    oFile.close()
