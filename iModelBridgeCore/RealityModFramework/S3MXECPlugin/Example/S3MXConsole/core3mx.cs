using Microsoft.WindowsAzure.Storage;
using Microsoft.WindowsAzure.Storage.Blob;
using System;
using System.Collections.Generic;
using System.IO;
using System.Threading.Tasks;
using System.Net;
using System.Linq;
using Newtonsoft.Json;
using System.Configuration;
using System.Net.Security;
using BentleyIT.STS.RelyingPartyCommon;
using System.IdentityModel.Tokens;
using System.Threading;

namespace S3MXConsole
    {
    class core3mx
        {
        private volatile int uploadCounter = 0;
        private readonly CloudStorageAccount _account;
        private readonly CloudBlobClient _blobClient;
        private readonly string _wsgBaseUrl;
        private static DateTime _tokenLife;
        private static TimeSpan _limitSpan = new TimeSpan (0, 30, 0);
        private static string _password;
        private static string _authToken;

        public core3mx ()
            {
            _wsgBaseUrl = ConfigurationManager.AppSettings["WsgBaseUrl"];
            _account = CloudStorageAccount.Parse(ConfigurationManager.AppSettings["StorageConnectionString"]);
            _blobClient = _account.CreateCloudBlobClient();
            _tokenLife = DateTime.Now;
            _password = null;

            // get the ims token from using BentleyIT.STS library
            string base64Token = extractTokenAction();
            if ( base64Token == null )
                return;

            Console.ForegroundColor = ConsoleColor.Green;
            _authToken = ConfigurationManager.AppSettings["WsgAuthScheme"] + " " + base64Token;
            }

        public CloudBlobContainer CreateContainer (string containerName)
            {
            CloudBlobContainer container = _blobClient.GetContainerReference(containerName);
            container.CreateIfNotExists();
            return container;
            }

        public int UploadDirectoryWithWsgMultiThreads (List<Tuple<string, string>> listSrcDest)
            {
            ServicePointManager.ServerCertificateValidationCallback = new RemoteCertificateValidationCallback(delegate { return true; });

            object locker = new object();
            Console.ForegroundColor = ConsoleColor.Green;
            Parallel.ForEach(listSrcDest, new ParallelOptions { MaxDegreeOfParallelism = 10 }, (item) =>
            {
                uploadCounter += 1;

                if ( (DateTime.Now - _tokenLife) > _limitSpan )
                    {
                    lock(locker){
                        Console.ForegroundColor = ConsoleColor.Cyan;
                        Console.WriteLine("Token swapping");
                        _tokenLife = DateTime.Now;
                        _authToken = ConfigurationManager.AppSettings["WsgAuthScheme"] + " " + extractTokenAction(_password);
                        Console.ForegroundColor = ConsoleColor.Green;
                        }
                    }

                string sourcePath = item.Item1;
                byte[] fileContent = null;
                if ( File.Exists(sourcePath) )
                    {
                    fileContent = File.ReadAllBytes(sourcePath);
                    }
                HttpWebRequest request = (HttpWebRequest) WebRequest.Create(_wsgBaseUrl + "S3MX/Document/" + item.Item2.Replace("/", "~2F") + "/$file");
                request.Method = "PUT";
                request.ReadWriteTimeout = 100000;
                request.Timeout = 100000;
                request.Headers.Add("Authorization: " + _authToken);
                request.Headers.Add("Content-Disposition: attachment; filename=\"" + item.Item2.Split('/').Last() + "\"");
                request.Headers.Add("Content-Range: bytes */" + fileContent.Length);
                request.ContentLength = 0;
                try
                    {
                    // the first call will always fail because WSG always return an Http Code 308 which is interpreted as an error...
                    HttpWebResponse myHttpWebResponse = (HttpWebResponse) request.GetResponse();
                    myHttpWebResponse.Close();
                    }
                catch ( WebException ex )
                    {
                    try
                        {
                        if ( ex.Response == null )
                            {
                            Console.ForegroundColor = ConsoleColor.Red;
                            Console.WriteLine(ex.Message);
                            Console.ForegroundColor = ConsoleColor.Green;
                            throw (new WebException("Request timed out"));
                            }

                        HttpWebRequest request2 = (HttpWebRequest) WebRequest.Create(_wsgBaseUrl + "S3MX/Document/" + item.Item2.Replace("/", "~2F") + "/$file");
                        request2.ReadWriteTimeout = 100000;
                        request2.Timeout = 100000;
                        request2.Method = "PUT";
                        request2.Headers.Add("Authorization: " + _authToken);
                        request2.Headers.Add("If-Match: " + ex.Response.Headers.Get("ETag"));
                        request2.Headers.Add("Content-Range: bytes 0-" + (fileContent.Length - 1) + "/" + fileContent.Length);
                        request2.ContentType = "text/plain";
                        request2.ContentLength = fileContent.Length;

                        Stream newStream = request2.GetRequestStream();
                        newStream.Write(fileContent, 0, fileContent.Length);
                        newStream.Close();
                        HttpWebResponse myHttpWebResponse2 = (HttpWebResponse) request2.GetResponse();
                        myHttpWebResponse2.Close();

                        if ( myHttpWebResponse2.StatusCode == HttpStatusCode.OK )
                            {
                            Console.WriteLine(item.Item2 + " is uploaded successfully" + " (" + uploadCounter + "/" + listSrcDest.Count + ")");
                            }
                        else
                            {
                            Console.ForegroundColor = ConsoleColor.Red;
                            Console.WriteLine("Error while uploading: " + item.Item2);
                            Console.ForegroundColor = ConsoleColor.Green;
                            }
                        }
                    catch ( WebException ex2 )
                        {
                        Console.ForegroundColor = ConsoleColor.Red;
                        Console.WriteLine(ex2.Message);
                        Console.ForegroundColor = ConsoleColor.Green;
                        }
                    }
            });
            Console.ForegroundColor = ConsoleColor.Gray;

            return 0;
            }

        public void ListDir (string instanceId = null)
            {
            string url = "";
            if ( instanceId == null )
                url = _wsgBaseUrl + "Navigation/NavNode/";
            else
                url = _wsgBaseUrl + "Navigation/NavNode/" + instanceId + "/NavNode/";

            HttpWebRequest request = (HttpWebRequest) WebRequest.Create(url);

            // this is only to bypass the invalid certificate on azure sandbox
            request.ServerCertificateValidationCallback += (sender, certificate, chain, sslPolicyErrors) =>
                {
                    return true;
                };
            request.Headers.Add("Authorization: " + _authToken);
            try
                {
                HttpWebResponse httpWebResponse = (HttpWebResponse) request.GetResponse();
                using ( var streamReader = new StreamReader(httpWebResponse.GetResponseStream()) )
                    {
                    var responseText = streamReader.ReadToEnd();
                    dynamic jsonResponse = JsonConvert.DeserializeObject(responseText);
                    Console.ForegroundColor = ConsoleColor.Gray;
                    Console.WriteLine(jsonResponse);
                    Console.ForegroundColor = ConsoleColor.Gray;
                    httpWebResponse.Close();
                    httpWebResponse.Dispose();
                    }
                httpWebResponse.Close();
                }
            catch ( WebException ex )
                {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine(ex.Message);
                Console.ForegroundColor = ConsoleColor.Gray;
                }
            }

        // DEPRECATED because it's not using WSG nor the ECPlugin
        //public bool UploadDirectory (string sourceDirectory, string containerName, string prefixAzureFolderName)
        //{
        //    return ExecuteWithExceptionHandlingAndReturnValue(
        //        () =>
        //        {
        //            CloudBlobContainer container = _blobClient.GetContainerReference(containerName);
        //            container.CreateIfNotExists();

        //            var folder = new DirectoryInfo(sourceDirectory);
        //            var files = folder.GetFiles();
        //            foreach (var fileInfo in files)
        //            {
        //                string blobName = fileInfo.Name;
        //                if (!string.IsNullOrEmpty(prefixAzureFolderName))
        //                {
        //                    blobName = prefixAzureFolderName + "/" + blobName;
        //                }

        //                using (var fileStream = File.OpenRead(fileInfo.FullName))
        //                {
        //                    CloudBlockBlob blockBlob = container.GetBlockBlobReference(blobName);
        //                    blockBlob.UploadFromStream(fileStream);
        //                    Console.WriteLine(blobName);
        //                }
        //            }
        //            var subFolders = folder.GetDirectories();
        //            foreach (var directoryInfo in subFolders)
        //            {
        //                var prefix = directoryInfo.Name;
        //                if (!string.IsNullOrEmpty(prefixAzureFolderName))
        //                {
        //                    prefix = prefixAzureFolderName + "/" + prefix;
        //                }
        //                UploadDirectory(directoryInfo.FullName, containerName, prefix);
        //            }
        //        });
        //}

        // DEPRECATED because it's not using WSG nor the ECPlugin
        //public bool DownloadDirectory(string targetDirectory, string downloadDirectory, string containerName)
        //{
        //    return ExecuteWithExceptionHandlingAndReturnValue(
        //        () =>
        //        {
        //            CloudBlobContainer container = _blobClient.GetContainerReference(containerName);
        //            foreach (IListBlobItem item in container.ListBlobs(null, true))
        //            {
        //                if (item.GetType() == typeof(CloudBlockBlob))
        //                {
        //                    CloudBlockBlob blob = (CloudBlockBlob)item;

        //                    Directory.CreateDirectory(downloadDirectory + blob.Parent.Prefix.Replace("/", "\\"));
        //                    using (var fileStream = File.OpenWrite(downloadDirectory + blob.Name.Replace("/", "\\")))
        //                    {
        //                        Console.WriteLine(blob.Name);
        //                        blob.DownloadToStream(fileStream);
        //                    }
        //                }
        //            }
        //        });
        //}

        public int GetListOfFileInfos (string sourceDirectory, string prefix, ref List<Tuple<string, string>> listSrcDest)
            {
            try
                {
                DirectoryInfo folder = new DirectoryInfo(sourceDirectory);
                FileInfo[] files = folder.GetFiles();
                foreach ( FileInfo file in files )
                    {
                    string blobName = file.Name;
                    if ( !string.IsNullOrEmpty(prefix) )
                        {
                        blobName = prefix + "/" + blobName;
                        }
                    Tuple<string, string> srcDest = new Tuple<string, string>(file.FullName, blobName);
                    listSrcDest.Add(srcDest);
                    }
                DirectoryInfo[] subFolders = folder.GetDirectories();
                foreach ( DirectoryInfo directoryInfo in subFolders )
                    {
                    var tempPrefix = directoryInfo.Name;
                    if ( !string.IsNullOrEmpty(prefix) )
                        {
                        tempPrefix = prefix + "/" + tempPrefix;
                        }
                    GetListOfFileInfos(directoryInfo.FullName, tempPrefix, ref listSrcDest);
                    }
                }
            catch ( DirectoryNotFoundException ex )
                {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine(ex.Message);
                Console.ForegroundColor = ConsoleColor.Gray;
                return 1;
                }

            return 0;
            }

        public IEnumerable<IListBlobItem> ListDirectories (string containerName, string prefix = null, bool useFlatBlobListing = false)
            {
            CloudBlobContainer container = _blobClient.GetContainerReference(containerName);
            if ( !container.Exists() )
                {
                return new List<IListBlobItem>();
                }

            return container.ListBlobs(prefix, useFlatBlobListing);
            }

        //List all blobs, pageBlob and directories under specified prefix and container
        //public void listDeprecated ()
        //    {
        //    IEnumerable<IListBlobItem> directories = ListDirectories("s3mx", "", true);
        //    foreach ( IListBlobItem item in directories )
        //        {
        //        if ( item.GetType() == typeof(CloudBlockBlob) )
        //            {
        //            CloudBlockBlob blob = (CloudBlockBlob) item;
        //            Console.WriteLine("Block blob of length {0}: {1}", blob.Properties.Length, blob.Uri);
        //            }
        //        else if ( item.GetType() == typeof(CloudPageBlob) )
        //            {
        //            CloudPageBlob pageBlob = (CloudPageBlob) item;
        //            Console.WriteLine("Page blob of length {0}: {1}", pageBlob.Properties.Length, pageBlob.Uri);
        //            }
        //        else if ( item.GetType() == typeof(CloudBlobDirectory) )
        //            {
        //            CloudBlobDirectory directory = (CloudBlobDirectory) item;
        //            Console.WriteLine("Directory: {0}", directory.Uri);
        //            }
        //        }
        //    }

        //==============================================================
        //Download from azure to computer with multiple threads
        //public void downloadDeprecated ()
        //    {
        //    Parallel.ForEach(ListDirectories("s3mx", "Marseille_3MX", true), (item) =>
        //    {
        //        if ( item.GetType() == typeof(CloudBlockBlob) )
        //            {
        //            CloudBlockBlob blob = (CloudBlockBlob) item;
        //            Directory.CreateDirectory(@"C:\TESTDOWNLOAD\" + blob.Parent.Prefix.Replace("/", "\\"));
        //            using ( var fileStream = File.OpenWrite(@"C:\TESTDOWNLOAD\" + blob.Name.Replace("/", "\\")) )
        //                {
        //                Console.WriteLine(blob.Name);
        //                blob.DownloadToStream(fileStream);
        //                }
        //            Console.WriteLine("Processing {0} on thread {1}", blob.Name, Thread.CurrentThread.ManagedThreadId);
        //            }
        //    });
        //    }

        //==============================================================
        //Upload from computer to azure with multiple threads
        //public void uploadDeprecated ()
        //    {
        //    getListOfFileInfos(@"C:\s3mx\Production_Graz_3MX", "Production_Graz_3MX", ref listSrcDest);
        //    CloudBlobContainer container = CreateContainer("s3mx");
        //    Parallel.ForEach(listSrcDest, (srcDest) =>
        //    {
        //        using ( var fileStream = File.OpenRead(srcDest.Item1) )
        //            {
        //            CloudBlockBlob blockBlob = container.GetBlockBlobReference(srcDest.Item2);
        //            blockBlob.UploadFromStream(fileStream);
        //            Console.WriteLine(srcDest.Item2);
        //            }
        //    });
        //    }

        //private void ExecuteWithExceptionHandling(Action action)
        //{
        //    try
        //    {
        //        action();
        //    }
        //    catch (StorageException ex)
        //    {
        //        Console.WriteLine(ex.Message);
        //    }
        //}

        //private bool ExecuteWithExceptionHandlingAndReturnValue(Action action)
        //{
        //    try
        //    {
        //        action();
        //        return true;
        //    }
        //    catch (StorageException ex)
        //    {
        //        Console.WriteLine(ex.Message);
        //        return false;
        //    }
        //}

        private static string extractTokenAction (string password = null)
            {
            StsHelper sts = new StsHelper("https://ims.bentley.com/ActiveSTSService.svc/UserName", new Uri("https://waz-search.bentley.com/").ToString().Replace("/api/", "/"));
            string clientKey = ConfigurationManager.AppSettings["ImsEmail"] ?? null;

            Console.ForegroundColor = ConsoleColor.Cyan;
            if ( clientKey == null)
                {
                Console.Write("Enter your email: ");
                clientKey = Console.ReadLine();
                }

            if ( password == null )
                {
                Console.Write("Enter your password: ");
                ConsoleKeyInfo key;

                do
                    {
                    key = Console.ReadKey(true);
                    if ( key.Key != ConsoleKey.Backspace && key.Key != ConsoleKey.Enter )
                        {
                        password += key.KeyChar;
                        Console.Write("*");
                        }
                    else
                        {
                        if ( key.Key == ConsoleKey.Backspace && password.Length > 0 )
                            {
                            password = password.Substring(0, (password.Length - 1));
                            Console.Write("\b \b");
                            }
                        }
                    }
                while ( key.Key != ConsoleKey.Enter );
                _password = password;
                Console.WriteLine();
                }
            Console.ForegroundColor = ConsoleColor.Green;

            SecurityToken token = null;
            try
                {
                token = sts.GetSecurityToken(clientKey, password, new List<Microsoft.IdentityModel.Protocols.WSTrust.RequestClaim>(), new Dictionary<string, string>());
                }
            catch ( System.ServiceModel.Security.MessageSecurityException e )
                {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine(e.Message);
                Console.WriteLine("Wrong Email/Password combination.");
                Console.ForegroundColor = ConsoleColor.Green;
                return null;
                }

            string tokenString = StsHelper.SerializeToken(token);
            return Convert.ToBase64String(System.Text.Encoding.UTF8.GetBytes(tokenString));
            }
        }
    }
