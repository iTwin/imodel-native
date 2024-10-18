#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import argparse, os, sys, time, datetime
from . import buildaction, buildgraph, buildpart, buildpaths, cmdutil, compat, globalvars, parallelbuild, provenanceaction 
from . import internal, strategy, symlinks, targetplatform, translationkit, utils
import xml.etree.cElementTree as ElementTree

DEFER_INFO_Level = utils.INFO_LEVEL_Interesting
DEFER_INFO_Color = utils.YELLOW

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class PartFilter (object):
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__ (self, partFileFilter, partNameFilter, makeFileFilter, matchAllSubparts, partPlatforms, libType):
        self.m_partFileFilter   = partFileFilter
        self.m_partNameFilter   = partNameFilter
        self.m_makeFileFilter   = makeFileFilter
        self.m_matchAllSubparts = matchAllSubparts
        self.m_partPlatforms    = partPlatforms
        self.m_libType          = libType
        self.m_matchedParts     = set()

        if (None != self.m_partFileFilter):
            if not ("*" == self.m_partFileFilter) and not (self.m_partFileFilter.lower().endswith (".partfile.xml")):
                self.m_partFileFilter += ".PartFile.xml"

        if (None != self.m_makeFileFilter):
            if "" == self.m_makeFileFilter or os.path.isdir (os.path.abspath (self.m_makeFileFilter)):
                # Like bmake, omitting the makefile name means "the one named after the current working directory"
                self.m_makeFileFilter = os.path.basename (os.getcwd()) + ".mke"
            self.m_makeFileFilter = os.path.abspath (self.m_makeFileFilter)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def _foundMatch (self, buildPart):
        if self.m_matchAllSubparts:
            self.m_matchedParts.add (buildPart)
        return True

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def DoesMatch (self, buildPart, buildGraph):
        partInfo = buildPart.m_info

        # matchAllSubparts -- if this part's parent matched, this part matches too
        for parent in buildGraph.Parents (partInfo.GetNodeIdentifier()):
            if buildGraph.PartNode(parent).m_part in self.m_matchedParts:
                return self._foundMatch (buildPart)

        # partfile/partname filter
        if (None != self.m_partFileFilter) and (None != self.m_partNameFilter):
            if utils.NameMatch (partInfo.m_file, self.m_partFileFilter) or utils.NameMatch (os.path.basename (partInfo.m_file), self.m_partFileFilter):
                if utils.NameMatch (partInfo.m_name, self.m_partNameFilter):
                    if None != self.m_partPlatforms and buildPart.GetPlatform() not in self.m_partPlatforms:
                        return False
                    if None != self.m_libType and ((self.m_libType == 'dynamic' and buildPart.IsStatic()) or (self.m_libType == 'static' and not buildPart.IsStatic())):
                        return False
                    return self._foundMatch (buildPart)

        # makefile filter
        if (None != self.m_makeFileFilter) and (None != buildPart.m_bmakeFile):
            partMakefile = os.path.join (partInfo.m_partFileDir, os.path.expandvars (buildPart.m_bmakeFile))
            if (utils.NameMatch (partMakefile, self.m_makeFileFilter)):
                if None != self.m_partPlatforms and buildPart.GetPlatform() not in self.m_partPlatforms:
                    return False
                if None != self.m_libType and ((self.m_libType == 'dynamic' and buildPart.IsStatic()) or (self.m_libType == 'static' and not buildPart.IsStatic())):
                    return False
                return self._foundMatch (buildPart)

        return False

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
def GetTabCompletionList(bgraph):
    completeList = []
    partDict = {}
    for i in bgraph.m_graph.AllNodes():
        i_partInfo = bgraph.m_graph.PartNode(i).m_part.m_info
        if i_partInfo.m_buildContext + ":" + i_partInfo.m_name not in completeList:
            completeList.append(i_partInfo.m_buildContext + ":" + i_partInfo.m_name)
        if not i_partInfo.m_name in partDict:
            partDict[i_partInfo.m_name] = []
        if not i_partInfo.m_buildContext in partDict[i_partInfo.m_name]:
            partDict[i_partInfo.m_name].append(i_partInfo.m_buildContext)
    completeList.sort()
    # Also include unambiguous partnames that match the input, i.e. the partname shows up in only one build context
    uniqueParts = [i for i,j in sorted(partDict.items()) if len(j) == 1]
    uniqueParts.sort()
    # Add a separator line: ==
    return completeList + ["=="] + uniqueParts

