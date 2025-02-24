#!Python
#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import argparse, os, time, sys
from . import compat
from . import buildpart, buildpaths, globalvars, strategy, symlinks, targetplatform, translationkit, utils
from . import buildpartaction

# For the open source of imodel we include a chunk of bb but we skip most of the actions other than build.
FULL_BB = os.path.exists (os.path.join (os.path.dirname(__file__), 'getpartsourceaction.py'))

if FULL_BB:
    from . import pushpartsourceaction, getpartsourceaction, savelkgaction, debugaction, ideaction, hgaction, gitaction
    from . import repositorystatusaction, repositorysyncpullaction, commitaction, incomingaction, outgoingaction
    from . import taglistaction, troubleshootaction, diffreportaction, savenugetaction
    from . import saveproductaction, savetranskitaction, queryaction, prodversionaction, configaction, toolsaction
    from . import buildinstallsetaction, bundlebuildaction, mergelocalizedaction, pseudolocalizeaction

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class BentleyBuildParser(utils.ArgumentParser) :

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__ (self, optParseUsage, actions):
        utils.ArgumentParser.__init__ (self, usage=optParseUsage)
        self.m_actions = actions

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class Action (object):

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__ (self, aliases, actionClass):
        self.m_aliases = aliases
        self.m_aliasString = ", ".join (aliases)
        self.m_actionClass = actionClass

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def _Compare (self, other):
        if self.m_aliases < other.m_aliases:
            return -1
        if self.m_aliases > other.m_aliases:
            return 1
        return 0

    def __lt__(self, other): return self._Compare (other) < 0
    def __eq__(self, other): return self._Compare (other) == 0
    def __ne__(self, other): return self._Compare (other) != 0
    def __gt__(self, other): return self._Compare (other) > 0
    def __le__(self, other): return self._Compare (other) <= 0
    def __ge__(self, other): return self._Compare (other) >= 0

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    @staticmethod
    def InstantiateFromList (actionList, command, commandArgs):
        for action in actionList:
            for alias in action.m_aliases:
                if alias.startswith (command):
                    newAction =  action.m_actionClass (commandArgs)
                    return newAction.GetSubAction() if newAction.GetSubAction() else newAction

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    @staticmethod
    def GetActionHelp (actionList):
        maxLeftColumnWidth = 0

        for action in actionList:
            if action.m_aliasString.__len__() > maxLeftColumnWidth:
                maxLeftColumnWidth = action.m_aliasString.__len__()

        lines = []
        lines.append ("Actions:\n\n")
        for action in sorted(actionList):
            lines.append (" " + action.m_aliasString + " " * (4 + maxLeftColumnWidth - action.m_aliasString.__len__()) + action.m_actionClass.GetHelp() + '\n')
        lines.append ('\n')
        return ''.join(lines)

#--------------------------------------------------------------------------------
# This was created to pass down top level build statistics etc to the action
# after the build completes. Some actions can use this specifically or do nothing with it.
# It should be invoked from out here because some things might fail due to exceptions.
# @bsimethod
#--------------------------------------------------------------------------------
class BentleyBuildShutdownContext (object):
    def __init__(self, startTime, endTime, statusCode, errorMessage=""):
        self.m_startTime = startTime
        self.m_endTime = endTime
        self.m_statusCode = statusCode
        self.m_errorMessage = errorMessage
