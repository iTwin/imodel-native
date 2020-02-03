/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/ImageDebugger/TypedefXmlReader.cs $
|    $RCSfile: TypedefXmlReader.cs, $
|   $Revision: 1 $
|       $Date: 2013/08/22 $
|     $Author: Julien Rossignol $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Reflection;
using System.IO;

namespace Bentley.ImageViewer
{

    /// <summary>
    /// Class used to read the typedef.xml file, which is used by the developper to tell the application where to find the required information to display the bitmap
    /// </summary>
    public struct XmlTypedefAttribute
    {
        public List<Version> version;
    }
    public struct Version
    {
        public string VersionName;
        public string BufferSize;
        public string Width;
        public string Height;
        public string Buffer;
        public string Palette;
        public string PaletteSize;
        public string PixelType;
        public string FixedPixelType;
        public int FixedPaletteSize;
        public string DefaultPixelType;
    }

    public class TypedefXmlReader
    {
        //Contains every typedef in the typedef.xml file, the uint key is specified as an attribute of the typedef and his given by the debugger to the app it allow easy identification of the type being debug
        private Dictionary<uint , XmlTypedefAttribute> m_XmlTypedefDictionary;

        //Read typedef.xml file
        public TypedefXmlReader()
        {
            m_XmlTypedefDictionary = new Dictionary<uint,XmlTypedefAttribute>();

            string codeBase = Assembly.GetExecutingAssembly().CodeBase;
            UriBuilder builder = new UriBuilder(codeBase);
            string path = Uri.UnescapeDataString(builder.Path);
            using( XmlReader reader = XmlReader.Create(Path.GetDirectoryName(path) + "/Typedef.xml"))
            {
                while(reader.ReadToFollowing("Type"))
                {
                    uint id = Convert.ToUInt32(reader.GetAttribute(0)); //read id attribute
                    XmlTypedefAttribute attribute = new XmlTypedefAttribute();
                    attribute.version = new List<Version>();

                    XmlReader subtreeReader = reader.ReadSubtree();
                    while( subtreeReader.ReadToFollowing("Version") )
                    {
                        Version version = new Version();

                        version.VersionName = reader.GetAttribute(0);

                        XmlReader attributeReader = subtreeReader.ReadSubtree();
                        while( attributeReader.ReadToFollowing("Attribute") )
                        {
                            switch( reader.GetAttribute(0) )//read attribute name
                            {
                                case "BufferSize":
                                    version.BufferSize = reader.ReadElementContentAsString();
                                    break;
                                case "Width":
                                    version.Width = reader.ReadElementContentAsString();
                                    break;
                                case "Height":
                                    version.Height = reader.ReadElementContentAsString();
                                    break;
                                case "Buffer":
                                    version.Buffer = reader.ReadElementContentAsString();
                                    break;
                                case "Palette":
                                    version.Palette = reader.ReadElementContentAsString();
                                    break;
                                case "PaletteSize":
                                    version.PaletteSize = reader.ReadElementContentAsString();
                                    break;
                                case "PixelType":
                                    version.PixelType = reader.ReadElementContentAsString();
                                    break;
                                case "FixedPixelType":
                                    version.FixedPixelType = reader.ReadElementContentAsString();
                                    break;
                                case "FixedPaletteSize":
                                    version.FixedPaletteSize = reader.ReadElementContentAsInt();
                                    break;
                                case "DefaultPixelType":
                                    version.DefaultPixelType = reader.ReadElementContentAsString();
                                    break;
                                default:
                                    break;
                            }
                        }
                        attribute.version.Add(version);
                    }
                    m_XmlTypedefDictionary.Add(id , attribute);
                }
            }
        }
        //return all informations read about a specified type
        public XmlTypedefAttribute GetTypedef(uint type)
        {
            return m_XmlTypedefDictionary[type];
        }

    }
}