#-------------------------------------------------------------------------------------------
# The BentleyBuildAction that processes Parts calling their BuildCommands and Bindings
# bsiclass
#-------------------------------------------------------------------------------------------
class BuildPartAction (buildaction.BentleyBuildAction):
    def ActionName(self):  return "Build"

    @staticmethod
    def GetHelp ():
        return "Build, starting at the part specifed in the BuildStrategy or using 'bb -p <partname>'"

    @staticmethod
    def GetUsage():      return "%(prog)s build [args]"

    #--------------------------------------------------------------------------------
    # bsimethod
    #--------------------------------------------------------------------------------
    @staticmethod
    def GetParser():
        return BuildPartAction.GetParserWithAction (False, BuildPartAction.GetHelp())

    #--------------------------------------------------------------------------------
    # bsimethod
    #--------------------------------------------------------------------------------
    @staticmethod
    def GetParserWithAction(usingRebuild, helpStr):
        parser = utils.ArgumentParser (usage=BuildPartAction.GetUsage(), description=helpStr)
        parser.add_argument ("-i",  "--ignoreErrors",               action="store_true",    dest="ignoreErrors",            help="Continue even if build command fails")
        parser.add_argument ("-I",  "--ignoreBmakeErrors",          action="store_true",    dest="ignoreBmakeErrors",       help="Ignore errors AND tell launched bmakes to ignore errors too")
        parser.add_argument (       "--ignoreProvenance",           action="store_true",    dest="ignoreProvenanceErrors",  help="Ignore only provenance errors")
        parser.add_argument ("-a",  "--autoRetry",                  action="store_true",    dest="autoRetry",               help="If build command fails, perform clean and then retry")
        parser.add_argument ("-p",  "--promptOnError",              action="store_true",    dest="promptOnError",           help="If build command fails, prompt for action. Allows trying again on same part")
        parser.add_argument ("-b",  "--brief",                      action="store_true",    dest="brief",                   help="Do not show the output of parts that succeed")
        parser.add_argument ("-l",  "--allowNewSymlinkTargets",     action="store_true",    dest="allowNewSymlinkTargets",  help="Suppress errors when binding an existing symlink to a new target")
        parser.add_argument (       "--useLocalizedDir",            action="store_true",    dest="useLocalizedDir",         help="[Transkit Only] Forces build to use translated resources from Localized/$(Language) folder.")
        parser.add_argument (       "--useL10nRepoDir",             action="store_true",    dest="useL10nRepoDir",          help="[Developer Only] Forces build to use translated resources from l10n repositories.")
        parser.add_argument ("-k",  "--tkOnly",                     action="store_true",    dest="tkOnly",                  help="Only affect transkit build")
        parser.add_argument (       "--excludeDefaultLanguage",     action="store_true",    dest="excludeEnglish",          help="Exclude default added in 'en'.")
        parser.add_argument ("-d",  "--define",                     action="append",        dest="vars",                    help="Define environment variable for build commands")
        parser.add_argument (       "--bmakeOpt",                   action="store",         dest="bmakeOpt",                help="bmake arguments (usually +p and +v for debugging)")
        parser.add_argument (       "--errorLog",                   action="store",         dest="linkErrorLog",            help="On failure, link the build log for the last failing part to this file name.  Must be full path.")
        parser.add_argument (       "--watchLog",                   action="store",         dest="watchLog",                help="Log all OutRoot filesystem changes for each part to logs in this directory name.  Must be full path to directory.  Also implies -N0")
        parser.add_argument (       "--procMon",                    action="store_true",    dest="procMon",                 help="Make registry reads for marking parts in a sysinternals ProcMon log")
        parser.add_argument ("-e",  "--emailUser",                  action="store_true",    dest="emailNotification",       help="Toggle e-mail notifications to be sent on build completion to the address specified by BB_EMAIL_ADDRESS")
        parser.add_argument (       "--address",                    action="store",         dest="emailAddress",            help="Override the default email to send build notifications to.")
        group1 = parser.add_argument_group(title="Force build options")
        group1.add_argument ("-c",  "--clean",                      action="store_true",    dest="clobberBuild",            help="Launch all build commands with +Da option, for npm/yarn runs script 'clean'")
        parser.add_argument_group (group1)
        
        # Rebuild-only options
        if usingRebuild:
            group1.add_argument ("-r",  "--removeBuildLogs",        action="store_true",    dest="removeBuildLogs",     help="Delete the logs for each part to rebuild before starting build.  This leaves the build re-startable with 'bb build'.")
            parser.add_argument ("-m",  "--makefile",               action="append",        dest="makefile",            help="Rebuild parts that would call the specified makefile")
            parser.add_argument ("-s",  "--buildSubparts",          action="store_true",    dest="buildSubparts",       help="Rebuild subparts of the specified parts")
            parser.add_argument ("-D",  "--buildDependents",        action="store_true",    dest="buildDependents",     help="Rebuild all dependents (\"superparts\") of the specified parts")
            parser.add_argument (       "--ide",                    action="store_true",    dest="IDE",                 help="Define bmake macro IDE to allow launching Visual Studio for project files")
            parser.add_argument ("-t",  "--tabComplete",            action="store_true",    dest="tabComplete",         help="Provides a tab-completion prompt for the parts list. Rebuild multiple parts by separating them with a space")
            parser.add_argument ("-A", "--partplatform",            action="store",         dest="partPlatform",        help="Define part platform to use. Platforms can be interated with +, i.e. x86+x64.")
            parser.add_argument ("-L", "--libtype",                 action="store",         dest="libType",             help="Define part library type: static, dynamic. Default is both.")
            parser.add_argument ('args', nargs=argparse.REMAINDER)  # Collects up the rest of the arguments to pass to command
        else:
            parser.add_argument (       "--reverseSubpartOrder",    action="store_true",    dest="reverseSubpartOrder", help="Reverse the order of subparts to help find order dependencies when moving to bb2. Also forces single-threaded build.")
            parser.add_argument (       "--defer",                  action="store",         dest="deferType",           help='A list of types and deferral action. Actions are a pass number or Never. Ex: --defer="tests=2;docs=Never"')
            group1.add_argument (       "--skipPartBuild",          action="store_true",    dest="skipPartBuild",       help="Skip building the part, do only the binding")
            group1.add_argument ("-r",  "--removeBuildContext",     action="store_true",    dest="removeBuildContext",  help="Delete BuildContexts directory before starting build")
            group1.add_argument ("-f",  "--forceBuild",             action="store_true",    dest="forceBuild",          help="Rebuild even if already built (when using BuildFromSource=\"Once\") ")
            group1.add_argument (       "--tmr",                    action="store_true",    dest="totalMassiveRebuild", help="Total Massive Rebuild (delete entire output tree, but don't start build)")
            group1.add_argument (       "--tmrbuild",               action="store_true",    dest="tmrThenBuild",        help="Like --tmr, but starts the build after successfully deleting output.")
            group1.add_argument (       "--noprompt",               action="store_true",    dest="tmrNoPrompt",         help="Do not prompt on --tmr (use with care!)")

        # useL10nRepoDir=(translationkit.IsDeveloperShell () == False), useLocalizedDir=(translationkit.IsTranskitShell () == False), \
        # For a Developer shell, default is to use Pseudo-localized dir, unless told otherwise.
        # For a Transkit shell, default is to use Repository dir, unless told otherwise.

        parser.set_defaults (ignoreErrors=False, clobberBuild=False, skipPartBuild=False, removeBuildContext=False, removeBuildLogs=False, forceBuild=False,
                             ignoreBmakeErrors=False, IDE=False, totalMassiveRebuild=False, tmrNoPrompt=False, tmrThenBuild=False, 
                             buildSubparts=False, buildDependents=False, useL10nRepoDir=(translationkit.IsTranskitShell ()), partplatform=None, libtype=None,
                             useLocalizedDir=(not translationkit.IsTranskitShell ()), makefile=[], excludeEnglish=False, promptOnError=False, tkOnly=False,
                             linkErrorLog='', watchLog='', brief=False, reverseSubpartOrder=False, emailNotification=False, deferType=None, tabComplete=None)
        return parser

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__(self, inArgs, fromOptions=None):

        buildaction.BentleyBuildAction.__init__ (self, inArgs)

        self.m_usingRebuild = ("Rebuild" == self.ActionName() or "Clean" == self.ActionName())
        if self.m_usingRebuild:
            self.m_pause = 0.1          # Rebuilds are much shorter than builds so shrink the pause.
        else:
            self.m_pause = 2            # Seconds to pause waiting for a part to finish. Building takes longer and 2 seconds is a good choice
        self.m_rebuildPartList = []  # Use this to look for any rebuilt arguments that don't build parts
        self.m_toolCacheTree = None
        self.m_toolCacheUpdated = False
        self.m_performedClean = False
        self.m_processedNugetPackages = set()
        self.m_processedParts = set()  # To support multi-strategy rebuilds we keep track and don't reprocess parts
        self.m_prioritySet = False     # If we've set priority yet; for multi-strat cases

        parser = BuildPartAction.GetParserWithAction (self.m_usingRebuild, self.GetHelp())
        buildOptions = parser.parse_args(inArgs)
        if fromOptions:
            buildOptions = fromOptions

        if buildOptions.IDE:
            if buildOptions.bmakeOpt == None:
                buildOptions.bmakeOpt = ""
            buildOptions.bmakeOpt += " -dIDE"

        if translationkit.IsTranskitShell ():
            buildOptions.useL10nRepoDir = (not buildOptions.useLocalizedDir and not globalvars.programOptions.useLocalizedDir)

        translationkit.SetUseL10NRepos (buildOptions.useL10nRepoDir)

        # If useLocalizedDir is set, then create temp files to get build going. But not all transkit file format are going support this.
        # If items is further used by installer or any linker or compiler, then vendor will need to copy items manually for such an exception
        translationkit.SetCreateTempMissingTranskitSources ((buildOptions.useLocalizedDir or globalvars.programOptions.useLocalizedDir) and not buildOptions.useL10nRepoDir)
        translationkit.gTkOnlyBuild     = buildOptions.tkOnly

        self.m_partFilters = None
        if self.m_usingRebuild:
            buildOptions.forceBuild = True
            self.m_partFilters = []
            partPlatforms = None
            libType = buildOptions.libType.lower() if buildOptions.libType else None
            if libType != None and not (libType == 'static' or libType == 'dynamic'):
                raise utils.StrategyError ("ERROR: unknown libType. \n Possible options: static or dynamic ")
            if buildOptions.partPlatform != None:
                partPlatforms = targetplatform.ResolvePlatformList(buildOptions.partPlatform, True, "ERROR: Unknown architecture argument value '{0}'. \n '-A' or '--architecture' must be one of {1}\n")
            for filterString in buildOptions.args:
                (partFileFilter, partNameFilter) = buildpart.splitPartDescriptor (filterString)
                self.m_rebuildPartList.append (partNameFilter)
                if None == partFileFilter:
                    partFileFilter = "*"
                if None == partNameFilter:
                    partNameFilter = "*"
                self.m_partFilters.append (PartFilter (partFileFilter=partFileFilter, partNameFilter=partNameFilter, makeFileFilter=None, matchAllSubparts=buildOptions.buildSubparts,
                partPlatforms=partPlatforms, libType=libType))
            for filterString in buildOptions.makefile:
                self.m_partFilters.append (PartFilter (partFileFilter=None, partNameFilter=None, makeFileFilter=filterString, matchAllSubparts=buildOptions.buildSubparts,
                partPlatforms=partPlatforms, libType=libType))

        if buildOptions.ignoreBmakeErrors:
            buildOptions.ignoreErrors = True

        if buildOptions.clobberBuild:
            buildOptions.ignoreErrors = True
            buildOptions.forceBuild = True

        # See if the environment is requesting a global limit on numThreads.
        # Check this before other mechanical limitation overrides below.
        if 'BB_DEFAULT_NUM_BUILD_THREADS' in os.environ:
            self.m_numThreads = int(os.environ['BB_DEFAULT_NUM_BUILD_THREADS'])
            
        if buildOptions.reverseSubpartOrder or buildOptions.watchLog or buildOptions.procMon:
            self.m_numThreads = 0

        if None != buildOptions.vars:
            self.m_envFromArgs = {}
            for var in buildOptions.vars:
                utils.defineEnvVar(self.m_envFromArgs, var)

        self.m_actionOptions    = buildOptions

        self.m_processFeatureAspectsFirst = False  # This should be removed and we will always do this once we trim down FA stuff a bit.

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def DoActionSetup (self):
        # Do the stuff that requires the strategies to be set up.
        if None == globalvars.programOptions.outRootDirectory:
            raise utils.BuildError ("No OutputRootDirectory specified")

        if translationkit.IsMultilingualBuild ():
            translationkit.SetUseL10NRepos (True)

        # Parse the deferral options
        if self.m_actionOptions.deferType:
            splitTypes = self.m_actionOptions.deferType.split(';')
            if len (splitTypes) == 0:
                raise utils.BuildError ('Incorrect --defer syntax: {0}\n'.format(repr(self.m_actionOptions.deferType)))
            for deferType in splitTypes:
                sp = deferType.split('=')
                if len (sp) != 2:
                    raise utils.BuildError ('Incorrect --defer syntax for item {0}\n'.format(repr(deferType)))
                defer = sp[1]
                if defer.lower() == strategy.DEFER_OPTION_Never.lower():
                    deferVal = strategy.DEFER_OPTION_Never  # Fix case if needed
                else:
                    deferVal = int(defer)
                if not strategy.IsValidDeferValue(defer):
                    raise utils.BuildError ('Incorrect --defer syntax; action must be a number or {0}\n'.format(strategy.DEFER_OPTION_Never))
                deferStrat = strategy.DeferStrategy (sp[0], deferVal)
                globalvars.buildStrategy.AddDeferredStrategy (deferStrat)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def DisplayBuildOutput (self, succeeded):
        if not succeeded:
            return True
        return not self.m_actionOptions.brief

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def PromptOnError (self):
        return self.m_actionOptions.promptOnError

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ShouldShowSuccessMessage (self):     
        return True    # Display "Buld Succeeded" at the end. Long-running operations like Get and Build want to do this.
    
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def CleanAllOutput (self):
        if self.m_performedClean:
            return
        self.m_performedClean = True # True if we tried, regardless of whether user says yes or no.
        
        # Collect up all the platforms in use
        platforms = self.m_bgraph.m_graph.AllPlatforms()
        platforms += list (set(self.m_allPlatforms)-set(platforms))
                
        # Check if there is anything to do; don't want to ask if the dirs are empty
        needToAsk = False
        for platform in platforms:
            dirToCheck = buildpaths.getOutputRoot(platform, False)
            if os.path.exists (dirToCheck) and len (os.listdir (dirToCheck)) > 0:
                needToAsk = True
                break
                
        if needToAsk:
            # Check with the user
            if self.m_actionOptions.tmrNoPrompt:
                ans = "y"
            else:
                platString = '\n'.join (['    ' + buildpaths.getOutputRoot(platform, False) for platform in platforms])
                utils.showInfoMsg ("Are you sure you want to TMR? This will delete: \n{0}\n [y/n] ".format(platString), utils.INFO_LEVEL_Essential)
                ans = sys.stdin.read(1)
            # Handle the cleaning
            if ans.lower() == 'y':
                for platform in platforms:
                    utils.cleanDirectory (buildpaths.getOutputRoot(platform, False), deleteFiles=True)
                utils.cleanDirectory (buildpaths.GetToolsOutputRoot())

                for platform in platforms:
                    allRemainingFiles = utils.listAllFiles (buildpaths.getOutputRoot(platform, False))
                    if allRemainingFiles:
                        message = 'Error: Files remain after clean. Please remove them manually.\n'
                        for arFile in allRemainingFiles:
                            message += '  '+arFile+'\n'
                        raise utils.BuildError (message)

        if not self.m_actionOptions.tmrThenBuild:
            utils.showInfoMsg ("\nRun the build action without --tmr to perform the build\n", utils.INFO_LEVEL_VeryInteresting, utils.LIGHT_BLUE)
            exit (0)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def RemoveBuildContexts (self):
        finishedContexts = set()
        utils.showStatusMsg ("Cleaning old build contexts", utils.INFO_LEVEL_VeryInteresting)
        for currnode in self.m_bgraph.m_graph.AllNodes():
            part = self.m_bgraph.m_graph.PartNode(currnode).m_part
            if (part.GetPlatform(), part.IsStatic()) not in finishedContexts:
                finishedContexts.add ((part.GetPlatform(), part.IsStatic()))
                utils.cleanDirectory (part.GetBuildContextRoot(part.IsStatic()))
                utils.cleanDirectory (part.GetProductRoot(), deleteFiles=True)
                utils.cleanDirectory (part.GetTranskitRoot())
                utils.cleanDirectory (part.GetLogFileRoot(), deleteFiles=True)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def BuildFeatureAspects (self):
    
        if not self.m_processFeatureAspectsFirst:
            return

        alreadyProcessedBeforeFA = set()
        faSet = set()

        for currnode in self.m_bgraph.m_graph.AllNodes():
            # Store off the toolset and any other processed items
            if self.m_bgraph.m_graph.PartNode(currnode).m_processed:
                alreadyProcessedBeforeFA.add (currnode)

            # Get all fa nodes and their dependencies
            currentPart = self.m_bgraph.m_graph.PartNode(currnode).m_part
            if currentPart.m_featureAspectPart and not self.m_usingRebuild:
                
                for faNode in self.m_bgraph.m_graph.ChildWalk (currnode):
                    faSet.add (faNode)

        if faSet:
            utils.showInfoMsg ("Processing featureaspect parts\n", utils.INFO_LEVEL_VeryInteresting)
            
            def FaFilterParts (_self):
                # Mark everything _not_ in the list as processed
                for currnode in self.m_bgraph.m_graph.AllNodes():
                    self.m_bgraph.m_graph.PartNode(currnode).m_processed = 0 if currnode in faSet else 1
                return True

            # Now process all FA parts using a filter for any non-FA parts
            origFilter = self.FilterParts                 # I think this is a pylint bug since it's defined in the superclass   pylint: disable=access-member-before-definition
            self.FilterParts = FaFilterParts
            pBuildMgr = parallelbuild.ParallelBuildManager (self.m_bgraph.m_graph, self.m_bgraph.m_rootNode, self)
            pBuildMgr.Build ()
            self.FilterParts = origFilter

            # Finally clear the processed bit for everything that wasn't built
            faSet.union (alreadyProcessedBeforeFA)
            for currnode in self.m_bgraph.m_graph.AllNodes():
                self.m_bgraph.m_graph.PartNode(currnode).m_processed = 1 if currnode in faSet else 0
                # Might as well process these now
                self.m_bgraph.m_graph.PartNode(currnode).m_part.ProcessFeatureAspects () 

            utils.showInfoMsg ("FeatureAspect parts finished\n\n", utils.INFO_LEVEL_VeryInteresting)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def BuildNugetPackages (self):
        nugetPkgList = list()

        # Build up a list of NuGetProducts in a breadth-first order to correctly add the dependency chain of SubNuGetProducts
        for currnode in self.m_bgraph.m_graph.BreadthFirstWalk(self.m_bgraph.m_rootNode):
            currPart = self.m_bgraph.m_graph.PartNode(currnode).m_part
            if currPart.IsNuGetProduct() and not self.m_usingRebuild:
                nugetPkgList.append(currnode)

        nugetPkgList.reverse()

        # Gathering all of the NuGetProducts and then traversing through them
        for currNode in nugetPkgList:
            currPart = self.m_bgraph.m_graph.PartNode(currNode).m_part
            for childNode in self.m_bgraph.m_graph.ChildWalk (currNode):
                # don't want to add a dependency on itself
                if currNode == childNode:
                    continue

                childPart = self.m_bgraph.m_graph.PartNode(childNode).m_part

                # Don't bind subparts of child-products to this NuGetProduct - we only want to bind to children for
                # which the NuGetProduct is in the Parent Product Context
                if currPart.GetShortRepresentation() not in childPart.m_parentProductContexts:
                    continue

                if childPart.m_deferType and globalvars.buildStrategy.GetDeferStrategy (childPart.m_deferType) == strategy.DEFER_OPTION_Never:
                    continue

                utils.showInfoMsg("Binding part {0} to NuGetProduct {1}\n".format(childPart.m_info.m_name, currPart.m_info.m_name), utils.INFO_LEVEL_Interesting)
                currPart.BindFromChildPart (childPart, None, False, False)

                if not childPart.IsNuGetProduct():
                    continue

                for targetFramework in childPart.m_nuspec.GetTargetFrameworks():
                    currPart.m_nuspec.AddDependency(childPart.m_nuspec.m_name, childPart.m_nuspec.m_version, targetFramework)

            # After all dependencies are added create the NuGetPackage
            currPart.CreateNugetPackage()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ToolFilter (self):
        for node in self.m_bgraph.m_graph.BreadthFirstWalk (self.m_bgraph.m_rootNode): # Mark all non-toolparts done
            if not node in self.m_bgraph.m_toolParts:
                self.m_bgraph.m_graph.PartNode(node).m_processed = 1  
        return True

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def DoBeforeAction (self):
        # setup optional watch log
        self.SetupWatchLog()

        if self.m_actionOptions.tabComplete:
            self.SetupTabCompletion()

        # Have to do these once we've collected the all the parts in order to know platforms to delete
        if self.m_actionOptions.totalMassiveRebuild or self.m_actionOptions.tmrThenBuild:
            self.CleanAllOutput ()
        elif self.m_actionOptions.removeBuildContext:
            self.RemoveBuildContexts ()

        # Set up the build context information on the parts
        self.SetUpBuildContext (self.m_bgraph)

        # Link the partfile into the buildcontext
        linkedPartFiles = set()
        
        def LinkContext (srcFile, destFile):
            if not destFile in linkedPartFiles:
                linkedPartFiles.add (destFile) # Have to use the destination because it can get linked into static and dynamic buildcontexts
                try:
                    symlinks.createFileSymLink (destFile, srcFile, checkSame=False, checkTargetExists=True)
                except symlinks.SymLinkError as err:
                    raise utils.BuildError (err.errmessage, trace=err.stackTrace)
        
        for currnode in self.m_bgraph.m_graph.AllNodes():
            currentPart = self.m_bgraph.m_graph.PartNode(currnode).m_part
            _, pfName = os.path.split (currentPart.m_info.m_file)
            destFile = os.path.join(currentPart.GetOutputBuildContextDir(currentPart.m_info.m_buildContext),pfName)
            LinkContext (currentPart.m_info.m_file, destFile)

        # For products, look through the ProductDirectoryList and link up everything there as well.
        # Doing this as a seperate loop because Repository was typically not specified on Directories elements and this keeps things working
        for currnode in self.m_bgraph.m_graph.AllNodes():
            currentPart = self.m_bgraph.m_graph.PartNode(currnode).m_part
            if currentPart.IsProduct() and currentPart.m_directories:
                for prodDirRef in currentPart.m_directories.m_listReferences:
                    # Use the current product root to get platform and static
                    destFile = os.path.join(currentPart.GetBuildContextRoot(None), prodDirRef[1], prodDirRef[1]+'.PartFile.xml') 
                    LinkContext (prodDirRef[3], destFile)
                    
        # Lower the file IO priority for the current python. Has to be done here because it has to apply to all threads.
        if not self.m_prioritySet:
            self.m_prioritySet = True
            cmdutil.beginBackGroundIOPriority ()
        
        # Reverse the subparts in the build if requested
        if self.m_actionOptions.reverseSubpartOrder:
            for currnode in self.m_bgraph.m_graph.AllNodes():
                if not self.m_bgraph.m_graph.PartNode(currnode).m_part.m_sequential:
                    self.m_bgraph.m_graph.Children (currnode).reverse()
        
        # Check all provenance. Doing it as a separate action to take advantage of threading since it has to be done children first.
        if not self.m_usingRebuild:
            provenanceaction.RunProvenance (self, self.m_actionOptions.ignoreProvenanceErrors)
        
        # Link up any tools first so they are available
        utils.showInfoMsg ("Processing Default Tool parts\n", utils.INFO_LEVEL_VeryInteresting)
        for toolName, toolType in globalvars.defaultTools:
            if toolType == globalvars.REPO_UPACK:
                upackSrc = globalvars.buildStrategy.GetUpackSource (toolName, targetplatform.GetHostPlatform())
                buildpart.ToolPart.BindToolCacheUpack (upackSrc)
                self.UpdateToolCacheData (upackSrc.m_name)
            elif toolType == globalvars.REPO_NUGET:
                nuget = globalvars.buildStrategy.GetNugetSource (toolName)
                buildpart.ToolPart.BindToolCacheNuGet (nuget)
                self.UpdateToolCacheData (nuget.m_name)
            elif toolType == globalvars.REPO_GIT:
                buildpart.ToolPart.BindToolCacheRepo (toolName)
                self.UpdateToolCacheData (toolName)
            else:
                raise utils.BuildError('Unsupported Default Tool Type: {0} {1}'.format(toolName, toolType))

        if self.m_bgraph.m_toolParts:
            oldFilter = self.FilterParts
            self.FilterParts = self.ToolFilter

            pBuildMgr = parallelbuild.ParallelBuildManager (self.m_bgraph.m_graph, self.m_bgraph.m_rootNode, self)
            utils.showInfoMsg ("Processing Tool parts\n", utils.INFO_LEVEL_VeryInteresting)
            pBuildMgr.Build ()
            utils.showInfoMsg ("Tool parts finished\n\n", utils.INFO_LEVEL_VeryInteresting)
            self.FilterParts = oldFilter

        # Process the toolset part
        if self.m_bgraph.m_toolset and not self.m_usingRebuild:
            pBuildMgr = parallelbuild.ParallelBuildManager (self.m_bgraph.m_graph, self.m_bgraph.m_toolset, self)
            utils.showInfoMsg ("Processing toolset part\n", utils.INFO_LEVEL_VeryInteresting)
            # Using BuildWithoutThreads because that works better (at all) with an off-top root. To do the parallel build
            # would require filtering, and the intent here is that is one part with one makefile. Thus we simplify until worse is needed.
            pBuildMgr.BuildWithoutThreads ()
            utils.showInfoMsg ("Toolset part finished\n\n", utils.INFO_LEVEL_VeryInteresting)

        # Process all FeatureAspect parts up front too.
        self.BuildFeatureAspects()

        # Resolve part versioning
        if 'BB_TEST_STRATEGY_VERSION' in os.environ:
            self.ResolvePartVersions()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def DoBuildAction (self, thisPart):
        """Build a part and all of its subparts."""

        if(thisPart.m_finished):
            return

        if self.m_actionOptions.watchLog or self.m_actionOptions.procMon:
            self.HandleWatchLog (thisPart)

        self.ResetPartMethodsForClean (thisPart)

        if not thisPart.FromLkgs() and not translationkit.IsTKBuildOnly ():
            # Handle SdkSources (just binds into build context)
            for component in thisPart.m_sdkSources:
                sdkSource = globalvars.buildStrategy.GetSdkSource (component)
                if sdkSource.IsPlatformAvailable (thisPart.GetPlatform()):
                    sdkSourceDir = symlinks.getSymlinkTarget(sdkSource.GetCurrentSdkDir (thisPart.GetPlatform()))
                    sdkSourceDir = os.path.join (sdkSourceDir, sdkSource.m_productDir)

                    outputBuildContext = thisPart.GetOutputBuildContextDir(thisPart.m_info.m_buildContext, thisPart.IsStatic())
                    sdkContextDir = os.path.join (outputBuildContext, 'SdkSources', sdkSource.m_name)

                    utils.showInfoMsg ("Connecting {0} to SdkSources {1}\n".format (sdkContextDir, sdkSource.m_name), utils.INFO_LEVEL_SomewhatInteresting)
                    thisPart.PerformBindToDir (sdkContextDir, sdkSourceDir, True, True)

            # Handle NuGet packages (flattens dir structure, binds to build context)
            for nugetUsage in thisPart.m_nugetPackages:
                nugetUsage.m_pkgSource.GetPkg().BuildAction (nugetUsage.m_pkgSource.m_name, nugetUsage.m_pkgFramework, nugetUsage.m_pkgSource, thisPart, self.m_processedNugetPackages)

            for (_, pkgSource) in thisPart.m_universalPackages:
                if not pkgSource.HasPartInfo():
                    pkgSource.GetUpkg().BuildAction (thisPart)

            for (_, adoArtifactSource) in thisPart.m_adoBuildArtifacts:
                adoArtifactSource.BuildAction (thisPart)

            # For UPacks converted to parts we want to generate the symlinks so they can be used in bindings.
            if thisPart.IsPackage():
                thisPart.LinkPackage()

        if thisPart.m_info.m_repo.m_toolcache:
            self.UpdateToolCacheData (thisPart.m_info.m_repo.m_name)

        if not translationkit.IsTKBuildOnly ():
            self.LinkToolCache (thisPart)

            if thisPart.WantTranskit ():
                thisPart.CleanTranskits ()

        # Check if this has been processed. Duplicate ID cannot happen in a given graph so it doesn't need a thread lock.
        #  However when processing multiple strategies with a rebuild it avoids multiple builds. In the TMR
        #  situation the provenance file worked well enough though it may be faster not hitting the files.
        #  We just want to skip the actual bulding because we need the rest of the links and such to happen.
        alreadyProcessed = True
        if thisPart.m_info.GetNodeIdentifier() not in self.m_processedParts:
            alreadyProcessed = False
            self.m_processedParts.add (thisPart.m_info.GetNodeIdentifier())
        
        if not translationkit.IsTKBuildOnly () or thisPart.m_info.m_includeToolInTranskit:
            if not thisPart.FromLkgs():     # if we're coming from last-known-good, we can't build
                self.SymlinkVendorApiToBuildContext(thisPart)
                self.SymlinkPublicApiToBuildContext(thisPart)

                if not alreadyProcessed and not self.m_actionOptions.skipPartBuild:
                ##########################################
                #### Do the actual build 
                ##########################################
                    try:
                        thisPart.RunBuildCmd (self.m_actionOptions.bmakeOpt)
                    except Exception:
                        # In order to preserve a call stack, recast the exception
                        if self.PromptOnError():
                            thisPart.m_failed = True
                        raise

            else:
                if not thisPart.HasBuiltLKGs ():
                    utils.showInfoMsg ("Using LKG for {0}<{1}>\n".format (thisPart.m_info.m_file, thisPart.m_info.m_name), utils.INFO_LEVEL_Interesting)
                    partStrategy = thisPart.m_info.GetPartStrategy ()
                    utils.showInfoMsg ("   per {0}\n".format (partStrategy), utils.INFO_LEVEL_SomewhatInteresting)
                    thisPart.LinkProvenance ()
        if thisPart.WantTranskit():
            thisPart.BuildTranskits ()

        # Update feature aspects when appropriate
        # Remove these lines when we process FA's first.
        if not self.m_processFeatureAspectsFirst:  
            thisPart.ProcessFeatureAspects ()

        # Bind this part into all parent BuildContexts
        for (context, parentStatic) in thisPart.m_parentPartContexts:
            linkStaticToDynamic = False
            if thisPart.IsStatic() and not parentStatic:
                linkStaticToDynamic = True
            thisPart.BindToBuildContext (context, linkStaticToDynamic)

        if not translationkit.IsTKBuildOnly ():
            thisPart.BindTranskitTools ()

        # Also bind to all products
        for productPart in thisPart.m_parentProductContexts.values():
            if not productPart.IsNuGetProduct():
                self.ResetPartMethodsForClean (productPart)
                productPart.BindFromChildPart (thisPart, None, False, False)

        # Since RunBuildCmd is not run in transkit environment, we need to bind SubProducts here
        if thisPart.IsProduct():
            thisPart.BindSubProducts(thisPart, False)

        # During a clean, also remove the special directory that makefiles use for objects
        if self.m_actionOptions.clobberBuild:
            utils.cleanDirectory (thisPart.GetPartBuildDir(), deleteFiles=True)
        thisPart.m_finished = True
        thisPart.m_failed = False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def SymlinkVendorApiToBuildContext(self, thisPart):
        if thisPart.m_bindings:
            for api in thisPart.m_bindings.m_vendorAPI:
                if api.m_sourceDirectory:
                    link = os.path.join(thisPart.GetMyBuildContextDir(), 'VendorAPI', api.m_val)
                    if api.m_upackName:
                        _, upackSource = next((x for x in thisPart.m_universalPackages if x[0].lower() == api.m_upackName.lower()), (None, None))

                        if upackSource is None:
                            raise utils.BuildError('No upack with the name of {0} in part {1}'.format(api.m_upackName, thisPart))

                        target = os.path.join(upackSource.GetLocalPath(), api.m_sourceDirectory)
                    elif api.m_nugetName:
                        nugetUsage = next((x for x in thisPart.m_nugetPackages if x[0].lower() == api.m_nugetName.lower()), (None, None, None))

                        if nugetUsage.m_pkgSource is None:
                            raise utils.BuildError('No nuget with the name of {0} in part {1}'.format(api.m_nugetName, thisPart))

                        target = os.path.join(nugetUsage.m_pkgSource.GetPackagePath(), api.m_sourceDirectory)
                    else:
                        target = symlinks.normalizePathName(os.path.join(os.path.dirname(thisPart.m_info.m_file), api.m_sourceDirectory))

                    if os.path.exists(target):
                        thisPart.PerformBindToDir(link, target, checkSame=False)
                    else:
                        utils.showInfoMsg('Target {0} does not exist. Part: {1}'.format(target, thisPart), utils.INFO_LEVEL_SomewhatInteresting)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def SymlinkPublicApiToBuildContext(self, thisPart):
        if thisPart.m_bindings:
            for api in thisPart.m_bindings.m_publicAPI:
                if api.m_sourceDirectory:
                    link = os.path.join(thisPart.GetMyBuildContextDir(), 'PublicAPI', api.m_val)
                    if api.m_upackName:
                        _, upackSource = next((x for x in thisPart.m_universalPackages if x[0].lower() == api.m_upackName.lower()), (None, None))

                        if upackSource is None:
                            raise utils.BuildError('No upack with the name of {0} in part {1}'.format(api.m_upackName, thisPart))

                        target = os.path.join(upackSource.GetLocalPath(), api.m_sourceDirectory)
                    elif api.m_nugetName:
                        _, _, nugetSource = next((x for x in thisPart.m_nugetPackages if x[0].lower() == api.m_nugetName.lower()), (None, None, None))

                        if nugetSource is None:
                            raise utils.BuildError('No nuget with the name of {0} in part {1}'.format(api.m_nugetName, thisPart))

                        target = os.path.join(nugetSource.GetPackagePath(), api.m_sourceDirectory)
                    else:
                        target = target = symlinks.normalizePathName(os.path.join(os.path.dirname(thisPart.m_info.m_file), api.m_sourceDirectory))

                    if os.path.exists(target):
                        thisPart.PerformBindToDir(link, target, checkSame=False)
                    else:
                        utils.showInfoMsg('Neither {0} or {1} paths exist. Part: {2}'.format(api.m_sourceDirectory, target, thisPart), utils.INFO_LEVEL_SomewhatInteresting)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def VerifyBuildContext (self, root, linkErrors):
        """Make sure the resultant BuildContext contains only links."""
        totalErrors = 0
        fileList = os.listdir(root)
        for curFile in fileList:
            testname = os.path.join (root, curFile)
            if symlinks.isSymbolicLink(testname):
                continue
            if os.path.isdir(testname):
                totalErrors = totalErrors + self.VerifyBuildContext(testname, linkErrors)
            else:
                totalErrors = totalErrors + 1
                linkErrors.append("#### {0} is not a link\n".format (testname))

        return totalErrors

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def DoAfterAction (self):
        self.ShutdownWatchLog()

        linkErrors = []
        # Iterate over all the various buildcontext directories in use
        contextRoots = set()
        for currnode in self.m_bgraph.m_graph.AllNodes():
            part = self.m_bgraph.m_graph.PartNode(currnode).m_part
            if (part.GetPlatform(), part.IsStatic()) not in contextRoots:
                contextRoots.add ((part.GetPlatform(), part.IsStatic()))
                contextDir = os.path.join (buildpaths.getOutputRoot(part.GetPlatform(), isStatic=part.IsStatic()), buildpaths.DIRNAME_BUILDCONTEXTS)
                self.VerifyBuildContext(contextDir, linkErrors)

        if len (linkErrors) > 0:
            utils.showErrorMsg ("{0}(1) error : BuildContexts must only contain links:".format (globalvars.programOptions.basePartFile))
            for msg in linkErrors:
                utils.showErrorMsg ("\n " + msg)
            utils.exitOnError (1, "")

        # Process all NuGetPackage parts after the build so all of the bindings needed will be in place.
        # This is needed to work around NuGet's way of setting TargetFramework and in order to get dependent NuGet packages
        # within the right group all of the bindings need to happen first.
        self.BuildNugetPackages()

        if self.m_toolCacheUpdated and utils.isBsi():
            self.OutputToolCacheXML ()

    #--------------------------------------------------------------------------------
    # @bsimethod
    #--------------------------------------------------------------------------------
    def OnBuildError (self, errorLogFile):
        if self.m_actionOptions.linkErrorLog:
            if os.path.exists (self.m_actionOptions.linkErrorLog):
                symlinks.deleteSymLink (self.m_actionOptions.linkErrorLog)
            symlinks.createFileSymLink (self.m_actionOptions.linkErrorLog, errorLogFile) 

    #--------------------------------------------------------------------------------
    # @bsimethod
    #--------------------------------------------------------------------------------
    def PrepareToReprocessPart (self, thisPart):
        # Called if a part fails and PromptOnError or Retry is about to happen.
        self.m_processedParts.remove (thisPart.m_info.GetNodeIdentifier())

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def RemoveFileBinding (self, _thisPart, toName, _fromFile, _checkSame, _checkTargetExists, _skipIntermediateLinks):
        if os.path.exists (toName) or os.path.islink(toName):
            symlinks.deleteSymLink (toName)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def RemoveDirBinding (self, _thisPart, toName, _fromDir, _checkSame, _checkTargetExists, _skipIntermediateLinks):
        if os.path.exists (toName) or os.path.islink(toName):
            symlinks.deleteSymLink (toName)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def IgnoreFileBindingError (self, _sourceName, _child, _binding):
        pass

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ResetPartMethodsForClean (self, thisPart):
        if not self.m_actionOptions.clobberBuild:
            return
        thisPart.PerformBindToFileAction = self.RemoveFileBinding
        thisPart.PerformBindToDirAction  = self.RemoveDirBinding
        thisPart.ShowFileBindingError = self.IgnoreFileBindingError

    #--------------------------------------------------------------------------------
    # @bsimethod
    #--------------------------------------------------------------------------------
    def LinkToolCache (self, thisPart):
        # Link the needed tools from the toolcache
        if not thisPart.m_additionalRepos:
            return
            
        for repoName in thisPart.m_additionalRepos:
            toolRepo = globalvars.buildStrategy.FindLocalRepository(repoName)
            toolDir = toolRepo.GetLocalDir()
            if os.path.exists (toolDir) and toolRepo.m_toolcache:      # so only toolcache repositories are linked this way
                linkDir = os.path.join(buildpaths.GetToolsOutputRoot(), toolRepo.GetToolCacheName())
                symlinks.createDirSymLink (linkDir, toolDir)
                self.UpdateToolCacheData (repoName)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def SetupWatchLog (self):
        if not self.m_actionOptions.watchLog:
            return

        checker = cmdutil.PythonPrereqChecker ()
        checker.CheckPackage ('watchdog') # needed for watchout
        
        import watchout
        symlinks.makeSureDirectoryExists(self.m_actionOptions.watchLog)
        datestamp = time.strftime('%Y-%m-%d_%H-%M-%S', time.localtime())
        filename = 'watch_{0}.log'.format(datestamp)
        pathname = os.path.join(self.m_actionOptions.watchLog, filename)
        self.m_watchLog = open(pathname, "wt")
        if (self.m_watchLog):
            header = ','.join (['BuildContext', 'PartName', 'PartType', 'Architecture', 'Dynamic', 'Timestamp', 'EventType', 'SourcePath', 'DestinationPath'])  # during initialization
            self.m_watchLog.write(header + '\n')
        outRoot = os.environ.get("OutRoot") if 'OutRoot' in os.environ else '.'
        self.m_watchPart = ','.join ([sys._getframe(1).f_code.co_name, 'NoPart', 'NoType', 'NoArch', 'NoStatic'])  # during initialization pylint: disable=protected-access
        self.m_watchOut = watchout.WatchOut(None, self.GetWatchPart)
        symlinks.makeSureDirectoryExists(outRoot)
        self.m_watchOut.start(outRoot)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def HandleWatchLog (self, thisPart):
        partId = ','.join ([ thisPart.m_info.m_buildContext, thisPart.m_info.m_name, buildpart.PartTypeNames[thisPart.m_info.m_partType], thisPart.GetPlatform().GetXmlName(), thisPart.m_info.GetStaticString(short=False) ])
        if self.m_actionOptions.watchLog:
            self.m_watchPart = partId
        if self.m_actionOptions.procMon:
            try:
                import _winreg as reg
                key = reg.OpenKey (reg.HKEY_CURRENT_USER, 'Software\\Bentley', 0, reg.KEY_READ)
                try: # just read partId which won't exist, but will add entry into ProcMon log
                    reg.QueryValueEx (key, 'BentleyBuild,' + partId)
                except:
                    # of course it isn't there: ignore it
                    pass
                reg.CloseKey(key)
            except:
                # can't access registry at all?  also ignore it?
                pass

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ShutdownWatchLog (self):
        if not self.m_actionOptions.watchLog:
            return

        import watchdog.events # necessary for identifying event types
        self.m_watchPart = ','.join ([sys._getframe(1).f_code.co_name, 'NoType', 'NoArch', 'NoStatic'])  # after done pylint: disable=protected-access
        self.m_watchOut.stop()
        for key in self.m_watchOut.dict():
            keyList = self.m_watchOut.dict()[key]
            for (eventTime, event) in keyList:
                if compat.py3:
                    msecs =  int((eventTime -  int(eventTime)) * 1000)
                else:
                    msecs = long((eventTime - long(eventTime)) * 1000) # pylint: disable=undefined-variable
                datestamp = time.strftime('%Y-%m-%dT%H:%M:%S', time.localtime(eventTime))
                message = '{0},{1}.{2:03d},{3},"{4}"'.format(key, datestamp, msecs, event.event_type, event.src_path)
                if watchdog.events.EVENT_TYPE_MOVED == event.event_type: # only event type with dest_path too
                    message += ',"{0}"'.format(event.dest_path)
                message += '\n'
                utils.showInfoMsg (message, utils.INFO_LEVEL_SomewhatInteresting)
                if (self.m_watchLog):
                    self.m_watchLog.write(message)
        if (self.m_watchLog):
            self.m_watchLog.close()
            self.m_watchLog = None

    #--------------------------------------------------------------------------------
    # @bsimethod
    #--------------------------------------------------------------------------------
    def SetupTabCompletion(self):
        checker = cmdutil.PythonPrereqChecker ()
        checker.CheckPackage ('pyreadline') # needed for readline

        import fnmatch
        import readline
        # Set the tab history file to the local repo
        TAB_HISTORY = os.path.join(buildpaths.GetBBCacheDir(), "history")
        completeList = GetTabCompletionList(self.m_bgraph)
        def auto_completer(text, state):
            for cmd in completeList:
                if cmd.lower().startswith(text.lower()) or fnmatch.fnmatch(cmd.lower(), text.lower()):
                    if not state:
                        return cmd
                    else:
                        state -= 1

        readline.read_history_file(TAB_HISTORY)
        readline.parse_and_bind("tab: complete")
        readline.set_completer(auto_completer)
        try:
            completedParts = compat.getInput('\nControl-D to exit\npart(s): ')
        except EOFError: #e.g. Control-D for escape
            completedParts = ""
        for c_part in completedParts.split(' '):
            if len(c_part) > 0 and c_part in completeList:
                if ':' in c_part:
                    self.m_partFilters.append (PartFilter (partFileFilter=c_part[:c_part.index(':')], partNameFilter=c_part[c_part.index(':')+1:], makeFileFilter=None, matchAllSubparts=self.m_actionOptions.buildSubparts, partPlatforms=None, libType=None))
                else:
                    self.m_partFilters.append (PartFilter (partFileFilter=None, partNameFilter=c_part, makeFileFilter=None, matchAllSubparts=self.m_actionOptions.buildSubparts, partPlatforms=None, libType=None))
        readline.add_history(completedParts)
        readline.set_history_length(40)
        readline.write_history_file(TAB_HISTORY)
        readline.set_completer(None)

    #--------------------------------------------------------------------------------
    # @bsimethod
    #--------------------------------------------------------------------------------
    def DoBentleyBuildShutdown (self, buildShutdownContext):
        self.SendEmailNotifications (buildShutdownContext)
        
    #--------------------------------------------------------------------------------
    # @bsimethod
    #--------------------------------------------------------------------------------
    def SendEmailNotifications (self, shutdownContext):
        stratName = globalvars.buildStrategy.m_strategyName if globalvars.buildStrategy else '(No strategy read)'
        dt = shutdownContext.m_endTime - shutdownContext.m_startTime
        timeMsg = time.strftime("%H:%M:%S", time.gmtime(dt))
        curBranch = 'current branch' # TODO - change to main branch of main git repo
        msg = "{0} - {1} - {2} - time {3}\n".format (curBranch, stratName, globalvars.programOptions.outRootDirectory, timeMsg)
        
        if None != os.environ.get("BB_EMAIL_VERBOSE"):
            msg += shutdownContext.m_errorMessage if shutdownContext.m_errorMessage != "" else "No Errors Detected"

        if None != self.m_actionOptions.emailAddress:
            addr = self.m_actionOptions.emailAddress
        elif None != os.environ.get("BB_EMAIL_ADDRESS"):
            addr = os.environ.get("BB_EMAIL_ADDRESS")
        else:
            import getpass
            addr = getpass.getuser() + internal.EMAIL_RECIPIENT_HOST

        if (self.m_actionOptions.emailNotification):
            import smtplib
            from email.mime.text import MIMEText

            mailmsg = MIMEText(msg)            
            mailmsg['Subject'] = 'Build Succeeded' if 0 == shutdownContext.m_statusCode else 'Build Failed'
            mailmsg['From'] = internal.EMAIL_SENDER
            mailmsg['To'] = addr

            s = smtplib.SMTP(internal.SMTP_SERVER)
            s.sendmail (mailmsg['From'], [mailmsg['To']], mailmsg.as_string())
            s.quit()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def UpdateToolCacheData(self, toolName):
        #update the list of repos used by this build so later we can look for stale repositories
        if not self.m_toolCacheTree:
            self.m_toolCacheTree = self.GetToolCacheTree()
        toolName = toolName.lower()

        srcRoot = buildpaths.GetSrcRoot()
        updatedExisting = False
        currentTime = str(datetime.datetime.now().strftime("%Y-%m-%dT%H:%M:%S")) 

        root = self.m_toolCacheTree.getroot()
        for tool in root:
            if tool.attrib['Name'] == toolName:
                for usage in tool:
                    if usage.attrib['SrcRoot'] == srcRoot:
                        usage.attrib['LastUsed'] = currentTime
                        updatedExisting = True
                if not updatedExisting:
                    ElementTree.SubElement(tool, "Usage", SrcRoot=srcRoot, LastUsed=currentTime)
                    updatedExisting = True
        if not updatedExisting:
            branch = ElementTree.SubElement(root, "Tool", Name=toolName)
            ElementTree.SubElement(branch, "Usage", SrcRoot=srcRoot, LastUsed=currentTime)

        self.m_toolCacheUpdated = True

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetToolCacheTree(self):
        if not self.m_toolCacheTree:
            toolListXml = os.path.join(buildpaths.GetToolCacheSourceRoot(), "ToolList.xml")
            if os.path.exists(toolListXml):
                self.m_toolCacheTree = ElementTree.parse(toolListXml)
            else:
                self.m_toolCacheTree = ElementTree.ElementTree(ElementTree.Element("ToolCache"))
        return self.m_toolCacheTree

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def OutputToolCacheXML(self):
        #helper function to "pretty print" the raw xml
        def indent(elem, level=0):
            i = "\n" + level*"  "
            if len(elem):
                if not elem.text or not elem.text.strip():
                    elem.text = i + "  "
                if not elem.tail or not elem.tail.strip():
                    elem.tail = i
                for elem in elem:
                    indent(elem, level+1)
                if not elem.tail or not elem.tail.strip():
                    elem.tail = i
            else:
                if level and (not elem.tail or not elem.tail.strip()):
                    elem.tail = i

        indent(self.m_toolCacheTree.getroot())

        xmlFile = os.path.join(buildpaths.GetToolCacheSourceRoot(), "ToolList.xml")
        self.m_toolCacheTree.write (xmlFile, encoding='utf-8')

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetWatchPart (self):
        # return the currently watched part
        if self.m_actionOptions.watchLog:
            return self.m_watchPart
        return None

