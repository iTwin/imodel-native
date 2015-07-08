#!/usr/bin/python
# ---------------------------------------------------------------------------------------------------------------------------
#  $Source: BuildTools/abmake.py $
#
#  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
#
#  Author      : Lee Bull <Lee.Bull@bentley.com>
#
#  Description : Converts source file hierarchy or Microsoft Visual Studio project (2005) to Bentley Makefiles (bmake)
# ---------------------------------------------------------------------------------------------------------------------------

import sys
import os
import time
import copy

#makeFileVarSourceRoot	= "$(basedir)src"
makeFileVarSourceRoot	= "$(basedir)"
makeFileVarIncludeRoot	= "$(basedir)"
makeFileVarObjRoot	= "$(o)"


class Stats:
	numRootPaths = 0
	numSourceFiles = 0
	numDependencyFiles = 0
	
	def getStatsString(self, config, aMake):
	
		stats = ""
		stats = stats + "Root Paths       : " + str(self.numRootPaths) + "\n"
		stats = stats + "Source Files     : " + str(self.numSourceFiles) + "\n"
		stats = stats + "Dependency Files : " + str(self.numDependencyFiles) + "\n"
		stats = stats + "Warnings         : " + str(len(aMake.warnings)) + "\n"
		
		return stats

import Config

#############################################################################################################################

from xml.dom import minidom

