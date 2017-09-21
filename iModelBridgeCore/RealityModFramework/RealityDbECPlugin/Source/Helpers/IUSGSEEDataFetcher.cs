using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;
using System.Web;
using System.Xml;
using Bentley.EC.Persistence.Operations;
using Bentley.EC.Persistence.Query;
using Bentley.ECSystem.Configuration;
using Newtonsoft.Json.Linq;

namespace IndexECPlugin.Source.Helpers
    {
    /// <summary>
    /// Interface for querying USGS Earth Explorer (EE) API
    /// </summary>
    public interface IUSGSEEDataFetcher
        {

        /// <summary>
        /// List of preselected datasets that are queried on a spatial request.
        /// </summary>
        List<UsgsEEDataset> DatasetList
            {
            get;
            }

        /// <summary>
        /// Executes a spatial query on the EE API
        /// </summary>
        /// <returns>All the data queried from all the preselected datasets</returns>
        List<EERequestBundle> SpatialQuery ();

        /// <summary>
        /// Gets a particuliar instance from EE
        /// </summary>
        /// <param name="entityId">The Id of the entity in EE API</param>
        /// <param name="datasetId">The entity's dataset Id</param>
        /// <returns>The Xml metadata document obtained from EE</returns>
        XmlDocument GetXmlDocForInstance (string entityId, string datasetId);
        }

    /// <summary>
    /// Implementation of the IUSGSEEDataFetcher interface
    /// </summary>
    public class UsgsEEDataFetcher : IUSGSEEDataFetcher
        {

        ECQuery m_query;
        IHttpResponseGetter m_httpResponseGetter;

        const string m_spatialFilterCanvas = "{\"filterType\":\"mbr\",\"lowerLeft\":{\"latitude\":_llLat,\"longitude\":_llLong},\"upperRight\":{\"latitude\":_urLat,\"longitude\":_urLong}}";
        const string m_jsonSpatialRequest = "{\"apiKey\":\"_apiKey\",\"datasetName\":\"_datasetName\",\"spatialFilter\":_spatialFilter,\"maxResults\":_maxResults}";

        string m_maxResults = ConfigurationRoot.GetAppSetting("RECPMaxResultsUSGSEE") ?? "400";

        readonly List<UsgsEEDataset> m_datasetList = new List<UsgsEEDataset>()
        {
            new UsgsEEDataset(){DatasetName = "HIGH_RES_ORTHO", DatasetId = "3411", DataFormat = "GeoTIFF", Classification = "Imagery"}
        };

        /// <summary>
        /// List of preselected datasets that are queried on a spatial request.
        /// </summary>
        public List<UsgsEEDataset> DatasetList
            {
            get
                {
                return m_datasetList;
                }
            }

        /// <summary>
        /// UsgsEEDataFetcher constructor
        /// </summary>
        /// <param name="query">ECQuery object</param>
        /// <param name="httpResponseGetter">IHttpResponseGetter used to query the API</param>
        public UsgsEEDataFetcher(ECQuery query, IHttpResponseGetter httpResponseGetter)
            {
            m_query = query;
            m_httpResponseGetter = httpResponseGetter;
            }

        /// <summary>
        /// Executes a spatial query on the EE API
        /// </summary>
        /// <returns>All the data queried from all the preselected datasets</returns>
        public List<EERequestBundle> SpatialQuery ()
            {
            List<EERequestBundle> responseList = new List<EERequestBundle>();
            BBox bbox = DbGeometryHelpers.ExtractBboxFromWKTPolygon(m_query.ExtractPolygonDescriptorFromQuery().WKT);
            string apiKey = null;
            string spatialFilter = m_spatialFilterCanvas.Replace("_llLat", bbox.minY.ToString()).Replace("_llLong", bbox.minX.ToString()).Replace("_urLat", bbox.maxY.ToString()).Replace("_urLong", bbox.maxX.ToString());

            try
                {
                apiKey = GetEEApiKey();
                var datasetList = GetDatasetList(apiKey, spatialFilter);


                //TODO: Parallelize this foreach
                foreach ( var dataset in m_datasetList )
                    {
                    try
                        {
                        if ( datasetList.Any(d => d == dataset.DatasetName) )
                            {

                            string jsonRequest = m_jsonSpatialRequest.Replace("_apiKey", apiKey).Replace("_datasetName", dataset.DatasetName).Replace("_spatialFilter", spatialFilter).Replace("_maxResults", m_maxResults);

                            string response = m_httpResponseGetter.GetHttpResponse(IndexConstants.EEBaseApiURL + "search?jsonRequest=" + Uri.EscapeUriString(jsonRequest));

                            JObject jsonResponse = JObject.Parse(response);
                            string errorCode = jsonResponse.TryToGetString("errorCode");
                            if ( !String.IsNullOrWhiteSpace(errorCode) )
                                {
                                throw new OperationFailedException("Usgs Earth Explorer API returned an error code : " + errorCode);
                                }

                            if ( (jsonResponse["data"] != null) && jsonResponse["data"]["results"] != null )
                                {
                                responseList.Add(new EERequestBundle()
                                {
                                    Dataset = dataset, jtokenList = jsonResponse["data"]["results"]
                                });
                                }
                            }
                        }
                    catch ( OperationFailedException ex)
                        {
                        //This case happens when the http call returns an unsuccessful error code. 
                        //We'll simply let the other calls end without rethrowing.
                        Log.Logger.error("Usgs Earth Explorer call problem: " + ex.Message);
                        continue;
                        }
                    catch ( TaskCanceledException )
                        {
                        //Request timed out. We return our empty list
                        //return instanceList;
                        throw new Bentley.Exceptions.EnvironmentalException("Earth Explorer request timed out");
                        }
                    catch ( System.AggregateException ex )
                        {
                        if ( (ex.InnerExceptions.Count == 1) && (ex.InnerException.GetType() == typeof(TaskCanceledException)) )
                            {
                            throw new Bentley.Exceptions.EnvironmentalException("Earth explorer request timed out");
                            }
                        else
                            {
                            throw ex;
                            }
                        }
                    }
                }
            finally
                {
                LogOutApiKey(apiKey);
                }
            return responseList;
            }

        private string GetEEApiKey()
            {
            string userName = ConfigurationRoot.GetAppSetting("RECPUserNameEE");
            string password = ConfigurationRoot.GetAppSetting("RECPPasswordEE");

            if(String.IsNullOrWhiteSpace(userName) || String.IsNullOrWhiteSpace(password))
            {
                throw new OperationFailedException("The service is not configured properly to query Usgs Earth Explorer API.");
            }

            string jsonRequest = "{\"username\":\"" + userName +"\",\"password\":\"" + password + "\",\"authType\":\"EROS\",\"catalogId\":\"EE\"}";
            string response = m_httpResponseGetter.GetHttpResponse(IndexConstants.EEBaseApiURL + "login?jsonRequest=" + Uri.EscapeUriString(jsonRequest));

            JObject jsonResponse = JObject.Parse(response);

            string errorCode = jsonResponse.TryToGetString("errorCode");
            if(String.IsNullOrWhiteSpace(errorCode))
                {
                return jsonResponse.TryToGetString("data");
                }
            throw new OperationFailedException("Could not retrieve api key to communicate with Usgs Earth Explorer API.");
            }

        private void LogOutApiKey(string apiKey)
            {
            try
                {
                if ( !String.IsNullOrWhiteSpace(apiKey) )
                    {
                    string jsonRequest = "{\"apiKey\":\"" + apiKey + "\"}";

                    //This call's response is not important. Only calling it should destroy the Api Key
                    m_httpResponseGetter.GetHttpResponse(IndexConstants.EEBaseApiURL + "logout?jsonRequest=" + Uri.EscapeUriString(jsonRequest));

                    }
                }
            catch ( Exception e )
                {
                Log.Logger.error("Usgs Earth Explorer Logout call problem: " + e.Message);
                throw;
                }
            }

        private List<string> GetDatasetList (string apiKey, string spatialFilter)
            {
            List<string> datasetList = new List<string>();
            string jsonRequest = "{\"apiKey\":\"" + apiKey + ",\"spatialFilter\":" + spatialFilter + "}";
            string response = m_httpResponseGetter.GetHttpResponse(IndexConstants.EEBaseApiURL + "datasets?jsonRequest=" + Uri.EscapeUriString(jsonRequest));

            JObject jobject = JObject.Parse(response);

            string errorCode = jobject.TryToGetString("errorCode");
            if ( String.IsNullOrWhiteSpace(errorCode) )
                {
                JArray datasetsArray = jobject["data"] as JArray;
                if ( datasetsArray != null )
                    {
                    foreach ( JObject datasetObject in datasetsArray )
                        {
                        string datasetName = datasetObject.TryToGetString("datasetName");
                        if ( !String.IsNullOrWhiteSpace(datasetName) )
                            {
                            datasetList.Add(datasetName);
                            }
                        }
                    return datasetList;
                    }
                }
            throw new OperationFailedException("Could not retrieve Dataset List from Usgs Earth Explorer API.");

            }

        /// <summary>
        /// Gets a particuliar instance from EE
        /// </summary>
        /// <param name="entityId">The Id of the entity in EE API</param>
        /// <param name="datasetId">The entity's dataset Id</param>
        /// <returns>The Xml metadata document obtained from EE</returns>
        public XmlDocument GetXmlDocForInstance (string entityId, string datasetId)
            {
            try
                {
                string url = IndexConstants.EEBaseMetadataURL + datasetId + "/" + entityId;
                string xmlString = m_httpResponseGetter.GetHttpResponse(url);

                XmlDocument xmlDoc = new XmlDocument();
                xmlDoc.LoadXml(xmlString);
                return xmlDoc;
                }
            catch ( Exception e )
                {
                Log.Logger.error("Usgs Earth Explorer single entity call problem: " + e.Message);
                throw;
                }
            }
        }


    }
