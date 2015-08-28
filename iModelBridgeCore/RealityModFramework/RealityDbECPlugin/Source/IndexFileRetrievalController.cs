using Bentley.EC.Persistence;
using Bentley.EC.Persistence.FileSystemResource;
using Bentley.EC.Persistence.Operations;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using Bentley.ECPluginExamples;
using Bentley.Exceptions;
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

namespace IndexECPlugin.Source
{
    internal class IndexFileRetrievalController : FileResourceRetrievalController
    {
        string m_packagesLocation;

        public IndexFileRetrievalController
            (
            IECInstance instance,
            FileResourceManager manager,
            RetrieveBackingFileOperation operation,
            string packagesLocation
            )
            : base(instance, manager, operation)
        {
            m_packagesLocation = packagesLocation;
        }

        public override void DoRetrieveFile(bool transferFile)
        {
            ////***We need to retrieve the thumbnail location. We call our API to ask for the location of the bentleyFile of known ID***
            //string URL = ResourceManager.Connection.RepositoryIdentifier.Location + "GetJSonOfAllFilesInQuery";
            //Object content = new
            //{
            //    polygonPoints = "",
            //    whereClause = String.Format("ID={0}", Instance.InstanceId),
            //    itemsByPage = 2,
            //    pageNumber = 1
            //};

            //var serializedContent = JsonConvert.SerializeObject(content);
            //HttpContent httpContent = new StringContent(serializedContent, Encoding.UTF8, "application/json");

            //string location;

            //using (var client = new HttpClient())
            //{
            //    ServicePointManager.ServerCertificateValidationCallback += (sender, cert, chain, sslPolicyErrors) => true; //This is necessary if the ssl certificate of the API is not valid...
            //    var response = client.PostAsync(URL, httpContent).Result;
            //    ServicePointManager.ServerCertificateValidationCallback -= (sender, cert, chain, sslPolicyErrors) => true;

            //    response.EnsureSuccessStatusCode();

            //    //if (response.IsSuccessStatusCode)
            //    //{
            //    string resultContent = response.Content.ReadAsStringAsync().Result;
            //    JsonResults summary = JsonConvert.DeserializeObject<JsonResults>(resultContent);
            //    location = summary.Results.First().ThumbnailLocation;
            //    //}
            //    //else
            //    //{
            //    //    throw new UserFriendlyException(response.Headers.);
            //    //}
            //}

            //We need to ask the database about the location of the file. 

            IECClass instanceClass = Instance.ClassDefinition;

            IECInstance fileHolderAttribute = instanceClass.GetCustomAttributes("FileHolder");

            if (fileHolderAttribute == null)
            {
                throw new UserFriendlyException(String.Format("There is no file associated to the {0} class", instanceClass.Name));
            }

            switch (fileHolderAttribute["Type"].StringValue)
            {

                case "PreparedPackage":
                    preparedPackageProcedure(Instance.InstanceId);
                    break;
                //case "IndexThumbnail":
                //    break;
                case "USGSThumbnail":
                    //USGSThumbnailProcedure(Instance.InstanceId);
                    //break;
                default:
                    throw new ProgrammerException("This type of file holder attribute is not implemented");
            }
        }

        //private void USGSThumbnailProcedure(string USGSInstanceId)
        //{
        //    string url = "https://www.sciencebase.gov/catalog/item/" + USGSInstanceId + "?format=json";

        //    using (HttpClient client = new HttpClient())
        //    {
        //        using (HttpResponseMessage response = client.GetAsync(url).Result)
        //        {
        //            using (HttpContent content = response.Content)
        //            {
        //                string responseString = content.ReadAsStringAsync().Result;
                        

        //                var json = JsonConvert.DeserializeObject(responseString) as JObject;
        //                var linkArray = json["webLinks"] as JArray;

        //                //We'll take the first link of type "browseImage"
        //                var link = linkArray.First(l => l["type"].Value<string>() == "browseImage");
        //                string thumbnailUri = link["uri"].Value<string>();

        //                DownloadThumbnail

        //                //foreach (var link in linkArray)
        //                //{
        //                //    if(link["type"].Value<string>() == "browseImage")
        //                //    {
        //                //        thumbnailUri = link["uri"].Value<string>();
        //                //        break;
        //                //    }
                            
        //                //}



        //            }
        //        }
        //    }
        //}

        private void preparedPackageProcedure(string name)
        {
            //TODO : Change this string with something placed in a config file
            string location = Path.Combine(m_packagesLocation, name);

            FileInfo fileInfo = new FileInfo(location);
            if (!fileInfo.Exists)
            {
                throw new UserFriendlyException("There is no such file associated to this instance");
            }

            DateTime WriteTime = fileInfo.LastWriteTimeUtc;



            var fileBackedDescriptor = new FileBackedDescriptor(ResourceManager.Connection.RepositoryIdentifier.ECPluginID, fileInfo.FullName, WriteTime);

            FileBackedDescriptorAccessor.SetIn(Instance, fileBackedDescriptor);
            ResourceManager.SetAsSynchronizedWithRepository(Instance, ResourceManager.Connection.RepositoryIdentifier.ECPluginID, fileBackedDescriptor.RelativePath, WriteTime, false);
        }

        public override bool InstanceRequiresLockForLocalFileModifications
        {
            get
            {
                return false;
            }
        }

        public override bool ObtainingLockDuringRetrieval
        {
            get
            {
                return false;
            }
        }
    }
}
