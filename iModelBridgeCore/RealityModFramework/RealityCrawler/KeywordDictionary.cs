/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityCrawler/KeywordDictionary.cs $
| 
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;

namespace Bentley.RealityPlatform.RealityCrawler
{
    class KeywordDictionary
    {
        #region Constants
        // Constants
        protected const String DEFAULT_DICTIONARY_FILE = "Dictionary.xml";
        protected const String KEYWORDS_NODE_NAME = "Keywords";
        #endregion

        #region Protected Members
        protected List<String> m_keywordList = new List<String>();
        #endregion

        #region Constructor
        public KeywordDictionary()
            : this(DEFAULT_DICTIONARY_FILE)
        {

        }

        public KeywordDictionary(String xmlFilePath)
        {
            XmlTextReader xmlDictionaryReader = new XmlTextReader(xmlFilePath);
            xmlDictionaryReader.ReadToFollowing(KEYWORDS_NODE_NAME);
            do
            {
                String keyword = xmlDictionaryReader.ReadString();
                m_keywordList.Add(keyword);
            } while (xmlDictionaryReader.ReadToNextSibling(KEYWORDS_NODE_NAME));
            xmlDictionaryReader.Dispose();
        }
        #endregion

        #region Methods
        #region Public Methods

        public Boolean FindKeyword(Uri link)
        {
            String LinkAbsoluteUri = link.AbsoluteUri.ToLower();
            foreach (String keyword in m_keywordList)
            {
                if(LinkAbsoluteUri.Contains(keyword.ToLower()))
                {
                    return true;
                }
            }
            return false;
        }
        #endregion
        #endregion
    }
}