#-------------------------------------------------------------------------------------------
# This is the similar to BuildPartAction but it has multiple Stages.
# bsiclass
#-------------------------------------------------------------------------------------------
class BuildPartActionMultiStage (BuildPartAction):

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__(self, inArgs):
        BuildPartAction.__init__ (self, inArgs)
        self.m_allTypes = {}
        self.m_hasFilter = False
        self.m_currentFilter = []
        self.m_currentDefaultContinue = True
        self.m_passNumber = 0
        self.m_deferString = 'Deferring'

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def PerformAction (self, partInfo):
        self.m_bgraph = buildgraph.BuildGraph.GetBuildGraph (partInfo, self)
        self.m_hasFilter = False  # Reset before toolset runs in the case of a multi-platform build.
        self.DoBeforeAction ()
        
        pBuildMgr = parallelbuild.ParallelBuildManager (self.m_bgraph.m_graph, self.m_bgraph.m_rootNode, self)

        # filter anything deferred. Used by FilterParts during build.
        self.InitAllPartDeferTypes()
        allPasses = list (set ([v for (k,v) in self.m_allTypes.items() if v != strategy.DEFER_OPTION_Normal and v != strategy.DEFER_OPTION_Never])) # Make it unique by using set, but need it to be sortable.
        self.m_currentFilter = [k for (k,v) in self.m_allTypes.items() if v != strategy.DEFER_OPTION_Normal]
        self.m_currentDefaultContinue = True
        self.m_passNumber = 1
        
        # First pass
        status = self.BuildWithRetries(pBuildMgr)

        if status:
            return status

        # Deferred stuff
        allPasses = sorted (allPasses)
        for curPass in allPasses:
            utils.showInfoMsg ("\n\nBuilding deferred parts BuildPass {0}, iteration {1}\n\n".format (curPass, self.m_passNumber), utils.INFO_LEVEL_Essential)

            self.m_currentFilter = [k for (k,v) in self.m_allTypes.items() if v != curPass]
            self.m_currentDefaultContinue = False # Remove everything built in the first pass
            self.m_passNumber += 1
            self.m_deferString = 'Skipping'

            status = self.BuildWithRetries(pBuildMgr)
            if status:
                return status

        self.DoAfterAction ()
        return status

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def InitAllPartDeferTypes (self):
        # Filter anything that lists as deferred or skipped
        allTypes = set()
        for currnode in self.m_bgraph.m_graph.AllNodes():
            currPart = self.m_bgraph.m_graph.PartNode(currnode).m_part
            if currPart.m_deferType:
                allTypes.add (currPart.m_deferType.lower())
                
        for item in allTypes:
            defer = globalvars.buildStrategy.GetDeferStrategy (item)
            self.m_allTypes [item] = defer
            if defer != strategy.DEFER_OPTION_Normal:
                self.m_hasFilter = True
            
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def FilterParts (self):                                                 # pylint: disable=method-hidden  

        # If no filter then do standard check for No Subparts
        if globalvars.programOptions.noSubParts:
            return buildaction.BentleyBuildAction.FilterParts (self)
        
        if not self.m_hasFilter:
            return False
                    
        def Defer (currPart, currnode):
            self.m_bgraph.m_graph.PartNode(currnode).m_processed = 1
            utils.showInfoMsg ("Pass {0}: {1} {2} {3} based on DeferType {4}\n".format
                (self.m_passNumber, self.m_deferString, currPart.m_info.GetShortPartDescriptor(), currPart.PartType(), currPart.m_deferType), 
                utils.INFO_LEVEL_Interesting)
    
        self.m_bgraph.m_graph.ClearProcessedBit ()
        numFiltered = 0
        for currnode in self.m_bgraph.m_graph.AllNodes():
            currPart = self.m_bgraph.m_graph.PartNode(currnode).m_part
            if currPart.m_deferType == None:
                if not self.m_currentDefaultContinue:
                    Defer (currPart, currnode)
                    numFiltered += 1
            elif currPart.m_deferType.lower() in self.m_currentFilter:
                Defer (currPart, currnode)
                numFiltered += 1

        return numFiltered > 0
    
