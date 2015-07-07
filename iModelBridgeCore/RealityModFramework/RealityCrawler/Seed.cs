using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Bentley.RealityPlatform.RealityCrawler
    {
    class Seed
        {
        #region Constructor
        public Seed (Uri seedUri) 
            : this (seedUri, false)
            {

            }

        public Seed (Uri seedUri, Boolean seedIsExternalPageCrawlingEnabled)
            {
            uri = seedUri;
            isExternalPageCrawlingEnabled = seedIsExternalPageCrawlingEnabled;
            }
        #endregion

        #region Public Properties
        public Uri uri
            {
            get;
            private set;
            }

        public Boolean isExternalPageCrawlingEnabled
            {
            get;
            private set;
            }
        #endregion
        }
    }
