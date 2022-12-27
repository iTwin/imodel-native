#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------

from . import buildpaths, compat, globalvars, utils, buildpart, targetplatform
import os

if compat.py3:
    import pickle
else:
    import cPickle as pickle


GRAPH_FILE_NAME = 'part.cache'
GRAPH_VERSION = 22

#-------------------------------------------------------------------------------------------
# Data for DAGraph.  It would not unpickle if this was defined inside the class.
#
# bsiclass
#-------------------------------------------------------------------------------------------
class GraphData (object):
    def __init__(self, buildNode):
        self.m_buildNode = buildNode
        self.m_children = []
        self.m_parents = []

#-------------------------------------------------------------------------------------------
# A simple Directed Acyclic Graph (DAG) implementation that allows one piece of data (the
#   partNode) and stores both forward and backwards edges.
#
# bsiclass
#-------------------------------------------------------------------------------------------
class DAGraph (object):
    def __init__(self):
        self.m_edgeDict = {}

    def __contains__(self, node):
        return node in self.m_edgeDict

    def AddNode (self, node, buildNode):
        self.m_edgeDict[node] = GraphData(buildNode)

    def DeleteNode (self, node):
        for child in self.m_edgeDict[node].m_children:
            self.m_edgeDict[child].m_parents.remove (node)
        del self.m_edgeDict[node]

    def AddEdge (self, parent, child):
        self.m_edgeDict[parent].m_children.append (child)
        self.m_edgeDict[child].m_parents.append (parent)

    def DeleteEdge (self, parent, child):
        self.m_edgeDict[parent].m_children.remove (child)
        self.m_edgeDict[child].m_parents.remove (parent)

    def Parents (self, node):
        return self.m_edgeDict[node].m_parents

    def Children (self, node):
        return self.m_edgeDict[node].m_children

    def PartNode (self, node):
        return self.m_edgeDict[node].m_buildNode

    def AllNodes (self):
        return list(self.m_edgeDict.keys())

    def Size (self):
        numNodes = len (self.m_edgeDict)
        numEdges = 0
        for _, data in self.m_edgeDict.items():
            numEdges += len(data.m_children)
        return numNodes, numEdges

    def ClearProcessedBit (self):
        # We use the processed bit for various operations.  Use zero so we can use it as a counter too
        for data in self.m_edgeDict.values():
            data.m_buildNode.m_processed = 0
            data.m_buildNode.m_childrenProcessed = 0

    def ClearWalkBit (self):
        for data in self.m_edgeDict.values():
            data.m_buildNode.m_walkProcessed = 0

    def AllPlatforms (self):
        platforms = []
        for currnode in self.AllNodes():
            if not self.PartNode(currnode).m_part.GetPlatform() in platforms:
                platforms.append (self.PartNode(currnode).m_part.GetPlatform())
        return platforms

    def BreadthFirstWalk (self, rootNode):
        self.ClearWalkBit ()
        readQueue = [rootNode]

        while len(readQueue) > 0:
            qNode = readQueue.pop()
            yield qNode

            for child in self.Children (qNode):
                self.PartNode(child).m_walkProcessed += 1 # Count parents processed
                # When all the parents have added contexts into this child you can process it.
                if self.PartNode(child).m_walkProcessed == len(self.Parents(child)):
                    readQueue.append(child)

    def DepthFirstWalk (self, rootNode):
        self.ClearWalkBit ()

        def DepthRecursiveWalk (graphToWalk, currentNode):
            for child in graphToWalk.Children (currentNode):
                if graphToWalk.PartNode(child).m_walkProcessed == 0:
                    for node in DepthRecursiveWalk (graphToWalk, child):
                        yield node

            graphToWalk.PartNode(currentNode).m_walkProcessed = 1
            yield currentNode

        for node in DepthRecursiveWalk (self, rootNode):
            yield node

    # It seems like we usually want the parent and child available.
    # Note that this does not present the root node as a child; it only appears as a parent (possibly multiple times).
    def BreadthFirstWalkOfChildren (self, rootNode):
        self.ClearWalkBit ()
        readQueue = [rootNode]

        while len(readQueue) > 0:
            qNode = readQueue.pop()

            for child in self.Children (qNode):
                yield qNode, child
                self.PartNode(child).m_walkProcessed += 1 # Count parents processed
                # When all the parents have added contexts into this child you can process it.
                if self.PartNode(child).m_walkProcessed == len(self.Parents(child)):
                    readQueue.append(child)

    # Just walks over children of a given root. Doesn't guarantee all parents processed first.
    # Unlike the BreadthFirst methods this will work for subgraphs.
    def ChildWalk (self, rootNode):
        self.ClearWalkBit ()
        readQueue = [rootNode]

        while len(readQueue) > 0:
            qNode = readQueue.pop()
            yield qNode

            for child in self.Children (qNode):
                if not self.PartNode(child).m_walkProcessed:
                    readQueue.append(child)
                self.PartNode(child).m_walkProcessed = 1

    # Just walks over parents of a given "root".
    def ParentWalk (self, rootNode):
        self.ClearWalkBit ()
        readQueue = [rootNode]

        while len(readQueue) > 0:
            qNode = readQueue.pop()
            yield qNode

            for parent in self.Parents (qNode):
                if not self.PartNode(parent).m_walkProcessed:
                    readQueue.append(parent)
                self.PartNode(parent).m_walkProcessed = 1

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class BuildNode (object):
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__(self, part):
        self.m_part = part                  # The Part to process
        self.m_processed = 0                # The general flag for node processing to avoid repeating a node
        self.m_nodeWeight = (0, 0)          # A weight to help do the nodes that would block the most first
        self.m_childrenProcessed = 0        # As child nodes are processed, keep track to know when it's safe to process this node
        self.m_walkProcessed = 0            # Flag for use when doing a breadth-first walk

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __str__(self):
        return self.m_part.m_info.GetNodeIdentifier()
    
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Build (self, paralleBuildInstance):
        if self.m_processed:
            return
        paralleBuildInstance.DoPartAction(self.m_part)
        self.m_processed = True
                

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
class BuildGraph (object):
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__(self):
        self.m_graph = DAGraph()
        self.m_rootNode = None
        self.m_version = GRAPH_VERSION
        self.m_bbVersion = globalvars.bbVersion
        self.m_partFiles = set()                        # Partfiles are used to determine if anything has changed and we need to recreate
        self.m_repoNames = set(x.lower() for x in globalvars.defaultRepos) # Repositories are used for commands that just walk the repositories.
        self.m_lkgRepoNames = set()
        self.m_strategyFiles = set()
        self.m_srcRoot = buildpaths.GetSrcRoot()
        self.m_outRoot = globalvars.programOptions.outRootDirectory
        self.m_host = globalvars.defaultPlatform
        self.m_envVars = {}
        self.m_toolset = None
        self.m_bbModTime = 0
        self.m_dirListCache = {}                        # Stores DirectoryList information
        self.m_toolParts = []
        self.m_strategyCacheTime = None
        
        
    @staticmethod
    def GraphFileName(): return os.path.join (buildpaths.GetBBCacheDir(), GRAPH_FILE_NAME)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def StoreBuildGraph (self):
        graphFileName = self.GraphFileName()
        pickle.dump (self, open (graphFileName, "wb"))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def RecreateBuildGraph (self, rootPartInfo, buildAction):
        # These are set up temporarily for use during recreation
        self.m_buildAction = buildAction  # Used during creation to allow action to be involved; mainly for bb pull.
        self.m_rejectedNodes = set()      # Used during creation to quickly reject nodes of the wrong platform or libtype; emptied after filling graph
        self.m_partFileCaseResolver = {}  # Used to get PartFile names in the on-disk case without having to look the file up for every part
        self.m_dirListCache = {}          # Wipe this cache

        utils.showRewritableLine ('Recreating part cache', utils.INFO_LEVEL_VeryInteresting)
        self.AssembleBuildGraph (rootPartInfo, None)
        self.AddToolsetPart ()
        self.AddStrategyToolParts ()
        self.AddFeatureAspectDependencies ()
        self.UpdateStrategyList ()
        self.m_bbModTime = utils.GetNewestBentleyBuildFileTime()
        self.m_strategyString = self.GetCurrentStrategyString ()
        self.UpdateEnvVars ()
        self.m_strategyCacheTime = globalvars.buildStrategy.m_cacheTime
    
        # Clean up the local variables that are only used during creation
        del (self.m_buildAction)
        del (self.m_rejectedNodes)
        del (self.m_partFileCaseResolver)

        numNodes, numEdges = self.m_graph.Size()
        utils.showRewritableLine ('{0} nodes {1} edges {2} partfiles {3} repositories'.format (numNodes, numEdges, len(self.m_partFiles), len(self.m_repoNames)), utils.INFO_LEVEL_VeryInteresting)

        if 0 == len(self.m_graph.m_edgeDict):
            raise utils.FatalError (message="No parts found for this strategy/platform\n")

        if utils.HasXmlParseErrors(): # Don't store if any errors because then they won't reproduce.
            raise utils.FatalError (message="There were XML parsing errors in the part files; please make sure the XML passes the XSD check.")
        if utils.isBsi():
            self.StoreBuildGraph () # Always cache the last one to disk

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def UpdatePartList (self, partInfo):
        if not os.path.exists (partInfo.m_file):
            modTime = 0
        else:
            modTime = os.stat (partInfo.m_file).st_mtime
            
        self.m_partFiles.add ((partInfo.m_file, modTime))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def UpdateRepoList (self, part):
        if part.FromLkgs():
            self.m_lkgRepoNames.add (part.m_info.m_repo.m_name.lower())
        else:
            # Has to happen after part is found; while additionalRepos is on partInfo it is set there by the part initialization.
            self.m_repoNames.add (part.m_info.m_repo.m_name.lower())
            if part.m_additionalRepos:
                for additionalRepo in part.m_additionalRepos:
                    self.m_repoNames.add (additionalRepo.lower())

        
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def UpdateStrategyList (self):
        for stratFile, modTime in globalvars.buildStrategy.m_accessedFiles.items():
            self.m_strategyFiles.add ((stratFile, modTime))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def UpdateEnvVars (self):
        # Some env vars control what gets built so have to check them too
        for var in os.environ:
            if var.lower().startswith('wippartenable'):
                self.m_envVars[var.lower()] = os.environ[var]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AnyEnvVarsChanged (self):
        allVars = list (self.m_envVars.keys())
        for var in os.environ:
            lvar = var.lower()
            if lvar.startswith('wippartenable'):
                if not lvar in self.m_envVars or os.environ[var] != self.m_envVars[lvar]:
                    return True
                allVars.remove (lvar)
                
        if len (allVars) > 0:
            return True
        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetRepoList (self, withLKGRepos=False):
        if not withLKGRepos:
            return self.m_repoNames

        return self.m_repoNames.union (self.m_lkgRepoNames)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AddToolsetPart (self):
        # Toolset parts need to be handled first, but I also want them to be in the tree as a child
        # of the root node so that they get processed for pull and so forth.
        if not globalvars.buildStrategy.m_toolsetPart or not self.m_rootNode:
            return
            
        # Create a subPartInfo
        subPartInfo = buildpart.SubPartInfo (globalvars.buildStrategy.m_toolsetPart[2], buildpart.PART_TYPE_Part)
        subPartInfo.SetPlatform (None)
        subPartInfo.SetRepo (globalvars.buildStrategy.m_toolsetPart[0])
        subPartInfo.SetFile (globalvars.buildStrategy.m_toolsetPart[1])

        # Read the part & add to tree
        parent = self.m_graph.PartNode(self.m_rootNode).m_part
        partInfo = self.GetSubPartInfo(parent, subPartInfo)
        self.AssembleBuildGraph (partInfo, parent, ignoreSequential=True)
        
        # Record for use during build.
        self.m_toolset = partInfo.GetNodeIdentifier ()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AddStrategyToolParts(self):
        if not globalvars.buildStrategy.m_toolParts or not self.m_rootNode:
            return

        parent = self.m_graph.PartNode(self.m_rootNode).m_part

        allplatforms = self.m_graph.AllPlatforms()

        for repo, pkgType, pkgName, partfile, partname, platformList, includeInTranskit in globalvars.buildStrategy.m_toolParts:
            # Make sure it's valid for this platform
            if platformList:
                found = False
                for plat in platformList:
                    if plat in allplatforms:
                        found = True
                        break
                if not found:
                    continue

            if repo:
                partType = buildpart.PART_TYPE_ToolPart
            else:
                partType = buildpart.PART_TYPE_ToolPackage
            subPartInfo = buildpart.SubPartInfo(partname, partType)

            if partType == buildpart.PART_TYPE_ToolPackage:
                pkgSource = None
                if pkgType.lower() == buildpart.PACKAGE_TYPE_Upack:
                    pkgSource = globalvars.buildStrategy.GetUpackSource(pkgName, None)
                if not pkgSource:
                    raise utils.PartBuildError ("Cannot find package source for SubToolPackage {0}".format(pkgName), parent)

                subPartInfo.SetRepo (pkgSource.m_alias.lower())
                subPartInfo.m_pkgType = pkgType.lower()

                if includeInTranskit:
                    subPartInfo.m_includeToolInTranskit = True
            else:
                subPartInfo.SetRepo(repo)

            subPartInfo.SetFile(partfile)

            partInfo = self.GetSubPartInfo(parent, subPartInfo)
            self.AssembleBuildGraph(partInfo, parent, ignoreSequential=True)
            # Creating the part adds it to the list based on type.

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AddToolPart(self, thisPart):
        thisPart.UpdatePkgToolPartsForCache()
        self.m_toolParts.append(thisPart.m_info.GetNodeIdentifier())

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AddFeatureAspectDependencies (self):
        # For things marked as dependent on all feature aspects being complete, create
        # dependencies on all FeatureAspect parts
        reqFaList = []
        for node in self.m_graph.AllNodes():
            currpart = self.m_graph.PartNode(node)
            if currpart.m_part.m_requiresFA:
                reqFaList.append (currpart)

        if not reqFaList:
            return

        for node in self.m_graph.AllNodes():
            currpart = self.m_graph.PartNode(node)
            if currpart.m_part.m_featureAspectPart:
                for reqFa in reqFaList:
                    utils.showInfoMsg ('Adding FeatureAspect subpart dependecy: {0} depends on {1}\n'.format(reqFa.m_part.m_info.GetShortPartDescriptor(), currpart.m_part.m_info.GetShortPartDescriptor()), utils.INFO_LEVEL_Interesting)
                    self.m_graph.AddEdge (reqFa.m_part.m_info.GetNodeIdentifier(), currpart.m_part.m_info.GetNodeIdentifier())

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AnyPartFileChanged (self, displayMessage):
        for partFile, modTime in self.m_partFiles:
            if not os.path.exists (partFile):
                if displayMessage:
                    utils.showInfoMsg ('Cannot find PartFile: {0}\n'.format(partFile), utils.INFO_LEVEL_VeryInteresting)
                return True
            st = os.stat (partFile)
            if modTime != st.st_mtime:
                if displayMessage:
                    utils.showInfoMsg ('Partfile changed: {0}\n'.format(partFile), utils.INFO_LEVEL_VeryInteresting)
                return True
        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AnyStrategyFileChanged (self):
        for strategyFile, modTime in self.m_strategyFiles:
            if not os.path.exists (strategyFile):
                return True
            st = os.stat (strategyFile)
            if modTime != st.st_mtime:
                return True
        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetCurrentStrategyString (self):
        stratString = globalvars.buildStrategy.m_strategyName
        if len(globalvars.buildStrategy.m_additionalImports) > 0:
            stratString += ';'
            stratString += ';'.join([x for x in globalvars.buildStrategy.m_additionalImports if 'teamLocations' not in x])
        return stratString

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def StrategiesChanged (self):
        return self.m_strategyString != self.GetCurrentStrategyString ()
    
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AnyBentleyBuildFileChanged (self):
        return utils.GetNewestBentleyBuildFileTime() != self.m_bbModTime

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    @staticmethod
    def ClearPartCache ():
        graphFileName = BuildGraph.GraphFileName()
        if os.path.exists (graphFileName):
            os.remove (graphFileName)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    @staticmethod
    def ValidateBuildGraph (graph, rootPartInfo):
        if not graph:
            utils.showInfoMsg ('Part cache object format changed\n', utils.INFO_LEVEL_VeryInteresting)
        elif graph.m_version != GRAPH_VERSION :
            utils.showInfoMsg ('Part cache version is out of date {0} != {1}; flushing part cache\n'.format (graph.m_version, GRAPH_VERSION), utils.INFO_LEVEL_VeryInteresting)
        elif graph.m_bbVersion != globalvars.bbVersion :
            utils.showInfoMsg ('BentleyBuild version is out of date {0} != {1}; flushing part cache\n'.format (graph.m_bbVersion, globalvars.bbVersion), utils.INFO_LEVEL_VeryInteresting)
        elif not graph or not graph.m_rootNode or not graph.m_graph:
            utils.showInfoMsg ('Cache incorrect; flushing part cache\n', utils.INFO_LEVEL_VeryInteresting)
        elif  graph.AnyPartFileChanged(False):
            utils.showInfoMsg ('Part files have been modified; flushing part cache\n', utils.INFO_LEVEL_VeryInteresting)
        elif  graph.AnyStrategyFileChanged():
            utils.showInfoMsg ('Strategy files have been modified; flushing part cache\n', utils.INFO_LEVEL_VeryInteresting)
        elif  graph.AnyBentleyBuildFileChanged():
            utils.showInfoMsg ('BentleyBuild files have been modified; flushing part cache\n', utils.INFO_LEVEL_VeryInteresting)
        elif  graph.m_graph.PartNode(graph.m_rootNode).m_part.GetPlatform().GetXmlName() != rootPartInfo.m_platform.GetXmlName():
            utils.showInfoMsg ('Different platform is specified ({0} != {1}); flushing part cache\n'.format (graph.m_graph.PartNode(graph.m_rootNode).m_part.GetPlatform().GetXmlName(), rootPartInfo.m_platform.GetXmlName()), utils.INFO_LEVEL_VeryInteresting)
        elif  graph.m_graph.PartNode(graph.m_rootNode).m_part.m_info.GetShortPartDescriptor() != rootPartInfo.GetShortPartDescriptor():
            utils.showInfoMsg ('Different root node is specified ({0} != {1}); flushing part cache\n'.format (graph.m_graph.PartNode(graph.m_rootNode).m_part.m_info.GetShortPartDescriptor(), rootPartInfo.GetShortPartDescriptor()), utils.INFO_LEVEL_VeryInteresting)
        elif graph.StrategiesChanged():
            utils.showInfoMsg ('Different build strategies are specified({0} != {1}); flushing part cache\n'.format (graph.m_strategyString, graph.GetCurrentStrategyString()), utils.INFO_LEVEL_VeryInteresting)
        elif graph.AnyEnvVarsChanged ():
            utils.showInfoMsg ('WipPartEnable environment variables have changed; flushing part cache\n', utils.INFO_LEVEL_VeryInteresting)
        elif graph.m_srcRoot != buildpaths.GetSrcRoot():
            utils.showInfoMsg ('SrcRoot has changed; flushing part cache\n', utils.INFO_LEVEL_VeryInteresting)
        elif graph.m_outRoot != globalvars.programOptions.outRootDirectory:
            utils.showInfoMsg ('OutRoot has changed; flushing part cache\n', utils.INFO_LEVEL_VeryInteresting)
        elif graph.m_host != globalvars.defaultPlatform:
            utils.showInfoMsg ('Host platform has changed; flushing part cache\n', utils.INFO_LEVEL_VeryInteresting)
        elif graph.m_strategyCacheTime != globalvars.buildStrategy.m_cacheTime:
            utils.showInfoMsg ('Strategy cache has changed; flushing part cache\n', utils.INFO_LEVEL_VeryInteresting)
        else:
            return True # Good graph
        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    @staticmethod
    def GetBuildGraph (rootPartInfo, buildAction):
        graphFileName = BuildGraph.GraphFileName()
        if os.path.exists (graphFileName):
            try:
                graph = pickle.load (open (graphFileName, "rb"))
            except:
                graph = None
                
            if BuildGraph.ValidateBuildGraph (graph, rootPartInfo):
                graph.PostCacheLoad ()
                return graph
        
        graph = BuildGraph ()
        graph.RecreateBuildGraph (rootPartInfo, buildAction)
        return graph

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def PostCacheLoad (self):
        # Since the pickled Platform objects will always be a personal copy, update them all to the single global version; useful for comparisons.
        for node in self.m_graph.AllNodes():
            currpart = self.m_graph.PartNode(node)
            currpart.m_part.m_info.m_platform = targetplatform.FindPlatformByXMLName (currpart.m_part.m_info.m_platform.GetXmlName())
            # Parts with packages need to point to the same upkg as the LocalRepo; otherwise when the version is updated it won't match.
            if currpart.m_part.IsPackage():
                currpart.m_part.UpdatePkgSourceForCache()
            # Toolpart is marked on strategy items post-strategy load, so it always has to be updated if we don't load the cache.
            if currpart.m_part.IsToolPart():
                currpart.m_part.UpdatePkgToolPartsForCache()
            # Update repositories too?
            # Update part strategies?  Or track strategies and regen.

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetSubPartInfo (self, thisPart, subpart):

        # Platform not specified; use parent
        if not subpart.m_platform:
            platform = thisPart.GetPlatform()
        else:
            platform = subpart.m_platform

        # Repository name not specified; use parent
        if not subpart.m_repoName:
            repo = thisPart.m_info.m_repo
        else:
            if subpart.m_pkgType == globalvars.REPO_UPACK:
                # multi-platform upacks will update the repo name.
                subpart.m_repoName = globalvars.buildStrategy.EnsureUpackLocalRepository (subpart.m_repoName, platform, subpart.m_subpartFile, subpart.m_subpartName)
            repo = globalvars.buildStrategy.FindLocalRepository (subpart.m_repoName)

        # Part file name not specified; use parent
        if not subpart.m_subpartFile:
            partfile = thisPart.m_info.m_relativePartFile
        else:
            partfile = subpart.m_subpartFile
            
        # Static type not specified; use parent
        if globalvars.LIB_TYPE_Parent == subpart.m_libType:
            isStatic = thisPart.IsStatic()
        else:
            isStatic = True if subpart.m_libType == globalvars.LIB_TYPE_Static else False

        # Get package information from repo.
        if not subpart.m_pkgType:
            subpart.m_pkgType = repo.m_repoType
        if subpart.m_pkgType == globalvars.REPO_UPACK:
            # Want all parts coming from partfiles in packages to be of type Package.
            if subpart.m_subpartType == buildpart.PART_TYPE_ToolPart:
                subpart.m_subpartType = buildpart.PART_TYPE_ToolPackage
            elif subpart.m_subpartType != buildpart.PART_TYPE_ToolPackage:
                subpart.m_subpartType = buildpart.PART_TYPE_Package

        partInfo = buildpart.PartInfo (subpart.m_subpartName, repo, partfile, subpart.m_subpartType, platform, isStatic, self.m_partFileCaseResolver)
        if subpart.m_bindToDirectory:
            partInfo.m_bindToDirectory = subpart.m_bindToDirectory

        if partInfo.m_partType == buildpart.PART_TYPE_ToolPart or partInfo.m_partType == buildpart.PART_TYPE_ToolPackage:
            partInfo.m_includeToolInTranskit = subpart.m_includeToolInTranskit

        partInfo.m_pkgType = subpart.m_pkgType
        return partInfo

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AssembleSubPartGraph(self, thisPart):
        for subPart in thisPart.m_subParts:
            subPartInfo = self.GetSubPartInfo(thisPart, subPart)
            self.AssembleBuildGraph (subPartInfo, thisPart)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AssembleBuildGraph (self, partInfo, parent, ignoreSequential=False):
        
        # Keep track of all partfiles we visit
        self.UpdatePartList (partInfo)

        idString = partInfo.GetNodeIdentifier()
        
        # If we've already rejected this one, it's still rejected
        if idString in self.m_rejectedNodes:
            return

        if partInfo.IsExcludedByStrategy():
            utils.showInfoMsg ('Skipping due to Exclude in Part Strategy: {0}\n'.format (partInfo), utils.INFO_LEVEL_RarelyUseful)
            self.m_rejectedNodes.add (idString)
            if parent == None:
                utils.showInfoMsg ('Root node {0} excluded due to Strategy; not building anything\n'.format (partInfo), utils.INFO_LEVEL_VeryInteresting, utils.YELLOW)
            return

        # Add the node if needed
        existingNode = idString in self.m_graph
        if not existingNode:
            # Allow a pre-read operation such as pulling down the source
            if self.m_buildAction:
                parentForPull = parent
                if parent is not None:
                    for parentPartId in self.m_graph.ParentWalk(parent.m_info.GetNodeIdentifier()):
                        if parentPartId is not None:
                            parentPart = self.m_graph.PartNode(parentPartId).m_part
                            if parentPart.m_info.m_buildContext.lower() != partInfo.m_buildContext.lower():
                                parentForPull = parentPart
                                break
                self.m_buildAction.PreReadPart (partInfo, parentForPull)
                partInfo.UpdatePackagePath()

            # Get the actual part
            partDom = buildpart.getPartFileDom (partInfo.m_file, parent, partInfo.m_fromLKG)  # parent just for error messages
            thisPart = buildpart.readPartFromDom (partInfo, parent, partDom)
            if thisPart == None:
                raise utils.PartBuildError ("Cannot find part {0}\n".format(partInfo.m_name), parent)
                
            # Skip if this is excluded based on platform or static library type; this method will print the message
            # We don't know if the part must be excluded until we read the actual part, but the partInfo has the 
            #   platform and libtype that will be used.
            if thisPart.IsExcluded (partInfo.m_platform, partInfo.m_static):
                self.m_rejectedNodes.add (idString)
                if parent == None:
                    if thisPart.m_wipPartEnvVar and not thisPart.m_wipPartEnvVar in os.environ:
                        utils.showInfoMsg ('Root node {0} excluded due to WipPartEnable {1}'.format (thisPart, thisPart.m_wipPartEnvVar), utils.INFO_LEVEL_VeryInteresting, utils.YELLOW)
                    else:
                        utils.showInfoMsg ('Root node {0} excluded due to platform {1}'.format (thisPart, partInfo.m_platform.GetXmlName()), utils.INFO_LEVEL_VeryInteresting, utils.YELLOW)
                return

            # Keep track of root node
            if self.m_rootNode is None:
                self.m_rootNode = idString

            # Keep track of all repositories in use
            self.UpdateRepoList (thisPart)

            msg = "Adding to graph: {0}:{1} Type={2} {3} {4}".format \
            (partInfo.m_buildContext, partInfo.m_name, buildpart.PartTypeNames[partInfo.m_partType], partInfo.m_platform.GetXmlName(), 
            'Static' if partInfo.m_static else 'Dynamic')
            utils.showRewritableLine (msg, utils.INFO_LEVEL_SomewhatInteresting)

            self.m_graph.AddNode(idString, BuildNode(thisPart))

            # Add ToolParts to list
            if thisPart.IsToolPart():
                self.AddToolPart (thisPart)

        # Found a case where a subpart was used without the repository. This yeilded an error when the partfile was read later in a different context.
        # It worked at all because the part was read from the original partfile first and then was existing.
        existingPart = self.m_graph.PartNode(idString).m_part
        if existingPart.m_info.m_repo.m_name.lower() != partInfo.m_repo.m_name.lower():
            parentPart = self.m_graph.PartNode(self.m_graph.Parents(idString)[0]).m_part.GetShortRepresentation()
            subpart1 = 'Part {0} [Repo: {1}]'.format (parentPart, existingPart.m_info.m_repo.m_name)
            subpart2 = 'Part {0} [Repo: {1}]'.format (parent.GetShortRepresentation(), partInfo.m_repo.m_name)
            utils.ShowAndDeferMessage ("Mismatch in Repository attributes of SubPart {0} between {1} and {2}\n  processing part {3}\nIs the Repository attribute missing or incorrect for one of them?".format(partInfo.m_name, subpart1, subpart2, parent.fullPath()), utils.INFO_LEVEL_VeryInteresting)
