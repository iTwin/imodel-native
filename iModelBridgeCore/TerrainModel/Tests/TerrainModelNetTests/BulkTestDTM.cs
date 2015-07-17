using System;
using System.Collections;
using System.IO;
using SD = System.Data;
using SDO = System.Data.OleDb;
using BGEO = Bentley.GeometryNET;
using NUnit.Framework;
using Bentley.TerrainModelNET;

namespace Bentley.TerrainModelNET.NUnit
{
    /// <summary>
    /// DTM Unit testing
    /// </summary>
    [TestFixture]
    public class BulkTestDTM : DTMUNitTest
    {
        #region Private Fields

        private SDO.OleDbConnection _dbConnection;

        private System.IFormatProvider _formatProvider = System.Globalization.NumberFormatInfo.CurrentInfo;

        #endregion

        #region Constructor
        /// <summary>
        /// Constructor
        /// </summary>
        public BulkTestDTM()
        {
        }

        /// <summary>
        /// Destructure
        /// </summary>
        ~BulkTestDTM()
        {
            if (_dbConnection != null)
                _dbConnection.Dispose();
        }
        #endregion
        /// <summary>
        /// Constructor
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
        /// Test reading and building dtms
        /// </summary>
        [Test]
        [Category("Build Test")]
        public void BuildTests()
        {
            //            IgnoreFirebug (); // If DTM_LEVEL = FIREBUG -> Ignore the test

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

            // Process all the input rows for each file name
            foreach (SD.DataRow row in rowCollection)
            {
                // Get the file name
                //				String fileName = row.ItemArray.["Filename"].ToString();
                String fileName = row["Filename"].ToString();
                string path = Helper.GetTestDataLocation () +"Bentley.Civil.Dtm.Light.NUnit.dll\\Build\\";
                string fullPath = Path.GetFullPath(Path.Combine(path, fileName));

                // Load the DTM
                DTM dtm = DTM.CreateFromFile(fullPath);
                Assert.IsNotNull(dtm, "Unable to Build TIN " + fullPath);

                // Test the basic properties
                BasicProperties(dtm);

                // Build a new tin mark time
                DateTime dt1 = DateTime.Now;
                dtm.Triangulate();
                DateTime dt2 = DateTime.Now;

                // Compute time difference
                TimeSpan dt3 = dt2 - dt1;

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
                    Assert.AreEqual(vol, System.Convert.ToDouble(volume), 0.0001, "Volume failure");
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

                dtm.Dispose();
            }
            // Update the database if any baseline data needed to be populated
            adapter.Update(ds, "Build");
            //			datatable.AcceptChanges();
            _dbConnection.Close();
        }
        /*
                /// <summary>
                /// Test Merging tins
                /// </summary>
                [Test]
                [Category ("Merge Test")]
                public void MergeTests ()
                    {
                     IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test
                    // Read the input from the database
                    if (_dbConnection.State == SD.ConnectionState.Closed)
                        _dbConnection.Open ();
                    SDO.OleDbDataAdapter adapter = new SDO.OleDbDataAdapter ("Select * from Merge", _dbConnection);
                    //SD.DataTable dataset = new SD.DataTable();
                    //dataset.Locale = System.Globalization.CultureInfo.InvariantCulture;
                    //adapter.Fill(dataset);
                    SD.DataSet ds = new SD.DataSet ();
                    adapter.Fill (ds, "Merge");
                    SD.DataRowCollection rowCollection = ds.Tables["Merge"].Rows;

                    // Process all the input rows for each file name
                    foreach (SD.DataRow row in rowCollection)
                        {
                        // Get the file name1
                        String fileName = row["FileName1"].ToString ();
                        string path = Helper.GetTestDataLocation () +@"Bentley.Civil.Dtm.NUnit.dll\DTMMergeTest\";
                        string fullPath = Path.GetFullPath (Path.Combine (path, fileName));

                        // Load the first DTM
                        DTM dtm = DTM.CreateFromFile (fullPath);
                        Assert.IsNotNull (dtm, "Unable to open TIN");

                        // Get the file name2
                        fileName = row["FileName2"].ToString ();
                        path = Helper.GetTestDataLocation () +@"Bentley.Civil.Dtm.NUnit.dll\DTMMergeTest\";
                        fullPath = Path.GetFullPath (Path.Combine (path, fileName));

                        // Load the second DTM
                        DTM dtm1 = DTM.CreateFromFile (fullPath);
                        Assert.IsNotNull (dtm1, "Unable to open TIN");

                        // Create the merge tin
                        DateTime dt1 = DateTime.Now;
                        dtm.Merge (dtm1);
                        DateTime dt2 = DateTime.Now;

                        // Compute time difference
                        TimeSpan dt3 = dt2 - dt1;

                        // Set up baseline time comparison - if no baseline numbers exists fill the table
                        String time = row["RunTime"].ToString ();
                        if (time != String.Empty)
                            {
                            double timeMilliseconds = System.Convert.ToDouble (time);
                            // Consider a 5% increase in time a failure
                            Assert.IsTrue ((dt3.TotalMilliseconds < (timeMilliseconds + 1) * 1.05), "Merge time failure");
                            }
                        else
                            {
                            row["RunTime"] = dt3.TotalMilliseconds;
                            }
                        // set up baseline triangles test
                        String triangles = row["NumberOfTriangles"].ToString ();
                        if (triangles != String.Empty)
                            {
                            Assert.AreEqual (System.Convert.ToInt32 (triangles), dtm.TrianglesCount, "Triangle count failure");
                            }
                        else
                            {
                            row["NumberOfTriangles"] = dtm.TrianglesCount;
                            }
                        // set up baseling volume test
                        double vol = PlanarVolume (dtm);
                        String volume = row["VolumeToZero"].ToString ();
                        if (volume != String.Empty)
                            {
                            Assert.AreEqual (vol, System.Convert.ToDouble (volume), "Volume failure");
                            }
                        else
                            {
                            row["VolumeToZero"] = vol;
                            }
                        // Run optional tests
                        if (System.Convert.ToBoolean (row["IsTestThemes"].ToString ()))
                            {
                            Themes (dtm);
                            }
                        if (System.Convert.ToBoolean (row["IsTestTrace"].ToString ()))
                            {
                            AscentTrace (dtm);
                            DescentTrace (dtm);
                            }
                        if (System.Convert.ToBoolean (row["IsTestBrowseFeatures"].ToString ()))
                            {
                            BrowseFeatures (dtm);
                            }
                        }
                    // Update the database if any baseline data needed to be populated
                    //			adapter.Update(dataset.DataSet,"Build");
                    //			datatable.AcceptChanges();
                    _dbConnection.Close ();
                    }

         */
        /// <summary>
        /// Test VOLUMES tins
        /// </summary>
        [Test]
        [Category("Volume Test")]
        public void VolumeTests()
        {

            // Check Dtm Test Level

            //            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
            //            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test


            // Read the input from the database
            if (_dbConnection.State == SD.ConnectionState.Closed)
                _dbConnection.Open();
            SDO.OleDbDataAdapter adapter = new SDO.OleDbDataAdapter("Select * from Volume", _dbConnection);
            //SD.DataTable dataset = new SD.DataTable();
            //dataset.Locale = System.Globalization.CultureInfo.InvariantCulture;
            //adapter.Fill(dataset);
            SD.DataSet ds = new SD.DataSet();
            adapter.Fill(ds, "Volume");
            SD.DataRowCollection rowCollection = ds.Tables["Volume"].Rows;

            // Process all the input rows for each file name
            foreach (SD.DataRow row in rowCollection)
            {
                // Get the file name1
                String fileName = row["FileName1"].ToString();
                string path = Helper.GetTestDataLocation () +"Bentley.Civil.Dtm.Light.NUnit.dll\\Volume\\";
                string fullPath = Path.GetFullPath(Path.Combine(path, fileName));

                // Load the first DTM
                Console.WriteLine("Importing From DTM {0}", fullPath);
                DTM dtm = DTM.CreateFromFile(fullPath);
                Assert.IsNotNull(dtm, "Unable to open TIN");

                // Get the file name2
                fileName = row["FileName2"].ToString();
                path = Helper.GetTestDataLocation () +"Bentley.Civil.Dtm.Light.NUnit.dll\\Volume\\";
                fullPath = Path.GetFullPath(Path.Combine(path, fileName));

                // Load the second DTM
                Console.WriteLine("Importing To DTM {0}", fullPath);
                DTM dtm1 = DTM.CreateFromFile(fullPath);
                Assert.IsNotNull(dtm1, "Unable to open TIN");

                // Compute the volumes
                DateTime dt1 = DateTime.Now;
                Console.WriteLine("Calculating Prismoidal Surface Volumes");
                VolumeResult volumeResult = dtm.CalculatePrismoidalVolumeToSurface(dtm1, new VolumeCriteria());
                DateTime dt2 = DateTime.Now;
                Console.WriteLine("cut Volume  = {0} fill Volume = {1} areaVolume = {2}", volumeResult.CutVolume, volumeResult.FillVolume, volumeResult.AreaVolume);

                // 

                // Compute time difference
                TimeSpan dt3 = dt2 - dt1;

                // Set up baseline time comparison - if no baseline numbers exists fill the table
                String time = row["RunTime"].ToString();
                if (time != String.Empty)
                {
                    double timeMilliseconds = System.Convert.ToDouble(time);
                    // Consider a 5% increase in time a failure
                    Assert.IsTrue((dt3.TotalMilliseconds > timeMilliseconds * 1.05), "Volume time failure");
                }
                else
                {
                    row["RunTime"] = dt3.TotalMilliseconds.ToString();
                }
                // set up baseline cut test
                String tcut = row["Cut"].ToString();
                if (tcut != String.Empty)
                {
                    Assert.AreEqual(System.Convert.ToDouble(tcut), volumeResult.CutVolume, "Cut Calculation Difference");
                }
                else
                {
                    row["Cut"] = volumeResult.CutVolume;
                }
                // set up baseline cut test
                String tfill = row["Fill"].ToString();
                if (tfill != String.Empty)
                {
                    Assert.AreEqual(System.Convert.ToDouble(tfill), volumeResult.FillVolume, "Fill Calculation Difference");
                }
                else
                {
                    row["Fill"] = volumeResult.FillVolume;
                }

                // Run optional tests
                if (System.Convert.ToBoolean(row["IsTestVolumeRange"].ToString()))
                {
                    VolumeRange(dtm);
                }
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
            //			adapter.Update(dataset.DataSet,"Build");
            //			datatable.AcceptChanges();
            _dbConnection.Close();
        }


        /// <summary>
        /// Test CLIPPING tins
        /// </summary>
        [Test]
        [Category("Clip Test")]
        public void ClipTests()
        {
            //            IgnoreFirebug (); // If DTM_LEVEL = FIREBUG -> Ignore the test

            // Read the input from the database
            if (_dbConnection.State == SD.ConnectionState.Closed)
                _dbConnection.Open();
            SDO.OleDbDataAdapter adapter = new SDO.OleDbDataAdapter("Select * from Clip", _dbConnection);
            //SD.DataTable dataset = new SD.DataTable();
            //dataset.Locale = System.Globalization.CultureInfo.InvariantCulture;
            //adapter.Fill(dataset);
            SD.DataSet ds = new SD.DataSet();
            adapter.Fill(ds, "Clip");
            SD.DataRowCollection rowCollection = ds.Tables["Clip"].Rows;

            // Process all the input rows for each file name
            foreach (SD.DataRow row in rowCollection)
            {
                // Get the file name1
                String fileName = row["FileName1"].ToString();
                string path = Helper.GetTestDataLocation () +"Bentley.Civil.Dtm.Light.NUnit.dll\\Clip\\";
                string fullPath = Path.GetFullPath(Path.Combine(path, fileName));

                // Load the first DTM
                DTM dtm2 = DTM.CreateFromFile(fullPath);
                Assert.IsNotNull(dtm2, "Unable to open TIN");


                // Get the file name2
                fileName = row["FileName2"].ToString();
                path = Helper.GetTestDataLocation () +"Bentley.Civil.Dtm.Light.NUnit.dll\\Clip\\";
                fullPath = Path.GetFullPath(Path.Combine(path, fileName));

                // Load the second DTM
                DTM dtm1 = DTM.CreateFromFile(fullPath);
                Assert.IsNotNull(dtm1, "Unable to open TIN");

                // Create the clip tin by using the hull of dtm1
                DateTime dt1 = DateTime.Now;
                DTM dtm = dtm2.CloneAndClip(dtm1.GetBoundary(), DTMClippingMethod.Internal);
                DateTime dt2 = DateTime.Now;
                Assert.IsNotNull(dtm2, "Unable to create clip TIN");

                // Compute time difference
                TimeSpan dt3 = dt2 - dt1;

                // *********************************************************************
                // RobC - Added 06/Aug/2009 To Test None Clone Clip 

                Console.WriteLine("Testing Clip Method");
                DTM cloneDTM = dtm2.Clone();
                cloneDTM.Clip(dtm1.GetBoundary(), DTMClippingMethod.External);

                // Check The Triangulation - Unit Test Checking Only

                Console.WriteLine("Checking Cipped DTM Triangulation");
                if (cloneDTM.CheckTriangulation() == false)
                {
                    throw new Exception("\nClipped DTM Triangulation Invalid");
                }
                Console.WriteLine("Clipped DTM Triangulation Valid");

                cloneDTM.Dispose();

                // **********************************************************************

                // Set up baseline time comparison - if no baseline numbers exists fill the table
                String time = row["RunTime"].ToString();
                if (time != String.Empty)
                {
                    double timeMilliseconds = System.Convert.ToDouble(time);
                    // Consider a 5% increase in time a failure
                    Assert.IsTrue((dt3.TotalMilliseconds > timeMilliseconds * 1.05), "Merge time failure");
                }
                else
                {
                    row["RunTime"] = dt3.TotalMilliseconds;
                }
                // set up baseline triangles test
                String triangles = row["NumberOfTriangles"].ToString();
                if (triangles != String.Empty)
                {
                    Assert.AreEqual(System.Convert.ToInt32(triangles), dtm.TrianglesCount, "Triangle count failure");
                }
                else
                {
                    row["NumberOfTriangles"] = dtm.TrianglesCount;
                }
                // set up baseling volume test
                double vol = PlanarVolume(dtm);
                String volume = row["VolumeToZero"].ToString();
                if (volume != String.Empty)
                {
                    Assert.AreEqual(vol, System.Convert.ToDouble(volume), "Volume failure");
                }
                else
                {
                    row["VolumeToZero"] = vol;
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
            //			adapter.Update(dataset.DataSet,"Build");
            //			datatable.AcceptChanges();
            _dbConnection.Close();
        }

        #endregion

        #region Simple Hard coded testing

        /// <summary>
        ///  Simple build and canalized feature test
        /// </summary>
        [Test]
        [Category("Simple build and canalized feature test")]
        public void SimpleBuildByFeatures()
        {
            // Specific test for dtm features
            DTM dtm = new DTM();

            // RobC - Added 05/08/2009 - Stops Initial allocation of memory for 2500000 points 
            dtm.SetMemoryAllocationParameters(1000, 1000);

            BGEO.DPoint3d randomSpot = new BGEO.DPoint3d();
            randomSpot.X = 50;
            randomSpot.Y = 50;
            randomSpot.Z = 50;

            BGEO.DPoint3d[] tPoint = new BGEO.DPoint3d[4];
            tPoint[0] = new BGEO.DPoint3d(100, 100, 100);
            tPoint[1] = new BGEO.DPoint3d(100, 200, 100);
            tPoint[2] = new BGEO.DPoint3d(200, 200, 100);
            tPoint[3] = new BGEO.DPoint3d(200, 100, 100);

            Console.WriteLine("Adding Point");
            dtm.AddPoint(randomSpot);

            Console.WriteLine("Adding Points");
            dtm.AddPoints(tPoint);

            // add spot feature
            Console.WriteLine("Adding Spot Feature");
            DTMFeatureId spotId = dtm.AddPointFeature(tPoint);
            Console.WriteLine("Spot Feature Id = {0}", spotId);


            // build the dtm
            dtm.Triangulate();

            // add break line features
            tPoint = new BGEO.DPoint3d[2];
            tPoint[0] = new BGEO.DPoint3d(100, 100, 100);
            tPoint[1] = new BGEO.DPoint3d(200, 200, 100);
            Console.WriteLine("Adding Break Line Feature");
            DTMFeatureId id = dtm.AddLinearFeature(tPoint, DTMFeatureType.Breakline);
            Console.WriteLine("Break Feature Id = {0}", id);

            // build the dtm
            dtm.Triangulate();
            DTMFeature dtmFeature = dtm.GetFeatureById(id);
            Assert.AreEqual((dtmFeature is DTMLinearFeature), true);

            DTMFeature feature = dtm.GetFeatureById(id);
            dtm.DeleteFeaturesByType(DTMFeatureType.Breakline);
            id = dtm.AddLinearFeature(tPoint, DTMFeatureType.Breakline);

            // add break line features
            tPoint = new BGEO.DPoint3d[2];
            tPoint[0] = new BGEO.DPoint3d(100, 100, 100);
            tPoint[1] = new BGEO.DPoint3d(200, 200, 100);
            id = dtm.AddLinearFeature(tPoint, DTMFeatureType.Breakline);

            // build the dtm
            dtm.Triangulate();
            // get the breakline feature by id
            dtmFeature = dtm.GetFeatureById(id);
            Assert.AreEqual((dtmFeature is DTMLinearFeature), true);

            // Test DTMDrapedLinearElement
            tPoint = new BGEO.DPoint3d[2];
            tPoint[0] = new BGEO.DPoint3d(100, 150, 100);
            tPoint[1] = new BGEO.DPoint3d(200, 150, 100);

// ToDo...Standalone
            //Bentley.Civil.Geometry.Linear.Line line = new Bentley.Civil.Geometry.Linear.Line(tPoint[0], tPoint[1]);
            //DTMDrapedLinearElement nSection = LinearGeometry.Helper.DTMDrapeLinearElement(dtm, line as Bentley.Civil.Geometry.Linear.LinearElement, 0.0001);
            //int nPoint = nSection.Count;

            // Specific test for dtm features
            DTM nDtmInput = new DTM();

            // add spot feature points
            BGEO.DPoint3d point = new BGEO.DPoint3d(0, 0, 0);

            point.XX = 100;
            point.YY = 100;
            point.ZZ = 100;
            Console.WriteLine("Storing Point 1");
            id = nDtmInput.AddPointFeature(point, 100);

            point.XX = 100;
            point.YY = 200;
            point.ZZ = 100;
            Console.WriteLine("Storing Point 2");
            id = nDtmInput.AddPointFeature(point, 200);

            point.XX = 200;
            point.YY = 200;
            point.ZZ = 100;
            Console.WriteLine("Storing Point 3");
            id = nDtmInput.AddPointFeature(point, 300);

            point.XX = 200;
            point.YY = 100;
            point.ZZ = 100;
            Console.WriteLine("Storing Point 4");
            id = nDtmInput.AddPointFeature(point, 400);

            Console.WriteLine("Triangulating");

            nDtmInput.Triangulate();		// Causes access violation


            Console.WriteLine("Deleting Feature {0}", id);
            nDtmInput.DeleteFeatureById(id);
            Console.WriteLine("Triangulating");
            nDtmInput.Triangulate();

            nDtmInput.Dispose();

            /*
            // Try after the dtm input is cloned
            DTMInput nDtmInputCloned = nDtmInput.Clone ();
            nDtm1 = nDtmInputCloned.BuildDTM ();
            dtmFeature = nDtm1.GetFeatureById (id);
            Assert.AreEqual ((dtmFeature is DTMLinearFeature), true);

            // Test DTM properties
            string path = Helper.GetTestDataLocation () +"Bentley.Civil.Dtm.Light.NUnit.dll\\Build\\t3.tin";
            DTM nDtm3 = DTM.CreateFromFile (path);
            Assert.AreEqual (17, nDtm3.LinesCount);
            Assert.AreEqual (2, nDtm3.BreakLinesCount);
            for (int iIter = 0; iIter < 0; iIter++)
                {
                // Test DTMInput method
                tPoint = new BGEO.DPoint3d[4];
                nDtmInput = new DTMInput ();
                tPoint[0] = new BGEO.DPoint3d (100, 100, 100);
                tPoint[1] = new BGEO.DPoint3d (100, 200, 100);
                tPoint[2] = new BGEO.DPoint3d (200, 200, 100);
                tPoint[3] = new BGEO.DPoint3d (200, 100, 100);
                DTMId spotId1 = nDtmInput.AddPointFeature (tPoint);
                DTMFeature spotFeature = nDtmInput.GetFeatureById (spotId1);
                DTMSpot dtmSpot = spotFeature as DTMSpot;

                BGEO.DPoint3d[] tPointSpot = dtmSpot.GetPoints ();
                Assert.AreEqual (4, tPointSpot.Length);

                int iPoint = 0;
                foreach (BGEO.DPoint3d pSpot in tPointSpot)
                    {
                    Assert.AreEqual (tPoint[iPoint++], pSpot);
                    }

                tPoint = new BGEO.DPoint3d[2];
                tPoint[0] = new BGEO.DPoint3d (100, 100, 100);
                tPoint[1] = new BGEO.DPoint3d (200, 200, 100);
                DTMId id1 = nDtmInput.AddFeatureByPointString (tPoint, DTMFeatureType.Breakline);
                DTMInput nDtmInputbak = nDtmInput.Clone ();
                DTM nDtm4 = nDtmInputbak.BuildDTM ();
                Assert.AreEqual (1, nDtm4.BreakLinesCount);
                Assert.AreEqual (nDtm4.FeaturesCount, 2);
                nDtm4.Dispose ();
                nDtmInputbak = nDtmInput.Clone ();
                nDtmInputbak.DeleteFeaturesByType (DTMFeatureType.Breakline);
                nDtm4 = nDtmInputbak.BuildDTM ();
                Assert.AreEqual (nDtm4.BreakLinesCount, 0);
                nDtm4.Dispose ();
                nDtmInput = nDtmInputbak.Clone ();
                nDtmInputbak.Dispose ();

                tPoint[0] = new BGEO.DPoint3d (100, 100, 100);
                tPoint[1] = new BGEO.DPoint3d (200, 200, 100);
                DTMId blId = nDtmInput.AddFeatureByPointString (tPoint, DTMFeatureType.Breakline, 101);
                DTMFeature blfeature1 = nDtmInput.GetFeatureById (blId);
                nDtmInput.Save ("tst.dat", true);
                nDtm4 = nDtmInput.BuildDTM ();
                Assert.AreEqual (nDtm4.BreakLinesCount, 1);
                blfeature1 = nDtm4.GetFeatureById (blId);
                Assert.AreEqual (blId.Id, blfeature1.Id.Id);
                if (blfeature1 is DTMLinearFeature)
                    {
                    DTMLinearFeature lFeature = blfeature1 as DTMLinearFeature;
                    Assert.AreEqual (1, lFeature.ElementsCount);

                    BCG.LinearElement[] tElement = lFeature.GetElements ();
                    Assert.AreEqual (1, tElement.Length);
                    BCG.LinearElement bLine = tElement[0];

                    Assert.AreEqual (tPoint[0], bLine.StartPoint.Coordinates);
                    Assert.AreEqual (tPoint[1], bLine.EndPoint.Coordinates);
                    }

                // Test feature enumerator
                DTMFeatureScanCriteria nDtmScan = new DTMFeatureScanCriteria ();
                nDtmScan.IncludeFeatureType (DTMFeatureType.Breakline);
                DTMFeatureEnumerator nFeatureEnumerator = nDtm4.GetFeatures (nDtmScan);
                nFeatureEnumerator.Reset ();
                nFeatureEnumerator.MoveNext ();
                DTMFeature nFeature = (nFeatureEnumerator.Current) as DTMFeature;
                BCG.LinearElement[] tBlElement = (nFeature as DTMLinearFeature).GetElements ();
                BCG.LineString breakline = tBlElement[0] as BCG.LineString;
                BCG.LinearPointCollection vertices = breakline.StrokeByEndPoints ();
                BGEO.DPoint3d[] breaklinePoints = vertices.GetVertices ();
                Assert.AreEqual (tPoint[0], breaklinePoints[0]);
                Assert.AreEqual (tPoint[1], breaklinePoints[1]);

                nDtmScan.ExcludeAllFeatureTypes ();
                nDtmScan.IncludeFeatureType (DTMFeatureType.PointFeature);
                nFeatureEnumerator = nDtm4.GetFeatures (nDtmScan);
                nFeatureEnumerator.Reset ();
                nFeatureEnumerator.MoveNext ();
                DTMFeature spotFt = (nFeatureEnumerator.Current) as DTMFeature;
                if (spotFt is DTMSpot)
                    {
                    DTMSpot dtmSpotFt = spotFt as DTMSpot;
                    BGEO.DPoint3d[] spotPoint = dtmSpotFt.GetPoints ();
                    Assert.AreEqual (4, spotPoint.Length);

                    foreach (BGEO.DPoint3d pSpot in spotPoint)
                        {
                        Assert.AreEqual (100.0, pSpot.Z);
                        }
                    }

                // Test : DTMInput.Append method
                tPoint = new BGEO.DPoint3d[5];
                DTMInput nApendDtmInput = new DTMInput ();
                DTMInput clonedDtmInput = nDtmInput.Clone ();

                tPoint[0] = new BGEO.DPoint3d (125, 125, 125);
                tPoint[1] = new BGEO.DPoint3d (125, 175, 125);
                tPoint[2] = new BGEO.DPoint3d (175, 175, 125);
                tPoint[3] = new BGEO.DPoint3d (175, 125, 125);
                tPoint[4] = new BGEO.DPoint3d (150, 150, 150);

                nApendDtmInput.AddPointFeature (tPoint);
                clonedDtmInput.Append (nApendDtmInput);
                DTM nClonedDtm = clonedDtmInput.BuildDTM ();
                clonedDtmInput.Dispose ();
                Assert.AreEqual (nClonedDtm.VerticesCount, 9);
                nClonedDtm.Dispose ();

                // End Test

                // Test SlopeIndicators callback
                DTMInput nSlpDtmInput = new DTMInput ();
                nSlpDtmInput.AddPointFeature (tPoint);
                SlopeIndicatorsBrowsingDelegate hdlSlpIndic = new SlopeIndicatorsBrowsingDelegate (processSlopeIndicator);
                DTM nDtmSlp = nSlpDtmInput.BuildDTM ();
                nDtmSlp.BrowseSlopeIndicators (nDtm4, 5, 1, hdlSlpIndic);
                nDtmSlp.Dispose ();

                // Test Browse methods
                tPoint = new BGEO.DPoint3d[5];
                tPoint[0] = new BGEO.DPoint3d (100, 100, 100);
                tPoint[1] = new BGEO.DPoint3d (100, 200, 100);
                tPoint[2] = new BGEO.DPoint3d (200, 200, 100);
                tPoint[3] = new BGEO.DPoint3d (200, 100, 100);
                tPoint[4] = new BGEO.DPoint3d (150, 150, 150);
                nDtmInput = new DTMInput ();
                nDtmInput.AddPointFeature (tPoint);
                DTM nDtm = nDtmInput.BuildDTM ();
                BGEO.DPoint3d[] tAscentTrace = nDtm.GetAscentTrace (1, tPoint[0]);
                tAscentTrace = nDtm.GetAscentTrace (1, tPoint[4]);
                BGEO.DPoint3d[] tDescentTrace = nDtm.GetDescentTrace (1, tPoint[0]);
                tDescentTrace = nDtm.GetDescentTrace (1, tPoint[4]);

                tAscentTrace = nDtm.GetAscentTrace (1, tPoint[0]);
                tAscentTrace = nDtm.GetAscentTrace (1, tPoint[4]);
                tDescentTrace = nDtm.GetDescentTrace (1, tPoint[0]);
                tDescentTrace = nDtm.GetDescentTrace (1, tPoint[4]);

                // Test hull property
                BCG.ClosedLineString hull = nDtm.Hull;
                Assert.AreEqual (5, hull.GetVertices ().Length);

                DynamicFeaturesBrowsingDelegate hdl2P = new DynamicFeaturesBrowsingDelegate (processPointsFeatures);
                ArrayList tTriangle = new ArrayList ();

                // Triangle  feature
                nDtm.BrowseDynamicFeatures (nDtm.Range3d, DTMDynamicFeatureType.Triangle, hdl2P, tTriangle);
                Assert.AreEqual (nDtm.TrianglesCount, tTriangle.Count);

                // Triangle Edge feature
                nDtm.BrowseDynamicFeatures (nDtm.Range3d, DTMDynamicFeatureType.TriangleEdge, hdl2P, tTriangle);

                nDtm.BrowseRidgeLines (nDtm.Range3d, hdl2P);
                nDtm.BrowseSumpLines (nDtm.Range3d, hdl2P);

                // Test : Process feature
                SinglePointFeaturesBrowsingDelegate hdlP = new SinglePointFeaturesBrowsingDelegate (processPointFeatures);
                nDtm.BrowseHighPoints (nDtm.Range3d, hdlP);
                nDtm.BrowseLowPoints (nDtm.Range3d, 1, hdlP);

                // Test theme
                DoubleRange[] tRange = new DoubleRange[2];
                tRange[0] = new DoubleRange (100, 120);
                tRange[1] = new DoubleRange (130, 150);
                nDtm.AnalyzeElevation (tRange, true, nDtm.Range3d, hdl2P);
                nDtm.AnalyzeElevation (tRange, false, nDtm.Range3d, hdl2P);

                tRange[0] = new DoubleRange (0, 10);
                tRange[1] = new DoubleRange (20, 30);
                nDtm.AnalyzeSlope (tRange, true, nDtm.Range3d, hdl2P);
                nDtm.AnalyzeSlope (tRange, false, nDtm.Range3d, hdl2P);

                tRange[0] = new DoubleRange (0, 1.6);
                tRange[1] = new DoubleRange (1.6, 3.14);
                nDtm.AnalyzeAspect (tRange, true, nDtm.Range3d, hdl2P);
                nDtm.AnalyzeAspect (tRange, false, nDtm.Range3d, hdl2P);

                // Test Drape methods
                //tPoint[4] = new BGEO.DPoint3d(150,150,150)
                BGEO.DPoint3d tstPoint = new BGEO.DPoint3d (150, 150, 200);
                DTMDrapedPoint drapedPoint = nDtm.DrapePoint (tstPoint);
                Assert.AreEqual (tPoint[4], drapedPoint.Coordinates);

                ContoursBrowsingDelegate hdl3P = new ContoursBrowsingDelegate (processContours);
                nDtm.BrowseContours (1, DTMContourSmoothingMethod.None, hdl3P);

                nDtm.Dispose ();
                }
            */
        }

        /*
        /// <summary>
        ///  Slope area build tests
        /// </summary>
        [Test]
        public void SimpleDTMSlopeAreaBuildTest ()
            {
            // Create a new DTMInput object to place all the features.
            DTMInput myDtmInput = new DTMInput ();

            // Create 4 points to use as the dtm spot elevations.
            // This is where one might read them from an ascii file.
            BGEO.DPoint3d[] spotPoints = new BGEO.DPoint3d[5];
            spotPoints[0] = new BGEO.DPoint3d (0, 0, 1);
            spotPoints[1] = new BGEO.DPoint3d (10, 0, 2);
            spotPoints[2] = new BGEO.DPoint3d (10, 10, 3);
            spotPoints[3] = new BGEO.DPoint3d (0, 10, 4);
            spotPoints[4] = new BGEO.DPoint3d (10, 5, 6);

            // Add the spot points to the the DTMInput object.
            DTMId spotId1 = myDtmInput.AddPointFeature (spotPoints);

            // Build-triangulate the DTM.
            DTM myDtm = myDtmInput.BuildDTM ();

            double slopeara = myDtm.CalculateSlopeArea ();
            double slopearea1 = myDtm.SlopeArea;
            }

        /// <summary>
        /// Simple function
        /// </summary>
        [Test]
        public void SimpleDTMFunctions ()
            {
            // Test Merge method
            string path1 = Helper.GetTestDataLocation () +"Bentley.Civil.Dtm.Light.NUnit.dll\\Build\\t1.tin";
            DTM nDtm1 = DTM.CreateFromFile (path1);
            string path2 = Helper.GetTestDataLocation () +"Bentley.Civil.Dtm.Light.NUnit.dll\\Build\\t2.tin";
            DTM nDtm2 = DTM.CreateFromFile (path2);
            nDtm1.Merge (nDtm2);
            string path3 = Helper.GetTestDataLocation () +"Bentley.Civil.Dtm.Light.NUnit.dll\\Build\\t3.tin";
            nDtm1.Save (path3);
            nDtm1.Dispose ();

            // Test DTMInput method
            BGEO.DPoint3d[] tPoint = new BGEO.DPoint3d[4];
            DTMInput nDtmInput = new DTMInput ();
            tPoint[0] = new BGEO.DPoint3d (100, 100, 100);
            tPoint[1] = new BGEO.DPoint3d (100, 200, 100);
            tPoint[2] = new BGEO.DPoint3d (200, 200, 100);
            tPoint[3] = new BGEO.DPoint3d (200, 100, 100);
            DTMId spotId = nDtmInput.AddPointFeature (tPoint);
            DTMFeature spotFeature = nDtmInput.GetFeatureById (spotId);
            DTMSpot dtmSpot = spotFeature as DTMSpot;

            BGEO.DPoint3d[] tPointSpot = dtmSpot.GetPoints ();
            Assert.AreEqual (4, tPointSpot.Length);

            int iPoint = 0;
            foreach (BGEO.DPoint3d pSpot in tPointSpot)
                {
                Assert.AreEqual (tPoint[iPoint++], pSpot);
                }

            tPoint = new BGEO.DPoint3d[2];
            tPoint[0] = new BGEO.DPoint3d (100, 100, 100);
            tPoint[1] = new BGEO.DPoint3d (200, 200, 100);
            DTMId id = nDtmInput.AddFeatureByPointString (tPoint, DTMFeatureType.Breakline);
            DTMInput nDtmInputbak = nDtmInput.Clone ();
            nDtm1 = nDtmInputbak.BuildDTM ();
            Assert.AreEqual (1, nDtm1.BreakLinesCount);
            Assert.AreEqual (nDtm1.FeaturesCount, 2);
            nDtm1.Dispose ();

            nDtmInput.DeleteFeaturesByType (DTMFeatureType.Breakline);
            nDtmInputbak = nDtmInput.Clone ();
            nDtm1 = nDtmInputbak.BuildDTM ();
            Assert.AreEqual (nDtm1.BreakLinesCount, 0);
            nDtm1.Dispose ();

            DTMId blId = nDtmInput.AddFeatureByPointString (tPoint, DTMFeatureType.Breakline, 101);
            DTMFeature feature = nDtmInput.GetFeatureById (blId);

            nDtmInputbak = nDtmInput.Clone ();
            nDtm1 = nDtmInputbak.BuildDTM ();
            Assert.AreEqual (nDtm1.BreakLinesCount, 1);
            feature = nDtm1.GetFeatureById (blId);
            Assert.AreEqual (blId.Id, feature.Id.Id);

            if (feature is DTMLinearFeature)
                {
                DTMLinearFeature lFeature = feature as DTMLinearFeature;
                Assert.AreEqual (1, lFeature.ElementsCount);

                BCG.LinearElement[] tElement = lFeature.GetElements ();
                Assert.AreEqual (1, tElement.Length);
                BCG.LinearElement bLine = tElement[0];

                Assert.AreEqual (tPoint[0], bLine.StartPoint.Coordinates);
                Assert.AreEqual (tPoint[1], bLine.EndPoint.Coordinates);

                }

            // Test feature enumerator
            DTMFeatureScanCriteria nDtmScan = new DTMFeatureScanCriteria ();
            nDtmScan.IncludeFeatureType (DTMFeatureType.Breakline);
            DTMFeatureEnumerator nFeatureEnumerator = nDtm1.GetFeatures (nDtmScan);
            nFeatureEnumerator.Reset ();
            nFeatureEnumerator.MoveNext ();
            DTMFeature nFeature = (nFeatureEnumerator.Current) as DTMFeature;
            BCG.LinearElement[] tBlElement = (nFeature as DTMLinearFeature).GetElements ();
            BCG.LineString breakline = tBlElement[0] as BCG.LineString;
            BCG.LinearPointCollection vertices = breakline.StrokeByEndPoints ();
            BGEO.DPoint3d[] breaklinePoints = vertices.GetVertices ();
            Assert.AreEqual (tPoint[0], breaklinePoints[0]);
            Assert.AreEqual (tPoint[1], breaklinePoints[1]);

            nDtm1.Dispose ();
            nDtmInput.DeleteFeatureById (blId);
            nDtmInputbak = nDtmInput.Clone ();
            nDtm1 = nDtmInputbak.BuildDTM ();
            Assert.AreEqual (0, nDtm1.BreakLinesCount);
            nDtmInput.DeleteFeaturesByUserTag (101);
            nDtmInputbak = nDtmInput.Clone ();
            nDtm1 = nDtmInputbak.BuildDTM ();
            Assert.AreEqual (nDtm1.BreakLinesCount, 0);

            nDtm1.Dispose ();

            id = nDtmInput.AddFeatureByPointString (tPoint, DTMFeatureType.Breakline, 101);

            DTM nDtm = nDtmInput.BuildDTM ();
            Assert.AreEqual (nDtm.VerticesCount, 4);
            Assert.AreEqual (nDtm.BreakLinesCount, 1);
            // Doesn't work            Assert.AreEqual(nDtm.FeaturesCount, 2);
            Assert.AreEqual (nDtm.TrianglesCount, 2);
            Assert.AreEqual (nDtm.LinesCount, 5);

            // Test : DTM.range
            BGEO.DRange3d range = nDtm.Range3d;
            Assert.AreEqual (range.low.X, tPoint[0].X);
            Assert.AreEqual (range.low.Y, tPoint[0].Y);
            Assert.AreEqual (range.low.Z, tPoint[0].Z);
            Assert.AreEqual (range.high.X, tPoint[1].X);
            Assert.AreEqual (range.high.Y, tPoint[1].Y);
            Assert.AreEqual (range.high.Z, tPoint[1].Z);
            // End Test

            tPoint = new BGEO.DPoint3d[4];
            tPoint[0] = new BGEO.DPoint3d (100, 100, 100);
            tPoint[1] = new BGEO.DPoint3d (100, 200, 100);
            tPoint[2] = new BGEO.DPoint3d (200, 200, 100);
            tPoint[3] = new BGEO.DPoint3d (200, 100, 100);
            nDtmInput.Dispose ();
            nDtmInput = new DTMInput ();
            nDtmInput.AddPointFeature (tPoint);
            nDtm = nDtmInput.BuildDTM ();

            // Test : DTMInput.Clone method
            DTMInput clonedDtmInput = nDtmInput.Clone ();
            DTM nClonedDtm = clonedDtmInput.BuildDTM ();
            Assert.AreEqual (nClonedDtm.VerticesCount, 4);
            // End Test

            // Test : DTM.Clone method
            nClonedDtm = nDtm.Clone ();
            Assert.AreEqual (nClonedDtm.VerticesCount, 4);
            // End Test

            // Test : Process feature
            SinglePointFeaturesBrowsingDelegate hdlP = new SinglePointFeaturesBrowsingDelegate (processPointFeatures);
            nDtm.BrowseHighPoints (nDtm.Range3d, hdlP);
            // End Test

            // Test : DTMInput.Append method
            tPoint = new BGEO.DPoint3d[5];
            DTMInput nApendDtmInput = new DTMInput ();

            tPoint[0] = new BGEO.DPoint3d (125, 125, 125);
            tPoint[1] = new BGEO.DPoint3d (125, 175, 125);
            tPoint[2] = new BGEO.DPoint3d (175, 175, 125);
            tPoint[3] = new BGEO.DPoint3d (175, 125, 125);
            tPoint[4] = new BGEO.DPoint3d (150, 150, 150);

            nApendDtmInput.AddPointFeature (tPoint);
            clonedDtmInput.Append (nApendDtmInput);
            nClonedDtm = clonedDtmInput.BuildDTM ();
            Assert.AreEqual (nClonedDtm.VerticesCount, 9);
            // End Test

            // Test
            DTMInput nSlpDtmInput = new DTMInput ();
            nSlpDtmInput.AddPointFeature (tPoint);
            SlopeIndicatorsBrowsingDelegate hdlSlpIndic = new SlopeIndicatorsBrowsingDelegate (processSlopeIndicator);
            DTM nDtmSlp = nSlpDtmInput.BuildDTM ();
            nDtmSlp.BrowseSlopeIndicators (nDtm, 5, 1, hdlSlpIndic);

            // Test GetAscentTrace & GetDescentTrace
            tPoint = new BGEO.DPoint3d[5];
            tPoint[0] = new BGEO.DPoint3d (100, 100, 100);
            tPoint[1] = new BGEO.DPoint3d (100, 200, 100);
            tPoint[2] = new BGEO.DPoint3d (200, 200, 100);
            tPoint[3] = new BGEO.DPoint3d (200, 100, 100);
            tPoint[4] = new BGEO.DPoint3d (150, 150, 150);
            nDtmInput = new DTMInput ();
            nDtmInput.AddPointFeature (tPoint);
            nDtm = nDtmInput.BuildDTM ();
            BGEO.DPoint3d[] tAscentTrace = nDtm.GetAscentTrace (1, tPoint[0]);
            tAscentTrace = nDtm.GetAscentTrace (1, tPoint[4]);
            BGEO.DPoint3d[] tDescentTrace = nDtm.GetDescentTrace (1, tPoint[0]);
            tDescentTrace = nDtm.GetDescentTrace (1, tPoint[4]);

            tAscentTrace = nClonedDtm.GetAscentTrace (1, tPoint[0]);
            tAscentTrace = nClonedDtm.GetAscentTrace (1, tPoint[4]);
            tDescentTrace = nClonedDtm.GetDescentTrace (1, tPoint[0]);
            tDescentTrace = nClonedDtm.GetDescentTrace (1, tPoint[4]);
            // End Test

            // Test Browse methods
            nClonedDtm.BrowseHighPoints (nClonedDtm.Range3d, hdlP);
            nClonedDtm.BrowseLowPoints (nClonedDtm.Range3d, 1, hdlP);

            DynamicFeaturesBrowsingDelegate hdl2P = new DynamicFeaturesBrowsingDelegate (processPointsFeatures);
            nClonedDtm.BrowseRidgeLines (nClonedDtm.Range3d, hdl2P);
            nClonedDtm.BrowseSumpLines (nClonedDtm.Range3d, hdl2P);

            ContoursBrowsingDelegate hdl3P = new ContoursBrowsingDelegate (processContours);
            nClonedDtm.BrowseContours (nClonedDtm.Range3d, 0, 0, 1, 0, DTMContourSmoothingMethod.None, hdl3P);
            nClonedDtm.RefineCatchments (1, nClonedDtm.Range3d);
            nClonedDtm.BrowseCatchments (nClonedDtm.Range3d, 1, hdl2P);

            // Test theme
            DoubleRange[] tRange = new DoubleRange[2];
            tRange[0] = new DoubleRange (100, 120);
            tRange[1] = new DoubleRange (130, 150);
            nClonedDtm.AnalyzeElevation (tRange, true, nClonedDtm.Range3d, hdl2P);
            nClonedDtm.AnalyzeElevation (tRange, false, nClonedDtm.Range3d, hdl2P);

            tRange[0] = new DoubleRange (0, 10);
            tRange[1] = new DoubleRange (20, 30);
            nClonedDtm.AnalyzeSlope (tRange, true, nClonedDtm.Range3d, hdl2P);
            nClonedDtm.AnalyzeSlope (tRange, false, nClonedDtm.Range3d, hdl2P);

            tRange[0] = new DoubleRange (0, 1.6);
            tRange[1] = new DoubleRange (1.6, 3.14);
            nClonedDtm.AnalyzeAspect (tRange, true, nClonedDtm.Range3d, hdl2P);
            nClonedDtm.AnalyzeAspect (tRange, false, nClonedDtm.Range3d, hdl2P);

            //  double elev, volume, area, depth;
            //  nClonedDtm.AnalyzePondAtPoint (tPoint[4], out elev, out depth, out area, out volume, hdl2P);

            nClonedDtm.Dispose ();
            }
        */
        #endregion

        #region Public General Purpose DTM Test Utilities

        /// <summary>
        /// 
        /// </summary>
        /// <param name="dtm"></param>
        public void BasicProperties(DTM dtm)
        {
            Assert.IsNotNull(dtm);
            BGEO.DRange3d range3d = dtm.Range3d;
            long nVerticesCount = dtm.VerticesCount;
            long nLinesCount = dtm.CalculateFeatureStatistics().TrianglesLinesCount;
            long nTrianglesCount = dtm.TrianglesCount;
            long nFeaturesCount = dtm.CalculateFeatureStatistics ().FeaturesCount;
            long nBreakLinesCount = dtm.CalculateFeatureStatistics ().BreakLinesCount;
            long nVoidsCount = dtm.CalculateFeatureStatistics ().VoidsCount;
            long nIslandsCount = dtm.CalculateFeatureStatistics ().IslandsCount;
            long nHolesCount = dtm.CalculateFeatureStatistics ().HolesCount;
            BGEO.DPoint3d[] pts = dtm.GetBoundary ();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="dtm"></param>
        /// <returns></returns>
        public double PlanarVolume(DTM dtm)
        {
            Assert.IsNotNull(dtm);
            VolumeResult volumeResult = dtm.CalculatePrismoidalVolumeToElevation(0.0, new VolumeCriteria());
            return -volumeResult.BalanceVolume;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="hull"></param>
        /// <returns></returns>
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
        public double AscentTrace (DTM nDtm)
            {
            BGEO.DPoint3d centroid = GetCentroid (nDtm.GetBoundary ());
            DynamicFeaturesBrowsingDelegate traceBrowsingDelegate = new DynamicFeaturesBrowsingDelegate (browseTrace);
            nDtm.TraceMaximumAscent (0.1, centroid, traceBrowsingDelegate, null);
            /*
                        if (tAscentTrace != null)
                        {
            // ToDo...Standalone
            //                BCG.LineString elm = new BCG.LineString(tAscentTrace);
            //                return elm.Length;
                        }
            */
            return 0;
            }

        bool browseTrace(DTMDynamicFeatureInfo info, object userArg)
        {
            Console.WriteLine("FeatureType = {0} **** Number Of Points = {1}", info.FeatureType, info.FeaturePoints.Length);
            return true;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="nDtm"></param>
        /// <returns></returns>
        public double DescentTrace(DTM nDtm)
        {
            BGEO.DPoint3d centroid = GetCentroid(nDtm.GetBoundary());
 //           BGEO.DPoint3d[] tDescentTrace = Helper.Copy(nDtm.GetDescentTrace(1, centroid));
            DynamicFeaturesBrowsingDelegate traceBrowsingDelegate = new DynamicFeaturesBrowsingDelegate(browseTrace);
            nDtm.TraceMaximumDescent(0.1, centroid, traceBrowsingDelegate, null);
            // ToDo...Standalone
            //BCG.LineString elm = new BCG.LineString(tDescentTrace);
            //return elm.Length;
            return 0;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="dtm"></param>
        public void Themes(DTM dtm)
        {
            // Test theme
            // ToDo
            //Bentley.GeometryNET.DInterval1d[] tRange = new Bentley.GeometryNET.DInterval1d[10];
            //double deltaZ = dtm.Range3d.high.Z - dtm.Range3d.low.Z;
            //int i = 0;
            //for (i = 0; i < 10; i++)
            //    {
            //    tRange[i] = new Bentley.GeometryNET.DInterval1d (dtm.Range3d.low.Z + i * deltaZ, dtm.Range3d.low.Z + (i + 1) * deltaZ);
            //    }
            //DynamicFeaturesBrowsingDelegate hdl2P = new DynamicFeaturesBrowsingDelegate(processPointsFeatures);
            //ElevationAnalyzingBrowsingCriteria elevationCriteria = new ElevationAnalyzingBrowsingCriteria();
            //elevationCriteria.PolygonizedResult = true;
            //elevationCriteria.DoubleRange = tRange;
            //dtm.AnalyzeElevation(elevationCriteria, hdl2P, null);
            //elevationCriteria.PolygonizedResult = false;
            //dtm.AnalyzeElevation(elevationCriteria, hdl2P, null);

            //for (i = 0; i < 10; i++)
            //    {
            //    tRange[i] = new Bentley.GeometryNET.DInterval1d (i * 5, (i + 1) * 5);
            //    }

            //SlopeAnalyzingBrowsingCriteria slopeCriteria = new SlopeAnalyzingBrowsingCriteria();
            //slopeCriteria.PolygonizedResult = true;
            //slopeCriteria.DoubleRange = tRange;
            //dtm.AnalyzeSlope(slopeCriteria, hdl2P, null);
            //slopeCriteria.PolygonizedResult = false;
            //dtm.AnalyzeSlope(slopeCriteria, hdl2P, null);

            //for (i = 0; i < 10; i++)
            //    {
            //    tRange[i] = new Bentley.GeometryNET.DInterval1d (i * 0.3, (i + 1) * 0.3);
            //    }
            //AspectAnalyzingBrowsingCriteria aspectCriteria = new AspectAnalyzingBrowsingCriteria();
            //aspectCriteria.PolygonizedResult = true;
            //aspectCriteria.DoubleRange = tRange;
            //dtm.AnalyzeAspect(aspectCriteria, hdl2P, null);
            //aspectCriteria.PolygonizedResult = false;
            //dtm.AnalyzeAspect(aspectCriteria, hdl2P, null);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="dtm"></param>
        public void BrowseFeatures(DTM dtm)
        {
            DynamicFeaturesBrowsingDelegate hdl2P = new DynamicFeaturesBrowsingDelegate(processPointsFeatures);
            ArrayList tTriangle = new ArrayList();

            DynamicFeaturesBrowsingCriteria criteria = new DynamicFeaturesBrowsingCriteria();

            // Triangle  features
            int took = System.Environment.TickCount;
            dtm.BrowseDynamicFeatures(criteria, DTMDynamicFeatureType.Triangle, hdl2P, null); //tTriangle);
            took = System.Environment.TickCount - took;
            System.Console.WriteLine("Took " + took.ToString());
            // Assert.AreEqual(dtm.TrianglesCount, tTriangle.Count);

            // Triangle Edge feature
            dtm.BrowseDynamicFeatures(criteria, DTMDynamicFeatureType.TriangleEdge, hdl2P, tTriangle);

            RidgeLinesBrowsingCriteria ridgeLineCriteria = new RidgeLinesBrowsingCriteria();
            dtm.BrowseRidgeLines(ridgeLineCriteria, hdl2P, null);
            SumpLinesBrowsingCriteria sumpLinesCriteria = new SumpLinesBrowsingCriteria();
            dtm.BrowseSumpLines(sumpLinesCriteria, hdl2P, null);

            SinglePointFeaturesBrowsingDelegate hdlP = new SinglePointFeaturesBrowsingDelegate(processPointFeatures);
            HighPointsBrowsingCriteria highPointsCriteria = new HighPointsBrowsingCriteria();
            dtm.BrowseHighPoints(highPointsCriteria, hdlP, null);
            LowPointsBrowsingCriteria lowPointsCriteria = new LowPointsBrowsingCriteria();
            dtm.BrowseLowPoints(lowPointsCriteria, 1, hdlP, null);

            ContoursBrowsingDelegate hdl3P = new ContoursBrowsingDelegate(processContours);
            ContoursBrowsingCriteria contoursCriteria = new ContoursBrowsingCriteria(1);
            dtm.BrowseContours(contoursCriteria, hdl3P, null);

            took = System.Environment.TickCount;
            LinearFeaturesBrowsingDelegate hdl4P = new LinearFeaturesBrowsingDelegate (processFeatures);
            dtm.BrowseLinearFeatures(new LinearFeaturesBrowsingCriteria(), DTMFeatureType.Breakline, hdl4P, null);
            took = System.Environment.TickCount - took;
            System.Console.WriteLine("Took " + took.ToString());
        }

        private bool processFeatures(DTMFeatureInfo info, object userArg)
        {
            return true;
        }
        /// <summary>
        /// 
        /// </summary>
        /// <param name="dtm"></param>
        public void VolumeRange(DTM dtm)
        {
            // Test Range
            VolumeRange[] tRange = new VolumeRange[10];
            double deltaZ = dtm.Range3d.High.Z - dtm.Range3d.Low.Z;
            int i = 0;
            for (i = 0; i < 10; i++)
            {
                tRange[i] = new VolumeRange(dtm.Range3d.Low.Z + i * deltaZ, dtm.Range3d.Low.Z + (i + 1) * deltaZ);
            }
            VolumeCriteria crit = new VolumeCriteria();
            crit.RangeTable = tRange;
            VolumeResult volumeResult = dtm.CalculatePrismoidalVolumeToElevation(0, crit);
            // TO BE COMPLETED
        }

        #endregion

        #region Private Test Utilities

        int processPointFeatures(DTMDynamicFeatureType featureType, BGEO.DPoint3d point)
        {
            return 0;
        }

        int processPointsFeatures(DTMDynamicFeatureType featureType, BGEO.DPoint3d[] tPoint)
        {
            return 0;
        }
        int processContours(BGEO.DPoint3d[] tPoint, BGEO.DPoint3d direction)
        {
            return 0;
        }
        int processSlopeIndicator(bool major, BGEO.DPoint3d startPoint, BGEO.DPoint3d endPoint)
        {
            return 0;
        }
        bool processContours(BGEO.DPoint3d[] tPoint, BGEO.DPoint3d direction, System.Object oArg)
        {
            return true;
        }

        bool processSlopeIndicator(bool major, BGEO.DPoint3d startPoint, BGEO.DPoint3d endPoint, System.Object oArg)
        {
            return true;
        }

        bool processPointFeatures(DTMDynamicFeatureType featureType, BGEO.DPoint3d point, System.Object oArg)
        {
            return true;
        }

        bool processPointsFeatures(DTMDynamicFeatureInfo info, System.Object oArg)
        {
            if (oArg != null)
            {
                ArrayList al = oArg as ArrayList;
                al.Add (info.FeaturePoints);
            }
            return true;
        }

        private void UpdateCommandAuto(SDO.OleDbDataAdapter adapter)
        {
            SDO.OleDbParameter workParam = null;

            //  This does not seems to be working when we place WHERE into our command. I can figure it later
            string query = "UPDATE Build SET RunTime  = ?, NumberOfTriangles = ?, VolumeToZero = ?  WHERE Filename = ? ";
            adapter.UpdateCommand = new SDO.OleDbCommand(query, _dbConnection);

            workParam = adapter.UpdateCommand.Parameters.Add("@RunTime", SDO.OleDbType.VarChar, 50, "RunTime");
            workParam.SourceVersion = SD.DataRowVersion.Current;

            workParam = adapter.UpdateCommand.Parameters.Add("@NumberOfTriangles", SDO.OleDbType.VarChar, 50, "NumberOfTriangles");
            workParam.SourceVersion = SD.DataRowVersion.Current;

            workParam = adapter.UpdateCommand.Parameters.Add("@VolumeToZero", SDO.OleDbType.VarChar, 50, "VolumeToZero");
            workParam.SourceVersion = SD.DataRowVersion.Current;

            workParam = adapter.UpdateCommand.Parameters.Add("@Filename", SDO.OleDbType.VarChar, 50);
            workParam.SourceColumn = "Filename";
            workParam.SourceVersion = SD.DataRowVersion.Original;
        }
        #endregion
    }
}

