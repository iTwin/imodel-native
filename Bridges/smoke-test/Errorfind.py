import fnmatch
import os


tempFile1 = open('C:\\bridge\\log.txt','rb')
resultFile=open('C:\\bridge\\Errors.txt','w')
mytext1 = tempFile1.readlines()
for line1 in mytext1:
    if line1.__contains__('Error')| line1.__contains__('ERROR'):
        resultFile.writelines(line1)

   # if line1.__contains__('ERROR iModelBridge'):
    #   print 'Smoke Fail'
     #  exit (-1) 
     

 
     
