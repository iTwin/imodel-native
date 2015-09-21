/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/IndexECPlugin.cs $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/

using Bentley.Collections;
using Bentley.EC.Persistence;
using Bentley.EC.Persistence.FileSystemResource;
using Bentley.EC.Persistence.Operations;
using Bentley.EC.Persistence.Query;
using Bentley.EC.PluginBuilder;
using Bentley.EC.PluginBuilder.Modules;
using Bentley.EC.PluginBuilder.SchemaManagement;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using Bentley.ECObjects.XML;
using Bentley.ECSystem.Extensibility;
using Bentley.ECSystem.Repository;
using Bentley.ECSystem.Session;
using Bentley.Exceptions;
using RealityDataPackageWrapper;
using IndexECPlugin.Source;
using IndexECPlugin.Source.FileRetrievalControllers;
using IndexECPlugin.Source.Helpers;
using IndexECPlugin.Source.QueryProviders;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Data.SqlClient;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Runtime.InteropServices;
using System.Xml;

namespace Bentley.ECPluginExamples
{
    /*====================================================================================**/
    /// <summary>
    /// Demonstrates how to create a file info ECPlugin using the ECPluginBuilder.
    /// </summary>
    /// <remarks>
    /// This is a very simple example using delegates to query files in a specific directory
    /// </remarks>
    /*==============+===============+===============+===============+===============+======*/
    [ComVisible(false)]
    [ECExtension]
    public class IndexECPlugin : ECStartupShutdownHandler
    {
        /// <summary>
        /// Instance of plugin.
        /// </summary>
        private static ECPlugin m_plugin = null;

        /// <summary>
        /// Name we will use for our ECSchema
        /// </summary>
        /// <remarks>This is only made public to simplify testing.</remarks>
        public const string SCHEMA_NAME = "RealityModeling";

        /// <summary>
        /// Name we will use for our ECPlugin
        /// </summary>
        /// <remarks>This is only made public to simplify testing.</remarks>
        public const string PLUGIN_NAME = "IndexECPlugin";

        private string m_connectionString = "";
        private string m_schemaLocation = "";
        private string m_packagesLocation = "";

        /// <summary>
        /// Make plugin available to contract tests.
        /// </summary>
        /// <remarks>Registered plugin.  See <see cref="Startup()"/>.</remarks>
        public ECPlugin Plugin
        {
            get
            {
                Startup();
                return m_plugin;
            }
        }

        /// <summary>
        /// Override this method to register your ECPlugin
        /// </summary>
        protected override void Startup()
        {
            // Only actually register plugin once.
            if (null == m_plugin)
            {
                //Start by calling the Register() method which takes a delegate that defines what components the plugin should be composed of
                m_plugin = ECPlugin.Register(PLUGIN_NAME, BuildPlugin);
            }
        }
        
        /// <summary>
        /// Actual builder method that composes our ECPlugin
        /// </summary>
        /// <param name="builder">builder to use for construction</param>
        private void BuildPlugin(ECPluginBuilder builder)
        {
            builder
                .SetDisplayLabel("IndexECPlugin") //Set a display label for the plugin
                .SetRepositorySupport(
                    new LocationFormatInfo()
                        {
                            BrowsingType = ConnectionFormatInfo.LocationBrowsingType.BrowseFolders
                        },
                    GetRepositoryIdentifier)
                .SetSchemaSupport(PopulateSchemas)
                //.SetConnectionSupport(GetConnectionFormat, OpenConnection, CloseConnection)
                .SetQuerySupport((EnumerableBasedQueryHandler)ExecuteQuery)
                .SetOperationSupport<RetrieveBackingFileOperation>(FileRetrievalOperation)
                .SetOperationSupport<InsertOperation>(ExecuteInsertOperation);
            
                IndexPolicyHandler.InitializeHandlers(builder);
            
        }

