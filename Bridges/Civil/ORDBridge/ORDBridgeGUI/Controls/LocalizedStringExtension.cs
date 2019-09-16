/*--------------------------------------------------------------------------------------+
|
|     $Source: ORDBridge/ORDBridgeGUI/Controls/LocalizedStringExtension.cs $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System;
//using BentleyB0200.DgnClientFx.Net;

namespace ORDBridgeGUI.Controls

    {
    //=======================================================================================
    // A class that returns the localized string corresponding to a message ID. For usage
    // in the XAML use LocalizedStringExtension found in BentleyB0200.DgnClientFx.Net. 
    //
    // @bsiclass
    //=======================================================================================
    internal class LocalizedStringExtension
        {
        private static readonly string ORDBridgeCommonNamespace = "ORDBridge";

        //---------------------------------------------------------------------------------------
        // Utility method (for non-XAML callers). Returns the localized string corresponding to
        // the specified string ID.
        // @bsimethod                           Arun.George                             11/2018
        //---------------------------------------------------------------------------------------
        public static string GetString (string id)
            {
            if ( String.IsNullOrEmpty(id) )
                return "<No Message ID Specified>";

            //if ( DgnClientFxL10NNet.HasString(ORDBridgeCommonNamespace, id) )
            //    return DgnClientFxL10NNet.GetString(ORDBridgeCommonNamespace, id);

            // Just echo the message key with some brackets around it. That should tell the developer (or tester)
            // the string hasn't been localized.
            return String.Format("[[[{0}]]]", id);
            }
        }
    }
