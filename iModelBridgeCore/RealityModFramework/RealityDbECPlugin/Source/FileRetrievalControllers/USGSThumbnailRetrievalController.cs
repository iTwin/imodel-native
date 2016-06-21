/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/FileRetrievalControllers/USGSThumbnailRetrievalController.cs $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/

using Bentley.EC.Persistence;
using Bentley.EC.Persistence.FileSystemResource;
using Bentley.EC.Persistence.Operations;
using Bentley.ECObjects.Instance;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;

namespace IndexECPlugin.Source.FileRetrievalControllers
    {

    //TODO : This class is to remake as a static class (if processThumbnailRetrieval works)
    internal class USGSThumbnailRetrievalController// : FileResourceRetrievalController
        {

        IECInstance m_instance;

        public USGSThumbnailRetrievalController (IECInstance instance
            //FileResourceManager manager,
            //RetrieveBackingFileOperation operation
                                               )
            {
            m_instance = instance;
            }


        //public override void DoRetrieveFile(bool transferFile)
        public void processThumbnailRetrieval ()
            {
            string url = "https://www.sciencebase.gov/catalog/item/" + m_instance.InstanceId + "?format=json";

            using ( HttpClient client = new HttpClient() )
                {
                using ( HttpResponseMessage response = client.GetAsync(url).Result )
                    {
                    using ( HttpContent content = response.Content )
                        {
                        string responseString = content.ReadAsStringAsync().Result;

                        try
                            {
                            var json = JsonConvert.DeserializeObject(responseString) as JObject;
                            var linkArray = json["webLinks"] as JArray;

                            //We'll take the first link of type "browseImage"
                            var link = linkArray.First(l => l["type"].Value<string>() == "browseImage");
                            string thumbnailUri = link["uri"].Value<string>();

                            //long contentLength;

                            MemoryStream thumbnailStream = DownloadThumbnail(thumbnailUri);

                            //This is a test!!!!
                            //Stream thumbnailStream = File.OpenRead(@"C:\RealityData\PackagesNewDb\test.txt");
                            //StreamBackedDescriptor streamDescriptor = new StreamBackedDescriptor(thumbnailStream, m_instance.InstanceId, 3, DateTime.UtcNow);

                            //TODO : Decide what to do with expectedSize (Currently 0)
                            //throw new Exception(String.Format("Length of the response : {0}", contentLength));
                            StreamBackedDescriptor streamDescriptor = new StreamBackedDescriptor(thumbnailStream, m_instance.InstanceId, thumbnailStream.Length, DateTime.UtcNow);
                            StreamBackedDescriptorAccessor.SetIn(m_instance, streamDescriptor);
                            //foreach (var link in linkArray)
                            //{
                            //    if(link["type"].Value<string>() == "browseImage")
                            //    {
                            //        thumbnailUri = link["uri"].Value<string>();
                            //        break;
                            //    }

                            //}
                            }
                        catch ( Exception e )
                            {
                            Log.Logger.error(String.Format("Instance {0} USGS thumbnail retrieval aborted. Received message : {1}"), m_instance.InstanceId, e.Message);
                            throw new OperationFailedException("There is a problem with the retrieval of this USGS thumbnail");
                            }


                        }
                    }
                }
            }

        //TODO : Set a memory limit on the MemoryStream
        private MemoryStream DownloadThumbnail (string thumbnailUri)
            {
            if ( thumbnailUri.StartsWith("ftp") )
                {
                //FtpWebRequest request = FtpWebRequest.Create(thumbnailUri) as FtpWebRequest;
                //request.Method = WebRequestMethods.Ftp.GetFileSize;

                //FtpWebResponse response = (FtpWebResponse)request.GetResponse();
                //contentLength = response.ContentLength;

                //request = FtpWebRequest.Create(thumbnailUri) as FtpWebRequest;
                //request.Method = WebRequestMethods.Ftp.DownloadFile;

                //response = (FtpWebResponse)request.GetResponse();

                //return response.GetResponseStream();
                using ( WebClient ftpClient = new WebClient() )
                    {

                    //string tempPath = Path.GetTempPath();
                    //string tempFilePath = Path.Combine(tempPath, Guid.NewGuid().ToString());
                    //using (Stream fstream = File.Create(tempFilePath))
                    //{
                    //    ftpClient.OpenRead(thumbnailUri).CopyTo(fstream);
                    //}
                    //return File.Open(tempFilePath, FileMode.Open);

                    using ( Stream image = ftpClient.OpenRead(thumbnailUri) )
                        {
                        MemoryStream imageInMemory = new MemoryStream();

                        image.CopyTo(imageInMemory);

                        return imageInMemory;
                        }
                    }

                }
            if ( thumbnailUri.StartsWith("http") )
                {
                using ( HttpClient client = new HttpClient() )
                    {
                    using ( HttpResponseMessage response = client.GetAsync(thumbnailUri).Result )
                        {
                        using ( HttpContent content = response.Content )
                            {

                            //contentLength = (content.Headers.ContentLength.HasValue ? content.Headers.ContentLength.Value : 0);

                            using ( Stream image = content.ReadAsStreamAsync().Result )
                                {
                                MemoryStream imageInMemory = new MemoryStream();
                                image.CopyTo(imageInMemory);
                                return imageInMemory;
                                }
                            }
                        }
                    }
                }
            else
                {
                throw new NotImplementedException("The download of the thumbnail located at " + thumbnailUri + " is not implemented yet.");
                }
            }

        //public override bool InstanceRequiresLockForLocalFileModifications
        //{
        //    get
        //    {
        //        return false;
        //    }
        //}

        //public override bool ObtainingLockDuringRetrieval
        //{
        //    get
        //    {
        //        return false;
        //    }
        //}
        }
    }
