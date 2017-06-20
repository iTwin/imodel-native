/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/bas/JobCompleteExtension.cs $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------+
|
|   Usings
|
+--------------------------------------------------------------------------------------*/
using System;
using Bentley.Automation;
using Bentley.Automation.Extensions;
using Bentley.Automation.Messaging;
using Bentley.Orchestration.Extensibility;
using Bentley.Automation.JobConfiguration;
//using PwApiWrapper;
using BSI.Orchestration.Utility;
using BSI.Automation;



namespace BentleyB0200.Dgn.DgnV8Mirror.ICS
{
    /// <summary>
    /// Implements the smart dispatcher extension.  This class is
    /// responsible for creating and controlling the step sequence required
    /// to index widgets.
    /// </summary>
    public class JobCompleteExtension : ASJobCompleteExtension
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public JobCompleteExtension
        (
        ): base (DgnV8MirrorICSPluginConstants.DocumentProcessorName,
            new Guid (DgnV8MirrorICSPluginConstants.DocumentProcessorGuid),
            DgnV8MirrorICSPluginConstants.DocumentProcessorDescription)
        {
        }

        /// <summary>
        /// Gives the smart dispatcher the sequence of steps needed to index widgets.
        /// </summary>
        /// <param name="asContext">Automation Service context object</param>
        /// <param name="previousPi">The previous step</param>
        /// <param name="isFirstRequestForProcessingInstructions">First step?</param>
        /// <returns>The next processing instruction to execute</returns>
        public override void OnJobComplete 
        (
        ASContext asContext
        )
        {
        }
    }
}