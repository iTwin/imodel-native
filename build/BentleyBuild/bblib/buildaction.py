#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------

from . import buildgraph, globalvars, parallelbuild, strategy, utils, versionutils
import sys, os, re

#-------------------------------------------------------------------------------------------
# The type of "action" being performed by this invocation of BentleyBuild.py
# This class just describes the external interface; use BentleyBuildAction or BentleyBuildRepoAction
# bsiclass
#-------------------------------------------------------------------------------------------
class BentleyBuildActionBase (object):
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__(self):
        # invalid values to make sure they get set
        self.m_numThreads = None
        self.m_actionOptions = None
        self.m_pause = 1            # Seconds to pause waiting for a part to finish. 
        self.m_allPlatforms = None  # Set in outer loop to a list of all platforms we are going to process.

    # The following functions must be overridden in derived classes
    @staticmethod
    def GetHelp():
        raise Exception("Subclasses must have a static GetHelp method")

    def ActionName(self):                   return "none"
    def GetNumThreads(self):                return self.m_numThreads
    def GetPauseDuration(self):             return self.m_pause
    def GetSubAction(self):                 return None         # For items with subactions, return action class here.

    def LoadBuildStrategy(self, strategyName, additionalImports, forceLoad): return strategy.BuildStrategy.GetBuildStrategy (strategyName, additionalImports, forceLoad, None, None)

    # This is the interface that is called from bentleybuild.py
    def PerformAction (self, partInfo):  # pylint: disable=unused-argument
        raise Exception("PerformAction must be implemented by subclasses of BentleyBuildActionBase")
    
    # This method is the actual action that happens on every part or repository.  Must be overridden.
    def DoBuildAction (self, thisPart):
        raise Exception("DoBuildAction must be implemented by subclasses")

    # Optional overrides
    def DoActionSetup (self):               pass    # Called once for a BentleyBuildAction, before PerformAction
    def DoBeforeAction (self):              pass    # Called once for a BentleyBuildAction, after the graph is set up
    def DoAfterAction (self):               pass    # Called once for a BentleyBuildAction after all parts have been processed
    
    def PreReadPart (self, partInfo, parent): pass  # Called before reading a part from the DOM based on a PartInfo.
    def ShowRequiredOutput (self):          pass    # Called after the build has finished, even if there is an exception.
    def AllPlatformsComplete (self, exitstat): pass # Called after all platforms habve been looped over, so once per strategy.
    def AllStrategiesComplete (self, exitstat): pass # Called after all strategies have been completed, so once per build.

    def DoBentleyBuildShutdown (self, buildShutdownContext): pass   # Called after bentleybuild finishes the action. Contains times and exception.  Used for build email.

    def ShouldMessageRunning (self):        return True    # After dumping output, list currently running parts
    
    def ShouldShowSuccessMessage (self):    return False   # Display "Buld Succeeded" at the end. Long-running operations like Get and Build want to do this, clutter on Stat etc.

    def FilterParts (self):                 pass    # Called before parallel part action to filter the part list
    def PrepareToReprocessPart (self, thisPart): pass    # Called if a part fails and PromptOnError or Retry is about to happen.

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ShouldIgnoreErrors (self):
        if not hasattr (self.m_actionOptions, "ignoreErrors"):
            return False
        return self.m_actionOptions.ignoreErrors