        private RepositoryIdentifier GetRepositoryIdentifier(RepositoryModule module,
                                                             ECSession session,
                                                             string location,
                                                             IExtendedParameters extendedParameters)
        {
            //Try to obtain the repository identifier for the provided location from the list of known identifiers
            ICollection<RepositoryIdentifier> knownRepositories = module.GetKnownRepositories(session, extendedParameters);
            RepositoryIdentifier knownRepository = knownRepositories.FirstOrDefault((repositoryIdentifier) =>
                string.Equals(location, repositoryIdentifier.Location, StringComparison.Ordinal));

            if (knownRepository != null)
            {
                return knownRepository;
            }



            ////we did not find an existing identifier, check if the directory on the location exists
            if (!File.Exists(location))
            {
                throw new LocationNotFoundException("The location given was not a valid location file");
            }

            ParseConfigFile(location);

            var identifier = new RepositoryIdentifier(module.ParentECPlugin.ECPluginId, location, location, location, null);
            module.RepositoryVerified(session, identifier); //tells the module to include that repository in the list of known repositories from now on
            return identifier;
            //}
        }

        private void ParseConfigFile(string location)
        {
            XmlDocument doc = new XmlDocument();
            try
            {
                doc.Load(location);
            }
            catch(Exception ex)
            {
                throw new NotSupportedException(String.Format("An error happened while loading the file at the location {0}", location), ex);
            }

            XmlNodeList nodelist = doc.SelectNodes("configuration");
            if(nodelist.Count != 1)
            {
                throw new NotSupportedException("There can be one and only one \"configuration\" section in the configuration file");
            }

            
            m_connectionString = nodelist[0].SelectSingleNode("ConnectionString").InnerText;
            m_schemaLocation = nodelist[0].SelectSingleNode("SchemaLocation").InnerText;
            m_packagesLocation = nodelist[0].SelectSingleNode("PackagesLocation").InnerText;
            //throw new Exception(String.Format("Testing : {0} {1} {2}", m_connectionString, m_schemaLocation, m_packagesLocation));

        }

        private void PopulateSchemas(SchemaModule module,
                                     ECPluginSchemaManager schemaManager)
        {
            //*****Following lines show how to read an XML schema and add it.*****

            //TODO : Code a real procedure to load a schema
            ECSchemaXmlStringReader schemaReader = new ECSchemaXmlStringReader(File.ReadAllText(m_schemaLocation));
            IECSchema schemaFromXML = schemaReader.Deserialize();
            schemaManager.AddSchema(ref schemaFromXML);

        }

