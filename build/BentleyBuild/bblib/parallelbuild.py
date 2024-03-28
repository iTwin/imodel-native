#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------

from . import globalvars, symlinks, utils
import bisect, threading, time, sys, traceback, queue


#-------------------------------------------------------------------------------------------
# Have to extract the stack string in the other thread.  Store it in this simple object
#
# bsiclass
#-------------------------------------------------------------------------------------------
class ExceptionWithStackString (object):
    def __init__(self, exception, stackString):
        self.errmessage = str(exception)
        self.exception = exception
        self.verbosity = utils.INFO_LEVEL_RarelyUseful   # Python exceptions we should always dump the stack.  Internal we only dump at V6.
        if not (isinstance (exception, utils.BBException) or isinstance (exception, symlinks.SymLinkError)):
            self.verbosity = utils.INFO_LEVEL_Essential
        
        self.stackString = utils.getErrorCallStack(exception)
        if not self.stackString:
            self.stackString = stackString

    def ShowError (self):
        utils.showInfoMsg (self.errmessage+'\n', utils.INFO_LEVEL_Essential, utils.RED)
        utils.showInfoMsg (self.stackString, self.verbosity)


#-------------------------------------------------------------------------------------------
# Keep a sorted list of available nodes.  When a node is built it will inform the class which
#   will check the parents.  If all the parents' children have fired then it moves into the list.
# This essentially simulates the build (albeit single-treaded) by generating a full list of the nodes in the order they
#   will build.  
#
# bsiclass
#-------------------------------------------------------------------------------------------
class LeafList ():
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__(self, graph):
        # Only called from main thread.
        self.m_daGraph = graph
        self.m_lock = threading.Lock()
        self.FillList()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def FillList (self):
        # Pick out the initial leaf nodes
        leaves = []
        for currnode in self.m_daGraph.AllNodes():
            if len(self.m_daGraph.Children (currnode)) == 0:
                leaves.append (currnode)

        self.m_list = sorted(leaves, key=lambda x: self.m_daGraph.PartNode(x).m_nodeWeight)
        # To keep things sorted I have to keep a separate array for the weights.
        self.m_weights = [self.m_daGraph.PartNode(x).m_nodeWeight for x in self.m_list]
        
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def UpdateForPreprocessed (self):
        # From Rebuild we will have a lot of items marked as completed.  Remove them.  Use a copy because original is being modified.
        for currnode in self.m_list[:]:
            if self.m_daGraph.PartNode(currnode).m_processed:
#                print '11111 Delisting built node {0}'.format (currnode)
                index = self.m_list.index (currnode)
                del (self.m_list[index])
                del (self.m_weights[index])
                self.MarkNodeBuilt (currnode)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetNextNode (self):
        with self.m_lock:
            nextNode = self.m_list.pop()
            self.m_weights.pop()
        return nextNode

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def InsertNode (self, node):
        # This is called from inside a locked section in NodeBuilt
        weight = self.m_daGraph.PartNode(node).m_nodeWeight
        insertPoint = bisect.bisect_left(self.m_weights, weight)
        self.m_list.insert(insertPoint, node)
        self.m_weights.insert(insertPoint, weight)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def MarkNodeBuilt (self, node): 
        # This is called from inside a locked section in NodeBuilt or from preprocessing.  
        # Don't want to lock here because it's recursive. 

        # Check all parents & add to the list if all children have been processed
        for parent in self.m_daGraph.Parents (node):
            parentPart = self.m_daGraph.PartNode(parent)
            parentPart.m_childrenProcessed += 1
            
            # For debugging
#            if not hasattr(parentPart, "m_processedChildrenList"):
#                parentPart.m_processedChildrenList = []
#            parentPart.m_processedChildrenList.append (node)
            
            if parentPart.m_childrenProcessed == len(self.m_daGraph.Children (parent)):
                if self.m_daGraph.PartNode(parent).m_processed: # Rebuild will mark nodes as processed so they don't get inserted; just want the rest of the behavior
                    utils.showInfoMsg ('Skipping part as filtered: {0}.\n'.format (self.m_daGraph.PartNode(parent).m_part.GetShortRepresentation()), utils.INFO_LEVEL_RarelyUseful)
                    self.MarkNodeBuilt (parent)
                else:
                    utils.showInfoMsg ('Adding part {0} to list of processable parts.\n'.format (self.m_daGraph.PartNode(parent).m_part.GetShortRepresentation()), utils.INFO_LEVEL_RarelyUseful)
                    self.InsertNode (parent)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def NodeBuilt (self, node):
        with self.m_lock:
            self.MarkNodeBuilt (node)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Size (self):
        with self.m_lock:
            return len(self.m_list)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Dump (self):
        for leaf in self.m_list:
            utils.showInfoMsg ('leaf ({0}): {1}'.format (self.m_daGraph.PartNode(leaf).m_nodeWeight, leaf), utils.INFO_LEVEL_Essential)

            