#-------------------------------------------------------------------------------------------
# Subclass this class for things that want to walk all the parts; use BentleyBuildRepoAction
#  for actions that just want to walk the repositories.
# bsiclass
#-------------------------------------------------------------------------------------------
class BentleyBuildAction (BentleyBuildActionBase):
    def __init__(self, _args):
        BentleyBuildActionBase.__init__ (self)
        cpuCount = utils.GetCpuCount()
        if cpuCount < 5:
            # Limited core boxes: Just build single-threaded.
            self.m_numThreads = 1
        elif cpuCount <= 16:
            # Standard dev box; leave 4 cores for the CL multithreading and the user to do things
            self.m_numThreads = utils.GetCpuCount()-4 
        else:
            # Empirically we have found that anything more than 12 does not help for MSTN, so leave the rest for the user
            self.m_numThreads = 12

    # Optional overrides
    # Display the ouptut of the build 
    def DisplayBuildOutput (self, succeeded):  return True    #pylint: disable=unused-argument
    def PromptOnError (self):  return False          # Retry failing part on error

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def FilterParts (self):
        # Called just before processing parts so that certain ones can be filtered out by setting the Processed bit; 
        #  return True if anything is processed
        
        # This version filters for No Subparts
        if globalvars.programOptions.noSubParts:
            for currnode in self.m_bgraph.m_graph.AllNodes():
                if currnode != self.m_bgraph.m_rootNode:
                    self.m_bgraph.m_graph.PartNode(currnode).m_processed = 1
            return True

        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def DoPartAction (self, part):
        # Store the action on the part to avoid requiring any globals
        part.m_action = self

        try:
            self.DoBuildAction (part)

        except utils.PartBuildError as err:
            lineNumber = "1" # setting this to 1 because that is what it used to be, and some dev's might copy into their editor.
            if hasattr(err, "m_lineNumber"):
                lineNumber = str(err.m_lineNumber)
            part.ExitOnError (1, "{0}({2}) error : {1}".format (part.m_info.m_file, err.errmessage, lineNumber), trace=err.stackTrace)
        except utils.PartPullError as err:
            # These errors tend to dissapear so trying to make them more obvious
            delineator = '\n--------------------------------------------------------------\n'
            part.ExitOnError (1, delineator + "{0} error : {1}".format (part.m_info.m_file, err.errmessage) + delineator, trace=err.stackTrace)
        except utils.BuildError as err:
            part.ExitOnError (1, "{0}(1) error : {1}".format (part.m_info.m_file, err.errmessage), trace=err.stackTrace)
    
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def BuildWithRetries(self, pBuildMgr):
        status = pBuildMgr.Build()
        if status and self.PromptOnError():
            while (status != 0):
                utils.showInfoMsg ("\nHit return to retry, 'c' to clean or 'x' to exit: ", utils.INFO_LEVEL_Important, utils.YELLOW)
                sys.stdout.flush()
                sys.stdin.flush()
                ans = sys.stdin.read(1)
                if ans.lower() == 'x' or ans.lower() == 'x\n':
                    return status

                for currNodeKey in self.m_bgraph.m_graph.AllNodes():
                    currNode = self.m_bgraph.m_graph.PartNode(currNodeKey)
                    currentPart = currNode.m_part
                    if currentPart.m_failed:
                        currNode.m_logBuffer = []
                        if ans.lower() == 'c' or ans.lower() == 'c\n':
                            currentPart.ReportFailAndClobberBuild ()
                        else:
                            currentPart.m_action.PrepareToReprocessPart(currentPart)

                pBuildMgr.m_errorStop = False
                pBuildMgr.m_errorStatus = []

                status = pBuildMgr.Build ()


        for currnode in self.m_bgraph.m_graph.AllNodes():
            currentPart = self.m_bgraph.m_graph.PartNode(currnode).m_part
            currentPart.m_finished = False
            currentPart.m_failed = False
        return status

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def PerformAction (self, partInfo):
        self.m_bgraph = buildgraph.BuildGraph.GetBuildGraph (partInfo, self)

        self.DoBeforeAction ()
        pBuildMgr = parallelbuild.ParallelBuildManager (self.m_bgraph.m_graph, self.m_bgraph.m_rootNode, self)

        status = self.BuildWithRetries (pBuildMgr)

        for currnode in self.m_bgraph.m_graph.AllNodes():
            currentPart = self.m_bgraph.m_graph.PartNode(currnode).m_part
            currentPart.m_finished = False
            currentPart.m_failed = False

        if not status:
            self.DoAfterAction ()
        return status

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def SetUpBuildContext (self, bgraph):
        # Add in the buildcontexts for each node so that bindings are easy.
        # Also link the partfiles into the buildcontexts so they are usable when looking for ProductDirectories later.
        # This is called from Build and SaveProduct to set up all the contexts
        utils.showInfoMsg ("Setting up for build contexts\n", utils.INFO_LEVEL_Interesting)

        for parentNode, childNode in bgraph.m_graph.BreadthFirstWalkOfChildren (self.m_bgraph.m_rootNode):
            currentPart = bgraph.m_graph.PartNode(parentNode).m_part
            childPart = bgraph.m_graph.PartNode(childNode).m_part

            # Add this context and any part contexts from the parent node, stopping at a product on the way up
            if currentPart.IsProduct():
                # A subproduct directly in a product doesn't link to the parent and thus the parent won't need a ProductDirectoryList.
                # It's a bit magic.  The same "container" effect can also be achieved by marking the parent product as an AddIn.
                if not childPart.IsProduct():
                    value = currentPart.GetShortRepresentation()
                    if not value in childPart.m_parentProductContexts:
                        childPart.m_parentProductContexts[value] = currentPart
            else:
                value = (currentPart.m_info.m_buildContext, currentPart.IsStatic())
                # Add immediate parent
                if not value in childPart.m_parentPartContexts:
                    childPart.m_parentPartContexts.add (value)
                # Also add any contexts from the parent node, stopping at a product on the way up
                childPart.m_parentPartContexts.update (currentPart.m_parentPartContexts)

                # All product contexts should be included
                childPart.m_parentProductContexts.update (currentPart.m_parentProductContexts)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def SetUpSaveLkgBit (self, bgraph):
        # We want the behavior like bb1 where saying that a part is not saved to LKGs means that none of its descendents are either
        # It will be a little different because it will be those descendents where all parents say not to save LKGs.
        for node in bgraph.m_graph.BreadthFirstWalk (bgraph.m_rootNode):
            parentSavesLkg = False
            thisPart = bgraph.m_graph.PartNode(node).m_part

            # Top node saves LKGs (unless otherwise specified)
            if len (bgraph.m_graph.Parents (node)) == 0:
                parentSavesLkg = thisPart.m_info.GetPartStrategy().ShouldSaveLKG(thisPart.m_info.m_platform)
            
            # Check the parents
            for parent in bgraph.m_graph.Parents (node):
                parentPart = bgraph.m_graph.PartNode(parent).m_part
                if parentPart.m_saveLkg:
                    parentSavesLkg = True
                    break

            thisPart.m_saveLkg = False if thisPart.IsPackage() else parentSavesLkg and thisPart.m_info.GetPartStrategy().ShouldSaveLKG(thisPart.m_info.m_platform)

        # Do a second loop only for LKG parts since we don't want them to depend on their parent parts
        # But there might be rare cases when a part not from LKG (most likely some thirparty lib component) has only LKG parents (badly formed graph)
        # So to handle that we do the first loop to apply the trees to be saved over the whole graph and then process the LKGs separately
        # NOTE: The case exists that if an LKG part is set to not be saved, it's children if not marked the same will be saved but that's super rare
        for node in bgraph.m_graph.BreadthFirstWalk (bgraph.m_rootNode):
            thisPart = bgraph.m_graph.PartNode(node).m_part

            if thisPart.FromLkgs() and thisPart.m_info.GetPartStrategy().ShouldSaveLKG(thisPart.m_info.m_platform):
                thisPart.m_saveLkg = True

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ResolvePartVersions (self):
        def getVersionFromPartStrategy (partStrategy):
            if not partStrategy.m_versionString:
                return None

            partVer = os.path.expandvars (partStrategy.m_versionString)
            if None != re.search (r"\${.*}", partVer):
                raise utils.BuildError ('Failed to expand environment variables in version "{0}" specified in part strategy "{1}". Have they been set?'
                    .format(partStrategy.m_versionString, partStrategy.m_name))

            if None == re.search (r"^(\*((-|\.)\*)*|(\d{1,2})(-|\.)(\*((-|\.)\*)*|(\d{1,2})(-|\.)(\*((-|\.)\*)*|(\d{1,2})(-|\.)(\*((-|\.)\*)*|(\d{1,2})))))$", partVer):
                raise utils.BuildError ('The version "{0}" specified in part strategy "{1}" is invalid.'
                    .format(partStrategy.m_versionString, partStrategy.m_name))

            return versionutils.Version (partVer)

        def getInheritedVersion (graph, rootNode, node):
            parents = graph.Parents (node)
            if not parents:
                # Root node, at this point get strategy version
                return globalvars.buildStrategy.GetVersionValue (graph.PartNode (rootNode).m_part)

            parentVer = graph.PartNode (parents[0]).m_part.m_version
            for parent in parents[1:]:
                if parentVer != graph.PartNode (parent).m_part.m_version:
                    # Parents have different versions, default to root version
                    return graph.PartNode (rootNode).m_part.m_version

            # Parents have the same version
            return parentVer

        nodes = self.m_bgraph.m_graph.BreadthFirstWalk (self.m_bgraph.m_rootNode)

        for node in nodes:
            partNode = self.m_bgraph.m_graph.PartNode (node)
            partStrategy = partNode.m_part.m_info.GetPartStrategy()
            version = getVersionFromPartStrategy (partStrategy)

            if not version:
                version = getInheritedVersion (self.m_bgraph.m_graph, self.m_bgraph.m_rootNode, node)
            elif version.NumValues() != 4:
                fallbackVersion = getInheritedVersion (self.m_bgraph.m_graph, self.m_bgraph.m_rootNode, node)

                # DONE: Removed these 2 lines: we will probably want to default to environment set version (or 99 if unset) in case we cannot resolve the version
                #if not fallbackVersion:
                #    raise utils.BuildError ("The version '{0}' specified in part strategy '{1}' cannot be resolved as parents have no version.".format(version, partStrategy.m_name))
                if fallbackVersion:
                    version = version.GetFilledInWildcardVersion (fallbackVersion)

            partNode.m_part.m_version = version