        private IEnumerable<IECInstance> ExecuteQuery(QueryModule sender,
                                                      RepositoryConnection connection,
                                                      ECQuery query,
                                                      ECQuerySettings querySettings)
        {
            //For now, we'll only manage the case where we receive one search class.
            //foreach (SearchClass searchClass in query.SearchClasses)
            SearchClass searchClass = query.SearchClasses.First();
            {
                //TODO : code a real procedure to fetch our connection string

                    IECQueryProvider helper/* = new SqlQueryProvider(query)*/;
                    
                    if (searchClass.Class.GetCustomAttributes("QueryType") == null || searchClass.Class.GetCustomAttributes("QueryType")["QueryType"].IsNull)
                    {
                        throw new UserFriendlyException(String.Format("The class {0} cannot be queried.", searchClass.Class.Name));
                    }
                    
                    //string queryType = searchClass.Class.GetCustomAttributes("QueryType")["QueryType"].StringValue;

                    string source;

                    if(query.ExtendedData.ContainsKey("source"))
                    {
                        source = query.ExtendedData["source"].ToString();
                    }
                    else
                    {
                        //TODO : We should rename queryType, as it has taken the role of a default parameter
                        source = searchClass.Class.GetCustomAttributes("QueryType")["QueryType"].StringValue;
                    }
                    //IEnumerable<IECInstance> instanceList;

                    switch (source.ToLower())
                    {
                        case "index":
                            using (SqlConnection sqlConnection = new SqlConnection(m_connectionString))
                            {
                                SchemaHelper schemaHelper = new SchemaHelper(sender.ParentECPlugin.SchemaModule, connection, SCHEMA_NAME); 
                                helper = new SqlQueryProvider(query, querySettings, sqlConnection, schemaHelper);
                                return helper.CreateInstanceList();
                            }

                        case "usgsapi":
                            helper = new UsgsAPIQueryProvider(query, querySettings);
                            return helper.CreateInstanceList();

                        case "all":
                            List<IECInstance> instanceList = new List<IECInstance>();
                            using (SqlConnection sqlConnection = new SqlConnection(m_connectionString))
                            {
                                SchemaHelper schemaHelper = new SchemaHelper(sender.ParentECPlugin.SchemaModule, connection, SCHEMA_NAME);
                                helper = new SqlQueryProvider(query, querySettings, sqlConnection, schemaHelper);
                                instanceList = helper.CreateInstanceList().ToList();
                            }

                            helper = new UsgsAPIQueryProvider(query, querySettings);
                            instanceList.AddRange(helper.CreateInstanceList());
                            return instanceList;
                        default:
                            //throw new UserFriendlyException(String.Format("The class {0} cannot be queried.", searchClass.Class.Name));
                            throw new UserFriendlyException("The source \"" + source + "\" does not exist. Choose between \"index\", \"usgsapi\" or \"all\"");
                    }


                ////SqlCommand cmd = helper.;


                ////***************************FOR NOW, WE ONLY TAKE THE ADDRESS OF THE API**************************
                ////We suppose that the location will be of the form : "Adress_Of_The_API,Location_Of_The_Repository"
                ////string baseURL = connection.RepositoryIdentifier.Location.Split(',')[0];
                //////string baseURL = "https://localhost/upload/Api/BentleyFiles/";
                ////*************************************************************************************************
                //string baseURL = connection.RepositoryIdentifier.Location;
                //string apiMethodURL = "GetJSonOfAllFilesInQuery";
                //int pageSize = -1;
                //if (query.ExtendedData.ContainsKey("pagesize"))
                //{
                //    if (int.TryParse(query.ExtendedData["pagesize"].ToString(), out pageSize))
                //    {
                //        throw new UserFriendlyException("Please enter a valid number for the pagesize parameter");
                //    }
                //}

                //int page = 1;
                //if (query.ExtendedData.ContainsKey("page"))
                //{
                //    if (int.TryParse(query.ExtendedData["page"].ToString(), out page))
                //    {
                //        throw new UserFriendlyException("Please enter a valid number for the page parameter");
                //    }
                //}

                //if (!query.ExtendedData.ContainsKey("polygon"))
                //{
                //    throw new UserFriendlyException("Please specify a polygon parameter for this query");
                //}
                //string polygonPointsVar = query.ExtendedData["polygon"].ToString();

                //string whereClauseString = query.WhereClause.ToString();
                //whereClauseString = whereClauseString.Replace("(", "").Replace(")", "");

                //return ReadInstancesFromAPI(bentleyFileClass, baseURL + apiMethodURL, polygonPointsVar, whereClauseString, pageSize, page, testToErase, testToErase2);

            }
        }





        //private static IEnumerable<IECInstance> ReadInstancesFromAPI(IECClass bentleyFileClass, string URL, string polygonPointsVar, string whereClauseString, int pageSize, int page, IECClass testToErase, IECRelationshipClass testToErase2)
        //{

        //    using (var client = new HttpClient())
        //    {
        //        //try
        //        //{

        //        Object content = new
        //        {
        //            polygonPoints = polygonPointsVar,
        //            whereClause = whereClauseString,
        //            itemsByPage = pageSize,
        //            pageNumber = page
        //        };

        //        var serializedContent = JsonConvert.SerializeObject(content);
        //        HttpContent httpContent = new StringContent(serializedContent, Encoding.UTF8, "application/json");

        //        var relatedInstanceTest = testToErase.CreateInstance();
        //        relatedInstanceTest.InstanceId = "testing";
        //        relatedInstanceTest["Name"].StringValue = "testing";

        //            ServicePointManager.ServerCertificateValidationCallback += (sender, cert, chain, sslPolicyErrors) => true; //This is necessary if the ssl certificate of the API is not valid...
        //            var response = client.PostAsync(URL, httpContent).Result;
        //            ServicePointManager.ServerCertificateValidationCallback -= (sender, cert, chain, sslPolicyErrors) => true;
        //            if (response.IsSuccessStatusCode)
        //            {
        //                string resultContent = response.Content.ReadAsStringAsync().Result;
        //                JsonResults summary = JsonConvert.DeserializeObject<JsonResults>(resultContent);
        //                return summary.Results.Select((model) =>
        //                {

