using System;
using System.Collections.Generic;
using System.Data;
using System.Data.Common;
using System.Data.SqlClient;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Bentley.EC.Persistence.Operations;
using Bentley.EC.Persistence.Query;
using Bentley.EC.PluginBuilder.Modules;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using Bentley.ECSystem.Repository;
using Bentley.Exceptions;
using Newtonsoft.Json;
using RealityDataPackageWrapper;

namespace IndexECPlugin.Source.Helpers
    {
    internal class Packager
        {

        private string m_connectionString;
        private EnumerableBasedQueryHandler m_executeQuery;

        public Packager(string connectionString, EnumerableBasedQueryHandler executeQuery)
            {
            m_connectionString = connectionString;
            m_executeQuery = executeQuery;
            }

        public string InsertPackageRequest (OperationModule sender, RepositoryConnection connection, IECInstance instance, QueryModule queryModule)
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

            if ( (requestedEntitiesECArray.Count == 0) && (osm == false) )
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

            try
                {
                selectedRegion = selectedRegionStr.Split(new char[] { ',', '[', ']' }, StringSplitOptions.RemoveEmptyEntries).Select(str => Convert.ToDouble(str)).ToList();
                }
            catch ( System.FormatException )
                {
                throw new UserFriendlyException("The given polygon's format was not correct.");
                }
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

            var queriedSpatialEntities = m_executeQuery(queryModule, connection, query, null);

            foreach ( IECInstance spatialEntity in queriedSpatialEntities )
                {
                RequestedEntity requestedEntity = basicRequestedEntities.First(e => e.ID == spatialEntity.GetPropertyValue("Id").StringValue);
                //IECRelationshipInstance firstWMSSourceRel = spatialEntity.GetRelationshipInstances().FirstOrDefault(relInst => relInst.ClassDefinition.Name == "SpatialEntityToSpatialDataSource" && relInst.Target.ClassDefinition.Name == "WMSSource");
                if ( spatialEntity.GetRelationshipInstances().Any(relInst => relInst.ClassDefinition.Name == "SpatialEntityToSpatialDataSource" && relInst.Target.ClassDefinition.Name == "WMSSource") )
                    {
                    WMSList.Add(CreateWMSSource(spatialEntity, coordinateSystem, requestedEntity));
                    }
                else
                    {
                    IECRelationshipInstance firstMetadataRel = spatialEntity.GetRelationshipInstances().First(relInst => relInst.ClassDefinition.Name == "SpatialEntityBaseToMetadata");
                    IECInstance firstMetadata = firstMetadataRel.Target;

                    IECRelationshipInstance dataSourceRel;
                    if ( requestedEntity.SpatialDataSourceID == null )
                        {
                        dataSourceRel = spatialEntity.GetRelationshipInstances().FirstOrDefault(relInst => relInst.ClassDefinition.Name == "SpatialEntityToSpatialDataSource" &&
                                                                                                           relInst.Target.ClassDefinition.Name == "SpatialDataSource");
                        if ( dataSourceRel == null )
                            {
                            throw new OperationFailedException("The selected spatial entity does not have any related spatial data source.");
                            }
                        }
                    else
                        {
                        dataSourceRel = spatialEntity.GetRelationshipInstances().FirstOrDefault(relInst => relInst.ClassDefinition.Name == "SpatialEntityToSpatialDataSource" &&
                                                                                                           relInst.Target.ClassDefinition.Name == "SpatialDataSource" &&
                                                                                                           relInst.Target.InstanceId == requestedEntity.SpatialDataSourceID);
                        if ( dataSourceRel == null )
                            {
                            throw new UserFriendlyException("The specified spatial dataSource ID is not related to the selected spatial entity");
                            }
                        }

                    IECInstance firstSpatialDataSource = dataSourceRel.Target;

                    UInt64 filesize = (firstSpatialDataSource.GetPropertyValue("FileSize") == null || firstSpatialDataSource.GetPropertyValue("FileSize").IsNull) ? 0 : (UInt64) ((long) firstSpatialDataSource.GetPropertyValue("FileSize").NativeValue);

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

        private static WmsSourceNet CreateWMSSource (IECInstance spatialEntity, string coordinateSystem, RequestedEntity requestedEntity)
            {
            IECRelationshipInstance wmsSourceRel;

            if ( requestedEntity.SpatialDataSourceID == null )
                {
                wmsSourceRel = spatialEntity.GetRelationshipInstances().First(relInst => relInst.ClassDefinition.Name == "SpatialEntityToSpatialDataSource" &&
                                                                                         relInst.Target.ClassDefinition.Name == "WMSSource");
                }
            else
                {
                wmsSourceRel = spatialEntity.GetRelationshipInstances().First(relInst => relInst.ClassDefinition.Name == "SpatialEntityToSpatialDataSource" &&
                                                                                         relInst.Target.ClassDefinition.Name == "WMSSource" &&
                                                                                         relInst.Target.InstanceId == requestedEntity.SpatialDataSourceID);
                }
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
                SelectedStyle = requestedEntity.SelectedStyle,
                SelectedFormat = requestedEntity.SelectedFormat,
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

            var queriedSpatialEntities = m_executeQuery(queryModule, connection, query, null);

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

            var queriedSpatialEntities = m_executeQuery(queryModule, connection, query, null);

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
                if ( !(datasourceInstance["FileSize"] == null || datasourceInstance["FileSize"].IsNull) )
                    {
                    fileSize = (long) datasourceInstance.GetPropertyValue("FileSize").NativeValue;
                    }
                ulong uFileSize = (fileSize > 0) ? (ulong) fileSize : 0;

                string location = null;
                if ( !(datasourceInstance.GetPropertyValue("LocationInCompound") == null || datasourceInstance.GetPropertyValue("LocationInCompound").IsNull) )
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

        private void SortRealityDataSourceNet (ImageryGroupNet imgGroup, ModelGroupNet modelGroup, TerrainGroupNet terrainGroup, Tuple<RealityDataSourceNet, string> sourceTuple)
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
            using ( DbConnection sqlConnection = new SqlConnection(m_connectionString) )
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
        }
    }
