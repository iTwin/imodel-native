#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import filecmp, fnmatch, glob, json, os, re, sys, threading, time, xml

from . import buildpaths, utils, featureaspects, cmdutil, compat, globalvars, symlinks, targetplatform, translationkit, installerelements, nugetpkg, versionutils, languagesettings

CFG_FILE_NAME               = "applicationload.cfg"
LOG_FILE_SUFFIX             = "_build.log"

# Part types
PART_TYPE_Part          = 0
PART_TYPE_Product       = 1
PART_TYPE_NuGetProduct  = 2
PART_TYPE_MultiPlatform = 3
PART_TYPE_UPackProduct  = 4
PART_TYPE_Package       = 5
PART_TYPE_Any           = 6
PART_TYPE_ToolPart      = 7
PART_TYPE_ToolPackage   = 8
PartTypeNames = ["Part", "Product", "NuGetProduct", "MultiPlatform", "UpackProduct", "Package", "Any", "ToolPart", "ToolPackage"]

ExcludeLibTypeDynamic = "Dynamic"
ExcludeLibTypeStatic = "Static"
ExcludeLibTypes = [ExcludeLibTypeDynamic, ExcludeLibTypeStatic]

PACKAGE_TYPE_Upack      = 'upack'
PackageTypes = [PACKAGE_TYPE_Upack]

# Cache the XML doms as they are read to avoid re-parsing the XML part files
g_currDoms = {}
g_partFileNames = set() # Just to check for dups

g_partWasExcludedAnnounced  = { }

g_isHandlingDotTranskitFiles = False

g_bindingLock = threading.Lock()

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getTargetPlatform():
    return globalvars.programOptions.platform

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def splitPartDescriptor(partDescriptor):
    if partDescriptor == None:
        return (None, None)

    descriptorMatch = re.match ("(.*:)?(.*)", partDescriptor)

    if descriptorMatch == None:
        return (None, None)

    partFile = (descriptorMatch.group (1) or "").strip().rstrip(":")
    partName = descriptorMatch.group (2).strip()

    if partFile == "":
        partFile = None
    elif partName == "":
        partName = None

    return (partFile, partName)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class ProvenanceError (utils.PartBuildError):
    def __init__(self, message, repo, first, second, trace=None):
        self.errmessage = message
        self.m_repo = repo
        self.m_first = first
        self.m_second = second
        utils.PartBuildError.__init__(self, message, part=None, domNode=None, trace=trace)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
class ProvenanceChanged (ProvenanceError):
    # Not really an error; it just means that the source needs to be rebuilt.
    def __init__(self, message, repo, first, second):
        ProvenanceError.__init__(self, message, repo, first, second)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class ResourceFileInfo (object):
    def __init__ (self, filePath, language):
        self.m_filePath = filePath
        self.m_language = language

#-------------------------------------------------------------------------------------------
# PartInfo describes what will be used to create a Part or a Product
# bsiclass
#-------------------------------------------------------------------------------------------
class PartInfo (object):
    def __init__(self, partName, repo, relativePartFile, partType, platform, isStatic, caseResolverDict):
        self.m_name = partName
        self.m_repo = repo
        self.m_static = isStatic
        self.m_platform = platform
        self.m_relativePartFile = relativePartFile
        self.m_file = self.ResolveFile (repo, self.m_relativePartFile, caseResolverDict) # Starts out as the source location
        self.m_partType = partType
        self.m_partFileDir, partFileBase = os.path.split (self.m_file)
        self.m_buildContext = partFileBase[:partFileBase.find(".")]
        self.m_additionalTranskitLkgs = None
        self.m_bindToDirectory = None
        self.m_pkgType = None     # Could be gotten from pkgSource if there's ever a second type.
        self.m_requiresBuild = True if self.m_partType == PART_TYPE_Package or self.m_partType == PART_TYPE_ToolPackage else False  # Even if partStrategy is build=Never, these must "build" since there are no LKGs.
        self.m_includeToolInTranskit = False

        # cannot use '.' in the partfile name.
        if partFileBase.lower().find(".") != partFileBase.lower().find(".partfile.xml"):
            raise utils.BuildError ("Cannot use '.' in partfile name. 'PARTFILENAME.PartFile.xml':  '{0}'.".format (partFileBase))

        self.m_partStrategy = globalvars.buildStrategy.FindPartStrategy (self.m_buildContext, self.m_name)

        # If it's coming from LKGs based on the strategy mark it here.
        if self.m_requiresBuild:
            # By decision, Packages never come from LKGs and must run the build script.
            self.m_requiresLKG = False
            self.m_requiresSource = True
        else:
            self.m_requiresLKG    = self.m_partStrategy.RequiresLKG ()
            self.m_requiresSource = self.m_partStrategy.RequiresSource ()

        # Since the partfile comes from the parent if it is not already defined, this will adjust it so that
        #   the partfile is correct for a single part coming from LKGs but subparts coming from source.
        self.m_fromLKG = False
        if self.m_requiresLKG:
            self.SwitchToLKG (caseResolverDict)
        else:
            self.SwitchFromLKG ()

        self.m_nodeIdentifier = "*".join([self.m_name, self.m_buildContext, str(self.m_partType), self.m_platform.GetXmlName(), self.GetStaticString(short=True)]).lower()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetStaticString (self, short=False):
        if short:
            return "stat" if self.m_static else "dyn"
        else:
            return "static" if self.m_static else "dynamic"

    #--------------------------------------------------------------------------------
    # @bsimethod
    #--------------------------------------------------------------------------------
    def GetFileRelativeToRepo (self):
        repoDir = os.path.expandvars(self.m_repo.GetLocalDir())
        repoFile = os.path.expandvars(self.m_file)
        if repoDir.lower() in repoFile.lower():
            repl = repoFile[len (repoDir)+1:]
        else:
            repl = repoFile
        return repl[:-len(".partfile.xml")]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def IsExcludedByStrategy (self):
        return self.m_partStrategy.m_exclude

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetShortPartDescriptor (self):
        return buildpaths.getShortPartDescriptor (self.m_file, self.m_name)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetNodeIdentifier (self):
        return self.m_nodeIdentifier

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def UpdatePackagePath (self):
        if self.m_partType == PART_TYPE_Package or self.m_partType == PART_TYPE_ToolPackage:  # Partfile path gets updated when correct version is pulled.
            self.m_file = self.ResolveFile (self.m_repo, self.m_relativePartFile, None)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetLkgSource (self):
        return globalvars.buildStrategy.FindLKGSource (self.m_partStrategy.m_LKGSource, self)

    #--------------------------------------------------------------------------------
    # @bsimethod
    #--------------------------------------------------------------------------------
    def GetPartStrategy (self):
        # This used to be looked up often. With the assertion that this does NOT change during build time, I have changed it
        # to be a member variable. If that assertion ever changes then we should revert back to globalvars.buildStrategy.FindPartStrategy () here.
        return self.m_partStrategy

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Matches (self, part):
        if None == part:
            return False
        return (part.m_info.m_file==self.m_file) and (part.m_info.m_name==self.m_name)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __str__(self):
        return "Part {0}<{1}> ({2}, PartFile='{3}', PartType='{4}' Platform='{5}' {6})".format \
            (self.m_buildContext, self.m_name, self.m_repo, self.m_file, self.m_partType, self.m_platform, self.GetStaticString())

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def _ResolvePartFile (self, partFile, caseResolverDict):
        result = utils.GetExactCaseFilePath(partFile)
        # Fails when coming from LKGs, but won't create those context dirs anyway so lowercase is cool.
        partFile = result[0] if result else partFile
        if caseResolverDict != None:
            caseResolverDict[partFile.lower()] = partFile
            utils.showInfoMsg("\nPosixDebug: _ResolvePartFile added {0} to caseResolverDict \n{1}\n".format(partFile, json.dumps(caseResolverDict, indent=4, separators=(',', ': '))), utils.INFO_LEVEL_TemporaryDebugging)
        return partFile

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ResolveFile (self, repo, relativePartFile, caseResolverDict):
        # Need to get canonical case.  Use whatever matches the actual PartFile.
        # This is used later to create the BuildContext directories so want to be consistent.
        partFile = repo.GetPartFile(relativePartFile)
        if caseResolverDict and partFile.lower() in caseResolverDict:
            partFile = caseResolverDict[partFile.lower()]
            utils.showInfoMsg("\nPosixDebug: _ResolveFile partFile {0} WAS found in caseResolverDict:\n{1}\n".format(partFile, json.dumps(caseResolverDict, indent=4, separators=(',', ': '))), utils.INFO_LEVEL_TemporaryDebugging)
        else:
            partFile = self._ResolvePartFile (partFile, caseResolverDict)
            utils.showInfoMsg("\nPosixDebug: _ResolveFile partFile {0} NOT found in caseResolverDict:\n{1}\n".format(partFile, json.dumps(caseResolverDict, indent=4, separators=(',', ': '))), utils.INFO_LEVEL_TemporaryDebugging)
        return partFile

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def SwitchToLKG (self, caseResolverDict):
        self.m_fromLKG = True
        _, subName = os.path.split (self.m_file)
        staticString = 'static' if self.m_static else ''
        # Get the case correct; fix up Context and partFileDir as well
        partFile = os.path.join (buildpaths.getLKGRoot(self.m_platform), staticString, self.m_buildContext, subName)
        if caseResolverDict and partFile.lower() in caseResolverDict:
            self.m_file = caseResolverDict[partFile.lower()]
            utils.showInfoMsg("\nPosixDebug: SwitchToLKG m_file {0} WAS found in caseResolverDict:\n{1}\n".format(self.m_file, json.dumps(caseResolverDict, indent=4, separators=(',', ': '))), utils.INFO_LEVEL_TemporaryDebugging)
        else:
            self.m_file = self._ResolvePartFile (partFile, caseResolverDict)
        partFileBase = os.path.basename (self.m_file)
        self.m_buildContext = partFileBase[:partFileBase.find(".")]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def SwitchFromLKG (self):
        self.m_fromLKG = False
        self.m_file = self.m_repo.GetPartFile(self.m_relativePartFile)
        utils.showInfoMsg("\nPosixDebug: SwitchFromLKG m_file {0}\n".format(self.m_file), utils.INFO_LEVEL_TemporaryDebugging)

#--------------------------------------------------------------------------------
# @bsiclass
#--------------------------------------------------------------------------------
class TranskitBinding (object):
    #--------------------------------------------------------------------------------
    # @bsimethod
    #--------------------------------------------------------------------------------
    def __init__(self, parentBinding=None):
        self.m_baseName = ""
        self.m_sourceDir = ""
        self.m_includeDir = ""
        self.m_includeDirWithoutRename = ""
        self.m_deliveryDir = ""
        self.m_parentBinding = parentBinding

    #--------------------------------------------------------------------------------
    # @bsimethod
    #--------------------------------------------------------------------------------
    def FromXml (self, part, transkitDomNode):
        self.m_baseName    = transkitDomNode.getAttribute("TkName")
        self.m_sourceDir   = transkitDomNode.getAttribute("SourceDirectory")
        if 'BBPW' in os.environ: # DMS speciality code until they update
            self.m_includeDir  = transkitDomNode.getAttribute("IncludeDirectory")
        self.m_includeDirWithoutRename  = transkitDomNode.getAttribute("IncludeDirectoryWithoutRename")

        if "" != self.m_includeDirWithoutRename:
            # Make sure this is valid - need the last name of a dir.
            check = part.GetTranskitIncludeDirDirect (self.m_includeDirWithoutRename)
            if "" == check:
                raise utils.PartBuildError ("IncludeDirectoryWithoutRename='{0}' must be a directory".format (self.m_includeDirWithoutRename), part, transkitDomNode)

        self.m_deliveryDir = transkitDomNode.getAttribute("DeliveryDirectory")    ### Can this be removed??? It isn't used.

        if "" == self.m_sourceDir:
            raise utils.PartBuildError ("No SourceDirectory for TransKit Binding Statement", part, transkitDomNode)

#-------------------------------------------------------------------------------------------
# PartBindings are the output of a Part and are used to combine Parts (and Products) to their SubParts
# bsiclass
#-------------------------------------------------------------------------------------------
class PartBinding (object):
    def __init__(self, val, target, dom, defaultDir, part, defaultRequired, bindToTranskit=False, bindToWixBuild=False, docItem=None, sourceDirectory=None, upack=None, nuget=None):
        self.m_apiNumber = part.m_apiNumber
        self.m_val        = utils.resolveBindingMacros (val, part.GetPlatform(), part, forceUseApiNumber=False)
        # secondary is used with the ApiNumber to double-bind assemblies (once with Api Number, once without)
        self.m_secondary  = None
        if "Assemblies" == defaultDir and part.m_apiNumber and globalvars.API_NUMBER in val:
            self.m_secondary = utils.resolveBindingMacros (val, part.GetPlatform(), part, forceUseApiNumber=False, ignoreApiNumber=True).replace (globalvars.API_NUMBER_MACRO, '')
        self.m_target     = target
        self.m_alreadyWarned = False
        self.m_required   = utils.getOptionalBoolAttr ("Required", defaultRequired, "'Required' attribute of Binding {0}".format (val), dom)
        self.m_addToTranskitTools = utils.getOptionalBoolAttr ("AddToTranskitTools", False, "'AddToTranskitTools' attribute of Binding {0}".format (val), dom)
        self.m_useSymLink = utils.getOptionalBoolAttr ("UseSymLink", True, "'UseSymLink' attribute of Binding {0}".format (val), dom)
        self.m_symlinkContents = utils.getOptionalBoolAttr ("SymlinkContents", False, "'SymlinkContents' attribute of Binding {0}".format (val), dom)
        self.m_installerGUID    = dom.getAttribute ("Guid")
        self.m_extractReg       = dom.getAttribute ("ExtractReg")
        self.m_extractRegOnlyProducts = [productName.lower() for productName in dom.getAttribute ("ExtractRegOnlyProducts").split(',')] if dom.hasAttribute ("ExtractRegOnlyProducts") else None
        self.m_bindToTranskit = bindToTranskit
        self.m_bindToWixBuild = bindToWixBuild
        self.m_boundToNugetPkg  = None   
        self.m_class            = None
        self.m_docItem          = docItem
        self.m_sourceDirectory  = sourceDirectory
        self.m_upackName        = upack
        self.m_nugetName        = nuget
        self.m_defaultDir       = defaultDir if defaultDir == "PublicAPI" else ""

        ifNotPresent = dom.getAttribute ("IfNotPresent")
        if "" == ifNotPresent:
            self.m_ifNotPresent = 0
        else:
            self.m_ifNotPresent = utils.resolveOption (ifNotPresent, ["stop", "warn", "continue"], "'IfNotPresent' attribute of Binding {0}".format(val))

        productDirectory = dom.getAttribute ("ProductDirectoryName")

        if "" == productDirectory:
            productDirectory = defaultDir
        self.m_productDirectoryNameForErrors = productDirectory  # Only used in error display
        self.m_productDirectory = productDirectory.lower()
        self.m_subDir = dom.getAttribute("ProductSubDirectory")

        bindIntoLanguagePackWix = dom.getAttribute ("BindIntoLanguagePackWix")
        self.m_bindIntoLanguagePackWix = "true" == bindIntoLanguagePackWix.lower()

        self.m_featureAspect = None
        self.m_featureDefault = utils.getOptionalAttr("FeatureDefault", 1, ['no', 'yes', 'choose'], "'FeatureDefault' attribute of Binding {0}".format (val), dom)

        featureAspect = dom.getAttribute("Feature")
        if "" == featureAspect:
            featureAspect = dom.getAttribute("FeatureAspect")
        if "" != featureAspect:
            self.m_featureAspect = []
            aspects = featureAspect.split ("||")
            for aspect in aspects:
                self.m_featureAspect.append (aspect.strip())

        self.m_loadConfig = None
        loadConfig = utils.getDomElementsByName(dom, "LoadConfig")
        if len(loadConfig) > 0:
            appName = loadConfig[0].getAttribute("AppName")
            if "" == appName:
                raise utils.PartBuildError ("No AppName for LoadConfig", part, loadConfig)
            cfgvars = loadConfig[0].getAttribute("ConfigVars")
            if "" == cfgvars:
                raise utils.PartBuildError ("No ConfigVars for LoadConfig", part, loadConfig)
            appName = utils.resolveBindingMacros (appName, part.GetPlatform(), part, forceUseApiNumber=False)
            self.m_loadConfig= (appName, cfgvars)

        self.m_transkitBindings = None
        transkitDomNodes = utils.getDomElementsByName(dom, "TransKit")
        if len(transkitDomNodes) > 0 and part.WantTranskit():
            self.m_transkitBindings = []
            for transkitDomNode in transkitDomNodes:
                transkitBinding = TranskitBinding (self)
                transkitBinding.FromXml (part, transkitDomNode)
                self.m_transkitBindings.append (transkitBinding)
        self.m_trankitResourceFiles = []

        # Rather than support tabs which we discourage anyway, just throw an exception
        # This should be covered in the validate command too.
        if '\t' in self.m_val:
            raise utils.PartBuildError ("Tabs found in partfile.  Please use spaces.", part, None)

        # for now, only one COM class registration per Assembly
        ComClasses = utils.getDomElementsByName(dom, "Class")
        if len(ComClasses) > 0:
            for CoClass in ComClasses:
                clsid = CoClass.getAttribute ("Id")
                context = CoClass.getAttribute ("Context")
                desc = CoClass.getAttribute ("Description")
                model = CoClass.getAttribute ("ThreadingModel")
                self.m_class = (clsid, desc, context, model)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __str__(self):
        return "PartBinding: val:{0} target:{1} required:{2}".format (self.m_val, self.m_target, self.m_required)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AddLoadConfigToProduct (self, product):
        if not self.m_loadConfig:
            return

        appname = self.m_loadConfig[0]
        cfgs = utils.splitSources (self.m_loadConfig[1])
        for cfg in cfgs:
            if cfg in product.m_loadConfig:
                if not appname in product.m_loadConfig[cfg]:
                    product.m_loadConfig[cfg].append (appname)
            else:
                product.m_loadConfig[cfg] = [self.m_loadConfig[0]]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AddTranskitsToProduct (self, part, _product, deliveryDir, inSaveProduct):
        if not self.m_transkitBindings:
            return

        for tkBinding in self.m_transkitBindings:
            tkFilePath = os.path.join (part.GetMyBuildContextDir (), tkBinding.m_sourceDir, "transkit.xml")
            resourceObjs = translationkit.getResourceObjs (tkFilePath, part, "")
            for resourceObj in resourceObjs:
                if resourceObj.IsTrueForBuild ():
                    if inSaveProduct:
                        resourceObj.ReadSourceFiles (tkBinding)
                        _, sourceTail= os.path.split (resourceObj.m_tkOutFilePath)
                        part.PerformFileCopy (os.path.join(deliveryDir, sourceTail), resourceObj.m_tkOutFilePath)
                    else:
                        translationkit.symlinkResourceToDir (resourceObj, deliveryDir, tkBinding)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def BuildTranskits (self, part, logFile, _languages=None, cleanTranskit=False):
        if not self.m_transkitBindings:
            return

        if translationkit.IsClobberBuild () and not cleanTranskit:
            return

        for tkBinding in self.m_transkitBindings:
            tkFilePath = os.path.join (part.GetMyBuildContextDir (), tkBinding.m_sourceDir, "transkit.xml")
            resourceObjs = translationkit.getResourceObjs (tkFilePath, part, "")
            for resourceObj in resourceObjs:
                resourceObj.SetLogFile (logFile)
                if resourceObj.IsTrueForBuild ():
                    if cleanTranskit:
                        resourceObj.RemoveBuiltFile (tkBinding)
                    else:
                        resourceObj.Compile (tkBinding)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ShouldSkipBinding (self, product):
        if utils.isBlank (self.m_productDirectory):
            return True

        if None == self.m_featureAspect:
            return False

        # fnmatch is not thread-safe. However it works by creating a regex and using that, so we can do it ourselves here.
        aspectRegexDict = {}
        def getAspectRegex (aspect):
            if not aspect in aspectRegexDict:
                regex = fnmatch.translate(aspect.lower())
                aspectRegexDict[aspect] = re.compile (regex)
            return aspectRegexDict[aspect]

        # if the FeatureDefault value is either "no" or "choose", look for it in the "FeatureInclude" list
        if self.m_featureDefault == 0 or self.m_featureDefault == 2:
            for aspect in self.m_featureAspect:
                for incl in product.m_includeFeatures:
                    if getAspectRegex(aspect).match (incl.lower()):
                        utils.showInfoMsg("Explicitly including Feature {0} in {1}\n".format(self.m_featureAspect[0], product), utils.INFO_LEVEL_SomewhatInteresting)
                        return False # explicitly included

            if self.m_featureDefault == 0: # if the default is no, and its nowhere in the include list, skip it
                utils.showInfoMsg("Implicitly excluding Feature {0} in {1}\n".format(self.m_featureAspect[0], product), utils.INFO_LEVEL_RarelyUseful)
                return True

        # the feature is either defaulted to "yes" or "choose" but it is not in the include list. If it is explicitly
        # in the FeatureExclude list, skip it (all features excluded)
        if len (product.m_omitFeatureAspects) > 0:
            with product.GetFeatureAspectLock():
                aspectShouldBeOmitted = True
                for aspect in self.m_featureAspect:
                    aspectInList = False
                    for omit in product.m_omitFeatureAspects:
                        if getAspectRegex(aspect).match (omit.lower()):
                            aspectInList = True # Found a match in the list.
                            break
                    if not aspectInList: # Only omit if every aspect is blacklisted
                        aspectShouldBeOmitted = False
                        break

                # Make sure we have one that is not excluded
                if aspectShouldBeOmitted:
                    utils.showInfoMsg("Explicitly excluding Feature {0} in {1}\n".format(self.m_featureAspect[0], product), utils.INFO_LEVEL_SomewhatInteresting)
                    return True

        # if the feature was specified as "Choose" and it is not in either list, stop now and report error
        if self.m_featureDefault == 2:
            raise utils.PartBuildError("Feature '{0}' must be either included or excluded".format(self.m_featureAspect[0]), product)

        # the default was "yes", don't skip it
        utils.showInfoMsg("Implicitly including Feature {0} in {1}\n".format(self.m_featureAspect[0], product), utils.INFO_LEVEL_RarelyUseful)
        return False

