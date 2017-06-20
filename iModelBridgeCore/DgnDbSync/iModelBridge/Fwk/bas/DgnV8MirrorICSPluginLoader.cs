/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/bas/DgnV8MirrorICSPluginLoader.cs $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System;
using Bentley.Orchestration.Extensibility;

namespace BentleyB0200.Dgn.DgnV8Mirror.ICS
    {
    /// *****************************************************************************
    /// <summary>
    /// Loads the extension modules for DgnV8MirrorICSPlugin
    /// </summary>
    /// @author                                             AnthonyFalcone 02/09
    /// *****************************************************************************
    public class DgnV8MirrorICSPluginLoader: IExtensionLoader
        {
        /// *****************************************************************************
        /// <summary>
        /// Tells the Orchestration Framework which extension modules to load.
        /// </summary>
        /// @author                                             AnthonyFalcone 02/09
        /// *****************************************************************************
        public void Initialize ()
            {
            ExtensionsDirectory.RegisterExtensionObject
                (new Guid ("5ECDAB1C-7E8C-47F2-BD88-907D9BA0EBE7"), new JobDefinitionExtension ());
            ExtensionsDirectory.RegisterExtensionObject
                (new Guid ("17659021-0C63-4A0E-BFAD-2E08A44EAD15"), new SmartDispatcherExtension ());
            ExtensionsDirectory.RegisterExtensionObject
                (new Guid ("AD4CAAA8-EB94-4E3D-B070-A17B0CB1568C"), new DelegateProcessorExtension ());
            ExtensionsDirectory.RegisterExtensionObject
                (new Guid ("90A24A37-B042-4280-B644-8BBF544CDB5C"), new JobCompleteExtension ());
            }
        }
    }
