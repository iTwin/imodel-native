﻿/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/IndexECPlugin.cs $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
using System.Text;
using System.Reflection;
using Bentley.ECSystem.Configuration;
using System.Data.Common;
using System.Data;

#if !IMSOFF
using Microsoft.IdentityModel.Configuration;
using Microsoft.IdentityModel.Tokens;
using Microsoft.IdentityModel.Web;
#endif

namespace IndexECPlugin.Source
    {
    /*====================================================================================**/
    /// <summary>
    /// Creates the IndexECPlugin using the ECPluginBuilder.
    /// </summary>
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

        private string ConnectionString
            {
            get
                {
                return ConfigurationRoot.GetAppSetting("RECPConnectionString");
                }
            }

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
        protected override void Startup ()
            {
            // Only actually register plugin once.
            if ( null == m_plugin )
                {
                //Start by calling the Register() method which takes a delegate that defines what components the plugin should be composed of
                m_plugin = ECPlugin.Register(PLUGIN_NAME, BuildPlugin);
                }
            }

        /// <summary>
        /// Actual builder method that composes our ECPlugin
        /// </summary>
        /// <param name="builder">builder to use for construction</param>
        private void BuildPlugin (ECPluginBuilder builder)
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
#if IMSOFF
                .SetConnectionSupport(GetConnectionFormat)
#else
.SetConnectionSupport(GetConnectionFormat, OpenConnection, CloseConnection)
#endif
.SetQuerySupport((EnumerableBasedQueryHandler) ExecuteQuery)
                .SetOperationSupport<RetrieveBackingFileOperation>(FileRetrievalOperation)
                .SetOperationSupport<InsertOperation>(ExecuteInsertOperation);

            IndexPolicyHandler.InitializeHandlers(builder);
            }

        private RepositoryIdentifier GetRepositoryIdentifier (RepositoryModule module,
                                                             ECSession session,
                                                             string location,
                                                             IExtendedParameters extendedParameters)
            {
            //Try to obtain the repository identifier for the provided location from the list of known identifiers
            ICollection<RepositoryIdentifier> knownRepositories = module.GetKnownRepositories(session, extendedParameters);
            RepositoryIdentifier knownRepository = knownRepositories.FirstOrDefault((repositoryIdentifier) =>
                string.Equals(location, repositoryIdentifier.Location, StringComparison.Ordinal));

            if ( knownRepository != null )
                {
                return knownRepository;
                }

            //ParseConfigFile(location);

            if ( location == "Server" )
                {

                var identifier = new RepositoryIdentifier(module.ParentECPlugin.ECPluginId, location, location, location, null);
                module.RepositoryVerified(session, identifier); //tells the module to include that repository in the list of known repositories from now on
                return identifier;
                }
            return null;
            //}
            }

        private void PopulateSchemas (SchemaModule module,
                                     ECPluginSchemaManager schemaManager)
            {
            ECSchemaXmlStreamReader schemaReader = new ECSchemaXmlStreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("ECSchemaDB.xml"));
            IECSchema schemaFromXML = schemaReader.Deserialize();
            schemaManager.AddSchema(ref schemaFromXML);
            }

        private IEnumerable<IECInstance> ExecuteQuery (QueryModule sender,
                                                      RepositoryConnection connection,
                                                      ECQuery query,
                                                      ECQuerySettings querySettings)
            {
            //For now, we'll only manage the case where we receive one search class.
            //foreach (SearchClass searchClass in query.SearchClasses)
            try
                {
                string fullSchemaName = sender.ParentECPlugin.SchemaModule.GetSchemaFullNames(connection).First();
                IECSchema schema = sender.ParentECPlugin.SchemaModule.GetSchema(connection, fullSchemaName);
                SearchClass searchClass = query.SearchClasses.First();
                    {
                    Log.Logger.info("Executing query " + query.ID + " : " + query.ToECSqlString(0) + ", custom parameters : " + String.Join(",", query.ExtendedData.Select(x => x.ToString())));

                    if ( (querySettings != null) && ((querySettings.LoadModifiers & LoadModifiers.IncludeStreamDescriptor) != LoadModifiers.None) && (searchClass.Class.Name == "PreparedPackage") )
                        {
                        IECInstance packageInstance = searchClass.Class.CreateInstance();
                        ECInstanceIdExpression exp = query.WhereClause[0] as ECInstanceIdExpression;
                        packageInstance.InstanceId = exp.RightSideString;
                        PackageStreamRetrievalController.SetStreamRetrieval(packageInstance, ConnectionString);
                        return new List<IECInstance> { packageInstance };
                        }

                    IECQueryProvider helper;

                    if ( searchClass.Class.GetCustomAttributes("QueryType") == null || searchClass.Class.GetCustomAttributes("QueryType")["QueryType"].IsNull )
                        {
                        throw new UserFriendlyException(String.Format("The class {0} cannot be queried.", searchClass.Class.Name));
                        }

                    string source;

                    if ( query.ExtendedData.ContainsKey("source") )
                        {
                        source = query.ExtendedData["source"].ToString();
                        }
                    else
                        {
                        //TODO : We should rename queryType, as it has taken the role of a default parameter
                        source = searchClass.Class.GetCustomAttributes("QueryType")["QueryType"].StringValue;
                        }

                    InstanceOverrider instanceOverrider = new InstanceOverrider(new DbQuerier());
                    InstanceComplement instanceComplement = new InstanceComplement(new DbQuerier());
                    using ( SqlConnection sqlConnection = new SqlConnection(ConnectionString) )
                        {
                        switch ( source.ToLower() )
                            {
                            case "index":
                                    {
                                    helper = new SqlQueryProvider(query, querySettings, sqlConnection, schema);
                                    IEnumerable<IECInstance> instances = helper.CreateInstanceList();
                                    instanceOverrider.Modify(instances, DataSource.Index, sqlConnection);
                                    instanceComplement.Modify(instances, DataSource.Index, sqlConnection);
                                    return instances;
                                    }

                            case "usgsapi":
                                    {
                                    helper = new UsgsAPIQueryProvider(query, querySettings, sqlConnection, schema);
                                    IEnumerable<IECInstance> instances = helper.CreateInstanceList();
                                    instanceOverrider.Modify(instances, DataSource.USGS, sqlConnection);
                                    instanceComplement.Modify(instances, DataSource.USGS, sqlConnection);
                                    return instances;
                                    }
                            case "all":
                                    {

                                    List<IECInstance> instanceList = new List<IECInstance>();
                                        {
                                        helper = new SqlQueryProvider(query, querySettings, sqlConnection, schema);
                                        instanceList = helper.CreateInstanceList().ToList();
                                        instanceOverrider.Modify(instanceList, DataSource.Index, sqlConnection);
                                        instanceComplement.Modify(instanceList, DataSource.Index, sqlConnection);

                                        helper = new UsgsAPIQueryProvider(query, querySettings, sqlConnection, schema);
                                        IEnumerable<IECInstance> instances = helper.CreateInstanceList();
                                        instanceOverrider.Modify(instances, DataSource.USGS, sqlConnection);
                                        instanceComplement.Modify(instances, DataSource.USGS, sqlConnection);
                                        instanceList.AddRange(instances);
                                        return instanceList;
                                        }
                                    }
                            default:
                                //throw new UserFriendlyException(String.Format("The class {0} cannot be queried.", searchClass.Class.Name));
                                //Log.Logger.error(String.Format("Query {0} aborted. The source chosen ({1}) is invalid", query.ID, source));
                                throw new UserFriendlyException("The source \"" + source + "\" does not exist. Choose between " + SourceStringMap.GetAllSourceStrings());
                            }
                        }
                    }
                }
            catch ( System.Data.Common.DbException ex)
                {
                //For now, we intercept all of these sql exceptions to prevent any "revealing" messages about the sql command.
                //It would be nice to parse the exception to make it easier to pinpoint the problem for the user.
                Log.Logger.error(String.Format("Query {0} aborted. DbException message : {1}", query.ID, ex.Message));
                Exception innerEx = ex.InnerException;
                while (innerEx != null)
                    {
                    Log.Logger.error(String.Format("Inner error message : {0}", ex.Message));
                    innerEx = innerEx.InnerException;
                    }
                throw new UserFriendlyException("The server has encountered a problem while processing your request. Please verify the syntax of your request. If the problem persists, the server may be down");
                }
            catch ( Exception e )
                {
                Log.Logger.error(String.Format("Query {0} aborted. Error message : {1}", query.ID, e.Message));
                throw;
                }
            }

        private void FileRetrievalOperation
            (OperationModule sender,
            RepositoryConnection connection,
            RetrieveBackingFileOperation operation,
            IECInstance instance,
            string comments,
            WriteModifiers writeModifiers,
            IExtendedParameters extendedParameters)
            {
            try
                {
                IECClass instanceClass = instance.ClassDefinition;

                IECInstance fileHolderAttribute = instanceClass.GetCustomAttributes("FileHolder");

                Log.Logger.trace("Retrieving instance " + instance.InstanceId + " of class " + instanceClass.Name);

                if ( fileHolderAttribute == null )
                    {
                    throw new UserFriendlyException(String.Format("There is no file associated to the {0} class", instanceClass.Name));
                    }



                switch ( fileHolderAttribute["Type"].StringValue )
                    {

                    case "PreparedPackage":
                        //var resourceManager = new FileResourceManager(connection);
                        //FileBackedDescriptorAccessor.SetIn(instance, new FileBackedDescriptor(""));
                        //var packageRetrievalController = new PackageRetrievalController(instance, resourceManager, operation, m_packagesLocation, m_connectionString);
                        //packageRetrievalController.Run();
                        PackageStreamRetrievalController.SetStreamRetrieval(instance, ConnectionString);

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

                        throw new ProgrammerException(String.Format("The retrieval of files of type {0} is not supported.", fileHolderAttribute["Type"].StringValue));
                    }
                }
            catch ( Exception e )
                {
                Log.Logger.error(String.Format("Aborting retrieval of instance {0}. Error message : {1}", instance.InstanceId, e.Message));
                throw;
                }
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
            string className = instance.ClassDefinition.Name;

            try
                {
                switch ( className )
                    {
                    case "PackageRequest":
                        InsertPackageRequest(sender, connection, instance, sender.ParentECPlugin.QueryModule);
                        return;
                    //case "AutomaticRequest":
                    //    InsertAutomaticRequest(sender, connection, instance, sender.ParentECPlugin.QueryModule);
                    //    return;

                    default:
                        Log.Logger.error(String.Format("Package request aborted. The class {0} cannot be inserted", className));
                        throw new Bentley.Exceptions.UserFriendlyException("The only insert operation permitted is a PackageRequest instance insertion.");
                    }
                }
            catch ( Exception e )
                {
                Log.Logger.error(String.Format("Package {1} creation aborted. Error message : {0}", e.Message, instance.InstanceId));
                throw;
                }

            }

        private string InsertPackageRequest (OperationModule sender, RepositoryConnection connection, IECInstance instance, QueryModule queryModule)
            {
            string coordinateSystem = null;

            string name = Guid.NewGuid().ToString();
            instance.InstanceId = name + ".xrdp";

            Log.Logger.trace(String.Format("Initiating creation of the package {0}", instance.InstanceId));

            var csPropValue = instance.GetPropertyValue("CoordinateSystem");

            if ( (csPropValue != null) && (!csPropValue.IsNull) )
                {
                coordinateSystem = instance.GetPropertyValue("CoordinateSystem").StringValue;
                }

            var osmPropValue = instance.GetPropertyValue("OSM");
            bool osm = false;
            if ( osmPropValue != null )
                {
                if ( osmPropValue.StringValue.ToLower() == "true" )
                    osm = true;
                }

            IECArrayValue requestedEntitiesECArray = instance.GetPropertyValue("RequestedEntities") as IECArrayValue;
            if ( requestedEntitiesECArray == null )
                {
                //This error should never happen except if the schema file is corrupted.
                //Log.Logger.error(String.Format("Aborting creation of package {0}. The PackageRequest entry is incorrect. Correct the ECSchema", instance.InstanceId));
                throw new ProgrammerException("The ECSchema is not valid. PackageRequest must have an array property.");
                }

            if ((requestedEntitiesECArray.Count == 0) && (osm == false))
                {
                throw new UserFriendlyException("The request is empty. Please specify items to include in the package");
                }
            //List<RequestedEntity> bentleyFileInfoList = new List<RequestedEntity>();
            List<RequestedEntity> indexRequestedEntities = new List<RequestedEntity>();
            List<RequestedEntity> usgsRequestedEntities = new List<RequestedEntity>();
            for ( int i = 0; i < requestedEntitiesECArray.Count; i++ )
                {

                var requestedEntity = ECStructToRequestedEntity(requestedEntitiesECArray[i] as IECStructValue);

                if ( requestedEntity.ID.Length != IndexConstants.USGSIdLenght )
                    {
                    indexRequestedEntities.Add(requestedEntity);
                    }
                else
                    {
                    usgsRequestedEntities.Add(requestedEntity);
                    }
                }

            // Create package bounding box (region of interest).
            List<double> selectedRegion = new List<double>();

            string selectedRegionStr = instance.GetPropertyValue("Polygon").StringValue;

            selectedRegion = selectedRegionStr.Split(new char[] { ',', '[', ']' }, StringSplitOptions.RemoveEmptyEntries).Select(str => Convert.ToDouble(str)).ToList();

            // Create data source.
            List<WmsSourceNet> wmsSourceList;// = WmsPackager(sender, connection, queryModule, coordinateSystem, wmsRequestedEntities);
            List<Tuple<RealityDataSourceNet, string>> basicSourceList;

            RealityDataPackager(sender, connection, queryModule, indexRequestedEntities, coordinateSystem, out basicSourceList, out wmsSourceList);

            List<Tuple<RealityDataSourceNet, string>> usgsSourceList = UsgsPackager(sender, connection, queryModule, usgsRequestedEntities);

            List<OsmSourceNet> osmSourceList = new List<OsmSourceNet>();
            if ( osm )
                osmSourceList.Add(OsmPackager(sender, connection, queryModule, selectedRegion));

            // Create data group and package.
            ImageryGroupNet imgGroup = ImageryGroupNet.Create();
            ModelGroupNet modelGroup = ModelGroupNet.Create();
            PinnedGroupNet pinnedGroup = PinnedGroupNet.Create();
            TerrainGroupNet terrainGroup = TerrainGroupNet.Create();

            foreach ( Tuple<RealityDataSourceNet, string> realityDataSource in basicSourceList )
                {
                SortRealityDataSourceNet(imgGroup, modelGroup, terrainGroup, realityDataSource);
                }

            foreach ( WmsSourceNet wmsSource in wmsSourceList )
                {
                imgGroup.AddData(wmsSource);
                }

            foreach ( Tuple<RealityDataSourceNet, string> usgsSourceTuple in usgsSourceList )
                {
                SortRealityDataSourceNet(imgGroup, modelGroup, terrainGroup, usgsSourceTuple);
                }

            foreach ( RealityDataSourceNet osmSource in osmSourceList )
                {
                modelGroup.AddData(osmSource);
                }

            // Create package.
            string description = "";
            string copyright = "";
            string packageId = "";

            //Until RealityPackageNet is changed, it creates the file in the temp folder, then we copy it in the database. 
            RealityDataPackageNet.Create(Path.GetTempPath(), name, description, copyright, packageId, selectedRegion, imgGroup, modelGroup, pinnedGroup, terrainGroup);

            UploadPackageInDatabase(instance);

            Log.Logger.info("Created the package file " + instance.InstanceId + ". Region selected : " + selectedRegionStr);
            return instance.InstanceId;
            }

        private static void SortRealityDataSourceNet (ImageryGroupNet imgGroup, ModelGroupNet modelGroup, TerrainGroupNet terrainGroup, Tuple<RealityDataSourceNet, string> sourceTuple)
            {
            //This switch case is temporary. The best thing we should have done
            //was to create a method for this, but these "sourceNet" will probably
            //change soon, so everything here is temporary until the database is in
            //a more complete form
            switch ( sourceTuple.Item2 )
                {

                //TODO: Correct the switch case. The choice of the group for each class was not verified.
                case "Roadway":
                case "Bridge":
                case "Building":
                case "WaterBody":
                case "PointCloud":
                    modelGroup.AddData(sourceTuple.Item1);
                    break;
                case "Terrain":
                    terrainGroup.AddData(sourceTuple.Item1);
                    break;
                case "Imagery":
                default:
                    imgGroup.AddData(sourceTuple.Item1);
                    break;
                }
            }

        private void UploadPackageInDatabase (IECInstance instance)
            {
            using ( DbConnection sqlConnection = new SqlConnection(ConnectionString) )
                {
                sqlConnection.Open();
                using ( DbCommand dbCommand = sqlConnection.CreateCommand() )
                    {
                    dbCommand.CommandText = "INSERT INTO dbo.Packages (Name, CreationTime, FileContent) VALUES (@param0, @param1, @param2)";
                    dbCommand.CommandType = CommandType.Text;

                    DbParameter param0 = dbCommand.CreateParameter();
                    param0.DbType = DbType.String;
                    param0.ParameterName = "@param0";
                    param0.Value = instance.InstanceId;
                    dbCommand.Parameters.Add(param0);

                    DbParameter param1 = dbCommand.CreateParameter();
                    param1.DbType = DbType.DateTime;
                    param1.ParameterName = "@param1";
                    param1.Value = DateTime.UtcNow;
                    dbCommand.Parameters.Add(param1);

                    FileStream fstream = new FileStream(Path.GetTempPath() + instance.InstanceId, FileMode.Open);
                    BinaryReader reader = new BinaryReader(fstream);

                    long longLength = fstream.Length;
                    int intLength;
                    if ( longLength > int.MaxValue )
                        {
                        //Log.Logger.error("Package requested is too large.");
                        throw new Bentley.Exceptions.UserFriendlyException("Package requested is too large. Please reduce the size of the order");
                        }
                    intLength = Convert.ToInt32(longLength);
                    byte[] fileBytes = new byte[fstream.Length];
                    fstream.Seek(0, SeekOrigin.Begin);
                    fstream.Read(fileBytes, 0, intLength);



                    DbParameter param2 = dbCommand.CreateParameter();
                    param2.DbType = DbType.Binary;
                    param2.ParameterName = "@param2";
                    param2.Value = fileBytes;
                    dbCommand.Parameters.Add(param2);

                    dbCommand.ExecuteNonQuery();
                    }
                sqlConnection.Close();
                }
            }

        private void RealityDataPackager (OperationModule sender, RepositoryConnection connection, QueryModule queryModule, List<RequestedEntity> basicRequestedEntities, string coordinateSystem, out List<Tuple<RealityDataSourceNet, string>> RDSNList, out List<WmsSourceNet> WMSList)
            {
            RDSNList = new List<Tuple<RealityDataSourceNet, string>>();
            WMSList = new List<WmsSourceNet>();
            if ( basicRequestedEntities.Count == 0 )
                {
                return;
                }

            IECRelationshipClass metadataRelClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "SpatialEntityBaseToMetadata") as IECRelationshipClass;
            IECClass metadataClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "Metadata");
            RelatedInstanceSelectCriteria metadataRelCrit = new RelatedInstanceSelectCriteria(new QueryRelatedClassSpecifier(metadataRelClass, RelatedInstanceDirection.Forward, metadataClass), false);

            IECRelationshipClass dataSourceRelClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "SpatialEntityToSpatialDataSource") as IECRelationshipClass;
            IECClass dataSourceClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "SpatialDataSource");
            RelatedInstanceSelectCriteria dataSourceRelCrit = new RelatedInstanceSelectCriteria(new QueryRelatedClassSpecifier(dataSourceRelClass, RelatedInstanceDirection.Forward, dataSourceClass), false);

            IECClass spatialEntityClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "SpatialEntity");

            IECClass wmsSourceClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "WMSSource");
            RelatedInstanceSelectCriteria wmsSourceRelCrit = new RelatedInstanceSelectCriteria(new QueryRelatedClassSpecifier(dataSourceRelClass, RelatedInstanceDirection.Forward, wmsSourceClass), false);

            IECRelationshipClass serverRelClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "ServerToSpatialDataSource") as IECRelationshipClass;
            IECClass wmsServerClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "WMSServer");
            RelatedInstanceSelectCriteria wmsServerRelCrit = new RelatedInstanceSelectCriteria(new QueryRelatedClassSpecifier(serverRelClass, RelatedInstanceDirection.Backward, wmsServerClass), false);

            ECQuery query = new ECQuery(spatialEntityClass);
            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties = new List<IECProperty>();
            query.SelectClause.SelectedProperties.Add(spatialEntityClass.First(prop => prop.Name == "Id"));
            query.SelectClause.SelectedProperties.Add(spatialEntityClass.First(prop => prop.Name == "Footprint"));
            query.SelectClause.SelectedProperties.Add(spatialEntityClass.First(prop => prop.Name == "Classification"));
            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression(basicRequestedEntities.Select(e => e.ID.ToString()).ToArray()));
            query.SelectClause.SelectedRelatedInstances.Add(metadataRelCrit);
            query.SelectClause.SelectedRelatedInstances.Add(dataSourceRelCrit);
            query.SelectClause.SelectedRelatedInstances.Add(wmsSourceRelCrit);
            wmsSourceRelCrit.SelectedRelatedInstances.Add(wmsServerRelCrit);

            metadataRelCrit.SelectAllProperties = false;
            metadataRelCrit.SelectedProperties = new List<IECProperty>();
            metadataRelCrit.SelectedProperties.Add(metadataClass.First(prop => prop.Name == "Legal"));

            dataSourceRelCrit.SelectAllProperties = false;
            dataSourceRelCrit.SelectedProperties = new List<IECProperty>();
            dataSourceRelCrit.SelectedProperties.Add(dataSourceClass.First(prop => prop.Name == "MainURL"));
            dataSourceRelCrit.SelectedProperties.Add(dataSourceClass.First(prop => prop.Name == "DataSourceType"));
            dataSourceRelCrit.SelectedProperties.Add(dataSourceClass.First(prop => prop.Name == "FileSize"));
            dataSourceRelCrit.SelectedProperties.Add(dataSourceClass.First(prop => prop.Name == "LocationInCompound"));

            wmsSourceRelCrit.SelectAllProperties = false;
            wmsSourceRelCrit.SelectedProperties = new List<IECProperty>();
            wmsSourceRelCrit.SelectedProperties.Add(wmsSourceClass.First(prop => prop.Name == "Layers"));

            wmsServerRelCrit.SelectAllProperties = false;
            wmsServerRelCrit.SelectedProperties = new List<IECProperty>();
            wmsServerRelCrit.SelectedProperties.Add(wmsServerClass.First(prop => prop.Name == "Legal"));
            wmsServerRelCrit.SelectedProperties.Add(wmsServerClass.First(prop => prop.Name == "GetMapURL"));
            wmsServerRelCrit.SelectedProperties.Add(wmsServerClass.First(prop => prop.Name == "GetMapURLQuery"));
            wmsServerRelCrit.SelectedProperties.Add(wmsServerClass.First(prop => prop.Name == "Version"));

            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

            var queriedSpatialEntities = ExecuteQuery(queryModule, connection, query, null);

            foreach ( IECInstance spatialEntity in queriedSpatialEntities )
                {

                //IECRelationshipInstance firstWMSSourceRel = spatialEntity.GetRelationshipInstances().FirstOrDefault(relInst => relInst.ClassDefinition.Name == "SpatialEntityToSpatialDataSource" && relInst.Target.ClassDefinition.Name == "WMSSource");
                if ( spatialEntity.GetRelationshipInstances().Any(relInst => relInst.ClassDefinition.Name == "SpatialEntityToSpatialDataSource" && relInst.Target.ClassDefinition.Name == "WMSSource") )
                    {
                    WMSList.Add(CreateWMSSource(spatialEntity, coordinateSystem, basicRequestedEntities));
                    }
                else
                    {

                    IECRelationshipInstance firstMetadataRel = spatialEntity.GetRelationshipInstances().First(relInst => relInst.ClassDefinition.Name == "SpatialEntityBaseToMetadata");
                    IECInstance firstMetadata = firstMetadataRel.Target;

                    IECRelationshipInstance firstDataSourceRel = spatialEntity.GetRelationshipInstances().First(relInst => relInst.ClassDefinition.Name == "SpatialEntityToSpatialDataSource" && relInst.Target.ClassDefinition.Name == "SpatialDataSource");
                    IECInstance firstSpatialDataSource = firstDataSourceRel.Target;

                    UInt64 filesize = (firstSpatialDataSource.GetPropertyValue("FileSize") == null || firstSpatialDataSource.GetPropertyValue("FileSize").IsNull) ? 0 : (UInt64)((long) firstSpatialDataSource.GetPropertyValue("FileSize").NativeValue);

                    string uri = firstSpatialDataSource.GetPropertyValue("MainURL").StringValue;
                    string type = firstSpatialDataSource.GetPropertyValue("DataSourceType").StringValue;
                    string copyright = (firstMetadata.GetPropertyValue("Legal") == null || firstMetadata.GetPropertyValue("Legal").IsNull) ? null : firstMetadata.GetPropertyValue("Legal").StringValue;
                    string id = spatialEntity.GetPropertyValue("Id").StringValue;
                    string provider = "";
                    string fileInCompound = (firstSpatialDataSource.GetPropertyValue("LocationInCompound") == null || firstSpatialDataSource.GetPropertyValue("LocationInCompound").IsNull) ? null : firstSpatialDataSource.GetPropertyValue("LocationInCompound").StringValue;
                    string metadata = "";
                    string classification = (spatialEntity.GetPropertyValue("Classification") == null || spatialEntity.GetPropertyValue("Classification").IsNull) ? null : spatialEntity.GetPropertyValue("Classification").StringValue;

                    List<string> sisterFiles = new List<string>();
                    RDSNList.Add(new Tuple<RealityDataSourceNet, string>(RealityDataSourceNet.Create(uri, type, copyright, id, provider, filesize, fileInCompound, metadata, sisterFiles), classification));
                    }
                }
            }

        private WmsSourceNet CreateWMSSource (IECInstance spatialEntity, string coordinateSystem, List<RequestedEntity> requestedEntities)
            {
            IECRelationshipInstance wmsSourceRel = spatialEntity.GetRelationshipInstances().First(relInst => relInst.ClassDefinition.Name == "SpatialEntityToSpatialDataSource" &&
                                                                                                             relInst.Target.ClassDefinition.Name == "WMSSource");
            IECInstance wmsSource = wmsSourceRel.Target;

            IECRelationshipInstance wmsServerRel = wmsSource.GetRelationshipInstances().First(relInst => relInst.ClassDefinition.Name == "ServerToSpatialDataSource" &&
                                                                                                       relInst.Source.ClassDefinition.Name == "WMSServer");
            IECInstance wmsServer = wmsServerRel.Source;

            string entityId = spatialEntity.GetPropertyValue("Id").StringValue;
            string polygonString = spatialEntity.GetPropertyValue("Footprint").StringValue;

            PolygonModel model;
            try
                {
                model = JsonConvert.DeserializeObject<PolygonModel>(polygonString);
                }
            catch ( JsonSerializationException )
                {
                //Log.Logger.error("Package creation aborted. The polygon format is not valid");
                throw new Bentley.Exceptions.UserFriendlyException(String.Format("The polygon format of the database entry {0} is not valid.", entityId));
                }

            MapInfo mapInfo = new MapInfo
            {
                GetMapURL = wmsServer.GetPropertyValue("GetMapURL").StringValue,
                GetMapURLQuery = (wmsServer.GetPropertyValue("GetMapURLQuery") == null || wmsServer.GetPropertyValue("GetMapURLQuery").IsNull) ? null : wmsServer.GetPropertyValue("GetMapURLQuery").StringValue,
                Version = wmsServer.GetPropertyValue("Version").StringValue,
                Layers = (wmsSource.GetPropertyValue("Layers") == null || wmsSource.GetPropertyValue("Layers").IsNull) ? null : wmsSource.GetPropertyValue("Layers").StringValue,
                CoordinateSystem = coordinateSystem,
                SelectedStyle = requestedEntities.First(e => e.ID == entityId).SelectedStyle,
                SelectedFormat = requestedEntities.First(e => e.ID == entityId).SelectedFormat,
                Legal = (wmsServer.GetPropertyValue("Legal") == null || wmsServer.GetPropertyValue("Legal").IsNull) ? null : wmsServer.GetPropertyValue("Legal").StringValue,
                Footprint = model.points
            };



            // Create WmsSource.

            // Extract min/max values for bbox.
            IEnumerator<double[]> pointsIt = mapInfo.Footprint.GetEnumerator();
            double minX = 90.0;
            double maxX = -90.0;
            double minY = 180.0;
            double maxY = -180.0;
            double temp = 0.0;
            while ( pointsIt.MoveNext() )
                {
                //x
                temp = pointsIt.Current[0];
                if ( minX > temp )
                    minX = temp;
                if ( maxX < temp )
                    maxX = temp;

                //y
                temp = pointsIt.Current[1];
                if ( minY > temp )
                    minY = temp;
                if ( maxY < temp )
                    maxY = temp;
                }

            //&&JFC Workaround for the moment (until we add a csType column in the database). 
            // We suppose CRS for version 1.3, SRS for 1.1.1 and below.
            string csType = "CRS";
            if ( !mapInfo.Version.Equals("1.3.0") )
                csType = "SRS";

            // We need to remove extra characters at the end of the vendor specific since 
            // this part is at the end of the GetMap query that will be created later.
            string vendorSpecific = mapInfo.GetMapURLQuery;
            if ( vendorSpecific.EndsWith("&") )
                vendorSpecific = vendorSpecific.TrimEnd('&');

            List<string> sisterFiles = new List<string>();

            return WmsSourceNet.Create(mapInfo.GetMapURL.TrimEnd('?'),      // Url
                                       mapInfo.Legal,                       // Copyright
                                       "",                                  // Id
                                       "",                                  // Provider
                                       0,                                   // Filesize
                                       "",                                  // Metadata
                                       sisterFiles,                         // Sister files
                                       mapInfo.GetMapURL.TrimEnd('?'),      // Url
                                       minX, minY, maxX, maxY,              // Bbox min/max values
                                       mapInfo.Version,                     // Version
                                       mapInfo.Layers,                      // Layers (comma-separated list)
                                       csType,                              // Coordinate System Type
                                       mapInfo.CoordinateSystem,            // Coordinate System Label
                                       10, 10,                              // MetaWidth and MetaHeight
                                       mapInfo.SelectedStyle,               // Styles (comma-separated list)
                                       mapInfo.SelectedFormat,              // Format
                                       vendorSpecific,                      // Vendor Specific
                                       true);                              // Transparency
            }

        private OsmSourceNet OsmPackager (OperationModule sender, RepositoryConnection connection, QueryModule queryModule, List<double> regionOfInterest)
            {
            IECClass spatialEntityClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "SpatialEntity");

            IECRelationshipClass dataSourceRelClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "SpatialEntityToSpatialDataSource") as IECRelationshipClass;
            IECClass osmSourceClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "OsmSource");
            RelatedInstanceSelectCriteria dataSourceRelCrit = new RelatedInstanceSelectCriteria(new QueryRelatedClassSpecifier(dataSourceRelClass, RelatedInstanceDirection.Forward, osmSourceClass), true);
            dataSourceRelCrit.SelectAllProperties = false;
            dataSourceRelCrit.SelectedProperties = new List<IECProperty>();
            dataSourceRelCrit.SelectedProperties.Add(osmSourceClass.First(prop => prop.Name == "MainURL"));
            dataSourceRelCrit.SelectedProperties.Add(osmSourceClass.First(prop => prop.Name == "AlternateURL1"));
            dataSourceRelCrit.SelectedProperties.Add(osmSourceClass.First(prop => prop.Name == "AlternateURL2"));

            IECRelationshipClass metadataRelClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "SpatialEntityBaseToMetadata") as IECRelationshipClass;
            IECClass metadataClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "Metadata");
            RelatedInstanceSelectCriteria metadataRelCrit = new RelatedInstanceSelectCriteria(new QueryRelatedClassSpecifier(metadataRelClass, RelatedInstanceDirection.Forward, metadataClass), true);
            metadataRelCrit.SelectAllProperties = false;
            metadataRelCrit.SelectedProperties = new List<IECProperty>();
            metadataRelCrit.SelectedProperties.Add(metadataClass.First(prop => prop.Name == "Legal"));

            ECQuery query = new ECQuery(spatialEntityClass);
            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties = new List<IECProperty>();
            query.WhereClause = new WhereCriteria(new PropertyExpression(RelationalOperator.EQ, spatialEntityClass.Properties(true).First(p => p.Name == "DataSourceTypesAvailable"), "OSM"));
            query.SelectClause.SelectedRelatedInstances.Add(dataSourceRelCrit);
            query.SelectClause.SelectedRelatedInstances.Add(metadataRelCrit);

            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

            var queriedSpatialEntities = ExecuteQuery(queryModule, connection, query, null);

            IECInstance spatialEntity = queriedSpatialEntities.First();

            string entityId = spatialEntity.GetPropertyValue("Id").StringValue;

            IECRelationshipInstance relInst = spatialEntity.GetRelationshipInstances().First(x => x.ClassDefinition.Name == "SpatialEntityToSpatialDataSource");
            IECInstance spatialDataSource = relInst.Target;

            string mainURL = spatialDataSource.GetPropertyValue("MainURL").StringValue;
            string alternateURL1 = spatialDataSource.GetPropertyValue("AlternateURL1").StringValue;
            string alternateURL2 = spatialDataSource.GetPropertyValue("AlternateURL2").StringValue;

            relInst = spatialEntity.GetRelationshipInstances().First(x => x.ClassDefinition.Name == "SpatialEntityBaseToMetadata");
            IECInstance metadata = relInst.Target;

            string legal = metadata.GetPropertyValue("Legal").StringValue;

            List<string> alternateUrls = new List<string>();
            alternateUrls.Add(alternateURL1);
            alternateUrls.Add(alternateURL2);

            return OsmSourceNet.Create(mainURL,                 // Url
                                       legal,                   // Data copyright
                                       "",                      // Id
                                       "",                      // Provider
                                       0,                       // Data size
                                       "",                      // Metadata
                                       new List<string>(),      // Sister Files 
                                       regionOfInterest,        // bbox
                                       alternateUrls);          // Alternate urls        
            }

        private List<Tuple<RealityDataSourceNet, string>> UsgsPackager (OperationModule sender, RepositoryConnection connection, QueryModule queryModule, List<RequestedEntity> usgsRequestedEntities)
            {
            List<Tuple<RealityDataSourceNet, string>> usgsSourceNetList = new List<Tuple<RealityDataSourceNet, string>>();

            if ( usgsRequestedEntities.Count == 0 )
                {
                return usgsSourceNetList;
                }

            IECClass spatialentityClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "SpatialEntity");

            IECRelationshipClass dataSourceRelClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "SpatialEntityToSpatialDataSource") as IECRelationshipClass;
            IECClass dataSourceClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "SpatialDataSource");
            RelatedInstanceSelectCriteria dataSourceRelCrit = new RelatedInstanceSelectCriteria(new QueryRelatedClassSpecifier(dataSourceRelClass, RelatedInstanceDirection.Forward, dataSourceClass), true);

            IECRelationshipClass metadataRelClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "SpatialEntityBaseToMetadata") as IECRelationshipClass;
            IECClass metadataClass = sender.ParentECPlugin.SchemaModule.FindECClass(connection, "RealityModeling", "Metadata");
            RelatedInstanceSelectCriteria metadataRelCrit = new RelatedInstanceSelectCriteria(new QueryRelatedClassSpecifier(metadataRelClass, RelatedInstanceDirection.Forward, metadataClass), true);

            ECQuery query = new ECQuery(spatialentityClass);
            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties = new List<IECProperty>();
            query.SelectClause.SelectedProperties.Add(spatialentityClass.First(prop => prop.Name == "Classification"));
            query.SelectClause.SelectedRelatedInstances.Add(dataSourceRelCrit);
            query.SelectClause.SelectedRelatedInstances.Add(metadataRelCrit);

            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression(usgsRequestedEntities.Select(e => e.ID.ToString()).ToArray()));

            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi"));

            var queriedSpatialEntities = ExecuteQuery(queryModule, connection, query, null);

            foreach ( var entity in queriedSpatialEntities )
                {
                IECInstance metadataInstance = entity.GetRelationshipInstances().First(rel => rel.Target.ClassDefinition.Name == metadataClass.Name).Target;
                IECInstance datasourceInstance = entity.GetRelationshipInstances().First(rel => rel.Target.ClassDefinition.Name == dataSourceClass.Name).Target;

                string metadata = null;
                if ( !(datasourceInstance.GetPropertyValue("Metadata") == null || datasourceInstance.GetPropertyValue("Metadata").IsNull) )
                    {
                    metadata = datasourceInstance.GetPropertyValue("Metadata").StringValue;
                    }

                string url = null;
                if ( !(datasourceInstance.GetPropertyValue("MainURL") == null || datasourceInstance.GetPropertyValue("MainURL").IsNull) )
                    {
                    url = datasourceInstance.GetPropertyValue("MainURL").StringValue;
                    }

                string type = null;
                if ( !(datasourceInstance.GetPropertyValue("DataSourceType") == null || datasourceInstance.GetPropertyValue("DataSourceType").IsNull) )
                    {
                    type = datasourceInstance.GetPropertyValue("DataSourceType").StringValue;
                    }

                string copyright = null;
                if ( !(metadataInstance.GetPropertyValue("Legal") == null || metadataInstance.GetPropertyValue("Legal").IsNull) )
                    {
                    copyright = metadataInstance.GetPropertyValue("Legal").StringValue;
                    }

                string id = null;
                if ( !(datasourceInstance.GetPropertyValue("Id") == null || datasourceInstance.GetPropertyValue("Id").IsNull) )
                    {
                    id = datasourceInstance.GetPropertyValue("Id").StringValue;
                    }

                long fileSize = 0;
                if(!(datasourceInstance["FileSize"] == null || datasourceInstance["FileSize"].IsNull))
                    {
                    fileSize = (long) datasourceInstance.GetPropertyValue("FileSize").NativeValue;
                    }
                ulong uFileSize = (fileSize > 0) ? (ulong) fileSize : 0;

                string location = null;
                if ( !(datasourceInstance.GetPropertyValue("LocationInCompound") == null || datasourceInstance.GetPropertyValue("LocationInCompound").IsNull ))
                    {
                    location = datasourceInstance.GetPropertyValue("LocationInCompound").StringValue;
                    }
                var classificationPropValue = entity.GetPropertyValue("Classification");
                string classification = null;
                if ( (classificationPropValue != null) && (!classificationPropValue.IsNull) )
                    {
                    classification = classificationPropValue.StringValue;
                    }

                usgsSourceNetList.Add(new Tuple<RealityDataSourceNet, string>(RealityDataSourceNet.Create(url,                  // Url
                                                                                                          type,                 // Main file type
                                                                                                          copyright,            // Data copyright
                                                                                                          id,                   // Id
                                                                                                          "usgs",               // Provider
                                                                                                          uFileSize,            // Data size
                                                                                                          location,             // Main file location
                                                                                                          metadata,             // Metadata
                                                                                                          new List<string>()),  // Sister files                                                                                                      
                                                                                                          classification));     // Classif

                }

            return usgsSourceNetList;
            }

        private RequestedEntity ECStructToRequestedEntity (IECStructValue structValue)
            {
            if ( structValue.ClassDefinition.Name != "RequestedEntity" )
                {
                //Log.Logger.error("Package request aborted. The PackageRequest entry is incorrect. Correct the ECSchema");
                throw new ProgrammerException("Error in the ECSchema. A PackageRequest must be composed of an array of RequestedEntity.");
                }

            return new RequestedEntity
            {
                ID = structValue.GetPropertyValue("ID").StringValue,
                SpatialDataSourceID = (structValue.GetPropertyValue("SpatialDataSourceID") == null || structValue.GetPropertyValue("SpatialDataSourceID").IsNull) ? null : structValue.GetPropertyValue("SpatialDataSourceID").StringValue,
                SelectedFormat = (structValue.GetPropertyValue("SelectedFormat") == null || structValue.GetPropertyValue("SelectedFormat").IsNull) ? null : structValue.GetPropertyValue("SelectedFormat").StringValue,
                SelectedStyle = (structValue.GetPropertyValue("SelectedStyle") == null || structValue.GetPropertyValue("SelectedStyle").IsNull) ? null : structValue.GetPropertyValue("SelectedStyle").StringValue
            };

            }

        private ConnectionFormatFieldInfo[] GetConnectionFormat (ConnectionModule sender,
                                                                ECSession session,
                                                                bool includeValues,
                                                                IExtendedParameters extendedParameters)
            {
            return new List<ConnectionFormatFieldInfo>
                {
                //new ConnectionFormatFieldInfo() {ID = "User", DisplayName = "username", IsRequired = true },    
                //new ConnectionFormatFieldInfo() {ID = "Password", DisplayName = "Password", IsRequired = true, Masked = true },
#if !IMSOFF
                new ConnectionFormatFieldInfo() {ID = "Token", DisplayName = "Token", IsRequired = true, IsAdvanced = true, IsCredential = true }
#endif
                }.ToArray();
            }

#if !IMSOFF
        private void OpenConnection (ConnectionModule sender,
                                    RepositoryConnection connection,
                                    IExtendedParameters extendedParameters)
            {

            string token = connection.ConnectionInfo.GetField("Token").Value;

            try
                {
                string serializedToken = Encoding.UTF8.GetString(Convert.FromBase64String(token.Trim()));

                using ( var reader = XmlReader.Create(new StringReader(serializedToken)) )
                    {
                    var bootstrapToken = FederatedAuthentication.ServiceConfiguration.SecurityTokenHandlers.ReadToken(reader);
                    SecurityTokenHandler handler = FederatedAuthentication.ServiceConfiguration.SecurityTokenHandlers[bootstrapToken];

                    var cic = handler.ValidateToken(bootstrapToken);
                    }
                }
            catch ( Exception )
                {
                Log.Logger.error("Invalid token. Access denied.");
                throw new Bentley.ECSystem.Repository.AccessDeniedException("Invalid token. Access denied.");
                }

            }

        private void CloseConnection (ConnectionModule sender,
                                     RepositoryConnection connection,
                                     IExtendedParameters extendedParameters)
            {
            }
#endif

        }


    }