#-------------------------------------------------------------------------------------------
# ProductBindings are used to bind a sub-product output to a specific parent product directory
# bsiclass
#-------------------------------------------------------------------------------------------
class ProductBinding (object):
    def __init__(self, subProduct, productDirectory, required=False, subDir=""):
        self.m_subProduct = subProduct
        self.m_productDirectory = productDirectory.lower()
        self.m_required = required
        self.m_subDir = subDir

#-------------------------------------------------------------------------------------------
# Every Part can have a list of PartBindings
# bsiclass
#-------------------------------------------------------------------------------------------
class PartBindings (object):
    def __init__(self, part, dom):
        self.m_publicAPI     = self.ReadPublicAPI (part, dom)
        self.m_vendorAPI     = self.ReadVendorAPI (part, dom)
        self.m_directories   = self.ReadDirectories (part, dom)
        self.m_libFiles      = self.ReadFileList ("Libs", dom, part, False)
        self.m_assemblyFiles = self.ReadFileList ("Assemblies", dom, part)
        self.m_mergeModules  = self.ReadFileList ("MergeModules", dom, part)
        self.m_noticeFiles   = self.ReadFileList ("VendorNotices", dom, part)
        self.m_miscFiles     = self.ReadMiscFiles (part, dom)
        self.m_documentations = self.ReadDocumentationFiles (part, dom)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ReadPublicAPI (self, part, dom):
        bindings = []
        publicAPIs = utils.getDomElementsByName(dom, "PublicAPI")
        for publicAPI in publicAPIs:
            domain = publicAPI.getAttribute ("Domain")
            sourceDirectory = publicAPI.getAttribute("SourceDirectory") if publicAPI.getAttribute("SourceDirectory") else None
            upack = publicAPI.getAttribute("Upack") if publicAPI.getAttribute("Upack") else None
            nuget = publicAPI.getAttribute("Nuget") if publicAPI.getAttribute("Nuget") else None
            if "" == domain:
                raise utils.PartBuildError ("No Domain given for PublicAPI", part, publicAPI)
            fc = publicAPI.firstChild
            bindings.append (PartBinding (domain, fc.data if fc else None, publicAPI, "PublicAPI", part, False, sourceDirectory=sourceDirectory, bindToTranskit=self.ParseBindToTranskitAttribute(publicAPI), bindToWixBuild=self.ParseBindToWixInstallerAttribute (publicAPI), upack=upack, nuget=nuget))
        return bindings

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ReadVendorAPI (self, part, dom):
        bindings = []
        vendorAPIs = utils.getDomElementsByName(dom, "VendorAPI")
        for vendorAPI in vendorAPIs:
            domain = vendorAPI.getAttribute ("Domain")
            sourceDirectory = vendorAPI.getAttribute("SourceDirectory") if vendorAPI.getAttribute("SourceDirectory") else None
            upack = vendorAPI.getAttribute("Upack") if vendorAPI.getAttribute("Upack") else None
            nuget = vendorAPI.getAttribute("Nuget") if vendorAPI.getAttribute("Nuget") else None
            if "" == domain:
                raise utils.PartBuildError ("No Domain given for VendorAPI", part, vendorAPI)
            fc = vendorAPI.firstChild
            bindings.append (PartBinding (domain, fc.data if fc else None, vendorAPI, "VendorAPI", part, False, sourceDirectory=sourceDirectory, upack=upack, nuget=nuget))
        return bindings

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ReadDirectories (self, part, dom):
        bindings = []
        dirs = utils.getDomElementsByName(dom, "Directory")
        for dirNum, dirEl in enumerate(dirs):
            targetVal = dirEl.getAttribute ("SubPartDirectory")
            source = dirEl.getAttribute ("SourceName")
            if "" == source:
                raise utils.PartBuildError ("No SourceName given for Directory Binding {0}".format(dirNum+1), part, dirEl)
            bindings.append (PartBinding (source, targetVal, dirEl, "", part, True))
        return bindings

    #--------------------------------------------------------------------------------
    # @bsimethod
    #--------------------------------------------------------------------------------
    def ParseBindToTranskitAttribute (self, domNode):
        return "true" == domNode.getAttribute ("BindToTranskit").lower()

    #--------------------------------------------------------------------------------
    # @bsimethod
    #--------------------------------------------------------------------------------
    def ParseBindToWixInstallerAttribute (self, domNode):
        return "true" == domNode.getAttribute ("BindToWixBuild").lower()


    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ReadFileList (self, bindingType, dom, part, defaultRequired=True):
        bindings = []
        files = utils.getDomElementsByName(dom, bindingType)
        for _, bfile in enumerate(files):
            targetVal = bfile.getAttribute ("SubPartDirectory")
            textChildren = [node for node in bfile.childNodes if node.nodeType == node.TEXT_NODE]
            allData = ' '.join([str(node.data) for node in textChildren])
            if allData is not None:
                if 'BBPW' in os.environ: # DMS speciality code until they update
                    if allData.startswith ('\\'):
                        allData = allData [1:] # bb1 somehow allowed absolute paths
                bindings.append (PartBinding (allData, targetVal, bfile, bindingType, part, defaultRequired, self.ParseBindToTranskitAttribute(bfile), self.ParseBindToWixInstallerAttribute (bfile)))
        return bindings

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ReadDocumentationFiles (self, part, dom):
        bindings = []
        if not part.m_docItems:
            return bindings

        docFiles = utils.getDomElementsByName(dom, "Documentation")
        for _, docFile in enumerate (docFiles):
            name = docFile.getAttribute ("Name").lower ()
            matchedNameDocItems = []
            for docItem in part.m_docItems.m_documentationItems:
                if name == docItem.m_name:
                    matchedNameDocItems.append (docItem)

            if len (matchedNameDocItems) == 0:
                raise utils.PartBuildError ("DocumentationItem with Name={0} is not defined".format (name), part)

            target = docFile.getAttribute ("SubPartDirectory")
            if "" != target:
                target = os.path.join ("SubParts", target)

            fc = docFile.firstChild
            for matchedNameDocItem in matchedNameDocItems:
                if fc and fc.nodeType == fc.TEXT_NODE:
                    text = fc.data.replace ("$(language)", matchedNameDocItem.GetLanguage ())
                    binding = PartBinding (text, target, docFile, "", part, True, self.ParseBindToTranskitAttribute(docFile), self.ParseBindToWixInstallerAttribute (docFile), docItem=matchedNameDocItem)
                    if matchedNameDocItem.m_useLanguageDir:
                        binding.m_subDir = os.path.join (("" if binding.m_subDir == None else binding.m_subDir), matchedNameDocItem.GetLanguage ())

                    bindings.append (binding)

        return bindings

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ReadMiscFiles (self, part, dom):
        bindings = []
        files = utils.getDomElementsByName(dom, "Files")
        filesWithMissingSubPartDir = []

        for _, sbfile in enumerate(files):
            target = sbfile.getAttribute ("SubPartDirectory")
            if "" != target:
                target = os.path.join ("SubParts", target)

            fc = sbfile.firstChild
            if fc and fc.nodeType == fc.TEXT_NODE:
                bindings.append (PartBinding (fc.data, target, sbfile, "", part, True, self.ParseBindToTranskitAttribute(sbfile), self.ParseBindToWixInstallerAttribute (sbfile)))

            if fc and "" == target:
                fclist= fc.data.replace(",", "\n").split("\n")
                for item in fclist:
                    if item.strip() != "":
                        filesWithMissingSubPartDir.append (item.strip())

        return bindings

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def _buildTranskits (self, part, bindingList, logFile, _languages=None, cleanTranskit=False):
        if None != bindingList:
            for binding in bindingList:
                binding.BuildTranskits(part, logFile, cleanTranskit=cleanTranskit)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def BuildTranskits (self, part, logFile, _languages=None, cleanTranskit=False):
        self._buildTranskits (part, self.m_assemblyFiles, logFile, cleanTranskit=cleanTranskit)
        self._buildTranskits (part, self.m_miscFiles, logFile, cleanTranskit=cleanTranskit)


#--------------------------------------------------------------------------------
# @bsiclass
#--------------------------------------------------------------------------------
class RequiredTranskitLkg (object):
    def __init__(self):
        self.m_lkgSource = ""
        self.m_platforms = []


#-------------------------------------------------------------------------------------------
# A single documentation item
# bsiclass
#-------------------------------------------------------------------------------------------
class DocItem (object):
    #
    # NEEDSWORK: this really should be a interface class, with derived classes for various types of doc fetching....
    #

    def __init__(self, doctype, dom, part, languageSetting = None):
        self.m_type = doctype.lower()
        self.m_name   = dom.getAttribute ("Name").lower ()
        excludeLanguages = dom.getAttribute ("ExcludeLanguages")
        self.m_excludeLanguages = []
        if len (excludeLanguages) > 0:
            self.m_excludeLanguages = [language.strip ().lower () for language in excludeLanguages.split (',')]

        includeLanguages = dom.getAttribute ("IncludeLanguages")
        self.m_includeLanguages = []
        if len (includeLanguages) > 0:
            self.m_includeLanguages = [language.strip ().lower () for language in includeLanguages.split (',')]


        language = dom.getAttribute ("Language")
        if language and len (language) > 0:
            self.m_languageSetting = languagesettings.LanguageSettings (language.lower ())
        else:
            self.m_languageSetting = languageSetting

        self.m_localDir = os.path.join (buildpaths.getDocumentationRoot (), part.m_info.m_buildContext, self.GetLanguage ())
        self.m_useLanguageDir = dom.getAttribute ("UseLanguageDir").lower () != "false" # Always use Language dir, unless set to false

        if self.m_type == "trisoft":
            self.m_format = dom.getAttribute ("Format")
            if "" == self.m_format:
                raise utils.PartBuildError ("'Format' attribute needed for {0} documentation references".format (doctype), part)

            self.m_baseline = dom.getAttribute ("Baseline")
            if "" == self.m_format:
                raise utils.PartBuildError ("'Baseline' attribute needed for {0} documentation references".format (doctype), part)

            if not dom.hasChildNodes():
                raise utils.PartBuildError ("A target file specification is needed for {0} documentation references".format (doctype), part)

            self.m_docfile = dom.firstChild.data.strip()

#        <Documentation>
#            <DocItem Type="Trisoft" Format="pdf" Baseline="Bentley Transmittal Services Implementation Guide-v1-GUID-DB9D1925-A04E-4B00-81BF-F8F3FAFFA306-2012/04/11 17:36:59">
#                Transmittals\Bentley Transmittal Services Implementation Guide
#            </DocItem>
#            <DocItem Type="Trisoft" Format="chm" Baseline="Bentley Transmittal Services v1.0 User Guide-v1-GUID-63D554EB-15A0-406F-928F-F5E2C1F22BCE-2012/07/08 23:07:20">
#                Transmittals\Bentley Transmittal Services v1.0 User Guide
#            </DocItem>
#        </Documentation>

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetLanguage (self):
        if not self.m_languageSetting:
            return "en"

        return self.m_languageSetting.m_culture.lower ()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def IsAllowedItem (self):
        # There shall be a language setting to be able to check for language. Otherwise, this item is true
        if not self.m_languageSetting:
            return True

        # It shall not be exclude languages
        if self.m_languageSetting.m_culture.lower () in self.m_excludeLanguages:
            return False

        # No include language is given, then item is true
        if len (self.m_includeLanguages) == 0:
            return True

        # Include languages are listed, but current language is not listed there, result is False
        if self.m_languageSetting.m_culture.lower () not in self.m_includeLanguages:
            return False

        return True
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __str__(self):
        return "DocItem: type:{0} format:{1} baseline:{2} file:{3}".format (self.m_type, self.m_format, self.m_baseline, self.m_docfile)


