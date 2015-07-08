/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityCrawler/WMSCapabilities.cs $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

using System;
using System.Collections.Generic;
using System.IO;

namespace Bentley.OGC.WMS
{    
    /// <summary>
    /// Structure containing all the contact information.
    /// </summary>
    public struct WMSContactInformation
        {
        /// <summary>
        /// Structure containing the primary contact person information.
        /// </summary>
        public struct WMSContactPersonPrimary
        {
        public String ContactPerson;
        public String ContactOrganization;
        };

        /// <summary>
        /// Structue containing the contact address
        /// </summary>
        public struct WMSContactAddress
        {
        public String AddressType;
        public String Address;
        public String City;
        public String StateOrProvince;
        public String PostCode;
        public String Country;
        };

        public WMSContactPersonPrimary ContactPersonPrimary;
        public String ContactPosition;
        public WMSContactAddress ContactAddress;
        public String ContactVoiceTelephone;
        public String ContactFacsimileTelephone;
        public String ContactElectronicMailAddress;
        };

    public class WMSCapabilities
    {
        public WMSCapabilities (Stream stream)
        {
            if (stream == null) throw new ArgumentNullException("stream");

            var parser = new WMSCapabilitiesParser(stream);

            Name = parser.GetName ();

            Title = parser.GetTitle ();

            Abstract = parser.GetAbstract ();

            ContactInformation = parser.GetContactInformation ();

            Fees = parser.GetFees ();

            AccessConstraints = parser.GetAccessConstraints ();

            GetCapabilitiesUrl = parser.GetCapabilitiesUrl();

            GetMapUrl = parser.GetMapUrl();

            GetLegendGraphicUrl = parser.GetLegendGraphicUrl();

            Version = parser.GetVersion();

            SupportedFormats = parser.GetSupportedFormats();

            RootLayer = parser.GetRootLayer();

            XML = parser.GetXML ();
        }

        public string Name { get; private set; }

        public string Title { get; private set; }

        public string Abstract { get; private set; }

        public WMSContactInformation ContactInformation { get; private set; }

        public string Fees { get; private set; }

        public string AccessConstraints { get; private set; }

        public string GetCapabilitiesUrl { get; private set; }

        public string GetMapUrl { get; private set; }

        public string GetLegendGraphicUrl { get; private set; }

        public string Version { get; private set; }

        public IEnumerable<String> SupportedFormats { get; private set; }

        public WMSLayer RootLayer { get; private set; }

        public string XML { get; private set; }
    }
}