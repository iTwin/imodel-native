using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;

namespace IndexECPlugin.Source
    {
    /// <summary>
    /// XmlNode class extension methods
    /// </summary>
    public static class XmlNodeExtensions
        {
        /// <summary>
        /// Combines SelectSingleNode and InnerText while handling null values from SelectSingleNode.
        /// </summary>
        /// <param name="xmlNode">The xmlNode</param>
        /// <param name="path">The xpath of the node to select</param>
        /// <param name="xmlnsm">The namespace manager</param>
        /// <returns></returns>
        public static string TrySelectSingleNodeInnerText(this XmlNode xmlNode, string path, XmlNamespaceManager xmlnsm)
            {
            if ( xmlNode.SelectSingleNode(path, xmlnsm) != null)
                return xmlNode.SelectSingleNode(path, xmlnsm).InnerText;
            return null;
            }
        }
    }