#            utils.exitOnError (1, "Mismatch in Repository attributes of SubPart {0} between {1} and {2}\n  processing part {3}\nIs the Repository attribute missing or incorrect for one of them?".format(partInfo.m_name, subpart1, subpart2, parent.fullPath()))

        if existingPart.IsToolPart() and not existingPart.m_info.m_includeToolInTranskit:
            existingPart.m_info.m_includeToolInTranskit = partInfo.m_includeToolInTranskit

        # Add the edge
        if parent:
            if idString in self.m_graph.Children (parent.m_info.GetNodeIdentifier()):
                if 'BBPW' in os.environ: # DMS speciality code until they update
                    return
                raise utils.PartBuildError ("Part {0} has duplicate child part {1}:{2}".format(parent.GetShortRepresentation(), partInfo.m_buildContext, partInfo.m_name), parent)
            self.m_graph.AddEdge(parent.m_info.GetNodeIdentifier(), idString)
            # Set up SubProduct binding
            existingPart = self.m_graph.PartNode(idString).m_part
            if existingPart.IsProduct () and partInfo.m_bindToDirectory:
                parent.AddSubProductBinding (existingPart, partInfo.m_bindToDirectory)

        # Recurse if we haven't been here yet.
        if not existingNode:
            self.AssembleSubPartGraph (thisPart)

        # The sequential option will allow the part to evade having to do that by essentially making each child dependent on
        #   the previous one to force them to complete in the required order. The only real use case so far is for Connect Client
        #   where the x64 build relies on the x86 build in a MultiPlatform. Using this is easier than making subparts support
        #   platform, although that may be next.
        if parent and parent.m_sequential and not ignoreSequential:
            prevId = None
            for child in self.m_graph.Children (parent.m_info.GetNodeIdentifier()):
                if prevId and not prevId in self.m_graph.Children (child):
                    self.m_graph.AddEdge(child, prevId)
                prevId = child

