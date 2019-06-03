using System;
using System.Collections;
using System.IO;
using SD = System.Data;
using SDO = System.Data.OleDb;
using BGEO = Bentley.GeometryNET;
using NUnit.Framework;

namespace Bentley.TerrainModelNET.NUnit
    {
    using Bentley.TerrainModelNET ;

    /// <summary>
    /// DTM Unit testing.
    /// </summary>
    [TestFixture]
    public class DTMLandXMLTest : DTMUNitTest 
        {
        #region Private Fields

        private SDO.OleDbConnection _dbConnection;

        private System.IFormatProvider _formatProvider = System.Globalization.NumberFormatInfo.CurrentInfo;

        #endregion

        #region Constructor

        /// <summary>
        /// Constructor
        /// </summary>
        public DTMLandXMLTest ()
            {
            }

        /// <summary>
        /// Destructure
        /// </summary>
        ~DTMLandXMLTest()
        {
            if(_dbConnection != null)
                _dbConnection.Dispose();
        }
        #endregion

        /// <summary>
        /// 
        /// </summary>
        [TestFixtureSetUp]
        public void Setup()
            {
            _dbConnection = new SDO.OleDbConnection("Jet OLEDB:Global Partial Bulk Ops=2;Jet OLEDB:Registry Path=;Jet OLEDB:Database Locking Mode=1;Jet OLEDB:Database Password=;Data Source=" + Helper.GetTestDataLocation () +"Bentley.Civil.Dtm.Light.NUnit.dll\\DTM.mdb;Password=;Jet OLEDB:Engine Type=5;Jet OLEDB:Global Bulk Transactions=1;Provider=Microsoft.Jet.OLEDB.4.0;Jet OLEDB:System database=;Jet OLEDB:SFP=False;Extended Properties=;Mode=Share Deny None;Jet OLEDB:New Database Password=;Jet OLEDB:Create System Database=False;Jet OLEDB:Don't Copy Locale on Compact=False;Jet OLEDB:Compact Without Replica Repair=False;User ID=Admin;Jet OLEDB:Encrypt Database=False");
            }

        /// <summary>
        /// 
        /// </summary>
        [TestFixtureTearDown]
        public void TearDown()
        {
            _dbConnection.Dispose();
            _dbConnection = null;
        }

        #region Main Bulk Testing

        /// <summary>
        /// Test exporting and importing LandXML
        /// </summary>
        [Test]
        [Category("LandXML Import/Export Tests")]
        public void LandXMLExportTests()
            {
            IgnoreFirebug (); // If DTM_LEVEL = FIREBUG -> Ignore the test

            // Read the input from the database
            if (_dbConnection.State == SD.ConnectionState.Closed)
                _dbConnection.Open();
            SDO.OleDbDataAdapter adapter = new SDO.OleDbDataAdapter("Select * from Build", _dbConnection);
            //SD.DataTable dataset = new SD.DataTable();
            //dataset.Locale = System.Globalization.CultureInfo.InvariantCulture;
            //adapter.Fill(dataset);
            SD.DataSet ds = new SD.DataSet();
            adapter.Fill(ds, "Build");
            this.UpdateCommandAuto(adapter);
            SD.DataRowCollection rowCollection = ds.Tables["Build"].Rows;

            //
            // Import LandXML File To DTM

            string pathXML = Helper.GetTestDataLocation () +"Bentley.Civil.Dtm.Light.NUnit.dll\\Build\\surf.xml";
            Bentley.Civil.LandXml.LandXml Xl = new Bentley.Civil.LandXml.LandXml();
            Console.WriteLine("Importing LandXML Surfaces To DTM ** pathXML = {0}", pathXML);
            string[] surfs = Xl.ListSurfaces(pathXML);
            if (surfs.Length > 0)
            {
                Console.WriteLine("surfs.length = {0}",surfs.Length);
                DTM[] YY = Xl.ImportNamedSurfaces(pathXML, new string[] { surfs[0] });
                Console.WriteLine("Importing LandXML Surfaces To DTM Completed");

                for (int n = 0; n < YY.Length ; ++n)
                {
                    Console.WriteLine("Checking Triangulation Surface {0}",n);
                    if (YY[n].CheckTriangulation() == false)
                    {
                        throw new Exception("\nTriangulation Invalid");
                    }
                    Console.WriteLine("Triangulation Valid");
                }

             }

            // Process all the input rows for each file name
            foreach (SD.DataRow row in rowCollection)
                {
                // Get the file name
                // String fileName = row.ItemArray.["Filename"].ToString();

                String fileName = row["Filename"].ToString();
                string path = Helper.GetTestDataLocation () +"Bentley.Civil.Dtm.Light.NUnit.dll\\Build\\";
                path = path.Replace("\\\\", "\\");
                string fullPath = Path.GetFullPath(Path.Combine(path, fileName));

                // Import the DTM

                Console.WriteLine("Importing DTM ** fullPath = {0}", fullPath);
                DTM dtm = DTM.CreateFromFile(fullPath);
                Assert.IsNotNull(dtm, "Unable to Import TIN " + fullPath);  // Redundant As It Would Threw An Exception

                // Check The Triangulation - Unit Test And Development Checking Only

                Console.WriteLine("Checking Triangulation");
                if (dtm.CheckTriangulation() == false)
                {
                    throw new Exception("\nTriangulation Invalid");
                }
                Console.WriteLine("Triangulation Valid");

                // Set LandXML Filename 

                string XMLpath = fullPath.Replace(".dat", ".xml");
                XMLpath = XMLpath.Replace(".tin", ".xml");
                Bentley.Civil.LandXml.LandXml l = new Bentley.Civil.LandXml.LandXml();

                // Export DTM To XML File

                Console.WriteLine("Exporting  DTM To LandXML Surface ** XMLpath = {0}", XMLpath);
                l.ExportDTM(dtm, XMLpath, "re", "fd", "1.0", true);
                Console.WriteLine("Exporting DTM To LandXML Completed");
                
                // Import LandXML File To DTM

                Console.WriteLine("Importing LandXML Surfaces To DTM ** XMLpath = {0}", XMLpath);
                DTM[] l2 = l.ImportSurfaces(XMLpath);
                Console.WriteLine("Importing LandXML Surfaces To DTM Completed");
                dtm = l2[0];

                // Check The Triangulation - Unit Test And Development Checking Only

                Console.WriteLine("Checking Triangulation");
                if (dtm.CheckTriangulation() == false)
                {
                    throw new Exception("\nTriangulation Invalid");
                }
                Console.WriteLine("Triangulation Valid");

                // Build a new tin mark time
                DateTime dt1 = DateTime.Now;
                dtm.Triangulate();                      // Redundant Not Necessary
                DateTime dt2 = DateTime.Now;

                // Compute time difference
                TimeSpan dt3 = dt2 - dt1;

                // Test the basic properties
                BasicProperties(dtm);


                // Set up baseline time comparison - if no baseline numbers exists fill the table
                String time = row["RunTime"].ToString();
                if (time != String.Empty)
                    {
                    double timeMilliseconds = System.Convert.ToDouble(time);
                    // Consider a 5% increase in time a failure
                    //                    Assert.IsTrue ((dt3.TotalMilliseconds < (timeMilliseconds + 1) * 1.05), "Build time failure");
                    }
                else
                    {
                    row[1] = dt3.TotalMilliseconds;
                    }
                // set up baseline triangles test
                String triangles = row["NumberOfTriangles"].ToString();
                if (triangles != String.Empty)
                    {
                    Assert.AreEqual(System.Convert.ToInt32(triangles), dtm.TrianglesCount, "Triangle count failure");
                    }
                else
                    {
                    row[2] = dtm.TrianglesCount;
                    }
                // set up baseline volume test
                double vol = PlanarVolume(dtm);
                String volume = row["VolumeToZero"].ToString();
                if (volume != String.Empty)
                    {
//                    Assert.AreEqual(vol, System.Convert.ToDouble(volume), 0.0001, "Volume failure");
                    }
                else
                    {
                    row[3] = vol;
                    }
                // Run optional tests
                if (System.Convert.ToBoolean(row["IsTestThemes"].ToString()))
                    {
                    Themes(dtm);
                    }
                if (System.Convert.ToBoolean(row["IsTestTrace"].ToString()))
                    {
                    AscentTrace(dtm);
                    DescentTrace(dtm);
                    }
                if (System.Convert.ToBoolean(row["IsTestBrowseFeatures"].ToString()))
                    {
                    BrowseFeatures(dtm);
                    }
                }
            // Update the database if any baseline data needed to be populated
            adapter.Update(ds, "Build");
            //			datatable.AcceptChanges();
            _dbConnection.Close();
            }

        /// <summary>
        /// Test exporting and importing LandXML
        /// </summary>
        [Test]
        [Category("LandXML Import Tests")]
        public void LandXMLImportTests()
            {
             // Check Dtm Test Level
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

            // Read the input from the database
            if (_dbConnection.State == SD.ConnectionState.Closed)
                _dbConnection.Open();
            SDO.OleDbDataAdapter adapter = new SDO.OleDbDataAdapter("Select * from LandXML", _dbConnection);
            //SD.DataTable dataset = new SD.DataTable();
            //dataset.Locale = System.Globalization.CultureInfo.InvariantCulture;
            //adapter.Fill(dataset);
            SD.DataSet ds = new SD.DataSet();
            adapter.Fill(ds, "LandXML");
            this.UpdateCommandAuto(adapter);
            SD.DataRowCollection rowCollection = ds.Tables["LandXML"].Rows;

            // Process all the input rows for each file name
            foreach (SD.DataRow row in rowCollection)
                {
                // Get the file name
                //				String fileName = row.ItemArray.["Filename"].ToString();
                String fileName = row["Filename"].ToString();
                string path = Helper.GetTestDataLocation () +"Bentley.Civil.Dtm.NUnit.dll\\LandXML\\";
                path = path.Replace("\\\\", "\\");
                string fullPath = Path.GetFullPath(Path.Combine(path, fileName));

                // Load the DTM
                Bentley.Civil.LandXml.LandXml l = new Bentley.Civil.LandXml.LandXml();

                Console.WriteLine("Importing XML Surfaces To DTM [] ** fullPath = {0}", fullPath);

                DTM[] l2 = l.ImportSurfaces(fullPath);

                Console.WriteLine("Importing XML Surfaces To DTM [] Completed");

                DTM dtm = l2[0];

                // Build a new tin mark time
                DateTime dt1 = DateTime.Now;
                dtm.Triangulate();
                DateTime dt2 = DateTime.Now;

                // Compute time difference
                TimeSpan dt3 = dt2 - dt1;

                // Test the basic properties
                BasicProperties(dtm);


                // Set up baseline time comparison - if no baseline numbers exists fill the table
                String time = row["RunTime"].ToString();
                if (time != String.Empty)
                    {
                    double timeMilliseconds = System.Convert.ToDouble(time);
                    // Consider a 5% increase in time a failure
                    //                    Assert.IsTrue ((dt3.TotalMilliseconds < (timeMilliseconds + 1) * 1.05), "Build time failure");
                    }
                else
                    {
                    row[1] = dt3.TotalMilliseconds;
                    }
                // set up baseline triangles test
                String triangles = row["NumberOfTriangles"].ToString();
                if (triangles != String.Empty)
                    {
                    Assert.AreEqual(System.Convert.ToInt32(triangles), dtm.TrianglesCount, "Triangle count failure");
                    }
                else
                    {
                    row[2] = dtm.TrianglesCount;
                    }
                // set up baseline volume test
                double vol = PlanarVolume(dtm);
                String volume = row["VolumeToZero"].ToString();
                if (volume != String.Empty)
                    {
                    //                    Assert.AreEqual(vol, System.Convert.ToDouble(volume), 0.0001, "Volume failure");
                    }
                else
                    {
                    row[3] = vol;
                    }
                // Run optional tests
                if (System.Convert.ToBoolean(row["IsTestThemes"].ToString()))
                    {
                    Themes(dtm);
                    }
                if (System.Convert.ToBoolean(row["IsTestTrace"].ToString()))
                    {
                    AscentTrace(dtm);
                    DescentTrace(dtm);
                    }
                if (System.Convert.ToBoolean(row["IsTestBrowseFeatures"].ToString()))
                    {
                    BrowseFeatures(dtm);
                    }
                }
            // Update the database if any baseline data needed to be populated
            adapter.Update(ds, "LandXML");
            //			datatable.AcceptChanges();
            _dbConnection.Close();
            }
        #endregion

            /// <summary>
            ///  Export bcLIB DTM and Geopak Tin Files To XML
            /// </summary>
            /// <category>LandXML Import/Export Tests</category>
            [Test]
            [Category("LandXML Import/Export Tests")]
            public void ExportToLandXML()
            {

                // Check Dtm Test Level

                Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
                IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

                //  Set Path To Folder Holding DTM and Tin Files To Export

                String DataPath = Helper.GetTestDataLocation () +@"Bentley.Civil.Dtm.NUnit.dll\DTMLandXMLTest\DTMLandXMLExportTest\";

                // Get Test Data Directories
                DirectoryInfo testFolder = new DirectoryInfo(DataPath);
                if (!testFolder.Exists)
                {
                    throw new DirectoryNotFoundException("DTM XYZ Triangulation Nunit Test Data Header Folder Not Found ");
                }

                // Scan And Export All The DTM and Tin Files In The Folder

                foreach (System.IO.FileInfo nextFile in testFolder.GetFiles())
                {
                    if (string.Compare(nextFile.Extension, ".tin") == 0 || string.Compare(nextFile.Extension, ".dtm") == 0)
                    {
                        Console.WriteLine("Export File = {0}", nextFile.Name);

                        // Import The DTM File

                        DateTime impStartTime = DateTime.Now;
                        DTM dtm = DTM.CreateFromFile(DataPath + nextFile.Name);
                        DateTime impEndTime = DateTime.Now;
                        TimeSpan impTime = impEndTime - impStartTime;
                        Console.WriteLine("Import Time was {0} seconds.", impTime.TotalSeconds);

                        // Check The Triangulation - Unit Test Checking Only

                        Console.WriteLine("Checking Triangulation");
                        DateTime chkStartTime = DateTime.Now;
                        if (dtm.CheckTriangulation() == false)
                        {
                            throw new Exception("\nTriangulation Invalid");
                        }
                        Console.WriteLine("Triangulation Valid");
                        DateTime chkEndTime = DateTime.Now;
                        TimeSpan chkTime = chkEndTime - chkStartTime;
                        Console.WriteLine("Triangulation Validation Time was {0} seconds.", chkTime.TotalSeconds);

                        // Set LandXML Filename 

                        string XMLpath = (DataPath + nextFile.Name).Replace(".dtm", ".xml");
                        XMLpath = XMLpath.Replace(".tin", ".xml");
                        Bentley.Civil.LandXml.LandXml landXML = new Bentley.Civil.LandXml.LandXml();

                        // Export DTM To XML File

                        Console.WriteLine("Exporting To LandXML File ** XMLpath = {0}", XMLpath);
                        DateTime expStartTime = DateTime.Now;
                        landXML.ExportDTM(dtm, XMLpath, "re", "fd", "1.0", true);
                        DateTime expEndTime = DateTime.Now;
                        TimeSpan expTime = expEndTime - expStartTime;
                        Console.WriteLine("Export Time was {0} seconds.", expTime.TotalSeconds);
                        Console.WriteLine("Exporting DTM To LandXML Completed");

                        // Dispose DTM To Release Allocated Unmanaged Memory

                        dtm.Dispose();
                    }
                }
            }

            /// <summary>
            ///  Import bcLIB DTM from XML
            /// </summary>
            /// <category>LandXML Import/Export Tests</category>
            [Test]
            [Category("LandXML Import/Export Tests")]
            public void ImportFromLandXML()
            {

                // Check Dtm Test Level

                Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
                IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

                //  Set Path To Folder Holding DTM and Tin Files To Export

                String DataPath = Helper.GetTestDataLocation () +@"Bentley.Civil.Dtm.NUnit.dll\DTMLandXMLTest\DTMLandXMLImportTest\";

                // Get Test Data Directories
                DirectoryInfo testFolder = new DirectoryInfo(DataPath);
                if (!testFolder.Exists)
                {
                    throw new DirectoryNotFoundException("DTM XYZ Triangulation Nunit Test Data Header Folder Not Found ");
                }

                // Scan And Export All The DTM and Tin Files In The Folder

                foreach (System.IO.FileInfo nextFile in testFolder.GetFiles())
                {
                    if (string.Compare(nextFile.Extension, ".xml") == 0)
                    {
                        Console.WriteLine("XML Import File = {0}", nextFile.Name);

                        // Create XML Surface

                        Bentley.Civil.LandXml.LandXml xmlSurface = new Bentley.Civil.LandXml.LandXml();
 
                        // Import Each Surface To A DTM

                        Console.WriteLine("Importing XML To DTM");
                        DateTime impStartTime = DateTime.Now;
                        DTM[] dtmSurfaces = xmlSurface.ImportSurfaces(DataPath+nextFile.Name);
                        DateTime impEndTime = DateTime.Now;
                        TimeSpan impTime = impEndTime - impStartTime;
                        Console.WriteLine("Import Time was {0} seconds", impTime.TotalSeconds);
                        Console.WriteLine("Importing XML To DTM Completed");

                        // Check The Triangulation - Unit Test And Development Checking Only

                        for (int n = 0 ; n < dtmSurfaces.Length ; ++n)
                        {

                            Console.WriteLine("Checking Triangulation Of Surface {0}",n);
                            if (dtmSurfaces[n].CheckTriangulation() == false)
                            {
                                throw new Exception("\nTriangulation Invalid");
                            }
                            Console.WriteLine("Triangulation Valid");

                        }

                        // Dispose DTM

                        for (int n = 0; n < dtmSurfaces.Length; ++n)
                        {
                            dtmSurfaces[n].Dispose();
                        }
                    }
                }
            }

            /// <summary>
            ///  Import bcLIB DTM from XML
            /// </summary>
            /// <category>LandXML Import/Export Tests</category>
            [Test]
            [Category("LandXML Import/Export Tests")]
            public void MergeAndAppendLandXML()
            {

                // Check Dtm Test Level

                Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
                IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

                string fileName = "D:\\CertData\\CivilPlatform\\MergeAndAppend00\\survey.xml";
                Console.WriteLine("Importing survey.xml");
                Bentley.TerrainModelNET.DTM[] dtms = Bentley.Civil.LandXml.LandXml.GetInstance().ImportSurfaces(fileName);


                Bentley.TerrainModelNET.DTM dtm1 = dtms[0];
  //              dtm1.RollBackOption = RollBackOption.Normal;
                dtm1.Triangulate();

                //Check Triangulation - Unit Test Checking Only

                Console.WriteLine("Checking Dtm1 Triangulation");
                if (dtm1.CheckTriangulation() == false)
                {
                    throw new Exception("Dtm1 Triangulation Invalid");
                }
                Console.WriteLine("Dtm1 Triangulation Valid");


                fileName = "D:\\CertData\\CivilPlatform\\MergeAndAppend00\\tree.xml";
                Console.WriteLine("Importing tree.xml");
                dtms = Bentley.Civil.LandXml.LandXml.GetInstance().ImportSurfaces(fileName);

                Bentley.TerrainModelNET.DTM dtm2 = dtms[0];
//                dtm2.RollBackOption = RollBackOption.Normal;
                dtm2.Triangulate();

                //Check Triangulation - Unit Test Checking Only

                Console.WriteLine("Checking Dtm2 Triangulation");
                if (dtm2.CheckTriangulation() == false)
                {
                    throw new Exception("Dtm2 Triangulation Invalid");
                }
                Console.WriteLine("Dtm2 Triangulation Valid");

                fileName = "D:\\CertData\\CivilPlatform\\MergeAndAppend00\\opt1 east.xml";
                Console.WriteLine("Importing east.xml");
                dtms = Bentley.Civil.LandXml.LandXml.GetInstance().ImportSurfaces(fileName);

                Bentley.TerrainModelNET.DTM dtm3 = dtms[0];
 //               dtm3.RollBackOption = RollBackOption.Normal;
                dtm3.Triangulate();

                //Check Triangulation - Unit Test Checking Only

                Console.WriteLine("Checking Dtm3 Triangulation");
                if (dtm3.CheckTriangulation() == false)
                {
                    throw new Exception("Dtm3 Triangulation Invalid");
                }
                Console.WriteLine("Dtm3 Triangulation Valid");

                fileName = "D:\\CertData\\CivilPlatform\\MergeAndAppend00\\opt1 west.xml";
                Console.WriteLine("Importing west.xml");
                dtms = Bentley.Civil.LandXml.LandXml.GetInstance().ImportSurfaces(fileName);

                Bentley.TerrainModelNET.DTM dtm4 = dtms[0];
 //               dtm4.RollBackOption = RollBackOption.Normal;
                dtm4.Triangulate();

                Console.WriteLine("Checking Dtm4 Triangulation");
                if (dtm4.CheckTriangulation() == false)
                {
                    throw new Exception("Dtm4 Triangulation Invalid");
                }
                Console.WriteLine("Dtm4 Triangulation Valid");

                dtm1 = dtm1.Clone();
 //               dtm1.RollBackOption = RollBackOption.None;

                dtm2 = dtm2.Clone();
                dtm1.Merge(dtm2);

                Console.WriteLine("Checking Merged DTM1 Triangulation");
                if (dtm4.CheckTriangulation() == false)
                {
                    throw new Exception("Merged DTM1 Triangulation Invalid");
                }
                Console.WriteLine("Merged DTM1 Triangulation Valid");


                dtm2.Dispose();

                dtm3 = dtm3.Clone();
                dtm1.Append(dtm3);

                Console.WriteLine("Checking Append DTM1 Triangulation");
                if (dtm4.CheckTriangulation() == false)
                {
                    throw new Exception("Append DTM1 Triangulation Invalid");
                }
                Console.WriteLine("Append DTM1 Triangulation Valid");

                dtm3.Dispose();

                dtm4 = dtm4.Clone();
                dtm1.Append(dtm4);

                Console.WriteLine("Checking Append DTM1 Triangulation");
                if (dtm4.CheckTriangulation() == false)
                {
                    throw new Exception("Append DTM1 Triangulation Invalid");
                }
                Console.WriteLine("Append DTM1 Triangulation Valid");

                dtm4.Dispose();

                dtm1.Save("D:\\CertData\\CivilPlatform\\MergeAndAppend00\\MergeAppend.bcdtm");
                dtm1.Triangulate(); // Crash will happen here…
            }


        #region Public General Purpose DTM Test Utilities

        /// <summary>
        /// 
        /// </summary>
        /// <param name="dtm"></param>
        public void BasicProperties (DTM dtm)
            {
            Assert.IsNotNull (dtm);
            BGEO.DRange3d range3d = dtm.Range3d;
            int nVerticesCount = dtm.VerticesCount;
            int nLinesCount = dtm.LinesCount;
            int nTrianglesCount = dtm.TrianglesCount;
            int nFeaturesCount = dtm.FeaturesCount;
            int nBreakLinesCount = dtm.BreakLinesCount;
            int nVoidsCount = dtm.VoidsCount;
            int nIslandsCount = dtm.IslandsCount;
            int nHolesCount = dtm.HolesCount;
            BGEO.DPoint3d[] hull = dtm.GetBoundary();
            }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="dtm"></param>
        /// <returns></returns>
        public double PlanarVolume (DTM dtm)
        {
            Assert.IsNotNull (dtm);
            VolumeResult volumeResult = dtm.CalculatePrismoidalVolumeToElevation(0.0, new VolumeCriteria());
            return volumeResult.BalanceVolume;
        }

    public BGEO.DPoint3d GetCentroid (BGEO.DPoint3d[] hull)
        {
        BGEO.DPoint3d pt = new BGEO.DPoint3d (0, 0, 0);
        for (int i = 0; i < hull.Length; i++)
            {
            pt.X += hull[i].X;
            pt.Y += hull[i].Y;
            pt.Z += hull[i].Z;
            }
        pt.X /= hull.Length;
        pt.Y /= hull.Length;
        pt.Z /= hull.Length;
        return pt;
        }
    /// <summary>
        /// 
        /// </summary>
        /// <param name="nDtm"></param>
        /// <returns></returns>
        /// 

        bool browseTrace(DTMDynamicFeatureType featureType, BGEO.DPoint3d[] points, object userArg)
        {
            //            Console.WriteLine("FeatureType = {0} **** Number Of Points = {1}", featureType, points.Length);
            return true;
        }

        public double AscentTrace (DTM nDtm)
            {
            BGEO.DPoint3d centroid = GetCentroid(nDtm.GetBoundary());
            DynamicFeaturesBrowsingDelegate traceBrowsingDelegate = new DynamicFeaturesBrowsingDelegate(browseTrace);
            nDtm.TraceMaximumAscent(0.1, centroid, traceBrowsingDelegate, null);
            // ToDo...Standalone
            //if (tAscentTrace != null)
            //    {
            //    BCG.LineString elm = new BCG.LineString (tAscentTrace);
            //    return elm.Length;
            //    }
            return 0;
            }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="nDtm"></param>
        /// <returns></returns>
        public double DescentTrace (DTM nDtm)
            {
// ToDo...Standalone            
            BGEO.DPoint3d centroid = GetCentroid (nDtm.GetBoundary ());
            DynamicFeaturesBrowsingDelegate traceBrowsingDelegate = new DynamicFeaturesBrowsingDelegate(browseTrace);
            nDtm.TraceMaximumDescent(0.1, centroid, traceBrowsingDelegate, null);
            //BCG.LineString elm = new BCG.LineString (tDescentTrace);
            //return elm.Length;
            return 0;
            }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="dtm"></param>
        public void Themes (DTM dtm)
            {
            // Test theme
            Bentley.GeometryNET.DInterval1d[] tRange = new Bentley.GeometryNET.DInterval1d[10];
            double deltaZ = dtm.Range3d.high.Z - dtm.Range3d.low.Z;
            int i = 0;
            for (i = 0; i < 10; i++)
                {
                tRange[i] = new Bentley.GeometryNET.DInterval1d (dtm.Range3d.low.Z + i * deltaZ, dtm.Range3d.low.Z + (i + 1) * deltaZ);
                }
            DynamicFeaturesBrowsingDelegate hdl2P = new DynamicFeaturesBrowsingDelegate (processPointsFeatures);
            ElevationAnalyzingBrowsingCriteria elevationCriteria = new ElevationAnalyzingBrowsingCriteria();
            elevationCriteria.PolygonizedResult = true;
            elevationCriteria.DoubleRange = tRange;
            dtm.AnalyzeElevation (elevationCriteria, hdl2P, null);
            elevationCriteria.PolygonizedResult = false;
            dtm.AnalyzeElevation (elevationCriteria, hdl2P, null);

            for (i = 0; i < 10; i++)
                {
                tRange[i] = new Bentley.GeometryNET.DInterval1d (i * 5, (i + 1) * 5);
                }

            SlopeAnalyzingBrowsingCriteria slopeCriteria = new SlopeAnalyzingBrowsingCriteria();
            slopeCriteria.PolygonizedResult = true;
            slopeCriteria.DoubleRange = tRange;
            dtm.AnalyzeSlope(slopeCriteria, hdl2P, null);
            slopeCriteria.PolygonizedResult = false;
            dtm.AnalyzeSlope(slopeCriteria, hdl2P, null);

            for (i = 0; i < 10; i++)
                {
                tRange[i] = new Bentley.GeometryNET.DInterval1d (i * 0.3, (i + 1) * 0.3);
                }
            AspectAnalyzingBrowsingCriteria aspectCriteria = new AspectAnalyzingBrowsingCriteria();
            aspectCriteria.PolygonizedResult = true;
            aspectCriteria.DoubleRange = tRange;
            dtm.AnalyzeAspect(aspectCriteria, hdl2P, null);
            aspectCriteria.PolygonizedResult = false;
            dtm.AnalyzeAspect(aspectCriteria, hdl2P, null);
            }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="dtm"></param>
        public void BrowseFeatures (DTM dtm)
            {
            DynamicFeaturesBrowsingDelegate hdl2P = new DynamicFeaturesBrowsingDelegate (processPointsFeatures);
            ArrayList tTriangle = new ArrayList ();

            DynamicFeaturesBrowsingCriteria criteria = new DynamicFeaturesBrowsingCriteria ();

            // Triangle  features
            dtm.BrowseDynamicFeatures (criteria, DTMDynamicFeatureType.Triangle, hdl2P, tTriangle);
            // Assert.AreEqual(dtm.TrianglesCount, tTriangle.Count);

            // Triangle Edge feature
            dtm.BrowseDynamicFeatures(criteria, DTMDynamicFeatureType.TriangleEdge, hdl2P, tTriangle);

            RidgeLinesBrowsingCriteria ridgeLineCriteria = new RidgeLinesBrowsingCriteria();
            dtm.BrowseRidgeLines(ridgeLineCriteria, hdl2P, null);
            SumpLinesBrowsingCriteria sumpLinesCriteria = new SumpLinesBrowsingCriteria();
            dtm.BrowseSumpLines(sumpLinesCriteria, hdl2P, null);

            SinglePointFeaturesBrowsingDelegate hdlP = new SinglePointFeaturesBrowsingDelegate (processPointFeatures);
            HighPointsBrowsingCriteria highPointsCriteria = new HighPointsBrowsingCriteria();
            dtm.BrowseHighPoints(highPointsCriteria, hdlP, null);
            LowPointsBrowsingCriteria lowPointsCriteria = new LowPointsBrowsingCriteria();
            dtm.BrowseLowPoints(lowPointsCriteria, 1, hdlP, null);

            ContoursBrowsingDelegate hdl3P = new ContoursBrowsingDelegate (processContours);
            ContoursBrowsingCriteria contoursCriteria = new ContoursBrowsingCriteria(1);
            dtm.BrowseContours (contoursCriteria, hdl3P, null);

            }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="dtm"></param>
        public void VolumeRange (DTM dtm)
            {
            // Test theme
            Bentley.GeometryNET.DInterval1d[] tRange = new Bentley.GeometryNET.DInterval1d[10];
            double deltaZ = dtm.Range3d.high.Z - dtm.Range3d.low.Z;
            int i = 0;
            for (i = 0; i < 10; i++)
                {
                tRange[i] = new Bentley.GeometryNET.DInterval1d (dtm.Range3d.low.Z + i * deltaZ, dtm.Range3d.low.Z + (i + 1) * deltaZ);
                }
            // TO BE COMPLETED
            }

        #endregion

        #region Private Test Utilities

        int processPointFeatures (DTMDynamicFeatureType featureType, BGEO.DPoint3d point)
            {
            return 0;
            }

        int processPointsFeatures (DTMDynamicFeatureType featureType, BGEO.DPoint3d[] tPoint)
            {
            return 0;
            }
        int processContours (BGEO.DPoint3d[] tPoint, BGEO.DPoint3d direction)
            {
            return 0;
            }
        int processSlopeIndicator (bool major, BGEO.DPoint3d startPoint, BGEO.DPoint3d endPoint)
            {
            return 0;
            }
        bool processContours (BGEO.DPoint3d[] tPoint, BGEO.DPoint3d direction, System.Object oArg)
            {
            return true;
            }

        bool processSlopeIndicator (bool major, BGEO.DPoint3d startPoint, BGEO.DPoint3d endPoint, System.Object oArg)
            {
            return true;
            }

        bool processPointFeatures (DTMDynamicFeatureType featureType, BGEO.DPoint3d point, System.Object oArg)
            {
            return true;
            }

        bool processPointsFeatures (DTMDynamicFeatureInfo info,  System.Object oArg)
            {
            if (oArg != null)
                {
                ArrayList al = oArg as ArrayList;
                al.Add (tPoint);
                }
            return true;
            }

        private void UpdateCommandAuto (SDO.OleDbDataAdapter adapter)
            {
            SDO.OleDbParameter workParam = null;

            //  This does not seems to be working when we place WHERE into our command. I can figure it later
            string query = "UPDATE Build SET RunTime  = ?, NumberOfTriangles = ?, VolumeToZero = ?  WHERE Filename = ? ";
            adapter.UpdateCommand = new SDO.OleDbCommand (query, _dbConnection);

            workParam = adapter.UpdateCommand.Parameters.Add ("@RunTime", SDO.OleDbType.VarChar, 50, "RunTime");
            workParam.SourceVersion = SD.DataRowVersion.Current;

            workParam = adapter.UpdateCommand.Parameters.Add ("@NumberOfTriangles", SDO.OleDbType.VarChar, 50, "NumberOfTriangles");
            workParam.SourceVersion = SD.DataRowVersion.Current;

            workParam = adapter.UpdateCommand.Parameters.Add ("@VolumeToZero", SDO.OleDbType.VarChar, 50, "VolumeToZero");
            workParam.SourceVersion = SD.DataRowVersion.Current;

            workParam = adapter.UpdateCommand.Parameters.Add ("@Filename", SDO.OleDbType.VarChar, 50);
            workParam.SourceColumn = "Filename";
            workParam.SourceVersion = SD.DataRowVersion.Original;
            }
        #endregion
        }
    }