#-------------------------------------------------------------------------------------------
# Thread class to run the build.
#
# bsimethod
#-------------------------------------------------------------------------------------------
class PartActionExecuter (threading.Thread):
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__(self, nodePart, manager):
        self.m_nodePart = nodePart
        self.m_manager = manager
        self.m_exception = None
        threading.Thread.__init__(self)
    
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def run(self):
        # Do the actions
        self.m_nodePart.m_part.AtThreadStart (self.ident)
        try:
            self.m_nodePart.Build(self.m_manager.GetBuildAction())
            self.m_nodePart.m_succeeded = True
        except Exception as err:
            # In order to preserve a call stack, recast the exception
            self.m_exception = ExceptionWithStackString (err, traceback.format_exc ())
            self.m_nodePart.m_succeeded = False
        finally:
            # Copy over the logbuffer
            self.m_nodePart.m_logBuffer = utils.getThreadLineWriter()
            # Notify the manager that this build is done
            self.m_manager.ActionFinished (self)
        
#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
class ParallelBuildManager (object):

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__(self, dependancygraph, rootNode, buildAction):
        self.m_daGraph = dependancygraph
        self.m_rootNode = rootNode
        self.m_buildAction = buildAction
        self.m_errorStop = False
        self.m_buildFinishedNodes = queue.Queue()   # Threadsafe
        self.m_completedNodes = []                  # Just a list of everything we've completed
        self.m_startedNodes = []                    # Just a list of everything we've started - temporary to help debugging.
        self.m_activeNodes = []

        self.m_errorStatus = []

        self.m_threadsAllowed = buildAction.GetNumThreads()
        self.m_pauseDuration = buildAction.GetPauseDuration()
        self.m_threadpoolCount = threading.BoundedSemaphore(self.m_threadsAllowed)
        self.m_partsExecuted = 0

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetBuildAction (self): 
        return self.m_buildAction

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetRunningPartsList (self): 
        return [x.m_part.GetShortRepresentation() for x in self.m_activeNodes]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetRemainingPartCount (self): 
        return self.m_totalNodes-self.m_partsExecuted

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetCurrentRunningThreads (self):
        if utils.py3:
            return self.m_threadsAllowed-self.m_threadpoolCount._value # pylint: disable=protected-access
        else:
            return self.m_threadsAllowed-self.m_threadpoolCount._Semaphore__value  # pylint: disable=protected-access,no-member

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ActionFinished (self, executer): 
        # Once the build is completed, adjust the list of child nodes as necessary and release 
        #  the thread back to the thread pool.
        # This runs in the child thread.
        self.m_leaves.NodeBuilt (str(executer.m_nodePart))
        if executer.m_exception:
            self.m_errorStop = True
            self.m_errorStatus.append (executer)
        else:
            self.m_buildFinishedNodes.put (executer.m_nodePart)
        self.m_activeNodes.remove (executer.m_nodePart)
        self.m_threadpoolCount.release()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def StartWorkerThread (self, nodePart):
        # Get next available thread
        self.m_threadpoolCount.acquire()
        # Double check that there wasn't a failure in the thread that just released for us to grab
        if self.m_errorStop:
            self.m_threadpoolCount.release()
            return
        self.m_partsExecuted += 1
        self.m_activeNodes.append (nodePart)
        self.m_startedNodes.append (nodePart)
