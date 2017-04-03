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

        public string RdsUrlBase 
            {
            get
                {
                return m_rdsUrlBase;
                }
            }
        //const string RealityDataClass = "RealityData/";

        string m_base64token;

        /// <summary>
        /// RDSDataFetcher constructor
        /// </summary>
        /// <param name="base64token">Then IMS token used to communicate with RDS</param>
        public RDSDataFetcher (string base64token)
            {
            m_base64token = base64token;

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

        private string GetHttpResponse (string url)
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
                    client.DefaultRequestHeaders.Authorization = new AuthenticationHeaderValue("Token", m_base64token);
                    client.DefaultRequestHeaders.Accept.Add(new MediaTypeWithQualityHeaderValue("application/json"));
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
                            throw new OperationFailedException("Reality Data Server returned an error : " + response.ReasonPhrase);
                            }
                        }
                    }
                }
            //return null;
            }

        /// <summary>
        /// Method for querying information on a single entity
        /// </summary>
        /// <param name="entityID">The ID of the entity</param>
        /// <returns>The Jobject containing the information requested.</returns>
        public JObject GetSingleData (string entityID)
            {

            string url = RdsUrlBase + IndexConstants.RdsRealityDataClass + "/" + entityID;

            JObject jsonResponse = JObject.Parse(GetHttpResponse(url));
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

            IEnumerable<SingleWhereCriteriaHolder> classificationCriteria = criteriaList.Where(criteria => criteria.Property.Name == "Classification");
            string classificationFilter = "";
            if(classificationCriteria.Count() != 0)
                {
                string[] convertedCriteria = classificationCriteria.Select(criterion => ConvertWhereCriteriaToWSG(criterion, criterion.Property.Name)).ToArray();
                classificationFilter = "&$filter=" + String.Join("+and+", convertedCriteria);
                }
            string url = RdsUrlBase + IndexConstants.RdsRealityDataClass + "?polygon=" + polygon + classificationFilter;
            JObject jsonResponse = JObject.Parse(GetHttpResponse(url));

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
                default:
                    throw new UserFriendlyException("This type of criteria is not possible when querying " + IndexConstants.RdsSourceName + " source.");
                }
            }
        }
    }
