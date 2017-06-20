/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/bas/Util.cs $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Serialization;
using Bentley.Automation.JobConfiguration;
using Bentley.Automation.Messaging;

namespace BentleyB0200.Dgn.DgnV8Mirror.ICS
    {
    /// <summary>
    /// Utility class
    /// </summary>
    internal class Util
        {
        /// <summary>
        /// Gets job definition parameters from context object
        /// </summary>
        /// <param name="asContext">context object</param>
        /// <returns>Job definition object with parameters defined</returns>
        internal static DgnV8MirrorICSPluginConfigData GetDgnV8MirrorICSPluginConfigData
        (
        ASContext asContext
        )
            {
            return GetDgnV8MirrorICSPluginConfigData (asContext.JobDefinition);
            }

        /// <summary>
        /// Gets job definition parameters from job definition
        /// </summary>
        /// <param name="jd">job definition</param>
        /// <returns>Job definition object with parameters defined</returns>
        internal static DgnV8MirrorICSPluginConfigData GetDgnV8MirrorICSPluginConfigData
        (
        JobDefinition jd
        )
            {
            XmlNode xmlNode = (XmlNode)jd.GetCustomData (new Guid (DgnV8MirrorICSPluginConstants.DocumentProcessorGuid));
            if (xmlNode != null)
                return GetDgnV8MirrorICSPluginConfigData ((XmlElement)xmlNode);

            return null;
            }

        /// <summary>
        /// Gets job definition parameters from job definition
        /// </summary>
        /// <param name="myDocProcConfigDataAsXmlElement">XmlElement stored in DB</param>
        /// <returns>Job definition object with parameters defined</returns>
        internal static DgnV8MirrorICSPluginConfigData GetDgnV8MirrorICSPluginConfigData 
        (
        XmlElement myDocProcConfigDataAsXmlElement
        )
            {
            return DeserializeFromXmlElement<DgnV8MirrorICSPluginConfigData> (myDocProcConfigDataAsXmlElement);
            }

        /// <summary>
        /// Serializes job definition object with parameters defined to XmlElement
        /// </summary>
        /// <param name="myDocProcConfigData">Job definition object with parameters defined</param>
        /// <returns>XmlElement representation of job definition object</returns>
        internal static XmlElement DgnV8MirrorICSPluginConfigData2XmlElement 
        (
        DgnV8MirrorICSPluginConfigData myDocProcConfigData
        )
            {
            return SerializeToXmlElement (myDocProcConfigData);
            }

        /// <summary>
        /// Determines PDF file from source file
        /// </summary>
        /// <param name="srcFile">source file</param>
        /// <returns>PDF file</returns>
        internal static string GetPDFFileFromSrcFile
        (
        string srcFile
        )
            {
            return Path.Combine (Path.GetDirectoryName (srcFile), Path.GetFileNameWithoutExtension (srcFile) + ".pdf");
            }


        /// <summary>
        /// Serializes object to XmlElement
        /// </summary>
        /// <param name="o">object</param>
        /// <returns>XmlElement</returns>
        internal static XmlElement SerializeToXmlElement 
        (
        object o
        )
            {
            XmlDocument doc = new XmlDocument ();

            using (XmlWriter writer = doc.CreateNavigator ().AppendChild ())
                {
                new XmlSerializer (o.GetType ()).Serialize (writer, o);
                }

            return doc.DocumentElement;
            }

        /// <summary>
        ///  Deserializes XmlElement to object
        /// </summary>
        /// <typeparam name="T">object type</typeparam>
        /// <param name="element">XmlElement</param>
        /// <returns>object</returns>
        internal static T DeserializeFromXmlElement<T> 
        (
        XmlElement element
        )
            {
            var serializer = new XmlSerializer (typeof (T));

            return (T)serializer.Deserialize (new XmlNodeReader (element));
            }
        }
    }
