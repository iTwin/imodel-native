/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/bas/DelegateProcessorExtension.cs $
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
using System.Collections.Generic;
using System.IO;
using System.Xml;
using System.Threading;
using Microsoft.Win32;

using Bentley.Automation;
using Bentley.Automation.Extensions;
using Bentley.Automation.Messaging;
using Bentley.Automation.JobConfiguration;
using BSI.Orchestration.Utility;
using BSI.Automation;

//using PwApiWrapper;

namespace BentleyB0200.Dgn.DgnV8Mirror.ICS
    {
/// <summary>
    /// Called by the delegate processor to work on DgnV8MirrorICSPlugin
    /// messages sent from the SmartDispatcher.
    /// </summary>
    public class DelegateProcessorExtension: ASDelegateProcessorExtension
        {
        /// <summary>
        /// Constructor
        /// </summary>
        public DelegateProcessorExtension 
        (
        )  : base (DgnV8MirrorICSPluginConstants.DocumentProcessorName,
                   new Guid (DgnV8MirrorICSPluginConstants.DocumentProcessorGuid),
                   DgnV8MirrorICSPluginConstants.DocumentProcessorDescription)
            {
            }

        /// <summary>
        /// Called when the processor is started
        /// </summary>
        public override void OnStart ()
            {
            }

        /// <summary>
        /// Called when the processor is shut down
        /// </summary>
        public override void OnShutdown ()
            {
            }

        /// <summary>
        /// Processes DgnV8MirrorICSPlugin message anyway you see fit, just no modal dialog boxes.
        /// </summary>
        /// <param name="asContext">Automation Service context object</param>
        /// <param name="currentProcessingInstruction">Currently active processing instruction.</param>
        /// <returns>true - message sucessfully processed.</returns>
        public override bool DoMessageProcessing 
        (
        ASContext asContext, 
        ProcessingInstruction currentProcessingInstruction
        )
            {
            asContext.ReplaceProcessingInstructions (new[] { currentProcessingInstruction });
            return true;
            }
        }
    }
