/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityCrawler/WMSStyle.cs $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

namespace Bentley.OGC.WMS
{
    /// <summary>
    /// Class that encapsulates a WMS style definition.
    /// </summary>
    public class WMSStyle
    {
        /// <summary>
        /// Instantiates a WMSStyle instance.
        /// </summary>
        /// <param name="name">The style instance's name.</param>
        /// <param name="title">The style instance's title.</param>
        public WMSStyle(string name, string title)
        {
            Name = name;
            Title = title;
        }

        /// <summary>
        /// The style's name.
        /// </summary>
        public string Name { get; private set; }

        /// <summary>
        /// The style's title.
        /// </summary>
        public string Title { get; private set; }
    }
}