class AMake:

	def convertVisualStudioProject(self, config, rootPathDictionary, vcProjFilePath):
	
		print "Visual Studio Conversion : " + vcProjFilePath
		print "Visual Studio Mode       : " + config.mode
				
		vcProjPath = os.path.split(vcProjFilePath)[0]
		rootPathDictionary[vcProjPath] = dict()
	
		vcProjXML = minidom.parse(vcProjFilePath)
							
		self.convertVisualStudioConfiguration(config, rootPathDictionary, vcProjFilePath, vcProjXML, config.buildName, config.buildPlatform)
		
		config.resolvePaths()
				
		self.convertVisualStudioSourceFiles(config, rootPathDictionary, vcProjFilePath, vcProjXML)


	def convertVisualStudioSourceFiles(self, config, rootPathDictionary, vcprojFilePath, vcProjXML):
		
		if config.mode == "VS2005":
			self.convertVisualStudioSourceFilesVS2005(config, rootPathDictionary, vcprojFilePath, vcProjXML, config.buildName, config.buildPlatform)

		if config.mode == "VS2012":
			self.convertVisualStudioSourceFilesVS2012(config, rootPathDictionary, vcprojFilePath, vcProjXML, config.buildName, config.buildPlatform)
					

	def convertVisualStudioSourceFilesVS2005(self, config, rootPathDictionary, vcprojFilePath, vcProjXML, configName, platformName):

		configPlatformName = configName + "|" + platformName
	
		vcProjPath = os.path.split(vcprojFilePath)[0]
		
		fileDictionary = rootPathDictionary[vcProjPath]
	
		fileList = vcProjXML.getElementsByTagName('File')
		
		sourceFileList = []
		
		for fileElement in fileList:
		
			filePath = fileElement.attributes['RelativePath'].value

			includeSourceFile = True
			
			if len(filePath) > 0:

				fileConfigurationList = fileElement.getElementsByTagName('FileConfiguration')

				for fileConfiguration in fileConfigurationList:

					configAffected = fileConfiguration.attributes['Name'].value

					if configAffected.lower() == configPlatformName.lower():

						excludedFromBuild = fileConfiguration.attributes.get("ExcludedFromBuild", None)

						if excludedFromBuild != None:

							if excludedFromBuild.value.lower() == "true":

								includeSourceFile = False

				if includeSourceFile == True:

					sourceFileList.append(filePath)
				else:
					fullFilePath = os.path.normpath(os.path.join(os.path.split(vcprojFilePath)[0], filePath))
					config.ignoreSourceFiles.append(fullFilePath.encode('ascii'))

						
		self.initializeVisualStudioSourceFiles(config, vcprojFilePath, fileDictionary, sourceFileList)



	def convertVisualStudioSourceFilesVS2012(self, config, rootPathDictionary, vcprojFilePath, vcProjXML, configName, platformName):

		configPlatformName = configName + "|" + platformName
		
		vcProjPath = os.path.split(vcprojFilePath)[0]
		
		fileDictionary = rootPathDictionary[vcProjPath]
				
		itemGroupList = vcProjXML.getElementsByTagName("ItemGroup")
		
		sourceFileList = []
		
		for item in itemGroupList:
		
			clList = item.getElementsByTagName("ClCompile")
			
			for cl in clList:

				excluded = False

				excludedConfigList = cl.getElementsByTagName("ExcludedFromBuild")

				if excludedConfigList != None:
					for excludedConfig in excludedConfigList:

						excludedToggle = excludedConfig.firstChild.data
						
						if excludedToggle.lower() == "true":	
							condition = excludedConfig.getAttribute("Condition")
							configPlatform = condition.split('==')[1]
							if(configPlatform != ""):
								configPlatform = configPlatform.strip("'")
								if configPlatform == configPlatformName:
									excluded = True

				sourceFile = cl.getAttribute("Include")
					
				if excluded == False:
					if(len(sourceFile) > 0):
						sourceFileList.append(sourceFile)
				else:
					fullFilePath = os.path.normpath(os.path.join(os.path.split(vcprojFilePath)[0], sourceFile))
					config.ignoreSourceFiles.append(fullFilePath.encode('ascii'))
					
		self.initializeVisualStudioSourceFiles(config, vcprojFilePath, fileDictionary, sourceFileList)
		

	def initializeVisualStudioSourceFiles(self, config, vcprojFilePath, fileDictionary, sourceFileList):

		for filePath in sourceFileList:
					
			fullFilePath = os.path.normpath(os.path.join(os.path.split(vcprojFilePath)[0], filePath))

			if self.isSourceFile(config, fullFilePath):
				split = os.path.splitext(fullFilePath)
				fullFilePath = split[0] + split[1].lower()
				fullFilePath = fullFilePath.encode("ascii")
				if self.isSourceFileIgnored(config, fullFilePath) == False:
					fileDictionary[fullFilePath] = None
				else:
					self.outputWarning(config, "Ignored Source File : " + fullFilePath)
		
					
	def convertVisualStudioConfiguration(self, config, rootPathDictionary, vcProjFilePath, vcProjXML, configName, platformName):
	
		if config.mode == "VS2005":
			self.convertVisualStudioConfigurationVS2005(config, rootPathDictionary, vcProjFilePath, vcProjXML, config.buildName, config.buildPlatform)
			
		if config.mode == "VS2012":
			self.convertVisualStudioConfigurationVS2012(config, rootPathDictionary, vcProjFilePath, vcProjXML, config.buildName, config.buildPlatform)
			
			
	def convertVisualStudioConfigurationVS2005(self, config, rootPathDictionary, vcProjFilePath, vcProjXML, configName, platformName):
	
		configPlatformName = configName + "|" + platformName
	
		configurations = vcProjXML.getElementsByTagName("Configuration")
		
		for configuration in configurations:
		
			projConfigName = configuration.attributes['Name'].value
			
			if projConfigName.lower() == configPlatformName.lower():
				self.parseVisualStudioConfigurationVS2005(config, configuration)

								
	def convertVisualStudioConfigurationVS2012(self, config, rootPathDictionary, vcProjFilePath, vcProjXML, configName, platformName):
	
		configPlatformName = configName + "|" + platformName
		
		itemDefinitionGroups = vcProjXML.getElementsByTagName("ItemDefinitionGroup")
		
		for itemDefinitionGroup in itemDefinitionGroups:
			condition = itemDefinitionGroup.attributes['Condition'].value
			
			configPlatform = condition.split('==')[1]
			if(configPlatform != ""):
				configPlatform = configPlatform.strip("'")
				if configPlatform == configPlatformName:
					print "Processing Visual Studio Configuration : " + configPlatform
					self.parseVisualStudioConfigurationVS2012(config, itemDefinitionGroup)
		
		return
		
	
		
	def initializeVisualStudioConfigLists(self, config, includeList, definitionsList, additionalLibraryDirectoriesList, additionalDependenciesList, ignoreLibrariesList):
			
		includes					= dict()
		additionalLibraryPaths		= dict()
		definitionsListA			= []
		additionalDependencyListA	= []
		ignoreLibrariesListA		= []
	
		for include in includeList:
			include = include.strip('"')
			include = include.encode("ascii")
			include = include.replace("(", "{");
			include = include.replace(")", "}");
			includes[include] = "*"
			
		for definition in definitionsList:
			definition = definition.encode("ascii")
			
			if definition.lower() != "%(preprocessordefinitions)":
				definitionsListA.append(definition)

		for libraryPath in additionalLibraryDirectoriesList:
			libraryPath = libraryPath.strip('"')
			libraryPath = libraryPath.encode("ascii")
			libraryPath = libraryPath.replace("(", "{");
			libraryPath = libraryPath.replace(")", "}");
			additionalLibraryPaths[libraryPath] = "*"
			
		for dependency in additionalDependenciesList:
			dependency = dependency.encode("ascii")
			if dependency.lower() != "%(additionaldependencies)":
				additionalDependencyListA.append(dependency)

		for ignore in ignoreLibrariesList:
			ignore = ignore.encode("ascii")
			ignoreLibrariesListA.append(ignore)

		config.additionalIncludePaths	= includes
		config.definitions				= definitionsListA
		config.additionalLibraryPaths	= additionalLibraryPaths
		config.additionalLibraries		= additionalDependencyListA
		config.ignoreLibraries			= ignoreLibrariesListA

		
	def parseVisualStudioConfigurationVS2005(self, config, configurationXML):
				
		includeList							= []
		definitionsList						= []
		additionalLibraryDirectoriesList	= []
		additionalDependenciesList			= []
		ignoreLibrariesList					= []
		
		tools = configurationXML.getElementsByTagName("Tool")

		for tool in tools:
			toolName = tool.attributes["Name"].value

			if toolName == "VCCLCompilerTool":
			
				additionalIncludes = tool.attributes["AdditionalIncludeDirectories"].value
				includeList = additionalIncludes.split(";")
											
				preprocessorDefinitions = tool.attributes["PreprocessorDefinitions"].value
				definitionsList = preprocessorDefinitions.split(";")
				
					
			if toolName == "VCLinkerTool":
			
				additionalLibraryDirectories = tool.attributes["AdditionalLibraryDirectories"].value
				additionalLibraryDirectoriesList = additionalLibraryDirectories.split(";")
				
				additionalDependencies = tool.attributes["AdditionalDependencies"].value
				additionalDependenciesList = additionalDependencies.split(" ")


				ignoreLibrariesNode = tool.attributes.get("IgnoreDefaultLibraryNames", None)
				if ignoreLibrariesNode != None:
					ignoreLibraries = ignoreLibrariesNode.value
					ignoreLibrariesList= ignoreLibraries.split(";")
				
		self.initializeVisualStudioConfigLists(config, includeList, definitionsList, additionalLibraryDirectoriesList, additionalDependenciesList, ignoreLibrariesList)
					
		
	def parseVisualStudioConfigurationVS2012(self, config, configurationXML):

		includeList							= []
		definitionsList						= []
		additionalLibraryDirectoriesList	= []
		additionalDependenciesList			= []
		ignoreLibrariesList					= []

		clCompile = configurationXML.getElementsByTagName("ClCompile")
				
		for cl in clCompile:
			additionalIncludes = cl.getElementsByTagName("AdditionalIncludeDirectories")
			includes = additionalIncludes[0].firstChild.data
			includeList = includes.split(";")
			
			preprocessorDefinitions = cl.getElementsByTagName("PreprocessorDefinitions")
			preprocessorDefinitions = preprocessorDefinitions[0].firstChild.data
			definitionsList = preprocessorDefinitions.split(";")
			
							
		link = configurationXML.getElementsByTagName("Link")
		
		for l in link:
			additionalLibraryDirectories = l.getElementsByTagName("AdditionalLibraryDirectories")
			if additionalLibraryDirectories != None and len(additionalLibraryDirectories) > 0:
				additionalLibraryDirectories = additionalLibraryDirectories[0].firstChild.data
				if additionalLibraryDirectories != None:
					additionalLibraryDirectoriesList = additionalLibraryDirectories.split(";")
			
			additionalDependencies = l.getElementsByTagName("AdditionalDependencies")
			if additionalDependencies != None and len(additionalDependencies) > 0:
				additionalDependencies = additionalDependencies [0].firstChild.data
				if additionalDependencies != None:
					additionalDependenciesList = additionalDependencies.split(";")

			ignoreLibraries = l.getElementsByTagName("IgnoreSpecificDefaultLibraries")

			if ignoreLibraries != None and len(ignoreLibraries) > 0:
				ignoreLibraries = ignoreLibraries[0].firstChild.data
				if ignoreLibraries != None:
					ignoreLibrariesList= ignoreLibraries.split(";")

		self.initializeVisualStudioConfigLists(config, includeList, definitionsList, additionalLibraryDirectoriesList, additionalDependenciesList, ignoreLibrariesList)
		

	def getRootPathSourceFiles(self, config, rootPathDictionary, rootPath):
										# Create file->dependencyFileList dictionary
		rootPathDictionary[rootPath] = dict()

		for root, _, files in os.walk(rootPath):
			for f in files:
				fullPath = os.path.normpath(os.path.join(root, f))
				if self.isSourceFile(config, fullPath):
					split = os.path.splitext(fullPath)
					fullPath = split[0] + split[1].lower()
					if self.isSourceFileIgnored(config, fullPath) == False:
						(rootPathDictionary[rootPath])[fullPath] = None
					else:
						self.outputWarning(config, "Ignored Source File : " + fullPath)
					
	def getImmediateFileDependencies(self, config, filePath):

		dependencyList = []

		if len(filePath) == 0:
			return dependencyList
		
		fileIn = open(filePath)
		document = fileIn.readlines()

		for line in document:
			includeStart = line.find("#include", 0)
			if includeStart != -1:
				commentStart = line.find("//", 0)
				if commentStart == -1 or includeStart < commentStart:
					lineStripped = line
					includePath = self.extractIncludeFilePath(config, line)
					
					fullIncludePath = self.resolveFullIncludePath(config, filePath, includePath)
					
					if fullIncludePath != "" and self.isSourceFile(config, fullIncludePath) == False:
						if self.isDependencyIgnored(config, fullIncludePath) == False:
							dependencyList.append(fullIncludePath)
						
		fileIn.close()
		
		return dependencyList
					

	def extractDelimitedStr(self, config, str, s1, s2):
		resultString = ""
		start = str.find(s1)
		if start != -1:
			end = str.find(s2, start + 1)
			if end != -1:
				resultString = str[start + 1 : end]

		return resultString

	def extractIncludeFilePath(self, config, str):
		r = self.extractDelimitedStr(config, str, '"', '"')
		if len(r) > 0:
			return r

		r = self.extractDelimitedStr(config, str, '<', '>')

		return r


	def getDependencyMakeString(self, config, dependencies, file):

		makeString = file + " : "

		for dependency in dependencies[file]:
			makeString = makeString + " " + dependency

		return makeString

	def getSectionString(self, config, message):
		s = ""
		s = s + "# --------------------------------------------------------------------------------------\n"
		s = s + message + "\n"
		s = s + "# --------------------------------------------------------------------------------------\n"
		s = s + "\n"
		return s

	def writeSectionHeader(self, config, out, message):

		sectionStr = self.getSectionString(config, message)

		out.write(sectionStr)

		if config.logFile != None:
			config.logFile.write(sectionStr + "\n")
			config.logFile.flush()

	def outputWarning(self, config, str, defer = True):
	
		warningStr = "Warning : " + str
		
		self.warnings.append(warningStr)
		
		if defer == False:
			if config.logFile != None:
				config.logFile.write(warningStr + "\n")
				config.logFile.flush()

			print warningStr

		return
		
	def outputAllWarnings(self, config):
	
		print "Warnings : " + str(len(self.warnings)) + "\n"

		for warning in self.warnings:
			if config.logFile != None:
				config.logFile.write(warning + "\n")
				config.logFile.flush()

			print warning		


	def getFileTargetPath(self, config, rootPath, filePath):
	
		targetPath = filePath
		
		if self.isSourceFile(config, targetPath):
			extension = os.path.splitext(targetPath)[1]
			extensionLower = extension.lower()
			
			targetPath = targetPath.replace(extension, config.sourceFileTypes[extensionLower])
			
			targetPath = makeFileVarObjRoot + os.path.basename(targetPath)
