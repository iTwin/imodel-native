/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/IndexECPlugin.cs $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
using System.Security.Claims;
using System.Threading;
//using System.Net;

#if !IMSOFF
using System.IdentityModel.Configuration;
using System.IdentityModel.Services;
using System.IdentityModel.Services.Configuration;
using System.IdentityModel.Tokens;
#endif

#if CONNECTENV
using Bentley.SelectServer.SaaS.Client;
using Bentley.SelectServer.SaaS.Client.FeatureTracking;
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

#if CONNECTENV

        private static IUsageTrackingContext m_usageTrackingContext = null;
        private static IUsageTrackingSender m_usageTrackingSender = null;

        

        /// <summary>
        /// Usage tracking context
        /// </summary>
        public static IUsageTrackingContext UsageTrackingContext
            {
            get
                {
                if ( null == m_usageTrackingContext )
                    {
                    m_usageTrackingContext = UsageTrackingContextFactory.Current.CreateContext(typeof(IndexECPlugin).Assembly.GetName().Version.ToString(), Environment.MachineName, IndexConstants.ProductId);
                    }
                return m_usageTrackingContext;
                }
            }

        /// <summary>
        /// Usage tracking sender
        /// </summary>
        public static IUsageTrackingSender UsageTrackingSender
            {
            get
                {
                string ftConnectionString = ConfigurationRoot.GetAppSetting("RECPFeatureTrackingConnectionString");
                if ( null == m_usageTrackingSender && !String.IsNullOrEmpty(ftConnectionString))
                    m_usageTrackingSender = new FeatureSender(ConfigurationRoot.GetAppSetting("RECPFeatureTrackingConnectionString"));
                return m_usageTrackingSender;
                }
            }

