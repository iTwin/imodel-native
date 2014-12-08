# *******************************************************************************************************
# (c) 2014 Bentley Systems
#
# Name:			AuditCRT
# Author:		Lee Bull
# Description:	Generates report of C++ CRTs used in a folder and sub folders
# Notes:		Must be run from a Visual Studio command shell
#
# *******************************************************************************************************


import os
import sys
import subprocess
from win32com.client import Dispatch

binaryExt = [".exe", ".dll", ".com"]

crtNames = {"msvcr80.dll x86":"VS2005 x86",
			"msvcr80d.dll x86":"VS2005 Debug x86",
			"msvcr90.dll x86":"VS2008 x86",
			"msvcr90d.dll x86":"VS2008 Debug x86",
			"msvcr100.dll x86":"VS2010 x86",
			"msvcr100d.dll x86":"VS2010 Debug x86",
			"msvcr110.dll x86":"VS2012 x86",
			"msvcr110d.dll x86":"VS2012 Debug x86",
			"msvcr120.dll x86":"VS2013 x86",
			"msvcr120d.dll x86":"VS2013 Debug x86",
			"msvcr80.dll x64":"VS2005 x64",
			"msvcr80d.dll x64":"VS2005 Debug x64",
			"msvcr90.dll x64":"VS2008 x64",
			"msvcr90d.dll x64":"VS2008 Debug x64",
			"msvcr100.dll x64":"VS2010 x64",
			"msvcr100d.dll x64":"VS2010 Debug x64",
			"msvcr110.dll x64":"VS2012 x64",
			"msvcr110d.dll x64":"VS2012 Debug x64",
			"msvcr120.dll x64":"VS2013 x64",
			"msvcr120d.dll x64":"VS2013 Debug x64"}
			

def getCRTVSVersion(name):

	n = crtNames.get(name)
	
	if n != None:
		return n

	return ""
	

def isBinary(file):

	fileName, fileExtension = os.path.splitext(file)
	
	fileExtension = fileExtension.lower()
	
	for ext in binaryExt:
		if fileExtension == ext:
			return True
			
	return False

	
def getCRTInLine(line):

	end = -1
	extFound = ""

	s = line.find("MSVCR")
	
	v = None
	
	if s > 0:
	
		for ext in binaryExt:
		
			e = line.find(ext, s)

			if e > 0:
				if end == -1:
					end = e
					extFound = ext
				else:
					if e < end:
						end = e
						extFound = ext
		
		if end != -1:
			v = line[s:end+len(extFound)]
	
			v = v.lower()
	
	return v

	
def getPlatformInLine(line):
	s = line.find("machine (x86)")
	if s > 0:
		return "x86"
			
	s = line.find("machine (x64)")
	if s > 0:
		return "x64"
		
	return ""
	
	
def auditCRT(rootdir):

	d = {}
	
	for root, _, files in os.walk(rootdir):

		for file in files:
						
			f = os.path.join(root, file)
						
			if isBinary(f):
												
				dumpbinOptions = "/imports " + '"' + f + '"'
				
				p = subprocess.Popen(["dumpbin", "/headers", "/imports", f], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
				out, err = p.communicate()
								
				outLines = out.split('\n')
				
				platform = ""
								
				if file == "scannermod.dll":
					print "Processing Scannermod"
				
				for line in outLines:
								
					if platform == "":
						platform = getPlatformInLine(line)
				
					v = getCRTInLine(line)
									
					if v != None:
					
						v = v + " " + platform
						
						if v not in d:
							d.update({v : [f]})
							print "::::" + file + " " + v
						else:
							d[v].insert(0, f)
							
							
	return d

	
def getReportText(d):

	s = ""
															
	for k in d:
	
		s += "********************************************" + "\n"
		s += k[0:k.find(" ")] + " (" + getCRTVSVersion(k) + ")" + "\n"
		s += "********************************************" + "\n\n"
		
		for f in d[k]:
			s = s + f + "\n"
			
		s += "\n\n"

	s += "\n"
	
	return s

	
if len(sys.argv) == 2:		

	rootdir = sys.argv[1]

	report = auditCRT(rootdir)
	
	reportText = getReportText(report)
	
	print reportText

