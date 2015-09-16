/*--------------------------------------------------------------------------------------+
|
|     $Source: WMSCapabilitiesNET/TestWrapper/Program.cs $
| 
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Net;
using System.IO;

using RealityPlatformWMSCapabilities;


namespace TestWrapper
    {
    class Program
        {
        static void Main (string[] args)
            {
            Console.WriteLine ("TestWrapper");
            //Uri wmsURI = new Uri (@"http://mrdata.usgs.gov/services/active-mines?SERVICE=WMS&REQUEST=GetCapabilities");
            Uri wmsURI = new Uri (@"http://kartta.liikennevirasto.fi/meriliikenne/dgds/wms_ip/merikartta\?SERVICE=WMS&REQUEST=GetCapabilities");
            WebRequest request = WebRequest.Create (wmsURI);
            WebResponse response = request.GetResponse ();
            //Get the Response Stream from the URL 
            Stream responseStream = response.GetResponseStream ();
            Encoding encode = Encoding.GetEncoding ("utf-8");
            StreamReader readStream = new StreamReader (responseStream, encode);
            String strContent = readStream.ReadToEnd ();
            WMSCapabilitiesNet capabilities = new WMSCapabilitiesNet (strContent);
            Console.WriteLine ("Version: {0}", capabilities.GetVersion());
            Console.WriteLine ("{0}", strContent);
            Console.ReadKey ();
            }
        }
    }
