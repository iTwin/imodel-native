/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/bas/Configurator.cs $
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
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using Bentley.Automation.Configuration;
using Bentley.Automation.Extension.Configurator;
using Bentley.Orchestration.MSProcessor;

namespace BentleyB0200.Dgn.DgnV8Mirror.ICS
    {
    public class Configurator : ASConfigurator
        {
        /// *****************************************************************************
        /// <summary>
        /// Constructor
        /// </summary>
        /// @author                                             AnthonyFalcone 11/12
        /// *****************************************************************************
        public Configurator ()
            : base (DgnV8MirrorICSPluginConstants.DocumentProcessorName,
                   new Guid(DgnV8MirrorICSPluginConstants.DocumentProcessorGuid),
                   DgnV8MirrorICSPluginConstants.DocumentProcessorDescription)
            {
            }

        public override bool DoesPlatformMeetRequirements (ref string errorMessage)
            {
            return true;
            }

        public override bool ShouldLaunchLicenseWizard
            {
            get
                {
                return false;
                }
            }
        public override bool OnAfterPluginCopiedToExtDir (string extDir)
            {
            return true;
            }

        public override bool OnAfterPluginDeletedFromExtDir (string extDir)
            {
            return true;
            }
        }
    }
