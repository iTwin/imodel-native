/*-------------------------------------------------------------------------------------
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
using System.Threading;

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
                        switch ( searchClass.Class.Name )
                            {
                            case "PreparedPackage":
                                IECInstance packageInstance = searchClass.Class.CreateInstance();
                                ECInstanceIdExpression exp = query.WhereClause[0] as ECInstanceIdExpression;
                                packageInstance.InstanceId = exp.RightSideString;
                                PackageStreamRetrievalController.SetStreamRetrieval(packageInstance, ConnectionString);
                                return new List<IECInstance> { packageInstance };

                            case "DownloadReport":

                                IECInstance DownloadReportInstance = searchClass.Class.CreateInstance();
                                ECInstanceIdExpression exp2 = query.WhereClause[0] as ECInstanceIdExpression;
                                DownloadReportInstance.InstanceId = exp2.RightSideString;
                                DRStreamRetrievalController.SetStreamRetrieval(DownloadReportInstance, ConnectionString);
                                return new List<IECInstance> { DownloadReportInstance };
                            default:
                                //We continue
                                break;
                            }

                        }

                    

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

                    using ( SqlConnection sqlConnection = new SqlConnection(ConnectionString) )
                        {
                        switch ( source.ToLower() )
                            {
                            case "index":
                                    {
                                    InstanceOverrider instanceOverrider = new InstanceOverrider(new DbQuerier());
                                    InstanceComplement instanceComplement = new InstanceComplement(new DbQuerier());
                                    IECQueryProvider helper = new SqlQueryProvider(query, querySettings, ConnectionString, schema);
                                    IEnumerable<IECInstance> instances = helper.CreateInstanceList();
                                    instanceOverrider.Modify(instances, DataSource.Index, ConnectionString);
                                    instanceComplement.Modify(instances, DataSource.Index, ConnectionString);
                                    return instances;
                                    }

                            case "usgsapi":
                                    {
                                    InstanceOverrider instanceOverrider = new InstanceOverrider(new DbQuerier());
                                    InstanceComplement instanceComplement = new InstanceComplement(new DbQuerier());
                                    IECQueryProvider helper = new UsgsAPIQueryProvider(query, querySettings, ConnectionString, schema);
                                    IEnumerable<IECInstance> instances = helper.CreateInstanceList();
                                    instanceOverrider.Modify(instances, DataSource.USGS, ConnectionString);
                                    instanceComplement.Modify(instances, DataSource.USGS, ConnectionString);
                                    return instances;
                                    }
                            case "all":
                                    {

                                    return QueryAllSources(query, querySettings, schema);

                                    }
                            default:
                                //throw new UserFriendlyException(String.Format("The class {0} cannot be queried.", searchClass.Class.Name));
                                //Log.Logger.error(String.Format("Query {0} aborted. The source chosen ({1}) is invalid", query.ID, source));
                                throw new UserFriendlyException("This source does not exist. Choose between " + SourceStringMap.GetAllSourceStrings());
                            }
                        }
                    }
                }
            catch ( Exception e )
                {
                if ( e is System.Data.Common.DbException )
                    {
                    //For now, we intercept all of these sql exceptions to prevent any "revealing" messages about the sql command.
                    //It would be nice to parse the exception to make it easier to pinpoint the problem for the user.
                    Log.Logger.error(String.Format("Query {0} aborted. DbException message : {1}. Stack Trace : {2}", query.ID, e.Message, e.StackTrace));
                    Exception innerEx = e.InnerException;
                    while ( innerEx != null )
                        {
                        Log.Logger.error(String.Format("Inner error message : {0}. Stack Trace : {1}", e.Message, e.StackTrace));
                        innerEx = innerEx.InnerException;
                        }
                    throw new UserFriendlyException("The server has encountered a problem while processing your request. Please verify the syntax of your request. If the problem persists, the server may be down");
                    }
                Log.Logger.error(String.Format("Query {0} aborted. Error message : {1}. Stack trace : {2}", query.ID, e.Message, e.StackTrace));
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

        private IEnumerable<IECInstance> QueryAllSources (ECQuery query, ECQuerySettings querySettings, IECSchema schema)
            {
            List<IECInstance> instanceList = new List<IECInstance>();

            IEnumerable<IECInstance> indexInstances = null;
            List<Exception> exceptions = new List<Exception>();
            Object exceptionsLock = new Object();
            Thread indexQuery = new Thread(() =>
            {
                try
                    {
                    InstanceOverrider instanceOverrider = new InstanceOverrider(new DbQuerier());
                    InstanceComplement instanceComplement = new InstanceComplement(new DbQuerier());
                    IECQueryProvider helper = new SqlQueryProvider(query, querySettings, ConnectionString, schema);
                    indexInstances = helper.CreateInstanceList().ToList();
                    instanceOverrider.Modify(indexInstances, DataSource.Index, ConnectionString);
                    instanceComplement.Modify(indexInstances, DataSource.Index, ConnectionString);
                    }
                catch ( Exception e )
                    {
                    indexInstances = new List<IECInstance>();
                    lock ( exceptionsLock )
                        {
                        exceptions.Add(e);
                        Log.Logger.error(String.Format("Index query aborted. Error message : {0}. Stack trace : {1}", e.Message, e.StackTrace));
                        }
                    }
            });

            IEnumerable<IECInstance> usgsInstances = null;
            Thread usgsQuery = new Thread(() =>
            {
                try
                    {
                    InstanceOverrider instanceOverrider = new InstanceOverrider(new DbQuerier());
                    InstanceComplement instanceComplement = new InstanceComplement(new DbQuerier());
                    IECQueryProvider helper = new UsgsAPIQueryProvider(query, querySettings, ConnectionString, schema);
                    usgsInstances = helper.CreateInstanceList();
                    instanceOverrider.Modify(usgsInstances, DataSource.USGS, ConnectionString);
                    instanceComplement.Modify(usgsInstances, DataSource.USGS, ConnectionString);
                    }
                catch ( Exception e )
                    {
                    usgsInstances = new List<IECInstance>();
                    lock ( exceptionsLock )
                        {
                        exceptions.Add(e);
                        Log.Logger.error(String.Format("USGS query aborted. Error message : {0}. Stack trace : {1}", e.Message, e.StackTrace));
                        }
                    }
            });
            indexQuery.Start();
            usgsQuery.Start();

            indexQuery.Join();
            usgsQuery.Join();

            instanceList.AddRange(indexInstances);
            instanceList.AddRange(usgsInstances);

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
                            Packager packager = new Packager(ConnectionString, (EnumerableBasedQueryHandler) ExecuteQuery);
                            packager.InsertPackageRequest(sender, connection, instance, sender.ParentECPlugin.QueryModule, version, 0, requestor, requestorVersion);
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

                        DownloadReportHelper.InsertInDatabase(streamBackedDescriptor.Stream, instance.InstanceId, ConnectionString);
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