#			targetPath = self.getPathRelativeToEnvVar(config, rootPath, targetPath, makeFileVarObjRoot)

		return targetPath


	def resolveFullIncludePath(self, config, filePath, includePath):
	
										# If filepath is full and file exists, use it
		if os.path.isfile(includePath):
			return includePath
										# Get file's directory
		fileDirPath = os.path.dirname(filePath)
										# Get include file relative to main file's path
		pathTest = os.path.normpath(os.path.join(fileDirPath, includePath))
										# If include file exists relative to main file, return it's full path
		if os.path.isfile(pathTest):
			return pathTest
										# Try to resolve include using set of include paths.
										# Find first include file
		for path in config.includePaths:
				
			pathTest = os.path.normpath(os.path.join(path, includePath))

			if os.path.isfile(pathTest):
				return pathTest

		self.outputWarning(config, "Include not resolved : " + filePath + " --> " + includePath)
		return ""


	def getPathRelativeToEnvVar(self, config, rootPath, filePath, envVar):

		path = filePath

		
		commonPathStart = os.path.commonprefix([rootPath, filePath])
		
		print ">>>>> " + path + ", " + commonPathStart + ", " + rootPath + "\n"
		
										# If root path is substring of given file path
#		if commonPathStart == rootPath:
		if len(commonPathStart) > 0:
			relativePart = path.replace(commonPathStart, "")
			relativePart = relativePart.lstrip(os.path.sep)
			path = os.path.join(envVar, relativePart)
			
