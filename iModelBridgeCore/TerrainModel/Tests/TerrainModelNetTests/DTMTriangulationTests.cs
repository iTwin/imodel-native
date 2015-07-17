#region Using
using System;
using System.IO;
using SD = System.Data;
using SDSQL = System.Data.SqlClient;
using SDO = System.Data.OleDb;
using BGEO = Bentley.GeometryNET;
using NUnit.Framework;
#endregion
// 
// These files have been copied from SimpleTestsDTM
// and modified to read a data file and create a DTM
//
namespace Bentley.TerrainModelNET.NUnit
    {
    using Bentley.TerrainModelNET;
    using SDO = System.Data.OleDb;

    /// <summary>
    ///   Triangulation
    /// </summary>
    /// <category>DTM Triangulation</category>

    [TestFixture]
    public class DTMTriangulationTests : DTMUNitTest
        {
        #region Private Fields
        private System.IFormatProvider _formatProvider = System.Globalization.NumberFormatInfo.CurrentInfo;
        #endregion Private Fields

        #region Constructor

        /// <summary>
        /// DTMTriangulationTests 
        /// </summary>
        public DTMTriangulationTests()
            {
            }
        #endregion constructor

        static int num;
        static int numPoints;

        /// <summary>
        /// DTM Creator Routine
        /// </summary>
        private void XYZTriangulationDriver(string TestToRun)
            {       
            //List constant string variables
            String dbPath = @"Bentley.Civil.Dtm.NUnit.dll\xyzTriangulation\";

            String XYZTriangulationDb = "XYZTriangulation.mdb";
            String XYZTriangulationTable = "XYZTriangulationTests";

            String DataPath = Helper.GetTestDataLocation () + dbPath;

            Console.WriteLine("DataPath = {0}", DataPath );

            DatabaseAccess myDb = new DatabaseAccess(DataPath, XYZTriangulationDb, XYZTriangulationTable, TestToRun);
            myDb.OpenTheDatabase();

            //Get the file name of the XYZ data file
            String XYZFileName = DataPath + myDb.ReadFromTheDatabase("FileName");
            if (!System.IO.File.Exists(XYZFileName))
                throw new Exception("CANNOT FIND FILE: The filename given in the database does not exist.\nFilename = " + XYZFileName + "\nDatabase Table= " + XYZTriangulationTable + "\nDatabase = " + XYZTriangulationDb);

            Double RunTime = RunAndTimeTheTest(XYZFileName);

            Console.WriteLine("Time to run test \"{0}\" was {1} seconds.", TestToRun, RunTime);
            myDb.CloseTheDatabase();
            }

        private void datTriangulationDriver(string TestToRun)
            {
            //List constant string variables
            String dbPath = @"Bentley.Civil.Dtm.NUnit.dll\datTriangulation\";
            String datTriangulationDb = "datTriangulation.mdb";
            String datTriangulationTable = "datTriangulationTests";

            // Create derived strings
            String DataPath = Helper.GetTestDataLocation () + dbPath;

            DatabaseAccess myDb = new DatabaseAccess(DataPath, datTriangulationDb, datTriangulationTable, TestToRun);
            myDb.OpenTheDatabase();

            //Get the file name of the Dat data file
            String datFileName = DataPath + myDb.ReadFromTheDatabase("FileName");
            if (!System.IO.File.Exists(datFileName))
                throw new Exception("CANNOT FIND FILE: The filename given in the database does not exist.\nFilename = " + datFileName + "\nDatabase Table= " + datTriangulationTable + "\nDatabase = " + datTriangulationDb);

            Double RunTime = RunAndTimeTheTest(datFileName);

            Console.WriteLine("Time to run test \"{0}\" was {1} seconds.", TestToRun, RunTime);
            myDb.CloseTheDatabase();
            }


        /// <summary>
        /// UpdateTheDatabase
        /// </summary>
        /// <param name="ThisRun"></param>
        /// <param name="SQLCmd"></param>
        /// <param name="DataTable"></param>
        /// <param name="WhichTest"></param>
        /// <param name="DbCon"></param>
        /// <param name="ds"></param>
        /// <param name="Adapter"></param>
        private void UpdateTheDatabase(Double ThisRun, String SQLCmd, String DataTable, String WhichTest, ref SDO.OleDbConnection DbCon, SD.DataSet ds, ref SDO.OleDbDataAdapter Adapter)
            {
            Console.WriteLine("Updating {0}.", DataTable);

            // Database "ThisRun" value becomes the LastRun value on Update            
            Double LastRun = Convert.ToDouble(ReadFromDatabase("ThisRun", SQLCmd, DataTable, ref DbCon, ds, ref Adapter));
            Double Mean = Convert.ToDouble(ReadFromDatabase("Mean", SQLCmd, DataTable, ref DbCon, ds, ref Adapter));
            Double Repeats = Convert.ToDouble(ReadFromDatabase("Repeats", SQLCmd, DataTable, ref DbCon, ds, ref Adapter));

            // Calculate Progressive Mean
            if (Mean != 0)
                Mean = (Repeats * Mean) / (Repeats + 1) + ThisRun / (Repeats + 1);
            else
                Mean = ThisRun;

            // Increase the number of repeats
            Repeats += 1.0;

            Double DRun = ThisRun - LastRun;
            Double DMean = ThisRun - Mean;

            //Bind the datatable to the dataset
            ds.Tables[DataTable].Rows[0]["LastRun"] = LastRun.ToString();
            ds.Tables[DataTable].Rows[0]["ThisRun"] = ThisRun.ToString();
            ds.Tables[DataTable].Rows[0]["Mean"] = Mean.ToString();
            ds.Tables[DataTable].Rows[0]["Repeats"] = Repeats.ToString();
            ds.Tables[DataTable].Rows[0]["DeltaRun"] = DRun.ToString();
            ds.Tables[DataTable].Rows[0]["DeltaMean"] = DMean.ToString();

            Adapter.UpdateCommand = new SDO.OleDbCommand("UPDATE " + DataTable + " SET LastRun = ?, ThisRun = ?, Mean = ?, Repeats = ?, DeltaRun = ?, DeltaMean = ? WHERE WhichTest = '" + WhichTest + "'", DbCon);

            Adapter.UpdateCommand.Parameters.Add("@LastRun", SDO.OleDbType.Char, 50, "LastRun");
            Adapter.UpdateCommand.Parameters.Add("@ThisRun", SDO.OleDbType.Char, 50, "ThisRun");
            Adapter.UpdateCommand.Parameters.Add("@Mean", SDO.OleDbType.Char, 50, "Mean");
            Adapter.UpdateCommand.Parameters.Add("@Repeats", SDO.OleDbType.Char, 50, "Repeats");
            Adapter.UpdateCommand.Parameters.Add("@DeltaRun", SDO.OleDbType.Char, 50, "DeltaRun");
            Adapter.UpdateCommand.Parameters.Add("@DeltaMean", SDO.OleDbType.Char, 50, "DeltaMean");

            Adapter.Update(ds, DataTable);

            // Output the values
            Console.WriteLine("\nTest Details\n------------");
            Console.WriteLine("Time for last test= {0}\nTime for this test= {1}\nDifference= {2}", LastRun, ThisRun, DRun);
            Console.WriteLine("Mean test time= {0}\nDifference= {1}", Mean, DMean);
            }


        /// <summary>
        /// GetXYZData
        /// </summary>
        /// <param name="WhichData"></param>
        /// <param name="SQLQuery"></param>
        /// <param name="DataTable"></param>
        /// <param name="DbCon"></param>
        /// <param name="ds"></param>
        /// <param name="Adapter"></param>
        /// <returns></returns>
        private static String ReadFromDatabase(String WhichData, String SQLQuery, String DataTable, ref SDO.OleDbConnection DbCon, SD.DataSet ds, ref SDO.OleDbDataAdapter Adapter)
            {
            SDO.OleDbCommand GetCmd = DbCon.CreateCommand();
            GetCmd.CommandText = SQLQuery;
            SDO.OleDbDataReader ThisReader = GetCmd.ExecuteReader();
            SD.DataTable Schema = ThisReader.GetSchemaTable();
            ThisReader.Read();
            String Info = ThisReader[WhichData].ToString();

            // Check for blank strings
            if (Info.Length == 0)
                Info = "0";
            ThisReader.Close();
            return Info;
            }

        /// <summary>
        ///  Triangulates XYZ Files
        /// </summary>
        /// <category>DTM Triangulation</category>
        [Test]
        [Category("DTMTriangulation")]
        public void TriangulateXYZFiles()
        {

           // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

           //  Set Path To Folder Holding XYZ Files To Triangulate

            String DataPath = Helper.GetTestDataLocation () +@"Bentley.Civil.Dtm.NUnit.dll\DTMTriangulationTest\DTMTriangulationXyzTest\";

          // Get Test Data Directories

            DirectoryInfo testFolder = new DirectoryInfo (DataPath);
            if (! testFolder.Exists)
            {
                throw new DirectoryNotFoundException("DTM XYZ Triangulation Nunit Test Data Header Folder Not Found ");
            }

         // Scan And Triangulate All The XYZ Files In The Folder

            foreach (System.IO.FileInfo nextFile in testFolder.GetFiles())
            {
                if (string.Compare(nextFile.Extension, ".xyz") == 0)
                {
                    Console.WriteLine("XYZ File = {0}", nextFile.Name);

                    // Import The XYZ File

                    Console.WriteLine("Importing {0} To DTM", nextFile.Name);
                    DateTime impStartTime = DateTime.Now;
                    DTM xyzDTM = DTM.CreateFromXyzFile(DataPath + nextFile.Name);
                    DateTime impEndTime = DateTime.Now;
                    TimeSpan impTime = impEndTime - impStartTime;
                    Console.WriteLine("Import Time was {0} seconds.", impTime.TotalSeconds);

                    // Triangulate Imported XYZ File

                    Console.WriteLine("Triangulating DTM");
                    DateTime trgStartTime = DateTime.Now;
                    xyzDTM.Triangulate();
                    DateTime trgEndTime = DateTime.Now;
                    TimeSpan trgTime = trgEndTime - trgStartTime;
                    Console.WriteLine("Triangulation Time was {0} seconds.", trgTime.TotalSeconds);

                    // Check The Triangulation - Unit Test Checking Only

                    Console.WriteLine("Checking Triangulation");
                    DateTime chkStartTime = DateTime.Now;
                    if (xyzDTM.CheckTriangulation() == false)
                    {
                        throw new Exception("\nTriangulation Invalid");
                    }
                    Console.WriteLine("Triangulation Valid");
                    DateTime chkEndTime = DateTime.Now;
                    TimeSpan chkTime = chkEndTime - chkStartTime;
                    Console.WriteLine("Triangulation Validation Time was {0} seconds.", chkTime.TotalSeconds);

                    // Dispose DTM To Release Allocated Unmanaged Memory

                    xyzDTM.Dispose();
                }
             }
        }


        /// <summary>
        ///  XYZ Triangulation Of 1 Million Points For Timing Purposes
        ///  
        /// </summary>
        /// <category>DTM Triangulation</category>
        [Test]
        [Category("DTMTriangulation")]
        public void Triangulate1M_XYZFile()
        {
            // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test
        
            //  Set Path To 1 Million Point XYZ File To Triangulate

            String DataPath = Helper.GetTestDataLocation () +@"Bentley.Civil.Dtm.NUnit.dll\DTMTriangulationTest\DTMTriangulationXyzTest\1m.xyz";
        
            if( System.IO.File.Exists(DataPath))
            {
                // Import The XYZ File

                Console.WriteLine("Importing 1M.XYZ To DTM");
                DateTime impStartTime = DateTime.Now;
                DTM xyzDTM = DTM.CreateFromXyzFile(DataPath);
                DateTime impEndTime = DateTime.Now;
                TimeSpan impTime = impEndTime - impStartTime;
                Console.WriteLine("Import Time was {0} seconds.", impTime.TotalSeconds);

                 // Triangulate Imported XYZ File

                 Console.WriteLine("Triangulating DTM");
                 DateTime trgStartTime = DateTime.Now;
                 xyzDTM.Triangulate();
                 DateTime trgEndTime = DateTime.Now;
                 TimeSpan trgTime = trgEndTime - trgStartTime;
                 Console.WriteLine("Triangulation Time was {0} seconds.", trgTime.TotalSeconds);

                 // Check The Triangulation - Unit Test Checking Only - Not Needed For Production Apps

                 Console.WriteLine("Checking Triangulation");
                 DateTime chkStartTime = DateTime.Now;
                 if (xyzDTM.CheckTriangulation() == false)
                 {
                        throw new Exception("\nTriangulation Invalid");
                 }
                 Console.WriteLine("Triangulation Valid");
                 DateTime chkEndTime = DateTime.Now;
                 TimeSpan chkTime = chkEndTime - chkStartTime;
                 Console.WriteLine("Triangulation Validation Time was {0} seconds.", chkTime.TotalSeconds);


                // Dispose DTM To Release Allocated Unmanaged Memory

                 xyzDTM.Dispose();
            }
        }


        /// <summary> 
        /// Triangulation Of 10 Million XYZ Points For Timing Purposes
        /// </summary>
        /// <category>DTM Triangulation</category>
        [Test]
        [Category("DTMTriangulation")]
        public void Triangulate10M_XYZFile()
        {
            // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

            //  Set Path To 10 Million Point XYZ File To Triangulate

            String DataPath = Helper.GetTestDataLocation () +@"Bentley.Civil.Dtm.NUnit.dll\DTMTriangulationTest\DTMTriangulationXyzTest\10m.xyz";

            if (System.IO.File.Exists(DataPath))
            {
                // Import The XYZ File

                Console.WriteLine("Importing 10M.XYZ To DTM");
                DateTime impStartTime = DateTime.Now;
                DTM xyzDTM = DTM.CreateFromXyzFile(DataPath);
                DateTime impEndTime = DateTime.Now;
                TimeSpan impTime = impEndTime - impStartTime;
                Console.WriteLine("Import Time was {0} seconds.", impTime.TotalSeconds);

                // Triangulate Imported XYZ File

                Console.WriteLine("Triangulating DTM");
                DateTime trgStartTime = DateTime.Now;
                xyzDTM.Triangulate();
                DateTime trgEndTime = DateTime.Now;
                TimeSpan trgTime = trgEndTime - trgStartTime;
                Console.WriteLine("Triangulation Time was {0} seconds.", trgTime.TotalSeconds);

                // Check The Triangulation - Unit Test Checking Only

                Console.WriteLine("Checking Triangulation");
                DateTime chkStartTime = DateTime.Now;
                if (xyzDTM.CheckTriangulation() == false)
                {
                    throw new Exception("\nTriangulation Invalid");
                }
                Console.WriteLine("Triangulation Valid");
                DateTime chkEndTime = DateTime.Now;
                TimeSpan chkTime = chkEndTime - chkStartTime;
                Console.WriteLine("Triangulation Validation Time was {0} seconds.", chkTime.TotalSeconds);

                // Dispose DTM To Release Allocated Unmanaged Memory

                xyzDTM.Dispose();
            }
        }


        /// <summary> 
        /// Triangulation Of 15 Million XYZ Points For Timing Purposes
        /// </summary>
        /// <category>DTM Triangulation</category>
        [Test]
        [Category("DTMTriangulation")]
        public void Triangulate15M_XYZFile()
        {
            // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

            //  Set Path To 15 Million Point XYZ Files To Triangulate

            String DataPath = Helper.GetTestDataLocation () +@"Bentley.Civil.Dtm.NUnit.dll\DTMTriangulationTest\DTMTriangulationXyzTest\15m.xyz";

            if (System.IO.File.Exists(DataPath))
            {
                // Import The XYZ File

                Console.WriteLine("Importing 15M.XYZ To DTM");
                DateTime impStartTime = DateTime.Now;
                DTM xyzDTM = DTM.CreateFromXyzFile(DataPath);
                DateTime impEndTime = DateTime.Now;
                TimeSpan impTime = impEndTime - impStartTime;
                Console.WriteLine("Import Time was {0} seconds.", impTime.TotalSeconds);

                // Triangulate Imported XYZ File

                Console.WriteLine("Triangulating DTM");
                DateTime trgStartTime = DateTime.Now;
                xyzDTM.Triangulate();
                DateTime trgEndTime = DateTime.Now;
                TimeSpan trgTime = trgEndTime - trgStartTime;
                Console.WriteLine("Triangulation Time was {0} seconds.", trgTime.TotalSeconds);

                // Check The Triangulation - Unit Test Checking Only

                Console.WriteLine("Checking Triangulation");
                DateTime chkStartTime = DateTime.Now;
                if (xyzDTM.CheckTriangulation() == false)
                {
                    throw new Exception("\nTriangulation Invalid");
                }
                Console.WriteLine("Triangulation Valid");
                DateTime chkEndTime = DateTime.Now;
                TimeSpan chkTime = chkEndTime - chkStartTime;
                Console.WriteLine("Triangulation Validation Time was {0} seconds.", chkTime.TotalSeconds);

                // Dispose DTM To Release Allocated Unmanaged Memory

                xyzDTM.Dispose();
            }
        }



        /// <summary> 
        /// Triangulation Of Soft Breaks
        /// </summary>
        /// <category>DTM Triangulation</category>
        [Test]
        [Category("DTMTriangulation")]
        public void TriangulateSoftBreak()
        {
            // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

            //  Set Path 

            String DataPath = Helper.GetTestDataLocation () +@"Bentley.Civil.Dtm.NUnit.dll\DTMTriangulationTest\DTMTriangulationDatTest\SoftBreaks00.dat";

            if (System.IO.File.Exists(DataPath))
            {
                // Import The XYZ File

                Console.WriteLine("Importing SoftBreaks00.dat To DTM");
                DateTime impStartTime = DateTime.Now;
                DTM xyzDTM = DTM.CreateFromGeopakDatFile(DataPath);
                DateTime impEndTime = DateTime.Now;
                TimeSpan impTime = impEndTime - impStartTime;
                Console.WriteLine("Import Time was {0} seconds.", impTime.TotalSeconds);

                // Triangulate Imported Dat File

                Console.WriteLine("Triangulating DTM");
                DateTime trgStartTime = DateTime.Now;
                xyzDTM.Triangulate();
                DateTime trgEndTime = DateTime.Now;
                TimeSpan trgTime = trgEndTime - trgStartTime;
                Console.WriteLine("Triangulation Time was {0} seconds.", trgTime.TotalSeconds);

                // Check The Triangulation - Unit Test Checking Only

                Console.WriteLine("Checking Triangulation");
                DateTime chkStartTime = DateTime.Now;
                if (xyzDTM.CheckTriangulation() == false)
                {
                    throw new Exception("\nTriangulation Invalid");
                }
                Console.WriteLine("Triangulation Valid");
                DateTime chkEndTime = DateTime.Now;
                TimeSpan chkTime = chkEndTime - chkStartTime;
                Console.WriteLine("Triangulation Validation Time was {0} seconds.", chkTime.TotalSeconds);

                // Browse The Soft Breaks
                LinearFeaturesBrowsingDelegate featureBrowser = new LinearFeaturesBrowsingDelegate(BrowseFeatures);
                LinearFeaturesBrowsingCriteria featuresCriteria = new LinearFeaturesBrowsingCriteria();
                int numFeatures = 0;
                xyzDTM.BrowseLinearFeatures(featuresCriteria, DTMFeatureType.SoftBreakline, featureBrowser, numFeatures);
                Console.WriteLine("Number Of Soft Breaks Browsed = {0}",numFeatures) ;

                // Copy The DTM To A Memory Block

                Console.WriteLine("Copying DTM To Memory Block");
                sbyte[] memoryBlock = xyzDTM.CopyToMemoryBlock();
                Console.WriteLine("Memory Block Size = {0}", memoryBlock.Length);

                // Create A New DTM From The Memory Block

                xyzDTM.Dispose();

                Console.WriteLine("Importing DTM From Memory Block");
                DTM blockDTM = DTM.CreateFromMemoryBlock(memoryBlock);

                // Check The Triangulation - Unit Test Checking Only - Not Required In Production Apps

                Console.WriteLine("Checking Triangulation");
                chkStartTime = DateTime.Now;
                if (blockDTM.CheckTriangulation() == false)
                {
                    throw new Exception("\nTriangulation Invalid");
                }
                Console.WriteLine("Triangulation Valid");
                chkEndTime = DateTime.Now;
                chkTime = chkEndTime - chkStartTime;
                Console.WriteLine("Triangulation Validation Time was {0} seconds.", chkTime.TotalSeconds);

                // Dispose DTM To Release Allocated Unmanaged Memory

                xyzDTM.Dispose();
                blockDTM.Dispose();
            }
        }


        private bool BrowseFeatures(DTMFeatureInfo featureInfo, object oArg)
        {
            Console.WriteLine("Number Of Points = {0}",featureInfo,featureInfo.Points.Length) ;
            // Store Large Points In DTM
            int numFeatures = (int)oArg;
            ++numFeatures ;
             return true;
        }



        # region Geopak DAT tests
        /// <summary>
        ///  Triangulates Dat Files
        /// </summary>
        /// <category>DTM Triangulation</category>
        [Test]
        [Category("DTMTriangulation")]
        public void TriangulateDatFiles()
        {

            // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

            //  Set Path To Folder Holding XYZ Files To Triangulate

            String DataPath = Helper.GetTestDataLocation () +@"Bentley.Civil.Dtm.NUnit.dll\DTMTriangulationTest\DTMTriangulationDatTest\";

            // Get Test Data Directories

            DirectoryInfo testFolder = new DirectoryInfo(DataPath);
            if (!testFolder.Exists)
            {
                throw new DirectoryNotFoundException(" DTM Dat Triangulation Nunit Test Data Header Folder Not Found ");
            }

            // Scan And Triangulate All The XYZ Files In The Folder

            foreach (System.IO.FileInfo nextFile in testFolder.GetFiles())
            {
                if( string.Compare(nextFile.Extension,".dat") == 0 )
                {
                     Console.WriteLine("******** Dat File = {0}", nextFile.Name);

                     // Import The Dat File

                     Console.WriteLine("Importing {0} To DTM", nextFile.Name);
                     DateTime impStartTime = DateTime.Now;
                     DTM datDTM = DTM.CreateFromGeopakDatFile(DataPath + nextFile.Name);
                     DateTime impEndTime = DateTime.Now;
                     TimeSpan impTime = impEndTime - impStartTime;
                     Console.WriteLine("Import Time was {0} seconds.", impTime.TotalSeconds);
                     DtmState dtmState = datDTM.GetDtmState();
                     Console.WriteLine("DtmState = {0}",dtmState);
                     if (dtmState != DtmState.Tin) Console.WriteLine("Dtm Not In Tin State");

                      // Triangulate Imported Dat File

                     Console.WriteLine("Triangulating DTM");
                     DateTime trgStartTime = DateTime.Now;
                     datDTM.Triangulate();
                     DateTime trgEndTime = DateTime.Now;
                     TimeSpan trgTime = trgEndTime - trgStartTime;
                     Console.WriteLine("Triangulation Time was {0} seconds.", trgTime.TotalSeconds);
                     Console.WriteLine("DtmState = {0}", datDTM.GetDtmState());

                     // Check The Triangulation - Unit Test Checking Only

                     Console.WriteLine("Checking Triangulation");
                     DateTime chkStartTime = DateTime.Now;
                     if (datDTM.CheckTriangulation() == false)
                        {
                         throw new Exception("\nTriangulation Invalid");
                        }
                     Console.WriteLine("Triangulation Valid");
                     DateTime chkEndTime = DateTime.Now;
                     TimeSpan chkTime = chkEndTime - chkStartTime;
                     Console.WriteLine("Triangulation Validation Time was {0} seconds.", chkTime.TotalSeconds);


                     // Convert To Untriangulated State

                     Console.WriteLine("Converting DTM From Tin State To Data State");
                     Console.WriteLine("Before Conversion ** DtmState = {0}", datDTM.GetDtmState());
                     datDTM.ConvertToDataState();
                     Console.WriteLine("After  Conversion ** DtmState = {0}", datDTM.GetDtmState());
                     bool triangulate = true ;
                     if (triangulate)
                     {
                         datDTM.Triangulate();
                         Console.WriteLine("After Triangulation ** DtmState = {0}", datDTM.GetDtmState());
                         Console.WriteLine("Checking Triangulation");
                         if (datDTM.CheckTriangulation() == false)
                         {
                             throw new Exception("\nTriangulation Invalid");
                         }
                         Console.WriteLine("Triangulation Valid");
                     }

                     // Dispose DTM To Release Allocated Unmanaged Memory

                     datDTM.Dispose();
                }
            }
        }
/*
        /// <summary>
        /// 
        /// </summary>
        [Test]
        [Category("DTMTriangulation")]
        public void TriangulateDatFile_Bayview()
        {
            // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test
            datTriangulationDriver("Bayview");
        }
        /// <summary>
        /// 
        /// </summary>
        [Test]
        [Category("DTMTriangulation")]
        public void TriangulateDatFile_Hatch()
        {
            // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test
            datTriangulationDriver("HatchTest");
        }
        /// <summary>
        /// 
        /// </summary>
        [Test]
        [Category("DTMTriangulation")]
        public void TriangulateDatFile_SurvTriangles()
        {
            // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test
            datTriangulationDriver("Survey_Triangles");
        }
 */

        # endregion Geopak DAT tests

        /// <summary>
        /// Run the test and calculate the run time
        /// </summary>
        private static Double RunAndTimeTheTest(String TestFile)
            {

            Console.WriteLine("-----------------");
            DateTime StartTime;
            DateTime EndTime;

            //Determine if the datafile has an XYZ or DAT extension
            String TheTestFile = TestFile.ToUpper();
            //            string TinTestFile=TheTestFile + ".tin" ;
            switch (TheTestFile.EndsWith("DAT"))
                {
                case true:
                    // Runs the DAT test and computes the elapsed time
                    Console.WriteLine("Reading Geopak DAT file {0}", TheTestFile);
                    StartTime = DateTime.Now;
                    DTM DatDTM = DTM.CreateFromGeopakDatFile(TestFile);

                    // Triangulate Imported Geopak Dat File
                    Console.WriteLine("Triangulating Geopak DAT DTM");
                    DatDTM.Triangulate();

                    EndTime = DateTime.Now;

                    // Check The Triangulation - Unit Test Checking Only
                    Console.WriteLine("Checking Triangulation");
                    if (DatDTM.CheckTriangulation() == false)
                        {
                        throw new Exception("\nTriangulation Invalid");
                        }
                    Console.WriteLine("Triangulation Valid");

                    // Export DTM To Geopak Tin
                    string TinFileName;
                    TinFileName = Path.ChangeExtension(TestFile, ".tin");
                    Console.WriteLine("Exporting To Geopak Tin File {0}", TinFileName);
                    DatDTM.SaveAsGeopakTinFile(TinFileName);

                    DatDTM.Dispose();
                    DatDTM = null;
                    break;

                case false:
                    // Runs the XYZ test and computes the elapsed time
                    Console.WriteLine("Reading XYZ file {0}", TestFile);
                    StartTime = DateTime.Now;

                    DTM XyzDTM = DTM.CreateFromXyzFile(TestFile);

                    Console.WriteLine("Reading XYZ file {0} Completed", TestFile);

                    // Triangulate Imported XYZ File
                    Console.WriteLine("Triangulating XYZ DTM");
                    XyzDTM.Triangulate();

                    EndTime = DateTime.Now;

                    // Check The Triangulation - Unit Test Checking Only
                    Console.WriteLine("Checking Triangulation");
                    if (XyzDTM.CheckTriangulation() == false)
                        {
                        throw new Exception("\nTriangulation Invalid");
                        }
                    Console.WriteLine("Triangulation Valid");

                    //num = 0;
                    //numPoints = 0;
                    //DateTime dt1 = DateTime.Now;
                    //ContoursBrowsingDelegate hdl3P = new ContoursBrowsingDelegate(processContours);
                    //ContoursBrowsingCriteria contoursCriteria = new ContoursBrowsingCriteria(1);
                    //XyzDTM.BrowseContours(contoursCriteria, hdl3P, null);

                    //DateTime dt2 = DateTime.Now;

                    //// Compute time difference
                    //TimeSpan dt3 = dt2 - dt1;

                    //// Set up baseline time comparison - if no baseline numbers exists fill the table
                    //double timeMilliseconds = dt3.TotalMilliseconds;

                    XyzDTM.Dispose();
                    XyzDTM = null;
                    break;

                default:
                    throw new Exception("UNKNOWN FILE EXTENSION: The file extension on the test file \"" + TestFile + "\" is not DAT or XYZ");
                }
            TimeSpan ElapsedTime = EndTime - StartTime;
            System.GC.Collect();
            return ElapsedTime.TotalSeconds;
            }
        static bool processContours(DTMFeatureId id, BGEO.DPoint3d[] tPoint, BGEO.DPoint3d direction, System.Object oArg)
            {
            num++;
            numPoints += tPoint.Length;
            return true;
            }

        }

    }