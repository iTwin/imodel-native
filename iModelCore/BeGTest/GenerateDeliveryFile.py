#----------------------------------------------------------------------------------------
#
#  $Source: GenerateDeliveryFile.py $
#
#  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
#
#----------------------------------------------------------------------------------------
import os, sys
import bentley_symlinks

def main ():
    if len (sys.argv) > 2:
        inputFile = sys.argv[1]
        outputDir = sys.argv[2].replace ('\\', '/')
        
        input = open (inputFile, 'r')
        for path in input.readlines ():
            path = path.replace ('\\', '/').strip ().rstrip ('/')
            
            if -1 != path.find (';'):
                s = path.split (';')
                path = bentley_symlinks.getSymlinkTarget (s[0])
                name = s[1]
            else:
                path = bentley_symlinks.getSymlinkTarget (path)
                name = os.path.basename (path)
                
            destPath = os.path.join (outputDir, name)
            sourcePath = path
            print ('src = ' + sourcePath.replace ('\\', '/').replace('$', '$$'))
            print ('dest = ' + destPath.replace ('\\', '/').replace('$', '$$'))
            if os.path.isdir (path):
                print ('always:')
                print ('    !~@mkdir $(dest)')
                print ('    python $(SrcBsiCommon)/build/CopyWithSymlinks.py $(src) $(dest)')
            else:
                print ('$(dest) : ${src}')
                print '    $(LinkFirstDepToFirstTarget)'
            print ('')
    else:
        print 'Specify input file and the output directory'

if __name__ == '__main__':
    main()
