from multiprocessing import Process
import os, time, sys

scriptsFolderPath = sys.argv[1]
TestExePath = sys.argv[2]
WindowsSdkDir = sys.argv[3]
LogfilesPath = sys.argv[4]
TestExeName = sys.argv[5]

def RunUMDHScript():
  print "Coming in UMDH Script"
  os.system(scriptsFolderPath +"RunUmdh.py "+scriptsFolderPath+" "+TestExePath+" "+WindowsSdkDir+" "+LogfilesPath+" "+TestExeName+" > "+LogfilesPath+"output.txt")
  print "Coming out of UMDH Script"

def RunTestExe():
  print "Coming in RunTestExe Script"
  os.system('"'+TestExePath+'"')
  print "Coming out of RunTestExe Script"


if __name__ == '__main__':
    p1 = Process(target=RunTestExe)
    p1.start()
    time.sleep(2)
    p2 = Process(target=RunUMDHScript)
    p2.start()
    p1.join()
    p2.join()


