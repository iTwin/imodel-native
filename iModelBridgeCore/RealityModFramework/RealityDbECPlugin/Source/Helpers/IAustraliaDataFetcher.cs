using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;
using Bentley.EC.Persistence.Operations;
using Bentley.EC.Persistence.Query;
using Newtonsoft.Json.Linq;
using FieldPair = System.Collections.Generic.KeyValuePair<string, string>;

namespace IndexECPlugin.Source.Helpers
    {
    /// <summary>
    /// Interface for retrieving Australian API information. Should not be used except for mocking AustraliaDataFetcher.
    /// </summary>
    public interface IAustraliaDataFetcher
        {
        /// <summary>
        /// A list of Australia Layers that we want to fetch
        /// </summary>
        List<AustraliaLayer> LayerTable
            {
            get;
            }

        /// <summary>
        /// Method for querying information on a single entity
        /// </summary>
        /// <param name="id">The ID of the entity</param>
        /// <param name="layer">The australiaLayer Object containing information about the layer containing the entity</param>
        /// <returns>The Jobject containing the information requested.</returns>
        JObject GetSingleData (string id, out AustraliaLayer layer);

        /// <summary>
        /// Method for querying information using spatial and classification criteria
        /// </summary>
        /// <param name="polygon">The polygon used to limit the search</param>
        /// <param name="criteriaList">A list of other property criteria</param>
        /// <returns>The JArray containing the information requested.</returns>
        IEnumerable<AURequestBundle> GetDataBySpatialQuery (string polygon, List<SingleWhereCriteriaHolder> criteriaList);
        }


    internal class AustraliaDataFetcher : IAustraliaDataFetcher
        {

        //
        private List<AustraliaLayer> m_LayerTable = new List<AustraliaLayer>
        {
            new AustraliaLayer(){Id="0", Classification="Terrain", IdField="objectid", GeometryType="esriGeometryPolygon", Dataset="", Type="adf", CopyrightText="", FieldMap=new Dictionary<string,string>(){{"Id", "objectid"}, {"Name", "name"}, {"MainURL", "object_url"}}},
            new AustraliaLayer(){Id="1", Classification="Terrain", IdField="objectid", GeometryType="esriGeometryPolygon", Dataset="", Type="adf", CopyrightText="", FieldMap=new Dictionary<string,string>(){{"Id", "objectid"}, {"Name", "name"}, {"MainURL", "object_url"}}},
            new AustraliaLayer(){Id="2", Classification="Terrain", IdField="objectid", GeometryType="esriGeometryPolygon", Dataset="", Type="tif", CopyrightText="", FieldMap=new Dictionary<string,string>(){{"Id", "objectid"}, {"Name", "object_name"}, {"MainURL", "object_url"}, {"Filesize", "object_size"}}}
        };

        public List<AustraliaLayer> LayerTable
            {
            get
                {
                return m_LayerTable;
                }
            }

        private string GetHttpResponse (string url)
            {
            using ( HttpClient client = new HttpClient() )
                {
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
                        throw new OperationFailedException("The Geoscience Australia API returned an error : " + response.ReasonPhrase);
                        }
                    }
                }
            //return null;
            }

        public JObject GetSingleData (string id, out AustraliaLayer layer)
            {
            if ( !SourceStringMap.IsValidId(DataSource.AU, id) )
                {
                throw new OperationFailedException("Invalid Geoscience Australia API Id");
                }

            string[] splitId = id.Split('_');
            string layerId = splitId[1];
            string dataId = splitId[2];

            layer = LayerTable.FirstOrDefault(l => l.Id == layerId);
            if(layer == null)
                {
                throw new OperationFailedException("The requested instance doesn't exist.");
                }

            string outFields = String.Join(",", layer.FieldMap.Select(map => map.Value));

            //bool returnGeometry = layer.Classification == "Imagery";
            bool returnGeometry = true;

            string url = IndexConstants.AUBaseUrl + layerId + "/query?f=json&outFields=" + outFields + "&returnGeometry=" + returnGeometry +"&where=" + layer.IdField + "='" + dataId + "'";

            JObject jsonResponse = JObject.Parse(GetHttpResponse(url));

            return jsonResponse;
            }

        public IEnumerable<AURequestBundle> GetDataBySpatialQuery (string polygon, List<SingleWhereCriteriaHolder> criteriaList)
            {

            List<AURequestBundle> responseList = new List<AURequestBundle>();

            List<String> requestedClassificationList = null;
            bool selectAllClasses = true;

            foreach ( var criteriaHolder in criteriaList )
                {
                if ( criteriaHolder.Property.Name == "Classification" &&
                    (criteriaHolder.Operator == RelationalOperator.IN || criteriaHolder.Operator == RelationalOperator.EQ) )
                    {
                    string[] newClassificationArray = null;
                    if ( criteriaHolder.Operator == RelationalOperator.IN )
                        {
                        newClassificationArray = criteriaHolder.Value.Split(',');
                        }
                    if ( criteriaHolder.Operator == RelationalOperator.EQ )
                        {
                        newClassificationArray = new string[] { criteriaHolder.Value };
                        }

                    if ( requestedClassificationList == null )
                        {
                        requestedClassificationList = newClassificationArray.ToList();
                        }
                    else
                        {
                        requestedClassificationList = requestedClassificationList.Intersect(newClassificationArray).ToList();
                        }
                    selectAllClasses = false;
                    }
                }

            PolygonModel model = DbGeometryHelpers.CreatePolygonModelFromJson(polygon);

            string polygonEsri = DbGeometryHelpers.CreateEsriPolygonFromPolyModel(model);

            foreach ( AustraliaLayer layer in LayerTable )
                {
                //If the layer does not fit the criteria
                if ( !selectAllClasses && !requestedClassificationList.Contains(layer.Classification) )
                    {
                    continue;
                    }

                string outFields = String.Join(",", layer.FieldMap.Select(map => map.Value));

                //bool returnGeometry = layer.Classification == "Imagery";
                bool returnGeometry = true;

                string url = IndexConstants.AUBaseUrl + layer.Id + "/query?f=json&outFields=" + outFields + "&returnGeometry=" + returnGeometry + "&geometryType=esriGeometryPolygon&geometry=" + polygonEsri + "&sr=" + model.coordinate_system;

                JObject jsonResponse = JObject.Parse(GetHttpResponse(url));

                responseList.Add(new AURequestBundle()
                {
                    Layer = layer, LayerContent = jsonResponse
                });
                }

            return responseList;
            }
        }
    }
