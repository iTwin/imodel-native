/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityCrawler/WMSBoundingBox.cs $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

namespace Bentley.OGC.WMS
{
    public class WMSBoundingBox
    {
        public WMSBoundingBox(int xmin, int ymin, int xmax, int ymax)
        {
            XMin = xmin;
            YMin = ymin;
            XMax = xmax;
            YMax = ymax;
        }

        public int XMin { get; private set; }

        public int YMin { get; private set; }

        public int XMax { get; private set; }

        public int YMax { get; private set; }
    }
}