#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetBbParser ():
    # The order of these actions defines how BentleyBuild responds to abbreviations
    # E.g. p, pu refer to pull, not push, because pull comes first
    actions = [
        Action( ('build'              ,           ),   buildpartaction.BuildPartActionMultiStage           ),
        Action( ('rebuild'            ,           ),   buildpartaction.RebuildPartAction                    ),
    ]

    if FULL_BB:
        actions.extend ([
            Action( ('pull'               , 'get'     ),   getpartsourceaction.GetPartSourceAction              ),
            Action( ('push'               ,           ),   pushpartsourceaction.PushPartSourceAction            ),
            Action( ('debug'              ,           ),   debugaction.DebugAction                              ),
            Action( ('ide'                ,           ),   ideaction.IdeAction                                  ),
            Action( ('hg'                 ,           ),   hgaction.HgAction                                    ),
            Action( ('git'                ,           ),   gitaction.GitAction                                  ),
            Action( ('status'             ,           ),   repositorystatusaction.RepositoryStatusAction        ),
            Action( ('syncpull'           ,           ),   repositorysyncpullaction.RepositorySyncPullAction    ),
            Action( ('clean'              ,           ),   buildpartaction.CleanPartAction                      ),
            Action( ('commit'             , 'ci'      ),   commitaction.CommitAction                            ),
            Action( ('savelkgs'           ,           ),   savelkgaction.SaveLKGAction                          ),
            Action( ('incoming'           ,           ),   incomingaction.IncomingAction                        ),
            Action( ('mergelocalized'     ,           ),   mergelocalizedaction.MergeLocalizedAction            ),
            Action( ('pseudolocalize'     ,           ),   pseudolocalizeaction.PseudoLocalizeAction            ),
            Action( ('outgoing'           ,           ),   outgoingaction.OutgoingAction                        ),
            Action( ('taglist'            ,           ),   taglistaction.TagListAction                          ),
            Action( ('troubleshoot'       ,           ),   troubleshootaction.TroubleshootAction                ),
            Action( ('diffreport'         ,           ),   diffreportaction.DiffReportAction                    ),
            Action( ('saveproduct'        ,           ),   saveproductaction.SaveProductAction                  ),
            Action( ('savetranskit'       ,           ),   savetranskitaction.SaveTranskitAction                ),
            Action( ('savenuget'          ,           ),   savenugetaction.SaveNugetAction                      ),
            Action( ('query'              ,           ),   queryaction.QueryAction                              ),
            Action( ('buildinstallset'    ,           ),   buildinstallsetaction.BuildInstallSetAction          ),
            Action( ('bundlebuild'        ,           ),   bundlebuildaction.BundleBuildAction                  ),
            Action( ('prodversion'        ,           ),   prodversionaction.ProdVersionAction                  ),
            Action( ('config'             ,           ),   configaction.ConfigAction                            ),
            Action( ('tools'              ,           ),   toolsaction.ToolsAction                              ),
            ])

    parser = BentleyBuildParser ("usage: %(prog)s [options] Action [actionargs]\n'%(prog)s Action -h' for help on a specific action\n\n{0}".format(Action.GetActionHelp (actions)), actions)
    parser.add_argument ("-r", "--repository",    action="store",         dest="repository",                      help="Repository of starting PartFile")
    parser.add_argument ("-f", "--partFile",      action="store",         dest="partFile",                        help="The part file")
    parser.add_argument ("-p", "--partName",      action="store",         dest="partName",                        help="The part name, or file and name in PartFile:PartName syntax")
    parser.add_argument (      "--srcroot",       action="store",         dest="srcRootDirectory",                help="Source tree root directory; normally set with SrcRoot in the environment")
    parser.add_argument ("-o", "--outputroot",    action="store",         dest="outRootDirectory",                help="Output tree root directory; normally set with OutRoot in the environment")
    parser.add_argument ("-v", "--verbosity",     action="store",         dest="verbosity",         type=int,     help="Verbosity of messages. 0=none, 6=all. Default=3")
    parser.add_argument ("-s", "--strategy",      action="store",         dest="buildStrategy",                   help="Override 'BuildStrategy' environment variable.  Strategies can be concatinated with ; and iterated with + i.e. ecf;buildall+dgnplatform;buildall.")
 
    parser.add_argument ("-n", "--noSubParts",    action="store_true",    dest="noSubParts",                      help="Only process the specified part, not its SubParts")
    parser.add_argument ("-a", "--architecture",  action="store",         dest="platformName",                    help="Define target platform to use.  This overrides DefaultTarget Platform in the Strategy file. Platforms can be iterated with +, i.e. x86+x64. {0}".format (str(targetplatform.GetCommandLineOptions())))
    parser.add_argument ("-l", "--languages",     action="store",         dest="languages",                       help="Define Comma-Separated target languages to use. {0}".format (str(translationkit.getCommandLineOptions ())))
    parser.add_argument ("-N", "--numThreads",    action="store",         dest="numThreads",                      help="The number of threads to use for this operation")
    parser.add_argument (      "--clearCache",    action="store_true",    dest="clearCache",                      help="Clear and regenerate the part cache")
    parser.add_argument (      "--priority",      action="store",         dest="processPriority",   type=int,     help="Set the CPU priority for this operation. 0=IdleCycles, 2=Normal, 4=High (default is 1)")
    parser.add_argument (      "--host",          action="store",         dest="asHost",                          help="Work as if the host is a different platform. Useful for syncpull, but it will break things like build. {0}".format(globalvars.HOST_PLATFORMS))
    parser.add_argument (      "--ignoreXmlErrors", action="store_true",  dest="ignoreXmlErrors",                 help="Ignore XML Validation errors. This is usually a bad idea.")
    parser.add_argument ('Action') 
    parser.add_argument ('actionargs', nargs=argparse.REMAINDER, help="Arguments for the Action")  # Collects up the rest of the arguments to pass to command
	
    parser.set_defaults (
        repository          = None  ,
        partFile            = None  ,
        partName            = None  ,
        srcRootDirectory    = None  ,
        outRootDirectory    = None  ,
        verbosity           = 3     ,
        buildStrategy       = None  ,
        noSubParts          = False ,
        processPriority     = 1     ,  # 2 is what the system picks when you specify nothing. Right now we don't allow for priority inheritance from the shell.
        ignoreErrors        = True,    # This is set in other spots in the program, even though it doesn't have a global option here
        useLocalizedDir     = False,
        clearCache          = False,
        languages           = None,
        numThreads          = -1,
        asHost              = None,
        ignoreXmlErrors     = False,
        platformName        = None,
        platform            = None     # This is not set by the parser but later it is set from options.platformName.
    )

    return parser

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def ParseStrategies(buildAction, buildStrategy, parser):
    additionalImports = []

    if translationkit.IsLanguageBuild() and not globalvars.programOptions.useLocalizedDir:
        additionalImports.append(translationkit.L10N_REPOSITORY_LISTS_BUILDSTRATEGY)

    # Have to do strategies after teams to get from correct place.
    if strategy.ensureStrategyDir ():
        return 1

    globalvars.programOptions.buildStrategy = buildStrategy

    try:
        globalvars.buildStrategy = buildAction.LoadBuildStrategy (globalvars.programOptions.buildStrategy, additionalImports, False)
        if None != globalvars.programOptions.repository:
            globalvars.buildStrategy.m_defaultRepo = globalvars.programOptions.repository

        if None == globalvars.buildStrategy.m_defaultRepo:
            parser.error ("No starting repository specified")

        if not globalvars.buildStrategy.m_buildControl.m_isBundleBuild and "BundleStrategy_BuildBundle" in os.environ:
            globalvars.buildStrategy.m_buildControl.m_isBundleBuild = True

    except utils.StrategyError as err:
        utils.showErrorMsg (err.errmessage)
        return 1
    except utils.FatalError as err:
        utils.showErrorMsg (err.errmessage)
        utils.showErrorCallStack (err)
        return 1

    return 0

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetPartfile(parser, repo):
    # Choose the partfile from the Command Line or Strategy element
    globalvars.programOptions.basePartFile = globalvars.programOptions.partFile
    if None == globalvars.programOptions.basePartFile:
        globalvars.programOptions.basePartFile = globalvars.buildStrategy.m_defaultPartFile

    if None == globalvars.programOptions.basePartFile:
        parser.error ("No PartFile specified")

    # Fully qualify the partfile.
    globalvars.programOptions.basePartFile = repo.GetPartFile(globalvars.programOptions.basePartFile)

    # Choose the partname from the Command Line or Strategy element
    globalvars.programOptions.basePartName = globalvars.programOptions.partName
    if None == globalvars.programOptions.basePartName:
        globalvars.programOptions.basePartName = globalvars.buildStrategy.m_defaultPartName

    if None == globalvars.programOptions.basePartName:
        parser.error ("No PartName specified")

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def RunAction (buildAction, repo, platformObj):
    exitstat = 0
    errMessage = ""

    # Set up the number of threads in the action based on Command Line or Strategy (here because of the latter).
    if globalvars.programOptions.numThreads != -1:
        buildAction.m_numThreads = int(globalvars.programOptions.numThreads)
    elif globalvars.buildStrategy.m_numThreads != -1:
        buildAction.m_numThreads = int(globalvars.buildStrategy.m_numThreads)

    partName = globalvars.programOptions.basePartName
    utils.showInfoMsg (
        "Starting at Part: {0}\n\n".format (buildpaths.getShortPartDescriptor (globalvars.programOptions.basePartFile, partName)),
        utils.INFO_LEVEL_Important,
        utils.LIGHT_BLUE
    )

    globalvars.currentAction = buildAction
    buildAction.DoActionSetup ()
    isStatic = globalvars.buildStrategy.GetLibTypeForPlatform (platformObj)
    exitstat = buildAction.PerformAction (buildpart.PartInfo (partName, repo, globalvars.programOptions.basePartFile, buildpart.PART_TYPE_Any, globalvars.programOptions.platform, isStatic, None))

    return exitstat, errMessage

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def BuildAllPlatforms (buildAction, repo):

    # This has to be done here because it may use the default from the current strategy.
    platformObjList = targetplatform.ResolvePlatform(globalvars.programOptions.platformName)

    # Store it on the action for clean.
    buildAction.m_allPlatforms = platformObjList
    
    # Run all the platforms
    for platformObj in platformObjList:
        if globalvars.buildStrategy.m_onlyPlatforms and not platformObj in globalvars.buildStrategy.m_onlyPlatforms:
            utils.showInfoMsg ("Skipping excluded platform {0} for strategy {1}\n".format(platformObj.GetXmlName(), globalvars.buildStrategy.m_strategyName), utils.INFO_LEVEL_Important)
            exitstat = 0
            errMessage = None
            continue

        globalvars.programOptions.platform = platformObj
        exitstat, errMessage = RunAction (buildAction, repo, platformObj)
        if exitstat:
            break

    buildAction.AllPlatformsComplete (exitstat)

    return exitstat, errMessage

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def BuildCurrentStrategy (buildAction, buildStrategy, parser, allBuildStrategies, allResolvedBuildStrategies):
    # Read all the strategy files
    status = ParseStrategies(buildAction, buildStrategy, parser)
    if status:
        return status, None

    # Get the primary repository for this strategy
    try:
        repo = globalvars.buildStrategy.FindLocalRepository (globalvars.buildStrategy.m_defaultRepo)
    except utils.StrategyError as err:
        utils.showErrorMsg (err.errmessage)
        return 1, None

    # Set up the default output dir
    if None == globalvars.programOptions.outRootDirectory:
        if globalvars.buildStrategy.m_defaultOutputRootDir: # I have not seen this, but many commands do not require an output dir so allow it to be blank.
            globalvars.programOptions.outRootDirectory = symlinks.normalizePathName (globalvars.buildStrategy.m_defaultOutputRootDir)
    else:
        globalvars.programOptions.outRootDirectory = symlinks.normalizePathName (globalvars.programOptions.outRootDirectory)
        # A lot of VCProj's use OutRoot instead of OutputRootDir (and then they try to ascertain the platform).
        os.environ['OutRoot'] = compat.getStringForEnv(globalvars.programOptions.outRootDirectory) + os.sep
        globalvars.buildStrategy.UpdateCachedEnvVariables()

    # Get the primary part file for this strategy
    GetPartfile(parser, repo)

    # Standard output to say what we are doing
    utils.showInfoMsg ("[{0}] {1}\n".format(buildAction.ActionName(), utils.getNow()), utils.INFO_LEVEL_VeryInteresting)
    utils.showInfoMsg ("Using BuildStrategy: {0}\n".format(buildStrategy), utils.INFO_LEVEL_Important, utils.LIGHT_BLUE)
    if allResolvedBuildStrategies != allBuildStrategies:
        utils.showInfoMsg ("Full BuildStrategy: {0} (resolved: {1})\n".format(allBuildStrategies, allResolvedBuildStrategies), utils.INFO_LEVEL_Important, utils.LIGHT_BLUE)

    return BuildAllPlatforms (buildAction, repo)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def BuildAllStrategies (buildAction, allBuildStrategies, parser):
    try:
        errMessage = ''
        exitstat = 0
        buildStrategyList = strategy.BuildStrategy.ExpandStrategyAliases(allBuildStrategies)
        allResolvedBuildStrategies = '+'.join(buildStrategyList) if len(buildStrategyList) > 1 else buildStrategyList[0] # For info messages
        # Parse the strategy and run the command
        for currStrategy in buildStrategyList:
            exitstat, errMessage = BuildCurrentStrategy (buildAction, currStrategy, parser, allBuildStrategies, allResolvedBuildStrategies)
            if exitstat:
                break
        buildAction.AllStrategiesComplete (exitstat)

    except utils.StrategyError as err:
        utils.showErrorMsg (err.errmessage)
        utils.showErrorCallStack (err)
        exitstat=1
        errMessage = err.errmessage
    except utils.BuildError as err:
        utils.showErrorMsg (err.errmessage)
        utils.showErrorCallStack (err)
        exitstat=1
        errMessage = err.errmessage
    except symlinks.SymLinkError as err:
        utils.showErrorMsg (err.errmessage)
        utils.showErrorCallStack (err)
        exitstat=1
        errMessage = 'Symlink:' + err.errmessage
    except KeyboardInterrupt as err:
        errMessage = "***Aborted by user\n"
        utils.showErrorCallStack (err)
        sys.stderr.write (errMessage)
        exitstat=1
    except utils.FatalError as err:
        errMessage = err.errmessage
        utils.showErrorMsg (errMessage)
        utils.showErrorCallStack (err)
        exitstat=1
    finally:
        buildAction.ShowRequiredOutput()

        # Show an error summary if we were ignoring errors
        if buildAction.ShouldIgnoreErrors() and len(globalvars.buildErrors) > 0:
            utils.showInfoMsg("\n\n" + "_"*10 +" Errors in {0} ".format(buildAction.ActionName()) + "_"*10+"\n\n", utils.INFO_LEVEL_Essential, utils.RED)
            num = 0
            for num, msg in enumerate(globalvars.buildErrors):
                utils.showInfoMsg("Error {0}:\n   {1}\n".format(num+1,msg), utils.INFO_LEVEL_Essential, utils.RED)

            if len (globalvars.deferredMessages) > 0:
                num += 2
                for item in globalvars.deferredMessages:
                    utils.showInfoMsg("Error {0}:\n\n   {1}\n\n".format(num,item[0]), item[1])
                globalvars.deferredMessages = []
            exitstat=1
            
        # Display any messages that are left until the end.
        utils.ShowDeferredMessages ()

        # On an error, dump the failing message again at the very end
        if exitstat:
            utils.showErrorMsg ('\n\n========================================\n')
            utils.showErrorMsg ('  {0} Failed\n'.format (buildAction.ActionName()))
            utils.showErrorMsg ('========================================\n')
            if errMessage:
                utils.showErrorMsg (errMessage)
        # Show a success message for long-running high-output operations like Get and Build.
        elif buildAction.ShouldShowSuccessMessage():
            utils.showInfoMsg ('\n\n========================================\n', utils.INFO_LEVEL_Essential)
            utils.showInfoMsg ('  {0} Succeeded\n'.format (buildAction.ActionName()), utils.INFO_LEVEL_Essential)
            utils.showInfoMsg ('========================================\n', utils.INFO_LEVEL_Essential)

    return exitstat, errMessage

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def RunBentleyBuild (messages, inputArgs=None):

    parser = GetBbParser ()
        
    programOptions = parser.parse_args(args = inputArgs)

    # Need to set these up early before output or XML files are read. Both based solely on command line.
    utils.g_verbosity = programOptions.verbosity
    
    # Display messages as soon as possible now that verbosity is set
    for msg, infoLevel in messages:
        utils.showInfoMsg (msg, infoLevel)

    # Set up the host if needed
    if programOptions.asHost:
        if not programOptions.asHost in globalvars.HOST_PLATFORMS:
            parser.error ("Host needs to be one of the following: {0}".format (globalvars.HOST_PLATFORMS))
        globalvars.defaultPlatform = programOptions.asHost

    # For non-Windows it's case senstive, but ADO makes things all-caps so we just set it to match
    if os.name != 'nt':
        def findEnvAnyCase (varName):
            varName = varName.lower()
            for k in os.environ.keys():
                if k.lower() == varName:
                    return os.environ[k]
            return None

        if not 'SrcRoot' in os.environ:
            otherCaseVar = findEnvAnyCase ('SrcRoot')
            if otherCaseVar:
                os.environ['SrcRoot'] = otherCaseVar
            buildpaths.UpdateSrcRoot()
        if not 'OutRoot' in os.environ:
            otherCaseVar = findEnvAnyCase ('OutRoot')
            if otherCaseVar:
                os.environ['OutRoot'] = otherCaseVar

    # This is for azure build pipelines where it's hard to get environment case correct so the command line will set it
    if programOptions.srcRootDirectory:
        os.environ['SrcRoot'] = compat.getStringForEnv(programOptions.srcRootDirectory).rstrip('/\\') + os.sep
        buildpaths.UpdateSrcRoot()
    if not 'SrcRoot' in os.environ:
        parser.error ("SrcRoot needs to be specified. Generally this is done in the environment but it can be done on the command line as well.")

    # Choose default partfile and partname from options
    (partFile, partName) = buildpart.splitPartDescriptor (programOptions.partName)
    if programOptions.partFile != None and partFile != None:
        parser.error ("A PartFile cannot be specified in both -p and -f options")

    programOptions.partName = partName
    programOptions.partFile = programOptions.partFile or partFile

    # Set and error check any languages passed in
    try:
        if programOptions.languages:
            if not translationkit.setLanguagesCSV (programOptions.languages, True):
                parser.error ("Wrong languages {0}".format (programOptions.languages))
    except utils.BBException as err:
        parser.error ("Wrong languages {0} ({1})".format (programOptions.languages, err.errmessage))

    # Error check priority
    if programOptions.processPriority not in range (0, 6):
        parser.error ("The priority argument must be a number from 0 (lowest priority) to 5 (highest priority)")

    globalvars.programOptions = programOptions

    # Choose the build strategy from the command line or env var
    buildStrategy = globalvars.programOptions.buildStrategy or os.getenv ("BuildStrategy")
    if not buildStrategy:
        parser.error( "No BuildStrategy specified" )

    if FULL_BB:
        debugaction.SaveActionList (parser)

    # Get the action object
    try:
        buildAction = Action.InstantiateFromList (parser.m_actions, programOptions.Action, programOptions.actionargs)
    except utils.BBException as err:
        utils.showErrorMsg (err.errmessage)
        utils.showErrorCallStack (err)
        return 1

    if not buildAction:
        parser.error ("Invalid action: {0}".format (programOptions.Action))

    startTime = time.time()

    # If requested clear the part cache
    if globalvars.programOptions.clearCache:
        import bblib.buildgraph
        bblib.buildgraph.BuildGraph.ClearPartCache()
        strategy.BuildStrategy.ClearStrategyCache()

    # Build all the strategies
    exitstat, errMessage = BuildAllStrategies (buildAction, buildStrategy, parser)

    # Display elapsed time
    currentTime = time.time()
    if None != buildAction:
        utils.showInfoMsg ("\n\n[{0}] {1}, elapsed time: {2}\n".format(buildAction.ActionName(),
                 utils.getNow(), time.strftime("%H:%M:%S", time.gmtime(currentTime-startTime))), utils.INFO_LEVEL_VeryInteresting)

    # Currently this just handles sending an email on a build
    statusContext = BentleyBuildShutdownContext (startTime, currentTime, exitstat, errMessage)
    if None != buildAction:
        buildAction.DoBentleyBuildShutdown (statusContext)
    return exitstat if exitstat != None else 0