#endif

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
                .SetOperationSupport<InsertOperation>(ExecuteInsertOperation)
                .SetOperationSupport<UpdateOperation>(ExecuteUpdateOperation);

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

                    if ( (querySettings != null) && ((querySettings.LoadModifiers & LoadModifiers.IncludeStreamDescriptor) != LoadModifiers.None) )
                    {
                        SqlDbConnectionCreator sqlDbConnectionCreator = new SqlDbConnectionCreator();
                        switch ( searchClass.Class.Name )
                            {
                            case "PreparedPackage":
                                IECInstance packageInstance = searchClass.Class.CreateInstance();
                                ECInstanceIdExpression exp = query.WhereClause[0] as ECInstanceIdExpression;
                                packageInstance.InstanceId = exp.RightSideString;
                                RetrievalController.RetrievePackage(packageInstance, ConnectionString, sqlDbConnectionCreator);
                                return new List<IECInstance> { packageInstance };

                            case "DownloadReport":

                                IECInstance DownloadReportInstance = searchClass.Class.CreateInstance();
                                ECInstanceIdExpression exp2 = query.WhereClause[0] as ECInstanceIdExpression;
                                DownloadReportInstance.InstanceId = exp2.RightSideString;
                                RetrievalController.RetrieveDownloadReport(DownloadReportInstance, ConnectionString, sqlDbConnectionCreator);
                                return new List<IECInstance> { DownloadReportInstance };
                            default:
                                //We continue
                                break;
                            }

                        }

                    if(searchClass.Class.Name == "PackageStats")
                        {
                        if(!IsBentleyEmployee(connection))
                            {
                            throw new UserFriendlyException("This operation is not permitted");
                            }
                        SqlDbConnectionCreator sqlDbConnectionCreator = new SqlDbConnectionCreator();
                        return Packager.ExtractStats(query, ConnectionString, schema, sqlDbConnectionCreator);
                        }



                    if ( searchClass.Class.GetCustomAttributes("QueryType") == null || searchClass.Class.GetCustomAttributes("QueryType")["QueryType"].IsNull )
                        {
                        throw new UserFriendlyException(String.Format("The class {0} cannot be queried.", searchClass.Class.Name));
                        }

                    string[] sources;

                    if ( query.ExtendedData.ContainsKey("source") )
                        {
                        sources = query.ExtendedData["source"].ToString().Split('&');
                        }
                    else
                        {
                        sources = new string[] { searchClass.Class.GetCustomAttributes("QueryType")["QueryType"].StringValue };
                        }

                    List<DataSource> sourcesList = new List<DataSource>();
                    foreach ( string source in sources )
                        {
                        try
                            {
                            sourcesList.Add(SourceStringMap.StringToSource(source));
                            }
                        catch ( NotImplementedException )
                            {
                            throw new UserFriendlyException("This source does not exist. Choose between " + SourceStringMap.GetAllSourceStrings());
                            }
                        }
                    return QueryMultipleSources(query, querySettings, schema, sourcesList, Convert.ToBase64String(Encoding.UTF8.GetBytes(GetToken(connection))));
                    }
                }
            catch ( Exception e )
                {
                if ( e is System.Data.Common.DbException )
                    {
                    if ( e is SqlException )
                        {
                        var sqlEx = e as SqlException;
                        Log.Logger.error(String.Format("Query {0} aborted. SqlException number : {3}. SqlException message : {1}. Stack Trace : {2}", query.ID, sqlEx.Message, sqlEx.StackTrace, sqlEx.Number));
                        if ( sqlEx.Number == 10928 || sqlEx.Number == 10929 )
                            throw new EnvironmentalException(String.Format("The server is currently busy. Please try again later"));
                        }
                    else
                        {
                        Log.Logger.error(String.Format("Query {0} aborted. DbException message : {1}. Stack Trace : {2}", query.ID, e.Message, e.StackTrace));
                        }
                    Exception innerEx = e.InnerException;
                    while ( innerEx != null )
                        {
                        Log.Logger.error(String.Format("Inner error message : {0}. Stack Trace : {1}", e.Message, e.StackTrace));
                        innerEx = innerEx.InnerException;
                        }
                    throw new EnvironmentalException("The server has encountered a problem while processing your request. Please verify the syntax of your request. If the problem persists, the server may be down");
                    }
                Log.Logger.error(String.Format("Query {0} aborted. Error message : {1}. Stack trace : {2}", query.ID, e.Message, e.StackTrace));
                if ( e is UserFriendlyException )
                    {
                    throw;
                    }
                if ( e is EnvironmentalException )
                    {
                    throw;
                    }
                else
                    {
                    throw new Exception("Internal Error.");
                    }
                }
            }

        private IEnumerable<IECInstance> QueryMultipleSources (ECQuery query, ECQuerySettings querySettings, IECSchema schema, IEnumerable<DataSource> sources, string base64token)
            {
            List<IECInstance> instanceList = new List<IECInstance>();

            IEnumerable<IECInstance> indexInstances = null;
            List<Exception> exceptions = new List<Exception>();
            Object exceptionsLock = new Object();

    //        ServicePointManager.ServerCertificateValidationCallback +=
    //(sender, cert, chain, sslPolicyErrors) => true;

            Thread indexQuery = null;
            if ( sources.Contains(DataSource.Index) || sources.Contains(DataSource.All) )
                {
                indexQuery = new Thread(() =>
                {
                    try
                        {
                        //InstanceOverrider instanceOverrider = new InstanceOverrider(new DbQuerier());
                        //InstanceComplement instanceComplement = new InstanceComplement(new DbQuerier());
                        IECQueryProvider helper = new SqlQueryProvider(query, querySettings, new DbQuerier(ConnectionString, new SqlDbConnectionCreator()), schema);
                        indexInstances = helper.CreateInstanceList().ToList();
                        //instanceOverrider.Modify(indexInstances, DataSource.Index, ConnectionString);
                        //instanceComplement.Modify(indexInstances, DataSource.Index, ConnectionString);
                        }
                    catch ( Exception e )
                        {
                        indexInstances = new List<IECInstance>();
                        lock ( exceptionsLock )
                            {
                            exceptions.Add(e);
                            Log.Logger.error(String.Format("Index query aborted. Error message : {0}. Stack trace : {1}", e.Message, e.StackTrace));
                            if ( e is AggregateException )
                                {
                                AggregateException ae = (AggregateException) e;
                                foreach ( Exception ie in ae.InnerExceptions )
                                    {
                                    Log.Logger.error("Index Aggregate exception message: " + ie.GetBaseException().Message);
                                    }

                                }
                            }
                        }
                });
                }
            IEnumerable<IECInstance> usgsInstances = null;
            Thread usgsQuery = null;
            if ( sources.Contains(DataSource.USGS) || sources.Contains(DataSource.All) )
                {

                usgsQuery = new Thread(() =>
                {
                    try
                        {
                        //InstanceOverrider instanceOverrider = new InstanceOverrider(new DbQuerier());
                        //InstanceComplement instanceComplement = new InstanceComplement(new DbQuerier());
                        IECQueryProvider helper = new UsgsSubAPIQueryProvider(query, querySettings, new DbQuerier(ConnectionString, new SqlDbConnectionCreator()), schema);
                        usgsInstances = helper.CreateInstanceList();
                        //instanceOverrider.Modify(usgsInstances, DataSource.USGS, ConnectionString);
                        //instanceComplement.Modify(usgsInstances, DataSource.USGS, ConnectionString);
                        }
                    catch ( Exception e )
                        {
                        usgsInstances = new List<IECInstance>();
                        lock ( exceptionsLock )
                            {
                            exceptions.Add(e);
                            Log.Logger.error(String.Format("USGS query aborted. Error message : {0}. Stack trace : {1}", e.Message, e.StackTrace));
                            if ( e is AggregateException )
                                {
                                AggregateException ae = (AggregateException) e;
                                foreach ( Exception ie in ae.InnerExceptions )
                                    {
                                    Log.Logger.error("USGS Aggregate exception message: " + ie.GetBaseException().Message);
                                    }

                                }
                            }
                        }
                });

                }

            IEnumerable<IECInstance> rdsInstances = null;
            Thread rdsQuery = null;
            //TODO: Reenable RDS when using All. This requires the operator "IN" to be usable in RDS (ideally) and some changes to the sub-index.
            if ( sources.Contains(DataSource.RDS) /* || sources.Contains(DataSource.All)*/ )
                {

                rdsQuery = new Thread(() =>
                {
                    try
                        {
                        //InstanceOverrider instanceOverrider = new InstanceOverrider(new DbQuerier());
                        //InstanceComplement instanceComplement = new InstanceComplement(new DbQuerier());
                        IECQueryProvider helper = new RdsAPIQueryProvider(query, querySettings, new DbQuerier(ConnectionString, new SqlDbConnectionCreator()), schema, base64token);
                        rdsInstances = helper.CreateInstanceList();
                        //instanceOverrider.Modify(usgsInstances, DataSource.RDS, ConnectionString);
                        //instanceComplement.Modify(usgsInstances, DataSource.RDS, ConnectionString);
                        }
                    catch ( Exception e )
                        {
                        rdsInstances = new List<IECInstance>();
                        lock ( exceptionsLock )
                            {
                            exceptions.Add(e);
                            Log.Logger.error(String.Format("RDS query aborted. Error message : {0}. Stack trace : {1}", e.GetBaseException().Message, e.StackTrace));
                                if (e is AggregateException)
                                {
                                    AggregateException ae = (AggregateException) e;
                                    foreach (Exception ie in ae.InnerExceptions)
                                    {
                                    Log.Logger.error("RDS Aggregate exception message: " + ie.GetBaseException().Message);
                                    }

                                }
                            }
                        }
                });

                }
            if ( indexQuery != null )
                {
                indexQuery.Start();
                }
            if ( usgsQuery != null )
                {
                usgsQuery.Start();
                }
            if ( rdsQuery != null )
                {
                rdsQuery.Start();
                }

            if ( indexQuery != null )
                {
                indexQuery.Join();
                }
            if ( usgsQuery != null )
                {
                usgsQuery.Join();
                }
            if ( rdsQuery != null )
                {
                rdsQuery.Join();
                }

            if ( indexInstances != null )
                {
                instanceList.AddRange(indexInstances);
                }

            if ( usgsInstances != null )
                {
                instanceList.AddRange(usgsInstances);
                }
            if ( rdsInstances != null )
                {
                instanceList.AddRange(rdsInstances);
                }

            if ( exceptions.Count != 0 )
                {
                if ( exceptions.Any(e => e is UserFriendlyException) )
                    {
                    //We throw this exception, since it is caused by the user's wrong input
                    throw exceptions.First(e => e is UserFriendlyException);
                    }
                if ( instanceList.Count == 0 )
                    {
                    //We did not return any instance and there were errors. 
                    //In this case, we throw the first one (DbExceptions in priority). 
                    //
                    if ( exceptions.Any(e => e is System.Data.Common.DbException) )
                        {
                        throw exceptions.First(e => e is System.Data.Common.DbException);
                        }
                    throw exceptions.First();
                    }
                }

            return instanceList;
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


                SqlDbConnectionCreator sqlDbConnectionCreator = new SqlDbConnectionCreator();
                switch ( fileHolderAttribute["Type"].StringValue )
                    {

                    case "PreparedPackage":
                        //var resourceManager = new FileResourceManager(connection);
                        //FileBackedDescriptorAccessor.SetIn(instance, new FileBackedDescriptor(""));
                        //var packageRetrievalController = new PackageRetrievalController(instance, resourceManager, operation, m_packagesLocation, m_connectionString);
                        //packageRetrievalController.Run();
                        RetrievalController.RetrievePackage(instance, ConnectionString, sqlDbConnectionCreator);

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
                Log.Logger.error(String.Format("Aborting retrieval of instance {0}. Error message : {1}. Stack trace : {2}", instance.InstanceId, e.Message, e.StackTrace));
                if ( e is UserFriendlyException )
                    {
                    throw;
                    }
                else
                    {
                    throw new Exception("Internal Error.");
                    }
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
                        int version;
                        string requestor = null;
                        string requestorVersion = null;
                        if ( (extendedParameters.ContainsKey("Version")) && (extendedParameters["Version"].ToString() == "2") )
                            {
                            version = 2;
                            }
                        else
                            {
                            version = 1;
                            }

                        if ( extendedParameters.ContainsKey("Requestor") )
                            {
                            requestor = extendedParameters["Requestor"].ToString();
                            }
                        if ( extendedParameters.ContainsKey("RequestorVersion") )
                            {
                            requestorVersion = extendedParameters["RequestorVersion"].ToString();
                            }
                        string fullSchemaName = sender.ParentECPlugin.SchemaModule.GetSchemaFullNames(connection).First();
                        IECSchema schema = sender.ParentECPlugin.SchemaModule.GetSchema(connection, fullSchemaName);
                        Packager packager = new Packager(new DbQuerier(ConnectionString, new SqlDbConnectionCreator()), (EnumerableBasedQueryHandler) ExecuteQuery);
                        packager.InsertPackageRequest(schema, connection, instance, sender.ParentECPlugin.QueryModule, version, 0, requestor, requestorVersion);
                        return;

                    default:
                        Log.Logger.error(String.Format("Package request aborted. The class {0} cannot be inserted", className));
                        throw new Bentley.Exceptions.UserFriendlyException("The only insert operation permitted is a PackageRequest instance insertion.");
                    }
                }
            catch ( Exception e )
                {
                Log.Logger.error(String.Format("Package {1} creation aborted. Error message : {0}. Stack trace : {2}", e.Message, instance.InstanceId, e.StackTrace));
                if ( e is UserFriendlyException )
                    {
                    throw;
                    }
                else
                    {
                    throw new Exception("Internal Error.");
                    }
                }

            }


        internal void ExecuteUpdateOperation
        (
            OperationModule sender,
            RepositoryConnection connection,
            UpdateOperation operation,
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
                    case "DownloadReport":
                        StreamBackedDescriptor streamBackedDescriptor;
                        try
                            {
                            streamBackedDescriptor = StreamBackedDescriptorAccessor.GetFrom(instance);
                            }
                        catch (System.Collections.Generic.KeyNotFoundException)
                            {
                            throw new UserFriendlyException("A download report insertion must contain a file");
                            }

                        SqlDbConnectionCreator sqlDbConnectionCreator = new SqlDbConnectionCreator();
                        DownloadReportHelper.InsertInDatabase(streamBackedDescriptor.Stream, instance.InstanceId, ConnectionString, sqlDbConnectionCreator);
                        return;

                    default:
                        Log.Logger.error(String.Format("Package request aborted. The class {0} cannot be inserted", className));
                        throw new Bentley.Exceptions.UserFriendlyException("The only update operation permitted is a DownloadReport instance update.");
                    }
                }
            catch ( Exception e )
                {
                Log.Logger.error(String.Format("Package {1} creation aborted. Error message : {0}. Stack trace : {2}", e.Message, instance.InstanceId, e.StackTrace));
                if ( e is UserFriendlyException )
                    {
                    throw;
                    }
                else
                    {
                    throw new Exception("Internal Error.");
                    }
                }
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
//#if !IMSOFF
                new ConnectionFormatFieldInfo() {ID = "Token", DisplayName = "Token", IsRequired = true, IsAdvanced = true, IsCredential = true }
