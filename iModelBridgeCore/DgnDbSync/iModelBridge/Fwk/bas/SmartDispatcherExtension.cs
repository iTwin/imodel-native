/*--------------------------------------------------------------------------------------+
|
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
|  Limited permission is hereby granted to reproduce and modify this copyrighted
|  material provided that the resulting code is used only in conjunction with
|  Bentley Systems products under the terms of the license agreement provided
|  therein, and that this notice is retained in its entirety in any such
|  reproduction or modification.
|
+--------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------+
|
|   Usings
|
+--------------------------------------------------------------------------------------*/
using System;
using System.Xml;
using Bentley.Applications;
using Bentley.Automation;
using Bentley.Automation.Extensions;
using Bentley.Automation.Messaging;
using Bentley.Orchestration.MSProcessor;
using Bentley.Orchestration.Extensibility;
using BSI.Orchestration.Utility;
using BSI.Automation;

namespace BentleyB0200.Dgn.DgnV8Mirror.ICS
    {
    /// <summary>
    /// Implements the smart dispatcher extension.  This class is
    /// responsible for creating and controlling the step sequence required
    /// to index widgets.
    /// </summary>
    public class SmartDispatcherExtension: ASSmartDispatcherExtension
        {
        /// <summary>
        /// Constructor
        /// </summary>
        public SmartDispatcherExtension
        (
        )
            : base (DgnV8MirrorICSPluginConstants.DocumentProcessorName,
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
        public override ProcessingInstruction GenerateProcessingInstructions
        (
        ASContext asContext,
        ProcessingInstruction previousPi,
        bool isFirstRequestForProcessingInstructions
        )
            {
            ProcessingInstruction nextPi = null;
            if (isFirstRequestForProcessingInstructions)
                {
                // Step 1 
                nextPi = GenerateDelegateExampleInstruction();
                }
            else
                {
                nextPi = GenerateExitInstruction (previousPi);
                }
            return nextPi;
            }

        private ProcessingInstruction GeneratePDFInstuction
        (
        ASContext asContext
        )
            {
            PowerPlatformProcessorInstruction mspi = new PowerPlatformProcessorInstruction ();
            mspi.DocumentProcessorName = DocumentProcessorName;
            mspi.DocumentProcessorGuid = DocumentProcessorGuid.ToString ();
            mspi.Step = DgnV8MirrorICSPluginConstants.Steps.PowerPlatformExample;
            PowerPlatformMessage msm = new PowerPlatformMessage ();

            msm.RequiredProcessorName = "MicroStation";
            msm.RequiredProcessorVersionRange = "08.11.09";
            msm.DocumentName = asContext.WorkingDocumentInfo.FilePath;
            msm.DocumentID = asContext.WorkingDocumentInfo.DocumentGuid;
            msm.Datasource = asContext.WorkingDocumentInfo.Datasource;
            msm.CustomDescription = String.Format ("BAS keyins for {0}", DgnV8MirrorICSPluginConstants.DocumentProcessorName);
            msm = AddAutomationServiceMDLApp (msm, "AutomationService.ma");
            msm.AddKeyin (String.Format ("automationservice setdocumentprocessor {0} {1}",
                       DgnV8MirrorICSPluginConstants.DocumentProcessorGuid, DgnV8MirrorICSPluginConstants.Steps.PowerPlatformExample));

            msm.AddKeyin (String.Format ("rd=\"{0}\"", asContext.WorkingDocumentInfo.FilePath));
            msm.AddKeyin ("mdl load bentley.microstation.printorganizer");
            msm.AddKeyin ("PRINTORGANIZER NEW");
            msm.AddKeyin ("PRINTORGANIZER PRINTERDRIVER pdf.pltcfg");
            msm.AddKeyin (String.Format ("PRINTORGANIZER ADD FILE \"{0}\"", asContext.WorkingDocumentInfo.FilePath));
            msm.AddKeyin ("PRINTORGANIZER PRINTTO FILE");
            msm.AddKeyin (String.Format ("PRINTORGANIZER PRINTDESTINATION \"{0}\"", Util.GetPDFFileFromSrcFile (asContext.WorkingDocumentInfo.FilePath)));
            msm.AddKeyin ("PRINTORGANIZER SUBMITAS SINGLE");
            msm.AddKeyin ("PRINTORGANIZER PRINT ALL");
            msm.AddKeyin ("automationservice stepcomplete");

            mspi.MicroStationMessage = msm;
            return mspi;
            }

        /// <summary>
        /// Generates exit instruction
        /// </summary>
        /// <param name="previousPi">Previous processing instruction</param>
        /// <returns></returns>
        private ProcessingInstruction GenerateExitInstruction
        (
        ProcessingInstruction previousPi
        )
            {
            ExitInstruction ei = null;
            if (previousPi.ProcessingResults != null)
                {
                ei = new ExitInstruction (previousPi.ProcessingResults);
                }
            else
                {
                string message = ASConstants.StringResources.GetString ("ErrorEmptyResults");
                ei = new ExitInstruction (new ProcessingResults (false, message));
                Log.Ref.Error (DgnV8MirrorICSPluginConstants.LoggingNamespace, message);
                }
            ei.DocumentProcessorName = DocumentProcessorName;
            ei.DocumentProcessorGuid = DocumentProcessorGuid.ToString ();
            return ei;
            }


        /*------------------------------------------------------------------------------------**/
        /// <author>Anthony.Falcone</author>                            <date>9/2014</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private PowerPlatformMessage AddAutomationServiceMDLApp
        (
        PowerPlatformMessage msm,
        string mdlApp
        )
            {
            string asInstallDir = "[PASInstallDir]";
            string ma = asInstallDir + mdlApp;
            string loadKeyin = "mdl load \"" + ma + "\"";
            msm.AddKeyin (loadKeyin);
            return msm;
            }


        /// <summary>
        /// Generates delegate processing instruction
        /// </summary>
        /// <returns>Delegate processing instruction</returns>
        private ProcessingInstruction GenerateDelegateExampleInstruction
        (
        )
            {
            DelegateInstruction di = new DelegateInstruction ();
            di.DocumentProcessorName = DocumentProcessorName;
            di.DocumentProcessorGuid = DocumentProcessorGuid.ToString ();
            di.Step = DgnV8MirrorICSPluginConstants.Steps.DelegateExample;
            return di;
            }
        }
    }
