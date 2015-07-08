/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityCrawler/WMSLayer.cs $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

using System;
using System.Collections.Generic;
using System.Xml;

namespace Bentley.OGC.WMS
{
    public class WMSLayer
    {
        private readonly WMSCapabilitiesParser m_parser;
        private Dictionary<string, WMSBoundingBox> m_boundingBoxes;
        private WMSBoundingBox m_latlonbbox;
        private string m_fullname;
        private List<WMSStyle> m_styles;
        private List<String> m_coordinateSystems;
        private XmlNode m_node;

        internal WMSLayer (string name, string title, WMSCapabilitiesParser parser, XmlNode node)
        {
            m_fullname = String.Empty;
            Name = name;
            Title = title;
            ParentLayer = null;
            ChildLayers = new List<WMSLayer>();
            m_boundingBoxes = null;
            m_latlonbbox = new WMSBoundingBox (0, 0, 0, 0);
            m_styles = null;
            m_coordinateSystems = null;
            m_node = node;
            m_parser = parser;
        }

        public string Fullname
        {
            get
            {
                if (String.IsNullOrEmpty(m_fullname))
                {
                    m_fullname = Title;

                    if (null != ParentLayer && !String.IsNullOrEmpty(ParentLayer.Fullname))
                        return ParentLayer.Fullname + "\\" + Title;
                }

                return m_fullname;
            }
        }

        public string Name { get; private set; }

        public string Title { get; private set; }

        public WMSLayer ParentLayer { get; internal set; }

        public List<WMSLayer> ChildLayers { get; private set; }

        public List<WMSStyle> Styles
        {
            get
            {
                if (null == m_styles)
                {
                m_styles = m_parser.GetLayerStyles (m_node);

                    if (null != ParentLayer)
                    {
                        m_styles.AddRange(ParentLayer.Styles);
                    }
                }

                return m_styles;
            }
        }

        // ToDo: BUG -> There is a major bug we aren't able to get the correct Node  if two Layer have the same Title or Name.
        public List<String> CoordinateSystems
            {
            get
                {
                if ( null == m_coordinateSystems )
                    {
                    m_coordinateSystems = m_parser.GetLayerCoordinateSystems (m_node);
                    // Add the coordinate systems from the parent layer.
                    if ( null != ParentLayer )
                        foreach (String cs in ParentLayer.CoordinateSystems)
                            {
                            if ( !m_coordinateSystems.Contains (cs) )
                                m_coordinateSystems.Add (cs);
                            }
                    }

                // Add the coordinate systems from the bounding box in case of no SRS/CRS node.
                Dictionary<string, WMSBoundingBox> boundingBoxes = BoundingBoxes;
                foreach ( string cs in boundingBoxes.Keys )
                    {
                    if ( !m_coordinateSystems.Contains (cs) )
                        m_coordinateSystems.Add (cs);
                    }
                return m_coordinateSystems;
                }
            }

        // ToDo: BUG -> There is a major bug we aren't able to get the correct Node  if two Layer have the same Title or Name.
        public Dictionary<string, WMSBoundingBox> BoundingBoxes
        {
            get
            {
                if (null == m_boundingBoxes)
                {
                m_boundingBoxes = m_parser.GetLayerBoundingBoxes (m_node);

                    if (null != ParentLayer)
                    {
                        Dictionary<string, WMSBoundingBox> parentBoundingBoxes = ParentLayer.BoundingBoxes;

                        foreach (string cs in parentBoundingBoxes.Keys)
                        {
                            if (!m_boundingBoxes.ContainsKey(cs))
                                m_boundingBoxes.Add(cs, parentBoundingBoxes[cs]);
                        }
                    }
                }

                return m_boundingBoxes;
            }
        }
	
        public WMSBoundingBox LatLonBoundingBox
            {
            get
                {
                if ( BoundingBoxes.ContainsKey ("EPSG:4326") )
                    m_latlonbbox = BoundingBoxes ["EPSG:4326"];

                return m_latlonbbox;
                }
            }
    }
}