#        self.PrintRunningParts (utils.INFO_LEVEL_Important, foregroundColor=utils.LIGHT_BLUE)
        # Creates a new thread every time.  Safer but less efficient.
        PartActionExecuter (nodePart, self).start()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def PrintCompletedNodeOutput (self):
        while not self.m_buildFinishedNodes.empty():
            node = self.m_buildFinishedNodes.get()
            self.m_completedNodes.append (node)
            if not node.m_logBuffer.Empty () and self.m_buildAction.DisplayBuildOutput(node.m_succeeded):
                utils.showInfoMsg ('Output from part {0}\n'.format(node.m_part.GetShortRepresentation()), utils.INFO_LEVEL_Important, foregroundColor=utils.LIGHT_BLUE)
                node.m_logBuffer.WriteToScreen()

        # After dumping all the output it's nice to see what's still running
        if self.m_buildAction.ShouldMessageRunning() and self.GetRemainingPartCount() > 0:
            utils.showStatusMsg ('Running parts ({1} remain): {0}\n'.format (self.GetRunningPartsList(), self.GetRemainingPartCount()), utils.INFO_LEVEL_Important, textColor=utils.LIGHT_BLUE)
            
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def SingleNodeAction (self, currnode):
        currpart = self.m_daGraph.PartNode(currnode)
        self.StartWorkerThread(currpart)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def VisitNodesForAction (self):
        utils.showInfoMsg ('Visiting all parts in {0} threads\n'.format(self.m_threadsAllowed), utils.INFO_LEVEL_VeryInteresting)
        self.m_partsExecuted = 0
        while self.m_partsExecuted < self.m_totalNodes and not self.m_errorStop:
            # if we don't have anything to process, wait for some of the other threads 
            currentStall = 0
            currentZeroThreadStall = 0
            currentExecuted = self.m_partsExecuted
            while self.m_leaves.Size() == 0:
                # Indefinite wait if we have running threads; no idea how long it will take for them to finish.
                if self.GetCurrentRunningThreads() > 0:
                    currentStall += self.m_pauseDuration

                    # For PRG they would like to see the log file that we are hung at so they can check.
                    if len (self.m_activeNodes) == 1 and currentStall > 1 and abs(currentStall % 200) < self.m_pauseDuration:
                        utils.showInfoMsg ('Logfile for {0} is {1}\n'.format \
                                (self.m_activeNodes[0].m_part.GetShortRepresentation(), self.m_activeNodes[0].m_part.GetLogFileName()), utils.INFO_LEVEL_VeryInteresting)

                    if abs(currentStall % 1) < self.m_pauseDuration:
                        utils.showStatusMsg ('Waiting for part(s) {0} for {1:.0f} seconds; {2} parts remain'.format \
                            ([x.m_part.GetShortRepresentation() for x in self.m_activeNodes], currentStall, self.m_totalNodes-self.m_partsExecuted), \
                            utils.INFO_LEVEL_VeryInteresting)
                    time.sleep (self.m_pauseDuration)
                # The case where we are waiting for a thread to fill the tree with new leaves. A few seconds max.
                else:
                    if currentZeroThreadStall < 5:
                        currentZeroThreadStall += 1
                        time.sleep(1)
#                        print '\n\n 11111 stalling {0}  threads {1}\n\n'.format(stallCount, self.GetCurrentRunningThreads())
                    else:
                        utils.exitOnError( 1, "Threading issue; no jobs available even after 5 seconds" )

                # dump any output
                if currentExecuted != self.m_partsExecuted:
                    self.PrintCompletedNodeOutput ()
                    currentExecuted = self.m_partsExecuted
                    