        //                    var instance = bentleyFileClass.CreateInstance();
        //                    instance.InstanceId = model.ID.ToString();
        //                    instance["Name"].StringValue = model.Name;
        //                    instance["ID"].IntValue = model.ID;
        //                    instance["ThumbnailLocation"].StringValue = model.ThumbnailLocation;

        //                    var relationInstance = testToErase2.CreateInstance() as ECRelationshipInstance;
        //                    if(relationInstance == null)
        //                    {
        //                        throw new UserFriendlyException("Your test failed!!!");
        //                    }
        //                    relationInstance.Target = instance;
        //                    relationInstance.Source = relatedInstanceTest;
        //                    relationInstance.InstanceId = "testRelationship";

        //                    instance.GetRelationshipInstances().Add(relationInstance);
        //                    return instance;
        //                });
        //            }
        //            else
        //            {
        //                throw new UserFriendlyException("There was a problem with the request.");
        //            }
        //        //}
        //        //catch(Exception)
        //        //{
        //        //    return null;
        //        //}
        //    }
        //}

        private void FileRetrievalOperation
            (OperationModule sender,
            RepositoryConnection connection,
            RetrieveBackingFileOperation operation,
            IECInstance instance,
            string comments,
            WriteModifiers writeModifiers,
            IExtendedParameters extendedParameters)
        {
            IECClass instanceClass = instance.ClassDefinition;

            IECInstance fileHolderAttribute = instanceClass.GetCustomAttributes("FileHolder");

            if (fileHolderAttribute == null)
            {
                throw new UserFriendlyException(String.Format("There is no file associated to the {0} class", instanceClass.Name));
            }



            switch (fileHolderAttribute["Type"].StringValue)
            {

                case "PreparedPackage":

                    var resourceManager = new FileResourceManager(connection);
                    FileBackedDescriptorAccessor.SetIn(instance, new FileBackedDescriptor(""));
                    var packageRetrievalController = new PackageRetrievalController(instance, resourceManager, operation, m_packagesLocation);
                    packageRetrievalController.Run();
                    break;
                case "SQLThumbnail":
                    //var sqlThumbnailRetrievalController = new SQLThumbnailRetrievalController(...);
                    //sqlThumbnailRetrievalController.Run();
                    break;
                case "USGSThumbnail":

                    //var usgsThumbnailRetrievalController = new USGSThumbnailRetrievalController(instance);
                    //usgsThumbnailRetrievalController.processThumbnailRetrieval();
                    break;
                default:
                    throw new ProgrammerException("This type of file holder attribute is not implemented");
            }

            //if (instance.ClassDefinition.Name == "BentleyFile")
            //{

            //}
            //else
            //{
            //    throw new UserFriendlyException("Only BentleyFile instances are backed by files");
            //}
        }

