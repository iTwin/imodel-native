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
using RealityPackageNet;

namespace IndexECPlugin.Source.Helpers
    {
    /// <summary>
    /// Class for creating packages.
    /// </summary>
    public class Packager
        {

        private IDbQuerier m_dbQuerier;
        private EnumerableBasedQueryHandler m_executeQuery;
        private bool m_osm;
        private List<double> m_selectedRegion;
        private BBox m_selectedBBox;
        private string m_coordinateSystem;
        private string m_email;

        /// <summary>
        /// Packager constructor
        /// </summary>
        /// <param name="dbQuerier">The IdbQuerier object that communicates with the database</param>
        /// <param name="executeQuery">The query handler for getting the instances from the plugin itself</param>
        public Packager (IDbQuerier dbQuerier, EnumerableBasedQueryHandler executeQuery)
            {
            m_dbQuerier = dbQuerier;
            m_executeQuery = executeQuery;
            }

        /// <summary>
        /// Insert package operation
        /// </summary>
        /// <param name="schema">The schema</param>
        /// <param name="connection">The repository connection</param>
        /// <param name="instance">The PackageRequest instance</param>
        /// <param name="queryModule">The query module</param>
        /// <param name="major">The requested major package version</param>
        /// <param name="minor">The requested minor package version</param>
        /// <param name="requestor">The name of the requesting app</param>
        /// <param name="requestorVersion">The version of the requesting app</param>
        /// <returns>The full name (including path) of the file</returns>
        public string InsertPackageRequest (IECSchema schema, RepositoryConnection connection, IECInstance instance, QueryModule queryModule, int major, int minor, string requestor, string requestorVersion)
            {

            string name = Guid.NewGuid().ToString();
            instance.InstanceId = name + ".xrdp";

            Log.Logger.trace(String.Format("Initiating creation of the package {0}", instance.InstanceId));

            var csPropValue = instance.GetPropertyValue("CoordinateSystem");

            if ( (csPropValue != null) && (!csPropValue.IsNull) )
                {
                m_coordinateSystem = instance.GetPropertyValue("CoordinateSystem").StringValue;
                }

            var osmPropValue = instance.GetPropertyValue("OSM");
            m_osm = false;
            if ( osmPropValue != null )
                {
                if ( osmPropValue.StringValue.ToLower() == "true" )
                    m_osm = true;
                }

            IECArrayValue requestedEntitiesECArray = instance.GetPropertyValue("RequestedEntities") as IECArrayValue;
            if ( requestedEntitiesECArray == null )
                {
                //This error should never happen except if the schema file is corrupted.
                //Log.Logger.error(String.Format("Aborting creation of package {0}. The PackageRequest entry is incorrect. Correct the ECSchema", instance.InstanceId));
                throw new ProgrammerException("The ECSchema is not valid. PackageRequest must have an array property.");
                }

            if ( (requestedEntitiesECArray.Count == 0) && (m_osm == false) )
                {
                throw new UserFriendlyException("The request is empty. Please specify items to include in the package");
                }
            //List<RequestedEntity> bentleyFileInfoList = new List<RequestedEntity>();
            List<RequestedEntity> indexRequestedEntities = new List<RequestedEntity>();
            List<RequestedEntity> subAPIRequestedEntities = new List<RequestedEntity>();
            for ( int i = 0; i < requestedEntitiesECArray.Count; i++ )
                {

                var requestedEntity = ECStructToRequestedEntity(requestedEntitiesECArray[i] as IECStructValue);

                if ( !SourceStringMap.IsValidId(DataSource.USGS, requestedEntity.ID) && !SourceStringMap.IsValidId(DataSource.RDS, requestedEntity.ID) && !SourceStringMap.IsValidId(DataSource.USGSEE, requestedEntity.ID) )
                    {
                    if ( !indexRequestedEntities.Any(e => e.ID == requestedEntity.ID && e.SpatialDataSourceID == requestedEntity.SpatialDataSourceID) )
                        {
                        if ( indexRequestedEntities.Any(e => e.ID == requestedEntity.ID && (e.SpatialDataSourceID == null || requestedEntity.SpatialDataSourceID == null)) )
                            {
                            throw new UserFriendlyException("Please specify a SpatialDataSourceID when requesting multiple entities having the same ID");
                            }
                        indexRequestedEntities.Add(requestedEntity);
                        }
                    }
                else
                    {
                    if ( !subAPIRequestedEntities.Any(e => e.ID == requestedEntity.ID) )
                        {
                        subAPIRequestedEntities.Add(requestedEntity);
                        }
                    }
                }

            // Create package bounding box (region of interest).
            m_selectedRegion = new List<double>();
            m_selectedBBox = new BBox();
            m_selectedBBox.minX = 90.0;
            m_selectedBBox.maxX = -90.0;
            m_selectedBBox.minY = 180.0;
            m_selectedBBox.maxY = -180.0;

            string selectedRegionStr = instance.GetPropertyValue("Polygon").StringValue;
            string[] selectedRegionStrArray = selectedRegionStr.Split(new char[] { ',', '[', ']', ' ' }, StringSplitOptions.RemoveEmptyEntries);

            try
                {
                m_selectedRegion = selectedRegionStrArray.Select(str => Convert.ToDouble(str)).ToList();
                }
            catch ( System.FormatException )
                {
                throw new UserFriendlyException("The given polygon's format was not correct.");
                }
            if ( (m_selectedRegion.Count % 2) != 0 || m_selectedRegion.Count < 6 )
                {
                //We need an even number of coordinates
                throw new UserFriendlyException("The given polygon's format was not correct.");
                }
            for ( int i = 0; i < m_selectedRegion.Count; i = i + 2 )
                {
                //We verify if the lat/long bounds are correct.
                if ( m_selectedRegion[i] < -180.0 || m_selectedRegion[i] > 180.0 || m_selectedRegion[i + 1] < -90.0 || m_selectedRegion[i + 1] > 90.0 )
                    {
                    throw new UserFriendlyException("The given polygon's format was not correct.");
                    }

                //We also extract the bbox
                //x
                double temp = m_selectedRegion[i];
                if ( m_selectedBBox.minX > temp )
                    m_selectedBBox.minX = temp;
                if ( m_selectedBBox.maxX < temp )
                    m_selectedBBox.maxX = temp;
                //y
                temp = m_selectedRegion[i + 1];
                if ( m_selectedBBox.minY > temp )
                    m_selectedBBox.minY = temp;
                if ( m_selectedBBox.maxY < temp )
                    m_selectedBBox.maxY = temp;
                }

            m_email = IndexECPlugin.GetEmailFromConnection(connection);
            if ( m_email == null )
                {
                throw new OperationFailedException("The email should not be null");
                }

            // Create data source.
            //List<WmsSourceNet> wmsSourceList;// = WmsPackager(sender, connection, queryModule, coordinateSystem, wmsRequestedEntities);


            List<RealityDataNet> realityDataNetList = RealityDataPackager(schema, connection, queryModule, indexRequestedEntities, m_coordinateSystem, major);

            realityDataNetList.AddRange(SubAPIPackager(schema, connection, queryModule, subAPIRequestedEntities, major));

            //List<OsmSourceNet> osmSourceList = new List<OsmSourceNet>();
            if ( m_osm )
                realityDataNetList.Add(OsmPackager(schema, connection, queryModule));

            // Create data group and package.
            List<ImageryDataNet> imgGroup = new List<ImageryDataNet>();
            List<ModelDataNet> modelGroup = new List<ModelDataNet>();
            List<PinnedDataNet> pinnedGroup = new List<PinnedDataNet>();
            List<TerrainDataNet> terrainGroup = new List<TerrainDataNet>();

            SortRealityDataNet(imgGroup, modelGroup, terrainGroup, pinnedGroup, realityDataNetList);

            //Until RealityPackageNet is changed, it creates the file in the temp folder, then we copy it in the database. 
            //RealityDataPackageNet.CreateV1(Path.GetTempPath(), name, description, copyright, packageId, selectedRegion, imgGroup, modelGroup, pinnedGroup, terrainGroup);
            RealityDataPackageNet package = RealityDataPackageNet.Create(name);
            package.SetDescription("");
            package.SetCopyright("");
            package.SetBoundingPolygon(m_selectedRegion);
            package.SetId(name);
            package.SetMajorVersion(major);
            package.SetMinorVersion(minor);
            package.SetOrigin("");
            package.SetCreationDate(DateTime.UtcNow.ToString("yyyy-MM-dd HH:mm:ss"));

            foreach ( ImageryDataNet img in imgGroup )
                {
                package.AddImageryData(img);
                }

            foreach ( ModelDataNet model in modelGroup )
                {
                package.AddModelData(model);
                }

            foreach ( PinnedDataNet pin in pinnedGroup )
                {
                package.AddPinnedData(pin);
                }

            foreach ( TerrainDataNet terrain in terrainGroup )
                {
                package.AddTerrainData(terrain);
                }
            string fullName = Path.GetTempPath() + name + ".xrdp";
            if ( !package.Write(fullName) )
                {
                throw new UserFriendlyException("Package creation failed.");
                }

            string version = String.Format("{0}.{1}", major, minor);

            string userId = IndexECPlugin.GetUserIdFromConnection(connection);

            UploadPackageInDatabase(instance, version, requestor, requestorVersion, m_email, userId, String.Join(" ", selectedRegionStrArray));

#if CONNECTENV
            string regionString = String.Join(" ", m_selectedRegion.Select(d => Convert.ToString(d)));

            Dictionary<string, object> additionalProperties = new Dictionary<string, object>();
            additionalProperties.Add("Polygon", regionString);


            IndexECPlugin.MarkFeature(new Guid(IndexConstants.PackageFeatureGuid), additionalProperties);

#endif

            Log.Logger.info("Created the package file " + instance.InstanceId + ". Region selected : " + selectedRegionStr);
            return fullName;
            }

        private List<RealityDataNet> RealityDataPackager (IECSchema schema, RepositoryConnection connection, QueryModule queryModule, List<RequestedEntity> basicRequestedEntities, string coordinateSystem, int major)
            {
            List<RealityDataNet> RDNList = new List<RealityDataNet>();
            if ( basicRequestedEntities.Count == 0 )
                {
                return RDNList;
                }

            IECRelationshipClass metadataRelClass = schema.GetClass("SpatialEntityToMetadata") as IECRelationshipClass;
            IECClass metadataClass = schema.GetClass("Metadata");
            RelatedInstanceSelectCriteria metadataRelCrit = new RelatedInstanceSelectCriteria(new QueryRelatedClassSpecifier(metadataRelClass, RelatedInstanceDirection.Forward, metadataClass), false);

            IECRelationshipClass dataSourceRelClass = schema.GetClass("SpatialEntityToSpatialDataSource") as IECRelationshipClass;
            IECClass dataSourceClass = schema.GetClass("SpatialDataSource");
            RelatedInstanceSelectCriteria dataSourceRelCrit = new RelatedInstanceSelectCriteria(new QueryRelatedClassSpecifier(dataSourceRelClass, RelatedInstanceDirection.Forward, dataSourceClass), false);

            IECClass spatialEntityClass = schema.GetClass("SpatialEntity");

            IECClass wmsSourceClass = schema.GetClass("WMSSource");
            RelatedInstanceSelectCriteria wmsSourceRelCrit = new RelatedInstanceSelectCriteria(new QueryRelatedClassSpecifier(dataSourceRelClass, RelatedInstanceDirection.Forward, wmsSourceClass), false);

            IECClass multibandSourceClass = schema.GetClass("MultibandSource");
            RelatedInstanceSelectCriteria multibandSourceRelCrit = new RelatedInstanceSelectCriteria(new QueryRelatedClassSpecifier(dataSourceRelClass, RelatedInstanceDirection.Forward, multibandSourceClass), false);

            IECRelationshipClass serverRelClass = schema.GetClass("ServerToSpatialDataSource") as IECRelationshipClass;

            IECClass serverClass = schema.GetClass("Server");
            RelatedInstanceSelectCriteria serverRelCrit = new RelatedInstanceSelectCriteria(new QueryRelatedClassSpecifier(serverRelClass, RelatedInstanceDirection.Backward, serverClass), false);

            IECClass wmsServerClass = schema.GetClass("WMSServer");
            RelatedInstanceSelectCriteria wmsServerRelCrit = new RelatedInstanceSelectCriteria(new QueryRelatedClassSpecifier(serverRelClass, RelatedInstanceDirection.Backward, wmsServerClass), false);

            ECQuery query = new ECQuery(spatialEntityClass);
            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties = new List<IECProperty>();
            query.SelectClause.SelectedProperties.Add(spatialEntityClass.First(prop => prop.Name == "Id"));
            query.SelectClause.SelectedProperties.Add(spatialEntityClass.First(prop => prop.Name == "Name"));
            query.SelectClause.SelectedProperties.Add(spatialEntityClass.First(prop => prop.Name == "Footprint"));
            query.SelectClause.SelectedProperties.Add(spatialEntityClass.First(prop => prop.Name == "Classification"));
            query.SelectClause.SelectedProperties.Add(spatialEntityClass.First(prop => prop.Name == "Dataset"));
            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression(basicRequestedEntities.Select(e => e.ID.ToString()).ToArray()));
            query.SelectClause.SelectedRelatedInstances.Add(metadataRelCrit);
            query.SelectClause.SelectedRelatedInstances.Add(dataSourceRelCrit);
            query.SelectClause.SelectedRelatedInstances.Add(wmsSourceRelCrit);
            query.SelectClause.SelectedRelatedInstances.Add(multibandSourceRelCrit);
            wmsSourceRelCrit.SelectedRelatedInstances.Add(wmsServerRelCrit);
            dataSourceRelCrit.SelectedRelatedInstances.Add(serverRelCrit);

            metadataRelCrit.SelectAllProperties = false;
            metadataRelCrit.SelectedProperties = new List<IECProperty>();
            metadataRelCrit.SelectedProperties.Add(metadataClass.First(prop => prop.Name == "Legal"));
            metadataRelCrit.SelectedProperties.Add(metadataClass.First(prop => prop.Name == "TermsOfUse"));
            metadataRelCrit.SelectedProperties.Add(metadataClass.First(prop => prop.Name == "MetadataURL"));

            dataSourceRelCrit.SelectAllProperties = false;
            dataSourceRelCrit.SelectedProperties = new List<IECProperty>();
            dataSourceRelCrit.SelectedProperties.Add(dataSourceClass.First(prop => prop.Name == "MainURL"));
            dataSourceRelCrit.SelectedProperties.Add(dataSourceClass.First(prop => prop.Name == "ParameterizedURL"));
            dataSourceRelCrit.SelectedProperties.Add(dataSourceClass.First(prop => prop.Name == "DataSourceType"));
            dataSourceRelCrit.SelectedProperties.Add(dataSourceClass.First(prop => prop.Name == "NoDataValue"));
            dataSourceRelCrit.SelectedProperties.Add(dataSourceClass.First(prop => prop.Name == "FileSize"));
            dataSourceRelCrit.SelectedProperties.Add(dataSourceClass.First(prop => prop.Name == "CoordinateSystem"));
            dataSourceRelCrit.SelectedProperties.Add(dataSourceClass.First(prop => prop.Name == "LocationInCompound"));
            dataSourceRelCrit.SelectedProperties.Add(dataSourceClass.First(prop => prop.Name == "SisterFiles"));
            dataSourceRelCrit.SelectedProperties.Add(dataSourceClass.First(prop => prop.Name == "Streamed"));

            multibandSourceRelCrit.SelectAllProperties = false;
            multibandSourceRelCrit.SelectedProperties = new List<IECProperty>();
            multibandSourceRelCrit.SelectedProperties.Add(multibandSourceClass.First(prop => prop.Name == "RedBandURL"));
            multibandSourceRelCrit.SelectedProperties.Add(multibandSourceClass.First(prop => prop.Name == "GreenBandURL"));
            multibandSourceRelCrit.SelectedProperties.Add(multibandSourceClass.First(prop => prop.Name == "BlueBandURL"));
            multibandSourceRelCrit.SelectedProperties.Add(multibandSourceClass.First(prop => prop.Name == "PanchromaticBandURL"));
            multibandSourceRelCrit.SelectedProperties.Add(multibandSourceClass.First(prop => prop.Name == "RedBandFileSize"));
            multibandSourceRelCrit.SelectedProperties.Add(multibandSourceClass.First(prop => prop.Name == "GreenBandFileSize"));
            multibandSourceRelCrit.SelectedProperties.Add(multibandSourceClass.First(prop => prop.Name == "BlueBandFileSize"));
            multibandSourceRelCrit.SelectedProperties.Add(multibandSourceClass.First(prop => prop.Name == "PanchromaticBandFileSize"));
            multibandSourceRelCrit.SelectedProperties.Add(multibandSourceClass.First(prop => prop.Name == "RedBandSisterFiles"));
            multibandSourceRelCrit.SelectedProperties.Add(multibandSourceClass.First(prop => prop.Name == "GreenBandSisterFiles"));
            multibandSourceRelCrit.SelectedProperties.Add(multibandSourceClass.First(prop => prop.Name == "BlueBandSisterFiles"));
            multibandSourceRelCrit.SelectedProperties.Add(multibandSourceClass.First(prop => prop.Name == "PanchromaticBandSisterFiles"));
            wmsSourceRelCrit.SelectAllProperties = false;
            wmsSourceRelCrit.SelectedProperties = new List<IECProperty>();
            wmsSourceRelCrit.SelectedProperties.Add(wmsSourceClass.First(prop => prop.Name == "Layers"));

            wmsServerRelCrit.SelectAllProperties = false;
            wmsServerRelCrit.SelectedProperties = new List<IECProperty>();
            wmsServerRelCrit.SelectedProperties.Add(wmsServerClass.First(prop => prop.Name == "Legal"));
            wmsServerRelCrit.SelectedProperties.Add(wmsServerClass.First(prop => prop.Name == "GetMapURL"));
            wmsServerRelCrit.SelectedProperties.Add(wmsServerClass.First(prop => prop.Name == "GetMapURLQuery"));
            wmsServerRelCrit.SelectedProperties.Add(wmsServerClass.First(prop => prop.Name == "Version"));

            serverRelCrit.SelectAllProperties = false;
            serverRelCrit.SelectedProperties = new List<IECProperty>();
            serverRelCrit.SelectedProperties.Add(serverClass.First(prop => prop.Name == "LoginKey"));
            serverRelCrit.SelectedProperties.Add(serverClass.First(prop => prop.Name == "LoginMethod"));
            serverRelCrit.SelectedProperties.Add(serverClass.First(prop => prop.Name == "RegistrationPage"));
            serverRelCrit.SelectedProperties.Add(serverClass.First(prop => prop.Name == "OrganisationPage"));

            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

            var queriedSpatialEntities = m_executeQuery(queryModule, connection, query, null);

            //foreach ( IECInstance spatialEntity in queriedSpatialEntities )
            foreach ( RequestedEntity requestedEntity in basicRequestedEntities )
                {
                IECInstance spatialEntity = queriedSpatialEntities.FirstOrDefault(s => s.GetPropertyValue("Id").StringValue == requestedEntity.ID);

                if ( spatialEntity == null )
                    {
                    throw new UserFriendlyException("At least one of the requested entities has an invalid identifier (Id).");
                    }

                GenericInfo genericInfo = ExtractGenericInfo(spatialEntity, requestedEntity);

                if ( genericInfo.URI == null )
                    {
                    //This should not happen, as the database entries should always contain the URI. Just to make sure, we'll skip these entries.
                    continue;
                    }

                if ( (major == 1) && (genericInfo.Streamed == true) )
                    {
                    //We don't put streamed data in a v1 package
                    continue;
                    }

                //IECRelationshipInstance firstWMSSourceRel = spatialEntity.GetRelationshipInstances().FirstOrDefault(relInst => relInst.ClassDefinition.Name == "SpatialEntityToSpatialDataSource" && relInst.Target.ClassDefinition.Name == "WMSSource");
                if ( spatialEntity.GetRelationshipInstances().Any(relInst => relInst.ClassDefinition.Name == "SpatialEntityToSpatialDataSource" && relInst.Target.ClassDefinition.Name == "WMSSource") )
                    {
                    //This is a WMS source
                    List<double> corners = ExtractCornersList(spatialEntity["Footprint"].StringValue);

                    if ( !RDNList.Any(rdn => rdn.GetDataId() == spatialEntity.InstanceId) )
                        {
                        string dataset = (spatialEntity.GetPropertyValue("Dataset") == null || spatialEntity.GetPropertyValue("Dataset").IsNull) ? null : spatialEntity.GetPropertyValue("Dataset").StringValue;

                        ImageryDataNet idn = ImageryDataNet.Create(CreateWMSSource(spatialEntity, coordinateSystem, requestedEntity, genericInfo), corners);
                        idn.SetDataId(spatialEntity.InstanceId);
                        idn.SetDataName(spatialEntity["Name"].StringValue);
                        idn.SetDataset(dataset);
                        RDNList.Add(idn);
                        }
                    else
                        {
                        RDNList.First(rdn => rdn.GetDataId() == spatialEntity.InstanceId).AddSource(CreateWMSSource(spatialEntity, coordinateSystem, requestedEntity, genericInfo));
                        }
                    }
                else if ( spatialEntity.GetRelationshipInstances().Any(relInst => relInst.ClassDefinition.Name == "SpatialEntityToSpatialDataSource" && relInst.Target.ClassDefinition.Name == "MultibandSource") )
                    {
                    //This is a multiband source

                    IECRelationshipInstance multibandSourceRel;
                    if ( requestedEntity.SpatialDataSourceID == null )
                        {
                        multibandSourceRel = spatialEntity.GetRelationshipInstances().FirstOrDefault(relInst => relInst.ClassDefinition.Name == "SpatialEntityToSpatialDataSource" &&
                                                                                                           relInst.Target.ClassDefinition.Name == "MultibandSource");
                        if ( multibandSourceRel == null )
                            {
                            throw new OperationFailedException("The selected spatial entity does not have any related spatial data source.");
                            }
                        }
                    else
                        {
                        multibandSourceRel = spatialEntity.GetRelationshipInstances().FirstOrDefault(relInst => relInst.ClassDefinition.Name == "SpatialEntityToSpatialDataSource" &&
                                                                                                           relInst.Target.ClassDefinition.Name == "MultibandSource" &&
                                                                                                           relInst.Target.InstanceId == requestedEntity.SpatialDataSourceID);
                        if ( multibandSourceRel == null )
                            {
                            throw new UserFriendlyException("The specified Multiband Source ID is not related to the selected spatial entity");
                            }
                        }
                    IECInstance firstMultibandSource = multibandSourceRel.Target;

                    string redBandURL = (firstMultibandSource.GetPropertyValue("RedBandURL") == null || firstMultibandSource.GetPropertyValue("RedBandURL").IsNull) ? null : firstMultibandSource.GetPropertyValue("RedBandURL").StringValue;
                    string greenBandURL = (firstMultibandSource.GetPropertyValue("GreenBandURL") == null || firstMultibandSource.GetPropertyValue("GreenBandURL").IsNull) ? null : firstMultibandSource.GetPropertyValue("GreenBandURL").StringValue;
                    string blueBandURL = (firstMultibandSource.GetPropertyValue("BlueBandURL") == null || firstMultibandSource.GetPropertyValue("BlueBandURL").IsNull) ? null : firstMultibandSource.GetPropertyValue("BlueBandURL").StringValue;
                    string panchromaticBandURL = (firstMultibandSource.GetPropertyValue("PanchromaticBandURL") == null || firstMultibandSource.GetPropertyValue("PanchromaticBandURL").IsNull) ? null : firstMultibandSource.GetPropertyValue("PanchromaticBandURL").StringValue;
                    long redBandFileSize = (firstMultibandSource.GetPropertyValue("RedBandFileSize") == null || firstMultibandSource.GetPropertyValue("RedBandFileSize").IsNull) ? 0 : (long) firstMultibandSource.GetPropertyValue("RedBandFileSize").NativeValue;
                    long greenBandFileSize = (firstMultibandSource.GetPropertyValue("GreenBandFileSize") == null || firstMultibandSource.GetPropertyValue("GreenBandFileSize").IsNull) ? 0 : (long) firstMultibandSource.GetPropertyValue("GreenBandFileSize").NativeValue;
                    long blueBandFileSize = (firstMultibandSource.GetPropertyValue("BlueBandFileSize") == null || firstMultibandSource.GetPropertyValue("BlueBandFileSize").IsNull) ? 0 : (long) firstMultibandSource.GetPropertyValue("BlueBandFileSize").NativeValue;
                    long panchromaticBandFileSize = (firstMultibandSource.GetPropertyValue("PanchromaticBandFileSize") == null || firstMultibandSource.GetPropertyValue("PanchromaticBandFileSize").IsNull) ? 0 : (long) firstMultibandSource.GetPropertyValue("PanchromaticBandFileSize").NativeValue;
                    string redBandSisterFilesString = (firstMultibandSource.GetPropertyValue("RedBandSisterFiles") == null || firstMultibandSource.GetPropertyValue("RedBandSisterFiles").IsNull) ? null : firstMultibandSource.GetPropertyValue("RedBandSisterFiles").StringValue;
                    string blueBandSisterFilesString = (firstMultibandSource.GetPropertyValue("BlueBandSisterFiles") == null || firstMultibandSource.GetPropertyValue("BlueBandSisterFiles").IsNull) ? null : firstMultibandSource.GetPropertyValue("BlueBandSisterFiles").StringValue;
                    string greenBandSisterFilesString = (firstMultibandSource.GetPropertyValue("GreenBandSisterFiles") == null || firstMultibandSource.GetPropertyValue("GreenBandSisterFiles").IsNull) ? null : firstMultibandSource.GetPropertyValue("GreenBandSisterFiles").StringValue;
                    string panchromaticBandSisterFilesString = (firstMultibandSource.GetPropertyValue("PanchromaticBandSisterFiles") == null || firstMultibandSource.GetPropertyValue("PanchromaticBandSisterFiles").IsNull) ? null : firstMultibandSource.GetPropertyValue("PanchromaticBandSisterFiles").StringValue;

                    MultiBandSourceNet msn;
                    if ( major == 1 )
                        {
                        msn = MultiBandSourceNet.Create(UriNet.Create(panchromaticBandURL, genericInfo.FileInCompound), genericInfo.Type);
                        }
                    else
                        {
                        msn = MultiBandSourceNet.Create(UriNet.Create(genericInfo.URI, genericInfo.FileInCompound), genericInfo.Type);
                        }

                    SetRdsnFields(msn, genericInfo);

                    redBandFileSize = Math.Max(0, redBandFileSize);
                    greenBandFileSize = Math.Max(0, greenBandFileSize);
                    blueBandFileSize = Math.Max(0, blueBandFileSize);
                    panchromaticBandFileSize = Math.Max(0, panchromaticBandFileSize);

                    RealityDataSourceNet redBandSN = RealityDataSourceNet.Create(UriNet.Create(redBandURL), genericInfo.Type);
                    SetRdsnFields(redBandSN, genericInfo);
                    redBandSN.SetSize((ulong) redBandFileSize);
                    if ( redBandSisterFilesString != null )
                        {
                        redBandSN.SetSisterFiles(redBandSisterFilesString.Split('|').Select(sf => UriNet.Create(sf)).ToList());
                        }

                    RealityDataSourceNet greenBandSN = RealityDataSourceNet.Create(UriNet.Create(greenBandURL), genericInfo.Type);
                    SetRdsnFields(greenBandSN, genericInfo);
                    greenBandSN.SetSize((ulong) greenBandFileSize);
                    if ( greenBandSisterFilesString != null )
                        {
                        greenBandSN.SetSisterFiles(greenBandSisterFilesString.Split('|').Select(sf => UriNet.Create(sf)).ToList());
                        }

                    RealityDataSourceNet blueBandSN = RealityDataSourceNet.Create(UriNet.Create(blueBandURL), genericInfo.Type);
                    SetRdsnFields(blueBandSN, genericInfo);
                    blueBandSN.SetSize((ulong) blueBandFileSize);
                    if ( blueBandSisterFilesString != null )
                        {
                        blueBandSN.SetSisterFiles(blueBandSisterFilesString.Split('|').Select(sf => UriNet.Create(sf)).ToList());
                        }

                    RealityDataSourceNet panchromaticBandSN = RealityDataSourceNet.Create(UriNet.Create(panchromaticBandURL), genericInfo.Type);
                    SetRdsnFields(panchromaticBandSN, genericInfo);
                    panchromaticBandSN.SetSize((ulong) panchromaticBandFileSize);
                    if ( panchromaticBandSisterFilesString != null )
                        {
                        panchromaticBandSN.SetSisterFiles(panchromaticBandSisterFilesString.Split('|').Select(sf => UriNet.Create(sf)).ToList());
                        }

                    msn.SetRedBand(redBandSN);
                    msn.SetGreenBand(greenBandSN);
                    msn.SetBlueBand(blueBandSN);
                    msn.SetPanchromaticBand(panchromaticBandSN);

                    List<double> corners = ExtractCornersList(spatialEntity["Footprint"].StringValue);

                    if ( !RDNList.Any(rdn => rdn.GetDataId() == spatialEntity.InstanceId) )
                        {
                        ImageryDataNet idn = ImageryDataNet.Create(msn, corners);
                        idn.SetDataId(genericInfo.SpatialEntityID);
                        idn.SetDataName(genericInfo.Name);
                        idn.SetDataset(genericInfo.Dataset);
                        RDNList.Add(idn);
                        }
                    else
                        {
                        RDNList.First(rdn => rdn.GetDataId() == spatialEntity.InstanceId).AddSource(msn);
                        }
                    }
                else
                    {
                    //This is a generic source, not needing any special treatment.

                    if ( genericInfo.ParameterizedURI )
                        {
                        SetParameterizedURL(genericInfo, m_selectedBBox, m_email, m_coordinateSystem);
                        }

                    RealityDataSourceNet rdsn = RealityDataSourceNet.Create(UriNet.Create(genericInfo.URI, genericInfo.FileInCompound), genericInfo.Type);

                    SetRdsnFields(rdsn, genericInfo);

                    if ( genericInfo.Name == "OpenStreetMap" || genericInfo.Type == "OSM" )
                        {
                        //We skip OpenStreetMap and make sure it will be done later. 
                        m_osm = true;
                        continue;
                        }

                    if ( !RDNList.Any(rdn => rdn.GetDataId() == genericInfo.SpatialEntityID) )
                        {
                        RDNList.Add(CreateAppropriateRDN(rdsn, genericInfo.Classification, genericInfo.SpatialEntityID, genericInfo.Name, genericInfo.Dataset, genericInfo.Footprint));
                        }
                    else
                        {
                        RDNList.First(rdn => rdn.GetDataId() == genericInfo.SpatialEntityID).AddSource(rdsn);
                        }

                    }
                }
            return RDNList;
            }

        private void SetParameterizedURL (GenericInfo genericInfo, BBox bbox, string email, string coordinateSystem)
            {
            string modifiedURI = genericInfo.URI.Replace("$(MINLONG)", bbox.minX.ToString("#.##########"));
            modifiedURI = modifiedURI.Replace("$(MAXLONG)", bbox.maxX.ToString("#.##########"));
            modifiedURI = modifiedURI.Replace("$(MINLAT)", bbox.minY.ToString("#.##########"));
            modifiedURI = modifiedURI.Replace("$(MAXLAT)", bbox.maxY.ToString("#.##########"));
            modifiedURI = modifiedURI.Replace("$(EMAIL_ADDRESS)", email);
            modifiedURI = modifiedURI.Replace("$(TARGET_GCS)", coordinateSystem);
            genericInfo.URI = modifiedURI;
            }

        private static void SetRdsnFields (RealityDataSourceNet rdsn, GenericInfo genericInfo)
            {
            if ( genericInfo.Copyright != null )
                {
                rdsn.SetCopyright(genericInfo.Copyright);
                }
            if ( genericInfo.CoordinateSystem != null )
                {
                rdsn.SetGeoCS(genericInfo.CoordinateSystem);
                }
            if ( genericInfo.TermsOfUse != null )
                {
                rdsn.SetTermOfUse(genericInfo.TermsOfUse);
                }
            if ( genericInfo.NoDataValue != null )
                {
                rdsn.SetNoDataValue(genericInfo.NoDataValue);
                }
            rdsn.SetId(genericInfo.SpatialDataSourceID);
            if ( genericInfo.Metadata != null )
                {
                rdsn.SetMetadata(genericInfo.Metadata);
                }
            if ( genericInfo.Provider != null )
                {
                rdsn.SetProvider(genericInfo.Provider);
                }
            rdsn.SetSize(genericInfo.FileSize);
            if ( genericInfo.SisterFiles != null )
                {
                rdsn.SetSisterFiles(genericInfo.SisterFiles);
                }
            if ( genericInfo.ServerLoginKey != null )
                {
                rdsn.SetServerLoginKey(genericInfo.ServerLoginKey);
                }
            if ( genericInfo.ServerLoginMethod != null )
                {
                rdsn.SetServerLoginMethod(genericInfo.ServerLoginMethod);
                }
            if ( genericInfo.ServerOrganisationPage != null )
                {
                rdsn.SetServerOrganisationPage(genericInfo.ServerOrganisationPage);
                }
            if ( genericInfo.ServerRegistrationPage != null )
                {
                rdsn.SetServerRegistrationPage(genericInfo.ServerRegistrationPage);
                }
            rdsn.SetStreamed(genericInfo.Streamed);
            }

        private static GenericInfo ExtractGenericInfo (IECInstance spatialEntity, RequestedEntity requestedEntity)
            {
            GenericInfo genericInfo = new GenericInfo();

            IECRelationshipInstance firstMetadataRel = spatialEntity.GetRelationshipInstances().First(relInst => relInst.ClassDefinition.Name == "SpatialEntityToMetadata");
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

            IECRelationshipInstance serverRel = firstSpatialDataSource.GetRelationshipInstances().FirstOrDefault(relServ => relServ.ClassDefinition.Name == "ServerToSpatialDataSource" &&
                                                                                                                   relServ.Source.ClassDefinition.Name == "Server");

            IECInstance server = null;
            if ( serverRel != null )
                {
                server = serverRel.Source;
                }

            long fileSize = (firstSpatialDataSource.GetPropertyValue("FileSize") == null || firstSpatialDataSource.GetPropertyValue("FileSize").IsNull) ? 0 : ((long) firstSpatialDataSource.GetPropertyValue("FileSize").NativeValue);
            genericInfo.FileSize = (fileSize < 0) ? 0 : (ulong) fileSize;
            genericInfo.URI = (firstSpatialDataSource.GetPropertyValue("MainURL") == null || firstSpatialDataSource.GetPropertyValue("MainURL").IsNull) ? null : firstSpatialDataSource.GetPropertyValue("MainURL").StringValue;
            genericInfo.ParameterizedURI = (firstSpatialDataSource.GetPropertyValue("ParameterizedURL") == null || firstSpatialDataSource.GetPropertyValue("ParameterizedURL").IsNull) ? false : (bool) firstSpatialDataSource.GetPropertyValue("ParameterizedURL").NativeValue;
            genericInfo.Type = firstSpatialDataSource.GetPropertyValue("DataSourceType").StringValue;
            genericInfo.Streamed = (firstSpatialDataSource.GetPropertyValue("Streamed") == null || firstSpatialDataSource.GetPropertyValue("Streamed").IsNull) ? false : (bool) firstSpatialDataSource.GetPropertyValue("Streamed").NativeValue;
            genericInfo.Copyright = (firstMetadata.GetPropertyValue("Legal") == null || firstMetadata.GetPropertyValue("Legal").IsNull) ? null : firstMetadata.GetPropertyValue("Legal").StringValue;
            genericInfo.TermsOfUse = (firstMetadata.GetPropertyValue("TermsOfUse") == null || firstMetadata.GetPropertyValue("TermsOfUse").IsNull) ? null : firstMetadata.GetPropertyValue("TermsOfUse").StringValue;
            genericInfo.Provider = (spatialEntity.GetPropertyValue("DataProvider") == null || spatialEntity.GetPropertyValue("DataProvider").IsNull) ? null : spatialEntity.GetPropertyValue("DataProvider").StringValue;
            genericInfo.SpatialEntityID = spatialEntity.InstanceId;
            genericInfo.Name = (spatialEntity.GetPropertyValue("Name") == null || spatialEntity.GetPropertyValue("Name").IsNull) ? null : spatialEntity.GetPropertyValue("Name").StringValue;
            genericInfo.Footprint = (spatialEntity.GetPropertyValue("Footprint") == null || spatialEntity.GetPropertyValue("Footprint").IsNull) ? null : spatialEntity.GetPropertyValue("Footprint").StringValue;
            genericInfo.SpatialDataSourceID = firstSpatialDataSource.InstanceId;
            genericInfo.FileInCompound = (firstSpatialDataSource.GetPropertyValue("LocationInCompound") == null || firstSpatialDataSource.GetPropertyValue("LocationInCompound").IsNull) ? null : firstSpatialDataSource.GetPropertyValue("LocationInCompound").StringValue;
            genericInfo.NoDataValue = (firstSpatialDataSource.GetPropertyValue("NoDataValue") == null || firstSpatialDataSource.GetPropertyValue("NoDataValue").IsNull) ? null : firstSpatialDataSource.GetPropertyValue("NoDataValue").StringValue;
            genericInfo.CoordinateSystem = (firstSpatialDataSource.GetPropertyValue("CoordinateSystem") == null || firstSpatialDataSource.GetPropertyValue("CoordinateSystem").IsNull) ? null : firstSpatialDataSource.GetPropertyValue("CoordinateSystem").StringValue;
            genericInfo.Classification = (spatialEntity.GetPropertyValue("Classification") == null || spatialEntity.GetPropertyValue("Classification").IsNull) ? null : spatialEntity.GetPropertyValue("Classification").StringValue;
            genericInfo.Metadata = (firstMetadata.GetPropertyValue("MetadataURL") == null || firstMetadata.GetPropertyValue("MetadataURL").IsNull) ? null : firstMetadata.GetPropertyValue("MetadataURL").StringValue;
            genericInfo.Dataset = (spatialEntity.GetPropertyValue("Dataset") == null || spatialEntity.GetPropertyValue("Dataset").IsNull) ? null : spatialEntity.GetPropertyValue("Dataset").StringValue;
            if ( server != null )
                {
                genericInfo.ServerLoginKey = (server.GetPropertyValue("LoginKey") == null || server.GetPropertyValue("LoginKey").IsNull) ? null : server.GetPropertyValue("LoginKey").StringValue;
                genericInfo.ServerLoginMethod = (server.GetPropertyValue("LoginMethod") == null || server.GetPropertyValue("LoginMethod").IsNull) ? null : server.GetPropertyValue("LoginMethod").StringValue;
                genericInfo.ServerRegistrationPage = (server.GetPropertyValue("RegistrationPage") == null || server.GetPropertyValue("RegistrationPage").IsNull) ? null : server.GetPropertyValue("RegistrationPage").StringValue;
                genericInfo.ServerOrganisationPage = (server.GetPropertyValue("OrganisationPage") == null || server.GetPropertyValue("OrganisationPage").IsNull) ? null : server.GetPropertyValue("OrganisationPage").StringValue;
                }


            string sisterFilesString = (firstSpatialDataSource.GetPropertyValue("SisterFiles") == null || firstSpatialDataSource.GetPropertyValue("SisterFiles").IsNull) ? null : firstSpatialDataSource.GetPropertyValue("SisterFiles").StringValue;
            if ( sisterFilesString != null )
                {
                genericInfo.SisterFiles = sisterFilesString.Split('|').Select(sf => UriNet.Create(sf)).ToList();
                }
            else
                {
                genericInfo.SisterFiles = null;
                }

            return genericInfo;
            }

        //private static MultibandSource CreateMultibandSource (IECInstance spatialEntity, RequestedEntity requestedEntity)
        //    {
        //    IECRelationshipInstance multibandSourceRel;
        //    if ( requestedEntity.SpatialDataSourceID == null )
        //        {
        //        multibandSourceRel = spatialEntity.GetRelationshipInstances().First(relInst => relInst.ClassDefinition.Name == "SpatialEntityToSpatialDataSource" &&
        //                                                                            relInst.Target.ClassDefinition.Name == "MultibandSource");
        //        }
        //    else
        //        {
        //        multibandSourceRel = spatialEntity.GetRelationshipInstances().First(relInst => relInst.ClassDefinition.Name == "SpatialEntityToSpatialDataSource" &&
        //                                                                            relInst.Target.ClassDefinition.Name == "MultibandSource" &&
        //                                                                            relInst.Target.InstanceId == requestedEntity.SpatialDataSourceID);
        //        }

        //    IECInstance multibandSource = multibandSourceRel.Target;

        //    string redBandURL = (multibandSource.GetPropertyValue("RedBandURL") == null || multibandSource.GetPropertyValue("RedBandURL").IsNull) ? null : multibandSource.GetPropertyValue("RedBandURL").StringValue;
        //    string greenBandURL = (multibandSource.GetPropertyValue("GreenBandURL") == null || multibandSource.GetPropertyValue("GreenBandURL").IsNull) ? null : multibandSource.GetPropertyValue("GreenBandURL").StringValue;
        //    string blueBandURL = (multibandSource.GetPropertyValue("BlueBandURL") == null || multibandSource.GetPropertyValue("BlueBandURL").IsNull) ? null : multibandSource.GetPropertyValue("BlueBandURL").StringValue;

        //    UInt64 redBandFileSize = (multibandSource.GetPropertyValue("RedBandFileSize") == null || multibandSource.GetPropertyValue("RedBandFileSize").IsNull) ? 0 : (UInt64) ((long) multibandSource.GetPropertyValue("RedBandFileSize").NativeValue);
        //    UInt64 greenBandFileSize = (multibandSource.GetPropertyValue("GreenBandFileSize") == null || multibandSource.GetPropertyValue("GreenBandFileSize").IsNull) ? 0 : (UInt64) ((long) multibandSource.GetPropertyValue("GreenBandFileSize").NativeValue);
        //    UInt64 blueBandFileSize = (multibandSource.GetPropertyValue("BlueBandFileSize") == null || multibandSource.GetPropertyValue("BlueBandFileSize").IsNull) ? 0 : (UInt64) ((long) multibandSource.GetPropertyValue("BlueBandFileSize").NativeValue);

        //    throw new NotImplementedException();
        //    }

        private static WmsDataSourceNet CreateWMSSource (IECInstance spatialEntity, string coordinateSystem, RequestedEntity requestedEntity, GenericInfo genericInfo)
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
                throw new Bentley.Exceptions.UserFriendlyException("The polygon format of a selected entry is not valid.");
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
                Footprint = model.Points
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

            List<Double> bbox = new List<double> { minX, minY, maxX, maxY };

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

            WmsMapSettingsNet wmsMapSettings = WmsMapSettingsNet.Create(mapInfo.GetMapURL.TrimEnd('?'), bbox, mapInfo.Version, mapInfo.Layers, csType, mapInfo.CoordinateSystem);

            wmsMapSettings.SetMetaWidth(10);
            wmsMapSettings.SetMetaHeight(10);
            wmsMapSettings.SetStyles(mapInfo.SelectedStyle);
            wmsMapSettings.SetFormat(mapInfo.SelectedFormat);
            wmsMapSettings.SetVendorSpecific(vendorSpecific);
            wmsMapSettings.SetTransparency(true);

            WmsDataSourceNet wdsn = WmsDataSourceNet.Create(genericInfo.URI);

            SetRdsnFields(wdsn, genericInfo);

            wdsn.SetCopyright(mapInfo.Legal);
            wdsn.SetMapSettings(wmsMapSettings.ToXml());

            return wdsn;

            }

        private RealityDataNet OsmPackager (IECSchema schema, RepositoryConnection connection, QueryModule queryModule)
            {
            IECClass spatialEntityClass = schema.GetClass("SpatialEntity");

            IECRelationshipClass dataSourceRelClass = schema.GetClass("SpatialEntityToSpatialDataSource") as IECRelationshipClass;
            IECClass osmSourceClass = schema.GetClass("OsmSource");
            RelatedInstanceSelectCriteria dataSourceRelCrit = new RelatedInstanceSelectCriteria(new QueryRelatedClassSpecifier(dataSourceRelClass, RelatedInstanceDirection.Forward, osmSourceClass), true);
            dataSourceRelCrit.SelectAllProperties = false;
            dataSourceRelCrit.SelectedProperties = new List<IECProperty>();
            dataSourceRelCrit.SelectedProperties.Add(osmSourceClass.First(prop => prop.Name == "MainURL"));
            dataSourceRelCrit.SelectedProperties.Add(osmSourceClass.First(prop => prop.Name == "AlternateURL1"));
            dataSourceRelCrit.SelectedProperties.Add(osmSourceClass.First(prop => prop.Name == "AlternateURL2"));
            dataSourceRelCrit.SelectedProperties.Add(osmSourceClass.First(prop => prop.Name == "CoordinateSystem"));

            IECRelationshipClass metadataRelClass = schema.GetClass("SpatialEntityToMetadata") as IECRelationshipClass;
            IECClass metadataClass = schema.GetClass("Metadata");
            RelatedInstanceSelectCriteria metadataRelCrit = new RelatedInstanceSelectCriteria(new QueryRelatedClassSpecifier(metadataRelClass, RelatedInstanceDirection.Forward, metadataClass), true);
            metadataRelCrit.SelectAllProperties = false;
            metadataRelCrit.SelectedProperties = new List<IECProperty>();
            metadataRelCrit.SelectedProperties.Add(metadataClass.First(prop => prop.Name == "Legal"));
            metadataRelCrit.SelectedProperties.Add(metadataClass.First(prop => prop.Name == "TermsOfUse"));

            ECQuery query = new ECQuery(spatialEntityClass);
            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties = new List<IECProperty>();
            query.SelectClause.SelectedProperties.Add(spatialEntityClass.First(prop => prop.Name == "Name"));
            query.WhereClause = new WhereCriteria(new PropertyExpression(RelationalOperator.EQ, spatialEntityClass.Properties(true).First(p => p.Name == "Name"), "OpenStreetMap"));
            query.WhereClause.Add(new PropertyExpression(RelationalOperator.EQ, spatialEntityClass.Properties(true).First(p => p.Name == "DataSourceTypesAvailable"), "OSM"));
            query.WhereClause.SetLogicalOperatorAfter(0, LogicalOperator.AND);
            query.SelectClause.SelectedRelatedInstances.Add(dataSourceRelCrit);
            query.SelectClause.SelectedRelatedInstances.Add(metadataRelCrit);

            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

            var queriedSpatialEntities = m_executeQuery(queryModule, connection, query, null);

            IECInstance spatialEntity = queriedSpatialEntities.First();

            string entityId = spatialEntity.GetPropertyValue("Id").StringValue;

            IECRelationshipInstance relInst = spatialEntity.GetRelationshipInstances().First(x => x.ClassDefinition.Name == "SpatialEntityToSpatialDataSource");
            IECInstance spatialDataSource = relInst.Target;

            string mainURL = spatialDataSource.GetPropertyValue("MainURL").StringValue;
            string alternateURL1 = (spatialDataSource.GetPropertyValue("AlternateURL1") == null || spatialDataSource.GetPropertyValue("AlternateURL1").IsNull) ? null : spatialDataSource.GetPropertyValue("AlternateURL1").StringValue;
            string alternateURL2 = (spatialDataSource.GetPropertyValue("AlternateURL2") == null || spatialDataSource.GetPropertyValue("AlternateURL2").IsNull) ? null : spatialDataSource.GetPropertyValue("AlternateURL2").StringValue;
            string cs = (spatialDataSource.GetPropertyValue("CoordinateSystem") == null || spatialDataSource.GetPropertyValue("CoordinateSystem").IsNull) ? null : spatialDataSource.GetPropertyValue("CoordinateSystem").StringValue;

            relInst = spatialEntity.GetRelationshipInstances().First(x => x.ClassDefinition.Name == "SpatialEntityToMetadata");
            IECInstance metadata = relInst.Target;

            string legal = metadata.GetPropertyValue("Legal").StringValue;

            List<string> alternateUrls = new List<string>();
            if ( alternateURL1 != null )
                {
                alternateUrls.Add(alternateURL1);
                }
            if ( alternateURL2 != null )
                {
                alternateUrls.Add(alternateURL2);
                }

            OsmDataSourceNet odsn = OsmDataSourceNet.Create(mainURL, m_selectedBBox.minX, m_selectedBBox.minY, m_selectedBBox.maxX, m_selectedBBox.maxY);

            List<double> bbox = new List<double>() { m_selectedBBox.minX, m_selectedBBox.minY, m_selectedBBox.maxX, m_selectedBBox.maxY };

            OsmResourceNet osmResource = OsmResourceNet.Create(bbox);
            osmResource.SetAlternateUrlList(alternateUrls);

            odsn.SetOsmResource(osmResource.ToXml());

            odsn.SetId(spatialDataSource.InstanceId);
            if ( legal != null )
                {
                odsn.SetCopyright(legal);
                }
            if ( cs != null )
                {
                odsn.SetGeoCS(cs);
                }
            odsn.SetSize(0);

            ModelDataNet mdn = ModelDataNet.Create(odsn);
            mdn.SetDataId(spatialEntity.InstanceId);
            mdn.SetDataName(spatialEntity["Name"].StringValue);

            return mdn;

            }

        private List<RealityDataNet> SubAPIPackager (IECSchema schema, RepositoryConnection connection, QueryModule queryModule, List<RequestedEntity> subAPIRequestedEntities, int major)
            {
            List<RealityDataNet> usgsSourceNetList = new List<RealityDataNet>();

            if ( subAPIRequestedEntities.Count == 0 )
                {
                return usgsSourceNetList;
                }

            IECClass spatialentityClass = schema.GetClass("SpatialEntity");

            IECRelationshipClass dataSourceRelClass = schema.GetClass("SpatialEntityToSpatialDataSource") as IECRelationshipClass;
            IECClass dataSourceClass = schema.GetClass("SpatialDataSource");
            RelatedInstanceSelectCriteria dataSourceRelCrit = new RelatedInstanceSelectCriteria(new QueryRelatedClassSpecifier(dataSourceRelClass, RelatedInstanceDirection.Forward, dataSourceClass), true);

            IECRelationshipClass metadataRelClass = schema.GetClass("SpatialEntityToMetadata") as IECRelationshipClass;
            IECClass metadataClass = schema.GetClass("Metadata");
            RelatedInstanceSelectCriteria metadataRelCrit = new RelatedInstanceSelectCriteria(new QueryRelatedClassSpecifier(metadataRelClass, RelatedInstanceDirection.Forward, metadataClass), true);

            IECRelationshipClass serverRelClass = schema.GetClass("ServerToSpatialDataSource") as IECRelationshipClass;
            IECClass serverClass = schema.GetClass("Server");
            RelatedInstanceSelectCriteria serverRelCrit = new RelatedInstanceSelectCriteria(new QueryRelatedClassSpecifier(serverRelClass, RelatedInstanceDirection.Backward, serverClass), true);


            ECQuery query = new ECQuery(spatialentityClass);
            query.SelectClause.SelectAllProperties = true;
            query.SelectClause.SelectedRelatedInstances.Add(dataSourceRelCrit);
            query.SelectClause.SelectedRelatedInstances.Add(metadataRelCrit);
            dataSourceRelCrit.SelectedRelatedInstances.Add(serverRelCrit);

            dataSourceRelCrit.SelectAllProperties = true;
            metadataRelCrit.SelectAllProperties = true;
            serverRelCrit.SelectAllProperties = true;

            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression(subAPIRequestedEntities.Select(e => e.ID.ToString()).ToArray()));

            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "usgsapi&rds&usgsee"));
            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>(IndexConstants.PRExtendedDataName, true));

            var queriedSpatialEntities = m_executeQuery(queryModule, connection, query, null);

            foreach ( var entity in queriedSpatialEntities )
                {
                //IECInstance metadataInstance = entity.GetRelationshipInstances().First(rel => rel.Target.ClassDefinition.Name == metadataClass.Name).Target;
                //IECInstance datasourceInstance = entity.GetRelationshipInstances().First(rel => rel.Target.ClassDefinition.Name == dataSourceClass.Name).Target;

                GenericInfo genericInfo = ExtractGenericInfo(entity, subAPIRequestedEntities.First(e => e.ID == entity.InstanceId));

                if(genericInfo.URI == null)
                    {
                    //The URI can be null in some entries of the rds subAPI. We skip these and don't include them in the package
                    continue;
                    }

                if ( (major == 1) && (genericInfo.Streamed == true) )
                    {
                    //We don't put streamed data in a v1 package
                    continue;
                    }

                RealityDataSourceNet rdsn = RealityDataSourceNet.Create(UriNet.Create(genericInfo.URI, genericInfo.FileInCompound), genericInfo.Type);

                SetRdsnFields(rdsn, genericInfo);

                RealityDataNet rdn = CreateAppropriateRDN(rdsn, genericInfo.Classification, genericInfo.SpatialEntityID, genericInfo.Name, genericInfo.Dataset, genericInfo.Footprint);

                usgsSourceNetList.Add(rdn);

                }

            return usgsSourceNetList;
            }

        //private void SortRealityDataSourceNet (List<ImageryDataNet> imgGroup, List<ModelDataNet> modelGroup, List<TerrainDataNet> terrainGroup, Tuple<RealityDataSourceNet, string> sourceTuple)
        //    {
        //    //This switch case is temporary. The best thing we should have done
        //    //was to create a method for this, but these "sourceNet" will probably
        //    //change soon, so everything here is temporary until the database is in
        //    //a more complete form
        //    switch ( sourceTuple.Item2 )
        //        {

        //        //TODO: Correct the switch case. The choice of the group for each class was not verified.
        //        case "Roadway":
        //        case "Bridge":
        //        case "Building":
        //        case "WaterBody":
        //        case "PointCloud":
        //            {
        //            //&&JFC TODO
        //            ModelDataNet modelData = ModelDataNet.Create ("", "", sourceTuple.Item1);
        //            modelGroup.Add(modelData);
        //            break;
        //            }
        //        case "Terrain":
        //            {
        //            //&&JFC TODO
        //            TerrainDataNet terrainData = TerrainDataNet.Create ("", "", sourceTuple.Item1);
        //            terrainGroup.Add(terrainData);
        //            break;
        //            }
        //        case "Imagery":
        //        default:
        //            {
        //            //&&JFC TODO
        //            ImageryDataNet imgData = ImageryDataNet.Create ("", "", sourceTuple.Item1, null);
        //            imgGroup.Add(imgData);
        //            break;
        //            }
        //        }
        //    }

        private RealityDataNet CreateAppropriateRDN (RealityDataSourceNet rdsn, string classification, string spatialEntityID, string name, string dataset, string footprint)
            {
            RealityDataNet dataNet;
            //This switch case is temporary. The best thing we should have done
            //was to create a method for this, but these "sourceNet" will probably
            //change soon, so everything here is temporary until the database is in
            //a more complete form
            switch ( classification )
                {

                //TODO: Correct the switch case. The choice of the group for each class was not verified.
                case "Model":
                case "Roadway":
                case "Bridge":
                case "Building":
                case "WaterBody":
                case "PointCloud":
                        {
                        dataNet = ModelDataNet.Create(rdsn);
                        break;
                        }
                case "Terrain":
                        {
                        dataNet = TerrainDataNet.Create(rdsn);
                        break;
                        }
                case "Imagery":
                default:
                        {
                        List<double> corners = ExtractCornersList(footprint);

                        dataNet = ImageryDataNet.Create(rdsn, corners);
                        break;
                        }
                }

            dataNet.SetDataId(spatialEntityID);
            dataNet.SetDataName(name);
            if ( dataset != null )
                {
                dataNet.SetDataset(dataset);
                }

            return dataNet;
            }

        private static List<double> ExtractCornersList (string footprint)
            {
            List<double> corners = new List<double>();
            PolygonModel polyModel = DbGeometryHelpers.CreatePolygonModelFromJson(footprint);

            double minX = double.MaxValue;
            double maxX = double.MinValue;
            double minY = double.MaxValue;
            double maxY = double.MinValue;

            foreach ( Double[] point in polyModel.Points )
                {
                minX = Math.Min(minX, point[0]);
                maxX = Math.Max(maxX, point[0]);
                minY = Math.Min(minY, point[1]);
                maxY = Math.Max(maxY, point[1]);
                }

            corners.Add(minX);
            corners.Add(minY);
            corners.Add(maxX);
            corners.Add(minY);
            corners.Add(maxX);
            corners.Add(maxY);
            corners.Add(minX);
            corners.Add(maxY);
            return corners;
            }

        private void SortRealityDataNet (List<ImageryDataNet> imgGroup, List<ModelDataNet> modelGroup, List<TerrainDataNet> terrainGroup, List<PinnedDataNet> pinnedGroup, List<RealityDataNet> rdnList)
            {
            foreach ( RealityDataNet rdn in rdnList )
                {
                if ( rdn is ModelDataNet )
                    {
                    modelGroup.Add(rdn as ModelDataNet);
                    }
                else if ( rdn is TerrainDataNet )
                    {
                    terrainGroup.Add(rdn as TerrainDataNet);
                    }
                else if ( rdn is ImageryDataNet )
                    {
                    imgGroup.Add(rdn as ImageryDataNet);
                    }
                else if ( rdn is PinnedDataNet )
                    {
                    pinnedGroup.Add(rdn as PinnedDataNet);
                    }
                }
            }

        private void UploadPackageInDatabase (IECInstance instance, string version, string requestor,
            string requestorVersion, string email, string userId, string boundingPolygon)
            {
            //using ( DbConnection sqlConnection = new SqlConnection(m_connectionString) )
            //    {
            //    sqlConnection.Open();
            //using ( DbCommand dbCommand = sqlConnection.CreateCommand() )
            //{
            GenericParamNameValueMap map = new GenericParamNameValueMap();

            //dbCommand.CommandText =
            //    "INSERT INTO dbo.Packages (Name, CreationTime, FileContent, PackageVersion, Requestor, RequestorVersion, BentleyInternal, BoundingPolygon, UserId) VALUES (@param0, @param1, @param2, @param3, @param4, @param5, @param6, @param7, @param8)";
            //dbCommand.CommandType = CommandType.Text;

            const string commandText =
                "INSERT INTO dbo.Packages (Name, CreationTime, FileContent, PackageVersion, Requestor, RequestorVersion, BentleyInternal, BoundingPolygon, UserId) VALUES (@param0, @param1, @param2, @param3, @param4, @param5, @param6, @param7, @param8)";

            //DbParameter param0 = dbCommand.CreateParameter();
            //param0.DbType = DbType.String;
            //param0.ParameterName = "@param0";
            //param0.Value = instance.InstanceId;
            //dbCommand.Parameters.Add(param0);

            map.AddParamNameValue("@param0", instance.InstanceId, DbType.String);

            //DbParameter param1 = dbCommand.CreateParameter();
            //param1.DbType = DbType.DateTime;
            //param1.ParameterName = "@param1";
            //param1.Value = DateTime.UtcNow;
            //dbCommand.Parameters.Add(param1);

            map.AddParamNameValue("@param1", DateTime.UtcNow, DbType.DateTime);

            using ( FileStream fstream = new FileStream(Path.GetTempPath() + instance.InstanceId, FileMode.Open) )
                {

                long longLength = fstream.Length;
                if ( longLength > int.MaxValue )
                    {
                    //Log.Logger.error("Package requested is too large.");
                    throw new Bentley.Exceptions.UserFriendlyException(
                        "Package requested is too large. Please reduce the size of the order");
                    }
                int intLength = Convert.ToInt32(longLength);
                byte[] fileBytes = new byte[fstream.Length];
                fstream.Seek(0, SeekOrigin.Begin);
                fstream.Read(fileBytes, 0, intLength);

                //DbParameter param2 = dbCommand.CreateParameter();
                //param2.DbType = DbType.Binary;
                //param2.ParameterName = "@param2";
                //param2.Value = fileBytes;
                //dbCommand.Parameters.Add(param2);

                map.AddParamNameValue("@param2", fileBytes, DbType.Binary);
                }
            //DbParameter param3 = dbCommand.CreateParameter();
            //param3.DbType = DbType.String;
            //param3.ParameterName = "@param3";
            //param3.Value = (version == null) ? (object) DBNull.Value : (object) version;
            //dbCommand.Parameters.Add(param3);

            map.AddParamNameValue("@param3", (version == null) ? (object) DBNull.Value : (object) version, DbType.String);

            //DbParameter param4 = dbCommand.CreateParameter();
            //param4.DbType = DbType.String;
            //param4.ParameterName = "@param4";
            //param4.Value = (requestor == null) ? (object) DBNull.Value : (object) requestor;
            //dbCommand.Parameters.Add(param4);

            map.AddParamNameValue("@param4", (requestor == null) ? (object) DBNull.Value : (object) requestor, DbType.String);

            //DbParameter param5 = dbCommand.CreateParameter();
            //param5.DbType = DbType.String;
            //param5.ParameterName = "@param5";
            //param5.Value = (requestorVersion == null) ? (object) DBNull.Value : (object) requestorVersion;
            //dbCommand.Parameters.Add(param5);

            map.AddParamNameValue("@param5", (requestorVersion == null) ? (object) DBNull.Value : (object) requestorVersion, DbType.String);

            //DbParameter param6 = dbCommand.CreateParameter();
            //param6.DbType = DbType.Boolean;
            //param6.ParameterName = "@param6";
            //param6.Value = email.EndsWith("bentley.com") || email.EndsWith("mailinator.com");
            //dbCommand.Parameters.Add(param6);

            map.AddParamNameValue("@param6", email.EndsWith("bentley.com") || email.EndsWith("mailinator.com"), DbType.Boolean);

            //DbParameter param7 = dbCommand.CreateParameter();
            //param7.DbType = DbType.String;
            //param7.ParameterName = "@param7";
            //param7.Value = (boundingPolygon == null) ? (object) DBNull.Value : (object) boundingPolygon;
            //dbCommand.Parameters.Add(param7);

            map.AddParamNameValue("@param7", (boundingPolygon == null) ? (object) DBNull.Value : (object) boundingPolygon, DbType.String);

            //DbParameter param8 = dbCommand.CreateParameter();
            //param8.DbType = DbType.String;
            //param8.ParameterName = "@param8";
            //param8.Value = (userId == null) ? (object) DBNull.Value : (object) userId;
            //dbCommand.Parameters.Add(param8);

            map.AddParamNameValue("@param8", (userId == null) ? (object) DBNull.Value : (object) userId, DbType.String);

            m_dbQuerier.ExecuteNonQueryInDb(commandText, map);

            //dbCommand.ExecuteNonQuery();
            //}
            //sqlConnection.Close();
            //}

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

        /// <summary>
        /// Extracts the packaging stats 
        /// </summary>
        /// <param name="query">The ECQuery requesting the stats. It is possible to filter by creation time using upper and lower bounds</param>
        /// <param name="connectionString">The connection string.</param>
        /// <param name="schema">The schema containing the stats ECClass</param>
        /// <param name="dbConnectionCreator">The factory creating the database connection.</param>
        /// <returns></returns>
        public static List<IECInstance> ExtractStats (ECQuery query, string connectionString, IECSchema schema, IDbConnectionCreator dbConnectionCreator)
            {
            List<IECInstance> StatsList = new List<IECInstance>();

            int i = 0;
            DateTime now = DateTime.Now;

            //By default, the start date and end Date are the entire last month (from the 1st of that month)
            DateTime startDate = (new DateTime(now.Year, now.Month, 1)).AddMonths(-1);
            bool useDefaultStartDate = true;

            DateTime endDate = (new DateTime(now.Year, now.Month, 1));
            bool useDefaultEndDate = true;

            string bentleyInternalStatement = GetBentleyInternalStatement(query);

            while ( i < query.WhereClause.Count )
                {
                WhereCriterion criterion = query.WhereClause[i];
                if ( i != (query.WhereClause.Count - 1) &&
                    query.WhereClause.GetLogicalOperatorAfter(i) != LogicalOperator.AND )
                    {
                    throw new UserFriendlyException("This query only uses AND logical operators");
                    }
                if ( criterion is PropertyExpression )
                    {
                    PropertyExpression propExp = criterion as PropertyExpression;
                    if ( propExp.LeftSideProperty.Name == "CreationTime" )
                        {
                        if ( propExp.Operator == RelationalOperator.LT || propExp.Operator == RelationalOperator.LTEQ )
                            {
                            if ( propExp.RightSide is DateTime )
                                {
                                if ( useDefaultEndDate || endDate > (DateTime) propExp.RightSide )
                                    {
                                    endDate = (DateTime) propExp.RightSide;
                                    }
                                useDefaultEndDate = false;
                                }
                            }
                        if ( propExp.Operator == RelationalOperator.GT || propExp.Operator == RelationalOperator.GTEQ )
                            {
                            if ( propExp.RightSide is DateTime )
                                {
                                if ( useDefaultStartDate || startDate < (DateTime) propExp.RightSide )
                                    {
                                    startDate = (DateTime) propExp.RightSide;
                                    }
                                useDefaultStartDate = false;
                                }
                            }
                        }
                    }
                i++;
                }
            if ( useDefaultEndDate != useDefaultStartDate )
                {
                throw new UserFriendlyException("Please specify both start and end times.");
                }
            using ( IDbConnection sqlConnection = dbConnectionCreator.CreateDbConnection(connectionString) )
                {
                sqlConnection.Open();
                using ( IDbCommand dbCommand = sqlConnection.CreateCommand() )
                    {
                    dbCommand.CommandText =
                        "SELECT t.Name, t.BoundingPolygon, t.CreationTime, t.UserId FROM dbo.Packages AS t WHERE t.CreationTime > @startTime AND t.CreationTime < @endTime" + bentleyInternalStatement;
                    dbCommand.CommandType = CommandType.Text;
                    dbCommand.Connection = sqlConnection;

                    IDbDataParameter param1 = dbCommand.CreateParameter();
                    param1.DbType = DbType.DateTime;
                    param1.ParameterName = "@startTime";
                    param1.Value = startDate;
                    dbCommand.Parameters.Add(param1);

                    IDbDataParameter param2 = dbCommand.CreateParameter();
                    param2.DbType = DbType.DateTime;
                    param2.ParameterName = "@endTime";
                    param2.Value = endDate;
                    dbCommand.Parameters.Add(param2);

                    using ( IDataReader reader = dbCommand.ExecuteReader() )
                        {
                        IECClass ecClass = schema.GetClass("PackageStats");
                        while ( reader.Read() )
                            {
                            IECInstance instance = ecClass.CreateInstance();

                            if ( !reader.IsDBNull(0) )
                                {
                                instance["Name"].StringValue = reader.GetString(0);
                                }
                            if ( !reader.IsDBNull(1) )
                                {
                                instance["BoundingPolygon"].StringValue = reader.GetString(1);
                                }
                            if ( !reader.IsDBNull(2) )
                                {
                                instance["CreationTime"].NativeValue = reader.GetDateTime(2);
                                }
                            if ( !reader.IsDBNull(3) )
                                {
                                instance["UserId"].StringValue = reader.GetString(3);
                                }

                            StatsList.Add(instance);
                            }
                        }
                    }
                }


            return StatsList;
            }

        internal static string GetBentleyInternalStatement (ECQuery query)
            {
            string bentleyInternalStatement = " AND t.BentleyInternal = 0 ";

            if ( query.ExtendedData.ContainsKey("includebentleyinternal") && query.ExtendedData["includebentleyinternal"].ToString().ToLower() == "true" )
                {
                bentleyInternalStatement = "";
                }
            return bentleyInternalStatement;
            }
        }
    }
