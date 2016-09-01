using System;
using System.Collections.Generic;
using System.Configuration;
using System.IdentityModel.Tokens;
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Linq;
using BentleyIT.STS.RelyingPartyCommon;
using Microsoft.WindowsAzure.Storage;
using Microsoft.WindowsAzure.Storage.Table;
using Newtonsoft.Json.Linq;

namespace UsageDataExtractor
    {

    //This program's purpose is to extract usage data about the GeoCoordination Service (GCS) from the CONNECT logs Azure tables
    class Program
        {

        static bool saveXrdp = Convert.ToBoolean(ConfigurationManager.AppSettings["saveXrdp"]);
        static Dictionary<string, int> CountryCounter = new Dictionary<string, int>();

        static void Main (string[] args)
            {
            try
                {
                if ( args.Length == 3 )
                    {
                    string fetchResultsFileName = args[1];
                    if ( args[0].ToLower() == "fetch" )
                        {
                        string tableName = args[2];
                        FetchResults(fetchResultsFileName, tableName);
                        return;
                        }
                    if ( args[0].ToLower() == "extract" )
                        {
                        string extractionResultsFileNameBase = args[2];
                        Extract(fetchResultsFileName, extractionResultsFileNameBase);
                        }
                    }
                else
                    {
                    PrintUsage();
                    return;
                    }
                }
            catch (Exception ex)
                {
                Console.Out.WriteLine(ex.Message);
                }
            }

        private static void PrintUsage()
            {
            Console.Out.WriteLine("Usage 1: UsageDataExtractor.exe fetch [fetchFileName] [azureTableName]");
            Console.Out.WriteLine("The fetch mode searches the GCS logs located in the azure table and gets all xrdp GUIDs captured by the logs, along with the email of the requester.");
            Console.Out.WriteLine("[fetchFileName] is the name of the file that will be written on disk and contain the results of the fetch operation.");
            Console.Out.WriteLine("[azureTableName] is the name of the azure table containing the logs of the GCS. Typically covers only one month.");
            Console.Out.WriteLine(@"Example: UsageDataExtractor.exe fetch C:\Users\MyName\Documents\QueryResults\xrdpGUIDsAUGUST.txt BCLogETW201608");
            Console.Out.WriteLine("-------------------------------------------------------------------------");
            Console.Out.WriteLine("Usage 2: UsageDataExtractor.exe extract [fetchFileName] [resultsFileNameBase]");
            Console.Out.WriteLine("Downloads the actual xrdp files from the GCS and extracts useful information.");
            Console.Out.WriteLine("[fetchFileName] is the name of the file containing the xrdp GUIDs. This should be the file created by the fetch mode.");
            Console.Out.WriteLine("[resultsFileNameBase] is the base name of the result files that will be written on disk. An extension will be added by the program.");
            Console.Out.WriteLine(@"Example: UsageDataExtractor.exe extract C:\Users\MyName\Documents\QueryResults\xrdpGUIDsAUGUST.txt C:\Users\MyName\Documents\QueryResults\resultsAUGUST");
            }

        private static void FetchResults (string fileName, string tableName)
            {
            CloudStorageAccount storageAccount = CloudStorageAccount.Parse(ConfigurationManager.AppSettings["connectionString"]);
            CloudTableClient tableClient = storageAccount.CreateCloudTableClient();
            CloudTable logTable = tableClient.GetTableReference(tableName);
            //string condition = TableQuery.GenerateFilterCondition("ProductShortName", QueryComparisons.Equal, "ContextServices") + " and " +;
            string condition = "ProductShortName eq 'ContextServices' and HttpMethod eq 'GET' and Content gt 'INFO - Bentley.Mas: Request GET:''https://connect-contextservices.bentley.com/v2.3/Repositories/IndexECPlugin--server/RealityModeling/PreparedPackag' and Content lt 'INFO - Bentley.Mas: Request GET:''https://connect-contextservices.bentley.com/v2.3/Repositories/IndexECPlugin--server/RealityModeling/PreparedPackah'";
            TableQuery<DynamicTableEntity> query = new TableQuery<DynamicTableEntity>().Where(condition).Select(new string[] { "Content", "User" });

            IEnumerable<DynamicTableEntity> results = logTable.ExecuteQuery(query);
            if ( !File.Exists(fileName) )
                {
                using ( StreamWriter file = new System.IO.StreamWriter(fileName) )
                    {
                    foreach ( var result in results )
                        {
                        string content = result.Properties["Content"].StringValue;
                        string[] contentArray = content.Split('/');
                        string id = contentArray[contentArray.Length - 2];
                        string user = result.Properties["User"].StringValue;

                        file.WriteLine(id + " " + user);
                        file.Flush();
                        System.Console.WriteLine(id + " " + user);
                        }
                    }
                }
            else
                {
                throw new IOException("The output file already exists.");
                }
            }

        static void Extract (string inputName, string baseResultName)
            {
            string line;

            string serverURL = ConfigurationManager.AppSettings["serverURL"];
            string password = GetPassword();

            DateTime lastTokenTime = DateTime.Now;
            string base64Token = GetToken(password);

            TimeSpan limitSpan = new TimeSpan(0, 30, 0);

            using ( StreamReader file = new System.IO.StreamReader(inputName) )
                {
                using ( StreamWriter fileToWrite = new System.IO.StreamWriter(baseResultName + ".txt") )
                    {
                    fileToWrite.WriteLine("PackageId, Date, Email, Country, Polygon");
                    while ( (line = file.ReadLine()) != null )
                        {

                        if ( line.IndexOf("@bentley.com", StringComparison.OrdinalIgnoreCase) == -1 )
                            {
                            if ( (DateTime.Now - lastTokenTime) > limitSpan )
                                {
                                lastTokenTime = DateTime.Now;
                                base64Token = GetToken(password);
                                }
                            var split = line.Split(' ');
                            string packageId = line.Split(' ').First();
                            string date;
                            string poly = GetPolygon(packageId, serverURL, base64Token, baseResultName, out date);
                            string email = line.Split(' ')[1];

                            if ( String.IsNullOrWhiteSpace(email) )
                                {
                                email = "UNKNOWN";
                                }

                            string country = ExtractCountry(poly);
                            if ( CountryCounter.ContainsKey(country) )
                                {
                                CountryCounter[country]++;
                                }
                            else
                                {
                                CountryCounter[country] = 1;
                                }

                            fileToWrite.WriteLine(line.Split(' ').First() + ", " + date + ", " + email + ", " + country + ", " + poly);



                            }
                        }
                    }
                using ( StreamWriter fileToWrite = new System.IO.StreamWriter(baseResultName + ".summary.txt") )
                    {
                    foreach ( KeyValuePair<string, int> pair in CountryCounter.OrderByDescending(p => p.Value) )
                        {
                        fileToWrite.WriteLine(pair.Key + "," + pair.Value.ToString());
                        }
                    }
                }
            }

        private static string ExtractCountry (string poly)
            {

            string[] splitPoly = poly.Split(' ');
            string latlng = splitPoly[1] + "," + splitPoly[0];
            string googleAPIURL = @"https://maps.googleapis.com/maps/api/geocode/json?latlng=" + latlng;

            using ( HttpClient client = new HttpClient() )
                {
                using ( HttpResponseMessage response = client.GetAsync(googleAPIURL).Result )
                    {
                    if ( response.IsSuccessStatusCode )
                        {
                        JObject jsonResult = null;
                        try
                            {

                            jsonResult = JObject.Parse(response.Content.ReadAsStringAsync().Result) as JObject;

                            var result = jsonResult["results"].First(r => r["types"].Any(entry => entry.Value<string>() == "country"));
                            return result["address_components"].First(r => r["types"].Any(entry => entry.Value<string>() == "country"))["long_name"].Value<string>();
                            }
                        catch ( Exception )
                            {
                            if ( jsonResult != null && jsonResult["status"] != null && jsonResult["status"].Value<string>() == "ZERO_RESULTS" )
                                {
                                return "None";
                                }
                            else
                                throw;
                            }
                        }
                    else
                        {
                        throw new Exception("Google API did not return successfully");
                        }
                    }
                }
            }

        private static string GetPassword ()
            {
            string password = ConfigurationManager.AppSettings["password"];
            if(password != null)
                {
                return password;
                }

            Console.Write("Enter your password: ");
            ConsoleKeyInfo key;

            password = "";

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

            Console.WriteLine();

            return password;
            }

        private static string GetToken (string password)
            {
            SecurityToken token = null;

            string strServiceURL = ConfigurationManager.AppSettings["serviceUrl"];

            Uri BaseUri = new Uri(strServiceURL);

            string activeSTSUrl = ConfigurationManager.AppSettings["activeSTSUrl"];

            StsHelper sts = new StsHelper(activeSTSUrl, BaseUri.ToString().Replace("/api/", "/"));
            string clientKey = ConfigurationManager.AppSettings["email"];

            token = sts.GetSecurityToken(clientKey, password, new List<Microsoft.IdentityModel.Protocols.WSTrust.RequestClaim>(), new Dictionary<string, string>());

            string tokenString = StsHelper.SerializeToken(token);

            return System.Convert.ToBase64String(System.Text.Encoding.UTF8.GetBytes(tokenString));
            }

        private static string GetPolygon (string packageId, string serverURL, string base64Token, string baseResultName, out string date)
            {

            using ( HttpClient client = new HttpClient() )
                {
                client.DefaultRequestHeaders.Accept.Add(new MediaTypeWithQualityHeaderValue("application/json"));
                client.DefaultRequestHeaders.Authorization = new AuthenticationHeaderValue("Token", base64Token);

                string requestURL = serverURL + "/v2.3/Repositories/IndexECPlugin--Server/RealityModeling/PreparedPackage/" + packageId + "/$file";

                using ( HttpResponseMessage response = client.GetAsync(requestURL).Result )
                    {
                    if ( response.IsSuccessStatusCode )
                        {
                        using ( HttpContent content = response.Content )
                            {
                            string xrdpPackage = content.ReadAsStringAsync().Result;
                            if ( saveXrdp )
                                {
                                string fileName = baseResultName + @"\" + packageId;
                                System.IO.FileInfo file = new System.IO.FileInfo(fileName);
                                file.Directory.Create();

                                File.WriteAllText(file.FullName, xrdpPackage);
                                }
                            var document = XDocument.Parse(xrdpPackage);
                            XNamespace ns = "http://www.bentley.com/RealityDataServer/v1";

                            date = document.Root.Element(ns + "CreationDate").Value;

                            return document.Root.Element(ns + "BoundingPolygon").Value;
                            //XmlDocument xmlDoc = new XmlDocument();
                            //xmlDoc.LoadXml(content.ReadAsStringAsync().Result);

                            //return xmlDoc.SelectSingleNode("BoundingPolygon").InnerText;
                            }
                        }
                    else
                        {
                        throw new Exception("Package retrieval failed");
                        }
                    }

                }
            }
        }
    }
