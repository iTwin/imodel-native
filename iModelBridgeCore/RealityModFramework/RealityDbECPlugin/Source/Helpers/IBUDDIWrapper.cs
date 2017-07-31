using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Bentley.Discovery.Buddi.Client;

namespace IndexECPlugin.Source.Helpers
    {
    /// <summary>
    /// Buddi wrapper used for testing.
    /// </summary>
    public interface IBUDDIWrapper
        {
        /// <summary>
        /// Get URL from BUDDI
        /// </summary>
        /// <param name="urlName">The name associated to the URL</param>
        /// <returns>The URL</returns>
        string GetUrl (string urlName);

        /// <summary>
        /// Get URL from BUDDI
        /// </summary>
        /// <param name="urlName">The name associated to the URL</param>
        /// <param name="regionCode">The buddi region code of the URL</param>
        /// <returns>The URL</returns>
        string GetUrl (string urlName, int regionCode);
        }

    /// <summary>
    /// Implementation of IBUDDIWrapper interface
    /// </summary>
    public class BuddiWrapper : IBUDDIWrapper
        {
        /// <summary>
        /// Get URL from BUDDI
        /// </summary>
        /// <param name="urlName">The name associated to the URL</param>
        /// <returns>The URL</returns>
        public string GetUrl (string urlName)
            {
            return new BUDDIClient().GetUrl(urlName);
            }

        /// <summary>
        /// Get URL from BUDDI
        /// </summary>
        /// <param name="urlName">The name associated to the URL</param>
        /// <param name="regionCode">The buddi region code of the URL</param>
        /// <returns>The URL</returns>
        public string GetUrl (string urlName, int regionCode)
            {
            return new BUDDIClient().GetUrl(urlName, regionCode);
            }
        }
    }
