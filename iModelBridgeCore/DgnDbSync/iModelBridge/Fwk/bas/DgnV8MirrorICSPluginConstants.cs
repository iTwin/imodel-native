/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/bas/DgnV8MirrorICSPluginConstants.cs $
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
using System.Resources;

namespace BentleyB0200.Dgn.DgnV8Mirror.ICS
{
    /// <summary>
    /// Constants for DgnV8MirrorICSPluginConstants.
    /// </summary>
    public abstract class DgnV8MirrorICSPluginConstants
    {
        // Open resource file for accessing translated strings.
        public static ResourceManager Resources = new ResourceManager (
            "BentleyB0200.Dgn.DgnV8Mirror.ICS.Resource1",
            System.Reflection.Assembly.GetAssembly (typeof (DgnV8MirrorICSPluginConstants)));

        public static readonly string DocumentProcessorName =
            DgnV8MirrorICSPluginConstants.Resources.GetString ("DocumentProcessorName");
        public static readonly string DocumentProcessorDescription =
            DgnV8MirrorICSPluginConstants.Resources.GetString ("DocumentProcessorDescription");
        public static readonly string DocumentProcessorGuid =
            "5FE79A47-6802-4E48-9147-FF8142A3924A";

        internal static string LoggingNamespace = DocumentProcessorName.Replace(' ', '_'); 
        
        public abstract class Steps
        {
            public static readonly string PowerPlatformExample
                    = Resources.GetString ("PowerPlatformExample");
            public static readonly string DelegateExample
                    = Resources.GetString ("iModelHubSync");
        }

    }

}
