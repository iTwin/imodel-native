using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;
using Bentley.EC.Persistence.Operations;
using Bentley.EC.Plugin;

namespace IndexECPlugin.Source.Helpers
    {
    /// <summary>
    /// Interface for mocking Http calls
    /// </summary>
    public interface IHttpResponseGetter
        {
        /// <summary>
        /// Get the http response to an url. Should return OperationFailedException if the status code is not successful
        /// </summary>
        /// <param name="url">The url</param>
        /// <returns>The http response</returns>
        string GetHttpResponse (string url);
        }

    /// <summary>
    /// Interface for mocking Generic Http calls
    /// </summary>
    public class GenericHttpResponseGetter : IHttpResponseGetter
        {
        string m_source;
        TimeSpan? m_timeout;

        /// <summary>
        /// GenericHttpResponseGetter constructor
        /// </summary>
        /// <param name="source">The name of the source (for logging and error messages).</param>
        /// <param name="timeout">The timeout timespan. If null, no timeout will be set.</param>
        public GenericHttpResponseGetter (string source, TimeSpan? timeout = null)
            {
            m_source = source;
            m_timeout = timeout;
            }

        /// <summary>
        /// Get the http response to an url. Returns OperationFailedException if the status code is not successful
        /// </summary>
        /// <param name="url">The url</param>
        /// <returns>The http response</returns>
        public string GetHttpResponse (string url)
            {
            using ( HttpClient client = new HttpClient() )
                {
                if ( m_timeout.HasValue )
                    {
                    client.Timeout = m_timeout.Value;
                    }
                using ( HttpResponseMessage response = client.GetAsync(url).Result )
                    {
                    if ( response.IsSuccessStatusCode )
                        {
                        using ( HttpContent content = response.Content )
                            {
                            return content.ReadAsStringAsync().Result;
                            }
                        }
                    else
                        {
                        if ( response.StatusCode == HttpStatusCode.ServiceUnavailable )
                            {
                            throw new OperationFailedException("The " + m_source + " source is unavailable for the moment. Please retry later.");
                            }
                        throw new OperationFailedException("The HTTP call to " + m_source + " returned an error : " + response.ReasonPhrase);
                        }
                    }
                }
            }
        }
    }
