using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IndexECPlugin.Source
{
    /// <summary>
    /// Contains extension methods for JToken
    /// </summary>
    public static class JTokenExtension
    {
        /// <summary>
        /// Tries to get a string from an entry in the jtoken. If there is no entry for the specified key, null is returned.
        /// </summary>
        /// <param name="token">The token on which we call the method</param>
        /// <param name="key">The key of the entry from which we want the string</param>
        /// <returns>The string if there is an entry for the specified key, null otherwise</returns>
        public static string TryToGetString(this JToken token, string key)
        {
            var subToken = token[key];

            if(subToken == null)
            {
                return null;
            }

            return subToken.Value<string>();
        }
    }
}
