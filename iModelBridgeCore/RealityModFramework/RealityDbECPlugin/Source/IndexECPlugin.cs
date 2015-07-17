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
//using BentleyRealityDataPackage;
using IndexECPlugin.Source;
using IndexECPlugin.Source.FileRetrievalControllers;
using IndexECPlugin.Source.Helpers;
using IndexECPlugin.Source.QueryProviders;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Data.SqlClient;
using System.Diagnostics.CodeAnalysis;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Net;
using System.Runtime.InteropServices;
using System.Security.Principal;
using System.Text;
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
        public const string SCHEMA_NAME = "BentleyFiles";

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
                using (SqlConnection sqlConnection = new SqlConnection(m_connectionString))
                {
                    IECQueryProvider helper/* = new SqlQueryProvider(query)*/;

                    if (searchClass.Class.GetCustomAttributes("QueryType") == null || searchClass.Class.GetCustomAttributes("QueryType")["QueryType"].IsNull)
                    {
                        throw new UserFriendlyException(String.Format("The class {0} cannot be queried.", searchClass.Class.Name));
                    }

                    string queryType = searchClass.Class.GetCustomAttributes("QueryType")["QueryType"].StringValue;

                    switch (queryType)
                    {
                        case "SQL":
                            helper = new SqlQueryProvider(query, sqlConnection);
                            break;

                        case "USGSAPI":
                            helper = new UsgsAPIQueryProvider(query);
                            break;
                        default:
                            throw new UserFriendlyException(String.Format("The class {0} cannot be queried.", searchClass.Class.Name));
                    }

                    return helper.CreateInstanceList();
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
//            return null;
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

            var resourceManager = new FileResourceManager(connection);
            FileBackedDescriptorAccessor.SetIn(instance, new FileBackedDescriptor(""));

            switch (fileHolderAttribute["Type"].StringValue)
            {

                case "PreparedPackage":
                    var packageRetrievalController = new PackageRetrievalController(instance, resourceManager, operation, m_packagesLocation);
                    packageRetrievalController.Run();
                    break;
                case "SQLThumbnail":
                    //var sqlThumbnailRetrievalController = new SQLThumbnailRetrievalController(...);
                    //sqlThumbnailRetrievalController.Run();
                    //break;
                case "USGSThumbnail":

                    var usgsThumbnailRetrievalController = new USGSThumbnailRetrievalController(instance);
                    usgsThumbnailRetrievalController.processThumbnailRetrieval();
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

            if(instance.ClassDefinition.Name != "PackageRequest")
            {
                throw new UserFriendlyException("The only insert operation permitted is a PackageResquest instance insertion.");
            }

            //TODO: Implement a package creation procedure that isn't hard coded for a particular schema... Probably impossible.
            
            string coordinateSystem = instance.GetPropertyValue("CoordinateSystem").StringValue;

            IECArrayValue requestedEntitiesECArray = instance.GetPropertyValue("RequestedEntities") as IECArrayValue;
            if(requestedEntitiesECArray == null)
            {
                throw new ProgrammerException("The ECSchema is not valid. PackageRequest must have an array property");
            }

            List<RequestedEntity> bentleyFileInfoList = new List<RequestedEntity>();
            List<RequestedEntity> requestedEntities = new List<RequestedEntity>();
            for(int i = 0; i < requestedEntitiesECArray.Count; i++)
            {
                requestedEntities.Add(ECStructToRequestedEntity(requestedEntitiesECArray[i] as IECStructValue));
            }

            IECClass spatialEntityClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "SpatialEntity");
            ECQuery query = new ECQuery(spatialEntityClass);
            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties = new List<IECProperty>();
            query.SelectClause.SelectedProperties.Add(spatialEntityClass.First(prop => prop.Name == "IsGroup"));
            query.SelectClause.SelectedProperties.Add(spatialEntityClass.First(prop => prop.Name == "ID"));
            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression(requestedEntities.Select(e => e.ID.ToString()).ToArray()));

            bool continueQueries = true;
            bool firstTime = true;

            List<RequestedEntity> oldParentList = null;

            while (continueQueries)
            {
                continueQueries = false;
                var queriedSpatialEntities = ExecuteQuery(null, connection, query, null);


                List<RequestedEntity> newParentList = new List<RequestedEntity>();

                foreach (var queriedInstance in queriedSpatialEntities)
                {
                    RequestedEntity queriedEntity;
                    if (firstTime)
                    {
                        //We have to get the infos given in the request
                        queriedEntity = requestedEntities.First(e => e.ID == queriedInstance.GetPropertyValue("ID").IntValue);
                    }
                    else
                    {
                        //We have to copy the infos (except the ID) from the parent
                        queriedEntity = new RequestedEntity();
                        queriedEntity.ID = queriedInstance.GetPropertyValue("ID").IntValue;
                        var parentEntity = oldParentList.First(e => e.ID == queriedInstance.GetPropertyValue("ParentGroupId").IntValue);

                        queriedEntity.SelectedFormat = parentEntity.SelectedFormat;
                        queriedEntity.SelectedStyle = parentEntity.SelectedStyle;
                    }

                    if(queriedInstance.GetAsString("IsGroup").ToLower() == "false")
                    {   
                        bentleyFileInfoList.Add(queriedEntity);
                    }
                    else
                    {
                        //There are parents in the IDs that were given. We will need to get all the children of these.
                        newParentList.Add(queriedEntity);
                        continueQueries = true;
                    }
                }

                if(continueQueries)
                {
                    query = new ECQuery(spatialEntityClass);
                    query.SelectClause.SelectAllProperties = false;
                    query.SelectClause.SelectedProperties = new List<IECProperty>();
                    query.SelectClause.SelectedProperties.Add(spatialEntityClass.First(prop => prop.Name == "IsGroup"));
                    query.SelectClause.SelectedProperties.Add(spatialEntityClass.First(prop => prop.Name == "ID"));
                    query.SelectClause.SelectedProperties.Add(spatialEntityClass.First(prop => prop.Name == "ParentGroupId"));
                    query.WhereClause = new WhereCriteria();
                    for(int i = 0; i < newParentList.Count; i++)
                    {
                        query.WhereClause.Add(new PropertyExpression(RelationalOperator.EQ, spatialEntityClass.First(prop => prop.Name == "ParentGroupId"), newParentList[i].ID));
                        if(i > 0)
                        {
                            query.WhereClause.SetLogicalOperatorBefore(i, LogicalOperator.OR);
                        }
                    }
                }
                oldParentList = newParentList;
                firstTime = false;
            }

            if(bentleyFileInfoList.Count == 0)
            {
                throw new UserFriendlyException("The package requested is empty. The request was aborted");
            }

            IECClass bentleyFileInfoClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "BentleyFileInfo");
            //Now, we have all the IDs of the bentleyFileInfos. We can query the locations from these and create the archive
            query = new ECQuery(bentleyFileInfoClass);
            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties = new List<IECProperty>();
            query.SelectClause.SelectedProperties.Add(bentleyFileInfoClass.First(prop => prop.Name == "SpatialEntityId"));
            query.SelectClause.SelectedProperties.Add(bentleyFileInfoClass.First(prop => prop.Name == "URL"));
            query.SelectClause.SelectedProperties.Add(bentleyFileInfoClass.First(prop => prop.Name == "Version"));
            query.SelectClause.SelectedProperties.Add(bentleyFileInfoClass.First(prop => prop.Name == "Layers"));
            query.SelectClause.SelectedProperties.Add(bentleyFileInfoClass.First(prop => prop.Name == "Footprint"));
            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression(bentleyFileInfoList.Select(e => e.ID.ToString()).ToArray()));

            var queriedBentleyFileInfos = ExecuteQuery(null, connection, query, null);

            //throw new Exception("Testing");
            List<MapInfo> mapInfoList = new List<MapInfo>();

            string polygonString;
            foreach (var queriedBentleyFileInfo in queriedBentleyFileInfos)
            {
                int entityId = queriedBentleyFileInfo.GetPropertyValue("SpatialEntityId").IntValue;

                polygonString = queriedBentleyFileInfo.GetPropertyValue("Footprint").StringValue;
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
                    URL = queriedBentleyFileInfo.GetPropertyValue("URL").StringValue,
                    Version = queriedBentleyFileInfo.GetPropertyValue("Version").StringValue,
                    Layers = queriedBentleyFileInfo.GetPropertyValue("Layers").StringValue,
                    CoordinateSystem = coordinateSystem,
                    SelectedStyle = bentleyFileInfoList.First(e => e.ID == entityId).SelectedStyle,
                    SelectedFormat = bentleyFileInfoList.First(e => e.ID == entityId).SelectedFormat,
                    Footprint = model.points
                };

                mapInfoList.Add(info);
            }

        }

        private RequestedEntity ECStructToRequestedEntity(IECStructValue structValue)
        {
            if (structValue.ClassDefinition.Name != "RequestedEntity")
            {
                throw new ProgrammerException("Error in the ECSchema. A PackageRequest must be composed of an array of RequestedEntity.");
            }

            return new RequestedEntity
            {
                ID = structValue.GetPropertyValue("ID").IntValue,
                SelectedFormat = structValue.GetPropertyValue("SelectedFormat").StringValue,
                SelectedStyle = structValue.GetPropertyValue("SelectedStyle").StringValue
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
            Console.WriteLine("Entering GetConnectionFormat");
            return new List<ConnectionFormatFieldInfo>
                {
                //new ConnectionFormatFieldInfo() {ID = "User", DisplayName = "User", IsRequired = true },    
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
            Console.WriteLine("Entering OpenConnection");
            //Here, we could use the connectionInfo to open a connection
            //Example :
            //string username = connection.ConnectionInfo.GetField("User").Value;
            //string password = connection.ConnectionInfo.GetField("Password").Value;
            //... Whatever you need to do with username and password ...
            
        }

        private void CloseConnection(ConnectionModule sender,
                                     RepositoryConnection connection,
                                     IExtendedParameters extendedParameters)
        {
            Console.WriteLine("Entering CloseConnection");
        }

    }


 }