#-------------------------------------------------------------------------------------------
# Parts can reference documentation sources
# bsiclass
#-------------------------------------------------------------------------------------------
class DocItems (object):
    def __init__(self, part, dom):
        self.m_documentationItems = self.ReadDocumentationItems (part, dom)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ReadDocumentationItems (self, part, dom):
        items = []
        for elem in utils.getDomElementsByName(dom, "DocumentationItem"):
            for languageSetting in translationkit.getLanguageSettings ():
                doctype = elem.getAttribute ("Type")
                if "" == doctype:
                    raise utils.PartBuildError ("'Type' attribute needed for DocumentationItem elements", part)

                d = DocItem (doctype, elem, part, languageSetting)
                if d.IsAllowedItem ():
                    d.m_docfile = d.m_docfile.replace ("$(language)", d.GetLanguage ())
                    items.append (d)

        return items

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class InstallerWixFile (object):
    def __init__ (self, dom, part):
        self.m_sourceFile = dom.getAttribute ("SourceFile")
        self.m_wixVersion = None
        self.m_wixRepository = None
        self.m_wixUpackTool = False

        if not self.m_sourceFile or self.m_sourceFile == "":
            raise utils.PartBuildError ("SourceFile attribute is required for InstallWixFile element", part)
        self.m_sourceFile = self.m_sourceFile + globalvars.installerWixDefintionFile

        wixVersion = dom.getAttribute ("WixVersion")
        if wixVersion and wixVersion != "":
            self.m_wixVersion = wixVersion
        wixRepo = dom.getAttribute ("WixRepository")
        if wixRepo and wixRepo != "":
            self.m_wixRepository = wixRepo

        self.m_wixUpackTool = utils.getOptionalBoolAttr ("WixUpackTool", False, "'WixUpackTool' attribute of InstallerWixFile", dom)

        if self.IsWixVersion () != self.IsWixRepo ():
            raise utils.PartBuildError ("One of the attribute WixVersion or WixRepository cannot be used. Provide both the attributes. Either provide both or none.\r\nWixVersion={0} WixRepository={1}".format (self.m_wixVersion, self.m_wixRepository), part)

    def GetSourceFile (self):
        return self.m_sourceFile

    def IsWixVersion (self):
        return self.m_wixVersion != None

    def GetWixVersion (self):
        return self.m_wixVersion

    def IsWixRepo (self):
        return self.m_wixRepository != None

    def GetWixRepo (self):
        return self.m_wixRepository

    def IsWixFromUpackTool (self):
        return self.m_wixUpackTool

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class SubPartInfo (object):
    # A struct to pass around for the info on a <subpart> line.
    def __init__(self, subpartName, partType):
        self.m_subpartName = subpartName
        self.m_subpartType = partType
        self.m_repoName = None
        self.m_subpartFile = None
        self.m_platform = None
        self.m_libType = globalvars.LIB_TYPE_Parent
        self.m_bindToDirectory = None
        self.m_pkgType = None
        self.m_includeToolInTranskit = False

    def SetRepo (self, repoName):     self.m_repoName = repoName
    def SetFile (self, subpartFile):  self.m_subpartFile = subpartFile
    def SetPlatform (self, platform): self.m_platform = platform
    def SetLibtype (self, libtype):   self.m_libType = libtype
    def SetBindToDirectory (self, bindToDirectory): self.m_bindToDirectory = bindToDirectory

    def ChooseStatic (self, isStatic):
        # If a subpart is marked as static, then make it static.  Otherwise follow superpart.
        return True if self.m_libType == globalvars.LIB_TYPE_Static else isStatic

    def __repr__(self):
        partType = 'Unknown'
        if   self.m_subpartType == PART_TYPE_Part:           partType = 'SubPart'
        elif self.m_subpartType == PART_TYPE_Product:        partType = 'SubProduct'
        elif self.m_subpartType == PART_TYPE_NuGetProduct:   partType = 'SubNuGetProduct'
        elif self.m_subpartType == PART_TYPE_MultiPlatform:  partType = 'SubMultiPlatform'
        elif self.m_subpartType == PART_TYPE_UPackProduct:   partType = 'SubUPackProduct'
        elif self.m_subpartType == PART_TYPE_Package:        partType = 'SubPackage-{0}'.format(self.m_pkgType)
        elif self.m_subpartType == PART_TYPE_ToolPart:       partType = 'SubToolPart'
        elif self.m_subpartType == PART_TYPE_ToolPackage:    partType = 'SubToolPackage-{0}'.format(self.m_pkgType)
        elif self.m_subpartType == PART_TYPE_Any:            partType = 'SubAny'
        return 'SubPartInfo ({0} {1} [repo:"{2}", file:"{3}"] Platform: {4} Static: {5})'.format (partType, self.m_subpartName, self.m_repoName, self.m_subpartFile, self.m_platform, self.m_libType)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class Part (object):
    def IsProduct(self):        return False
    def IsNuGetProduct(self):   return False
    def IsUPackProduct(self):   return False
    def IsPackage(self):        return False
    def IsToolPart(self):       return False
    def PartType(self):         return "Part"
    def __repr__(self):         return self.__str__()
    def __str__(self):          return "{0} <{1}>".format (self.m_info.m_file, self.m_info.m_name)
    # To be used in Part Histories, differentiating between part and product.  Perhaps this should replace the string in __str__.
    def GetUniqueRepresentation(self):  return "{0}:{1} <{2} {3} {4}>".format (self.m_info.m_buildContext, self.m_info.m_name,  self.PartType(), self.GetPlatform().GetDirName(), self.m_info.GetStaticString())
    def GetShortRepresentation(self): return "{0}:{1}".format (self.m_info.m_buildContext, self.m_info.m_name)
    def IsPrivateScope (self):  return self.m_scope == globalvars.PART_SCOPE_Private
    def IsStatic (self):        return self.m_info.m_static
    def GetPlatform (self):     return self.m_info.m_platform
    def IsMultiPlatform(self):  return self.m_isMultiPlatform
    def GetPropertiesPath(self): return os.path.join(buildpaths.getOutputRoot(self.m_info.m_platform, self.m_info.m_static), "Build", "MSBuild", self.m_info.m_buildContext, "BM-{}.properties".format(self.m_info.m_name))

    def GetLogFileRoot(self):                       return os.path.join (buildpaths.getOutputRoot(self.m_info.m_platform, self.m_info.m_static), buildpaths.DIRNAME_LOGFILES)

    def GetTranskitBuildRoot(self):                 return os.path.join (buildpaths.getOutputRoot(self.m_info.m_platform, self.m_info.m_static), buildpaths.DIRNAME_TRANSKIT_BUILD)
    def GetTranskitInclude(self):                   return os.path.join (self.GetTranskitRoot(), buildpaths.DIRNAME_TRANSKIT_INCLUDE)
    def GetTranskitRoot (self):                     return os.path.join (buildpaths.getOutputRoot(self.m_info.m_platform, self.m_info.m_static), buildpaths.DIRNAME_TRANSKIT)
    def GetTranskitLocalizedRoot(self,language):    return os.path.join (self.GetTranskitRoot(), buildpaths.DIRNAME_TRANSKIT_LOCALIZED, language)
    def GetCombinedInstallerRoot(self):             return os.path.normpath (os.path.join(globalvars.programOptions.outRootDirectory, buildpaths.DIRNAME_COMBINED, buildpaths.DIRNAME_INSTALLER))
    def AddSubProductBinding(self, subProduct, directoryName):
        self.m_subProductBindings.add(ProductBinding(subProduct, directoryName))

    def __init__(self, partInfo, spec):
        self.m_info             = partInfo
        self.m_provenances      = None
        self.m_subParts         = None
        self.m_bindings         = None
        self.m_bmakeFile        = None
        self.m_bmakeOptions     = None
        self.m_useBBmake        = False
        self.m_projectFile      = None
        self.m_dotNetSdkProjectFile = None
        self.m_npmFile          = None
        self.m_yarnFile         = None
        self.m_projectName      = None
        self.m_solutionExtensibilityGlobals = None
        self.m_configFile       = None
        self.m_apiNumber        = None
        self.m_prgDirName       = None
        self.m_excludePlatforms = None
        self.m_excludeLibTypes  = None
        self.m_action           = None
        self.m_bindAlways       = False
        self.m_parentPartContexts   = set()
        self.m_parentProductContexts = {}
        self.m_subProductBindings = set()
        self.m_didOpenLogFile   = False
        self.m_wipPartEnvVar    = None
        self.m_featureAspectPart = False        # True if this is the part that builds Feature Aspects - a couple extra args to the makefile
        self.m_featureAspectProduct = None      # The parent product which has FeatureAspects set.
        self.m_currentLibType   = None
        self.m_docItems         = None          # Trisoft stuff to pull
        self.m_additionalRepos  = None          # RequiredRepository
        self.m_provProduct      = None          # Top-level product for provenance (version), if set up
        self.m_scope            = globalvars.PART_SCOPE_Public
        self.m_isBuildInstallSet = True if spec.getAttribute ("BuildInstallSet").lower () == "true" else False
        self.m_packageBuildType  = installerelements.GetPackageTypeToBuildFromOption (spec.getAttribute ("InstallSetType"))
        self.m_sequential       = False         # For subparts to be sequential because of bad build procedure.
        self.m_deferType        = None          # None means always build as-found. A string will be used for filtering.
        self.m_requiresFA       = False         # If a part requires all Feature Aspects to be complete. Used to create subparts to all FA parts.
        self.m_nugetBindingList = False
        self.m_finished         = False
        self.m_failed           = False
        self.m_version          = None
        self.m_isMultiPlatform = False

        self.m_isSaveTranskit = True if spec.getAttribute ("SaveTranskit").lower () == "true" else False
        self.m_sdkSources        = []
        self.m_nugetPackages     = []
        self.m_adoBuildArtifacts = []
        self.m_npmLocations      = []
        self.m_nugetProvenance   = set()         # For LKGs, when we parse provenance build up this list.
        self.m_universalPackages = []

        docElems = utils.getDomElementsByName (spec, "Documentation")
        if len(docElems) > 0:
            self.m_docItems = DocItems (self, docElems[0])

        deferType = spec.getAttribute("DeferType")
        if deferType:
            self.m_deferType = deferType

        faComplete = utils.getDomElementsByName(spec, "RequiresFeatureAspectsComplete")
        if 0 != len(faComplete):
            self.m_requiresFA = True

        extraRepos = utils.getDomElementsByName(spec, "RequiredRepository")
        if 0 != len(extraRepos):
            if self.m_additionalRepos == None:
                self.m_additionalRepos = []
            for repo in extraRepos:
                if None != repo.firstChild.data:
                    self.m_additionalRepos.append (repo.firstChild.data.strip())

        self.m_subParts = self.ReadSubParts(spec)
        self.m_prgDirName = spec.getAttribute("PrgOutputDir")
        sdkSources = utils.getDomElementsByName(spec, "SdkSource")
        if 0 != len(sdkSources):
            for comp in sdkSources:
                if None != comp.firstChild.data:
                    self.m_sdkSources.append (comp.firstChild.data.strip())

        nugetPackages = utils.getDomElementsByName (spec, "NuGetPackage")
        if 0 != len(nugetPackages):
            for pkg in nugetPackages:
                if None != pkg.firstChild.data:
                    pkgName = pkg.firstChild.data.strip()
                    pkgFramework = None
                    if 'targetFramework' in pkg.attributes.keys():
                        pkgFramework = pkg.getAttribute('targetFramework')
                    source = globalvars.buildStrategy.GetNugetSource(pkgName)
                    if source.m_multiplatform:
                        pkgName = nugetpkg.TransformNugetName(pkgName, self.GetPlatform(), False, self.IsToolPart())

                    # For FB LKGs we have a stream name that is in the package because versions don't follow branches.
                    # So store the nuget source with the real name, although it does contain the original name as well.
                    if pkgName == source.m_alias:
                        pkgSource = source
                    else:
                        pkgSource = source.Clone (pkgName)
                        globalvars.buildStrategy.AddNugetSource (pkgSource) # Get it into the list for use later.
                    self.m_nugetPackages.append (nugetpkg.NuGetUsage(pkgSource, pkgFramework, self.GetPlatform(), self.IsStatic(), self.IsToolPart()))

        ADOBuildArtifacts = utils.getDomElementsByName (spec, "ADOBuildArtifact")
        for buildArtifact in ADOBuildArtifacts:
            if None != buildArtifact.firstChild.data:
                name = buildArtifact.firstChild.data.strip()
                adoBuildArtifactSource = globalvars.buildStrategy.GetADOBuildArtifactSource(name)
                if not adoBuildArtifactSource:
                    raise utils.PartBuildError ("Missing ADOBuildArtifactSource for ADOBuildArtifact {0} in part {1}".format(name, partInfo.GetShortPartDescriptor()), self, spec)
                self.m_adoBuildArtifacts.append((name, adoBuildArtifactSource))

        universalPackages = utils.getDomElementsByName (spec, "Upack")
        if 0 != len(universalPackages):
            for pkg in universalPackages:
                if None != pkg.firstChild.data:
                    pkgName = pkg.firstChild.data.strip()
                    pkgSource = globalvars.buildStrategy.GetUpackSource(pkgName, self.m_info.m_platform)
                    if not pkgSource: 
                        raise utils.PartBuildError ("Missing UPackSource for Upack {0} in part {1}".format(pkgName, partInfo.GetShortPartDescriptor()), self, spec)
                    self.m_universalPackages.append ((pkgName, pkgSource))

        npmDependencies = utils.getDomElementsByName (spec, "NpmDependency")
        if 0 != len(npmDependencies):
            for npmDep in npmDependencies:
                if None != npmDep.firstChild.data:
                    npmDepName = npmDep.firstChild.data.strip()
                    self.m_npmLocations.append (npmDepName)
                    # As a convenience, always add nodejs as a required repository so we can pull NPM
                    if self.m_additionalRepos == None:
                        self.m_additionalRepos = []
                    # Pulling a newer version for doing the npm gets; hoping it wil fix problems pulling from ADO
                    npmRepo = 'Nodejs14'
                    if globalvars.buildStrategy.HasLocalRepository (npmRepo) and not npmRepo in self.m_additionalRepos:
                        self.m_additionalRepos.append (npmRepo)
                    # Also pulling the older version if the newer one is not in the build strategies (older branch).
                    else:
                        npmRepo = 'Nodejs'
                        if not npmRepo in self.m_additionalRepos:
                            self.m_additionalRepos.append (npmRepo)

        self.ReadRequiredTranskitLkg (spec)

        installerFiles = utils.getDomElementsByName(spec, "InstallerWixFile")
        self.m_installerFileSource = None
        if 0 != len (installerFiles):
            for installerFile in installerFiles:
                self.m_installerFileSource = InstallerWixFile (installerFile, self)
                # This is setting a global variable which doesn't work with a cached part, but leaving it in 
                # because it also checks to make sure all the versions are the same.
                installerelements.SetWixVersion (self)
                break

        self.m_wixBundleFile = None
        wixBundleFiles = utils.getDomElementsByName(spec, "WixBundleFile")
        if len (wixBundleFiles) > 0:
            self.m_wixBundleFile = wixBundleFiles [0].getAttribute ("SourceFile")

        nuspecFiles = utils.getDomElementsByName(spec, "NuSpecFile")
        self.m_nuSpecFile =  None
        if 0 != len (nuspecFiles):
            for nuspecFile in nuspecFiles:
                self.m_nuSpecFile = nuspecFile.getAttribute ("SourceFile")
                break

        if self.m_nuSpecFile:
            self.m_nuSpecFile = symlinks.normalizePathName (os.path.join (partInfo.m_partFileDir, self.m_nuSpecFile))

        self.m_wixBundleFiles = None
        bundleFiles = utils.getDomElementsByName (spec, "WixBundleFile")
        if 0 != len (bundleFiles):
            for bundleFile in bundleFiles:
                self.m_wixBundleFiles = (bundleFile.getAttribute ("SourceFile"), bundleFile.getAttribute ("LanguageBundle"), bundleFile.getAttribute ("LanguagePackBundle"))

    #--------------------------------------------------------------------------------
    # bsimethod
    #--------------------------------------------------------------------------------
    def SetBuildTargetFromDom (self, spec):
        # Validate atributes. Should be set to error at some point
        isBMakeFile = spec.hasAttribute ("BMakeFile")
        isBentleyBuildMakeFile = spec.hasAttribute ("BentleyBuildMakeFile")
        isProjectFile = spec.hasAttribute ("ProjectFile")
        isDotNetSdkFile = spec.hasAttribute ("DotNetSdkFile")
        isNpmFile = spec.hasAttribute ("NpmFile")
        isYarnFile = spec.hasAttribute ("YarnFile")
        hasBMakeOptions = spec.hasAttribute ("BMakeOptions")
        hasBentleyBuildMakeOptions = spec.hasAttribute ("BentleyBuildMakeOptions")
        hasConfigFile = spec.hasAttribute ("ConfigFile")
        hasSolutionExtensibilityGlobals = spec.hasAttribute ("SolutionExtensibilityGlobals")

        if hasBMakeOptions and hasBentleyBuildMakeOptions:
            utils.showInfoMsg("Warning! {0} {1}: Both BMakeOptions and BentleyBuildMakeOptions can't be defined.\n".format(self.m_info.m_file, self.m_info.m_name), utils.INFO_LEVEL_VeryInteresting, utils.YELLOW)

        if isBMakeFile and hasBentleyBuildMakeOptions:
            utils.showInfoMsg("Warning! {0} {1}: BMakeFile expects BMakeOptions to be defined, BentleyBuildMakeOptions should be used with BentleyBuildMakeFile.\n".format(self.m_info.m_file, self.m_info.m_name), utils.INFO_LEVEL_VeryInteresting, utils.YELLOW)

        if isBentleyBuildMakeFile and hasBMakeOptions:
            utils.showInfoMsg("Warning! {0} {1}: BentleyBuildMakeFile expects BentleyBuildMakeOptions to be defined, BMakeOptions should be used with BMakeFile, DotNetSdkFile or ProjectFile.\n".format(self.m_info.m_file, self.m_info.m_name), utils.INFO_LEVEL_VeryInteresting, utils.YELLOW)

        if (isProjectFile or isDotNetSdkFile) and hasBentleyBuildMakeOptions:
            utils.showInfoMsg("Warning! {0} {1}: ProjectFile and DotNetSdkFile expects BMakeOptions to be defined, BentleyBuildMakeOptions should be used with BentleyBuildMakeFile.\n".format(self.m_info.m_file, self.m_info.m_name), utils.INFO_LEVEL_VeryInteresting, utils.YELLOW)

        if isBMakeFile and isBentleyBuildMakeFile:
            utils.showInfoMsg("Warning! {0} {1}: Both BMakeFile and BentleyBuildMakeFile can't be defined.\n".format(self.m_info.m_file, self.m_info.m_name), utils.INFO_LEVEL_VeryInteresting, utils.YELLOW)

        if isBMakeFile and isProjectFile:
            utils.showInfoMsg("Warning! {0} {1}: Both BMakeFile and ProjectFile can't be defined.\n".format(self.m_info.m_file, self.m_info.m_name), utils.INFO_LEVEL_VeryInteresting, utils.YELLOW)

        if isBentleyBuildMakeFile and isProjectFile:
            utils.showInfoMsg("Warning! {0} {1}: Both BentleyBuildMakeFile and ProjectFile can't be defined.\n".format(self.m_info.m_file, self.m_info.m_name), utils.INFO_LEVEL_VeryInteresting, utils.YELLOW)

        if isBMakeFile and isDotNetSdkFile:
            utils.showInfoMsg("Warning! {0} {1}: Both BMakeFile and DotNetSdkFile can't be defined.\n".format(self.m_info.m_file, self.m_info.m_name), utils.INFO_LEVEL_VeryInteresting, utils.YELLOW)

        if isBentleyBuildMakeFile and isDotNetSdkFile:
            utils.showInfoMsg("Warning! {0} {1}: Both BentleyBuildMakeFile and DotNetSdkFile can't be defined.\n".format(self.m_info.m_file, self.m_info.m_name), utils.INFO_LEVEL_VeryInteresting, utils.YELLOW)

        if isProjectFile and isDotNetSdkFile:
            utils.showInfoMsg("Warning! {0} {1}: Both ProjectFile and DotNetSdkFile can't be defined.\n".format(self.m_info.m_file, self.m_info.m_name), utils.INFO_LEVEL_VeryInteresting, utils.YELLOW)

        # Read attributes
        if isBMakeFile:
            self.m_bmakeFile = spec.getAttribute("BMakeFile")
        elif isBentleyBuildMakeFile:
            self.m_bmakeFile = spec.getAttribute("BentleyBuildMakeFile")
            self.m_useBBmake = True
        elif isProjectFile:
            self.m_projectFile = symlinks.normalizePathName(os.path.join (self.m_info.m_partFileDir, os.path.expandvars(spec.getAttribute("ProjectFile"))))
            self.m_projectName = os.path.splitext(os.path.basename(self.m_projectFile))[0]
        elif isDotNetSdkFile:
            self.m_dotNetSdkProjectFile = symlinks.normalizePathName(os.path.join (self.m_info.m_partFileDir, os.path.expandvars(spec.getAttribute("DotNetSdkFile"))))
            self.m_projectName = os.path.splitext(os.path.basename(self.m_dotNetSdkProjectFile))[0]
        elif isNpmFile:
            self.m_npmFile = symlinks.normalizePathName(os.path.join(self.m_info.m_partFileDir, os.path.expandvars(spec.getAttribute("NpmFile"))))
        elif isYarnFile:
            self.m_yarnFile = symlinks.normalizePathName(os.path.join(self.m_info.m_partFileDir, os.path.expandvars(spec.getAttribute("YarnFile"))))

        if hasConfigFile:
            self.m_configFile = symlinks.normalizePathName(os.path.join (self.m_info.m_partFileDir, os.path.expandvars(spec.getAttribute("ConfigFile"))))

        if hasBMakeOptions:
            self.m_bmakeOptions = spec.getAttribute("BMakeOptions")
        elif hasBentleyBuildMakeOptions:
            self.m_bmakeOptions = spec.getAttribute("BentleyBuildMakeOptions")
        
        if hasSolutionExtensibilityGlobals:
            self.m_solutionExtensibilityGlobals = spec.getAttribute("SolutionExtensibilityGlobals").split(";")

    #--------------------------------------------------------------------------------
    # bsimethod
    #--------------------------------------------------------------------------------
    def ReadRequiredTranskitLkg (self, spec):
        extraTranskitLkgs = utils.getDomElementsByName(spec, "RequiredTranskitLkg")
        if 0 != len(extraTranskitLkgs):
            if len(extraTranskitLkgs) > 1:
                msg = "There should only be 1 RequiredTranskitLkg for {0}.\n".format(self.m_info.m_name)
                raise utils.PartBuildError (msg, self, extraTranskitLkgs[1])

            if None != self.m_info.m_additionalTranskitLkgs:
                msg = "PartInfo {0} has already filled out RequiredTranskitLkg. This scenario is not defined - contact Kevin.\n".format(self.m_info.m_name)
                raise utils.PartBuildError (msg, self)

            currTkLkg = None # Set this up to use in the local method to avoid premature closure inside the loop.
            def handleNoPlatformMatch (csv, nonvalidMatch):
                raise utils.PartBuildError ("RequiredTranskitLkg({0}):OnlyPlatforms='{1}' has wildcard '{2}' which does not match any platform, must be one of {3}.".format(self.m_info.m_name, csv, nonvalidMatch, str(targetplatform.GetXmlOptions())), self, currTkLkg)

            for tkLkg in extraTranskitLkgs:
                currTkLkg = tkLkg
                reqTranskitLkg = RequiredTranskitLkg()
                reqTranskitLkg.m_lkgSource = tkLkg.getAttribute ("LastKnownGoodSource")
                if "" == reqTranskitLkg.m_lkgSource:
                    msg = "RequiredTranskitLkg {0} is missing LastKnownGoodSource attribute which specifies which LKG to use.\n".format(self.m_info.m_name)
                    raise utils.PartBuildError (msg, self, tkLkg)

                onlyPlatforms = tkLkg.getAttribute ("OnlyPlatforms")
                if len (onlyPlatforms) > 0:
                    targetplatform.UpdatePlatformListByName (reqTranskitLkg.m_platforms, onlyPlatforms, handleNoPlatformMatch)

                if 0 == len (reqTranskitLkg.m_platforms):
                    msg = "RequiredTranskitLkg is missing OnlyPlatforms attribute with valid platforms for part {0} OnlyPlatforms='{1}'.\n".format(self.m_info.m_name, onlyPlatforms)
                    raise utils.PartBuildError (msg, self, tkLkg)

                self.m_info.m_additionalTranskitLkgs = reqTranskitLkg

    #--------------------------------------------------------------------------------
    # bsimethod
    #--------------------------------------------------------------------------------
    def TrackProvenance (self):
        # When this is turned on, BuildAction will add a list of provenance items to the part
        #   and test the values, thowing execptions if there are inconsistencies.  Used during
        #   build and saveLKGs and possibley other places
        if None == self.m_provenances:
            self.m_provenances = {}

    #--------------------------------------------------------------------------------
    # bsimethod
    #--------------------------------------------------------------------------------
    def FromLkgs (self):
        return self.m_info.m_fromLKG

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def fullPath(self):
        myName = "\n    " + self.__str__()
        return myName

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetOutputBuildContextDir (self, buildContextName, isStatic=None):
        if None == isStatic:
            isStatic = self.IsStatic()
        return os.path.join (self.GetBuildContextRoot(isStatic), buildContextName)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetMyBuildContextDir (self,isStatic=None, isDocumentationBuildContext=False):
        if True == self.FromLkgs():
            if None == isStatic:
                isStatic = self.IsStatic()
            staticDir = 'static' if isStatic else ''
            return os.path.join (buildpaths.getLKGRoot(self.GetPlatform()), staticDir, self.m_info.m_buildContext)

        elif translationkit.IsTranskitShell ():
            if isDocumentationBuildContext == True:
                return self.GetOutputBuildContextDir (self.m_info.m_buildContext, isStatic)

            return os.path.join (symlinks.normalizePathName (os.path.join("${SrcRoot}", "TranskitBuildContext")), self.GetPlatform().GetDirName(), self.m_info.m_buildContext)
        else:
            return self.GetOutputBuildContextDir (self.m_info.m_buildContext, isStatic)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    # Returns whether the wix xml is coming from source and the full path to it. If it doesn't exist, returns None objects
    def GetInstallerFilePath (self):
        if None == self.m_installerFileSource:
            return (None, None)

        wixFilePathInSource = os.path.join (self.m_info.m_partFileDir, self.m_installerFileSource.GetSourceFile ())
        if os.path.exists(wixFilePathInSource):
            return (True, wixFilePathInSource)

        # symlink to this location is supposed to be create by Binding
        return (False, os.path.join (self.GetMyBuildContextDir (), self.m_installerFileSource.GetSourceFile ()))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetLogFileName(self):
        return os.path.join(self.GetLogFileRoot(), self.m_info.m_buildContext, self.m_info.m_name + LOG_FILE_SUFFIX)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetProvenanceFileName(self):
        return os.path.join (self.GetLogFileRoot(), self.m_info.m_buildContext, self.m_info.m_name + globalvars.provfileName)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetBaseLogName (self):
        return os.path.join(self.GetOutputBuildContextDir (self.m_info.m_buildContext), "Logs", self.m_info.m_name)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetPrgDirName (self):
        return self.m_prgDirName

    #--------------------------------------------------------------------------------
    # bsimethod
    #--------------------------------------------------------------------------------
    def GetBuildContextRoot(self, isStatic):
        if None == isStatic:  # Have to allow passing in of IsStatic to be able to link static to dynamic.
            isStatic = self.m_info.m_static
        return os.path.join (buildpaths.getOutputRoot(self.GetPlatform(), isStatic), buildpaths.DIRNAME_BUILDCONTEXTS)

    #--------------------------------------------------------------------------------
    # bsimethod
    #--------------------------------------------------------------------------------
    def GetProductRoot(self):
        return self.GetProductRootForPlatform (self.GetPlatform())

    #--------------------------------------------------------------------------------
    # bsimethod
    #--------------------------------------------------------------------------------
    def GetProductRootForPlatform (self, platform):
        if None == platform:  # Have to allow passing in of platform to be able to handle multi-compiles.
            platform = self.GetPlatform()
        # Products are always in the dynamic tree.
        return os.path.join (buildpaths.getOutputRoot(platform, isStatic=False), buildpaths.DIRNAME_PRODUCT)

    #--------------------------------------------------------------------------------
    # bsimethod
    #--------------------------------------------------------------------------------
    def GetTranskitIncludeDirDirect (self, buildContextDir):
        if "" == os.path.split (buildContextDir)[1]: # peel off trailing slash.
            buildContextDir = os.path.split (buildContextDir)[0]

        if "" == os.path.split (buildContextDir)[1]:
            return ""

        return os.path.join (self.GetTranskitInclude(), os.path.split (buildContextDir)[1])

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetPartBuildDir (self):
        return os.path.join(buildpaths.getOutputRoot(self.GetPlatform(), self.IsStatic()), 'build', self.m_info.m_buildContext, self.m_info.m_name) + os.sep

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetQualifiedNugetName(self, isLkg=False, isStatic=None):
        # Set isStatic to appropriate boolean value if static directory and naming needs to be supported (i.e. LKGs);
        # otherwise non-static paths and names will be used for 3rd-party nuget packages
        return nugetpkg.TransformNugetName(self.m_info.m_buildContext if isLkg else self.m_info.m_name, self.m_info.m_platform, isStatic if isStatic else False, False)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetNugetPackageRoot(self, isStatic=None):
        # Set isStatic to appropriate boolean value if static directory and naming needs to be supported (i.e. LKGs);
        # otherwise non-static paths and names will be used for 3rd-party nuget packages
        return os.path.join(buildpaths.getOutputRoot(self.GetPlatform(), isStatic if isStatic else False), buildpaths.DIRNAME_NUGETPKG)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetNugetPackageDirectory(self, directory = '', isLkg = False, isStatic=None):
        # Set isStatic to appropriate boolean value if static directory and naming needs to be supported (i.e. LKGs);
        # otherwise non-static paths and names will be used for 3rd-party nuget packages
        return os.path.join(directory if directory else self.GetNugetPackageRoot(isStatic if isStatic else False), 'LKGs' if isLkg else '', self.m_info.m_buildContext if isLkg  else self.m_info.m_name)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetNugetPackagePath(self, directory='', isLkg = False, isStatic=None):
        # Set isStatic to appropriate boolean value if static directory and naming needs to be supported (i.e. LKGs);
        # otherwise non-static paths and names will be used for 3rd-party nuget packages
        resolvedIsStatic = isStatic if isStatic else False
        return os.path.join(self.GetNugetPackageDirectory(directory, isLkg, resolvedIsStatic), '{0}.{1}.nupkg'.format(self.GetQualifiedNugetName(isLkg, resolvedIsStatic), nugetpkg.Nuspec.FormatVersionString(self.GetVersionString())))

    #-------------------------------------------------------------------------------------------
    # NEEDS WORK: This should probably be based on the buildchain
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def WantTranskit(self):
        return self.GetPlatform().WantTranskit()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def IsClobberBuild (self):
        return globalvars.currentAction and hasattr (globalvars.currentAction.m_actionOptions, 'clobberBuild') and globalvars.currentAction.m_actionOptions.clobberBuild
        
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AtThreadStart (self, threadId):
        self.m_threadId = threadId
        utils.addBufferToCurrentThread()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def SetScopeFlagFromDom (self, partDom):
        scopeFlag = partDom.getAttribute("PartScope")
        if len (scopeFlag) > 0:
            self.m_scope = utils.resolveOption (scopeFlag, globalvars.partScopeOptions, "Part {0} PartScope='{1}'".format (self.m_info.m_name, scopeFlag))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def SetSequentialFlagFromDom (self, partDom):
        sequentialFlag = partDom.getAttribute("Sequential")
        if len (sequentialFlag) > 0:
            self.m_sequential = utils.resolveBoolean (sequentialFlag, "Part {0} Sequential='{1}'".format (self.m_info.m_name, sequentialFlag))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def IsExcluded (self, platform, isStatic=None, indent=0):
        ''' Check if a part is excluded by platform or library type'''
        if platform.IsExcluded (self.m_excludePlatforms):
            # Keep track of the fact we printed this.
            if self.GetShortRepresentation() in g_partWasExcludedAnnounced:
                return True
            g_partWasExcludedAnnounced[self.GetShortRepresentation()] = 1
            utils.showInfoMsg (indent*' ' + "Skipping evaluation of {0} because \"{1}\" is an excluded platform for this part\n".format \
                    (self.m_info.GetShortPartDescriptor(), platform.GetXmlName()), utils.INFO_LEVEL_SomewhatInteresting)
            return True

        if self.m_excludeLibTypes:
            if not isStatic:
                isStatic = self.IsStatic()

            outputString = None
            if isStatic and ExcludeLibTypeStatic in self.m_excludeLibTypes:
                outputString = 'Static'
            elif not isStatic and ExcludeLibTypeDynamic in self.m_excludeLibTypes:
                outputString = 'Dynamic'

            if outputString:
                utils.showInfoMsg (indent*' ' + 'Skipping evaluation of {0} because "{1}" is an excluded Library type for this part\n'.format \
                        (self.m_info.GetShortPartDescriptor(), outputString), utils.INFO_LEVEL_SomewhatInteresting)
                return True

        if self.m_wipPartEnvVar:
            if not self.m_wipPartEnvVar in os.environ:
                utils.showInfoMsg (indent*' ' + 'Skipping evaluation of {0} because {1} is not set in the environment\n'.format \
                        (self.m_info.GetShortPartDescriptor(), self.m_wipPartEnvVar), utils.INFO_LEVEL_SomewhatInteresting)
                return True

        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetLibraryExclusionsFromDom (self, curDom):
        excludeLibType = curDom.getAttribute("ExcludeLibType")
        if len (excludeLibType) > 0:
            self.m_excludeLibTypes = []
            for libType in excludeLibType.split (','):
                libType = libType.strip()
                if not libType in ExcludeLibTypes:
                    raise utils.PartBuildError ("Part has invalid ExcludeLibType '{0}'. It must be one of: {1}.".format(libType, ExcludeLibTypes), self, curDom)
                self.m_excludeLibTypes.append (libType)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def SetPlatformExclusionsFromDom (self, curDom):
        if curDom.hasAttribute ("ExcludePlatforms") and curDom.hasAttribute ("OnlyPlatforms"):
            raise utils.PartBuildError ("Part <{0}> cannot have both ExcludePlatforms and OnlyPlatforms.".format (self.m_info.m_name), self, curDom)

        excludePlatforms = curDom.getAttribute("ExcludePlatforms")
        if len (excludePlatforms) > 0:
            def handleNoPlatformMatch (csv, nonvalidMatch):
                raise utils.PartBuildError ("<{0}> ExcludePlatforms='{1}' has platform wildcard '{2}' that does not match any platform, must be one of {3}.".format(self.m_info.m_name, csv, nonvalidMatch, targetplatform.GetXmlOptions().__str__()), self, curDom)

            self.m_excludePlatforms = []
            targetplatform.UpdatePlatformListByName (self.m_excludePlatforms, excludePlatforms, handleNoPlatformMatch)

        onlyPlatforms = curDom.getAttribute("OnlyPlatforms")
        if len (onlyPlatforms) > 0:
            def handleNoPlatformMatch2 (csv, nonvalidMatch):
                raise utils.PartBuildError ("<{0}> OnlyPlatforms='{1}' has platform wildcard '{2}' that does not match any platform, must be one of {3}.".format(self.m_info.m_name, csv, nonvalidMatch, targetplatform.GetXmlOptions().__str__()), self, curDom)

            onlyPlatformsList = []
            targetplatform.UpdatePlatformListByName (onlyPlatformsList, onlyPlatforms, handleNoPlatformMatch2)
            allPlatforms = set (targetplatform.PlatformConstants)
            self.m_excludePlatforms = list (allPlatforms.difference (set (onlyPlatformsList)))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetWipInfoFromDom (self, curDom):
        wipPart = curDom.getAttribute("WipPartEnable")
        if len (wipPart) > 0:
            self.m_wipPartEnvVar = 'WipPartEnable_' + wipPart

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ProcessFeatureAspects (self):
        if self.m_featureAspectPart:
            self.m_featureAspectProduct.EvaluateFeatureAspects ()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def IsCheckSameDisabled (self):
        if globalvars.currentAction and hasattr (globalvars.currentAction.m_actionOptions, "allowNewSymlinkTargets") and globalvars.currentAction.m_actionOptions.allowNewSymlinkTargets:
            return True
        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def PerformBindToFileAction (self, _thisPart, linkName, targetFile, checkSame, checkTargetExists, skipIntermediateLinks):
        # This one is the one that gets overridden in SaveLKGs.  It needs the part argument.
        utils.showInfoMsg ("Binding {0} to file {1}\n".format (linkName, targetFile), utils.INFO_LEVEL_SomewhatInteresting)
        checkSame = checkSame and not self.IsCheckSameDisabled()

        if os.path.isdir(targetFile):
            if 'BBPW' in os.environ: # DMS speciality code until they update
                return
            raise utils.PartBuildError ("Cannot bind Target File '{0}' because it is a directory. Check your partfile to ensure your binding statements do not include directories. ".format (targetFile), self)

        return symlinks.createFileSymLink (linkName, targetFile, checkSame, checkTargetExists, skipIntermediateLinks)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def PerformBindToFile (self, linkName, targetFile, checkSame=True, checkTargetExists=False, skipIntermediateLinks=True):
        return self.PerformBindToFileAction (self, linkName, targetFile, checkSame, checkTargetExists, skipIntermediateLinks)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def PerformFileCopyAction (self, _thisPart, targetFile, sourceFile):
        # This gets overridden in SaveProduct
        utils.showInfoMsg ("Copying {0} to {1}\n".format (sourceFile, targetFile), utils.INFO_LEVEL_SomewhatInteresting)
        if os.path.isdir(targetFile):
            raise utils.PartBuildError ("Cannot copy Target File '{0}' because it is a directory. Check your partfile to ensure your binding statements do not include directories. ".format (targetFile), self)
        return utils.copyFile (sourceFile, targetFile)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def PerformFileCopy (self, targetFile, sourceFile):
        return self.PerformFileCopyAction (self, targetFile, sourceFile)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def PerformDirCopyAction (self, _thisPart, destDir, sourceDir):
        # This gets overridden in SaveProduct
        utils.showInfoMsg ("Copying {0} to {1}\n".format (sourceDir, destDir), utils.INFO_LEVEL_SomewhatInteresting)
        return cmdutil.roboCopyDir(sourceDir, destDir)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def PerformDirCopy (self, destDir, sourceDir):
        return self.PerformDirCopyAction (self, destDir, sourceDir)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def PerformBindToDirAction (self, _thisPart, linkName, targetDir, checkSame, checkTargetExists, skipIntermediateLinks):
        # This one is the one that gets overridden in SaveLKGs.  It needs the part argument.
        utils.showInfoMsg ("Binding {0} to directory {1}\n".format (linkName, targetDir), utils.INFO_LEVEL_SomewhatInteresting)
        checkSame = checkSame and not self.IsCheckSameDisabled()
        return symlinks.createDirSymLink (linkName, targetDir, checkSame, checkTargetExists, skipIntermediateLinks)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def PerformBindToDir (self, linkName, targetDir, checkSame=True, checkTargetExists=False, skipIntermediateLinks=True):
        return self.PerformBindToDirAction (self, linkName, targetDir, checkSame, checkTargetExists, skipIntermediateLinks)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def PerformBindToFileWithRetries (self, linkName, targetFile, checkSame=True, checkTargetExists=False, skipIntermediateLinks=True, retries=1):
        curTry = 0
        while curTry <= retries:
            try:
                return self.PerformBindToFile (linkName, targetFile, checkSame, checkTargetExists, skipIntermediateLinks)
            except symlinks.SymLinkError as err:
                curTry+=1
                if curTry > retries:
                    raise err
                time.sleep (1)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ShowProvenanceError (self, msg, repo, part=None):
        if not hasattr (self, "m_provErrs"):
            self.m_provErrs = {}

        # Only report each prov error once per build
        if repo in self.m_provErrs:
            return
        self.m_provErrs[repo] = True

        if part:
            msg += " building part {0}".format(part)

        utils.showInfoMsg (msg+"\n", utils.INFO_LEVEL_VeryInteresting, utils.YELLOW)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AddProvenanceString (self, root, repoName, provString, fromLKG, fromSource, repoType):
        repoName = repoName.lower()
        if None == self.m_provenances:
            return
        if not repoName in self.m_provenances:
            self.m_provenances[repoName] = (root, provString, fromLKG, fromSource, repoType)
            return
        if self.m_provenances[repoName][1] == provString:
            return

        prov1 = self.m_provenances[repoName][1]
        was   = self.m_provenances[repoName][0]

        # If the provenance is different and neither one is from LKGs then it should just trigger a rebuild, which is done via this exception.
        if not fromLKG and not self.m_provenances[repoName][2]:
            if fromSource: # Always choose the version from the Source rather than from the Provenance File
                self.m_provenances[repoName] = (root, provString, False, fromSource, repoType)
            raise ProvenanceChanged ("%%%Provenance changed in repository '{0}' [{1}] vs. [{2}]\n"
                .format(repoName, prov1[:12] if prov1 else 'Unkn', provString[:12] if provString else 'Unkn'), repoName, was, self)

        msgTail = '<{1}> refers to version {2} of {0} vs. <{3}> refers to version {4} of {0}\n   {5}\n   {6}\n' \
                .format(repoName, was.m_info.GetShortPartDescriptor(), prov1,
                        root.m_info.GetShortPartDescriptor(), provString,
                        was.m_info.m_file, root.m_info.m_file)

        # Check if the error will be ignored by the strategy and just print a warning now.
        # There is still a chance that all prov errors will be ignored by the command line switch but those are handled at a higher level.
        # We want to differentiate between those we specifically ignore and those with a global ignore (previously FB).
        if 0 == globalvars.buildStrategy.FindProvenanceErrorStrategy (repoName):
            was.ShowProvenanceError ('Provenance inconsistency (ignored by strategy): ' + msgTail, repoName, self)
            return

        raise ProvenanceError ('%%%Provenance inconsistency: ' + msgTail, repoName, was, self)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetProductForProvenance (self):
        parentPart = self.m_provProduct
        # First choice - top level product
        if parentPart:
            if "" != parentPart.GetPrgDirName() and None != parentPart.GetPrgDirName():
                utils.showInfoMsg ("Using product {0} for provenance\n".format (repr(parentPart)), utils.INFO_LEVEL_RarelyUseful)
                return parentPart.m_info.GetShortPartDescriptor(), parentPart.GetPrgDirName()
            else:
                utils.showInfoMsg ("Provenance candidate product {0} has no PRG Dir\n".format (repr(parentPart)), utils.INFO_LEVEL_RarelyUseful)

        utils.showInfoMsg ("Using current part {0} for provenance\n".format (repr(self)), utils.INFO_LEVEL_RarelyUseful)
        return self.m_info.GetShortPartDescriptor(), self.GetPrgDirName()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def WriteProvenanceFile (self, provName = None):
        if None == self.m_provenances:
            return

        if provName == None:
            provName = self.GetProvenanceFileName()
            
        if self.IsClobberBuild():
            utils.showInfoMsg ("Deleting prov file: {0}\n".format (repr(provName)), utils.INFO_LEVEL_RarelyUseful)
            utils.deleteFileWithRetries (provName)
            return

        symlinks.makeSureBaseDirectoryExists (provName)
        with open (provName, "wt") as provFile:
            provFile.write ("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n")
            provFile.write ("<Provenance>\n")

            productName, prgDirName = self.GetProductForProvenance ()
            if productName:
                (relV, majV, minV, subminV) = utils.GetVersionForProvenance ()
                verString = 'ReleaseVersion="{0}" MajorVersion="{1}" MinorVersion="{2}" BuildNumber="{3}" PrgOutputDir="{4}"'.format(relV, majV, minV, subminV, prgDirName)
                provFile.write ("<Product Name=\"{0}\" {1}/>\n".format (productName, verString))

            for name,prov in sorted(self.m_provenances.items()):
                provStr = prov[1]
                if None != provStr:
                    provFile.write ("<Repository Name=\"{0}\" Identifier=\"{1}\" Type=\"{2}\" />\n".format (name, provStr, prov[4]))

            for nugetUsage in self.m_nugetPackages:
                depList = nugetUsage.m_pkgSource.GetPkg().GetAllNugetDepsFromProvenance (nugetUsage.m_pkgFramework)
                for dep in depList:
                    provFile.write ('<NugetPackage Name="{0}" Version="{1}" Framework="{2}" Url="{3}"/>\n'.format (dep[0], dep[1], dep[2], dep[3]))

            provFile.write ("</Provenance>\n")
        provFile.close()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ReadProvenanceFile (self, provName, fromLKG):
        if True != os.access (provName, os.R_OK):
            raise utils.PartBuildError ("Can't open provenance file '{0}'".format(provName), self)

        try:
            provDom = utils.parseXml (provName)
        except xml.parsers.expat.ExpatError as errIns:
            raise utils.PartBuildError("Provenance parse error {0}: {1}".format(provName, errIns.__str__()), self)

        provError = None
        provEl = utils.getDomElementsByName(provDom, "Provenance")
        repoEntries = utils.getDomElementsByName(provEl[0], "Repository")
        for repoDom in repoEntries:
            name = repoDom.getAttribute("Name").lower()
            if "" == name:
                raise utils.PartBuildError ("Invalid provenance file {0}".format(provName), self)
            repoType = repoDom.getAttribute("Type") if repoDom.hasAttribute("Type") else globalvars.REPO_HG
            try:
                self.AddProvenanceString (self, name, repoDom.getAttribute("Identifier"), fromLKG, False, repoType)
            except ProvenanceError as err:
                if None == provError:
                    provError = err

        nugetEntries = utils.getDomElementsByName(provEl[0], "NugetPackage")
        for nugetDom in nugetEntries:
            name = nugetDom.getAttribute("Name").lower()
            if "" == name:
                raise utils.PartBuildError ("Invalid provenance file {0}".format(provName), self)
            url = nugetDom.getAttribute("Url")
            version = nugetDom.getAttribute("Version")
            self.m_nugetProvenance.add ((name, url, version))

        if provError:
            raise provError  # pylint: disable=raising-bad-type

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def _ParseProvenanceVersion (self, repoDom, attrName):
        verStr = repoDom.getAttribute(attrName)
        if not verStr or len (verStr) < 1:
            return None
        try:
            return int(verStr)
        except:
            return None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ReadProvenanceVersion (self, provName):
        if True != os.access (provName, os.R_OK):
            raise utils.PartBuildError ("Can't open provenance file '{0}'".format(provName), self)

        try:
            provDom = utils.parseXml (provName)
        except xml.parsers.expat.ExpatError as errIns:
            raise utils.PartBuildError("Provenance parse error {0}: {1}".format(provName, errIns.__str__()), self)

        provEl = utils.getDomElementsByName(provDom, "Provenance")
        prodNode = utils.getDomElementsByName(provEl[0], "Product")

        if not prodNode or len(prodNode) == 0:
            return None, None, None, None, None

        prodNode = prodNode[0]
        prodName = prodNode.getAttribute("Name")
        relV    = self._ParseProvenanceVersion (prodNode, "ReleaseVersion")
        majV    = self._ParseProvenanceVersion (prodNode, "MajorVersion")
        minV    = self._ParseProvenanceVersion (prodNode, "MinorVersion")
        subminV = self._ParseProvenanceVersion (prodNode, "BuildNumber")
        prgDirName = prodNode.getAttribute("PrgOutputDir")
        if relV == None or majV == None or minV == None or subminV == None:
            return None, None, None, None, None, None
        return prodName, relV, majV, minV, subminV, prgDirName

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetLogDirEntryName (self, filesuffix):
        if self.FromLkgs():
            return os.path.join (self.GetMyBuildContextDir(), "Logs", self.m_info.m_name + filesuffix)

        return os.path.join (self.GetLogFileRoot(), self.m_info.m_buildContext, self.m_info.m_name + filesuffix)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def LinkLogFile(self):
        # The logfile is linked into the buildcontext as a flag; if it is there we know that we don't need to rebuild.
        #   The log is created both failure and success, but it is only linked on a success.
        logtarget = self.GetLogDirEntryName (LOG_FILE_SUFFIX)
        if not os.path.isfile(logtarget):       # no buildcmd
            return

        # Sometimes the logfile takes a while to close.  Loop on symlinking.  Do it at this level so it only blocks one thread.
        count = 30 if self.m_action.GetNumThreads() > 1 else 3
        linked = False
        error = (None, None)
        while count > 0:
            try:
                self.PerformBindToFile (self.GetBaseLogName() + LOG_FILE_SUFFIX, logtarget, False, False, skipIntermediateLinks=False)
                linked = True
                break
            except utils.BuildError as err:
                error = (err.errmessage, err.stackTrace)
                time.sleep (1)
            except Exception as err:
                error = (str(err), None)
                time.sleep (1)
            count -= 1
        if not linked:
            raise utils.PartBuildError ("LogFile error: {0}".format(error[0]), self, trace=error[1])

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def LinkProvenance(self):
        linkName = self.GetBaseLogName()+globalvars.provfileName
        if self.IsClobberBuild():
            if symlinks.symlinkTargetExists (linkName):
                symlinks.deleteSymLink (linkName)
        else:
            self.PerformBindToFile (linkName, self.GetLogDirEntryName(globalvars.provfileName), False, False)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ExitOnError (self, errVal, message="", postErrorMsgCallback=None, trace=None):
        """Exit with status, except if 'ignoreErrors' flag is on. postErrorMsgCallback is a callback that is invoked after the error message is printed so users can print suggestions etc."""
        if not message.endswith ('\n'):
            message += '\n'
        ignoringErrors = self.m_action.ShouldIgnoreErrors()

        if ignoringErrors:
            igMessage = message
            if not igMessage.endswith ('\n'):
                igMessage += '\n'
            utils.showInfoMsg (igMessage, utils.INFO_LEVEL_Essential, utils.RED)
            utils.appendBuildError (igMessage)

        if None != postErrorMsgCallback:
            postErrorMsgCallback (errVal, message)

        if not ignoringErrors:
            raise utils.FatalError (errVal, message, trace=trace)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def BuildWithLogFile (self, cmd, _languages=None, localEnv=None):
        if None == localEnv:
            localEnv = globalvars.buildStrategy.GetEnv()

        if self.m_isMultiPlatform:
            utils.appendEnvVar (localEnv, "InstallerDir", buildpaths.getInstallerRoot(self.m_info.m_platform, self.m_info.m_static) + os.sep)
            utils.appendEnvVar (localEnv, "CombinedInstallerDir", self.GetCombinedInstallerRoot() + os.sep)
            utils.appendEnvVar (localEnv, "ProductDir", self.GetProductRoot() + os.sep)

            def getProductDirString (platform):
                return self.GetProductRootForPlatform(platform) + os.sep

            targetplatform.CreateAllEnvironmetVariables ('ProductDir', getProductDirString, localEnv)

        def getDirString (platform):
            return os.path.join (buildpaths.getOutputRoot(platform, self.m_info.m_static), buildpaths.DIRNAME_BUILDCONTEXTS, self.m_info.m_buildContext)

        if hasattr (self.m_action, 'm_envFromArgs'):
            localEnv.update (self.m_action.m_envFromArgs)

        utils.appendEnvVar (localEnv, "PartFileName", self.m_info.m_buildContext)  # useful for keeping module build dirs separate and manageable.
        utils.appendEnvVar (localEnv, "BuildContext", self.GetMyBuildContextDir()+os.sep)
        utils.appendEnvVar (localEnv, "BuildToolCache", buildpaths.GetToolsOutputRoot()+os.sep)
        # I'm not sure why we need all of these at once, but so be it.
        targetplatform.CreateAllEnvironmetVariables ('BuildContext', getDirString, localEnv)
        utils.appendEnvVar (localEnv, "OutputRootDir", buildpaths.getOutputRoot(self.GetPlatform(), self.IsStatic())+os.sep)
        utils.appendEnvVar (localEnv, "OutBuildContexts", os.path.join(buildpaths.getOutputRoot(self.GetPlatform(), self.IsStatic()), "BuildContexts")+os.sep)
        utils.appendEnvVar (localEnv, "PartBuildDir", self.GetPartBuildDir())
        utils.appendEnvVar (localEnv, "DEFAULT_TARGET_PROCESSOR_ARCHITECTURE", self.GetPlatform().GetArchitecture())
        utils.appendEnvVar (localEnv, "TRANSKIT_TOOLS_DIR", translationkit.TranskitToolsDir (self.GetPlatform())+os.sep)
        if globalvars.buildStrategy.GetToolsetForPlatform (self.GetPlatform()):
            utils.appendEnvVar (localEnv, "BB_DEFAULT_TOOLSET", globalvars.buildStrategy.GetToolsetForPlatform (self.GetPlatform()))
        if globalvars.buildStrategy.GetToolVersionForPlatform (self.GetPlatform()):
            utils.appendEnvVar (localEnv, "TOOL_VERSION", globalvars.buildStrategy.GetToolVersionForPlatform (self.GetPlatform()))
        if globalvars.buildStrategy.GetDotNetRuntimeForPlatform (self.GetPlatform()):
            utils.appendEnvVar (localEnv, "DEFAULT_TARGET_FRAMEWORK_VERSION", globalvars.buildStrategy.GetDotNetRuntimeForPlatform (self.GetPlatform()))
        if globalvars.buildStrategy.GetWindowsSdkVersionForPlatform (self.GetPlatform()):
            utils.appendEnvVar (localEnv, "DEFAULT_TARGET_WINDOWS_SDK_VERSION", globalvars.buildStrategy.GetWindowsSdkVersionForPlatform (self.GetPlatform()))
        if self.m_configFile:
            utils.appendEnvVar (localEnv, "BB_DEFAULT_CONFIGURATION_FILE", self.m_configFile)
        if self.m_projectFile:
            utils.appendEnvVar (localEnv, "MSBUILD_PROJECT_PATH", self.m_projectFile)
            utils.appendEnvVar (localEnv, "MSBUILD_PROPERTIES_PATH", self.GetPropertiesPath())
        if self.m_dotNetSdkProjectFile:
            utils.appendEnvVar (localEnv, "DOTNET_SDK_PROJECT_PATH", self.m_dotNetSdkProjectFile)
            utils.appendEnvVar (localEnv, "DOTNET_SDK_PROPERTIES_PATH", self.GetPropertiesPath())
            utils.appendEnvVar (localEnv, "BMAKE_CLEANER_ENVIRONMENT", "1")
            utils.appendEnvVar (localEnv, "DOTNET_SDK_PROJECT_BUILD", "1")

        versionutils.pushPartStrategyVersionToEnv (localEnv, self)

        stat = [1]

        if not self.m_action.m_actionOptions.tkOnly:
            cwdir = self.m_info.m_partFileDir

            def doLogging (logFile):
                utils.showAndLogMsg ("_"*90+"\r\n\r\n", logFile, utils.INFO_LEVEL_SomewhatInteresting)
                utils.showAndLogMsg (" Building Part: {0}\r\n".format(self), logFile, utils.INFO_LEVEL_SomewhatInteresting)
                utils.showAndLogMsg (" Date:          {0}\r\n".format(utils.getNow()), logFile, utils.INFO_LEVEL_SomewhatInteresting)
                utils.showAndLogMsg (" Username:      {0}\r\n".format(os.getenv("username")), logFile, utils.INFO_LEVEL_SomewhatInteresting)
                utils.showAndLogMsg (" ComputerName:  {0}\r\n".format(os.getenv("COMPUTERNAME")), logFile, utils.INFO_LEVEL_SomewhatInteresting)
                utils.showAndLogMsg (" UserDnsDomain: {0}\r\n\r\n".format(os.getenv("USERDNSDOMAIN")), logFile, utils.INFO_LEVEL_SomewhatInteresting)
                utils.showAndLogMsg (" Build Command:\r\n cd {0} & {1}\r\n".format(cwdir, ' '.join(cmd)), logFile, utils.INFO_LEVEL_SomewhatInteresting)
                utils.showAndLogMsg ("_"*90+"\r\n\r\n", logFile, utils.INFO_LEVEL_SomewhatInteresting)

                stat[0] = cmdutil.runAndLogWithEnv (cmd, localEnv, logFile, cwdir)

            try:
                self.__DoWithLogFile (doLogging)
            except OSError as err:
                if not self.m_action.ShouldIgnoreErrors():
                    raise utils.PartBuildError (cmd[0] + " : " + str(err), self)
        else:
            stat[0] = 0

        return stat[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def BuildTranskits (self):
        if not self.FromLkgs():
            self.BuildDocumentation ()

        def doLogging (logFile):
            if None != self.m_bindings:
                self.m_bindings.BuildTranskits (self, logFile)

        self.__DoWithLogFile (doLogging)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def CleanTranskits (self):
        # Why CleanTranskit is a separate function.
        # 1. The bindings for Transkit object are obtained from Transkit.xml. This files lives in BuildContext folder.
        #       If RunBuildCommand executed before CleanTranskit, the there may not be a transkit.xml. We will not be able
        #       to find output file, which needs to be deleted.
        # 2. CleanTranskits needs to be called for items from LKG as well. Since, transkit do not have a lkg system.
        #       the behavior is same as that of BuildTranskits
        if not translationkit.IsClobberBuild ():
            return

        def doLogging (logFile):
            if None != self.m_bindings:
                self.m_bindings.BuildTranskits (self, logFile, cleanTranskit=True)

        self.__DoWithLogFile (doLogging)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ReportFailAndClobberBuild (self):
        utils.showInfoMsg ("Part {0} Failed, cleaning\n".format (self.m_info.m_name), utils.INFO_LEVEL_Important, utils.PINK)
        self.m_action.m_actionOptions.clobberBuild = True
        self.m_action.PrepareToReprocessPart (self)
        self.RunBuildCmd() #perform the clean for failing part
        self.m_action.m_actionOptions.clobberBuild = False
        utils.cleanDirectory (self.GetPartBuildDir(), deleteFiles=True)
        utils.showInfoMsg ("Attempting Retry of build for Part {0}\n".format (self.m_info.m_name), utils.INFO_LEVEL_Important, utils.PINK)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def HasBuiltLKGs (self):
        """Checks if the LKGs are upto date."""

        logfile = self.GetLogDirEntryName(globalvars.provfileName)
        logLink = self.GetBaseLogName()+globalvars.provfileName
        if not self.m_action.m_actionOptions.forceBuild and os.path.isfile(logfile) and os.path.exists(logLink) and symlinks.isTargetSame (logLink, logfile):
            utils.showInfoMsg ("%%%LKGs <{0}> has already been processed, not rebuilding\n".format (self.m_info.m_name), utils.INFO_LEVEL_Interesting)
            utils.showInfoMsg ("   To force a rebuild, use '-f' or delete '{0}'\n". format (logLink), utils.INFO_LEVEL_Interesting)
            return True

        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def BuildDocumentation (self):
        if not self.m_bindings:
            return

        if not self.m_bindings.m_documentations or len (self.m_bindings.m_documentations) == 0:
            return

        for binding in self.m_bindings.m_documentations:
            if not binding.m_docItem:
                continue

            files = self.SplitBindings (binding.m_val)
            if len (files) == 0:
                continue

            # One documentation binding to talk about only one file.
            targetFile = os.path.join (self.GetMyBuildContextDir (isDocumentationBuildContext=True), files [0])
            source  = os.path.join (binding.m_docItem.m_localDir, os.path.basename (files [0]))

            if not os.path.exists (source) and translationkit.IsTranskitShell ():
                msg = "Using temp file for Documentation Source is not found {0}.".format (source)
                source = translationkit.TranskitTempFile (self.GetPlatform(), msg)
                msg = msg + "TempFilePath={0}\r\n".format (source)
                utils.showInfoMsg (msg, utils.INFO_LEVEL_VeryInteresting, utils.YELLOW)

            symlinks.makeSureBaseDirectoryExists (targetFile)
            if self.IsClobberBuild():
                symlinks.deleteSymLink (targetFile)
                return

            if not os.path.exists (source):
                raise utils.PartBuildError ("Cannot find Documentation {0}\r\n".format (source), self)

            symlinks.createFileSymLink (targetFile, source)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetNpmBuildCmd(self):
        if self.IsClobberBuild():
            return ["npm.cmd", "run-script", "clean", "--prefix", os.path.dirname(self.m_npmFile)]
        else:
            return ["npm.cmd", "run-script", "build", "--prefix", os.path.dirname(self.m_npmFile)]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetYarnBuildCmd(self):
        if self.IsClobberBuild():
            return ["yarn.cmd", "--cwd", os.path.dirname(self.m_yarnFile), "run", "clean"]
        else:
            return ["yarn.cmd", "--cwd", os.path.dirname(self.m_yarnFile), "run", "build"]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetBMakeBuildCmd(self, additionalBmakeOptions):
        bmakeToolRoot = os.path.join (buildpaths.GetToolsOutputRoot(), 'bmake')
        if self.m_useBBmake:
            path = os.path.join(bmakeToolRoot, 'BentleyBuildMake')
            if  self.GetPlatform().UseEXE():
                path = path + ".exe"
            path = os.path.realpath (path)
        else:
            if sys.platform != 'win32':
                self.ExitOnError (1, 'Bmake is only supported on Windows. On other platforms please use BentleyBuildMake.')
            path = os.path.join(bmakeToolRoot, 'bmake.exe')
        buildCmd = [path]

        if None != self.m_bmakeOptions and "" != self.m_bmakeOptions:
            buildCmd.extend (self.m_bmakeOptions.split())

        if additionalBmakeOptions:
            buildCmd.extend (additionalBmakeOptions.split())

        if None != self.m_apiNumber and "" != self.m_apiNumber:
            buildCmd.append ("+dDLM_API_NUMBER=" + self.m_apiNumber)

        if self.IsStatic():
            buildCmd.append ('+dCREATE_STATIC_LIBRARIES=1')

        # Check if this part is marked as building the Feature Aspects.  If so, get the details from the product.
        if self.m_featureAspectPart:
            if not self.m_featureAspectProduct:
                errMsg = 'Failed to find a product with a FeatureAspects definition as a parent for part\n   {0}\n'.format (self)
                self.ExitOnError (1, errMsg)
            buildCmd.append ('+dTemplateFile='+self.m_featureAspectProduct.m_featureAspects[0])
            buildCmd.append ('+dInstanceFile='+self.m_featureAspectProduct.m_featureAspects[1])
            # This is used by FeatureAspects.mke.  It is required to generate the Trusted Application List (TAL) based on the MA's and CLRApps in the output tree.
#            buildCmd.append ('+dProductDeliveryDir='+self.m_featureAspectProduct.GetDeliveryRoot() + os.sep)

        if self.IsClobberBuild():
            buildCmd.append ("+Da")
        elif self.m_action.m_actionOptions.ignoreBmakeErrors:
            buildCmd.append ("+im")

        # Replace a few $(macro) expressions in m_bmakeFile
        if self.m_projectFile:
            makefilepath = utils.GetSharedMkiFile("projbuild.mke")
        elif self.m_dotNetSdkProjectFile:
            makefilepath = utils.GetSharedMkiFile("dotnetSdkProjectBuild.mke")
        elif self.m_bmakeFile.lower().startswith ('$(buildcontext)'):
            makefilepath = os.path.join (self.GetMyBuildContextDir(), self.m_bmakeFile[15:].lstrip('\\/'))
        elif self.m_bmakeFile.lower().startswith ('$(product)'):
            makefilepath = os.path.join (self.GetProductRoot(), self.m_bmakeFile[10:].lstrip('\\/'))
        else:
            makefilepath = self.m_bmakeFile

        bmakeFile = os.path.join (self.m_info.m_partFileDir, os.path.expandvars(makefilepath))

        buildCmd.append (os.path.abspath(bmakeFile))

        return buildCmd

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def RunBuildCmd (self, additionalBmakeOptions=None):
        """Execute the build command for this part."""
        partStrategy = self.m_info.GetPartStrategy ()
        if not self.m_info.m_requiresBuild and partStrategy.m_buildFromSource == 1: # Never
            return

        # Use an array due to python closure rules.
        didShowHeader = [False]
        def showPartBuildingHeader():
            if not didShowHeader[0]:
                didShowHeader[0] = True
                utils.showInfoMsg ( utils.getSeparatorLine(), utils.INFO_LEVEL_VeryInteresting )
                buildingPartOutput = "Building {0} ({1}) {2}".format (buildpaths.getShortPartDescriptor (self.m_info.m_file, self.m_info.m_name), self.GetPlatform().GetXmlName(), '[static]' if self.IsStatic() else '')
                utils.showInfoMsg ("\n" + buildingPartOutput + "\n", utils.INFO_LEVEL_Essential, utils.LIGHT_BLUE)

        if partStrategy.m_buildFromSource == 2: # Once
            logfile = self.GetLogFileName()
            try:
                provFile = self.GetProvenanceFileName()
                if os.path.isfile (provFile):
                    self.ReadProvenanceFile (provFile, False)
            except ProvenanceChanged as err: # The case where the source has changed and we need to rebuild.
                showPartBuildingHeader()
                utils.showInfoMsg (err.errmessage+"\n", utils.INFO_LEVEL_Interesting)
                utils.showInfoMsg ("%%%Part <{0}> was built with a different version of '{1}', rebuilding\n".format(self.m_info.m_name, err.m_repo), utils.INFO_LEVEL_VeryInteresting)
                # Remove the logfile to force the rebuild.
                if os.path.isfile (logfile):
                    os.remove (logfile)
            except ProvenanceError as err:
                err.m_first.ShowProvenanceError (err.errmessage, err.m_repo)
            except utils.PartBuildError as err:
                showPartBuildingHeader()
                utils.showInfoMsg (err.errmessage+"\n", utils.INFO_LEVEL_Interesting)
                utils.showInfoMsg ("%%%Part <{0}> is out of date, rebuilding\n".format(self), utils.INFO_LEVEL_VeryInteresting)
                if os.path.isfile (logfile):
                    os.remove (logfile)

            if not self.m_action.m_actionOptions.forceBuild and os.path.isfile(logfile) and symlinks.isTargetSame (self.GetBaseLogName() + LOG_FILE_SUFFIX, logfile):
                utils.showInfoMsg ("%%%Part <{0}> has already been built once, not rebuilding\n".format (self.m_info.m_name), utils.INFO_LEVEL_Interesting)
                utils.showInfoMsg ("   per {0}\n".format (partStrategy), utils.INFO_LEVEL_SomewhatInteresting)
                utils.showInfoMsg ("   To force a rebuild, use '-f' or delete '{0}'\n". format (logfile), utils.INFO_LEVEL_Interesting)
                return

        self.WriteProvenanceFile ()
        if not self.m_bmakeFile and not self.m_projectFile and not self.m_dotNetSdkProjectFile and not self.m_npmFile and not self.m_yarnFile:
            return

        showPartBuildingHeader()

        buildContextDir = self.GetMyBuildContextDir()
        symlinks.makeSureDirectoryExists (buildContextDir)

        if self.m_bmakeFile or self.m_projectFile or self.m_dotNetSdkProjectFile:
            buildCmd = self.GetBMakeBuildCmd(additionalBmakeOptions)
        elif self.m_npmFile:
            buildCmd = self.GetNpmBuildCmd()
        elif self.m_yarnFile:
            buildCmd = self.GetYarnBuildCmd()

        utils.showInfoMsg ("\n" + " ".join(buildCmd) + "\n", utils.INFO_LEVEL_VeryInteresting)

        sys.stderr.flush()
        sys.stdout.flush()

        logfile = self.GetBaseLogName() + LOG_FILE_SUFFIX # before we run the buildcmd, remove logfile from buildcontext
        if os.path.isfile (logfile):
            os.remove (logfile)

        errMsg = ""
        try:
            stat = self.BuildWithLogFile (buildCmd)
            if 0 == stat:
                if not self.m_action.m_actionOptions.ignoreBmakeErrors and not self.IsClobberBuild():
                    self.LinkLogFile()
                return
            else:
                errMsg = "!!!BuildCommand '{0}' failed.".format (' '.join(buildCmd))
        except OSError as err:
            errMsg = "!!!BuildCommand '{0}' failed: {1}".format (' '.join(buildCmd), err.strerror)

        except utils.PartBuildError as err:
            errMsg = "!!!Error: {0}".format (err.errmessage)

        except utils.BuildError as err:
            errMsg = "!!!BuildCommand '{0}' failed: {1}".format (' '.join(buildCmd), err.errmessage)

        errMsg += self.fullPath()

        logFileName = self.GetLogFileName()
        if os.path.isfile (logFileName):
            errMsg += "\n    See logfile: {0}\n".format(logFileName)
            if hasattr (self.m_action, "OnBuildError"):
                self.m_action.OnBuildError (logFileName)

        if 2 == partStrategy.m_onError:
            utils.showInfoMsg ("%%%Continuing despite errors for part {0} per {1}\n".format (self, partStrategy), utils.INFO_LEVEL_Important)
            return

        # auto-retry means that when a build fails, clean the part and then try again. If it still fails, stop.
        if self.m_action.m_actionOptions.autoRetry and not self.IsClobberBuild():
            self.m_action.m_actionOptions.autoRetry = False # so second failure will stop
            self.ReportFailAndClobberBuild ()          # report the failure and do a clobber build on this part.
            self.RunBuildCmd()                         # attempt to re-build it.
            self.m_action.m_actionOptions.autoRetry = True #if we got here, the second try worked, keep going.
            return

        def doRebuildSuggestion (_errVal, _errMsg):
            suggestion = "\nSuggested command to rebuild failing part:\n"
            utils.showInfoMsg (suggestion, utils.INFO_LEVEL_Essential, utils.LIGHT_BLUE)
            suggestion = "    {0}\n".format (self.GetRebuildCommand ())
            utils.showInfoMsg (suggestion, utils.INFO_LEVEL_Essential)

        self.ExitOnError (1, errMsg, doRebuildSuggestion)

    #--------------------------------------------------------------------------------
    # @bsimethod
    #--------------------------------------------------------------------------------
    def GetRebuildCommand (self):
        rootPart = self.m_action.m_bgraph.m_graph.PartNode (self.m_action.m_bgraph.m_rootNode).m_part
        if self.m_action.GetNumThreads() != 0:
            return "bb -s {0} -r {1} -f {2} -p {3} -a{4} re -A {7} -L {8} {5}:{6}".format (globalvars.programOptions.buildStrategy, rootPart.m_info.m_repo.m_name,
             rootPart.m_info.GetFileRelativeToRepo(), rootPart.m_info.m_name, globalvars.programOptions.platform.m_option, self.m_info.m_buildContext, self.m_info.m_name,
             self.GetPlatform().GetOption(), self.m_currentLibType.lower())
        else:
            return "bb -s {0} -a{1} re -A {4} -L {5} {2}:{3}".format (globalvars.programOptions.buildStrategy, globalvars.programOptions.platform.m_option,
             self.m_info.m_buildContext, self.m_info.m_name, self.GetPlatform().GetOption(), self.m_currentLibType.lower())

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ReadSubParts (self, dom):
        subPartList = []

        for subpart in dom.childNodes:
            if subpart.nodeType == subpart.ELEMENT_NODE:
                partType = PART_TYPE_Part
                if subpart.nodeName == "SubProduct":
                    subpartName = subpart.getAttribute ("ProductName")
                    if "" == subpartName:
                        raise utils.PartBuildError ("No ProductName for SubProduct", self, subpart)
                    partType = PART_TYPE_Product
                elif subpart.nodeName == "SubNuGetProduct":
                    subpartName = subpart.getAttribute ("NuGetProductName")
                    if "" == subpartName:
                        raise utils.PartBuildError ("No NuGetProductName for SubNuGetProduct", self, subpart)
                    partType = PART_TYPE_NuGetProduct
                elif subpart.nodeName == "SubUPackProduct":
                    subpartName = subpart.getAttribute ("UPackProductName")
                    if "" == subpartName:
                        raise utils.PartBuildError ("No UPackProductName for SubUPackProduct", self, subpart)
                    partType = PART_TYPE_UPackProduct
                elif subpart.nodeName == "SubPackage":
                    pkgName = subpart.getAttribute ("Name")
                    if "" == pkgName:
                        raise utils.PartBuildError ("No Name for SubPackage", self, subpart)
                    pkgType = subpart.getAttribute ("PkgType")
                    if "" == pkgType:
                        raise utils.PartBuildError ("No PkgType for SubPackage {0}".format(pkgName), self, subpart)
                    if not pkgType.lower() in PackageTypes:
                        raise utils.PartBuildError ("Invalid PkgType '{0}' for SubPackage {1}. Valid types: {2}".format(pkgType, pkgName, PackageTypes), self, subpart)
                    # This is really a pass-through to the actual part so use the real partName
                    pkgSource = None
                    if pkgType.lower() == PACKAGE_TYPE_Upack:
                        pkgSource = globalvars.buildStrategy.GetUpackSource(pkgName, None)
                    if not pkgSource:
                        raise utils.PartBuildError ("Cannot find package source for SubPackage {0}".format(pkgName), self, subpart)

                    # Can be overridden on the part but normally use strategy
                    subpartName = subpart.getAttribute ("PartName")
                    if "" == subpartName:
                        subpartName = pkgSource.m_partName
                    if not subpartName:
                        raise utils.PartBuildError ("No PartName for SubPackage {0}".format(pkgName), self, subpart)
                    partType = PART_TYPE_Package
                elif subpart.nodeName == "SubMultiPlatform":
                    subpartName = subpart.getAttribute ("Name")
                    if "" == subpartName:
                        raise utils.PartBuildError ("No Name for SubMultiPlatform", self, subpart)
                    partType = PART_TYPE_MultiPlatform
                elif subpart.nodeName == "SubPart":
                    subpartName = subpart.getAttribute ("PartName")
                    if "" == subpartName:
                        raise utils.PartBuildError ("No PartName for SubPart", self, subpart)
                elif subpart.nodeName == "SubToolPart":
                    subpartName = subpart.getAttribute ("Name")
                    if "" == subpartName:
                        raise utils.PartBuildError ("No Name for SubToolPart", self, subpart)
                    partType = PART_TYPE_ToolPart
                elif subpart.nodeName == "SubToolPackage":
                    pkgName = subpart.getAttribute ("Name")
                    if "" == pkgName:
                        raise utils.PartBuildError ("No Name for SubToolPackage", self, subpart)
                    pkgType = subpart.getAttribute ("PkgType")
                    if "" == pkgType:
                        raise utils.PartBuildError ("No PkgType for SubToolPackage {0}".format(pkgName), self, subpart)
                    if not pkgType.lower() in PackageTypes:
                        raise utils.PartBuildError ("Invalid PkgType '{0}' for SubToolPackage {1}. Valid types: {2}".format(pkgType, pkgName, PackageTypes), self, subpart)
                    # This is really a pass-through to the actual part so use the real partName
                    pkgSource = None
                    if pkgType.lower() == PACKAGE_TYPE_Upack:
                        pkgSource = globalvars.buildStrategy.GetUpackSource(pkgName, None)
                    if not pkgSource:
                        raise utils.PartBuildError ("Cannot find package source for SubToolPackage {0}".format(pkgName), self, subpart)

                    # Can be overridden on the part but normally use strategy
                    subpartName = subpart.getAttribute ("ToolPartName")
                    if "" == subpartName:
                        subpartName = pkgSource.m_partName
                    if not subpartName:
                        raise utils.PartBuildError ("No ToolPartName for SubToolPackage {0}".format(pkgName), self, subpart)
                    partType = PART_TYPE_ToolPackage
                else:
                    continue

                if self.m_info.m_fromLKG:
                    skipLoadingSubPart = utils.getOptionalBoolAttr ("SkipIfInLKG", False, "Skip loading SubPart if the parent is coming from LKGs", subpart)
                    if skipLoadingSubPart:
                        utils.showInfoMsg("Skipping load of SubPart {} and its dependencies into graph for Part {} because it comes from LKGs and SkipIfInLkg attribute is set to true\n".format(subpartName, self.m_info.m_name), utils.INFO_LEVEL_SomewhatInteresting)
                        continue

                subPartInfo = SubPartInfo (subpartName, partType)

                platformStr = subpart.getAttribute ("Platform")
                platform = None
                if "" != platformStr:
                    platform = targetplatform.FindPlatformByXMLName (platformStr)
                    if not platform:
                        raise utils.PartBuildError ("Part has invalid platform '{0}', must be one of {1}.".format(platformStr, targetplatform.GetXmlOptions().__str__()), self, subpart)
                else:
                    platform = None

                if platform != None and self.GetPlatform() != platform:
                    self.m_isMultiPlatform = True

                subPartInfo.SetPlatform (platform)

                if partType == PART_TYPE_Package or partType == PART_TYPE_ToolPackage:
                    # Packages are pass-through so we need to update the repo and file name
                    subPartInfo.SetRepo (pkgSource.m_alias.lower())
                    partFile = subpart.getAttribute ("PartFile") or pkgSource.m_partFile
                    if not partFile:
                        raise utils.PartBuildError ("SubPackage {} should have either both PartName and PartFile defined or neither".format(subpartName), self, subpart)
                    subPartInfo.SetFile (partFile)
                    subPartInfo.m_pkgType = pkgType.lower()
                else:
                    subPartInfo.SetRepo (subpart.getAttribute ("Repository"))
                    subPartInfo.SetFile (subpart.getAttribute ("PartFile"))

                if partType == PART_TYPE_ToolPart or partType == PART_TYPE_ToolPackage:
                    includeInTranskit = utils.getOptionalBoolAttr ("IncludeInTranskit", False, "Include tool when creating the transkit", subpart)
                    if includeInTranskit:
                        subPartInfo.m_includeToolInTranskit = True

                libType = subpart.getAttribute ("LibType")
                if 'static' == libType.lower():
                    subPartInfo.SetLibtype (globalvars.LIB_TYPE_Static)
                elif 'dynamic' == libType.lower():
                    subPartInfo.SetLibtype (globalvars.LIB_TYPE_Dynamic)

                bindToDirectory = subpart.getAttribute ("BindToDirectory")
                if bindToDirectory and bindToDirectory != "":
                    subPartInfo.SetBindToDirectory (bindToDirectory)

                subPartList.append (subPartInfo)

        return subPartList

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AfterDoSubpart (self):
        pass

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def _getApiLinkDir (self, apiName, binding, buildContext, linkStaticToDynamic):
        # The idea is to choose the correct output dir, and adjust it so that it will always
        #   choose to link to the dynamic dir.  This is less confusing than having it link to
        #   whichever part (static or dynamic) happens to come up first, and it should be the
        #   same header files.

        domain = binding.m_val
        outputAPIDir = os.path.join (apiName, binding.m_target if binding.m_target else domain)
        sourceDir = os.path.join(self.GetMyBuildContextDir (), outputAPIDir)

        if linkStaticToDynamic:
            targetDir = os.path.join(self.GetOutputBuildContextDir(buildContext, isStatic=False), outputAPIDir)
            if os.path.exists(targetDir):
                # either it's already the static target, or it's the dynamic target and we don't want to change it
                if not symlinks.checkSameTarget (targetDir, sourceDir):
                    return True, None, None
        else:
            targetDir = os.path.join(self.GetOutputBuildContextDir(buildContext), outputAPIDir)

            if os.path.exists(targetDir):
                # if it's not the dynamic target, then make sure it's the static target
                if not symlinks.checkSameTarget (targetDir, sourceDir):
                    # Can't shortcut and return that we should skip because PerformBindToDir is overridden by LKGs.

                    staticSourceDir = os.path.join(self.GetMyBuildContextDir (isStatic=True), outputAPIDir)
                    if os.path.exists (staticSourceDir) and symlinks.checkSameTarget (targetDir, staticSourceDir):
                        # Delete the static target so dynamic will replace it
                        symlinks.deleteSymLink (targetDir)

        return False, sourceDir, targetDir

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def _BindAPI (self, buildContext, linkStaticToDynamic, bindingsToProcess, apiDir):
        # Bind either public API or vendor API with a retry to handle the race condition.
        def LinkDir (binding, buildContext, linkStaticToDynamic):
            skip, sourceDir, targetDir = self._getApiLinkDir (apiDir, binding, buildContext, linkStaticToDynamic)
            if skip:
                return

            self.PerformBindToDir (targetDir, sourceDir, checkTargetExists=True, skipIntermediateLinks=True)

        for binding in bindingsToProcess:
            try:
                LinkDir (binding, buildContext, linkStaticToDynamic)
            except symlinks.SymLinkError as err:
                try:
                    # Give it one retry. There is a race condition that occurs when linking static and dynamic partfiles.
                    time.sleep (1)
                    LinkDir (binding, buildContext, linkStaticToDynamic)
                except symlinks.SymLinkError as err:
                    raise utils.PartBuildError ("Binding error: " + err.errmessage, self, trace=err.stackTrace)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def BindPublicAPI (self, buildContext, linkStaticToDynamic):
        return self._BindAPI (buildContext, linkStaticToDynamic, self.m_bindings.m_publicAPI, 'PublicAPI')

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def BindVendorAPI (self, buildContext, linkStaticToDynamic):
        return self._BindAPI (buildContext, linkStaticToDynamic, self.m_bindings.m_vendorAPI, 'VendorAPI')

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ShowFileBindingError (self, sourceName, child, binding):
        errmsg = "File Binding {0} does not exist\n".format (sourceName)
        if binding.m_ifNotPresent == 0:
            raise utils.PartBuildError ("#### " + errmsg, child)
        if binding.m_ifNotPresent == 1 and not binding.m_alreadyWarned:
            binding.m_alreadyWarned = True
            utils.showInfoMsg ("#### Warning: " + errmsg, utils.INFO_LEVEL_VeryInteresting, utils.YELLOW)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:Part
    #-------------------------------------------------------------------------------------------
    def GetBindingLock (self):
        return g_bindingLock

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def BindFileListAction (self, _thisPart, sourceList, _buildContext, target, binding, targetDir, _linkStaticToDynamic, ignoreMissingSourceFiles, handleSecondaryList, filterFunc):
        # This one is the one that gets overridden in SaveLKGs.  It needs the part argument.
        sourceDir = self.GetMyBuildContextDir ()

        curFile = 0
        for source in sourceList:
            sourceName = os.path.join(sourceDir, source)
            sourceFiles = glob.glob (sourceName)

            if len(sourceFiles) == 0:
                if ignoreMissingSourceFiles:
                    continue
                self.ShowFileBindingError (sourceName, self, binding)

            secondaryList = None
            if handleSecondaryList and binding.m_secondary and not self.IsStatic():
                secondaryList = self.SplitBindings (binding.m_secondary)

            for sourceFile in sourceFiles:
                _, sourceTail= os.path.split (sourceFile)
                with self.GetBindingLock():
                    if filterFunc and filterFunc (sourceFile, os.path.join (targetDir, sourceTail), self):
                        continue
                    try:
                        if None != target and "" != target or self.m_bindAlways:
                            self.PerformBindToFile (os.path.join (targetDir, sourceTail), sourceFile, checkSame=True, checkTargetExists=False, skipIntermediateLinks=True)
                        # For assemblies with an API number we want to also link a version of the DLL without the API number in it
                        if handleSecondaryList and None != secondaryList:
                            secondaryFile = os.path.basename (secondaryList[curFile])
                            self.PerformBindToFile (os.path.join(targetDir, secondaryFile), os.path.join(targetDir, sourceTail), checkSame=True, checkTargetExists=False)
                    except symlinks.SymLinkError as err:
                        raise utils.PartBuildError ("Binding error: " + err.errmessage, self, trace=err.stackTrace)
            curFile += 1

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def BindFileList (self, sourceList, buildContext, target, binding, targetDir, linkStaticToDynamic, ignoreMissingSourceFiles=False, handleSecondaryList=True, filterFunc=None):
        return self.BindFileListAction (self, sourceList, buildContext, target, binding, targetDir, linkStaticToDynamic, ignoreMissingSourceFiles, handleSecondaryList, filterFunc)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def UpdateBindingsForLanguages (self):
        return

    #--------------------------------------------------------------------------------
    # @bsimethod
    #--------------------------------------------------------------------------------
    def SplitBindings (self, binding, _languages=None):
        bindings = self._SplitBindings(binding)
        expandedList = []
        for binding in bindings:
            languageMacro = "$(language)"
            if -1 == binding.lower().find (languageMacro):
                expandedList.append (binding)
            else:
                expandedString = binding.replace (languageMacro, "")
                expandedList.append (expandedString)

        return expandedList

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def _SplitBindings (self, binding):
        bindings = utils.splitSources(binding)
        return bindings

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def BindMiscFiles (self, buildContext, languages, linkStaticToDynamic):
        """Bind to all outer parts, all files mentioned in Bindings section for a subpart."""
        treatAsStatic = False if linkStaticToDynamic else self.IsStatic()
        for binding in self.m_bindings.m_miscFiles:
            if binding.m_addToTranskitTools:
                translationkit.PutFilesToTranskitTools (binding, self)
                continue

            target = binding.m_target
            if not target:
                target = os.path.join('SubParts', 'Files')
            targetDir = os.path.join(self.GetOutputBuildContextDir(buildContext, isStatic=treatAsStatic), target)
            self.BindFileList (self.SplitBindings (binding.m_val, languages), buildContext, target, binding, targetDir, linkStaticToDynamic)

            if binding.m_productDirectory.lower () == "transkittools":
                if not self.IsClobberBuild():
                    translationkit.PutFilesToTranskitTools (binding, self)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def BindLibFiles (self, buildContext, _languages, linkStaticToDynamic):
        """Bind to all outer parts, all files mentioned in Bindings/Libs section into Subparts/Lib."""
        for binding in self.m_bindings.m_libFiles:
            if linkStaticToDynamic:
                target = os.path.join("SubParts", "StaticLibs")
                targetDir = os.path.join (self.GetOutputBuildContextDir(buildContext, isStatic=False), target)
            else:
                target = os.path.join("SubParts", "Libs")
                targetDir = os.path.join (self.GetOutputBuildContextDir(buildContext), target)
            libList = self.SplitBindings(binding.m_val)
            self.BindFileList (libList, buildContext, target, binding, targetDir, linkStaticToDynamic)

            # For windows static builds we want to link the PDB as well
            if not self.IsStatic() or not self.GetPlatform().UseEXE():
                continue

            # Get PDB names.  For now relies on them having the same name as the LIB; could make it an attribute.
            staticLibs = []
            for lib in libList:
                if '.lib' in lib:
                    newLib = lib.replace('.lib', '.pdb')
                    staticLibs.append (newLib)
                    utils.showInfoMsg ("Adding PDB for static lib: {0}\n".format(newLib), utils.INFO_LEVEL_SomewhatInteresting)

            # Ignore missing PDBs, some third party Dlls are consumed by static libs and they do not have PDB.
            self.BindFileList (staticLibs, buildContext, target, binding, targetDir, linkStaticToDynamic, True)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def BindAssemblyFiles (self, buildContext, _languages, linkStaticToDynamic):
        """Bind to all outer parts, all files mentioned in Bindings/Assemblies section into Subparts/Assemblies."""
        for binding in self.m_bindings.m_assemblyFiles:
            if self.IsStatic() and not linkStaticToDynamic:
                target = os.path.join("SubParts", "Libs")
                targetDir = os.path.join(self.GetOutputBuildContextDir(buildContext), target, binding.m_target)
                self.BindFileList (self.SplitBindings(binding.m_val), buildContext, target, binding, targetDir, linkStaticToDynamic)

            else:
                target = os.path.join("SubParts", "Assemblies")
                treatAsStatic = False if linkStaticToDynamic else self.IsStatic()
                targetDir = os.path.join(self.GetOutputBuildContextDir(buildContext, isStatic=treatAsStatic), target, binding.m_target)
                self.BindFileList (self.SplitBindings(binding.m_val), buildContext, target, binding, targetDir, linkStaticToDynamic)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def BindMergeModuleFiles (self, buildContext, _languages, linkStaticToDynamic):
        """Bind to all outer parts, all files mentioned in Bindings/MergeModules section into Subparts/Install."""
        for binding in self.m_bindings.m_mergeModules:
            target = os.path.join("SubParts", "Install")
            targetDir = os.path.join (self.GetOutputBuildContextDir(buildContext), target)
            self.BindFileList (self.SplitBindings(binding.m_val), buildContext, target, binding, targetDir, linkStaticToDynamic)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def BindDirectories (self, buildContext):
        """Bind to all outer parts, a directory mentioned in Bindings section for a subpart."""
        for binding in self.m_bindings.m_directories:
            if binding.m_addToTranskitTools:
                translationkit.PutFilesToTranskitTools (binding, self)
                continue

            sourceDir = os.path.join (self.GetMyBuildContextDir (), binding.m_val)
            targetDir = os.path.join (self.GetOutputBuildContextDir(buildContext), "SubParts", binding.m_target)

            try:
                if "" != binding.m_target or self.m_bindAlways:
                    if binding.m_symlinkContents:
                        utils.mirrorDirWithSymlinks(sourceDir, targetDir)
                    else:
                        self.PerformBindToDir (targetDir, sourceDir, skipIntermediateLinks=True)

            except symlinks.SymLinkError as err:
                raise utils.PartBuildError ("Binding error: " + err.errmessage, self, trace=err.stackTrace)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    @staticmethod
    def CheckForDupNotice (sourceFile, targetFile, thisPart):
        utils.showInfoMsg ("Checking for duplicated Vendor Notice: {0} vs {1}\n".format (sourceFile, targetFile), utils.INFO_LEVEL_RarelyUseful)
        if thisPart.m_bindAlways or not os.path.exists (targetFile):
            utils.showInfoMsg ("BindAlways: {0}   Target Path Exists: {1}\n".format (thisPart.m_bindAlways, repr(os.path.exists (targetFile))), utils.INFO_LEVEL_RarelyUseful)
            return False
        oldSourceFile = symlinks.getSymlinkTarget (targetFile)
        newSourceFile = symlinks.getSymlinkTarget (sourceFile)
        if oldSourceFile.lower() != newSourceFile.lower():
            # Compare contents
            if not filecmp.cmp (oldSourceFile, newSourceFile):
                msg = 'Error: Notice files being linked to {0} are not the same. \n  {1}\n  {2}'.format (targetFile, oldSourceFile, newSourceFile)
                raise utils.PartBuildError (msg, thisPart)
            else:
                # Barring any comparison problems, skip this file
                utils.showInfoMsg ("Binding for Notice skipped as redundant: {0} exists.\n".format (targetFile), utils.INFO_LEVEL_RarelyUseful)
                return True
        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def BindNoticeFiles (self, buildContext, languages, linkStaticToDynamic):
        # The problem with notice files is that they are always delivered with the same name and
        # can come from both static and dynamic builds. When coming from LKGs that means 2 different
        # sources but the same file.
        treatAsStatic = False if linkStaticToDynamic else self.IsStatic()
        for binding in self.m_bindings.m_noticeFiles:
            target = binding.m_target
            if not target:
                target = os.path.join('SubParts', 'VendorNotices')
            targetDir = os.path.join(self.GetOutputBuildContextDir(buildContext, isStatic=treatAsStatic), target)

            self.BindFileList (self.SplitBindings (binding.m_val, languages), buildContext, target, binding, targetDir, languages, linkStaticToDynamic, filterFunc=self.CheckForDupNotice)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def BindFromChildPart (self, child, languages, inSaveProduct, linkStaticToDynamic=False):   # pylint: disable=unused-argument
        child.BindToBuildContext (self.m_info.m_buildContext, linkStaticToDynamic)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def BindToBuildContext (self, buildContext, linkStaticToDynamic=False):
        self.UpdateBindingsForLanguages ()  # Installer parts need to update based on what languages were built
        if translationkit.IsTKBuildOnly ():
            return

        if None == self.m_bindings:
            return

        self.BindPublicAPI (buildContext, linkStaticToDynamic)
        self.BindVendorAPI (buildContext, linkStaticToDynamic)
        self.BindDirectories (buildContext)
        self.BindLibFiles (buildContext, None, linkStaticToDynamic)
        self.BindAssemblyFiles (buildContext, None, linkStaticToDynamic)
        self.BindMergeModuleFiles (buildContext, None, linkStaticToDynamic)
        self.BindNoticeFiles (buildContext, None, linkStaticToDynamic)
        self.BindMiscFiles (buildContext, None, linkStaticToDynamic)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def BindTranskitTools (self):
        if not self.m_bindings:
            return

        for binding in self.m_bindings.m_directories:
            if binding.m_addToTranskitTools:
                translationkit.PutFilesToTranskitTools (binding, self)

        for binding in self.m_bindings.m_miscFiles:
            if binding.m_addToTranskitTools:
                translationkit.PutFilesToTranskitTools (binding, self)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __DoWithLogFile (self, loggingAction):
        logFileName = self.GetLogFileName()

        # Choose whether to write or append depending on whether we've already used the file.
        if not self.m_didOpenLogFile:
            shouldAppend = False
            self.m_didOpenLogFile = True
        else:
            shouldAppend = True

        with utils.FileLogBuffer(logFileName, append=shouldAppend) as logBuffer:
            loggingAction (logBuffer)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetLKGNugetOutputDir(self, isStatic=None):
        return os.path.join(buildpaths.getOutputRoot(self.GetPlatform(), isStatic if isStatic else self.IsStatic()), "Nugets", self.m_info.m_buildContext)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:Part
    #-------------------------------------------------------------------------------------------
    def GetVersionString(self):
        (relV, majV, minV, subminV) = utils.GetVersionForProvenance()
        return "{0}.{1}.{2}.{3}".format(relV, majV, minV, subminV)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class PackagePart (Part):
    def IsPackage(self):        return True
    def PartType(self):         return "Package"

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:PackagePart
    #-------------------------------------------------------------------------------------------
    def __init__(self, partInfo, spec):
        Part.__init__(self, partInfo, spec)
        PackagePart.Init(self, partInfo, spec)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    @staticmethod
    def Init(packagePart, partInfo, spec):
        pass

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:PackagePart
    #-------------------------------------------------------------------------------------------
    def LinkPackage (self):
        if self.m_info.m_pkgType == PACKAGE_TYPE_Upack:
            self.m_info.m_repo.m_upackSource.GetUpkg().BuildAction (self)
        else:
            raise utils.PartBuildError ("Unknown package type", self, None)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:PackagePart
    #-------------------------------------------------------------------------------------------
    def UpdatePkgSourceForCache (self):
        # If the cache is read from disk then we will have 2 pkg source objects; unify them.
        if self.m_info.m_pkgType == PACKAGE_TYPE_Upack:
            upackSource = globalvars.buildStrategy.GetUpackSource (self.m_info.m_repo.m_upackSource.m_alias, self.m_info.m_platform)
            upackSource.CreateRepoEntries (globalvars.buildStrategy)
            localRepo = globalvars.buildStrategy.FindLocalRepository (self.m_info.m_repo.m_name)
            self.m_info.m_repo = localRepo
        else:
            raise utils.PartBuildError ("Unknown package type", self, None)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class ProductDirectory:
    def __init__(self, default):
        self.m_allPlatformDefault = default
        self.m_platformSpecific = dict()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:ProductDirectory
    #-------------------------------------------------------------------------------------------
    def AddSpecificForPlatform(self, platform, path, deliver, relativeTo, saveProduct):
        self.m_platformSpecific[str(platform)] = (deliver, relativeTo, path, saveProduct)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:ProductDirectory
    #-------------------------------------------------------------------------------------------
    def AddDefaults(self, defaultPath, defaultDeliver, defaultRelativeTo, saveProduct):
        self.m_allPlatformDefault = (defaultDeliver, defaultRelativeTo, defaultPath, saveProduct)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:ProductDirectory
    #-------------------------------------------------------------------------------------------
    def GetPath(self, platform):
        return self.m_allPlatformDefault[2] if str(platform) not in self.m_platformSpecific else self.m_platformSpecific[str(platform)][2]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:ProductDirectory
    #-------------------------------------------------------------------------------------------
    def GetDeliver(self, platform):
        return self.m_allPlatformDefault[0] if str(platform) not in self.m_platformSpecific else self.m_platformSpecific[str(platform)][0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:ProductDirectory
    #-------------------------------------------------------------------------------------------
    def GetRelativeTo(self, platform):
        return self.m_allPlatformDefault[1] if str(platform) not in self.m_platformSpecific else self.m_platformSpecific[str(platform)][1]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:ProductDirectory
    #-------------------------------------------------------------------------------------------
    def GetSaveProduct(self, platform):
        return self.m_allPlatformDefault[3] if str(platform) not in self.m_platformSpecific else self.m_platformSpecific[str(platform)][3]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:ProductDirectory
    #-------------------------------------------------------------------------------------------
    def GetFormedTuple(self, platform=None):
        #deliver relative path saveproduct
        if platform == None:
            return (self.m_allPlatformDefault[0], self.m_allPlatformDefault[1], self.m_allPlatformDefault[2], self.m_allPlatformDefault[3]) if self.m_allPlatformDefault else None
        else:
            return (self.GetDeliver(platform), self.GetRelativeTo(platform), self.GetPath(platform), self.GetSaveProduct(platform))

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class ProductDirectories (object):
    def __init__(self):
        self.m_deliveryDirs = None
        self.m_listReferences = []
        self.m_resolved = False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def StoreDirectoryListReference (self, product, dom, isStatic=False):
        # Because Directory Lists can refer to partfiles that have not yet been pulled we
        # have to store the items up front but delay loading them until the first time it
        # is needed.
        listName = dom.getAttribute ("DirectoryListName").lower()
        if "" == listName:
            raise utils.PartBuildError ("No DirectoryListName specified for Directories", product, dom)

        partFilePath = None
        partFileName = dom.getAttribute ("PartFile")
        if "" == partFileName:
            # Part file name not specified; use parent
            partFileName = product.m_info.m_buildContext
            partFilePath = product.m_info.m_file

        repoName = dom.getAttribute ("Repository")
        if "" == repoName:
            # Part file name not specified; use parent
            repoName = product.m_info.m_repo.m_name
            repo = product.m_info.m_repo
        else:
            repo = globalvars.buildStrategy.FindLocalRepository (repoName)

        excludePlatforms = dom.getAttribute ("ExcludePlatforms")
        if excludePlatforms:
            def handleNoPlatformMatchDirectoriesExcludePlatforms (csv, nonvalidMatch):
                raise utils.PartBuildError ("Directories '{0}' ExcludePlatforms='{1}' has platform or wildcard '{2}' that does not match any platform, must be one of {3}.".format(listName, csv, nonvalidMatch, targetplatform.GetXmlOptions().__str__()), product, dom)

            if (product.GetPlatform().IsExcluded (targetplatform.GetPlatformsFromCSV(excludePlatforms, handleNoPlatformMatchDirectoriesExcludePlatforms))):
                return

        onlyPlatforms = dom.getAttribute ("OnlyPlatforms")
        if onlyPlatforms:
            def handleNoPlatformMatchDirectoriesOnlyPlatforms (csv, nonvalidMatch):
                raise utils.PartBuildError ("Directories '{0}' OnlyPlatforms='{1}' has platform or wildcard '{2}' that does not match any platform, must be one of {3}.".format(listName, csv, nonvalidMatch, targetplatform.GetXmlOptions().__str__()), product, dom)

            if (not product.GetPlatform().IsExcluded (targetplatform.GetPlatformsFromCSV(onlyPlatforms, handleNoPlatformMatchDirectoriesOnlyPlatforms))):
                return

        # We need to link any partfiles listed in ProductDirectoryList into their BuildContext
        # so we need to store off the partfile location. Use a PartInfo to get it right.
        # Of course if it's the current file then use the current values.
        if not partFilePath:
            info = PartInfo ('ProductDirectoryList', repo, partFileName, PART_TYPE_Part, product.GetPlatform(), isStatic, None)
            partFilePath = info.m_file

        self.m_listReferences.append ((listName, partFileName, repoName, partFilePath))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def FindDirectoryList (self, product, listName, partFileName):
        # We have to get the partfile from the build context because it could be from LKGs. For
        # example, PowerProducts use PowerPlatform LKGS and Directory Lists.
        # Since LKGness is only known on the part level we will rely on the fact that all the
        # partfiles are linked in during the BeforeAction step of build.
        partFile = os.path.join (product.GetOutputBuildContextDir (partFileName), partFileName+'.PartFile.xml')

        # Only access the cache this if the product has a reference to the bgraph
        if product.m_action and product.m_action.m_bgraph:
            key = "{0}|{1}".format(partFile.lower(), listName)
            if key in product.m_action.m_bgraph.m_dirListCache:
                return product.m_action.m_bgraph.m_dirListCache[key]

        listDom = getPartFileDom (partFile, product)
        deliveryLists= utils.getDomElementsByName(listDom, "ProductDirectoryList")

        for dlist in deliveryLists:
            name = dlist.getAttribute ("ListName").lower()
            if name == listName:
                # Cache this so we can lookup later without going back to disk
                # Cache it in the bgraph so that it persists between BB commands
                # But only access the cache this if the product has a reference to the bgraph
                if product.m_action and product.m_action.m_bgraph:
                    product.m_action.m_bgraph.m_dirListCache[key] = dlist
                return dlist

        raise utils.BuildError ("Can't find ProductDirectoryList '{0}' from '{1}' as used by Product: {2}".format(listName, partFile, product.m_info.m_name))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ResolveLists (self, product):
        if self.m_resolved:
            return

        for (listName, partFileName, _, _) in self.m_listReferences:
            listDom = self.FindDirectoryList (product, listName, partFileName)

            if None == self.m_deliveryDirs:
                self.m_deliveryDirs = dict()

            deliveryDirs = utils.getDomElementsByName(listDom, "ProductDirectory")
            for deliveryDir in deliveryDirs:
                name = deliveryDir.getAttribute ("Name").lower()
                if "" == name:
                    raise utils.BuildError ("No Name given for ProductDirectory as used by Product: {0}".format(product.GetShortPartDescriptor()))
                doDeliver = utils.getOptionalBoolAttr ("Deliver", True, "'Delivery' attribute of ProductDirectory {0}".format (name), deliveryDir)
                saveProduct = utils.getOptionalBoolAttr ("SaveProduct", True, "'SaveProduct' attribute of ProductDirectory {0}".format (name), deliveryDir)

                platformsAttrStr = deliveryDir.getAttribute ("Platforms")
                platforms = []
                if "" != platformsAttrStr:
                    targetplatform.UpdatePlatformListByName(platforms, platformsAttrStr)

                libType = deliveryDir.getAttribute ("LibType")
                if 'static' == libType.lower():
                    name = self.AdjustNameForStatic (name, True)

                path = deliveryDir.getAttribute ("Path")

                if name in self.m_deliveryDirs:
                    if len(platforms) == 0:
                        self.m_deliveryDirs[name].AddDefaults(path, doDeliver, deliveryDir.getAttribute ("RelativeTo"), saveProduct)
                    else:
                        for platform in platforms:
                            self.m_deliveryDirs[name].AddSpecificForPlatform(platform, path, doDeliver, deliveryDir.getAttribute ("RelativeTo"), saveProduct)
                else:
                    if len(platforms) == 0:
                        self.m_deliveryDirs[name] = ProductDirectory((doDeliver, deliveryDir.getAttribute ("RelativeTo"), path, saveProduct))
                    else:
                        self.m_deliveryDirs[name] = ProductDirectory(None)
                        for platform in platforms:
                            self.m_deliveryDirs[name].AddSpecificForPlatform(platform, path, doDeliver, deliveryDir.getAttribute ("RelativeTo"), saveProduct)

        self.m_resolved = True

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AdjustNameForStatic (self, name, isStatic):     #pylint: disable=function-redefined
                                                        # Method gets overwritten with "buildpart.AdjustNameForStatic=...". Could use a better tactic.
        addName = '*static' if isStatic else ''
        return name + addName

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def FindProductDirectory (self, lookForName):
        if None == self.m_deliveryDirs or not lookForName.lower() in self.m_deliveryDirs:
            return None

        return self.m_deliveryDirs[lookForName.lower()]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ResolveProductDirectory (self, product, binding, child):
        return self.ResolveProductDirectoryName (product, binding.m_productDirectory, binding, child, binding.m_productDirectoryNameForErrors)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ResolveProductDirectoryName (self, product, lookForNameIn, binding, child, lookForNameForErrors):
        """Resolve a ProductDirectory variable to a full path. Return None if Delivery=False"""
        self.ResolveLists(product) # Make sure data is loaded

        if None == self.m_deliveryDirs:
            return None

        lookForName = self.AdjustNameForStatic (lookForNameIn, child.IsStatic())

        checkedNames = []
        resolvedName = ""
        while True:
            if lookForName in checkedNames:
                raise utils.PartBuildError ("Illegal circular ProductDirectory '{0}' from '{1}'".format (lookForNameForErrors, checkedNames), child)

            checkedNames.append(lookForName)

            productDirectory = self.FindProductDirectory (lookForName)
            if None == productDirectory:
                # Can't find ProductDirectory. If the binding isn't required or this product is an AddIn, just skip the binding.
                if (None != binding and not binding.m_required) or product.IsAddIn():
                    return None
                else:
                    staticMsg = ' with LibType="static" ' if product.IsStatic() else ''
                    utils.ShowAndDeferMessage ("Warning: Cannot find ProductDirectory '{0}' {1} for Product '{2}'. Product specification is incomplete.\n  Used by part {3}\n".format (
                                  lookForNameForErrors, staticMsg, product.GetShortRepresentation(), child.GetUniqueRepresentation()), utils.INFO_LEVEL_Important)
                    return None

            if not productDirectory.GetDeliver(child.GetPlatform()): # Deliver="False"
                return None

            resolvedName = os.path.join (os.path.expandvars(productDirectory.GetPath(child.GetPlatform())), resolvedName)

            if "" == productDirectory.GetRelativeTo(child.GetPlatform()):
                subDir = ""
                if None != binding and "" != binding.m_subDir:
                    subDir = binding.m_subDir
                return os.path.join (product.GetDeliveryRoot(), resolvedName, subDir)

            lookForName = productDirectory.GetRelativeTo(child.GetPlatform()).lower()

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class Product (Part):
    def IsProduct(self):        return True
    def IsNuGetProduct(self):   return False
    def IsUPackProduct(self):   return False
    def IsAddIn(self):          return self.m_isAddIn      # AddIn means that it can ignore any or all Product Directories
    def PartType(self):         return "Product"
    def GetDeliveryRoot(self):  return os.path.join (self.GetProductRoot(), self.m_info.m_name)
    def IsProductSavable (self): return self.m_isProductSavable # used for bb saveproduct

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:Product
    #-------------------------------------------------------------------------------------------
    def __init__(self, partInfo, productDom):
        Part.__init__(self, partInfo, productDom)
        bindings = utils.getDomElementsByName(productDom, "Bindings")
        if len(bindings) > 0:
            raise utils.PartBuildError ("Products can not have Bindings", self, productDom)

        featureAspects = utils.getDomElementsByName(productDom, "FeatureAspects")
        if len(featureAspects) > 0:
            self.m_featureAspects = (featureAspects[0].getAttribute("Template"), featureAspects[0].getAttribute("Instance"))
        else:
            self.m_featureAspects = None

        if translationkit.IsTranskitShell () and self.m_featureAspects:
            partFilePath = os.path.join (os.path.dirname (self.m_info.m_file), "FeatureAspects")
            self.m_featureAspects = ( os.path.join (partFilePath, os.path.basename (self.m_featureAspects [0])), os.path.join (partFilePath, os.path.basename (self.m_featureAspects [1])))


        addInAttr = productDom.getAttribute ("AddIn")
        isAddIn = (addInAttr != None) and ("true" == addInAttr.lower())

        self.m_nuspec = None
        self.m_isAddIn = isAddIn
        self.m_transkitEntries = dict()
        self.m_loadConfig = dict()
        self.m_directories = None
        self.m_omitFeatureAspects = set()
        self.m_featureAspectsEvaluated = False
        self.m_includeFeatures = set()
        self.m_featureAspectOmitLock = None  # Set this up later because of part cache

        directories = utils.getDomElementsByName(productDom, "Directories")
        if len (directories) > 0 :
            self.m_directories = ProductDirectories()
            for directoryList in directories:
                self.m_directories.StoreDirectoryListReference(self, directoryList, isStatic=self.IsStatic())

        # products can provide a list of features to exclude
        featureExclude = utils.getDomElementsByName(productDom, "FeatureExclude")
        for _, feature in enumerate(featureExclude):
            fc = feature.firstChild
            if fc.nodeType == fc.TEXT_NODE:
                for feat in utils.splitSources(fc.data):
                    self.m_omitFeatureAspects.add(feat)  # Doesn't need lock because it will finish before children are processed.

        # products can provide a list of features to include
        featureInclude = utils.getDomElementsByName(productDom, "FeatureInclude")
        for _, feature in enumerate(featureInclude):
            fc = feature.firstChild
            if fc.nodeType == fc.TEXT_NODE:
                for feat in utils.splitSources(fc.data):
                    self.m_includeFeatures.add(feat)

        self.m_isProductSavable = False
        saveProd = productDom.getAttribute("SaveProduct")
        if saveProd.lower() == "true":
            self.m_isProductSavable = True

        # Check for a product having a static library type
        libType = productDom.getAttribute("LibType")
        if libType:
            raise utils.PartBuildError ("Products LibType removed", self, productDom)

        self.SetScopeFlagFromDom (productDom)

        # Also check if it excludes static or dynamic library builds
        self.GetLibraryExclusionsFromDom (productDom)
        self.SetPlatformExclusionsFromDom (productDom)
        self.GetWipInfoFromDom (productDom)
        self.m_languagePackFileHistory = {} # KN_MERGE: Should use history class?

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:Product
    #-------------------------------------------------------------------------------------------
    def GetFeatureAspectLock (self):
        # The feature aspect list gets things added to it at some point; another part may be processing causing an error "added to list while iterating"
        if not self.m_featureAspectOmitLock:
            self.m_featureAspectOmitLock = threading.Lock()
        return self.m_featureAspectOmitLock

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:Product
    #-------------------------------------------------------------------------------------------
    def EvaluateFeatureAspects (self):
        if None != self.m_featureAspects and (len(self.m_omitFeatureAspects) == 0 or not self.m_featureAspectsEvaluated):
            self.m_featureAspectsEvaluated = True
            template = self.m_featureAspects[0].replace ('${OutputRootDir}', buildpaths.getOutputRoot(self.GetPlatform(), self.IsStatic()))
            instance = self.m_featureAspects[1].replace ('${OutputRootDir}', buildpaths.getOutputRoot(self.GetPlatform(), self.IsStatic()))

            with self.GetFeatureAspectLock():
                featureaspects.interpretFeatureAspects (self.m_omitFeatureAspects, template, instance)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:Product
    #-------------------------------------------------------------------------------------------
    def AddDir (self, binding, root, child, deliveryDir):
        if None == root: #if no Root specified, just use deliveryDir for target and binding for source
            sourceDir = os.path.join(child.GetMyBuildContextDir(), binding.m_val)
        else:
            sourceDir = os.path.join (child.GetMyBuildContextDir(), root, binding.m_val)
            deliveryDir = os.path.join (deliveryDir, binding.m_val)

        if not child.IsClobberBuild ():
            if not symlinks.isTargetSame( sourceDir, deliveryDir ):
                self.EnsureNoSymLinks (deliveryDir)

        if not binding.m_useSymLink:
            self.PerformDirCopy (deliveryDir, sourceDir)
        else:
            utils.showInfoMsg ("Binding Directory {0} to {1}\n".format (sourceDir, deliveryDir), utils.INFO_LEVEL_RarelyUseful)
            try:
                if binding.m_symlinkContents:
                    utils.mirrorDirWithSymlinks(sourceDir, deliveryDir)
                else:
                    self.PerformBindToDir (deliveryDir, sourceDir, checkSame=True, checkTargetExists=True, skipIntermediateLinks=True)
            except symlinks.SymLinkError as err:
                raise utils.PartBuildError (err.errmessage, child, trace=err.stackTrace)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:Product
    #-------------------------------------------------------------------------------------------
    def AddDirToProduct (self, binding, root, child, inSaveProduct):
        if binding.ShouldSkipBinding (self):
            return
        if None == self.m_directories:
            raise utils.PartBuildError ("{1}Product '{0}' has no Directories listed".format (self.m_info.m_name, 'NuGet' if self.IsNuGetProduct() else ''), child)

        # Bind only once per product/version
        if (self.IsNuGetProduct() and binding.m_boundToNugetPkg and binding.m_boundToNugetPkg[0] == self.m_nuspec.m_name and binding.m_boundToNugetPkg[1] == self.m_nuspec.m_version):
            utils.showInfoMsg ("Binding already in nuspec {0} nuspec.\n".format(binding.m_boundToNugetPkg[0]), utils.INFO_LEVEL_Interesting)
            if child.IsNuGetProduct():
                self.m_nuspec.AddDependency(binding.m_boundToNugetPkg[0], binding.m_boundToNugetPkg[1], binding.m_boundToNugetPkg[2])
            return

        deliveryDir = self.m_directories.ResolveProductDirectory (self, binding, child)

        if inSaveProduct and not self.ShouldSaveProduct (binding.m_productDirectory, child.GetPlatform()):
            return
        if None != deliveryDir:
            self.AddDir (binding, root, child, deliveryDir)

            if self.IsNuGetProduct():
                pathRelativeToProduct = deliveryDir.replace(self.GetDeliveryRoot(), '')
                self.m_nuspec.AddFiles(os.path.join(deliveryDir, '**', '*'), pathRelativeToProduct)

                # Attempt to get the targetFramework from path
                targetFramework = nugetpkg.GetTargetFrameworkFromPath(pathRelativeToProduct)
                binding.m_boundToNugetPkg = (self.m_nuspec.m_name, self.m_nuspec.m_version, targetFramework)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:Product
    #-------------------------------------------------------------------------------------------
    def ShouldSaveProduct(self, name, platform):
        name = self.m_directories.AdjustNameForStatic (name, self.IsStatic())
        value = False if not name in self.m_directories.m_deliveryDirs else self.m_directories.FindProductDirectory(name).GetSaveProduct(platform)
        return value

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:Product
    #-------------------------------------------------------------------------------------------
    def AddDirListToProduct (self, binding, root, child, inSaveProduct):
        if translationkit.IsTranskitShell ():
            # In Transkit shell, build and bind only transkit items.
            return
        for bdir in binding:
            self.AddDirToProduct(bdir, root, child, inSaveProduct)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:Product
    #-------------------------------------------------------------------------------------------
    def EnsureNoSymLinks (self, path):
        (head, rightmostDir) = os.path.split (path)
        # If the path ends with a slash, rightmostDir will be empty, so get it again to get the directory name
        if rightmostDir == '':
            (head, rightmostDir) = os.path.split (head)

        pathComponents = []
        while rightmostDir != "":
            pathComponents.append (rightmostDir)
            (head, rightmostDir) = os.path.split (head)
        pathComponents.append (head)
        pathComponents.reverse ()

        pathToTest = ""
        for directory in pathComponents:
            pathToTest = os.path.join (pathToTest, directory)
            if symlinks.isSymbolicLink (pathToTest):
                raise utils.PartBuildError (
                    "Binding error: Cannot add items within symbolically linked directories ({0})".format (pathToTest),
                    self
                )

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:Product
    #-------------------------------------------------------------------------------------------
    def AddFileListToProduct (self, child, sourceList, target, binding, filterFunc):
        targetDir = target
        self.EnsureNoSymLinks (targetDir)

        sourceDir = child.GetMyBuildContextDir(isDocumentationBuildContext=(binding.m_docItem!=None))
        for source in sourceList:
            sourceName = os.path.join(sourceDir, source)
            sourceFiles = glob.glob (sourceName)

            if len(sourceFiles) == 0:
                self.ShowFileBindingError (sourceName, child, binding)
                return
            for sourceFile in sourceFiles:
                _, sourceTail= os.path.split (sourceFile)
                if filterFunc and filterFunc (sourceFile, os.path.join (targetDir, sourceTail), self):
                    continue

                try:
                    targetFile = os.path.join (targetDir, sourceTail)
                    if not binding.m_useSymLink:
                        self.PerformFileCopy (targetFile, sourceFile)
                    else:
                        utils.showInfoMsg ("Binding File {0} to {1}\n".format (sourceFile, targetFile), utils.INFO_LEVEL_RarelyUseful)
                        self.PerformBindToFile (targetFile, sourceFile, checkSame=True, checkTargetExists=True, skipIntermediateLinks=True)

                except symlinks.SymLinkError as err:
                    raise utils.PartBuildError (err.errmessage, child, trace=err.stackTrace)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:Product
    #-------------------------------------------------------------------------------------------
    def AddDocFilesToProduct (self, bindings, child, _inSaveProduct):

        for binding in bindings:
            if binding.ShouldSkipBinding (self):
                continue

            deliveryDir = self.m_directories.ResolveProductDirectory (self, binding, child)
            if None != deliveryDir:
                if not binding.m_docItem:
                    continue
                self.AddFileListToProduct (child, self.SplitBindings(binding.m_val), deliveryDir, binding, None)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:Product
    #-------------------------------------------------------------------------------------------
    def AddFilesToProduct(self, bindings, child, inSaveProduct, filterFunc=None):

        for binding in bindings:
            if binding.ShouldSkipBinding (self):
                continue

            # Bind only once per product/version
            if (self.IsNuGetProduct() and binding.m_boundToNugetPkg and binding.m_boundToNugetPkg[0] == self.m_nuspec.m_name and binding.m_boundToNugetPkg[1] == self.m_nuspec.m_version):
                utils.showInfoMsg ("Binding already in nuspec {0} nuspec.\n".format(binding.m_boundToNugetPkg[0]), utils.INFO_LEVEL_Interesting)
                if child.IsNuGetProduct():
                    self.m_nuspec.AddDependency(binding.m_boundToNugetPkg[0], binding.m_boundToNugetPkg[1], binding.m_boundToNugetPkg[2])
                continue

            deliveryDir = self.m_directories.ResolveProductDirectory (self, binding, child)
            if None != deliveryDir:
                if not translationkit.IsTKBuildOnly ():
                    self.AddFileListToProduct (child, self.SplitBindings(binding.m_val), deliveryDir, binding, filterFunc)
                binding.AddTranskitsToProduct (child, self, deliveryDir, inSaveProduct)
                binding.AddLoadConfigToProduct (self)

                if self.IsNuGetProduct():
                    pathRelativeToProduct = deliveryDir.replace(self.GetDeliveryRoot(), '')
                    self.m_nuspec.AddFiles(os.path.join(deliveryDir, '*'), pathRelativeToProduct)

                    # Attempt to get the targetFramework from path
                    targetFramework = nugetpkg.GetTargetFrameworkFromPath(pathRelativeToProduct)
                    binding.m_boundToNugetPkg = (self.m_nuspec.m_name, self.m_nuspec.m_version, targetFramework)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:Product
    #-------------------------------------------------------------------------------------------
    def BindSubProducts (self, child, inSaveProduct):
        if inSaveProduct and not self.IsProductSavable():
            return
        for productBinding in child.m_subProductBindings:
            if None == self.m_directories:
                raise utils.PartBuildError ("{1}Product '{0}' has no Directories listed".format (self.m_info.m_name, 'NuGet' if self.IsNuGetProduct() else ''), child)

            if inSaveProduct and not self.ShouldSaveProduct (productBinding.m_productDirectory, child.GetPlatform()):
                return
            deliveryDir = self.m_directories.ResolveProductDirectoryName (self, productBinding.m_productDirectory, None, productBinding.m_subProduct, productBinding.m_productDirectory)
            if None != deliveryDir:
                sourceDir = os.path.join (productBinding.m_subProduct.GetProductRoot (), productBinding.m_subProduct.m_info.m_name)
                if not os.path.exists(sourceDir):
                    utils.showInfoMsg ("Skipping binding Product {0} to Product {1} because it is empty\n".format \
                        (productBinding.m_subProduct.m_info.m_name, self.m_info.m_name), utils.INFO_LEVEL_SomewhatInteresting)
                    continue

                if not child.IsClobberBuild ():
                    if not symlinks.isTargetSame(sourceDir, deliveryDir):
                        self.EnsureNoSymLinks (deliveryDir)

                utils.showInfoMsg ("Binding Directory {0} to {1}\n".format (sourceDir, deliveryDir), utils.INFO_LEVEL_RarelyUseful)
                try:
                    # When building, this allows BindToDirectory to nest several consumed SubProducts inside one another in the current Product directory tree
                    # For SaveProduct, we simply use PerformBindToDir as that gets overriden with a copy to the appropriate LKG location
                    if not inSaveProduct:
                        utils.mirrorDirWithSymlinks(sourceDir, deliveryDir)
                    else:
                        self.PerformBindToDir (deliveryDir, sourceDir, checkSame=True, checkTargetExists=True, skipIntermediateLinks=True)
                except symlinks.SymLinkError as err:
                    raise utils.PartBuildError (err.errmessage, child, trace=err.stackTrace)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:Product
    #-------------------------------------------------------------------------------------------
    def BindFromChildPart (self, child, languages, inSaveProduct, linkStaticToDynamic=False):      # pylint: disable=unused-argument
        if not child.IsProduct ():
            self.BindSubProducts(child, inSaveProduct)

        if child.m_bindings and not self.m_directories:
            if self.IsAddIn():  # AddIn means that it can ignore any or all Product Directories
                return
            child.ExitOnError( 1, 'Product {0} is missing a Directories element.'.format (self) )

        if None != child.m_bindings:
            self.AddDirListToProduct (child.m_bindings.m_publicAPI, "PublicAPI", child, inSaveProduct)
            self.AddDirListToProduct (child.m_bindings.m_vendorAPI, "VendorAPI", child, inSaveProduct)
            self.AddDirListToProduct (child.m_bindings.m_directories, None, child, inSaveProduct)
            self.AddFilesToProduct (child.m_bindings.m_libFiles, child, inSaveProduct)
            self.AddFilesToProduct (child.m_bindings.m_assemblyFiles, child, inSaveProduct)
            self.AddFilesToProduct (child.m_bindings.m_mergeModules, child, inSaveProduct)
            self.AddFilesToProduct (child.m_bindings.m_noticeFiles, child, inSaveProduct, self.CheckForDupNotice)
            self.AddFilesToProduct (child.m_bindings.m_miscFiles, child, inSaveProduct)
            if not self.IsNuGetProduct():
                self.AddDocFilesToProduct (child.m_bindings.m_documentations, child, inSaveProduct)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:Product
    #-------------------------------------------------------------------------------------------
    def GetInstallerFilePath (self):
        if self.m_installerFileSource == None:
            return (None, None)

        return (True, os.path.join (self.m_info.m_partFileDir, self.m_installerFileSource.GetSourceFile ()))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetConfigFileName (self):
        return os.path.join (self.GetLogFileRoot(), self.m_info.m_buildContext, CFG_FILE_NAME)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def BindLoadConfigFileToProduct (self):
        if not self.m_loadConfig: return

        # Split out this way so SaveProduct can get to it as well
        apploadName = self.GetConfigFileName()
        configOutDir = self.m_directories.ResolveProductDirectoryName (self, "ConfigDir", None, self, "ConfigDir")
        if None != configOutDir:
            outFile = os.path.join (configOutDir,  "system", CFG_FILE_NAME)
            self.PerformBindToFileWithRetries (outFile, apploadName, retries=30) # Not sure why 10 wasn't enough. What holds these files open?

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:Product
    #-------------------------------------------------------------------------------------------
    def WriteLoadConfigFile (self):
        if not self.m_loadConfig:
            return

        apploadName = self.GetConfigFileName()
        apploadFile = open (apploadName, "wt")
        for cfgName,apps in self.m_loadConfig.items():
            for app in apps:
                apploadFile.write ("{0} > {1}\n".format (cfgName,app))
        apploadFile.close()

        self.BindLoadConfigFileToProduct ()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:Product
    #-------------------------------------------------------------------------------------------
    def RunBuildCmd (self, additionalBmakeOptions=None):
        self.WriteProvenanceFile ()
        self.WriteLoadConfigFile()

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class NuGetProduct (Product):
    def IsProduct(self):        return True
    def IsNuGetProduct(self):   return True
    def PartType(self):         return "NuGetProduct"

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:NuGetProduct
    #-------------------------------------------------------------------------------------------
    def __init__(self, partInfo, productDom):
        self.m_licensePath      = None

        description = productDom.getAttribute ("Description")
        licenseName = productDom.getAttribute ("LicenseFile")

        if (not description) == (not licenseName):
            raise utils.BuildError("Must define exactly ONE of 'Description' and 'LicenseFile' for NuGetProduct elements: {0}. The Description should be either defined in the PartFile or present in the LicenseFile if it is defined.".format(partInfo.GetShortPartDescriptor()))

        if licenseName:
            filepath = os.path.join(partInfo.m_partFileDir, licenseName)
            self.m_licensePath = filepath
            licenseFile = compat.readJsonFile(filepath)
            name = licenseFile["name"]
            # override the name if applicable
            partInfo.m_name = name

        nativeAttr = productDom.getAttribute ("Native")
        isNative = (nativeAttr and nativeAttr.lower()=='true')

        Product.__init__(self, partInfo, productDom)
        self.m_nuspec = nugetpkg.Nuspec(self.GetDeliveryRoot(), self.GetQualifiedNugetName(), self.GetVersionString(), description, partInfo.m_platform.GetXmlName(), isNative)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:NuGetProduct
    #-------------------------------------------------------------------------------------------
    def BindFromChildPart (self, child, languages, inSaveProduct, linkStaticToDynamic=False):
        Product.BindFromChildPart(self, child, languages, inSaveProduct, linkStaticToDynamic)
        self.AddDependenciesToNuget(child.m_nugetPackages, child)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:NuGetProduct
    #-------------------------------------------------------------------------------------------
    def CreateNugetPackage(self):
        if not self.IsNuGetProduct():
            return

        (status, msg) = self.m_nuspec.WriteToXml(self.m_licensePath)

        if status != 0:
            raise utils.PartBuildError("NuGet package creation error {0}".format(msg), self)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:NuGetProduct
    #-------------------------------------------------------------------------------------------
    def AddDependenciesToNuget(self, nugetPkgs, child):
        utils.showInfoMsg ("Adding dependencies to {0} from child part {1}\n".format(self.m_info.m_name, child.m_info.m_name), utils.INFO_LEVEL_Interesting)
        for nugetUsage in nugetPkgs:
            self.m_nuspec.AddDependency(nugetUsage.m_pkgSource.m_name, nugetUsage.m_pkgSource.GetVersion(), nugetUsage.m_pkgFramework)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class UPackProduct (Product):
    def IsProduct(self):        return True
    def IsUPackProduct(self):   return True
    def PartType(self):         return "UPackProduct"
    def GetUPackName(self):     return "{0}_{1}".format(self.m_info.m_name, self.GetPlatform().GetXmlName()) if self.m_multiPlatform else self.m_info.m_name

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:UPackProduct
    #-------------------------------------------------------------------------------------------
    def __init__(self, partInfo, productDom):
        self.m_description = productDom.getAttribute ("Description")
        self.m_multiPlatform = utils.getOptionalBoolAttr ("MultiPlatform", False, "'MultiPlatform' attribute of UPackProduct", productDom)
        if not self.m_description:
            raise utils.BuildError("Must define attribute 'Description' for UPackProduct elements: {0}.".format(partInfo.GetShortPartDescriptor()))

        Product.__init__(self, partInfo, productDom)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class ToolPart(Part):

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__(self, partInfo, spec):
        Part.__init__(self, partInfo, spec)
        ToolPart.Init(self, partInfo, spec)

    def IsToolPart(self):       return True
    def PartType(self):         return "ToolPart"

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    @staticmethod
    def Init(toolPart, partInfo, spec):
        pass

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    @staticmethod
    def BindToolCacheNuGet(nugetPkg):
        link = os.path.join(buildpaths.GetToolsOutputRoot(), nugetPkg.m_name)
        target = nugetPkg.GetPackagePath()
        symlinks.createDirSymLink(link, target, checkSame=False, checkTargetExists=True)
        # TODO: Above should use self.PerformBindToDir, but have to figure out self vs static

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    @staticmethod
    def BindToolCacheUpack(upack):
        upack.GetUpkg().ResolveVersionLocal()
        name = upack.m_name
#        if upack.m_multiPlatform and upack.m_origName:  # Put in output tree without platform
        if upack.m_origName:  # Put in output tree without platform
            name = upack.m_origName
        link = os.path.join(buildpaths.GetToolsOutputRoot(), name)
        target = upack.GetLocalPath()
        symlinks.createDirSymLink(link, target, checkSame=False, checkTargetExists=True)
        # TODO: Above should use self.PerformBindToDir, but have to figure out self vs static

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    @staticmethod
    def BindToolCacheRepo(reqRepo):
        link = os.path.join(buildpaths.GetToolsOutputRoot(), reqRepo)
        target = globalvars.buildStrategy.FindLocalRepository (reqRepo).GetLocalDir()
        symlinks.createDirSymLink(link, target, checkSame=False, checkTargetExists=True)
        # TODO: Above should use self.PerformBindToDir, but have to figure out self vs static

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def BindToolCache(self):
        for nugetUsage in self.m_nugetPackages:
            self.BindToolCacheNuGet (nugetUsage.m_pkgSource)
            self.m_action.UpdateToolCacheData (nugetUsage.m_pkgSource.m_name)

        for _, upack in self.m_universalPackages:
            self.BindToolCacheUpack (upack)
            self.m_action.UpdateToolCacheData (upack.m_name)

        for reqRepo in self.m_additionalRepos or []:
            self.BindToolCacheRepo (reqRepo)
            self.m_action.UpdateToolCacheData (reqRepo)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def RunBuildCmd(self, additionalBmakeOptions=None):
        self.BindToolCache()
        Part.RunBuildCmd(self, additionalBmakeOptions)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def UpdatePkgToolPartsForCache(self):
        # These aren't set up with the strategy so store in a way that they can be easily updated in the strategy
        # and any copies will also get the benefit of this knowledge.
        for _, pkgSource in self.m_universalPackages:
            globalvars.buildStrategy.MarkToolPart (pkgSource.m_name, globalvars.REPO_UPACK)

        for nugetUsage in self.m_nugetPackages:
            globalvars.buildStrategy.MarkToolPart (nugetUsage.m_pkgSource.m_name, globalvars.REPO_NUGET)

        for reqRepo in self.m_additionalRepos or []:
            globalvars.buildStrategy.MarkToolPart (reqRepo, globalvars.REPO_GIT)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class ToolPackagePart (Part):
    def IsPacakge(self):        return True
    def IsToolPart(self):       return True
    def PartType(self):         return "ToolPackage"

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:ToolPackagePart
    #-------------------------------------------------------------------------------------------
    def __init__(self, partInfo, spec):
        Part.__init__(self, partInfo, spec)
        ToolPart.Init(self, partInfo, spec)
        PackagePart.Init(self, partInfo, spec)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:ToolPackagePart
    #-------------------------------------------------------------------------------------------
    def LinkPackage (self):
        return PackagePart.LinkPackage(self)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:ToolPackagePart
    #-------------------------------------------------------------------------------------------
    def BindToolCache(self):
        return ToolPart.BindToolCache(self)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:ToolPackagePart
    #-------------------------------------------------------------------------------------------
    def RunBuildCmd(self, additionalBmakeOptions=None):
        ToolPart.RunBuildCmd(self, additionalBmakeOptions)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:ToolPackagePart
    #-------------------------------------------------------------------------------------------
    def UpdatePkgToolPartsForCache(self):
        return ToolPart.UpdatePkgToolPartsForCache(self)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:ToolPackagePart
    #-------------------------------------------------------------------------------------------
    def UpdatePkgSourceForCache (self):
        return PackagePart.UpdatePkgSourceForCache(self)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getPartFileDom (fileName, parent, _fromLkgs=False):
    lcFileName = fileName.lower()
    if lcFileName in g_currDoms:
        return g_currDoms[lcFileName]

    baseFileName = os.path.basename (fileName)
    # TODO Reinstate this check
#    if not fromLkgs and baseFileName.lower() in g_partFileNames:
#        otherFile = [pfile for pfile in g_currDoms if baseFileName.lower() in pfile]
#        raise utils.BuildError("Duplicate Part File Name found: {0}\n    {1}\n    {2}".format(baseFileName, lcFileName, otherFile[0]))

    if not os.path.isfile (fileName):
        utils.showInfoMsg("\nPosixDebug: getPartFileDom fileName {0} NOT found in file system\n".format(fileName), utils.INFO_LEVEL_TemporaryDebugging)
        result = utils.GetExactCaseFilePath(fileName)
        if not result:
            caller = parent.m_info.GetShortPartDescriptor() if parent else 'unknown'
            raise utils.PartBuildError("PartFile not found {0} called from {1} \n".format(fileName, caller), parent)
        else:
            utils.showInfoMsg("\nPosixDebug: getPartFileDom result {0}\n".format(str(result)), utils.INFO_LEVEL_TemporaryDebugging)
            fileName = result[0]
    try:
        pfDom = utils.parseXml (fileName)
    except xml.parsers.expat.ExpatError as errIns:
        raise utils.BuildError("Parse Error\n    {0}: ".format(fileName) + errIns.__str__())

    bcDomList = utils.getDomElementsByName (pfDom, "BuildContext")
    if 0 == len(bcDomList):
        raise utils.BuildError("PartFile {0} has no BuildContext".format(fileName))
    elif len(bcDomList) > 1:
        raise utils.BuildError("PartFile {0} has more than one BuildContext".format(fileName))
    bcDom = bcDomList[0]

    shouldVerifyXsdPath = False # lower priority atm - this will break on LKGs anyway.
    if shouldVerifyXsdPath:
        # Check to make sure that the xsd can be located from the file.
        if not bcDom.hasAttribute ("xsi:noNamespaceSchemaLocation"):
            raise utils.BuildError("PartFile {0} BuildContext node has no attribute xsi:noNamespaceSchemaLocation which is required to be the relative path to the file in bsicommon\\build\\PartFile.xsd.".format(fileName))

        xsdPath = bcDom.getAttribute ("xsi:noNamespaceSchemaLocation")
        xsdUnnormalizedPath = os.path.join (os.path.split (fileName)[0], xsdPath)
        xsdAbsPath = symlinks.normalizePathName (xsdUnnormalizedPath)
        if not os.path.exists (xsdAbsPath):
            raise utils.BuildError("PartFile has BuildContext node attribute xsi:noNamespaceSchemaLocation='{0}' which cannot be found at {1}. \nIt must be relatively mapped to %srcroot%bsicommon\\build\\PartFile.xsd.".format(xsdPath, xsdUnnormalizedPath))

    g_currDoms[lcFileName] = bcDom
    g_partFileNames.add (baseFileName.lower())
    return bcDom

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def emptyPartFileDom ():
    global g_currDoms
    g_currDoms = {}

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def readPartFromDomNonCached (info, parent, partDom):
    partName = info.m_name.lower()
    partType = info.m_partType

    ret = None

    if partType in [PART_TYPE_Product, PART_TYPE_Any]:
        products = utils.getDomElementsByName(partDom, "Product")
        for product in products:
            if partName == product.getAttribute("Name").lower():
                info.m_partType = PART_TYPE_Product
                if ret != None:
                    utils.ShowAndDeferMessage("Duplicate Product: {0}\n".format(info.GetShortPartDescriptor()), utils.INFO_LEVEL_Essential, utils.RED)
                    return ret
                ret = Product (info, product)
        if ret != None:
            return ret
        if partType != PART_TYPE_Any:
            raise utils.PartBuildError ("Cannot find Product <{0}>".format(info.m_name), parent)

    if partType in [PART_TYPE_NuGetProduct, PART_TYPE_Any]:
        nugetProducts = utils.getDomElementsByName(partDom, "NuGetProduct")
        for nugetProduct in nugetProducts:
            if partName == nugetProduct.getAttribute("Name").lower():
                info.m_partType = PART_TYPE_NuGetProduct
                if ret != None:
                    utils.ShowAndDeferMessage("Duplicate NuGetProduct: {0}\n".format(info.GetShortPartDescriptor()), utils.INFO_LEVEL_Essential, utils.RED)
                    return ret
                ret = NuGetProduct(info, nugetProduct)
        if ret != None:
            return ret
        if partType != PART_TYPE_Any:
            raise utils.PartBuildError ("Cannot find NuGetProduct <{0}>".format(info.m_name), parent)

    if partType in [PART_TYPE_UPackProduct, PART_TYPE_Any]:
        upackProducts = utils.getDomElementsByName(partDom, "UPackProduct")
        for upackProduct in upackProducts:
            if partName == upackProduct.getAttribute("Name").lower():
                info.m_partType = PART_TYPE_UPackProduct
                if ret != None:
                    utils.ShowAndDeferMessage("Duplicate UPackProduct: {0}\n".format(info.GetShortPartDescriptor()), utils.INFO_LEVEL_Essential, utils.RED)
                    return ret
                ret = UPackProduct(info, upackProduct)
        if ret != None:
            return ret
        if partType != PART_TYPE_Any:
            raise utils.PartBuildError ("Cannot find UPackProduct <{0}>".format(info.m_name), parent)

    if partType in [PART_TYPE_MultiPlatform, PART_TYPE_Any]:
        multiPlatforms = utils.getDomElementsByName(partDom, "MultiPlatform")
        for mp in multiPlatforms:
            if partName == mp.getAttribute("Name").lower():
                info.m_partType = PART_TYPE_MultiPlatform
                if ret != None:
                    utils.ShowAndDeferMessage("Duplicate MultiPlatform: {0}\n".format(info.GetShortPartDescriptor()), utils.INFO_LEVEL_Essential, utils.RED)
                    return ret
                ret = Part (info, mp)
                utils.ShowAndDeferMessage("[DEPRECATED] MultiPlatform elements are no longer needed because the functionality has been wrapped into Parts. Please replace MultiPlatform with Part in {0}.\n".format(ret.GetShortRepresentation()), utils.INFO_LEVEL_Essential, utils.YELLOW)
                processPartSpec(info, ret, parent, mp)
        if ret != None:
            return ret
        if partType != PART_TYPE_Any:
            raise utils.PartBuildError ("Cannot find MultiPlatform <{0}>".format(info.m_name), parent)

    if partType in [PART_TYPE_ToolPart, PART_TYPE_ToolPackage, PART_TYPE_Any]:
        toolParts = utils.getDomElementsByName(partDom, "ToolPart")
        for tp in toolParts:
            if partName == tp.getAttribute("Name").lower():
                info.m_partType = PART_TYPE_ToolPackage if partType == PART_TYPE_ToolPackage else PART_TYPE_ToolPart
                if ret != None:
                    utils.ShowAndDeferMessage("Duplicate ToolPart: {0}\n".format(info.GetShortPartDescriptor()), utils.INFO_LEVEL_Essential, utils.RED)
                    return ret
                if partType == PART_TYPE_ToolPackage:
                    ret = ToolPackagePart (info, tp)
                else:
                    ret = ToolPart (info, tp)
                processPartSpec(info, ret, parent, tp)
        if ret != None:
            return ret
        if partType != PART_TYPE_Any:
            raise utils.PartBuildError ("Cannot find ToolPart <{0}>".format(info.m_name), parent)

    partSpecs = utils.getDomElementsByName(partDom, "Part")
    thisPart = None
    for spec in partSpecs:
        specName = spec.getAttribute("Name").lower()
        if "" == specName:
            raise utils.PartBuildError ("Part has no 'Name' attribute", parent, spec)
        if partName == specName:
            if thisPart != None:
                utils.ShowAndDeferMessage("Duplicate Part: {0}\n".format(info.GetShortPartDescriptor()), utils.INFO_LEVEL_Essential, utils.RED)
                return thisPart
            if partType == PART_TYPE_Package:
                thisPart = PackagePart (info, spec)
            else:
                thisPart = Part (info, spec)
            processPartSpec(info, thisPart, parent, spec)
    if thisPart != None:
        return thisPart

    raise utils.PartBuildError ("Cannot find Part <{0}>\n".format(info.GetShortPartDescriptor()), parent)

#------------------------------------------------------------------------------------------
# @bsimethod
#------------------------------------------------------------------------------------------
def processPartSpec (partInfo, thisPart, parent, spec):
    def chooseStatic ():
        if parent and parent.m_currentLibType == globalvars.LIB_TYPE_Static or partInfo.m_static:
            return globalvars.LIB_TYPE_Static
        else:
            return globalvars.LIB_TYPE_Dynamic

    # Needs to be read before bindings
    thisPart.m_apiNumber      = spec.getAttribute(globalvars.API_NUMBER)

    # Append this now so it's used throughout.
    if thisPart.m_apiNumber and 'DLL_VENDOR_SUFFIX' in os.environ:
        thisPart.m_apiNumber += os.environ['DLL_VENDOR_SUFFIX']
    bindings = utils.getDomElementsByName(spec, "Bindings")
    if len(bindings) > 0:
        thisPart.m_bindings = PartBindings (thisPart, bindings[0])
        if len(bindings) > 1:
            utils.ShowAndDeferMessage("Part elements can only have one 'Bindings' sub-element. Ignoring any extra elements: {0}\n".format(partInfo.GetShortPartDescriptor()), utils.INFO_LEVEL_Essential, utils.YELLOW)

    thisPart.SetBuildTargetFromDom(spec)
    thisPart.GetLibraryExclusionsFromDom (spec)
    thisPart.SetPlatformExclusionsFromDom (spec)
    thisPart.GetWipInfoFromDom (spec)

    if len(spec.getAttribute("FeatureAspectBuild")) > 0:
        thisPart.m_featureAspectPart = True
        # For now we're going to make the assumption that the FeatureAspects part is a direct child of the Product that has the Feature Aspects.
        if not parent.IsProduct() or None == parent.m_featureAspects:
            raise utils.PartBuildError ("A FeatureAspectBuild part ({0}) must be a direct child of a product with FeatureAspects.".format(thisPart.m_info.m_name), thisPart, spec)
        thisPart.m_featureAspectProduct = parent

    thisPart.SetScopeFlagFromDom (spec)
    thisPart.SetSequentialFlagFromDom (spec)

    thisPart.m_currentLibType = chooseStatic()

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
def readPartFromDom (info, parent, partDom):
    newlyReadPart = readPartFromDomNonCached (info, parent, partDom)
    if newlyReadPart.IsPrivateScope() and parent.m_info.m_file != newlyReadPart.m_info.m_file:
        raise utils.PartBuildError ("Trying to access privately-scoped part from outside the partfile:\n        Accessing Part <{0}>\n        From Part <{1}>\n".format (newlyReadPart, parent), parent)
    return newlyReadPart


