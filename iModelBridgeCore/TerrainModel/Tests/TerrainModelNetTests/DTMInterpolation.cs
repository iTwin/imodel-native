
using System;
using System.IO;
using System.Text;
using System.Collections;
using SD = System.Data;
using SDO = System.Data.OleDb;
using BGEO = Bentley.GeometryNET;
using NUnit.Framework;


namespace Bentley.TerrainModelNET.NUnit
{
 using Bentley.TerrainModelNET;
 
    /// <summary>
    /// 
    /// </summary>
    /// <category></category>
    [TestFixture]
    public class DTMInterpolationTests : DTMUNitTest
    {

        static String DataDirectory = @"Bentley.Civil.Dtm.NUnit.dll\DTMInterpolationTest\";
        static String DataPath = Helper.GetTestDataLocation () + DataDirectory;
 
        /// <summary>
        /// DTMInterpolationTest
        /// </summary>
        /// <returns></returns>
        [Test]
        [Category("DTMInterpolationTests")]
        public void InterpolationTest1()
        {
         /// <summary>
         /// DTMInterpolationTest1
         /// </summary>

         // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}",this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

         // Get Test Data Directories

            Console.WriteLine("DataPath = {0}",DataPath);

            DirectoryInfo testFolder = new DirectoryInfo(DataPath);
            if (! testFolder.Exists)
            {
                throw new DirectoryNotFoundException(" DTM Interpolation Nunit Test Data Header Folder Not Found + DataPath");
            }

         // List All The Test Data Directories

            foreach (DirectoryInfo nextFolder in testFolder.GetDirectories())
            {
                Console.WriteLine("Test Data Folder = {0}", nextFolder.Name);

                if (string.Compare(nextFolder.Name, "CVS") != 0)
                {

                    // Read DTM's From File

                    DTM pointsDTM = DTM.CreateFromFile(DataPath + nextFolder.Name + "\\points.dtm");
                    DTM dtmFeaturesDTM = DTM.CreateFromFile(DataPath + nextFolder.Name + "\\features.dtm");

                    // Create New DTM For Storing The Interpolated DTM Features

                    DTM interpolationDTM = new DTM() ;

                    //Interpolate The DTM Features In dtmFeaturesDTM

                    double snapTolerance = 5.0;
                    DateTime StartTime = DateTime.Now;
                    Console.WriteLine("Interpolating DTM Features");
                    InterpolationResult interpolationResult = dtmFeaturesDTM.InterpolateDtmFeatureType(DTMFeatureType.Breakline, snapTolerance, pointsDTM, interpolationDTM);
                    DateTime EndTime = DateTime.Now;

                    // Write Interpolation Results

                    Console.WriteLine("Number Of DTM Features = {0}", interpolationResult.NumDtmFeatures);
                    Console.WriteLine("Number Of DTM Features Interpolated = {0}",interpolationResult.NumDtmFeaturesInterpolated) ;

                    //  Dispose the DTM's

                    pointsDTM.Dispose();
                    dtmFeaturesDTM.Dispose();
                    interpolationDTM.Dispose();

                }
            }
        }
    }
}