//#endif
                }.ToArray();
            }

#if !IMSOFF

        private readonly Lazy<FederationConfiguration> m_federationConfiguration =
    new Lazy<FederationConfiguration>(() => FederatedAuthentication.FederationConfiguration);

        /// <summary>
        /// 
        /// </summary>
        private FederationConfiguration FederationConfiguration
            {
            get
                {
                return m_federationConfiguration.Value;
                }
            }

        private void OpenConnection (ConnectionModule sender,
                                    RepositoryConnection connection,
                                    IExtendedParameters extendedParameters)
        {

            string token = GetToken(connection);

            try
                {
                //string serializedToken = Encoding.UTF8.GetString(Convert.FromBase64String(token.Trim()));

                using ( var reader = XmlReader.Create(new StringReader(token)) )
                    {
                    var bootstrapToken = FederationConfiguration.IdentityConfiguration.SecurityTokenHandlers.ReadToken(reader);
                    SecurityTokenHandler handler = FederationConfiguration.IdentityConfiguration.SecurityTokenHandlers[bootstrapToken];

                    var cic = handler.ValidateToken(bootstrapToken);
                    var cp = new ClaimsPrincipal(cic);
                    System.Web.HttpContext.Current.User = cp;
                    System.Threading.Thread.CurrentPrincipal = cp;

                    Log.Logger.debug("TokenAuthenticator: authenticated with user: {0}, ", cp.Identity == null ? "null" : cp.Identity.Name);
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

#if CONNECTENV
        private static string GetClaimValue (string claimType)
            {
            var user = (ClaimsPrincipal) System.Web.HttpContext.Current.User;
            if ( user == null )
                {
                return null;
                }
            var claimsIdentity = (ClaimsIdentity) user.Identity;
            if ( claimsIdentity.Claims == null )
                {
                return null;
                }
            var claim = claimsIdentity.Claims.FirstOrDefault(c => c.Type == claimType);
            if ( null == claim )
                return null;
            return claim.Value;
            }

        /// <summary>
        /// Mark a feature for usage tracking purpose
        /// </summary>
        /// <param name="featureGuid">The unique GUID associated to the feature</param>
        /// <param name="additionalProperties">User defined properties to be included in the feature marking</param>
        public static void MarkFeature (Guid featureGuid, IEnumerable<KeyValuePair<string, object>> additionalProperties = null)
            {

            IUsageTrackingContext eventContext = UsageTrackingContext.AsVolatile();
            IUsageTrackingSender eventSender = UsageTrackingSender;
            if ( null == eventContext || eventSender == null)
                return;

            string userId = GetClaimValue("http://schemas.bentley.com/ws/2011/03/identity/claims/userid");

            string ipAddress = System.Web.HttpContext.Current.Request.ServerVariables["HTTP_X_FORWARDED_FOR"];
            if ( string.IsNullOrEmpty(ipAddress) )
                {
                ipAddress = System.Web.HttpContext.Current.Request.UserHostAddress;
                }
            else
                {
                ipAddress = ipAddress.Split(',')[0];
                }

            eventContext.UserImsId = new Guid(userId);
            eventContext.FeatureId = featureGuid;
            eventContext.ProjectId = Guid.Empty;
            eventContext.ClientId = ipAddress;

            // Add additional arbitrary properties.
            if ( additionalProperties != null )
                {
                foreach ( KeyValuePair<string, object> additionalProperty in additionalProperties )
                    {
                    eventContext.Properties.Add(additionalProperty);
                    }
                }

            eventSender.MarkFeature(eventContext.AsMarkEvent());
            }

#endif
        /// <summary>
        /// Get email address of the caller from the connection
        /// </summary>
        /// <param name="connection"></param>
        /// <returns></returns>
        public static string GetEmailFromConnection
        (
            RepositoryConnection connection
        )
            {
                try
                {
                    //var isAuth = System.Web.HttpContext.Current.User.Identity.IsAuthenticated;
                    var isAuth = System.Web.HttpContext.Current.User.Identity.IsAuthenticated;
                    if (!isAuth)
                    {
                        throw (new Exception());
                    }
                    else
                    {
                        //IEnumerable<Claim> claims = ((ClaimsIdentity) System.Web.HttpContext.Current.User.Identity).Claims;

                        IEnumerable<Claim> claims =
                            ((ClaimsIdentity) System.Web.HttpContext.Current.User.Identity).Claims;
                        Claim organizationClaim =
                            claims.FirstOrDefault(
                                    c => c.Type == "http://schemas.bentley.com/ws/2011/03/identity/claims/organizationid");
                        Claim emailClaim =
                            claims.FirstOrDefault(
                                    c => c.Type == "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/emailaddress");
                        return emailClaim.Value.ToLower();
                    }
                }
                catch
                {
                }
                //if on connect environment use this
                string token = GetToken(connection);
                
                var xml = new XmlDocument();
                xml.LoadXml(token);
                var nsmgr = new XmlNamespaceManager(xml.NameTable);
                nsmgr.AddNamespace("saml", "urn:oasis:names:tc:SAML:1.0:assertion");

                return xml.SelectSingleNode("//saml:AttributeStatement//saml:Attribute[@AttributeName='emailaddress']", nsmgr).InnerText.ToLower();
                }

        private static string GetToken(RepositoryConnection connection)
            {
                try
                {
                    return Encoding.UTF8.GetString(Convert.FromBase64String(connection.ConnectionInfo.GetField("Token").Value.Trim()));
                }
                catch (Exception)
                {
                    return connection.ConnectionInfo.GetField("Token").Value;
                }
            }

            /// <summary>
        /// Get user Id address of the caller from the connection
        /// </summary>
        /// <param name="connection"></param>
        /// <returns></returns>
        public static string GetUserIdFromConnection
        (
            RepositoryConnection connection
        )
            {
                try
                {
                    //var isAuth = System.Web.HttpContext.Current.User.Identity.IsAuthenticated;
                    var isAuth = System.Web.HttpContext.Current.User.Identity.IsAuthenticated;
                    if (!isAuth)
                    {
                        throw (new Exception());
                    }
                    else
                    {
                        //IEnumerable<Claim> claims = ((ClaimsIdentity) System.Web.HttpContext.Current.User.Identity).Claims;

                        IEnumerable<Claim> claims =
                            ((ClaimsIdentity) System.Web.HttpContext.Current.User.Identity).Claims;
                        Claim userIdClaim =
                            claims.FirstOrDefault(
                                c => c.Type == "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/userid");
                        return userIdClaim.Value.ToLower();
                    }
                }
                catch
                {
                    //if on connect environment use this
                    string token = GetToken(connection);

                    var xml = new XmlDocument();
                    xml.LoadXml(token);
                    var nsmgr = new XmlNamespaceManager(xml.NameTable);
                    nsmgr.AddNamespace("saml", "urn:oasis:names:tc:SAML:1.0:assertion");

                    return
                        xml.SelectSingleNode("//saml:AttributeStatement//saml:Attribute[@AttributeName='userid']", nsmgr)
                            .InnerText.ToLower();
                }
            }

            /// <summary>
            /// Determines if caller is a Bentley Employee
            /// </summary>
            /// <param name="connection"></param>
            /// <returns></returns>
            public static bool IsBentleyEmployee
            (
                RepositoryConnection connection
            )
            {
                try
                {
                    //var isAuth = System.Web.HttpContext.Current.User.Identity.IsAuthenticated;
                    var isAuth = System.Web.HttpContext.Current.User.Identity.IsAuthenticated;
                    if (!isAuth)
                    {
                        throw (new Exception());
                    }
                    else
                    {
                        //IEnumerable<Claim> claims = ((ClaimsIdentity) System.Web.HttpContext.Current.User.Identity).Claims;

                        IEnumerable<Claim> claims =
                            ((ClaimsIdentity) System.Web.HttpContext.Current.User.Identity).Claims;
                        Claim employeeClaim =
                            claims.FirstOrDefault(
                                c => c.Type == "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/isbentleyemployee");
                        return (employeeClaim.Value.ToLower() == "true");
                    }
                }
                catch
                {

//if on connect environment use this
                    string token = GetToken(connection);
                    var xml = new XmlDocument();
                    xml.LoadXml(token);
                    var nsmgr = new XmlNamespaceManager(xml.NameTable);
                    nsmgr.AddNamespace("saml", "urn:oasis:names:tc:SAML:1.0:assertion");

                    return
                        xml.SelectSingleNode(
                                "//saml:AttributeStatement//saml:Attribute[@AttributeName='isbentleyemployee']", nsmgr)
                            .InnerText.ToLower() == "true";
                }

            }

        }



    }

