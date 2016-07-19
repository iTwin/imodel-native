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
                        Packager packager = new Packager(ConnectionString, (EnumerableBasedQueryHandler) ExecuteQuery);
                        packager.InsertPackageRequest(sender, connection, instance, sender.ParentECPlugin.QueryModule);
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
