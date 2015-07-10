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

        #region Constructor
        /// <summary>
        /// Constructor of the seed creator that use the defaut seed file 
        /// name "CrawledSeed.xml" in the current application path and 
        /// that append the seed at the end of the file by default.
        /// </summary>
        public SeedCreator()
            : this(DEFAULT_APPEND_VALUE)
        {

        }

        /// <summary>
        /// Constructor of the seed creator that use the defaut seed file name "CrawledSeed.xml" in the current application path.
        /// </summary>
        /// <param name="append">True to append at the end of the file, False to overwrite.</param>
        public SeedCreator(Boolean append)
            : this(DEFAULT_SEED_FILE, append)
        {

        }

        /// <summary>
        /// Constructor of the seed creator that append the seed at the end of the file by default.
        /// </summary>
        /// <param name="xmlFilePath">Path of the seed file.</param>
        public SeedCreator(String xmlFilePath)
            : this(xmlFilePath, DEFAULT_APPEND_VALUE)
        {

        }

        /// <summary>
        /// Constructor of the seed creator.
        /// </summary>
        /// <param name="xmlFilePath">Path of the seed file.</param>
        /// <param name="append">True to append at the end of the file, False to overwrite.</param>
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
        #endregion

        #region Methods
        #region Public Methods
        /// <summary>
        /// Add a new seed to the seed list.
        /// </summary>
        /// <param name="seed">Uri of the seed.</param>
        public void Add(Uri seed)
        {
            if (!m_seedList.Contains(seed))
                m_seedList.Add(seed);
        }

        /// <summary>
        /// Save the new seed list.
        /// </summary>
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
                writer = XmlWriter.Create(m_XMLFilePath, settings);
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
