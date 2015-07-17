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

        public USGSThumbnailRetrievalController(IECInstance instance
                                                //FileResourceManager manager,
                                                //RetrieveBackingFileOperation operation
                                               )
        {
            m_instance = instance;
        }


        //public override void DoRetrieveFile(bool transferFile)
        public void processThumbnailRetrieval()
        {
            string url = "https://www.sciencebase.gov/catalog/item/" + m_instance.InstanceId + "?format=json";

            using (HttpClient client = new HttpClient())
            {
                using (HttpResponseMessage response = client.GetAsync(url).Result)
                {
                    using (HttpContent content = response.Content)
                    {
                        string responseString = content.ReadAsStringAsync().Result;
                        

                        var json = JsonConvert.DeserializeObject(responseString) as JObject;
                        var linkArray = json["webLinks"] as JArray;

                        //We'll take the first link of type "browseImage"
                        var link = linkArray.First(l => l["type"].Value<string>() == "browseImage");
                        string thumbnailUri = link["uri"].Value<string>();

                        Stream thumbnailStream = DownloadThumbnail(thumbnailUri);

                        //TODO : Decide what to do with expectedSize (Currently 0)
                        StreamBackedDescriptor streamDescriptor = new StreamBackedDescriptor(thumbnailStream, m_instance.InstanceId, 0, DateTime.Now);
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
                }
            }
        }

        private Stream DownloadThumbnail(string thumbnailUri)
        {
            if(thumbnailUri.StartsWith("ftp"))
            {
                //FtpWebRequest request = FtpWebRequest.Create(thumbnailUri) as FtpWebRequest;
                using(WebClient ftpClient = new WebClient())
                {
                    
                    //string tempPath = Path.GetTempPath();
                    //string tempFilePath = Path.Combine(tempPath, Guid.NewGuid().ToString());
                    //using (Stream fstream = File.Create(tempFilePath))
                    //{
                    //    ftpClient.OpenRead(thumbnailUri).CopyTo(fstream);
                    //}
                    //return File.Open(tempFilePath, FileMode.Open);
                    return ftpClient.OpenRead(thumbnailUri);
                }
                
            }
            if(thumbnailUri.StartsWith("http"))
            {
                using (HttpClient client = new HttpClient())
                {
                    using (HttpResponseMessage response = client.GetAsync(thumbnailUri).Result)
                    {
                        using (HttpContent content = response.Content)
                        {
                            return content.ReadAsStreamAsync().Result;
                        }
                    }
                }
            }
            else
            {
                throw new NotImplementedException("The download of the thumbnail located at " + thumbnailUri);
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