#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class RebuildPartAction (BuildPartAction):
    def ActionName(self):  return "Rebuild"

    @staticmethod
    def GetHelp ():
        return "Force rebuild of a list of parts"
    @staticmethod
    def GetUsage():      return "%(prog)s rebuild [args] [partfile1:][partname1] [partfile2:][partname2] ... "

    #--------------------------------------------------------------------------------
    # bsimethod
    #--------------------------------------------------------------------------------
    @staticmethod
    def GetParser():
        return BuildPartAction.GetParserWithAction (True, RebuildPartAction.GetHelp())

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def FilterParts (self):                                             # pylint: disable=method-hidden 
        # If rebuilding, mark everyone that does not match the filter as completed
        # Have to do depth-first to support "build subparts"
        anythingFiltered = set()
        anythingFound = set()

        for node in self.m_bgraph.m_graph.BreadthFirstWalk (self.m_bgraph.m_rootNode):
            partNode = self.m_bgraph.m_graph.PartNode(node)
            buildThisNode=False
            for partFilter in self.m_partFilters:
                if partFilter.DoesMatch (partNode.m_part, self.m_bgraph.m_graph):
                    # Remove things as they are found so we can check if everything was hit
                    if partFilter.m_partNameFilter in self.m_rebuildPartList:
                        self.m_rebuildPartList.remove (partFilter.m_partNameFilter)
                    buildThisNode = True
                    break
                    
            if not buildThisNode:
                anythingFiltered.add (node)
            else:
                anythingFound.add (node)

        if len(anythingFound) <= 0:
            utils.exitOnError (1, "Nothing found to rebuild -- no parts match the given arguments")
        elif len(self.m_rebuildPartList) > 0:
            utils.exitOnError (1, "Did not find any parts matching these rebuild arguments: [" + ', '.join(self.m_rebuildPartList) + ']')

        # buildDependents -- for every part that was found, remove its parents from the filter
        if (self.m_actionOptions.buildDependents):
            parentsFound = set()
            for node in anythingFound:
                if node not in parentsFound:    # Don't bother with any nodes included in an earlier ParentWalk - we won't find anything new
                    parentsFound.update(self.m_bgraph.m_graph.ParentWalk(node))

            anythingFiltered -= parentsFound
            anythingFound |= parentsFound

        # Now clear the processed bit again and mark things that we don't need to build.
        self.m_bgraph.m_graph.ClearProcessedBit ()
        for currnode in anythingFiltered:
            self.m_bgraph.m_graph.PartNode(currnode).m_processed = 1

        # removeBuildLogs -- for every part that was found, delete it's build log
        if (self.m_actionOptions.removeBuildLogs):
            utils.showStatusMsg ("Cleaning old build logs", utils.INFO_LEVEL_VeryInteresting)
            for node in anythingFound:
                part = self.m_bgraph.m_graph.PartNode(node).m_part
                logFileName = part.GetLogFileName()
                utils.showInfoMsg ("Removing {0} from part '{1}'\n".format(logFileName, part.GetShortRepresentation()), utils.INFO_LEVEL_SomewhatInteresting)
                if os.path.isfile (logFileName):
                    os.remove (logFileName)

        return len(anythingFiltered) > 0
                
#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class CleanPartAction (RebuildPartAction):
    def ActionName(self):  return "Clean"

    @staticmethod
    def GetHelp ():
        return "Perform 'bmake +Da' for a list of parts (this is just an alias for 'rebuild -c')"

    @staticmethod
    def GetUsage():      return "%(prog)s clean [partfile1:][partname1] [partfile2:][partname2] ... "

    @staticmethod
    def GetParser():
        return BuildPartAction.GetParserWithAction (True, CleanPartAction.GetHelp())

    def __init__(self, inArgs):
        newArgs = ['-c'] + inArgs
        RebuildPartAction.__init__ (self, newArgs)

