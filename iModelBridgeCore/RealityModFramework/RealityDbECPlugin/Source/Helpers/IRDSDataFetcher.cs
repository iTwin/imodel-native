using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Net.Security;
using System.Text;
using System.Threading.Tasks;
using Bentley.Discovery.Buddi.Client;
using Bentley.EC.Persistence.Operations;
using Bentley.EC.Persistence.Query;
using Bentley.ECSystem.Configuration;
using Bentley.Exceptions;
using Newtonsoft.Json.Linq;

namespace IndexECPlugin.Source.Helpers
    {

    /// <summary>
    /// Interface for retrieving rds information. Should not be used except for mocking RDSDataFetcher.
    /// </summary>
    public interface IRDSDataFetcher
        {

        /// <summary>
        /// The base URL of the RDS Service.
        /// </summary>
        string RdsUrlBase {get;}

        /// <summary>
        /// Method for querying information on a single entity
        /// </summary>
        /// <param name="entityID">The ID of the entity</param>
        /// <returns>The Jobject containing the information requested.</returns>
        JObject GetSingleData (string entityID);

        /// <summary>
        /// Method for querying the ReadAccessAzureToken of a single entity
        /// </summary>
        /// <param name="entityID">The ID of the entity</param>
        /// <returns>The Jobject containing the information requested.</returns>
        JObject GetReadAccessAzureToken (string entityID);

        /// <summary>
        /// Method for querying information using spatial and classification criteria
        /// </summary>
        /// <param name="polygon">The polygon used to limit the search</param>
        /// <param name="criteriaList">A list of other property criteria</param>
        /// <returns>The JArray containing the information requested.</returns>
        JArray GetDataBySpatialQuery (string polygon, List<SingleWhereCriteriaHolder> criteriaList);

        }

    internal class RDSDataFetcher : IRDSDataFetcher
        {
        string m_rdsUrlBase;
        IHttpResponseGetter m_httpResponseGetter;

        public string RdsUrlBase 
            {
            get
                {
                return m_rdsUrlBase;
                }
            }
        //const string RealityDataClass = "RealityData/";

        /// <summary>
        /// RDSDataFetcher constructor
        /// </summary>
        /// <param name="httpResponseGetter">An httpResponseGetter object for communicating with RDS</param>
        public RDSDataFetcher (IHttpResponseGetter httpResponseGetter)
            {
            m_httpResponseGetter = httpResponseGetter;

            string buddiRegionCode = ConfigurationRoot.GetAppSetting("RECPBuddiRegionCode");
            BUDDIClient buddiClient = new BUDDIClient();

            int buddiRegionCodeInt; 
            bool successfulParse = int.TryParse(buddiRegionCode, out buddiRegionCodeInt);

            try
                {
                string rdsBaseName;
                if ( successfulParse )
                    {
                    rdsBaseName = buddiClient.GetUrl(IndexConstants.RdsName, buddiRegionCodeInt);
                    }
                else
                    {
                    rdsBaseName = buddiClient.GetUrl(IndexConstants.RdsName);
                    }
                m_rdsUrlBase = rdsBaseName + "v2.3/repositories/S3MXECPlugin--Server/S3MX/";
                }
            catch ( Exception )
                {
                m_rdsUrlBase = ConfigurationRoot.GetAppSetting("RECPRdsUrlBase");
                if ( m_rdsUrlBase == null )
                    {
                    throw new OperationFailedException("There was an error getting RDS url.");
                    }
                }
            }

        /// <summary>
        /// Method for querying information on a single entity
        /// </summary>
        /// <param name="entityID">The ID of the entity</param>
        /// <returns>The Jobject containing the information requested.</returns>
        public JObject GetSingleData (string entityID)
            {

            string url = RdsUrlBase + IndexConstants.RdsRealityDataClass + "/" + entityID;

            JObject jsonResponse = JObject.Parse(m_httpResponseGetter.GetHttpResponse(url));
            JArray array = jsonResponse["instances"] as JArray;

            return array.First as JObject;
            }

        /// <summary>
        /// Method for querying the ReadAccessAzureToken of a single entity
        /// </summary>
        /// <param name="entityID">The ID of the entity</param>
        /// <returns>The Jobject containing the information requested.</returns>
        public JObject GetReadAccessAzureToken (string entityID)
            {
            string url = RdsUrlBase + IndexConstants.RdsRealityDataClass + "/" + entityID + IndexConstants.RdsReadAccessAzureTokenUrlEnd;

            JObject jsonResponse = JObject.Parse(m_httpResponseGetter.GetHttpResponse(url));
            JArray array = jsonResponse["instances"] as JArray;

            return array.First as JObject;
            }

        /// <summary>
        /// Method for querying information using spatial and classification criteria
        /// </summary>
        /// <param name="polygon">The polygon used to limit the search</param>
        /// <param name="criteriaList">A list of other property criteria</param>
        /// <returns>The JArray containing the information requested.</returns>
        public JArray GetDataBySpatialQuery (string polygon, List<SingleWhereCriteriaHolder> criteriaList)
            {
            //For now, we only include the classification criteria
            string visibilityCriteria = "&$filter=Visibility+in+['PUBLIC','ENTERPRISE']";

            IEnumerable<SingleWhereCriteriaHolder> classificationCriteria = criteriaList.Where(criteria => criteria.Property.Name == "Classification");
            string classificationFilter = "";
            if(classificationCriteria.Count() != 0)
                {
                string[] convertedCriteria = classificationCriteria.Select(criterion => ConvertWhereCriteriaToWSG(criterion, criterion.Property.Name)).ToArray();
                classificationFilter = "+and+" + String.Join("+and+", convertedCriteria);
                }
            string url = RdsUrlBase + IndexConstants.RdsRealityDataClass + "?polygon=" + polygon + visibilityCriteria + classificationFilter;
            JObject jsonResponse = JObject.Parse(m_httpResponseGetter.GetHttpResponse(url));

            return jsonResponse["instances"] as JArray;
            }

        /// <summary>
        /// This method's purpose is to transform a criterion into a WSG filter statement.
        /// </summary>
        /// <param name="criteria">The criterion</param>
        /// <param name="propertyName">The property name in RDS. This parameter is there in case the properties' names in RDS and indexECPlugin are different</param>
        /// <returns>The string containing the filter statement (without $filter= in case there are many criteria)</returns>
        private string ConvertWhereCriteriaToWSG(SingleWhereCriteriaHolder criteria, string propertyName)
            {
            switch(criteria.Operator)
                {
                case RelationalOperator.EQ:
                    return propertyName + "+eq+" + criteria.Value;
                case RelationalOperator.LT:
                    return propertyName + "+lt+" + criteria.Value;
                case RelationalOperator.LTEQ:
                    return propertyName + "+le+" + criteria.Value;
                case RelationalOperator.GT:
                    return propertyName + "+gt+" + criteria.Value;
                case RelationalOperator.GTEQ:
                    return propertyName + "+ge+" + criteria.Value;
                case RelationalOperator.NE:
                    return propertyName + "+ne+" + criteria.Value;
                //case RelationalOperator.IN:
                //    return "(" + string.Join("+or+", criteria.Value.Split(',').Select(s => propertyName + "+eq+" + s)) + ")";
                default:
                    throw new UserFriendlyException("This type of criteria is not possible when querying " + IndexConstants.RdsSourceName + " source.");
                }
            }
        }

    /// <summary>
    /// Class used for communicating with RDS, specialized in setting the IMS Token.
    /// </summary>
    public class RdsHttpResponseGetter : IHttpResponseGetter
        {
        readonly string m_base64Token;

        /// <summary>
        /// RdsHttpResponseGetter constructor
        /// </summary>
        /// <param name="base64Token">The IMS token</param>
        public RdsHttpResponseGetter(string base64Token)
            {
            m_base64Token = base64Token;
            }

        /// <summary>
        /// Get the http response to an url. Returns OperationFailedException if the status code is not successful
        /// </summary>
        /// <param name="url">The url</param>
        /// <returns>The http response</returns>
        public string GetHttpResponse (string url)
            {
            using ( var handler = new WebRequestHandler() )
                {
                handler.ServerCertificateValidationCallback = (sender, certificate, chain, errors) =>
                {
                    if ( certificate.GetCertHashString() == ConfigurationRoot.GetAppSetting("RECPRdsCertHashString") )
                        {
                        return true;
                        }

                    return errors == SslPolicyErrors.None;

                };

                using ( HttpClient client = new HttpClient(handler) )
                    {
                    client.DefaultRequestHeaders.Authorization = new AuthenticationHeaderValue("Token", m_base64Token);
                    client.DefaultRequestHeaders.Accept.Add(new MediaTypeWithQualityHeaderValue("application/json"));
                    using ( HttpResponseMessage response = client.GetAsync(url).Result )
                        {
                        if ( response.IsSuccessStatusCode && response.RequestMessage.RequestUri.OriginalString.Contains(url) )
                            {
                            using ( HttpContent content = response.Content )
                                {
                                return content.ReadAsStringAsync().Result;
                                }
                            }
                        else
                            {
                            if ( !response.IsSuccessStatusCode )
                                {
                                throw new OperationFailedException("Reality Data Server returned an error : " + response.ReasonPhrase);
                                }
                            else
                                {
                                throw new OperationFailedException("Reality Data Server returned an error.");
                                }
                            }
                        }
                    }
                }
            }
        }
    }
