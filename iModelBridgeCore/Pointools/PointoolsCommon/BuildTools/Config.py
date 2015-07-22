import sys
import os
import os.path
import copy

class Config:
	configFile							= ""
	rootPaths 							= []
	outputMKE							= False
	outputMultiCompileDepends			= False
	outputMinimalDependencies			= False
	outputPrecompiledHeaders			= False
	precompiledHeadersForcedInclude		= True
	outputDLMFormat						= False
	defaultMKEPolicy					= ""
		
	includePathsStd						= dict()
	includePathsLocal					= dict()
	includePaths						= dict()
	logFilePath							= ""
	logFile								= None
	ignoreSourceFile					= []
	ignoreDependencies					= dict()
	sourceFileTypes						= dict()
	projectSourceFile					= ""
	
	additionalLibraryPaths				= dict()
	additionalLibraries					= []
	standardLibraries					= ["kernel32.lib", "user32.lib", "gdi32.lib", "winspool.lib", "comdlg32.lib", "advapi32.lib", "shell32.lib", "ole32.lib", "oleaut32.lib", "uuid.lib", "odbc32.lib", "odbccp32.lib", "opengl32.lib", "glu32.lib", "shlwapi.lib", "wsock32.lib", "Ws2_32.lib"]
	ignoreLibraries						= []
	libraries							= []

	createStaticLibraries				= False
	platformSpecific					= True
	
	buildName							= "Debug"
	buildPlatform						= "Win32"
	
	target								= ""
	
	vsConfigPrefix						= "VS_Config_"
	
	outputPrecompuledHeaderFile			= False
	precompiledHeaderFile				= ""
	excludeFromPrecompiledHeader 		= []
	
	
	definitions							= []

	createBuildContextLibraries			= False
	
	outputDefinitions					= True
	outputIncludePaths					= True
	outputTargets						= True
	outputObjects						= True
	outputLibraryPaths					= True
	outputStandardLibraries				= True
	outputAdditionalLibraries			= True
	outputIgnoreLibraries				= True


	def getConfigString(self):
		s = "***********************************************************************" + "\n\n"
		s = s + "Visual Studio Configuration" + "\n\n"
		s = s + "***********************************************************************" + "\n\n"
		s = s + "Project File        : " + "\n" + self.projectSourceFile + "\n\n"
		s = s + "Build Name          : " + "\n" + self.buildName + "\n\n"
		s = s + "Build Platform      : " + "\n" + self.buildPlatform + "\n\n"
		s = s + "Root Paths          : " + "\n" + str(self.rootPaths).strip('[]') + "\n\n"
		s = s + "Include Paths       : " + "\n" + str(self.includePaths).strip('[]') + "\n\n"
		s = s + "Ignore Dependencies : " + "\n" + str(self.ignoreDependencies).strip('[]') + "\n\n"
		s = s + "Ignore Source Files : " + "\n" + str(self.ignoreSourceFiles).strip('[]') + "\n\n"
		s = s + "Definitions         : " + "\n" + str(self.definitions) + "\n\n"
		s = s + "Library Paths       : " + "\n" + str(self.additionalLibraryPaths).strip('[]') + "\n\n"
		s = s + "Standard Libraries  : " + "\n" + str(self.standardLibraries).strip('[]') + "\n\n"
		s = s + "Additional Libraries: " + "\n" + str(self.additionalLibraries).strip('[]') + "\n\n"
		s = s + "Ignore Libraries    : " + "\n" + str(self.ignoreLibraries).strip('[]') + "\n\n"
		s = s + "***********************************************************************"

		return s

	def __del__(self):
		if self.logFile != None:
			self.logFile.close()

	def expandFilePath(self, filePath):

		filePath = os.path.normpath(filePath)
		expandedFilepath = os.path.expandvars(filePath)

		while(expandedFilepath != filePath):
			filePath = os.path.normpath(expandedFilepath)
			expandedFilepath = os.path.expandvars(filePath)

		return os.path.normpath(filePath)


	def initialize(self):

		self.mode = "File"

		self.projectSourceFile = self.expandFilePath(self.projectSourceFile)


	def readConfig(self, configPath):

		print "Reading config file : " + configPath + "\n"

		fileIn=open(configPath)
				
		for line in fileIn:
			assignment=line.split('=')
			if len(assignment)==2:
				lhs=assignment[0].strip()
				rhs=assignment[1].strip()
				
				print lhs + "===>" + rhs
				
				setattr(self, lhs, eval(rhs))
		
		self.includePaths = copy.deepcopy(self.includePathsStd)

		
	def resolvePaths(self):
		
		expandedPaths = dict()
		
		for path in self.rootPaths:
			expandedPath = self.expandFilePath(path)
			expandedPaths[expandedPath] = self.rootPaths[path]
		self.rootPaths = expandedPaths


		expandedPaths = dict()
		for path in self.includePathsStd:
			expandedPath = self.expandFilePath(path)
			expandedPaths[expandedPath] = self.includePathsStd[path]
															# If Map to path is wildcard, copy unexpanded original path			
			if expandedPaths[expandedPath] == "*":
				expandedPaths[expandedPath] = os.path.normpath(path)
		self.includePathsStd = expandedPaths
		
		expandedPaths = dict()
		for path in self.additionalIncludePaths:
			expandedPath = self.expandFilePath(path)
			expandedPaths[expandedPath] = self.additionalIncludePaths[path]
			
			if expandedPaths[expandedPath] == "*":
				expandedPaths[expandedPath] = os.path.normpath(path)
		self.additionalIncludePaths = expandedPaths
		
		expandedPaths = dict()
		for path in self.includePaths:
			expandedPath = self.expandFilePath(path)
			expandedPaths[expandedPath] = self.includePaths[path]
															# If Map to path is wildcard, copy unexpanded original path			
			if expandedPaths[expandedPath] == "*":
				expandedPaths[expandedPath] = os.path.normpath(path)
		self.includePaths = expandedPaths

		expandedIgnorePaths = []
		for path in self.ignoreSourceFiles:
			expandedPath = self.expandFilePath(path)
			expandedIgnorePaths.append(expandedPath)
		self.ignoreSourceFiles = expandedIgnorePaths
		
		expandedIgnorePaths = []
		for path in self.ignoreDependencies:
			expandedPath = self.expandFilePath(path)
			expandedIgnorePaths.append(expandedPath)
		self.ignoreDependencies = expandedIgnorePaths
															# Expand link additional include paths		
		expandedPaths = dict()
		for path in self.additionalLibraryPaths:
			expandedPath = self.expandFilePath(path)
			expandedPaths[expandedPath] = self.additionalLibraryPaths[path]
			
			if expandedPaths[expandedPath] == "*":
				expandedPaths[expandedPath] = os.path.normpath(path)
		self.additionalLibraryPaths = expandedPaths


		if self.logFilePath != "":
			self.logFile = open(self.logFilePath, "w")

		self.output = self.expandFilePath(self.output)


		
	def isModeVisualStudio(self, config):
	
		return config.mode == "VS2005" or config.mode == "VS2012"
