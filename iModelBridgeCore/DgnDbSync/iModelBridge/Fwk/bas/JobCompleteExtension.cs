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
using System.Diagnostics;
using System.Xml;
using System.Threading;
using Bentley.Automation;
using Bentley.Automation.Extensions;
using Bentley.Automation.Messaging;
using Bentley.Orchestration.MSProcessor;
using Bentley.Automation.JobConfiguration;
using BSI.Orchestration.Utility;
using BSI.Automation;

namespace BentleyB0200.Dgn.DgnV8Mirror.ICS
{
    /// <summary>
    /// The extension that calls iModelBridgeFwk to do the work of calling bridges, updating the briefcase, and pushing changesets to iModelHub.
    /// </summary>
    public class JobCompleteExtension : ASJobCompleteExtension
        {
        /// <summary>
        /// Constructor
        /// </summary>
        public JobCompleteExtension
        (
        ) : base(DgnV8MirrorICSPluginConstants.DocumentProcessorName,
            new Guid(DgnV8MirrorICSPluginConstants.DocumentProcessorGuid),
            DgnV8MirrorICSPluginConstants.DocumentProcessorDescription)
            {
            }


        /// <summary>
        /// Called when the job is to be completed. This is where we actually process the files.
        /// </summary>
        /// <param name="asContext">Automation Service context object</param>
        /// <returns></returns>
        public override void OnJobComplete
        (
        ASContext asContext
        )
            {

            }
        }
}