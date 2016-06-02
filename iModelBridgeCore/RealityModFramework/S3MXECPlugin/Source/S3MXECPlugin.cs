/*-------------------------------------------------------------------------------------
|
|     $Source: S3MXECPlugin/Source/S3MXECPlugin.cs $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/

using Bentley.Collections;
using Bentley.EC.Persistence;
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
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Reflection;
using Bentley.ECSystem.Configuration;
using S3MX.Source;

#if !IMSOFF
#endif

namespace S3MXECPlugin.Source
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
    public class S3MXECPlugin : ECStartupShutdownHandler
    {
        /// <summary>
        /// Instance of plugin.
        /// </summary>
        private static ECPlugin m_plugin = null;
        /// <summary>
        /// Name we will use for our ECSchema
        /// </summary>
        /// <remarks>This is only made public to simplify testing.</remarks>
        private const string SCHEMA_NAME = "S3MX";

        /// <summary>
        /// Name we will use for our ECPlugin
        /// </summary>
        /// <remarks>This is only made public to simplify testing.</remarks>
        private const string PLUGIN_NAME = "S3MXECPlugin";

        private string m_connectionString = ConfigurationRoot.GetAppSetting("S3MXECPConnectionString");

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
        /// Create our navigation module
        /// </summary>
        public S3MXNavigationModule navigationModule = new S3MXNavigationModule();

        private void BuildPlugin(ECPluginBuilder builder)
        {
            builder
                .SetDisplayLabel("S3MXECPlugin")
                .SetRepositorySupport(
                    new LocationFormatInfo()
                    {
                        BrowsingType = ConnectionFormatInfo.LocationBrowsingType.BrowseFolders
                    },
                    GetRepositoryIdentifier)
                .SetSchemaSupport(PopulateSchemas)
                .SetQuerySupport((EnumerableBasedQueryHandler)ExecuteQuery)
                .SetOperationSupport<UpdateOperation>(UploadFile)
                .SetNavigationSupport(navigationModule);

            IndexPolicyHandler.InitializeHandlers(builder);
        }

        /// <summary>
        /// Delegate function that need to be use by SetOperationSupport in the BuildPlugin function
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="connection"></param>
        /// <param name="operation"></param>
        /// <param name="instance"></param>
        /// <param name="comments"></param>
        /// <param name="writeModifiers"></param>
        /// <param name="extendedParameters"></param>
        private void UploadFile(OperationModule sender, RepositoryConnection connection, UpdateOperation operation, IECInstance instance, string comments, WriteModifiers writeModifiers, IExtendedParameters extendedParameters)
        {
            StreamBackedDescriptor streamBackedDescriptor = StreamBackedDescriptorAccessor.GetFrom(instance);

            S3MXAzureBlobApi azureBlobApi = new S3MXAzureBlobApi(m_connectionString);
            string uploadDestination = instance.InstanceId;
            if(!azureBlobApi.UploadFile("s3mx", uploadDestination, streamBackedDescriptor))
            {
                throw new NotImplementedException();
            }
        }

        private RepositoryIdentifier GetRepositoryIdentifier(RepositoryModule module, ECSession session, string location, IExtendedParameters extendedParameters)
        {
            //Try to obtain the repository identifier for the provided location from the list of known identifiers
            ICollection<RepositoryIdentifier> knownRepositories = module.GetKnownRepositories(session, extendedParameters);
            RepositoryIdentifier knownRepository = knownRepositories.FirstOrDefault((repositoryIdentifier) => string.Equals(location, repositoryIdentifier.Location, StringComparison.Ordinal));

            if (knownRepository != null)
            {
                return knownRepository;
            }

            if (location == "Server")
            {
                var identifier = new RepositoryIdentifier(module.ParentECPlugin.ECPluginId, location, location, location, null);
                module.RepositoryVerified(session, identifier);
                return identifier;
            }
            return null;
        }

        private void PopulateSchemas(SchemaModule module, ECPluginSchemaManager schemaManager)
        {
            //This method is called a single time by the schema module. So we can directly load the schema here.
            //if the schema was big, we could also just register its name in the schema manager with a load-delegate
            ECSchemaXmlStreamReader schemaReader = new ECSchemaXmlStreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("S3MXECPlugin.01.00.ecschema.xml"));
            IECSchema schemaFromXML = schemaReader.Deserialize();
            schemaManager.AddSchema(ref schemaFromXML);
        }

        private IEnumerable<IECInstance> ExecuteQuery(QueryModule sender, RepositoryConnection connection, ECQuery query, ECQuerySettings querySettings)
        {
            try
            {
                SearchClass searchClass = query.SearchClasses.First();
                
                // this is where we download a file
                if ((querySettings != null) && ((querySettings.LoadModifiers & LoadModifiers.IncludeStreamDescriptor) != LoadModifiers.None) && (searchClass.Class.Name == "Document"))
                {
                    // extract the instanceId which is the path for the wanted resource
                    ECInstanceIdExpression exp = query.WhereClause[0] as ECInstanceIdExpression;
                    string targetDocument = exp.RightSideString;

                    // get the desired class for the download
                    IECInstance documentInstance = searchClass.Class.CreateInstance();
                    documentInstance.InstanceId = "s3mx";

                    // downloading the file into the current stream
                    S3MXAzureBlobApi azureBlobApi = new S3MXAzureBlobApi(m_connectionString);
                    MemoryStream mStream = new MemoryStream();
                    if(!azureBlobApi.DownloadFile("s3mx", targetDocument, ref mStream))
                    {
                        throw new UserFriendlyException("The Azure blob container does not exist.");
                    }
                    else
                    {
                        StreamBackedDescriptor desc = new StreamBackedDescriptor(mStream, documentInstance.InstanceId, mStream.Length, DateTime.UtcNow);
                        StreamBackedDescriptorAccessor.SetIn(documentInstance, desc);
                    }

                    return new List<IECInstance> { documentInstance };
                }

                return new List<IECInstance>();
            }
            catch (System.Data.Common.DbException ex)
            {
                Exception innerEx = ex.InnerException;
                while (innerEx != null)
                {
                    Log.Logger.error(String.Format("Inner error message : {0}", ex.Message));
                    innerEx = innerEx.InnerException;
                }
                throw new UserFriendlyException("The server has encountered a problem while processing your request. Please verify the syntax of your request. If the problem persists, the server may be down");
            }
            catch (Exception e)
            {
                Log.Logger.error(String.Format("Query {0} aborted. Error message : {1}", query.ID, e.Message));
                throw;
            }
        }
    }
}
