
using System;
using System.IO;
using System.Text;
using System.Collections;
using SD = System.Data;
using SDO = System.Data.OleDb;
using NUnit.Framework;


namespace Bentley.TerrainModelNET.NUnit
{
 using Bentley.TerrainModelNET;
 
    /// <summary>
    /// 
    /// </summary>
    /// <category></category>
    [TestFixture]
    public class DTMMergeTests : DTMUNitTest 
    {

        static String DataDirectory = @"Bentley.Civil.Dtm.NUnit.dll\DTMMergeTest\";
        static String DataPath = Helper.GetTestDataLocation () + DataDirectory;
 
        /// <summary>
        /// DTMMergeTest
        /// </summary>
        /// <returns></returns>
        [Test]
        [Category("DTMMergeTests")]
        public void MergeTest1()
        {
         /// <summary>
         /// DTMMergeTest1
         /// Merging Data Sets With Previous Merge Problems
         /// </summary>

         // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}",this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

         // Get Test Data Directories

            Console.WriteLine("DataPath = {0}",DataPath);

            DirectoryInfo testFolder = new DirectoryInfo(DataPath);
            if (! testFolder.Exists)
            {
                throw new DirectoryNotFoundException("DTM Merge Nunit Test Data Header Folder Not Found + DataPath");
            }

         // List All The Test Data Directories

            foreach (DirectoryInfo nextFolder in testFolder.GetDirectories())
            {

                if (string.Compare(nextFolder.Name, "CVS") != 0)
                {

                    Console.WriteLine("Merge Folder = {0}", nextFolder.Name);

                    // Read DTM's From File

                    DTM merge1DTM = DTM.CreateFromFile(DataPath + nextFolder.Name + "\\merge1.tin");
                    DTM merge2DTM = DTM.CreateFromFile(DataPath + nextFolder.Name + "\\merge2.tin");

                    // Create New DTM For The Merged DTM

                    DTM mergeDTM = merge1DTM.Clone();

                    //Merge The DTM

                    DateTime startTime = DateTime.Now;
                    Console.WriteLine("Merging DTMs");
                    mergeDTM.Merge(merge2DTM);
                    TimeSpan elapsedTime = DateTime.Now - startTime;
                    Console.WriteLine("Time To Merge DTMs = {0} seconds", elapsedTime.TotalSeconds);

                    // Check The Merged Triangulation

                    if (mergeDTM.CheckTriangulation() == false)
                    {
                        throw new Exception("Merged Triangulation Invalid");
                    }
                    Console.WriteLine("Merged Triangulation Valid");

                    //  Dispose the DTM's

                    merge1DTM.Dispose();
                    merge2DTM.Dispose();
                    mergeDTM.Dispose();

                }
            }
        }
    }
}
