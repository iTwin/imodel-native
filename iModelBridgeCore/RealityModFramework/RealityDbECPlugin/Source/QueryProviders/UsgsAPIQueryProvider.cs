using Bentley.EC.Persistence.Query;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using Bentley.Exceptions;
using IndexECPlugin.Source;
using IndexECPlugin.Source.Helpers;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;


namespace IndexECPlugin.Source.QueryProviders
{

    internal class UsgsAPIQueryProvider : IECQueryProvider
    {

        const string WebReqCategories = "http://viewer.nationalmap.gov/tnmaccess/api/searchCategories?";
        const string WebReq = "http://viewer.nationalmap.gov/tnmaccess/api/searchProducts?bbox=_bbox&q=&start=&end=&dateType=&datasets=_datasets&prodFormats=_prodFormats&prodExtents=&polyCode=&polyType=&max=200&offset=0";

        private readonly List<UsgsAPICategory> CategoryTable = new List<UsgsAPICategory> 
        { new UsgsAPICategory(){Title = "Elevation Products (3DEP)", SubTitle = "2 arc-second DEM - Alaska", Format = "none", Priority = 0},
          new UsgsAPICategory(){Title = "Elevation Products (3DEP)", SubTitle = "1 meter DEM", Format = "IMG", Priority = 1},
          new UsgsAPICategory(){Title = "Elevation Products (3DEP)", SubTitle = "1/9 arc-second DEM", Format = "IMG", Priority = 2},
          new UsgsAPICategory(){Title = "Elevation Products (3DEP)", SubTitle = "1/3 arc-second DEM", Format = "IMG", Priority = 3},
          new UsgsAPICategory(){Title = "Elevation Products (3DEP)", SubTitle = "1 arc-second DEM", Format = "IMG", Priority = 4},
          new UsgsAPICategory(){Title = "Elevation Products (3DEP)", SubTitle = "1/3 arc-second Contours", Format = "Shapefile", Priority = 6},
          new UsgsAPICategory(){Title = "Elevation Source Data (3DEP)", SubTitle = "DEM Source (OPR)", Format = "none", Priority = 0},
          new UsgsAPICategory(){Title = "Elevation Source Data (3DEP)", SubTitle = "Ifsar Digital Surface Model (DSM)", Format = "none", Priority = 0},
          new UsgsAPICategory(){Title = "Elevation Source Data (3DEP)", SubTitle = "Ifsar Orthorectified Radar Image (ORI)", Format = "none", Priority = 0},
          new UsgsAPICategory(){Title = "Elevation Source Data (3DEP)", SubTitle = "Lidar Point Cloud (LPC)", Format = "LAS", Priority = 1},
          new UsgsAPICategory(){Title = "Hydrography (NHD) and Watersheds (WBD)", SubTitle = "National Hydrography Dataset (NHD) Best Resolution", Format = "Shapefile", Priority = 1},
          new UsgsAPICategory(){Title = "Hydrography (NHD) and Watersheds (WBD)", SubTitle = "National Hydrography Dataset (NHD) Medium Resolution", Format = "Shapefile", Priority = 2},
          new UsgsAPICategory(){Title = "National Land Cover Database (NLCD)", SubTitle = "National Land Cover Database (NLCD) - 2011", Format = "GeoTIFF", Priority = 1},
          new UsgsAPICategory(){Title = "National Land Cover Database (NLCD)", SubTitle = "National Land Cover Database (NLCD) - 2006", Format = "GeoTIFF", Priority = 2},
          new UsgsAPICategory(){Title = "National Land Cover Database (NLCD)", SubTitle = "National Land Cover Database (NLCD) - 2001", Format = "GeoTIFF", Priority = 3},
          new UsgsAPICategory(){Title = "Hydrography (NHD) and Watersheds (WBD)", SubTitle = "National Watershed Boundary Dataset (WBD)", Format = "Shapefile", Priority = 1},
          new UsgsAPICategory(){Title = "Boundaries - National Boundary Dataset", SubTitle = "Boundaries - National Boundary Dataset", Format = "none", Priority = 0},
          new UsgsAPICategory(){Title = "Imagery - 1 foot (HRO)", SubTitle = "Imagery - 1 foot (HRO)", Format = "JPEG2000", Priority = 1},
          new UsgsAPICategory(){Title = "Imagery - 1 meter (NAIP)", SubTitle = "Imagery - 1 meter (NAIP)", Format = "JPEG2000", Priority = 1},
          new UsgsAPICategory(){Title = "Historical Topographic Maps", SubTitle = "Historical Topographic Maps", Format = "none", Priority = 0},
          new UsgsAPICategory(){Title = "Map Indices", SubTitle = "Map Indices", Format = "none", Priority = 0},
          new UsgsAPICategory(){Title = "Names - Geographic Names Information System (GNIS)", SubTitle = "Names - Geographic Names Information System (GNIS)", Format = "none", Priority = 0},
          new UsgsAPICategory(){Title = "Small-scale Datasets", SubTitle = "Small-scale Datasets", Format = "none", Priority = 0},
          new UsgsAPICategory(){Title = "Structures - National Structures Dataset", SubTitle = "Structures - National Structures Dataset", Format = "Shapefile", Priority = 2},
          new UsgsAPICategory(){Title = "Transportation - National Transportation Dataset", SubTitle = "Transportation - National Transportation Dataset", Format = "Shapefile", Priority = 1},
          new UsgsAPICategory(){Title = "US Topo", SubTitle = "US Topo", Format = "none", Priority = 0},
          new UsgsAPICategory(){Title = "Woodland Tint", SubTitle = "Woodland Tint", Format = "none", Priority = 0}
        };

