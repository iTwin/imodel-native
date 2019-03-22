using System;
using System.Collections.Generic;

namespace S3MXConsole
    {
    class Program
        {
        // /h /H /? for help in console
        // /u /U    for upload feature
        // /ls /LS  for file listing feature

        private static List<Tuple<string, string>> listSrcDest = new List<Tuple<string, string>>();
        static void Main (string[] args)
            {
            if ( args.Length <= 0 || args == null )
                {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine("No action has been chosen. Use the /h command for help");
                Console.ForegroundColor = ConsoleColor.Gray;
                return;
                }


            core3mx s3mxAzureApi = null;
            if ( args[0].ToString() == "/u" || args[0].ToString() == "/U" || args[0].ToString() == "/ls" || args[0].ToString() == "/LS" )
                {
                // create the 3mxApi object
                s3mxAzureApi = new core3mx();
                // create the container s3mx if it does not exist
                s3mxAzureApi.CreateContainer("s3mx");
                }

            switch ( args[0].ToString() )
                {
                case "/h":
                case "/H":
                case "/?":
                    printHelper();
                    break;

                case "/u":
                case "/U":
                    if ( args.Length <= 1 )
                        {
                        Console.ForegroundColor = ConsoleColor.Red;
                        Console.WriteLine("Source is needed");
                        Console.ForegroundColor = ConsoleColor.Gray;
                        Console.WriteLine();
                        break;
                        }

                    if ( args.Length <= 2 )
                        {
                        Console.ForegroundColor = ConsoleColor.Red;
                        Console.WriteLine("Destination name is needed");
                        Console.ForegroundColor = ConsoleColor.Gray;
                        Console.WriteLine();
                        break;
                        }

                    string source = args[1].ToString();
                    string destName = args[2].ToString();

                    int getListStatus = s3mxAzureApi.GetListOfFileInfos(source, destName, ref listSrcDest);
                    int uploadStatus = s3mxAzureApi.UploadDirectoryWithWsgMultiThreads(listSrcDest);
                    if ( getListStatus == 0 && uploadStatus == 0 )
                        {
                        Console.ForegroundColor = ConsoleColor.Green;
                        Console.WriteLine("All directories uploaded successfully");
                        Console.ForegroundColor = ConsoleColor.Gray;
                        }
                    break;

                case "/ls":
                case "/LS":
                    if ( args.Length > 1 )
                        {
                        s3mxAzureApi.ListDir(args[1].ToString());
                        }
                    else
                        {
                        s3mxAzureApi.ListDir();
                        }

                    break;
                default:
                    Console.ForegroundColor = ConsoleColor.Red;
                    Console.WriteLine("Unknown command");
                    Console.ForegroundColor = ConsoleColor.Gray;
                    Console.WriteLine();
                    printHelper();
                    break;
                }

            return;
            }

        private static void printHelper ()
            {
            Console.ForegroundColor = ConsoleColor.Gray;
            Console.WriteLine("Interact with the s3mx service. A valid IMS account is needed.");
            Console.WriteLine();
            Console.WriteLine("Bentley.S3MXConsole.exe [/LS[:path]] [/U:source:name]");
            Console.WriteLine();
            Console.WriteLine("/U:      used to upload a whole directory to azure blob storage");
            Console.WriteLine("         Parameter 1: The source folder as a path");
            Console.WriteLine("         Parameter 2: Wanted name for the folder once on azure");
            Console.WriteLine("         u C:\\s3mx\\Production_Graz_3MX Production_Graz_3MX ");
            Console.WriteLine();
            Console.WriteLine("/LS:     used to list all folders/documents under the desired InstanceId");
            Console.WriteLine("         Optional Parameter: IntanceId");
            Console.WriteLine("         ls ECObjects--S3MX-Folder-Production_Graz_3MX~2FScene~2F");
            }
        }
    }
