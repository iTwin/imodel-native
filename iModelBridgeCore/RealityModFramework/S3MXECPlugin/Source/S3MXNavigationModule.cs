using Bentley.Collections;
using Bentley.EC.PluginBuilder.Modules;
using Bentley.EC.Presentation.Navigation;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using Bentley.ECSystem;
using Bentley.ECSystem.Configuration;
using Bentley.ECSystem.Repository;
using Bentley.Exceptions;
using Microsoft.WindowsAzure.Storage.Blob;
using System;
using System.Collections.Generic;
using System.Linq;

namespace S3MX.Source
{
    /// <summary>
    /// Class that implement NavigationModule
    /// </summary>
    public class S3MXNavigationModule : NavigationModule
    {
        /// <summary>
        /// GetNodeByMoniker not yet implemented in the navigation module
        /// </summary>
        /// <param name="connection"></param>
        /// <param name="moniker"></param>
        /// <param name="extendedParameters"></param>
        /// <returns></returns>
        public override NavNode GetNodeByMoniker(RepositoryConnection connection, string moniker, IExtendedParameters extendedParameters)
        {
            return null;
        }
        /// <summary>
        /// GetParent not yet implemented in the navigation module
        /// </summary>
        /// <param name="connection"></param>
        /// <param name="ecNavNode"></param>
        /// <param name="extendedParameters"></param>
        /// <returns></returns>
        public override NavNode GetParent(RepositoryConnection connection, NavNode ecNavNode, IExtendedParameters extendedParameters)
        {
            return null;
        }
        /// <summary>
        /// GetMonikerByNode not yet implemented in the navigation module
        /// </summary>
        /// <param name="connection"></param>
        /// <param name="navNode"></param>
        /// <param name="extendedParameters"></param>
        /// <returns></returns>
        public override string GetMonikerByNode(RepositoryConnection connection, NavNode navNode, IExtendedParameters extendedParameters)
        {
            return null;
        }
        /// <summary>
        /// GetNodeFromKey return a node with the desired class
        /// </summary>
        /// <param name="connection"></param>
        /// <param name="key"></param>
        /// <param name="extendedParameters"></param>
        /// <returns></returns>
        public override NavNode GetNodeFromKey(RepositoryConnection connection, ECTypedKey key, IExtendedParameters extendedParameters)
        {
            string fullSchemaName = ParentECPlugin.SchemaModule.GetSchemaFullNames(connection).First();
            IECSchema schema = ParentECPlugin.SchemaModule.GetSchema(connection, fullSchemaName);

            switch (key.ClassName)
            {
                case "Folder":
                    IECClass classFolder = schema.GetClass(key.ClassName);
                    IECInstance instanceFolder = classFolder.CreateInstance();
                    instanceFolder.InstanceId = key.InstanceId;
                    NavNode navNode = new NavNode(instanceFolder);
                    return navNode;
                default:
                    throw new UserFriendlyException("It's impossible to list a document");
            }
        }
        /// <summary>
        /// GetRootNodes get the rootNode of the s3mx blob container
        /// </summary>
        /// <param name="connection"></param>
        /// <param name="extendedParameters"></param>
        /// <returns></returns>
        public override PageableContainer<NavNode> GetRootNodes(RepositoryConnection connection, IExtendedParameters extendedParameters)
        {
            // get the schema to use appropriate classes
            string fullSchemaName = ParentECPlugin.SchemaModule.GetSchemaFullNames(connection).First();
            IECSchema schema = ParentECPlugin.SchemaModule.GetSchema(connection, fullSchemaName);

            // get the list of blob under the current node (instanceId a the path to the desired directory)
            S3MXAzureBlobApi s3mxBlobApi = new S3MXAzureBlobApi(ConfigurationRoot.GetAppSetting("S3MXECPConnectionString"));
            IEnumerable<IListBlobItem> items = s3mxBlobApi.ListDirectories("s3mx");

            if (items == null)
                throw new UserFriendlyException("The Azure blob container does not exist.");

            // creating the List<NavNode> to return to WSG
            List<NavNode> navNodes = new List<NavNode>();
            foreach (IListBlobItem item in items)
            {
                if (item.GetType() == typeof(CloudBlockBlob))
                {
                    throw new UserFriendlyException("The root layer does not have any blob");
                }
                else if (item.GetType() == typeof(CloudPageBlob))
                {
                    throw new UserFriendlyException("The root layer does not have any page blob");
                }
                else if (item.GetType() == typeof(CloudBlobDirectory))
                {
                    // create a Folder instance
                    CloudBlobDirectory directory = (CloudBlobDirectory)item;
                    IECClass classFolder = schema.GetClass("Folder");
                    IECInstance instanceFolder = classFolder.CreateInstance();
                    instanceFolder.InstanceId = directory.Prefix;
                    NavNode navNodeFolder = new NavNode(instanceFolder);
                    navNodeFolder.Description = extractLastSegmentOfUri(directory.Prefix, '/');
                    navNodes.Add(navNodeFolder);
                }
            }

            PageableContainer<NavNode> pageableContainer = new PageableContainer<NavNode>(connection, navNodes);
            return pageableContainer;
        }
        /// <summary>
        /// GetChildren should be called directly after GetNodeFromKey returning all documents/folders under the desired node
        /// </summary>
        /// <param name="connection"></param>
        /// <param name="ecNavNode"></param>
        /// <param name="pageSize"></param>
        /// <param name="extendedParameters"></param>
        /// <returns></returns>
        public override PageableContainer<NavNode> GetChildren(RepositoryConnection connection, NavNode ecNavNode, int pageSize, IExtendedParameters extendedParameters)
        {
            // get the schema to use appropriate classes
            string fullSchemaName = ParentECPlugin.SchemaModule.GetSchemaFullNames(connection).First();
            IECSchema schema = ParentECPlugin.SchemaModule.GetSchema(connection, fullSchemaName);

            // get the list of blob under the current node (instanceId a the path to the desired directory)
            S3MXAzureBlobApi s3mxBlobApi = new S3MXAzureBlobApi(ConfigurationRoot.GetAppSetting("S3MXECPConnectionString"));
            IEnumerable<IListBlobItem> items = s3mxBlobApi.ListDirectories("s3mx", ecNavNode.IECInstance.InstanceId);

            if (items == null)
                throw new UserFriendlyException("The Azure blob container does not exist.");

            // creating the List<NavNode> to return to WSG
            List<NavNode> navNodes = new List<NavNode>();
            foreach (IListBlobItem item in items)
            {
                if (item.GetType() == typeof(CloudBlockBlob))
                {
                    // create a Document instance
                    CloudBlockBlob blob = (CloudBlockBlob)item;
                    IECClass classDocument = schema.GetClass("Document");
                    IECInstance instanceDocument = classDocument.CreateInstance();
                    instanceDocument.InstanceId = blob.Name;
                    NavNode navNodeFolder = new NavNode(instanceDocument);
                    navNodeFolder.Description = extractLastSegmentOfUri(blob.Name, '/');
                    navNodes.Add(navNodeFolder);
                }
                else if (item.GetType() == typeof(CloudPageBlob))
                {
                    throw new NotImplementedException("CloudPageBlob not supported");
                }
                else if (item.GetType() == typeof(CloudBlobDirectory))
                {
                    // create a Folder instance
                    CloudBlobDirectory directory = (CloudBlobDirectory)item;
                    IECClass classFolder = schema.GetClass("Folder");
                    IECInstance instanceFolder = classFolder.CreateInstance();
                    instanceFolder.InstanceId = directory.Prefix;
                    NavNode navNodeFolder = new NavNode(instanceFolder);
                    navNodeFolder.Description = extractLastSegmentOfUri(directory.Prefix, '/');
                    navNodes.Add(navNodeFolder);
                }
            }

            PageableContainer<NavNode> pageableContainer = new PageableContainer<NavNode>(connection, navNodes);
            return pageableContainer;
        }

        private static string extractLastSegmentOfUri(string uri, char separator)
        {
            string[] fragments = uri.Split(separator);
            if (fragments.Last() != "" && fragments.Length > 1)
                return fragments.Last();
            
            return fragments[fragments.Length - 2];
        }
    }
}
