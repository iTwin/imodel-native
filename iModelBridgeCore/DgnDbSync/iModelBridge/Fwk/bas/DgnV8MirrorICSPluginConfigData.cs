/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/bas/DgnV8MirrorICSPluginConfigData.cs $
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
using System.Xml;
using System.Xml.Serialization;
using System.Xml.XPath;
using System.IO;
using System.Collections;

namespace BentleyB0200.Dgn.DgnV8Mirror.ICS
{
    /// <summary>
    /// A friendly class interface to the DgnV8MirrorICSPlugin configuration data.
    /// </summary>
    public class DgnV8MirrorICSPluginConfigData
    {
        public string Version = new Version (1, 0, 0, 0).ToString ();
    }
}