#			print "=========================================================="
#			print commonPathStart
#			print rootPath
#			print filePath
#			print envVar
#			print path
#			print "=========================================================="

		return path

	def addQuotes(self, config, filePath):
		path = "\"" + filePath + "\""
		return path

	def getFullDependencyPath(self, config, rootPath, targetPath, filePath, dependencyPath):

		path = self.resolveFullIncludePath(config, filePath, dependencyPath)

		envVarPath = self.getPathRelativeToEnvVar(config, rootPath, path, makeFileVarIncludeRoot)

		return envVarPath


	def writeMakefileTargetDependencies(self, config, out, rootPath, targetPath, filePath, fileDictionary):

		dependentFile = self.getPathRelativeToEnvVar(config, rootPath, filePath, makeFileVarSourceRoot)

		dependencyStr = targetPath + " : " + dependentFile

		if config.outputMinimalDependencies == False:
			for f in fileDictionary[filePath]:
				fullDependencyPath = self.getFullDependencyPath(config, rootPath, targetPath, filePath, f)
				dependencyStr = dependencyStr + " " + fullDependencyPath
		else:
			if config.outputMultiCompileDepends == True:
				dependencyStr = dependencyStr + " " + "${MultiCompileDepends}"

				
		out.write(dependencyStr + "\n")


	def getSourceFiles(self, config, rootPathDictionary):
	
		for rootPath in config.rootPaths:
			self.getRootPathSourceFiles(config, rootPathDictionary, rootPath)

			
	def isSourceFile(self, config, filePath):
	
		extension = os.path.splitext(filePath.lower())[1]

		for suffix in config.sourceFileTypes:
			if suffix == extension:
				return True
		
		return False


	def isStandardLibrary(self, config, library):

		for lib in config.standardLibraries:
			if lib == library:
				print "Found " + lib + "\n"
				return True

		return False

			
	def getStats(self, config, rootPathDictionary):

		stats = Stats()	
		stats.numRootPaths = 0
		stats.numSourceFiles = 0
		stats.numDependencyFiles = 0
		
		for rootPath in rootPathDictionary:
			for file in rootPathDictionary[rootPath]:
				if self.isSourceFile(config, file):
					stats.numSourceFiles = stats.numSourceFiles + 1
				else:
					stats.numDependencyFiles = stats.numDependencyFiles + 1

		stats.numRootPaths = len(rootPathDictionary)

		return stats		

         		
	def isSourceFileIgnored(self, config, filePath):

		for ignorePath in config.ignoreSourceFiles:
			
			if filePath.find(ignorePath) != -1:
				return True
								
		return False
		
		
	def isDependencyIgnored(self, config, filePath):
	
		for ignorePath in config.ignoreDependencies:
			if filePath.find(ignorePath) != -1:
