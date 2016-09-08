/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/Helpers/IUSGSDataFetcher.cs $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/

using Bentley.EC.Persistence.Query;
using Bentley.Exceptions;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using Bentley.ECSystem.Configuration;
using Bentley.EC.Persistence.Operations;
using System.Net;

namespace IndexECPlugin.Source.Helpers
    {
    /// <summary>
    /// Interface for retrieving USGS information. Should not be used except for mocking USGSDataFetcher.
    /// </summary>
    public interface IUSGSDataFetcher
        {
        /// <summary>
        /// A list of USGS API Categories that we want to fetch
        /// </summary>
        List<UsgsAPICategory> CategoryTable
            {
            get;
            }

        /// <summary>
        /// Retrieves the data from the sciencebase entry of given ID
        /// </summary>
        /// <param name="sourceID">The ID of the wanted entry in sciencebase</param>
        /// <returns>The JObject containing the information</returns>
        JObject GetSciencebaseJson (string sourceID);

        /// <summary>
        /// Gets the xml document located at given URL.
        /// </summary>
        /// <param name="URL">The URL pointing at the xml document</param>
        /// <returns>The xml document located at given URL</returns>
        XmlDocument GetXmlDocFromURL (string URL);

        /// <summary>
        /// Queries USGS for data located in the selected bbox and in the predetermined datasets (see CategoryTable)
        /// </summary>
        /// <param name="whereCriteriaList">The where criteria list.</param>
        /// <returns>Non formatted data (still in JSON form), along with the ID of the parent dataset. This ID was added there to prevent
        /// having to query USGS for every single data</returns>

        //TODO : CHANGE THE LIST OF RAW WHERE CRITERIA TO A LIST OF CLASSIFICATIONS AND BBOX
        IEnumerable<USGSRequestBundle> GetNonFormattedUSGSResults (List<SingleWhereCriteriaHolder> whereCriteriaList);
        }

    internal class USGSDataFetcher : IUSGSDataFetcher
        {
        //TODO : Add outputFormat=JSON in WebReq???
        const string WebReqCategories = "https://viewer.nationalmap.gov/tnmaccess/api/datasets?";
        const string WebReq = "https://viewer.nationalmap.gov/tnmaccess/api/products?bbox=_bbox&q=&start=&end=&dateType=&datasets=_datasets&prodFormats=_prodFormats&prodExtents=&polyCode=&polyType=&max=_maxResults&offset=0&version=2";

        //const string WebReqCategories = "http://tnmbeta.cr.usgs.gov/tnmaccess/api/datasets?";
        //const string WebReq = "http://tnmbeta.cr.usgs.gov/tnmaccess/api/products?bbox=_bbox&q=&start=&end=&dateType=&datasets=_datasets&prodFormats=_prodFormats&prodExtents=&polyCode=&polyType=&max=_maxResults&offset=0&version=2";

        string MaxResults = ConfigurationRoot.GetAppSetting("RECPMaxResultsUSGS") ?? "400";
        //const int USGSIdLenght = 24;
        private readonly List<UsgsAPICategory> m_categoryTable = new List<UsgsAPICategory> 
        { //new UsgsAPICategory(){Title = "Elevation Products (3DEP)", SubTitle = "2 arc-second DEM - Alaska", Format = "none", Priority = 0, Type = "", SbDatasetTag = "National Elevation Dataset (NED) Alaska 2 arc-second"},
          new UsgsAPICategory(){Title = "Elevation Products (3DEP)", SubTitle = "1 meter DEM", Format = "IMG", Priority = 1, Classification = "Terrain", SbDatasetTag = "Digital Elevation Model (DEM) 1 meter"},
          new UsgsAPICategory(){Title = "Elevation Products (3DEP)", SubTitle = "1/9 arc-second DEM", Format = "IMG", Priority = 2, Classification = "Terrain", SbDatasetTag = "National Elevation Dataset (NED) 1/9 arc-second"},
          new UsgsAPICategory(){Title = "Elevation Products (3DEP)", SubTitle = "1/3 arc-second DEM", Format = "IMG", Priority = 3, Classification = "Terrain", SbDatasetTag = "National Elevation Dataset (NED) 1/3 arc-second"},
          new UsgsAPICategory(){Title = "Elevation Products (3DEP)", SubTitle = "1 arc-second DEM", Format = "IMG", Priority = 4, Classification = "Terrain", SbDatasetTag = "National Elevation Dataset (NED) 1 arc-second"},
          //new UsgsAPICategory(){Title = "Elevation Products (3DEP)", SubTitle = "1/3 arc-second Contours", Format = "Shapefile", Priority = 6, Classification = "Terrain", SbDatasetTag = "National Elevation Dataset (NED) 1/3 arc-second - Contours"},
          //new UsgsAPICategory(){Title = "Elevation Source Data (3DEP)", SubTitle = "DEM Source (OPR)", Format = "none", Priority = 0, Classification = ""},
          //new UsgsAPICategory(){Title = "Elevation Source Data (3DEP)", SubTitle = "Ifsar Digital Surface Model (DSM)", Format = "none", Priority = 0, Classification = ""},
          //new UsgsAPICategory(){Title = "Elevation Source Data (3DEP)", SubTitle = "Ifsar Orthorectified Radar Image (ORI)", Format = "none", Priority = 0, Classification = ""},
          //new UsgsAPICategory(){Title = "Elevation Source Data (3DEP)", SubTitle = "Lidar Point Cloud (LPC)", Format = "LAS", Priority = 1, Classification = "PointCloud", SbDatasetTag = "Lidar Point Cloud (LPC)"},
          //new UsgsAPICategory(){Title = "Hydrography (NHD) and Watersheds (WBD)", SubTitle = "National Hydrography Dataset (NHD) Best Resolution", Format = "Shapefile", Priority = 1, Classification = "WaterBody", SbDatasetTag = "National Hydrography Dataset (NHD) Best Resolution"},
          //new UsgsAPICategory(){Title = "Hydrography (NHD) and Watersheds (WBD)", SubTitle = "National Hydrography Dataset (NHD) Medium Resolution", Format = "Shapefile", Priority = 2, Classification = "WaterBody" SbDatasetTag = "National Hydrography Dataset (NHD) Medium Resolution"},
          //new UsgsAPICategory(){Title = "National Land Cover Database (NLCD)", SubTitle = "National Land Cover Database (NLCD) - 2011", Format = "GeoTIFF", Priority = 1, Classification = "Imagery", SbDatasetTag = "National Land Cover Database (NLCD) - 2011"},
          //new UsgsAPICategory(){Title = "National Land Cover Database (NLCD)", SubTitle = "National Land Cover Database (NLCD) - 2006", Format = "GeoTIFF", Priority = 2, Classification = "Imagery", SbDatasetTag = "National Land Cover Database (NLCD) - 2006"},
          //new UsgsAPICategory(){Title = "National Land Cover Database (NLCD)", SubTitle = "National Land Cover Database (NLCD) - 2001", Format = "GeoTIFF", Priority = 3, Classification = "Imagery", SbDatasetTag = "National Land Cover Database (NLCD) - 2001"},
          //new UsgsAPICategory(){Title = "Hydrography (NHD) and Watersheds (WBD)", SubTitle = "National Watershed Boundary Dataset (WBD)", Format = "Shapefile", Priority = 1, Classification = "WaterBody", SbDatasetTag = "National Watershed Boundary Dataset (WBD)"},
          //new UsgsAPICategory(){Title = "Boundaries - National Boundary Dataset", SubTitle = "Boundaries - National Boundary Dataset", Format = "none", Priority = 0, Classification = ""},
          new UsgsAPICategory(){Title = "Imagery - 1 foot (HRO)", SubTitle = "Imagery - 1 foot (HRO)", Format = "JPEG2000", Priority = 1, Classification = "Imagery", SbDatasetTag = "High Resolution Orthoimagery"},
          new UsgsAPICategory(){Title = "Imagery - 1 meter (NAIP)", SubTitle = "Imagery - 1 meter (NAIP)", Format = "JPEG2000", Priority = 1, Classification = "Imagery", SbDatasetTag = "USDA National Agriculture Imagery Program (NAIP)"},
          //new UsgsAPICategory(){Title = "Historical Topographic Maps", SubTitle = "Historical Topographic Maps", Format = "none", Priority = 0, Classification = ""},
          //new UsgsAPICategory(){Title = "Map Indices", SubTitle = "Map Indices", Format = "none", Priority = 0, Classification = ""},
          //new UsgsAPICategory(){Title = "Names - Geographic Names Information System (GNIS)", SubTitle = "Names - Geographic Names Information System (GNIS)", Format = "none", Priority = 0, Classification = ""},
          //new UsgsAPICategory(){Title = "Small-scale Datasets", SubTitle = "Small-scale Datasets", Format = "none", Priority = 0, Classification = ""},
          //new UsgsAPICategory(){Title = "Structures - National Structures Dataset", SubTitle = "Structures - National Structures Dataset", Format = "Shapefile", Priority = 2, Classification = "Bridge,Building", SbDatasetTag = "National Structures Dataset (NSD)"},
          //new UsgsAPICategory(){Title = "Transportation - National Transportation Dataset", SubTitle = "Transportation - National Transportation Dataset", Format = "Shapefile", Priority = 1, Classification = "Roadway", SbDatasetTag = "National Transportation Dataset (NTD)"},
          //new UsgsAPICategory(){Title = "US Topo", SubTitle = "US Topo", Format = "none", Priority = 0, Classification = ""},
          //new UsgsAPICategory(){Title = "Woodland Tint", SubTitle = "Woodland Tint", Format = "none", Priority = 0, Classification = ""}
        };

        ECQuery m_query;

        public USGSDataFetcher (ECQuery query)
            {
            m_query = query;
            }

        public List<UsgsAPICategory> CategoryTable
            {
            get
                {
                return m_categoryTable;
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
                    throw new OperationFailedException("USGS returned an error : " + response.ReasonPhrase);
                    }
                    }
                }
            //return null;
            }

        /// <summary>
        /// Returns the json of the item of specified source ID in the sciencebase catalog
        /// </summary>
        /// <param name="sourceID">The source ID of the item</param>
        /// <returns>The JObject read if the request was successful, null otherwise</returns>
        public JObject GetSciencebaseJson (string sourceID)
            {
            string url = "https://www.sciencebase.gov/catalog/item/" + sourceID + "?format=json";

            string jsonString = GetHttpResponse(url);
            if ( jsonString != null )
                {
                return JObject.Parse(jsonString) as JObject;
                }

            return null;
            }

        public XmlDocument GetXmlDocFromURL (string URL)
            {
            string xmlString = GetHttpResponse(URL);
            //using (XmlReader xmlReader = XmlReader.Create(xmlString))
            //{

            //}
            XmlDocument xmlDoc = new XmlDocument();
            xmlDoc.LoadXml(xmlString);
            return xmlDoc;
            }

        /// <summary>
        /// Queries USGS for data located in the selected bbox and in the predetermined datasets (see CategoryTable)
        /// </summary>
        /// <returns>Non formatted data (still in JSON form), along with the ID of the parent dataset. This ID was added there to prevent
        /// having to query USGS for every single data</returns>
        public IEnumerable<USGSRequestBundle> GetNonFormattedUSGSResults (List<SingleWhereCriteriaHolder> whereCriteriaList)
            {

            List<USGSRequestBundle> instanceList = new List<USGSRequestBundle>();

            string bbox = ExtractBboxFromQuery();

            //List<string> formatList = ExtractFormatList();
            List<string> formatList = null;
            

            List<UsgsRequest> reqList = new List<UsgsRequest>();

            List<String> requestedClassificationList = null;
            bool selectAllClasses = true;
            bool selectAllFormats = true;

            foreach ( var criteriaHolder in whereCriteriaList )
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
                //This doesn't work because there may be multiple datasourcetypes in this field for the database, so we cannot use the operator "IN" to be consistent
                //For now, we'll use the automatic filtering from WSG, even if it is less efficient to query all of instances from USGS then filtering, rather than filtering
                //with the USGS API. We could also go back to using ExtractFormatList with a custom query parameter, but it is not a good idea to use these parameters only for
                //the USGS API.
                //if ( (criteriaHolder.Operator == RelationalOperator.IN) && criteriaHolder.Property.Name == "DataSourceTypesAvailable" )
                //    {
                //    foreach ( var format in criteriaHolder.Value.Split(',') )
                //        {
                //        formatList.Add(format);
                //        selectAllFormats = false;
                //        }
                //    }
                }

            try
                {
                using ( HttpClient client = new HttpClient() )
                    {

                    client.Timeout = new TimeSpan(0, 0, 15);
                    //client.Timeout = new TimeSpan(15000);
                    using ( HttpResponseMessage response = client.GetAsync(WebReqCategories).Result )
                        {
                        if ( !response.IsSuccessStatusCode )
                            {
                            if(response.StatusCode == HttpStatusCode.ServiceUnavailable)
                                {
                                throw new OperationFailedException("USGS service unavailable for the moment. Please retry later.");
                                }
                            throw new OperationFailedException("USGS did not send a successful response.");
                            }
                        using ( HttpContent content = response.Content )
                            {
                            string responseString = content.ReadAsStringAsync().Result;

                            JArray json = JArray.Parse(responseString);

                            foreach ( var entry in json )
                                {
                                if ( entry["tags"].HasValues )
                                    {

                                    foreach ( JProperty subEntry in entry["tags"] )
                                        {
                                        System.Console.Out.Write(subEntry.Value["title"]);
                                        foreach ( var category in CategoryTable )
                                            {
                                            if ( entry["title"].Value<string>() == category.Title &&
                                               subEntry.Value["title"].Value<string>() == category.SubTitle &&
                                               category.Priority != 0 &&
                                               (selectAllFormats || formatList.Any(f => f.ToLower() == category.Format.ToLower())) &&
                                               (selectAllClasses || requestedClassificationList.Any(c => c == category.Classification)) )
                                                {
                                                UsgsRequest req = new UsgsRequest()
                                                {
                                                    //Dataset = subEntry.Value["sbDatasetTag"].Value<string>(),
                                                    Dataset = category.SbDatasetTag,
                                                    DatasetID = subEntry.Value["id"].Value<string>(),
                                                    Format = category.Format,
                                                    Category = category.Title,
                                                    Priority = category.Priority,
                                                    Classification = category.Classification
                                                };

                                                reqList.Add(req);
                                                break;
                                                }
                                            }
                                        }
                                    }
                                else
                                    {
                                    foreach ( var category in CategoryTable )
                                        {
                                        if ( entry["title"].Value<string>() == category.Title &&
                                           entry["title"].Value<string>() == category.SubTitle &&
                                           category.Priority != 0 &&
                                           (selectAllFormats || formatList.Any(f => f.ToLower() == category.Format.ToLower())) &&
                                           (selectAllClasses || requestedClassificationList.Any(c => c == category.Classification)) )
                                            {
                                            UsgsRequest req = new UsgsRequest()
                                            {
                                                //Dataset = entry["sbDatasetTag"].Value<string>(),
                                                Dataset = category.SbDatasetTag,
                                                DatasetID = entry["id"].Value<string>(),
                                                Format = category.Format,
                                                Category = category.Title,
                                                Priority = category.Priority,
                                                Classification = category.Classification
                                            };
                                            reqList.Add(req);
                                            break;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            catch ( TaskCanceledException )
                {
                //Request timed out. We return our empty list
                //return instanceList;
                throw new Bentley.Exceptions.EnvironmentalException("USGS request timed out");
                }
            catch ( System.AggregateException ex )
                {
                if ( (ex.InnerExceptions.Count == 1) && (ex.InnerException.GetType() == typeof(TaskCanceledException)) )
                    {
                    throw new Bentley.Exceptions.EnvironmentalException("USGS request timed out");
                    }
                else
                    {
                    throw;
                    }
                }
            reqList.Sort();

            //string curCat = "";

            object locker = new object();



            //TODO : We have deactivated the category feature for the parallel foreach we could reimplement it by storing all results in a temporary list of list, 
            //       then process it to have the same list we would have had before
            Parallel.ForEach(reqList, req =>
            {
                //if (curCat == req.Category)
                //{
                //    continue;
                //}

                string readyToSend = WebReq.Replace("_bbox", bbox).Replace("_datasets", req.Dataset).Replace("_prodFormats", req.Format).Replace(' ', '+').Replace("_maxResults", MaxResults);

                try
                    {
                    using ( HttpClient client = new HttpClient() )
                        {
                        client.Timeout = new TimeSpan(0, 0, 15);
                        using ( HttpResponseMessage response = client.GetAsync(readyToSend).Result )
                            {
                            if ( response.IsSuccessStatusCode )
                                {
                                using ( HttpContent content = response.Content )
                                    {
                                    string responseString = content.ReadAsStringAsync().Result;

                                    JObject jsonResp = JObject.Parse(responseString);

                                    lock ( locker )
                                        {
                                        instanceList.Add(new USGSRequestBundle
                                        {
                                            jtokenList = jsonResp["items"], DatasetId = req.DatasetID, Dataset = req.Dataset, Classification = req.Classification
                                        });
                                        //foreach (var item in jsonResp["items"] as JArray)
                                        //{
                                        //    instanceList.Add(item);

                                        //}
                                        }

                                    }
                                }
                            }
                        }
                    }
                catch ( TaskCanceledException )
                    {
                    //Request timed out. We return our empty list
                    //return instanceList;
                    throw new Bentley.Exceptions.EnvironmentalException("USGS request timed out");
                    }
                catch ( System.AggregateException ex )
                    {
                    if ( (ex.InnerExceptions.Count == 1) && (ex.InnerException.GetType() == typeof(TaskCanceledException)) )
                        {
                        throw new Bentley.Exceptions.EnvironmentalException("USGS request timed out");
                        }
                    else
                        {
                        throw ex;
                        }
                    }
            });

            return instanceList;
            }

        private string ExtractBboxFromQuery ()
            {
            if ( !m_query.ExtendedData.ContainsKey("polygon") )
                {
                throw new UserFriendlyException("This request must contain a \"polygon\" parameter in the form of a WKT polygon string.");
                }

            string polygonString = m_query.ExtendedData["polygon"].ToString();
            PolygonModel model = DbGeometryHelpers.CreatePolygonModelFromJson(polygonString);

            string polygonWKT = DbGeometryHelpers.CreateWktPolygonString(model.points);

            PolygonDescriptor polyDesc = new PolygonDescriptor
            {
                WKT = polygonWKT,
                SRID = model.coordinate_system
            };

            //We should now extract a bbox from this wkt

            return DbGeometryHelpers.ExtractBboxFromWKTPolygon(polyDesc.WKT);

            }

        ///// <summary>
        ///// Extracts all formats from the extended data key "format". 
        ///// </summary>
        ///// <returns>Null if there is no format key, otherwise list of all formats requested</returns>
        //private List<string> ExtractFormatList ()
        //    {
        //    if ( !m_query.ExtendedData.ContainsKey("format") )
        //        {
        //        return null;
        //        }
        //    string formatString = m_query.ExtendedData["format"].ToString();

        //    string[] formatArray = formatString.Split(new char[] { ',' }, StringSplitOptions.RemoveEmptyEntries);
        //    List<string> formatList = new List<string>();

        //    foreach ( var format in formatArray )
        //        {
        //        formatList.Add(format.Trim());
        //        }

        //    return formatList;
        //    }
        }
    }
