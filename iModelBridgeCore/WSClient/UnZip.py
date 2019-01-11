#---------------------------------------------------------------------------------------+
#
#     $Source: UnZip.py $
#
#  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
#
#---------------------------------------------------------------------------------------+

#---------------------------------------------------------------------------------------+
# Extract zip file to target directory 
# ARGUMENTS:
#  1 - zip file path.
#  2 - output folder path where contents of zip will be placed. Will remove if exists.
#---------------------------------------------------------------------------------------+

import os, shutil, sys, zipfile

file = sys.argv[1]
outdir = sys.argv[2]

if os.path.exists(outdir):
    shutil.rmtree(outdir)

os.makedirs(outdir)

fh = open(file,'rb')

z = zipfile.ZipFile(fh)
for name in z.namelist():
    outpath = outdir+'/';
    z.extract(name, outpath)
fh.close()
