/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityCrawler/WMSCapabilitiesParser.cs $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;
using System.Xml;

namespace Bentley.OGC.WMS
    {
    public class WMSCapabilitiesParser
        {
        private readonly XmlDocument m_xmlDocument;
        private readonly XmlNamespaceManager m_xmlNamespaceManager;

        public WMSCapabilitiesParser (Stream stream)
            {
            if ( stream == null )
                throw new ArgumentNullException ("stream");

            m_xmlDocument = new XmlDocument ();
            m_xmlDocument.XmlResolver = null;
            //ToDo: Manage error here.
            m_xmlDocument.Load (stream);
            m_xmlNamespaceManager = new XmlNamespaceManager (m_xmlDocument.NameTable);
            string xmlns = m_xmlDocument.DocumentElement.GetAttribute ("xmlns");
            m_xmlNamespaceManager.AddNamespace ("ns", xmlns);
            }

        public Boolean IsValidCapabilities ()
            {
            XmlNode root = m_xmlDocument.DocumentElement;
            if ( null != root.Attributes )
                {
                return true;
                }
            return false;
            }

        public string GetXML ()
            {            
            return m_xmlDocument.InnerXml;
            }

        public string GetName ()
            {
            return GetServiceChild ("Name");
            }

        public string GetTitle ()
            {
            return GetServiceChild ("Title");
            }

        public string GetAbstract ()
            {
            return GetServiceChild ("Abstract");
            }

        public WMSContactInformation GetContactInformation ()
            {
            WMSContactInformation getContactInformation;
            getContactInformation.ContactPersonPrimary.ContactPerson = GetContactPersonPrimaryChild ("ContactPerson");
            getContactInformation.ContactPersonPrimary.ContactOrganization = GetContactPersonPrimaryChild ("ContactOrganization");
            getContactInformation.ContactPosition = GetContactInformationChild ("ContactPosition");
            getContactInformation.ContactAddress.AddressType = GetContactAddressChild ("AddressType");
            getContactInformation.ContactAddress.Address = GetContactAddressChild ("Address");
            getContactInformation.ContactAddress.City = GetContactAddressChild ("City");
            getContactInformation.ContactAddress.StateOrProvince = GetContactAddressChild ("StateOrProvince");
            getContactInformation.ContactAddress.PostCode = GetContactAddressChild ("PostCode");
            getContactInformation.ContactAddress.Country = GetContactAddressChild ("Country");
            getContactInformation.ContactVoiceTelephone = GetContactInformationChild ("ContactVoiceTelephone");
            getContactInformation.ContactFacsimileTelephone = GetContactInformationChild ("ContactFacsimileTelephone");
            getContactInformation.ContactElectronicMailAddress = GetContactInformationChild ("ContactElectronicMailAddress");
            return getContactInformation;
            }

        public string GetContactInformationChild (String nodeName)
            {
            XmlNode root = m_xmlDocument.DocumentElement;
            XmlNode getContactInformation = root.SelectSingleNode ("ns:Service", m_xmlNamespaceManager).SelectSingleNode ("ns:ContactInformation/ns:" + nodeName, m_xmlNamespaceManager);
            return null != getContactInformation ? getContactInformation.InnerText : String.Empty;
            }

        public string GetContactPersonPrimaryChild (String nodeName)
            {
            XmlNode root = m_xmlDocument.DocumentElement;
            XmlNode getContactPersonPrimary = root.SelectSingleNode ("ns:Service", m_xmlNamespaceManager).SelectSingleNode ("ns:ContactInformation/ns:ContactPersonPrimary/ns:" + nodeName, m_xmlNamespaceManager);
            return null != getContactPersonPrimary ? getContactPersonPrimary.InnerText : String.Empty;
            }

        public string GetContactAddressChild (String nodeName)
            {
            XmlNode root = m_xmlDocument.DocumentElement;
            XmlNode getContactAddress = root.SelectSingleNode ("ns:Service", m_xmlNamespaceManager).SelectSingleNode ("ns:ContactInformation/ns:ContactAddress/ns:" + nodeName, m_xmlNamespaceManager);
            return null != getContactAddress ? getContactAddress.InnerText : String.Empty;
            }

        public string GetFees ()
            {
            return GetServiceChild ("Fees");
            }

        public string GetServiceChild (String nodeName)
            {
            XmlNode root = m_xmlDocument.DocumentElement;
            XmlNode getServiceChild = root.SelectSingleNode ("ns:Service", m_xmlNamespaceManager).SelectSingleNode ("ns:" + nodeName, m_xmlNamespaceManager);
            return null != getServiceChild ? getServiceChild.InnerText : String.Empty;
            }

        public string GetAccessConstraints ()
            {
            XmlNode root = m_xmlDocument.DocumentElement;
            XmlNode getAccessConstraints = root.SelectSingleNode ("ns:Service", m_xmlNamespaceManager).SelectSingleNode ("ns:AccessConstraints", m_xmlNamespaceManager);
            return null != getAccessConstraints ? getAccessConstraints.InnerText : String.Empty;
            }

        public string GetVersion ()
            {
            XmlNode root = m_xmlDocument.DocumentElement;
            if ( null != root.Attributes )
                {
                foreach ( XmlAttribute attribute in root.Attributes )
                    {
                    if ( String.Compare (attribute.Name, "version", false) == 0 )
                        return attribute.Value;
                    }
                }

            return String.Empty;
            }

        public string GetCapabilitiesUrl ()
            {
            XmlNode root = m_xmlDocument.DocumentElement;
            XmlNode getMap = root.SelectSingleNode ("ns:Capability", m_xmlNamespaceManager).
                SelectSingleNode ("ns:Request", m_xmlNamespaceManager).
                SelectSingleNode ("ns:GetCapabilities", m_xmlNamespaceManager).
                SelectSingleNode ("ns:DCPType", m_xmlNamespaceManager).
                SelectSingleNode ("ns:HTTP", m_xmlNamespaceManager).
                SelectSingleNode ("ns:Get", m_xmlNamespaceManager).
                SelectSingleNode ("ns:OnlineResource", m_xmlNamespaceManager);

            return getMap.Attributes["xlink:href"].Value;
            }


        public string GetMapUrl ()
            {
            XmlNode root = m_xmlDocument.DocumentElement;
            XmlNode getMap = root.SelectSingleNode ("ns:Capability", m_xmlNamespaceManager).
                SelectSingleNode ("ns:Request", m_xmlNamespaceManager).
                SelectSingleNode ("ns:GetMap", m_xmlNamespaceManager).
                SelectSingleNode ("ns:DCPType", m_xmlNamespaceManager).
                SelectSingleNode ("ns:HTTP", m_xmlNamespaceManager).
                SelectSingleNode ("ns:Get", m_xmlNamespaceManager).
                SelectSingleNode ("ns:OnlineResource", m_xmlNamespaceManager);

            return getMap.Attributes["xlink:href"].Value;
            }

        public string GetLegendGraphicUrl ()
            {
            // See if we have a <Capability><Request><GetLegendGraphic> node - if not return an empty string
            XmlNode root = m_xmlDocument.DocumentElement;
            XmlNode legendGraphicNode = root.SelectSingleNode ("ns:Capability", m_xmlNamespaceManager).
                SelectSingleNode ("ns:Request", m_xmlNamespaceManager).
                SelectSingleNode ("ns:GetLegendGraphic", m_xmlNamespaceManager);

            if ( legendGraphicNode == null )
                {
                return string.Empty;
                }

            XmlNode onlineResourceNode = legendGraphicNode.SelectSingleNode ("ns:DCPType", m_xmlNamespaceManager).
                SelectSingleNode ("ns:HTTP", m_xmlNamespaceManager).
                SelectSingleNode ("ns:Get", m_xmlNamespaceManager).
                SelectSingleNode ("ns:OnlineResource", m_xmlNamespaceManager);

            return onlineResourceNode.Attributes["xlink:href"].Value;
            }

        public IEnumerable<string> GetSupportedFormats ()
            {
            XmlNode root = m_xmlDocument.DocumentElement;
            XmlNodeList formats = root.SelectSingleNode ("ns:Capability", m_xmlNamespaceManager).
                SelectSingleNode ("ns:Request", m_xmlNamespaceManager).
                SelectSingleNode ("ns:GetMap", m_xmlNamespaceManager).
                SelectNodes ("ns:Format", m_xmlNamespaceManager);

            var formatList = new List<string> (formats.Count);

            foreach ( XmlNode format in formats )
                {
                formatList.Add (format.InnerText);
                }

            return formatList;
            }

        private string GetLayerName (XmlNode node)
            {
            XmlNode nameNode = node.SelectSingleNode ("ns:Name", m_xmlNamespaceManager);

            return null != nameNode ? nameNode.InnerText : String.Empty;
            }

        private string GetLayerTitle (XmlNode node)
            {
            XmlNode titleNode = node.SelectSingleNode ("ns:Title", m_xmlNamespaceManager);

            return null != titleNode ? titleNode.InnerText : String.Empty;
            }

        private XmlNode GetCapabilityNode ()
            {
            return m_xmlDocument.DocumentElement.SelectSingleNode ("ns:Capability", m_xmlNamespaceManager);
            }

        internal List<string> GetLayerCoordinateSystems (WMSLayer layer)
            {
            return GetLayerCoordinateSystems (GetLayerNode (layer));
            }

        internal List<string> GetLayerCoordinateSystems (XmlNode xmlNode)
            {
            var coordinatesystems = new List<String> ();
            if ( null != xmlNode )
                {
                string crsString = "CRS";

                if ( GetVersion ().StartsWith ("1.1.") )
                    crsString = "SRS";

                XmlNodeList nodeList = xmlNode.SelectNodes ("ns:" + crsString, m_xmlNamespaceManager);
                foreach ( XmlNode csNode in nodeList )
                    {
                    string csName = csNode.InnerText.ToUpper();
                    // Make sure that no coordinate system instance that shares the same name exists
                    if ( !coordinatesystems.Contains(csName))
                       coordinatesystems.Add (csName);
                    }
                }
            return coordinatesystems;
            }

        internal List<WMSStyle> GetLayerStyles (WMSLayer layer)
            {
            return GetLayerStyles (GetLayerNode (layer));
            }

        internal List<WMSStyle> GetLayerStyles (XmlNode xmlNode)
            {
            var styles = new List<WMSStyle> ();

            if ( null != xmlNode )
                {
                XmlNodeList nodeList = xmlNode.SelectNodes ("ns:Style", m_xmlNamespaceManager);

                foreach ( XmlNode styleNode in nodeList )
                    {
                    string name = styleNode.SelectSingleNode ("ns:Name", m_xmlNamespaceManager).InnerText;
                    string title = styleNode.SelectSingleNode ("ns:Title", m_xmlNamespaceManager).InnerText;

                    // Make sure that no style instance that shares the same name exists
                    if (
                        styles.Count (style => string.Compare (name, style.Name, StringComparison.InvariantCulture) == 0) ==
                        0 )
                        {
                        styles.Add (new WMSStyle (name, title));
                        }
                    }
                }

            return styles;
            }

        // ToDo: BUG -> There is a major bug we aren't able to get the correct Node  if two Layer have the same Title or Name.
        private XmlNode GetLayerNode (WMSLayer layer)
            {
            XmlNode capability = GetCapabilityNode ();

            string xPath;
            if ( String.IsNullOrEmpty (layer.Name) )
                xPath = String.Format ("descendant::ns:Layer[ns:Title={0}]", FormatXPathString (layer.Title));
            else
                xPath = String.Format ("descendant::ns:Layer[ns:Name={0}]", FormatXPathString (layer.Name));

            XmlNode node;

            try
                {
                node = capability.SelectSingleNode (xPath, m_xmlNamespaceManager);
                }
            catch ( Exception )
                {
                return null;
                }

            return node;
            }

        /// <summary>
        /// This method takes a string used as an attribute of and XPath query and handles quotation marks as neccesary.
        /// </summary>
        /// <param name="p"></param>
        /// <returns>
        /// 
        /// </returns>
        private string FormatXPathString (string value)
            {
            string[] splittedValue = value.Split ('\"');

            if ( splittedValue.Length == 1 )
                return String.Format ("\"{0}\"", value);

            // A double quote wrapped between single quotes is correctly interpreted
            return String.Format ("concat(\"{0}\")", String.Join ("\", '\"', \"", splittedValue));
            }

        internal Dictionary<string, WMSBoundingBox> GetLayerBoundingBoxes (WMSLayer layer)
            {
            return GetLayerBoundingBoxes(GetLayerNode(layer));
            }

        internal Dictionary<string, WMSBoundingBox> GetLayerBoundingBoxes (XmlNode node)
            {
            var boundingBoxes = new Dictionary<string, WMSBoundingBox> ();

            if ( null != node && null != boundingBoxes )
                {
                double xMin, yMin, xMax, yMax;
                string crsString = "CRS";

                if ( GetVersion ().StartsWith ("1.1.") )
                    crsString = "SRS";

                // Get the explicit bounding box specified for this layer.
                XmlNodeList boundingBoxesNodes = node.SelectNodes ("ns:BoundingBox", m_xmlNamespaceManager);
                foreach ( XmlNode boundingBoxNode in boundingBoxesNodes )
                    {
                    if ( TryParseBoundingBoxValue (boundingBoxNode, "minx", out xMin) &&
                        TryParseBoundingBoxValue (boundingBoxNode, "miny", out yMin) &&
                        TryParseBoundingBoxValue (boundingBoxNode, "maxx", out xMax) &&
                        TryParseBoundingBoxValue (boundingBoxNode, "maxy", out yMax) )
                        {

                        // Patch to correct malformed WMS capabilities that didn't respect the 
                        // crsString corresponding to the version of the schema.
                        if ( boundingBoxNode.Attributes[crsString] == null )
                            crsString = crsString.Equals ("CRS") ? "SRS" : "CRS";
                        
                        string key = boundingBoxNode.Attributes[crsString].Value;
                        var value = new WMSBoundingBox ((int) xMin, (int) yMin, (int) xMax, (int) yMax);

                        //boundingBoxes.Add(key, value);
                        // Correction to avoid duplicate key
                        boundingBoxes[key] = value;
                        }
                    }

                // In WMS 1.1.1, bounding box for SRS=EPSG:4326 can be expressed in the <LatLonBoundingBox> tag.
                XmlNodeList latLonBoudingBoxesNodes = node.SelectNodes ("ns:LatLonBoundingBox", m_xmlNamespaceManager);
                foreach ( XmlNode latLonBoudingBoxNode in latLonBoudingBoxesNodes )
                    {
                    if ( TryParseBoundingBoxValue (latLonBoudingBoxNode, "minx", out xMin) &&
                        TryParseBoundingBoxValue (latLonBoudingBoxNode, "miny", out yMin) &&
                        TryParseBoundingBoxValue (latLonBoudingBoxNode, "maxx", out xMax) &&
                        TryParseBoundingBoxValue (latLonBoudingBoxNode, "maxy", out yMax) )
                        {
                        var value = new WMSBoundingBox ((int) xMin, (int) yMin, (int) xMax, (int) yMax);

                        // EPSG:4326 is explicit                       
                        //boundingBoxes.Add("EPSG:4326", value);
                        // Correction to avoid duplicate key
                        boundingBoxes["EPSG:4326"] = value;
                        }
                    }

                // In WMS 1.3, bounding box for CRS=EPSG:4326 can be expressed in the <EX_GeographicBoundingBox> tag.
                XmlNodeList EX_GeographicBoundingBoxNodes = node.SelectNodes ("ns:EX_GeographicBoundingBox", m_xmlNamespaceManager);
                foreach ( XmlNode latLonBoudingBoxNode in EX_GeographicBoundingBoxNodes )
                    {
                    if ( TryParseEX_GeographicBoundingBoxValue (latLonBoudingBoxNode, "westBoundLongitude", out xMin) &&
                        TryParseEX_GeographicBoundingBoxValue (latLonBoudingBoxNode, "southBoundLatitude", out yMin) &&
                        TryParseEX_GeographicBoundingBoxValue (latLonBoudingBoxNode, "eastBoundLongitude", out xMax) &&
                        TryParseEX_GeographicBoundingBoxValue (latLonBoudingBoxNode, "northBoundLatitude", out yMax) )
                        {
                        var value = new WMSBoundingBox ((int) xMin, (int) yMin, (int) xMax, (int) yMax);

                        // EPSG:4326 is explicit                       
                        //boundingBoxes.Add("EPSG:4326", value);
                        // Correction to avoid duplicate key
                        boundingBoxes["EPSG:4326"] = value;
                        }
                    }
                }

            return boundingBoxes;
            }

        private bool TryParseBoundingBoxValue (XmlNode node, string attributeName, out double result)
            {
            return Double.TryParse (node.Attributes[attributeName].Value, NumberStyles.Float,
                                   CultureInfo.InvariantCulture, out result);
            }

        private bool TryParseEX_GeographicBoundingBoxValue (XmlNode node, string nodeName, out double result)
            {
            return Double.TryParse (node.SelectSingleNode ("ns:" + nodeName, m_xmlNamespaceManager).InnerText, NumberStyles.Float,
                                   CultureInfo.InvariantCulture, out result);
            }

        public WMSLayer GetRootLayer ()
            {
            WMSLayer rootLayer = null;

            XmlNode capability = GetCapabilityNode ();
            XmlNode mainLayerNode = capability.SelectSingleNode ("ns:Layer", m_xmlNamespaceManager);

            if ( null != mainLayerNode )
                {
                rootLayer = CreateLayer (mainLayerNode);
                var layersStack = new Stack<KeyValuePair<WMSLayer, XmlNode>> ();
                layersStack.Push (new KeyValuePair<WMSLayer, XmlNode> (rootLayer, mainLayerNode));

                while ( layersStack.Count > 0 )
                    {
                    KeyValuePair<WMSLayer, XmlNode> layerNodeMapping = layersStack.Pop ();

                    WMSLayer currentLayer = layerNodeMapping.Key;

                    XmlNodeList nodes = layerNodeMapping.Value.SelectNodes ("child::ns:Layer", m_xmlNamespaceManager);

                    foreach ( XmlNode node in nodes )
                        {
                        WMSLayer layer = CreateLayer (node);
                        layer.ParentLayer = currentLayer;
                        currentLayer.ChildLayers.Add (layer);

                        layersStack.Push (new KeyValuePair<WMSLayer, XmlNode> (layer, node));
                        }
                    }
                }

            return rootLayer;
            }

        private WMSLayer CreateLayer (XmlNode node)
            {
            //var regex = new Regex("eBLayer:(?<ClassTypeId>\\d+):(?<ClassId>\\d+):(?<AttributeId>\\d+)");
            string layerName = GetLayerName (node);

            //Match match = regex.Match(layerName);

            //if (match.Success)
            //{
            //    var eBLayer = new WMSeBLayer(layerName, GetLayerTitle(node), this);
            //    eBLayer.ClassTypeId = int.Parse(match.Groups["ClassTypeId"].Value);
            //    eBLayer.ClassId = int.Parse(match.Groups["ClassId"].Value);
            //    eBLayer.AttributeId = int.Parse(match.Groups["AttributeId"].Value);

            //    return eBLayer;
            //}

            return new WMSLayer (layerName, GetLayerTitle (node), this, node);
            }
        }
    }