#            print '11111 self.m_partsExecuted = {0} self.m_totalNodes = {1}  self.m_errorStop = {2}'.format (self.m_partsExecuted, self.m_totalNodes, self.m_errorStop)
#            print '11111 self.m_leaves.Size() = {0} self.runningThreads = {1} currentStall = {2}'.format (self.m_leaves.Size(), self.GetCurrentRunningThreads(), currentStall)
            # Process the node
            if self.m_partsExecuted != self.m_totalNodes: # If we were stalling waiting for the last few parts to finish, do not try to process
                node = self.m_leaves.GetNextNode()
                self.SingleNodeAction (node)
            self.PrintCompletedNodeOutput ()

        # Wait for everything to finish
        currentStall = 0
        while len(self.m_activeNodes) > 0:
            if abs(currentStall % 1) < self.m_pauseDuration:
                utils.showStatusMsg ('Waiting for part(s) to finish{2}: {0}  for {1:.0f} seconds'.format \
                    ([x.m_part.GetShortRepresentation() for x in self.m_activeNodes], currentStall, ' after failure' if self.m_errorStop else ''), \
                    utils.INFO_LEVEL_VeryInteresting, textColor=utils.LIGHT_BLUE)
            currentStall += self.m_pauseDuration
            time.sleep (self.m_pauseDuration)

        # Write remaining successful bits
        self.PrintCompletedNodeOutput ()

        # Success condition
        if not self.m_errorStop:
            utils.showInfoMsg ('\nBuilt {0} of {1} items\n'.format (self.m_partsExecuted, self.m_totalNodes), utils.INFO_LEVEL_VeryInteresting)
            return 0 
            
        # Dump out any builds that failed
        numPrevPartsToDump = 2 * self.m_threadsAllowed
        for ex in self.m_errorStatus:
            # Dump the "clean" output
            ex.m_nodePart.m_logBuffer.WriteToScreen()
            # Dump the exceptions
            if ex.m_exception:
                utils.showInfoMsg ('======================================================\n', utils.INFO_LEVEL_Interesting)
                utils.showInfoMsg ('Exception in part {0} [{1}]\n'.format (ex.m_nodePart.m_part.GetShortRepresentation(),ex. m_nodePart.m_part.m_info.m_file), utils.INFO_LEVEL_Interesting)
                utils.showInfoMsg ('last {0} completed parts: \n   {1}\n'.format (numPrevPartsToDump, \
                    '\n   '.join([x.m_part.GetUniqueRepresentation() for x in self.m_completedNodes[-numPrevPartsToDump:]])), \
                    utils.INFO_LEVEL_Interesting)
                utils.showInfoMsg ('\nlast {0} started parts: \n   {1}\n'.format (numPrevPartsToDump, \
                    '\n   '.join([x.m_part.GetUniqueRepresentation()  for x in self.m_startedNodes[-numPrevPartsToDump:]])), \
                    utils.INFO_LEVEL_Interesting)
                utils.showInfoMsg ('======================================================\n\n', utils.INFO_LEVEL_Interesting)
                ex.m_exception.ShowError()

        utils.showInfoMsg ('\nBuilt {0} of {1} items\n'.format (self.m_partsExecuted, self.m_totalNodes), utils.INFO_LEVEL_VeryInteresting)
        return 1
        
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def WeightNodes (self, rootNode):
        # FeatureAspect parts need to happen first, along with all their sub-parts.
        # I'm adding a "phase' number up front that should force higher numbers to go first.
        self.m_daGraph.PartNode(rootNode).m_nodeWeight = (0, 1)
        for node in self.m_daGraph.AllNodes():
            currentPart = self.m_daGraph.PartNode(node)
            if currentPart.m_part.m_featureAspectPart:
                currentPart.m_nodeWeight = (1, currentPart.m_nodeWeight[1])
            
        for parentNode, childNode in self.m_daGraph.BreadthFirstWalkOfChildren (rootNode):
            currentPart = self.m_daGraph.PartNode(parentNode)
            childPart = self.m_daGraph.PartNode(childNode)

            weight = childPart.m_nodeWeight[1] + currentPart.m_nodeWeight[1]
            phase = currentPart.m_nodeWeight[0] if currentPart.m_nodeWeight[0] > childPart.m_nodeWeight[0] else childPart.m_nodeWeight[0]
            childPart.m_nodeWeight = (phase, weight)
    
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def CheckForCircular (self, currnode, processed):
        processed.append(currnode)
        self.m_daGraph.PartNode(currnode).m_processed = True
        for child in self.m_daGraph.Children (currnode):
            # Convenient place to look for circular dependencies.
            if child in processed:
                circMsg = "Illegal circular PartFile nesting for {0} from {1} :".format \
                    (self.m_daGraph.PartNode(child).m_part.m_info.m_name, self.m_daGraph.PartNode(currnode).m_part.m_info.m_name)
                msg2 = '\nIf you have used any Sequential part attributes this can cause or exacerbate this problem.\n'
                raise utils.PartBuildError (circMsg+msg2, self.m_daGraph.PartNode(currnode).m_part)

            if not self.m_daGraph.PartNode(child).m_processed:
                self.CheckForCircular (child, processed)
        processed.pop()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def DepthFirstBuild (self, currnode):
        # Build parts in the old style - depth first walk
        for child in self.m_daGraph.Children (currnode):
            if self.m_daGraph.PartNode(child).m_processed != 1:  # 1 is processed, 2 is filtered
                self.DepthFirstBuild (child)
                
        # Build and dump output
        currPartNode = self.m_daGraph.PartNode(currnode)
        
        # Check for filtered
        if currPartNode.m_processed == 2:
            currPartNode.m_processed = 1
            return

        while True:
            try:
                currPartNode.Build(self.GetBuildAction())
                currPartNode.m_succeeded = True
                self.m_buildFinishedNodes.put (currPartNode)
                self.m_partsExecuted += 1
                break
            except Exception as err:
                currPartNode.m_succeeded = False
                self.m_buildFinishedNodes.put (currPartNode)

                if not self.GetBuildAction().PromptOnError():
                    if isinstance (err, utils.BBException) or isinstance (err, symlinks.SymLinkError):
                        raise err
                    else:
                        raise utils.BuildError (str(err) + '\n' + traceback.format_exc ())

                # This is the promptOnError case.
                utils.showInfoMsg ("\nHit return to retry, 'a' to clean and retry, or 'x' to exit: ", utils.INFO_LEVEL_Important, utils.YELLOW)
                sys.stdout.flush()
                sys.stdin.flush()
                ans = sys.stdin.read(1)
                if ans.lower() == 'a':                              # do a clean and retry it.
                    currPartNode.m_part.ReportFailAndClobberBuild ()
                    continue
                elif ans.lower() == 'x':                            # user wants us to exit, so just break out of loop
                    raise err
                else:                                               # on any other input (including just return), continue trying to build this part normally.
                    currPartNode.m_part.m_action.PrepareToReprocessPart(currPartNode.m_part)
                    continue
        
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def DepthFirstCount (self, currnode, count):
        for child in self.m_daGraph.Children (currnode):
            if self.m_daGraph.PartNode(child).m_processed != 1:  # 1 is processed, 2 is filtered
                self.DepthFirstCount (child, count)
                
        currPartNode = self.m_daGraph.PartNode(currnode)

        if currPartNode.m_processed == 0:
            count[0] += 1
        
        # Check for filtered
        currPartNode.m_processed = 1

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def BuildWithoutThreads (self):
        # If 0 threads are specified then build depth-first.
        # This is different than 1 thread where it will still go through the weighting mechanism and 
        #   spawn a single thread, store the output, and so forth.
        utils.showInfoMsg ('Visiting all parts in a single thread (depth-first)\n', utils.INFO_LEVEL_VeryInteresting)
        self.m_daGraph.ClearProcessedBit ()

        # Store the size for later use
        counter = [0]
        self.DepthFirstCount (self.m_rootNode, counter)
        self.m_totalNodes = counter[0]
        self.m_daGraph.ClearProcessedBit ()

        if self.m_buildAction.FilterParts ():
            # Reduce the count to how many nodes we will actually build
            for currnode in self.m_daGraph.AllNodes():
                if self.m_daGraph.PartNode(currnode).m_processed:
                    self.m_totalNodes -= 1
                    self.m_daGraph.PartNode(currnode).m_processed = 2  # Use this as a flag since we have to walk children in depth-first mode

        self.DepthFirstBuild (self.m_rootNode)
        utils.showInfoMsg ('\nBuilt {0} of {1} items\n'.format (self.m_partsExecuted, self.m_totalNodes), utils.INFO_LEVEL_VeryInteresting)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Build (self):
        # Check for circular references
        self.m_daGraph.ClearProcessedBit ()
        self.CheckForCircular (self.m_rootNode, [])
        
        # Store the size for later use
        self.m_totalNodes, _ = self.m_daGraph.Size ()

        # If 0 threads are specified then build depth-first.
        if self.GetBuildAction().GetNumThreads() == 0:
            return self.BuildWithoutThreads ()
    
        # Weight the nodes by the total number of parents between the node and the root so that 
        #   the nodes that will block the most builds will happen first.
        self.WeightNodes (self.m_rootNode)
       
        # Find the initial set of leaves - parts with no dependencies.
        self.m_leaves = LeafList (self.m_daGraph)

        # Allow the action to mark things as processed (Rebuild)
        self.m_daGraph.ClearProcessedBit ()
        if self.m_buildAction.FilterParts ():
            # Reduce the count to how many nodes we will actually build
            for currnode in self.m_daGraph.AllNodes():
                if self.m_daGraph.PartNode(currnode).m_processed:
                    self.m_totalNodes -= 1
            # Clear down to first processable nodes
            self.m_leaves.UpdateForPreprocessed ()
            