        ECQuery m_query;

        public UsgsAPIQueryProvider(ECQuery query)
        {
            m_query = query;
        }

        //This method will map to other methods, depending on the search class in the query
        public IEnumerable<IECInstance> CreateInstanceList()
        {
            string className = m_query.SearchClasses.First().Class.Name;

            switch (className)
            {
                case "USGSEntity":
                    return QueryUSGSEntity();
                //case "USGSThumbnail":
                //    return QueryUSGSThumbnail();
                default:
                    throw new UserFriendlyException("It is impossible to query instances of the class \"" + className + "\" ");
            }
        }

        private IEnumerable<IECInstance> QueryUSGSEntity()
        {
            List<IECInstance> instanceList = new List<IECInstance>();

            string bbox = ExtractBboxFromQuery();

            using (HttpClient client = new HttpClient())
            {
                List<UsgsRequest> reqList = new List<UsgsRequest>();
                using (HttpResponseMessage response = client.GetAsync(WebReqCategories).Result)
                {
                    using (HttpContent content = response.Content)
                    {
                        string responseString = content.ReadAsStringAsync().Result;

                        var json = JsonConvert.DeserializeObject(responseString) as JArray;

                        foreach (var entry in json)
                        {
                            if (entry["tags"].HasValues)
                            {

                                foreach (JProperty subEntry in entry["tags"])
                                {
                                    System.Console.Out.Write(subEntry.Value["title"]);
                                    foreach (var category in CategoryTable)
                                    {
                                        if (entry["title"].Value<string>() == category.Title &&
                                           subEntry.Value["title"].Value<string>() == category.SubTitle &&
                                           category.Priority != 0)
                                        {
                                            UsgsRequest req = new UsgsRequest()
                                            {
                                                Dataset = subEntry.Value["sbDatasetTag"].Value<string>(),
                                                Format = category.Format,
                                                Category = category.Title,
                                                Priority = category.Priority
                                            };

                                            reqList.Add(req);
                                            break;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                foreach (var category in CategoryTable)
                                {
                                    if (entry["title"].Value<string>() == category.Title &&
                                       entry["title"].Value<string>() == category.SubTitle &&
                                       category.Priority != 0)
                                    {
                                        UsgsRequest req = new UsgsRequest()
                                        {
                                            Dataset = entry["sbDatasetTag"].Value<string>(),
                                            Format = category.Format,
                                            Category = category.Title,
                                            Priority = category.Priority
                                        };
                                    }
                                }
                            }
                        }
                    }
                }
                reqList.Sort();

                string curCat = "";

                //Since each of these requests can be long (especially when the server is down), 
                //it might be a good idea to use multiple threads (or parallel foreach?) and add a timeout when launching these queries.
                //There are HttpClient.GetAsync method overloads using a cancellation token, this could be used for a timeout mechanism.
                foreach (var req in reqList)
                {
                    if (curCat == req.Category)
                    {
                        continue;
                    }

                    string readyToSend = WebReq.Replace("_bbox", bbox).Replace("_datasets", req.Dataset).Replace("_prodFormats", req.Format).Replace(' ', '+');

                    using (HttpResponseMessage response = client.GetAsync(readyToSend).Result)
                    {
                        if (response.IsSuccessStatusCode)
                        {
                            using (HttpContent content = response.Content)
                            {
                                string responseString = content.ReadAsStringAsync().Result;

                                var jsonResp = JsonConvert.DeserializeObject(responseString) as JObject;

                                foreach (var item in jsonResp["items"] as JArray)
                                {

                                    //USGSPayload payload = new USGSPayload() 
                                    //  { 
                                    //      Title = item["title"].Value<string>(),
                                    //      PreviewLink = item["previewGraphicURL"].Value<string>(),
                                    //      DownloadLink = item["downloadURL"].Value<string>(),
                                    //      BoundingBox = String.Format("{0},{1},{2},{3}", item["boundingBox"]["minX"].Value<string>(), item["boundingBox"]["minY"].Value<string>(), item["boundingBox"]["maxX"].Value<string>(), item["boundingBox"]["maxY"].Value<string>())
                                    //  };

                                    //byte[] ba = System.Text.Encoding.ASCII.GetBytes(JsonConvert.SerializeObject(payload));
                                    //string hexstring = BitConverter.ToString(ba);
                                    //hexstring = hexstring.Replace("-", "");

                                    IECClass ecClass = m_query.SearchClasses.First().Class;
                                    IECInstance instance = ecClass.CreateInstance();
                                    instance["Title"].StringValue = item["title"].Value<string>();
                                    instance["PreviewLink"].StringValue = item["previewGraphicURL"].Value<string>();
                                    instance["DownloadLink"].StringValue = item["downloadURL"].Value<string>();
                                    instance["BoundingBox"].StringValue = String.Format("{0},{1},{2},{3}", item["boundingBox"]["minX"].Value<string>(), item["boundingBox"]["minY"].Value<string>(), item["boundingBox"]["maxX"].Value<string>(), item["boundingBox"]["maxY"].Value<string>());
                                    //((ECBinaryValue)instance["Payload"]).BinaryValue = System.Text.Encoding.Default.GetBytes(JsonConvert.SerializeObject(payload));
                                    //instance["Payload"].StringValue = hexstring;
                                    instance.InstanceId = item["sourceId"].Value<string>();
                                    //Lock here if parallel???
                                    instanceList.Add(instance);
                                    
                                }

                            }
                        }
                    }
                }
            }
            return instanceList;
        }

        //private IEnumerable<IECInstance> QueryUSGSThumbnail()
        //{
        //    List<IECInstance> instanceList = new List<IECInstance>();

        //    //Byte[] bytePayload = ExtractPayload();

        //    //string jsonString = Encoding.ASCII.GetString(bytePayload);

        //    //USGSPayload payload = JsonConvert.DeserializeObject<USGSPayload>(jsonString);

        //    IECClass ecClass = m_query.SearchClasses.First().Class;
        //    IECInstance instance = ecClass.CreateInstance();
        //    instance["ThumbnailURL"].StringValue = payload.PreviewLink;


        //    using (HttpClient client = new HttpClient())
        //    {
        //        using (HttpResponseMessage response = client.GetAsync(payload.PreviewLink).Result)
        //        {
        //            using (HttpContent content = response.Content)
        //            {
        //                ((ECBinaryValue)instance["ThumbnailData"]).BinaryValue = content.ReadAsByteArrayAsync().Result;
        //            }
        //        }
        //    }



        //    instanceList.Add(instance);

        //    return instanceList;
        //}

        //private byte[] ExtractPayload()
        //{
        //    if (!m_query.ExtendedData.ContainsKey("payload"))
        //    {
        //        throw new UserFriendlyException("This request must contain a \"payload\" parameter in the form of a WKT polygon string.");
        //    }
        //    //throw new Exception(m_query.ExtendedData["payload"].GetType().FullName);
        //    string hexString = m_query.ExtendedData["payload"].ToString();

        //    int hexLength = hexString.Length;
        //    byte[] bytes = new byte[hexLength / 2];
        //    for (int i = 0; i < hexLength; i += 2)
        //        bytes[i / 2] = Convert.ToByte(hexString.Substring(i, 2), 16);
        //    return bytes;

        //}

        private string ExtractBboxFromQuery()
        {
            if (!m_query.ExtendedData.ContainsKey("polygon"))
            {
                throw new UserFriendlyException("This request must contain a \"polygon\" parameter in the form of a WKT polygon string.");
            }

            string polygonString = m_query.ExtendedData["polygon"].ToString();
            PolygonModel model;
            try
            {
                model = JsonConvert.DeserializeObject<PolygonModel>(polygonString);
            }
            catch (JsonSerializationException)
            {
                throw new UserFriendlyException("The polygon format is not valid.");
            }

            int polygonSRID;
            if (!int.TryParse(model.coordinate_system, out polygonSRID))
            {
                throw new UserFriendlyException("The polygon format is not valid.");
            }

            string polygonWKT = DbGeometryHelpers.CreateWktPolygonString(model.points);

            PolygonDescriptor polyDesc = new PolygonDescriptor
            {
                WKT = polygonWKT,
                SRID = polygonSRID
            };

            //We should now extract a bbox from this wkt

            return DbGeometryHelpers.ExtractBboxFromWKTPolygon(polyDesc.WKT);

        }
    }
}
