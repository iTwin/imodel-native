using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;

namespace Bentley.RealityPlatform.RealityCrawler
    {
    class SeedReader
        {
        #region Constants
        // Constants
        protected const String DEFAULT_SEED_FILE = "RealityCrawlerSeed.xml";
        protected const String SEED_NODE_NAME = "Seed";
        protected const String SEED_ATTRIBUTE_MIN = "RangeMin";
        protected const String SEED_ATTRIBUTE_MAX = "RangeMax";
        protected const String SEED_ATTRIBUTE_INC = "Increment";
        protected const String SEED_ATTRIBUTE_RIGHT = "RightPart";
        protected const String SEED_ATTRIBUTE_EXTERNALCRAWLING = "IsExternalPageCrawlingEnabled";
        #endregion

        #region Protected Members
        protected List<Seed> m_seedList = new List<Seed> ();
        #endregion

        #region Constructor
        /// <summary>
        /// Constructor of the seedReder object that initialize it with the seed contained 
        /// in the dfault XML file "RealityCrawlerSeed.xml" located in the same directory as the project.
        /// </summary>
        public SeedReader ()
            : this (DEFAULT_SEED_FILE)
            {
            }

        /// <summary>
        /// Constructor of the seedReder object that initialize it with the seed contained in the provided XML file.
        /// </summary>
        /// <param name="xmlFilePath">String path of the XML seed file to read.</param>
        public SeedReader (String xmlFilePath)
            {
            try
                {
                XmlTextReader xmlSeedReader = new XmlTextReader (xmlFilePath);
                if ( xmlSeedReader.ReadToDescendant (SEED_NODE_NAME) )
                    {
                    do
                        {
                        Boolean isExternalPageCrawlingEnabled = false;
                        Boolean useSeedRange = false;
                        if ( xmlSeedReader.HasAttributes )
                            {
                            // Set the isExternalPageCrawlingEnabled value (default = false).
                            String seedAttribute = xmlSeedReader.GetAttribute (SEED_ATTRIBUTE_EXTERNALCRAWLING);
                            if ( !String.IsNullOrWhiteSpace (seedAttribute) )
                                {
                                isExternalPageCrawlingEnabled = seedAttribute.Equals("true");
                                }

                            // Set the RangeMax value if not valid don't use the attributes.
                            seedAttribute = xmlSeedReader.GetAttribute (SEED_ATTRIBUTE_MAX);
                            if ( !String.IsNullOrWhiteSpace (seedAttribute) )
                                {
                                useSeedRange = true;
                                int rangeMax;
                                if ( int.TryParse (seedAttribute, out rangeMax) )
                                    {
                                    // Set the RangeMin value (default = 0).
                                    seedAttribute = xmlSeedReader.GetAttribute (SEED_ATTRIBUTE_MIN);
                                    int RangeMin = 0;
                                    if ( !String.IsNullOrWhiteSpace (seedAttribute) )
                                        {
                                        if ( !int.TryParse (seedAttribute, out RangeMin) )
                                            RangeMin = 0;
                                        }

                                    // Set the Increment value (default = 1).
                                    seedAttribute = xmlSeedReader.GetAttribute (SEED_ATTRIBUTE_INC);
                                    int increment = 1;
                                    if ( !String.IsNullOrWhiteSpace (seedAttribute) )
                                        {
                                        if ( !int.TryParse (seedAttribute, out increment) )
                                            increment = 1;
                                        }

                                    // Set the RightPart value (default = "").
                                    String rightPart = xmlSeedReader.GetAttribute (SEED_ATTRIBUTE_RIGHT);
                                    if ( String.IsNullOrWhiteSpace (seedAttribute) )
                                        rightPart = "";
                                    String seedString = xmlSeedReader.ReadElementContentAsString ();
                                    // Generate the seed for the range.
                                    for ( int i = RangeMin; i <= rangeMax; i = i + increment )
                                        {
                                        String seedStringRange = seedString + i + rightPart;
                                        Uri seedUri = new Uri (seedStringRange);
                                        Seed xmlSeed = new Seed (seedUri, isExternalPageCrawlingEnabled);
                                        m_seedList.Add (xmlSeed);
                                        }
                                    }
                                }
                            
                            }
                        // ToDo: Use the new Seed object to fill the list.
                        // If the range attributes are not present just add the seed to the list.
                        if ( !useSeedRange )
                            {
                            String seedString = xmlSeedReader.ReadElementContentAsString ();
                            Uri seedUri = new Uri (seedString);
                            Seed xmlSeed = new Seed (seedUri, isExternalPageCrawlingEnabled);
                            m_seedList.Add (xmlSeed);
                            }
                        } while ( xmlSeedReader.ReadToNextSibling (SEED_NODE_NAME) );
                    }
                xmlSeedReader.Dispose ();
                }
            catch ( Exception )
                {
                Console.WriteLine ("Error Reading the seed file {0}", xmlFilePath);
                }
            }
        #endregion

        #region Public Properties
        /// <summary>
        /// Return a list with the seed Uri.
        /// </summary>
        public List<Seed> seedList
            {
            get
                {
                return m_seedList;
                }
            }
        #endregion

        #region Methods
        #region Public Methods
        /// <summary>
        /// Get a list of the seed Uri.
        /// </summary>
        /// <returns>List of the seed URI</returns>
        public List<Seed> GetSeeds ()
            {
            return seedList;
            }

        #endregion
        #endregion
        }
    }
