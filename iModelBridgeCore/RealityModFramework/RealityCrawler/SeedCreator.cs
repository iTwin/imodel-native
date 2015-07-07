using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;

namespace Bentley.RealityPlatform.RealityCrawler
{
    class SeedCreator
    {
        #region Constants
        // Constants
        protected const String DEFAULT_SEED_FILE = "CrawledSeed.xml";
        protected const Boolean DEFAULT_APPEND_VALUE = true;
        protected const String SEED_NODE_NAME = "Seed";
        protected const String SEED_NODE_ROOT = "Seeds";
        #endregion

        #region Protected Members
        protected List<Uri> m_seedList = new List<Uri>();
        protected String m_XMLFilePath;
        protected Boolean m_Append;
        #endregion

        public SeedCreator()
            : this(DEFAULT_APPEND_VALUE)
        {

        }

        public SeedCreator(Boolean append)
            : this(DEFAULT_SEED_FILE, append)
        {

        }

        public SeedCreator(String xmlFilePath)
            : this(xmlFilePath, DEFAULT_APPEND_VALUE)
        {

        }

        public SeedCreator(String xmlFilePath, Boolean append)
        {
            m_XMLFilePath = xmlFilePath;
            m_Append = append;
            if (append) 
            {
                try 
                { 
                    SeedReader seeds = new SeedReader(DEFAULT_SEED_FILE);
                    foreach (Seed seed in seeds.seedList)
                    {
                        m_seedList.Add(seed.uri);
                    }
                }
                catch (Exception e)
                {
                    Console.WriteLine("{0} Exception caught.", e);
                }
            }
        }

        #region Methods
        #region Public Methods
        public void Add(Uri seed)
        {
            if (!m_seedList.Contains(seed))
                m_seedList.Add(seed);
        }

        public void Write() 
        {
            XmlWriter writer = null;

            try
            {
                // Create an XmlWriterSettings object with the correct options. 
                XmlWriterSettings settings = new XmlWriterSettings();
                settings.Indent = true;
                settings.IndentChars = ("\t");
                settings.OmitXmlDeclaration = false;

                //XmlTextWriter writer = new XmlTextWriter(m_XMLFilePath, System.Text.Encoding.UTF8);
                writer = XmlWriter.Create(m_XMLFilePath, settings);
                //writer.WriteStartDocument(true);
                //writer.Formatting = Formatting.Indented;
                //writer.Indentation = 2;
                writer.WriteStartElement(SEED_NODE_ROOT);
                foreach (Uri seed in m_seedList)
                {
                    writer.WriteStartElement(SEED_NODE_NAME);
                    writer.WriteCData(seed.AbsoluteUri);
                    writer.WriteEndElement();
                }
                writer.WriteEndElement();

                writer.Flush();

                if (writer != null)
                    writer.Close();
            }
            catch(Exception)
            {
                
                if (writer != null)
                {
                    writer.Flush();
                    writer.Close();
                }
            }
            finally
            {
                if (writer != null)
                {
                    writer.Flush();
                    writer.Close();
                } 
            }
        }

        #endregion
        #endregion
    }
}
