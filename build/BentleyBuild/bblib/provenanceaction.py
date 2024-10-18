#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------

import threading
from . import buildaction, buildpart, globalvars, parallelbuild, utils, translationkit

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def RunProvenance (thisAction, ignoreProvenanceErrors):
    # Check all provenance. Doing it as a separate action to take advantage of threading since it has to be done children-first.
    utils.showInfoMsg ('Updating provenance for all parts\n', utils.INFO_LEVEL_Interesting)
    provAction = CheckProvenanceAction (['provenance'])
    provAction.m_bgraph = thisAction.m_bgraph
    if thisAction.m_numThreads == 0:  # Have to respect 0 threads because this does rely on subparts being correct.
        provAction.m_numThreads = 0 
    provAction.DoBeforeAction ()
    provBuildMgr = parallelbuild.ParallelBuildManager (provAction.m_bgraph.m_graph, provAction.m_bgraph.m_rootNode, provAction)
    status = provBuildMgr.Build ()
    if status:
        raise utils.BuildError ('Provenance generation failed.')
    if provAction.m_provErrorList and not ignoreProvenanceErrors:
        thisAction.m_provWarningList = provAction.m_provWarningList
        thisAction.m_provErrorList = provAction.m_provErrorList
        provAction.m_additionalRequiredOutput = thisAction.ShowRequiredOutput
        thisAction.ShowRequiredOutput = provAction.ShowRequiredOutput
        utils.exitOnError (1, "Provenance Problems")
    else:
        provAction.DumpOuput (utils.UNCHANGED, utils.YELLOW)
    utils.showInfoMsg ('Provenance updated for all parts\n', utils.INFO_LEVEL_Interesting)
    return len(provAction.m_provErrorList) > 0

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class CheckProvenanceAction (buildaction.BentleyBuildAction):
    def ActionName(self):  return "CheckProvenance"

    @staticmethod
    def GetHelp ():
        return "Check all provenance"

    def GetUsage(self):      return "%(prog)s build --checkProvenance "

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__(self, inArgs):
        buildaction.BentleyBuildAction.__init__ (self, inArgs)
        
        self.m_numThreads = min (utils.GetCpuCount() * 2, 24)
        self.m_pause = 0.01

        self.m_provIssueRepos = set()
        self.m_provIssueLock = threading.Lock()
        self.m_provWarningList = []
        self.m_provErrorList = []
        self.m_additionalRequiredOutput = None
        self.m_repoProvs = {}
        self.m_repoProvLock = threading.Lock()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ShouldMessageRunning (self):        
        return False  # Don't put out a "running" message during parallel build

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def DoBeforeAction (self):
        # Store the top-most product or part for provenance.  Prefer a product, use a part if there is nothing else.
        # Also turn on provenance tracking.
        rootNode = self.m_bgraph.m_graph.PartNode(self.m_bgraph.m_rootNode).m_part
        for currnode in self.m_bgraph.m_graph.AllNodes():
            currentPart = self.m_bgraph.m_graph.PartNode(currnode).m_part
            currentPart.TrackProvenance ()
            currentPart.m_provProduct = rootNode

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def HandleProvError (self, msg, repoName, thisPart=None):
        # Override this method on Part to convert ignored changes to errors which we catch consistently.

        # Store/print only one of each message
        with self.m_provIssueLock:
            if repoName in self.m_provIssueRepos:
                return
            self.m_provIssueRepos.add (repoName)

        if thisPart:
            msg = msg + " building part {0}\n\n".format(thisPart)
        if 0 == globalvars.buildStrategy.FindProvenanceErrorStrategy (repoName):  # 0 = warn, 1 = error
            self.m_provWarningList.append (msg)
        else:
            self.m_provErrorList.append (msg)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetChildProvenance (self, thisPart):
        # Get all the provenances from children
        for child in thisPart.m_action.m_bgraph.m_graph.Children (thisPart.m_info.GetNodeIdentifier()):
            for repoName, repoProv in  thisPart.m_action.m_bgraph.m_graph.PartNode(child).m_part.m_provenances.items():
                try:
                    thisPart.AddProvenanceString (repoProv[0], repoName, repoProv[1], repoProv[2], repoProv[3], repoProv[4])
                except buildpart.ProvenanceError as err:
                    self.HandleProvError (err.errmessage, err.m_repo, thisPart)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetRepoProvenance (self, repo):
        # Cache string; for monorepos with lots of partfiles this is a help.
        if repo.m_name not in self.m_repoProvs:
            with self.m_repoProvLock:
                self.m_repoProvs[repo.m_name] = repo.GetProvenanceString()
        return self.m_repoProvs[repo.m_name]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def DoBuildAction (self, thisPart):
        if globalvars.buildStrategy.PartIsSkippedByDeferStrategy(thisPart.m_deferType):  # For parts with defer type Never just skip.
            utils.showInfoMsg ('Skipping saving lkgs for part {0} because it is deferred to Never.\n'.format(thisPart.m_info.m_name), utils.INFO_LEVEL_SomewhatInteresting)
            return

        try:
            thisPart.ShowProvenanceError = self.HandleProvError
            self.GetChildProvenance (thisPart)

            if not translationkit.IsTranskitShell ():
                if thisPart.FromLkgs():  # Add provenance from LKG, whether we've built this part already or not
                    thisPart.ReadProvenanceFile (thisPart.GetLogDirEntryName(globalvars.provfileName), True)
                else:
                    thisPart.AddProvenanceString (thisPart, thisPart.m_info.m_repo.m_name, self.GetRepoProvenance(thisPart.m_info.m_repo), False, True, thisPart.m_info.m_repo.GetType())
                    if thisPart.m_additionalRepos:
                        for additionalRepo in thisPart.m_additionalRepos:
                            localRepo = globalvars.buildStrategy.FindLocalRepository (additionalRepo)
                            thisPart.AddProvenanceString (thisPart, localRepo.m_name, self.GetRepoProvenance(localRepo), False, True, localRepo.GetType())

        except buildpart.ProvenanceError as err:
            self.HandleProvError (err.errmessage, err.m_repo, thisPart)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def DumpOuput (self, warnColor, errColor):
        if len(self.m_provWarningList) > 0:
            utils.showSummaryHeader( "Provenance Warnings" )
            for item in self.m_provWarningList:
                utils.showInfoMsg (item, utils.INFO_LEVEL_Essential, warnColor)

        if len(self.m_provErrorList) > 0:
            utils.showSummaryHeader( "Provenance Errors" )
            for item in self.m_provErrorList:
                utils.showInfoMsg (item, utils.INFO_LEVEL_Essential, errColor)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ShowRequiredOutput (self):
        if self.m_additionalRequiredOutput:
            self.m_additionalRequiredOutput ()
        self.DumpOuput (utils.YELLOW, utils.RED)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def DoAfterAction (self):
        if len(self.m_provErrorList) > 0:
            utils.exitOnError (1, "")