#				print "==== Ignoring path : " + filePath + " due to " + ignorePath + "\n"
				return True
			
		return False
		

	def getImmediateDependencies(self, config, rootPathDictionary):

		for rootPath in rootPathDictionary:
			fileDictionary = rootPathDictionary[rootPath]
			for filePath in fileDictionary:
				fileDictionary[filePath] = self.getImmediateFileDependencies(config, filePath)

			
	
	def getFullDependencies(self, config, rootPathDictionary):
	
		for rootPath in rootPathDictionary:

			fileDictionary = rootPathDictionary[rootPath]
			
			filePaths = []
			for filePath in fileDictionary:
				filePaths.append(filePath)
			
			for filePath in filePaths:
				fullDependencyList = []
				self.getFullDependenciesRec(config, fileDictionary, filePath, fullDependencyList)
				print filePath
				fileDictionary[filePath] = fullDependencyList
				
				
	def getFullDependenciesRec(self, config, fileDictionary, filePath, fullDependencyList, depth = 1):
																# Get file's immediate dependency list	
		immediateDependencyList = fileDictionary.get(filePath, None)
																# If immediate dependency list does not exist, generate it now
		if immediateDependencyList == None:
			fileDictionary[filePath] = self.getImmediateFileDependencies(config, filePath)
			immediateDependencyList = fileDictionary[filePath]
			
																# For each of file's immediate dependencies			
		for dependency in immediateDependencyList:
																# If not already processed (in depth-wise include order)
			if dependency not in fullDependencyList:
																# Add dependency to full dependency list
				fullDependencyList.append(dependency)
																# Process depdency's own immediate dependencies (depth first)
				self.getFullDependenciesRec(config, fileDictionary, dependency, fullDependencyList, depth + 1)

				
	def getUnderscoreString(self, s):
	
		return s.replace(" ", "_")

		
	def writeMakefile(self, config, rootPathDictionary):

		out = open(config.output, "w")
		
		print "Writing Makefile : " + config.output + "\n"
		
		t = time.localtime(time.time())
		timeStr = str(t.tm_mday) + "/" + str(t.tm_mon) + "/" + str(t.tm_year) + " " + str(t.tm_hour) + ":" + str(t.tm_min) + ":" + str(t.tm_sec)
		
		headerStr = "# (c) 2015 Bentley Systems (UK) Ltd. Auto Generated Makefile (abMake)\n\n"
		headerStr = headerStr + "# " + config.output + " " + timeStr + "\n"
		
		if config.isModeVisualStudio(config):
				if config.platformSpecific == True:
					out.write(config.vsConfigPrefix + self.getUnderscoreString(config.buildName) + "=1\n")
					out.write(config.vsConfigPrefix + config.buildPlatform + "=1\n")
						
		self.writeSectionHeader(config, out, headerStr)

		if config.outputMKE:
			self.writeMacros(config, out)
			
			if config.outputPrecompiledHeaders == True:
				self.writePrecompiledHeaders(config, out)
				
		
		if config.outputDefinitions == True:
			self.writeMakefileDefinitions(config, out)

		print "*********************************************************\n"
		print "Output Include Paths : " + str(config.outputIncludePaths) + "\n"
		print "*********************************************************\n"
		
		if config.outputIncludePaths == True:
			self.writeMakefileIncludePaths(config, out)

		if config.outputTargets == True:		
			self.writeMakefileTargets(config, rootPathDictionary, out)
		
		if config.outputObjects == True:
			self.writeMakefileObjects(config, rootPathDictionary, out)
		
		if config.outputLibraryPaths == True:
			self.writeMakefileLibraryPaths(config, out)

		if config.outputStandardLibraries == True or config.outputAdditionalLibraries == True:
			self.writeMakefileLibraries(config, out)

		if config.outputIgnoreLibraries == True:
			self.writeMakefileIgnoreLibraries(config, out)
			
		if config.outputMKE == True:
			self.writeBuild(config, out)

		out.close()

		
	def writeMacros(self, config, out):

		headerStr = "# Macros"
		self.writeSectionHeader(config, out, headerStr)
				
		out.write("PolicyFile = " + config.defaultMKEPolicy + "\n")
		out.write("SolutionPolicyMki = $(PolicyFile)\n")
		out.write("%include mdl.mki\n")
		out.write("\n\n")
		
		out.write("VendorAPIDir    = $(BuildContext)VendorAPI/$(appName)/\n\n")

		out.write("\n\n")
			
		if config.outputMultiCompileDepends:
		
			out.write("appSrc              = $(_MakeFilePath)\n")
			out.write("appPublicApi        = $(appSrc)PublicAPI/$(appName)/\n")
			out.write("MultiCompileDepends = $(_MakeFileSpec)")

			if config.outputPrecompiledHeaders == True:
				out.write("$(appSrc)/Include/$(appName)Internal.h\n")
				
			out.write("\n")
			
		out.write("always:\n")
		out.write("\t!~@mkdir $(o)\n\n")

		
	def writePrecompiledHeaders(self, config, out):
	
		self.writeSectionHeader(config, out, "# Precompiled Headers")
		
		out.write("PchCompiland        = $(basedir)/src/$(appName)Internal.cpp\n")
		out.write("PchOutputDir        = $(o)\n")
		out.write("PchExtraOptions     = -Zm160\n")
		out.write("PchExplicitDepends  = $(MultiCompileDepends)\n")
		out.write("\n\n")

		out.write("%if defined (winNT) && $(BUILD_TOOLSET) == \"GCC\"\n")
		out.write("\tGCC_NO_PRE_COMPILED_HEADER = 1\n")
		out.write("%endif")
		out.write("\n\n")

		out.write("%include $(SharedMki)PreCompileHeader.mki\n")
		out.write("CCPchOpts = $(UsePrecompiledHeaderOptions)\n")
		out.write("#CCPchOpts + -showIncludes\n")
		out.write("CPchOpts  = $(UsePrecompiledHeaderOptions)\n")
		
		out.write("\n\n")
		out.write("#DisableMultiCompile=1\n")
		
		out.write("\n\n")

		
	def writeBuild(self, config, out):
	
		self.writeSectionHeader(config, out, "# Build")
		
		if config.createStaticLibraries == True:
			out.write("CREATE_STATIC_LIBRARIES     = 1\n\n")
		
		out.write("DLM_NAME                    = $(appName)\n")
		out.write("DLM_OBJECT_FILES            = $(MultiCompileObjectList)\n")
		out.write("DLM_OBJECT_DEST             = $(o)\n")
		
		if config.outputPrecompiledHeaders == True:
			out.write("DLM_OBJECT_PCH              = $(o)$(appName)Internal$(oext)\n")
			
		out.write("DLM_DEST                    = $(o)\n")
		out.write("DLM_NOMSBUILTINS            = 1\n")
		out.write("DLM_EXPORT_DEST             = $(o)\n")
		out.write("DLM_NOENTRY                 = 1\n")
		out.write("DLM_NO_DLS                  = 1\n")
		out.write("DLM_NO_DEF                  = 1\n")
		out.write("DLM_CONTEXT_LOCATION        = $(BuildContext)Delivery/\n")
		out.write("DLM_LIB_CONTEXT_LOCATION    = $(BuildContext)Delivery/\n")
		out.write("DLM_CREATE_LIB_CONTEXT_LINK = 1\n")
		out.write("\n\n")
		
		# out.write("DLM_SPECIAL_LINKOPT     + -def:$(baseDir)api.def\n")
		# out.write("DLM_SPECIAL_LINKOPT     + -NODEFAULTLIB:LIBCMT\n")

		out.write("%include $(sharedMki)linkLibrary.mki\n\n")
		


	def writeMakefileTargets(self, config, rootPathDictionary, out):

		self.writeSectionHeader(config, out, "# Target Dependencies")
		
		self.targetPaths = []

		if config.outputMultiCompileDepends == True:
			out.write("%include MultiCppCompileRule.mki\n\n\n")
				
		for rootPath in rootPathDictionary:

			fileDictionary = rootPathDictionary[rootPath]
			
			for filePath in fileDictionary:
				if self.isSourceFile(config, filePath):
					dependencyList = fileDictionary[filePath]
					targetPath = self.getFileTargetPath(config, rootPath, filePath)
					self.targetPaths.append(targetPath)
					self.writeMakefileTargetDependencies(config, out, rootPath, targetPath, filePath, fileDictionary)
					out.write("\n")

		if config.outputMultiCompileDepends == True:
			out.write("\n%include MultiCppCompileGo.mki\n\n")
		
		if config.outputPrecompiledHeaders == True:
			out.write("%undef CCPchOpts\n")
			out.write("%undef CPchOpts\n")
			
		out.write("\n\n")
			
	def getBuildPlatformOutput(self, config):
		if config.platformSpecific == True:
			return config.buildPlatform
			
		return ""
					
	def writeMakefileIncludePaths(self, config, out):
	
		self.writeSectionHeader(config, out, "# Include Paths : " + config.buildName + " " + self.getBuildPlatformOutput(config))
		
		for path in config.additionalIncludePaths:
			out.write('cIncs +% -I"' + config.additionalIncludePaths[path] + '"\n')

		out.write("\n\n\n")			

			
	def writeMakefileLibraryPaths(self, config, out):
	
		self.writeSectionHeader(config, out, "# Library Paths : " + config.buildName + " " + self.getBuildPlatformOutput(config))

		if config.isModeVisualStudio(config):
			if config.platformSpecific == True:
				out.write("%if " + config.vsConfigPrefix + self.getUnderscoreString(config.buildName) + "\n")
				out.write("%if " + config.vsConfigPrefix + config.buildPlatform + "\n")
		
		for path in config.additionalLibraryPaths:
			out.write('CLinkOpts + -LIBPATH:"' + config.additionalLibraryPaths[path] + '"\n')	
			
		if config.isModeVisualStudio(config):
			
			if config.platformSpecific == True:
				out.write("%endif\n")
				out.write("%endif\n")

		out.write("\n\n")			


	def writeMakefileDefinitions(self, config, out):
		
		self.writeSectionHeader(config, out, "# Preprocessor Definitions : " + config.buildName + " " + self.getBuildPlatformOutput(config))
		
		if config.isModeVisualStudio(config):
			
			if config.platformSpecific == True:
				out.write("%if " + config.vsConfigPrefix + self.getUnderscoreString(config.buildName) + "\n")
				out.write("%if " + config.vsConfigPrefix + config.buildPlatform + "\n")
		
		for definition in config.definitions:
			out.write('    CCompOpts + -D"' + definition + '"' + "\n")

		if config.isModeVisualStudio(config):
			
			if config.platformSpecific == True:
				out.write("%endif\n")
				out.write("%endif\n")

		out.write("\n\n")				


	def writeMakefileObjects(self, config, rootPathDictionary, out):

		self.writeSectionHeader(config, out, "# Target Objects")

		out.write("dlmObjs = ")

		for targetPath in self.targetPaths:
			out.write("\\")
			out.write("\n")
			out.write("    ")
			out.write(targetPath)

		out.write("\n\n")


	def writeMakefileLibraries(self, config, out):
	
		if config.outputStandardLibraries == False and config.outputAdditionalLibraries == False:
			return
	
		self.writeSectionHeader(config, out, "# Libraries : " + config.buildName + " " + self.getBuildPlatformOutput(config))

		if config.isModeVisualStudio(config):
			
			if config.platformSpecific == True:
				out.write("%if " + config.vsConfigPrefix + self.getUnderscoreString(config.buildName) + "\n")
				out.write("%if " + config.vsConfigPrefix + config.buildPlatform + "\n")

		if config.outputDLMFormat == False:
			out.write("dlmLibs = ")
		else:
			out.write("LINKER_LIBRARIES = ")

		if config.outputStandardLibraries == True:
		
			for library in config.standardLibraries:
				out.write("\\")
				out.write("\n")
				out.write("    ")
				out.write(library)

		if config.outputAdditionalLibraries == True:
		
			for library in config.additionalLibraries:

				if self.isStandardLibrary(config, library) == False:

					out.write("\\")
					out.write("\n")
					out.write("    ")
					if config.createBuildContextLibraries == True:
						library = library

					out.write(library)
			
		out.write("\n")
						
		if config.isModeVisualStudio(config):
			
			if config.platformSpecific == True:
				out.write("%endif\n")
				out.write("%endif\n")
			
		out.write("\n\n")


	def writeMakefileIgnoreLibraries(self, config, out):
	
		self.writeSectionHeader(config, out, "# Ignore Libraries : " + config.buildName + " " + self.getBuildPlatformOutput(config))
				
		for library in config.ignoreLibraries:
			libraryStripped = library.strip()
			if libraryStripped != '':
				out.write("CLinkOpts + -nodefaultlib:" + libraryStripped + "\n")

		out.write("\n\n")
		

	def parseArguments(self, config):

		for arg in sys.argv:
			if arg[0] == '-':
				expr = arg.split('=')
				if(len(expr) == 2):
					expr[0] = expr[0].strip("-")
					
					if expr[1] == "True":
						expr[1] = True
						
					setattr(config, expr[0], expr[1])
					
					print "abmake param : " + str(expr[0]) + " = " + str(expr[1])
					
				else:
				
					print "Bad Argument : " + arg + "\n"
				

	def initializeGenerate(self, config):
					
										# Parse arguments pre configuration to establish base overrides (e.g. configuration file to use)
		self.parseArguments(config)
										# Read configuraiton file
		config.readConfig(config.configFile)
										# Parse arguments post configuration to establish main overrides
		self.parseArguments(config)
										# Initialize configuration for use	
		config.initialize()
										# Clear warnings	
		self.warnings = []


	def getVisualStudioMode(self, config, filePath):

		if filePath != "":
		
			filePathLower = os.path.splitext(filePath)[1].lower()
		
			if filePathLower == ".vcproj":
				config.mode = "VS2005"
				return True
			
			if filePathLower == ".vcxproj":
				config.mode = "VS2012"
				return True
				
			return False
			

	def generate(self, config):
										# Parse parameters and initialize configuration	
		self.initializeGenerate(config)
		
										# Create rootPath->rootPathSourceFiles dictionary
		rootPathDictionary = dict()
				
										# If converting Visual Studio project, convert now								
		if self.getVisualStudioMode(config, config.projectSourceFile):
			self.convertVisualStudioProject(config, rootPathDictionary, config.projectSourceFile)
				
		config.resolvePaths()
										# Combine additional include with standard includes
		config.includePaths.update(config.additionalIncludePaths)
										# Combine standard libs with additional libs		
		config.libraries = config.standardLibraries + config.additionalLibraries
										# Print out configuration				
		print self.getSectionString(config, config.getConfigString())
									
										# Add filePath->dependencyPath dictionary entry
		if config.mode == "File":
			config.includePaths.update(config.includePathsLocal)
			self.getSourceFiles(config, rootPathDictionary)
					
		self.getFullDependencies(config, rootPathDictionary)
		
		print "\n"
		self.outputAllWarnings(config)
				
		stats = self.getStats(config, rootPathDictionary)
		statsString = self.getSectionString(config, stats.getStatsString(config, self))
		print statsString
				
		
		self.writeMakefile(config, rootPathDictionary)
					

print "ABMake Start..."
sys.stdout.flush()

aMake = AMake()
config = Config.Config()
aMake.generate(config)

print "ABMake End..."
sys.stdout.flush()
