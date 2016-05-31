using Bentley.EC.Persistence;
using Microsoft.WindowsAzure.Storage;
using Microsoft.WindowsAzure.Storage.Blob;
using System;
using System.Collections.Generic;
using System.IO;

namespace S3MX.Source
{
    class S3MXAzureBlobApi
    {
        private readonly CloudStorageAccount _account;
        private readonly CloudBlobClient _blobClient;

        public S3MXAzureBlobApi(string connectionString)
        {
            _account = CloudStorageAccount.Parse(connectionString);
            _blobClient = _account.CreateCloudBlobClient();
        }

        /// <summary>
        /// Download a single file from the azure blob storage
        /// </summary>
        /// <param name="containerName"></param>
        /// <param name="targetBlob"></param>
        /// <param name="mStream"></param>
        /// <returns></returns>
        public bool DownloadFile(string containerName, string targetBlob, ref MemoryStream mStream)
        {
            CloudBlobContainer container = _blobClient.GetContainerReference(containerName);
            if (!container.Exists())
                return false;

            CloudBlockBlob blockBlob = container.GetBlockBlobReference(targetBlob);
            blockBlob.DownloadToStream(mStream);

            return true;
        }

        /// <summary>
        /// Upload a single file from an http request to azure blob storage
        /// </summary>
        /// <param name="containerName"></param>
        /// <param name="uploadDestination"></param>
        /// <param name="streamBackedDescriptor"></param>
        /// <returns></returns>
        public bool UploadFile(string containerName, string uploadDestination, StreamBackedDescriptor streamBackedDescriptor)
        {
            CloudBlobContainer container = _blobClient.GetContainerReference(containerName);
            if (!container.Exists())
                return false;

            CloudBlockBlob blockBlob = container.GetBlockBlobReference(uploadDestination);
            blockBlob.UploadFromStream(streamBackedDescriptor.Stream);
            return true;
        }

        /// <summary>
        /// List all Directory/Blob/PageBlob under the desired container
        /// </summary>
        /// <param name="containerName"></param>
        /// <param name="prefix">Set a prefix value to follow your desired path</param>
        /// <param name="useFlatBlobListing">Set to true if you want a flat list of all elements</param>
        /// <returns></returns>
        public IEnumerable<IListBlobItem> ListDirectories(string containerName, string prefix = null, bool useFlatBlobListing = false)
        {
            CloudBlobContainer container = _blobClient.GetContainerReference(containerName);
            if (!container.Exists())
            {
                return null;
            }

            return container.ListBlobs(prefix, useFlatBlobListing);
        }

        /// <summary>
        /// Upload a whole directory to azure blob storage
        /// This is not supported with ECFramework
        /// </summary>
        /// <param name="sourceDirectory"></param>
        /// <param name="containerName"></param>
        /// <param name="prefix"></param>
        /// <returns></returns>
        public bool UploadDirectory(string sourceDirectory, string containerName, string prefix)
        {
            return ExecuteWithExceptionHandlingAndReturnValue(
                () =>
                {
                    CloudBlobContainer container = _blobClient.GetContainerReference(containerName);
                    container.CreateIfNotExists();

                    var folder = new DirectoryInfo(sourceDirectory);
                    var files = folder.GetFiles();
                    foreach (var fileInfo in files)
                    {
                        string blobName = fileInfo.Name;
                        if (!string.IsNullOrEmpty(prefix))
                        {
                            blobName = prefix + "/" + blobName;
                        }

                        using (var fileStream = File.OpenRead(fileInfo.FullName))
                        {
                            CloudBlockBlob blockBlob = container.GetBlockBlobReference(blobName);
                            blockBlob.UploadFromStream(fileStream);
                        }
                    }
                    var subFolders = folder.GetDirectories();
                    foreach (var directoryInfo in subFolders)
                    {
                        var prefixTemp = directoryInfo.Name;
                        if (!string.IsNullOrEmpty(prefix))
                        {
                            prefixTemp = prefix + "/" + prefixTemp;
                        }
                        UploadDirectory(directoryInfo.FullName, containerName, prefixTemp);
                    }
                });
        }

        /// <summary>
        /// Download a whole directory stored in azure blob storage
        /// This is not supported with ECFramework
        /// </summary>
        /// <param name="targetDirectory"></param>
        /// <param name="downloadDirectory"></param>
        /// <param name="containerName"></param>
        /// <returns></returns>
        public bool DownloadDirectory(string targetDirectory, string downloadDirectory, string containerName)
        {
            return ExecuteWithExceptionHandlingAndReturnValue(
                () =>
                {
                    CloudBlobContainer container = _blobClient.GetContainerReference(containerName);
                    foreach (IListBlobItem item in container.ListBlobs(null, true))
                    {
                        if (item.GetType() == typeof(CloudBlockBlob))
                        {
                            CloudBlockBlob blob = (CloudBlockBlob)item;

                            Directory.CreateDirectory(downloadDirectory + blob.Parent.Prefix.Replace("/", "\\"));
                            using (var fileStream = File.OpenWrite(downloadDirectory + blob.Name.Replace("/", "\\")))
                            {
                                blob.DownloadToStream(fileStream);
                            }
                        }
                    }
                });
        }

        private void ExecuteWithExceptionHandling(Action action)
        {
            try
            {
                action();
            }
            catch (StorageException ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        private bool ExecuteWithExceptionHandlingAndReturnValue(Action action)
        {
            try
            {
                action();
                return true;
            }
            catch (StorageException ex)
            {
                Console.WriteLine(ex.Message);
                return false;
            }
        }
    }
}