        internal void ExecuteInsertOperation
        (
        OperationModule sender,
        RepositoryConnection connection,
        InsertOperation operation,
        IECInstance instance,
        string comments,
        WriteModifiers writeModifiers,
        IExtendedParameters extendedParameters
        )
        {
            //For now, we only "insert" objects of the PackageRequest class

            QueryModule queryModule = sender.ParentECPlugin.QueryModule;

            if(instance.ClassDefinition.Name != "PackageRequest")
            {
                throw new UserFriendlyException("The only insert operation permitted is a PackageResquest instance insertion.");
            }

            //TODO: Implement a package creation procedure that isn't hard coded for a particular schema... Probably impossible.
            
            string coordinateSystem = null;

            var csPropValue = instance.GetPropertyValue("CoordinateSystem");

            if ((csPropValue != null) && (!csPropValue.IsNull))
            {
                coordinateSystem = instance.GetPropertyValue("CoordinateSystem").StringValue;
            }

            IECArrayValue requestedEntitiesECArray = instance.GetPropertyValue("RequestedEntities") as IECArrayValue;
            if(requestedEntitiesECArray == null)
            {
                throw new ProgrammerException("The ECSchema is not valid. PackageRequest must have an array property");
            }

            //List<RequestedEntity> bentleyFileInfoList = new List<RequestedEntity>();
            List<RequestedEntity> dbRequestedEntities = new List<RequestedEntity>();
            List<RequestedEntity> usgsRequestedEntities = new List<RequestedEntity>();
            for(int i = 0; i < requestedEntitiesECArray.Count; i++)
            {

                var requestedEntity = ECStructToRequestedEntity(requestedEntitiesECArray[i] as IECStructValue);

                if (requestedEntity.ID.Length != IndexConstants.USGSIdLenght)
                {
                    dbRequestedEntities.Add(requestedEntity);
                }
                else
                {
                    usgsRequestedEntities.Add(requestedEntity);
                }
            }

            List<WmsSourceNet> wmsSourceList = IndexPackager(sender, connection, queryModule, coordinateSystem, dbRequestedEntities);

            List<UsgsSourceNet> usgsSourceList = UsgsPackager(sender, connection, queryModule, usgsRequestedEntities);

            // Create package bounding box (region of interest).
            List<double> selectedRegion = new List<double>();

            string selectedRegionStr = instance.GetPropertyValue("Polygon").StringValue;

            selectedRegion = selectedRegionStr.Split(new char[] { ',', '[', ']' }, StringSplitOptions.RemoveEmptyEntries).Select(str => Convert.ToDouble(str)).ToList();

            // Create data group and package.
            string name = Guid.NewGuid().ToString();
            try
            {
                // Create imagery group.
                ImageryGroupNet imgGroup = ImageryGroupNet.Create ();
                foreach (WmsSourceNet wmsSource in wmsSourceList)
                    {
                    imgGroup.AddData (wmsSource);
                    }
                foreach (UsgsSourceNet usgsSource in usgsSourceList)
                    {
                    imgGroup.AddData (usgsSource);
                    }

                // Create model group.
                ModelGroupNet modelGroup = ModelGroupNet.Create ();

                // Create pinned group.
                PinnedGroupNet pinnedGroup = PinnedGroupNet.Create ();

                // Create terrain group.
                TerrainGroupNet terrainGroup = TerrainGroupNet.Create ();

                // Create package.
                string description = "";
                string copyright = "";
                RealityDataPackageNet.Create (m_packagesLocation, name, description, copyright, selectedRegion, imgGroup, modelGroup, pinnedGroup, terrainGroup);
            }
            catch (Exception e)
            {
                throw new UserFriendlyException("There was a problem with the processing of the order.", e);
            }
            instance.InstanceId = name + ".xrdp";
        }

        private List<UsgsSourceNet> UsgsPackager(OperationModule sender, RepositoryConnection connection, QueryModule queryModule, List<RequestedEntity> usgsRequestedEntities)
        {
            List<UsgsSourceNet> usgsSourceNetList = new List<UsgsSourceNet>();

            if(usgsRequestedEntities.Count == 0)
            {
                return usgsSourceNetList;
            }

            IECClass dataSourceClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "SpatialDataSource");
            IECClass metadataClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "Metadata");