#        self.m_leaves.Dump()
        if self.m_totalNodes == 0:
            utils.showInfoMsg ('All nodes filtered out\n', utils.INFO_LEVEL_SomewhatInteresting)
            return 0

        return self.VisitNodesForAction ()        
    
#-------------------------------------------------------------------------------------------
# Thread class to run the build.
#
# bsimethod
#-------------------------------------------------------------------------------------------
class RepoActionExecuter (threading.Thread):
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__(self, item, manager):
        self.m_item = item
        self.m_manager = manager
        self.m_exception = None
        threading.Thread.__init__(self)
    
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def run(self):
        # Do the actions
        utils.addBufferToCurrentThread()
        try:
            self.m_manager.GetBuildAction().DoBuildAction(self.m_item)
        except Exception as err:
            # In order to preserve a call stack, recast the exception
            self.m_exception = ExceptionWithStackString (err, traceback.format_exc ())
        finally:
            # Copy over the logbuffer
            self.m_item.m_logBuffer = utils.getThreadLineWriter()
            # Notify the manager that this build is done
            self.m_manager.ActionFinished (self)
        
#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
class ParallelWalkManagerBase (object):

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__(self, buildgraph, buildAction):
        self.m_bgraph = buildgraph
        self.m_buildAction = buildAction
        self.m_errorStop = False

        self.m_errorStatus = []

        self.m_buildFinishedNodes = queue.Queue()   # Threadsafe
        self.m_completedNodes = []                  # Just a list of everything we've completed

        self.m_threadsAllowed = buildAction.GetNumThreads()
        self.m_threadpoolCount = threading.BoundedSemaphore(self.m_threadsAllowed)
        
        # Must be overridden by subclass
        self.m_multiThreadMessage = None
        self.m_singleThreadMessage = None
    
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetBuildAction (self): 
        return self.m_buildAction

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetCurrentRunningThreads (self): 
        if utils.py3:
            return self.m_threadsAllowed-self.m_threadpoolCount._value # pylint: disable=protected-access
        else:
            return self.m_threadsAllowed-self.m_threadpoolCount._Semaphore__value  # pylint: disable=protected-access,no-member

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetItemList (self):
        pass

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ItemExceptionMessage (self, ex):
        pass

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def PreprocessItem (self, listItem): 
        return listItem

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def PrintRemaining (self, iItem): 
        utils.showStatusMsg ('{0} remain'.format (self.m_itemListSize-iItem+self.GetCurrentRunningThreads()), utils.INFO_LEVEL_VeryInteresting)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ActionFinished (self, executer): 
        # Once the build is completed release the thread back to the thread pool.
        if executer.m_exception:
            self.m_errorStop = True
            self.m_errorStatus.append (executer)
        else:
            self.m_buildFinishedNodes.put (executer.m_item)
        self.m_threadpoolCount.release()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def PrintCompletedNodeOutput (self):
        while not self.m_buildFinishedNodes.empty():
            node = self.m_buildFinishedNodes.get()
            self.m_completedNodes.append (node)
            if not node.m_logBuffer.Empty ():
                self.m_buildAction.AnnounceOutput (node, utils.INFO_LEVEL_Important, utils.LIGHT_BLUE)
                node.m_logBuffer.WriteToScreen()
                node.m_logBuffer = None  # Turn it off so that future output won't be cached
            
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def StartWorkerThread (self, repo):
        # Get next available thread
        self.m_threadpoolCount.acquire()
        # Double check that there wasn't a failure in the thread that just released for us to grab
        if self.m_errorStop:
            self.m_threadpoolCount.release()
            return
        self.m_reposProcessed += 1
        # Creates a new thread every time.  Safer but less efficient.
        RepoActionExecuter (repo, self).start()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def SingleRepoAction (self, repo):
        self.StartWorkerThread(repo)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def VisitReposForAction (self):
        utils.showInfoMsg (self.m_multiThreadMessage, utils.INFO_LEVEL_VeryInteresting)
        self.m_reposProcessed = 0
        
        # Process all the repos
        iItem = 0
        itemList = self.GetItemList() # pylint: disable=assignment-from-no-return
        self.m_itemListSize = len (itemList)
        for item in itemList:
            itemToProcess = self.PreprocessItem (item)
            self.SingleRepoAction (itemToProcess)
            iItem += 1
            self.PrintRemaining (iItem)
            self.PrintCompletedNodeOutput()
            if self.m_errorStop:
                break
        
        # Wait for everything to finish
        while self.GetCurrentRunningThreads() > 0:
            self.PrintRemaining (iItem)
            time.sleep (2)
        
        # Write remaining successful bits
        self.PrintCompletedNodeOutput ()

        if not self.m_errorStop:
            return 0 # Success!

        # Dump out any builds that failed
        for ex in self.m_errorStatus:
            if ex.m_exception:
                utils.showInfoMsg ('======================================================\n', utils.INFO_LEVEL_Essential, foregroundColor=utils.RED)
                utils.showInfoMsg (self.ItemExceptionMessage(ex), utils.INFO_LEVEL_Essential, foregroundColor=utils.RED)
                utils.showInfoMsg ('======================================================\n\n', utils.INFO_LEVEL_Essential, foregroundColor=utils.RED)
                ex.m_exception.ShowError()
        return 1

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def BuildWithoutThreads (self):
        # If 0 threads are specified then build sequentially.
        # This is different than 1 thread where it will still go through the weighting mechanism and 
        #   spawn a single thread, store the output, and so forth.
        utils.showInfoMsg (self.m_singleThreadMessage, utils.INFO_LEVEL_VeryInteresting)
        for repoName in self.GetItemList():
            itemToProcess = self.PreprocessItem (repoName)
            itemToProcess.m_logBuffer = utils.LogBuffer() # Dummy buffer; stuff will fall out immediately
            self.GetBuildAction().DoBuildAction(itemToProcess)
            self.m_buildFinishedNodes.put (itemToProcess)
            self.PrintCompletedNodeOutput()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Build (self):
        # If 0 threads are specified then run sequentially.
        if self.GetBuildAction().GetNumThreads() == 0:
            return self.BuildWithoutThreads ()
        else:
            return self.VisitReposForAction ()

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
class ParallelRepoWalkManager (ParallelWalkManagerBase):

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__(self, buildgraph, buildAction):
        ParallelWalkManagerBase.__init__(self, buildgraph, buildAction)
        self.m_singleThreadMessage = 'Visiting all repositories in a single thread\n'
        self.m_multiThreadMessage = 'Visiting all repositories\n'

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetItemList (self): 
        if self.GetBuildAction().TraverseLkgReposToo():
            fullList = self.m_bgraph.m_repoNames.union (self.m_bgraph.m_lkgRepoNames)
            return fullList
        else:
            return self.m_bgraph.m_repoNames

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def PreprocessItem (self, listItem): 
        return globalvars.buildStrategy.FindLocalRepository (listItem)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ItemExceptionMessage (self, ex): 
        return 'Exception in repo {0}\n'.format (ex.m_item.m_name)


#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
class ParallelPartWalkManager (ParallelWalkManagerBase):

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__(self, buildgraph, buildAction):
        ParallelWalkManagerBase.__init__(self, buildgraph, buildAction)
        self.m_singleThreadMessage = 'Visiting all parts in a single thread\n'
        self.m_multiThreadMessage = 'Visiting all parts\n'

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetItemList (self): 
        partList = []
        self.m_buildAction.FilterParts()
        for currnode in self.m_bgraph.m_graph.AllNodes():
            if not self.m_bgraph.m_graph.PartNode(currnode).m_processed == 1:
                partList.append (self.m_bgraph.m_graph.PartNode(currnode).m_part)

        return partList

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def PreprocessItem (self, listItem): 
        return listItem

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ItemExceptionMessage (self, ex): 
        return 'Exception in part {0}\n'.format (ex.m_item.m_info.m_name)
