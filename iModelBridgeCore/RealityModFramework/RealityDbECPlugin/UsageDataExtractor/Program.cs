using System;
using System.Collections.Generic;
using System.Configuration;
using System.IdentityModel.Tokens;
using System.IO;
using System.Linq;
using System.Net;
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
            ServicePointManager
    .ServerCertificateValidationCallback +=
    (sender, cert, chain, sslPolicyErrors) => true;

            try
                {
                string fileName;
                if ( args.Length == 4 )
                    {
                    int year = Convert.ToInt32(args[2]);
                    int month = Convert.ToInt32(args[3]);

                    fileName = args[1];
                    if ( args[0].ToLower() == "extractv2" )
                        {
                        DateTime beginning = new DateTime(year, month, 1);
                        DateTime end = beginning.AddMonths(1);

                        ExtractV2(fileName, beginning, end);
                        }
                       
                    if ( args[0].ToLower() == "fetch" )
                        {

                        if ( !File.Exists(fileName) ) 
                            {
                            using ( StreamWriter file = new System.IO.StreamWriter(fileName) )
                                {

                                DateTime time = new DateTime(year, month, 1);

                                string tableName = "BCLogETW" + time.ToString("yyyyMM");
                                int day = 1;
                                while ( day <= 31 )
                                    {
                                    int hour = 0;
                                    while ( hour <= 23 )
                                        {
                                        try
                                            {
                                            time = new DateTime(year, month, day, hour, 0, 0);
                                            string lowerPartitionKey = time.ToString("yyyy-MM-dd:HH");
                                            string upperPartitionKey = time.AddHours(1).ToString("yyyy-MM-dd:HH");

                                            Console.Out.WriteLine(String.Format("Day: {0}, hour: {1}", day, hour));
                                            FetchResults(file, tableName, lowerPartitionKey, upperPartitionKey);
                                            }
                                        catch ( ArgumentOutOfRangeException )
                                            {
                                            //This is not a valid day, meaning we've ended the month.
                                            hour = 50;
                                            day = 50;
                                            }
                                        hour++;
                                        }
                                    day++;
                                    }
                                }
                            return;
                            }
                        else
                            {
                            throw new IOException("The output file already exists.");
                            }
                        }
                    }
                else if ( args.Length == 3 )
                    {
                    fileName = args[1];
                    if ( args[0].ToLower() == "extract" )
                        {
                        string extractionResultsFileNameBase = args[2];
                        Extract(fileName, extractionResultsFileNameBase);
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
            Console.Out.WriteLine("Before using the program, please configure the email entry of the exe.config file.");
            Console.Out.WriteLine("The password entry is optional. If not set, the program will ask for it when it is needed.");
            Console.Out.WriteLine(" ");
            Console.Out.WriteLine("Usage : UsageDataExtractor.exe extractv2 [resultFileName] [Year] [Month]");
            Console.Out.WriteLine("    [resultFileName] is the name of the file that will be written on disk and contain the results of the fetch operation.");
            Console.Out.WriteLine("    [Year] Is the year of the period for which we want the data.");
            Console.Out.WriteLine("    [Month] Is the month of the period for which we want the data.");
            Console.Out.WriteLine("***THE FOLLOWING IS DEPRECATED***");
            Console.Out.WriteLine("**********************************************************************************************");
            Console.Out.WriteLine("Usage 1: UsageDataExtractor.exe fetch [fetchFileName] [Year] [Month]");
            Console.Out.WriteLine("  The fetch mode searches the GCS logs located in the azure table and gets all xrdp GUIDs captured by the logs,");
            Console.Out.WriteLine("  along with the email of the requester.");
            Console.Out.WriteLine("    [fetchFileName] is the name of the file that will be written on disk and contain the results of the fetch operation.");
            Console.Out.WriteLine("    [Year] Is the year of the period for which we want the data.");
            Console.Out.WriteLine("    [Month] Is the month of the period for which we want the data.");
            Console.Out.WriteLine("    The extractor uses the year and month to know from which azure table to extract the data.");
            Console.Out.WriteLine("         Example : For 2016 08, the table name is BCLogETW201608");
            Console.Out.WriteLine(" ");
            Console.Out.WriteLine(@"   Example: UsageDataExtractor.exe fetch C:\Data\QueryResults\xrdpGUIDsAUGUST.txt 2016 08");
            Console.Out.WriteLine(" ");
            Console.Out.WriteLine("  BE WARNED: THIS MODE CAN RUN FOR MANY HOURS, PROBABLY SEVERAL DAYS");
            Console.Out.WriteLine("-------------------------------------------------------------------------");
            Console.Out.WriteLine("Usage 2: UsageDataExtractor.exe extract [fetchFileName] [resultsFileNameBase]");
            Console.Out.WriteLine("    Downloads the actual xrdp files from the GCS and extracts useful information.");
            Console.Out.WriteLine("    [fetchFileName] is the name of the file containing the xrdp GUIDs. This should be the file created by the fetch mode.");
            Console.Out.WriteLine("    [resultsFileNameBase] is the base name of the result files that will be written on disk.");
            Console.Out.WriteLine("                          An extension will be added by the program.");
            Console.Out.WriteLine(" ");
            Console.Out.WriteLine(@"   Example: UsageDataExtractor.exe extract C:\data\xrdpGUIDsAUGUST.txt C:\data\QueryResults\resultsAUGUST");
            Console.Out.WriteLine("**********************************************************************************************");


            }

        private static void FetchResults (StreamWriter file, string tableName, string lowerPartitionKey, string upperPartitionKey)
            {
            CloudStorageAccount storageAccount = CloudStorageAccount.Parse(ConfigurationManager.AppSettings["connectionString"]);
            CloudTableClient tableClient = storageAccount.CreateCloudTableClient();
            CloudTable logTable = tableClient.GetTableReference(tableName);
            //string condition = TableQuery.GenerateFilterCondition("ProductShortName", QueryComparisons.Equal, "ContextServices") + " and " +;
            string condition = "PartitionKey ge '" + lowerPartitionKey + "' and PartitionKey lt '" + upperPartitionKey + "' and RowKey gt 'INFO_ContextServicesWeb' and RowKey lt 'INFO_ContextServicesWec' and ProductShortName eq 'ContextServices' and HttpMethod eq 'GET' and Content gt 'INFO - Bentley.Mas: Request GET:''https://connect-contextservices.bentley.com/v2.3/Repositories/IndexECPlugin--Server/RealityModeling/PreparedPackag' and Content lt 'INFO - Bentley.Mas: Request GET:''https://connect-contextservices.bentley.com/v2.3/Repositories/IndexECPlugin--Server/RealityModeling/PreparedPackah'";
            TableQuery<DynamicTableEntity> query = new TableQuery<DynamicTableEntity>().Where(condition).Select(new string[] { "Content", "User" });

            IEnumerable<DynamicTableEntity> results = logTable.ExecuteQuery(query);

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

        static void Extract (string inputName, string baseResultName)
            {
            string line;

            string serverURL = ConfigurationManager.AppSettings["serverURL"];

            DateTime lastTokenTime = DateTime.Now;
            string base64Token = GetToken();

            TimeSpan limitSpan = new TimeSpan(0, 30, 0);

            using ( StreamReader file = new System.IO.StreamReader(inputName) )
                {
                using ( StreamWriter fileToWrite = new System.IO.StreamWriter(baseResultName + ".txt") )
                    {
                    fileToWrite.WriteLine("PackageId, Date, Email, Country, Polygon");
                    //fileToWrite.WriteLine("PackageId, Email, Country, Polygon");
                    while ( (line = file.ReadLine()) != null )
                        {

                        if ( line.IndexOf("bentley.com", StringComparison.OrdinalIgnoreCase) == -1 && line.IndexOf("@mailinator.com", StringComparison.OrdinalIgnoreCase) == -1 )
                            {
                            if ( (DateTime.Now - lastTokenTime) > limitSpan )
                                {
                                lastTokenTime = DateTime.Now;
                                base64Token = GetToken();
                                }
                            string packageId = line.Split(' ').First();
                            string date;
                            string poly;
                            try
                                {
                                poly = GetPolygon(packageId, serverURL, base64Token, baseResultName, out date);
                                }
                            catch (Exception)
                                {
                                continue;
                                }
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

                            fileToWrite.WriteLine(line.Split(' ').First() + ", " + date.Substring(0, 10) + ", " + email + ", " + country + ", " + poly);

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

        private static void ExtractV2 (string resultFileName, DateTime beginning, DateTime end)
            {

            string serverURL = ConfigurationManager.AppSettings["serverURL"];

            DateTime lastTokenTime = DateTime.Now;
            string base64Token = GetToken();

            JArray statsArray = GetJSonStats(serverURL, base64Token, beginning, end);

            using ( StreamWriter file = new StreamWriter(resultFileName) )
                {
                file.WriteLine("PackageId, Date, UserId, Country, Polygon");
                foreach ( JToken packageStatToken in statsArray )
                    {
                    JObject packageStat = packageStatToken as JObject;
                    JObject properties = packageStat["properties"] as JObject;


                    string packageId = properties["Name"].Value<string>();
                    string dateString = properties["CreationTime"].Value<string>().Substring(0, 10);
                    string userId = properties["UserId"].Value<string>();
                    string poly = properties["BoundingPolygon"].Value<string>();
                    string country = ExtractCountry(poly);

                    file.WriteLine(packageId + "," + dateString + "," + userId + "," + country + "," + poly);

                    }
                }
            }

        private static JArray GetJSonStats(string serverURL, string base64Token, DateTime beginning, DateTime end)
            {
            string beginningCriteria = beginning.ToString("yyyy-MM-dd");
            string endCriteria = end.ToString("yyyy-MM-dd");
            string requestURL = serverURL + "v2.4/Repositories/IndexECPlugin--Server/RealityModeling/PackageStats?$filter=CreationTime+gt+datetime'" + beginningCriteria + "'+and+CreationTime+lt+datetime'" + endCriteria + "'";

            using ( HttpClient client = new HttpClient() )
                {
                client.DefaultRequestHeaders.Accept.Add(new MediaTypeWithQualityHeaderValue("application/json"));
                client.DefaultRequestHeaders.Authorization = new AuthenticationHeaderValue("Token", base64Token);

                using ( HttpResponseMessage response = client.GetAsync(requestURL).Result )
                    {
                    if ( response.IsSuccessStatusCode )
                        {
                        using ( HttpContent content = response.Content )
                            {
                            JObject jsonResponse = JObject.Parse(content.ReadAsStringAsync().Result);

                            return jsonResponse["instances"] as JArray;
                            }
                        }
                    else
                        {
                        throw new Exception("Packaging stats extraction failed");
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

                            var result = jsonResult["results"].FirstOrDefault(r => r["types"].Any(entry => entry.Value<string>() == "country"));
                            if(result == null)
                                {
                                result = jsonResult["results"].FirstOrDefault();
                                }
                            var countryJson = result["address_components"].FirstOrDefault(r => r["types"].Any(entry => entry.Value<string>() == "country"));
                            if ( countryJson != null )
                                {
                                return result["address_components"].First(r => r["types"].Any(entry => entry.Value<string>() == "country"))["long_name"].Value<string>();
                                }
                            else
                                {
                                return "None";
                                }
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

        private static string GetToken ()
            {
            //SecurityToken token = null;

            //string strServiceURL = ConfigurationManager.AppSettings["serviceUrl"];

            //Uri BaseUri = new Uri(strServiceURL);

            //string activeSTSUrl = ConfigurationManager.AppSettings["activeSTSUrl"];

            //StsHelper sts = new StsHelper(activeSTSUrl, BaseUri.ToString().Replace("/api/", "/"));
            //string clientKey = ConfigurationManager.AppSettings["email"];

            //token = sts.GetSecurityToken(clientKey, password, new List<Microsoft.IdentityModel.Protocols.WSTrust.RequestClaim>(), new Dictionary<string, string>());

            //string tokenString = StsHelper.SerializeToken(token);

            //return System.Convert.ToBase64String(System.Text.Encoding.UTF8.GetBytes(tokenString));
            var connectApi = new ConnectAPI(ConfigurationManager.AppSettings["serviceUrl"]);
            return connectApi.GetTokenIfConnected();
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

                            try
                                {
                                date = document.Root.Element(ns + "CreationDate").Value;
                                }
                            catch ( Exception e )
                                {
                                date = default(DateTime).ToString("yyyy-MM-dd'T'HH:mm:ss'Z'");
                                }

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