            ECQuery query = new ECQuery(dataSourceClass);
            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties = new List<IECProperty>();
            query.SelectClause.SelectedProperties.Add(dataSourceClass.First(prop => prop.Name == "Metadata"));
            query.SelectClause.SelectedProperties.Add(dataSourceClass.First(prop => prop.Name == "MainURL"));
            query.SelectClause.SelectedProperties.Add(dataSourceClass.First(prop => prop.Name == "DataSourceType"));

            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression(usgsRequestedEntities.Select(e => e.ID.ToString()).ToArray()));

            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));

            var queriedSpatialEntities = ExecuteQuery(queryModule, connection, query, null);

            query = new ECQuery(metadataClass);
            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties = new List<IECProperty>();
            query.SelectClause.SelectedProperties.Add(metadataClass.First(prop => prop.Name == "Legal"));

            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression(usgsRequestedEntities.Select(e => e.ID.ToString()).ToArray()));

            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));

            var queriedMetadatas = ExecuteQuery(queryModule, connection, query, null);

            foreach(var entity in queriedSpatialEntities)
            {
                string metadata = entity.GetPropertyValue("Metadata").StringValue;
                string url = entity.GetPropertyValue("MainURL").StringValue;
                string type = entity.GetPropertyValue("DataSourceType").StringValue;
                string copyright = queriedMetadatas.First(m => m.InstanceId == entity.InstanceId).GetPropertyValue("Legal").StringValue;

                usgsSourceNetList.Add(UsgsSourceNet.Create(copyright, url, type, "", new List<string>(), metadata));
            }

            return usgsSourceNetList;
        }

        private List<WmsSourceNet> IndexPackager(OperationModule sender, RepositoryConnection connection, QueryModule queryModule, string coordinateSystem, List<RequestedEntity> dbRequestedEntities)
        {
            List<WmsSourceNet> wmsMapInfoList = new List<WmsSourceNet>();

            if(dbRequestedEntities.Count == 0)
            {
                return wmsMapInfoList;
            }

            if(coordinateSystem == null)
            {
                throw new Bentley.Exceptions.InvalidInputException("Please enter a coordinate system when requesting this type of data.");
            }

            IECClass spatialEntityClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "SpatialEntity");

            IECRelationshipClass dataSourceRelClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "SpatialEntityToSpatialDataSource") as IECRelationshipClass;
            IECClass dataSourceClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "SpatialDataSource");
            RelatedInstanceSelectCriteria dataSourceRelCrit = new RelatedInstanceSelectCriteria(new QueryRelatedClassSpecifier(dataSourceRelClass, RelatedInstanceDirection.Forward, dataSourceClass), true);

            IECRelationshipClass metadataRelClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "SpatialEntityBaseToMetadata") as IECRelationshipClass;
            IECClass metadataClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "Metadata");
            //RelatedInstanceSelectCriteria metadataRelCrit = new RelatedInstanceSelectCriteria(new QueryRelatedClassSpecifier(metadataRelClass, RelatedInstanceDirection.Forward, metadataClass), true);

            ECQuery query = new ECQuery(spatialEntityClass);
            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties = new List<IECProperty>();
            query.SelectClause.SelectedProperties.Add(spatialEntityClass.First(prop => prop.Name == "Id"));
            query.SelectClause.SelectedProperties.Add(spatialEntityClass.First(prop => prop.Name == "Footprint"));
            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression(dbRequestedEntities.Select(e => e.ID.ToString()).ToArray()));
            query.SelectClause.SelectedRelatedInstances.Add(dataSourceRelCrit);
            //query.SelectClause.SelectedRelatedInstances.Add(metadataRelCrit);

            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

            var queriedSpatialEntities = ExecuteQuery(queryModule, connection, query, null);

            List<MapInfo> mapInfoList = new List<MapInfo>();

            foreach (IECInstance spatialEntity in queriedSpatialEntities)
            {
                string polygonString = spatialEntity.GetPropertyValue("Footprint").StringValue;

                //query = new ECQuery(metadataClass);
                //query.SelectClause.SelectAllProperties = false;
                //query.SelectClause.SelectedProperties.Add(metadataClass.First(prop => prop.Name == "Legal"));
                //query.WhereClause = new WhereCriteria(new RelatedCriterion();

                string entityId = spatialEntity.GetPropertyValue("Id").StringValue;

                IECRelationshipInstance firstRel = spatialEntity.GetRelationshipInstances().First();
                IECInstance firstSpatialDataSource = firstRel.Target;

                //WMSSource table query *******************************

                IECClass wmsSourceClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "WMSSource");

                query = new ECQuery(wmsSourceClass);
                query.SelectClause.SelectAllProperties = false;
                query.SelectClause.SelectedProperties = new List<IECProperty>();
                query.SelectClause.SelectedProperties.Add(wmsSourceClass.First(prop => prop.Name == "Layers"));
                query.WhereClause = new WhereCriteria(new ECInstanceIdExpression(firstSpatialDataSource.InstanceId));

                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

                var queriedWMSSourceClass = ExecuteQuery(queryModule, connection, query, null);

                //***********************************************************************

                //Server table query **************************************

                IECRelationshipClass ServerRelClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "ServerToSpatialDataSource") as IECRelationshipClass;
                IECClass serverClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "Server");


                query = new ECQuery(serverClass);

                query.SelectClause.SelectAllProperties = false;
                query.SelectClause.SelectedProperties = new List<IECProperty>();
                query.WhereClause = new WhereCriteria(new RelatedCriterion(new QueryRelatedClassSpecifier(ServerRelClass, RelatedInstanceDirection.Forward, dataSourceClass), new WhereCriteria(new ECInstanceIdExpression(firstSpatialDataSource.InstanceId))));

                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

                var queriedServer = ExecuteQuery(queryModule, connection, query, null);

                //**************************************************************

                //WMSServer table query

                IECClass wmsServerClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "WMSServer");

                query = new ECQuery(wmsServerClass);
                query.SelectClause.SelectAllProperties = false;
                query.SelectClause.SelectedProperties = new List<IECProperty>();
                query.SelectClause.SelectedProperties.Add(wmsServerClass.First(prop => prop.Name == "Legal"));
                query.SelectClause.SelectedProperties.Add(wmsServerClass.First(prop => prop.Name == "GetMapURL"));
                query.SelectClause.SelectedProperties.Add(wmsServerClass.First(prop => prop.Name == "GetMapURLQuery"));
                query.SelectClause.SelectedProperties.Add(wmsServerClass.First(prop => prop.Name == "Version"));
                query.WhereClause = new WhereCriteria(new ECInstanceIdExpression(queriedServer.First().InstanceId));

                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

                var queriedWMSServer = ExecuteQuery(queryModule, connection, query, null);

                PolygonModel model;
                try
                {
                    model = JsonConvert.DeserializeObject<PolygonModel>(polygonString);
                }
                catch (JsonSerializationException)
                {
                    throw new UserFriendlyException("The polygon format is not valid.");
                }

                MapInfo info = new MapInfo
                {
                    GetMapURL = queriedWMSServer.First().GetPropertyValue("GetMapURL").StringValue,
                    GetMapURLQuery = queriedWMSServer.First().GetPropertyValue("GetMapURLQuery").StringValue,
                    Version = queriedWMSServer.First().GetPropertyValue("Version").StringValue,
                    Layers = queriedWMSSourceClass.First().GetPropertyValue("Layers").StringValue,
                    CoordinateSystem = coordinateSystem,
                    SelectedStyle = dbRequestedEntities.First(e => e.ID == entityId).SelectedStyle,
                    SelectedFormat = dbRequestedEntities.First(e => e.ID == entityId).SelectedFormat,
                    Legal = queriedWMSServer.First().GetPropertyValue("Legal").StringValue,
                    Footprint = model.points
                };

                mapInfoList.Add(info);
            }

            // Create WmsSource.
            
            foreach (var mapInfo in mapInfoList)
            {
                // Extract min/max values for bbox.
                IEnumerator<double[]> pointsIt = mapInfo.Footprint.GetEnumerator();
                double minX = 90.0; double maxX = -90.0;
                double minY = 180.0; double maxY = -180.0;
                double temp = 0.0;
                while (pointsIt.MoveNext())
                {
                    //x
                    temp = pointsIt.Current[0];
                    if (minX > temp)
                        minX = temp;
                    if (maxX < temp)
                        maxX = temp;

                    //y
                    temp = pointsIt.Current[1];
                    if (minY > temp)
                        minY = temp;
                    if (maxY < temp)
                        maxY = temp;
                }

                //&&JFC Workaround for the moment (until we add a csType column in the database). 
                // We suppose CRS for version 1.3, SRS for 1.1.1 and below.
                string csType = "CRS";
                if (!mapInfo.Version.Equals("1.3.0"))
                    csType = "SRS";

                // We need to remove extra characters at the end of the vendor specific since 
                // this part is at the end of the GetMap query that will be created later.
                string vendorSpecific = mapInfo.GetMapURLQuery;
                if (vendorSpecific.EndsWith("&"))
                    vendorSpecific = vendorSpecific.TrimEnd('&');

                wmsMapInfoList.Add(WmsSourceNet.Create(mapInfo.Legal,                      // Copyright
                                                       mapInfo.GetMapURL.TrimEnd('?'),    // Url
                                                       minX, minY, maxX, maxY,             // Bbox min/max values
                                                       mapInfo.Version,                    // Version
                                                       mapInfo.Layers,                     // Layers (comma-separated list)
                                                       csType,                             // Coordinate System Type
                                                       mapInfo.CoordinateSystem,           // Coordinate System Label
                                                       10, 10,                             // MetaWidth and MetaHeight
                                                       mapInfo.SelectedStyle,              // Styles (comma-separated list)
                                                       mapInfo.SelectedFormat,             // Format
                                                       vendorSpecific,                     // Vendor Specific
                                                       true));                             // Transparency

            }
            return wmsMapInfoList;
        }

        private RequestedEntity ECStructToRequestedEntity(IECStructValue structValue)
        {
            if (structValue.ClassDefinition.Name != "RequestedEntity")
            {
                throw new ProgrammerException("Error in the ECSchema. A PackageRequest must be composed of an array of RequestedEntity.");
            }

            return new RequestedEntity
            {
                ID = structValue.GetPropertyValue("ID").StringValue,
                SelectedFormat = structValue.GetPropertyValue("SelectedFormat").StringValue,
                SelectedStyle = structValue.GetPropertyValue("SelectedStyle").StringValue,
            };

        }

        private void CreateArchive(IECInstance instance, List<string> LocationOfFilesList, out string guid)
        {
            string archivePath;

            do
            {
                guid = Guid.NewGuid().ToString();
                archivePath = Path.Combine(m_packagesLocation, guid + ".zip");
            }
            while (File.Exists(archivePath));

            FileInfo fileInfo = new FileInfo(archivePath);
            fileInfo.Directory.Create();

            try
            {
                using (ZipArchive archive = ZipFile.Open(archivePath, ZipArchiveMode.Create))
                {
                    foreach (var location in LocationOfFilesList)
                    {
                        archive.CreateEntryFromFile(location, Path.GetFileName(location));
                    }
                }
            }
            catch (Exception)
            {
                File.Delete(archivePath);
                throw;
            }

            //This ID will be used as the name of the package generated. 
        }

        private ConnectionFormatFieldInfo[] GetConnectionFormat(ConnectionModule sender,
                                                                ECSession session,
                                                                bool includeValues,
                                                                IExtendedParameters extendedParameters)
        {
            //Console.WriteLine("Entering GetConnectionFormat");
            return new List<ConnectionFormatFieldInfo>
                {
                new ConnectionFormatFieldInfo() {ID = "username", DisplayName = "username", IsRequired = true },    
                //new ConnectionFormatFieldInfo() {ID = "Password", DisplayName = "Password", IsRequired = true, Masked = true },
            //    //new ConnectionFormatFieldInfo() {ID = eBECPluginConstants.DomainsKey, DisplayName = "Domain", IsRequired = false, IsAdvanced = true }, unused
            //    new ConnectionFormatFieldInfo() {ID = eBECPluginConstants.DefaultScopeKey, DisplayName = "DefaultScope", IsRequired = false, IsAdvanced = false },
            //    new ConnectionFormatFieldInfo() {ID = eBECPluginConstants.ActiveScopesKey, DisplayName = "ActiveScopes", IsRequired = false, IsAdvanced = false },
            //    //new ConnectionFormatFieldInfo() {ID = eBECPluginConstants.ImsRelyingPartyServiceUrlKey, DisplayName = "ImsRelyingPartyServiceUrl", IsRequired = false, IsAdvanced = true } unused
            //    new ConnectionFormatFieldInfo() {ID = "Token", DisplayName = "Token", IsRequired = false, IsAdvanced = true, IsCredential = true }
                }.ToArray();
        }

        private void OpenConnection(ConnectionModule sender,
                                    RepositoryConnection connection,
                                    IExtendedParameters extendedParameters)
        {
            //Console.WriteLine("Entering OpenConnection");
            //Here, we could use the connectionInfo to open a connection
            //Example :
            //string username = connection.ConnectionInfo.GetField("User").Value;
            //string password = connection.ConnectionInfo.GetField("Password").Value;
            //... Whatever you need to do with username and password ...

            string username = connection.ConnectionInfo.GetField("username").Value;

            if (username != "Test")
            {
                throw new Bentley.ECSystem.Repository.AccessDeniedException("Invalid username");
            }
            
        }

        private void CloseConnection(ConnectionModule sender,
                                     RepositoryConnection connection,
                                     IExtendedParameters extendedParameters)
        {
            //Console.WriteLine("Entering CloseConnection");
        }

    }


 }