#-------------------------------------------------------------------------------------------
# Subclass this class for things that want to walk just the repositories; use BentleyBuildAction
#  for actions that want to walk the parts.
# bsiclass
#-------------------------------------------------------------------------------------------
class BentleyBuildRepoAction (BentleyBuildActionBase):
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__(self, _args):
        BentleyBuildActionBase.__init__ (self)
        self.m_numThreads = utils.GetCpuCount()-1       # For repository operations more threads can be utilized
        self.m_bgraph = None
        self.m_threadId = None

    # This method is the actual action that happens on every part or repository.  Must be overridden.
    # Here to rename the argument once with one pylint exception.
    def DoBuildAction (self, repo): #pylint: disable=arguments-renamed
        raise Exception("DoBuildAction must be implemented by subclasses")

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def TraverseLkgReposToo (self):
        # Return True to traverse both the normal and LKG repos; the latter is needed for the transkit.
        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AnnounceOutput (self, repo, level, foregroundColor):
        utils.showInfoMsg ('Output from  repository {0}\n'.format(repo.m_name), level, foregroundColor)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def PerformAction (self, partInfo):
        self.m_bgraph = buildgraph.BuildGraph.GetBuildGraph (partInfo, self)
        self.DoBeforeAction ()
        pBuildMgr = parallelbuild.ParallelRepoWalkManager (self.m_bgraph, self)
        status = pBuildMgr.Build ()
        if not status:
            self.DoAfterAction ()
        return status

#-------------------------------------------------------------------------------------------
# Subclass this class for things that want to walk just the parts ignoring all dependencies.
#
# bsiclass
#-------------------------------------------------------------------------------------------
class BentleyBuildPartWalkAction (BentleyBuildActionBase):
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__(self, _args):
        BentleyBuildActionBase.__init__ (self)
        self.m_numThreads = utils.GetCpuCount()-1       # A lot of parts will be skipped
        self.m_bgraph = None
        self.m_threadId = None
        
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AnnounceOutput (self, part, level, foregroundColor):
        utils.showInfoMsg ('Output from  part {0}\n'.format(part.m_info.m_name), level, foregroundColor)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def PerformAction (self, partInfo):
        self.m_bgraph = buildgraph.BuildGraph.GetBuildGraph (partInfo, self)
        self.DoBeforeAction ()
        pBuildMgr = parallelbuild.ParallelPartWalkManager (self.m_bgraph, self)
        status = pBuildMgr.Build ()
        if not status:
            self.DoAfterAction ()
        return status

