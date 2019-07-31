using System;
using System.Collections;
using System.IO;
using SD = System.Data;
using SDO = System.Data.OleDb;
using BGEO = Bentley.GeometryNET;

using NUnit.Framework;

namespace Bentley.TerrainModelNET.NUnit
{
    using Bentley.TerrainModelNET;

    /// <summary>
    ///   Simple DTM tests
    /// </summary>
    [TestFixture]
    public class SimpleTestDTM : DTMUNitTest
    {
        #region Private Fields
        int nContourInterval = 0;


        private SDO.OleDbConnection _dbConnection;

        private System.IFormatProvider _formatProvider = System.Globalization.NumberFormatInfo.CurrentInfo;

        #endregion

        #region Constructor

        /// <summary>
        /// Constructor
        /// </summary>
        public SimpleTestDTM()
        {
        }

        /// <summary>
        /// Destructure
        /// </summary>
        ~SimpleTestDTM()
        {
            if (_dbConnection != null)
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


        /// <summary>
        /// Add and removes spots
        /// </summary>
        [Test]
        [Category("Claude Test")]
//        [ExpectedException(typeof(Bentley.TerrainModelNET.DTMException), "Less Than Three Points In DTM")]
        public void CheckDTM_AddAndRemoveSpotFeatures()
        {

            // Specific test for dtm features
            DTM nDtmInput = new DTM();

            // add spot features
            BGEO.DPoint3d[] tPoint = new BGEO.DPoint3d[4];
            tPoint[0] = new BGEO.DPoint3d(100, 100, 100);
            tPoint[1] = new BGEO.DPoint3d(100, 200, 100);
            tPoint[2] = new BGEO.DPoint3d(200, 200, 100);
            tPoint[3] = new BGEO.DPoint3d(200, 100, 100);
            DTMFeatureId spotId = nDtmInput.AddPointFeature(tPoint, 10);

            Console.WriteLine("Triangulating");
            nDtmInput.Triangulate();

            // Check The Triangulation - Unit Test Checking Only Not For Production release
            Console.WriteLine("Checking Triangulation");
            if (nDtmInput.CheckTriangulation() == false)
            {
                throw new Exception("\nTriangulation Invalid");
            }
            Assert.AreEqual (1, nDtmInput.CalculateFeatureStatistics ().FeaturesCount);

            Console.WriteLine("Deleting Feature With Id {0}", spotId);
            nDtmInput.DeleteFeatureById(spotId);

            // DTM Will Throw An Exception With Less Than Three Points If You Try To Retriangulate
            Console.WriteLine("Triangulating");
            nDtmInput.Triangulate();
            // Check The Triangulation - Unit Test Checking Only Not For Production release
            Console.WriteLine("Checking Triangulation");
            if (nDtmInput.CheckTriangulation() == false)
            {
                throw new Exception("\nTriangulation Invalid");
            }

            Assert.AreEqual (0, nDtmInput.CalculateFeatureStatistics ().FeaturesCount);
        }

        /// <summary>
        /// Add and removes spots
        /// </summary>
        [Test]
        [Category("Lots of points test")]
        public void CheckDTM_AddLotsOfSpots()
        {
            IgnoreFirebug ();

            // Read the input from the database
            if (_dbConnection.State == SD.ConnectionState.Closed)
                _dbConnection.Open();
            SDO.OleDbDataAdapter adapter = new SDO.OleDbDataAdapter("Select * from AddLotsOfSpots", _dbConnection);

            SD.DataSet ds = new SD.DataSet();
            adapter.Fill(ds, "AddLotsOfSpots");
            SD.DataRowCollection rowCollection = ds.Tables["AddLotsOfSpots"].Rows;

            // Process all the input rows for each file name
            foreach (SD.DataRow row in rowCollection)
            {
                int iniPoints = System.Convert.ToInt32(row["IniPoints"].ToString());
                int incPoints = System.Convert.ToInt32(row["IncPoints"].ToString());
                int numPoints = System.Convert.ToInt32(row["NumberPoints"].ToString());
                bool randomPoints = System.Convert.ToBoolean(row["RandomPoints"].ToString());

                System.GC.Collect();

                // Specific test for dtm features
                DTM nDtmInput = new DTM(iniPoints, incPoints);

                // add spot features

                //            BGEO.DPoint3d[] tPoint = new BGEO.DPoint3d[numPoints];
                BGEO.DPoint3d[] tPoint = new BGEO.DPoint3d[1000];


                if (randomPoints)
                {
                    System.Random rand = new System.Random();
                    for (int i = 0; i < numPoints; i = i + 1000)
                    {
                        for (int j = 0; j < 1000; ++j)
                        {
                            tPoint[j] = new BGEO.DPoint3d(rand.NextDouble() * 10000.0, rand.NextDouble() * 10000.0, rand.NextDouble() * 10000.0);
                        }
                        nDtmInput.AddPointFeature(tPoint);
                    }
                }
                else
                {
                    for (int iLoop = 0; iLoop < numPoints; iLoop = iLoop + 1000)
                    {
                        // RobC - Changed The loop Points To Stop The Creation Of Sliver Tins
                        //
                        //                            tPoint[iLoop] = new BGEO.DPoint3d(iLoop, iLoop, 100);
                        //                            tPoint[iLoop + 1] = new BGEO.DPoint3d(iLoop, iLoop + 10, 100);
                        //                            tPoint[iLoop + 2] = new BGEO.DPoint3d(iLoop, iLoop - 10, 100);
                        //                            tPoint[iLoop + 3] = new BGEO.DPoint3d(iLoop + 10, iLoop + 10, 100);
                        for (int j = 0; j < 1000; j = j + 4)
                        {
                            tPoint[j] = new BGEO.DPoint3d(iLoop + j, 0, 100);
                            tPoint[j + 1] = new BGEO.DPoint3d(iLoop + j, 10, 100);
                            tPoint[j + 2] = new BGEO.DPoint3d(iLoop + j, 20, 100);
                            tPoint[j + 3] = new BGEO.DPoint3d(iLoop + j, 30, 100);
                        }
                        nDtmInput.AddPointFeature(tPoint);
                    }
                }

                nDtmInput.Triangulate();
                nDtmInput.Dispose();
                nDtmInput = null;
                tPoint = null;
            }
        }

        /*
        /// <summary>
        ///  Xml files import 
        /// </summary>
        [Test]
        public void CheckLandXml_Import_Base ()
            {
            // Needs the following files :
            // topoc.XML
            // refDtm.XML
            // ground.XML

            BCLX.LandXml.GetInstance ().ExportSurfacesToDat (utilDtm.getFullPathName ("LandXml\\topoc.XML"), "");
            string[] tinFileNameTab = BCLX.LandXml.GetInstance ().ExportSurfacesToTin (utilDtm.getFullPathName ("LandXml\\topoc.XML"), "");
            Assert.AreEqual (1, tinFileNameTab.Length);
            DTM nDtm = DTM.CreateFromFile ("topoc.tin");
            Assert.AreEqual (31, nDtm.BreakLinesCount);

            DTM[] refDtmInput = BCLX.LandXml.GetInstance ().ImportSurfaces (utilDtm.getFullPathName ("LandXml\\refDtm.XML"));
            nDtm = refDtmInput[0].Triangulate ();
            BCLX.LandXml.GetInstance ().ExportDTM (nDtm, utilDtm.getFullPathName ("LandXml\\newDtm.XML"), "Project Name", "Project Description", "1.0.0.0", true);
            DTMInput[] myDtmInput = BCLX.LandXml.GetInstance ().ImportSurfaces (utilDtm.getFullPathName ("LandXml\\newDtm.XML"));
            DTM nDtm1 = myDtmInput[0].Triangulate ();

            Assert.AreEqual (nDtm.TrianglesCount, nDtm1.TrianglesCount);

            myDtmInput = BCLX.LandXml.GetInstance ().ImportSurfaces (utilDtm.getFullPathName ("LandXml\\ground.xml"));
            nDtm1 = myDtmInput[0].Triangulate ();
            BCLX.LandXml.GetInstance ().ExportDTM (nDtm1, utilDtm.getFullPathName ("LandXml\\newground.XML"), "Project Name", "Project Description", "1.0.0.0", true);
            myDtmInput = BCLX.LandXml.GetInstance ().ImportSurfaces ("newground.xml");
            nDtm = myDtmInput[0].Triangulate ();
            Assert.AreEqual (nDtm.TrianglesCount, nDtm1.TrianglesCount);
            BCLX.LandXml.GetInstance ().ExportDTM (nDtm, utilDtm.getFullPathName ("LandXml\\newground2.XML"), "Project Name", "Project Description", "1.0.0.0", true);
            }
        */


        /// <summary>
        /// 
        /// </summary>
        [Test]
        [Category("Claude Test")]
        public void CheckDTM_DatFileNoFeature()
        {
            String DataDirectory = @"Bentley.Civil.Dtm.Light.NUnit.dll\Build\";
            String DataPath = Helper.GetTestDataLocation () + DataDirectory;
            String DatFileName = DataPath + "test.dat";
            for (int iLoop = 0; iLoop < 100; iLoop++)
            {
                DTM nDtmInput = DTM.CreateFromGeopakDatFile(DatFileName);
                if (nDtmInput != null)
                {
                    // build the dtm
                    nDtmInput.Triangulate();
                    nDtmInput.Dispose();
                }
            }
        }

        bool browseTrace(DTMDynamicFeatureInfo info, object userArg)
        {
//            Console.WriteLine("FeatureType = {0} **** Number Of Points = {1}", featureType, points.Length);
            return true;
        }
        /// <summary>
        /// General tests
        /// </summary>
        [Test]
        [Category("Claude Test")]
        public void CheckDTM()
        {
           // Needs the following files :
            // t1.tin
            // t2.tin

            // Specific test for dtm features
            DTM nDtmInput = new DTM();

            // add spot features
            BGEO.DPoint3d[] tPoint = new BGEO.DPoint3d[4];
            tPoint[0] = new BGEO.DPoint3d(100, 100, 100);
            tPoint[1] = new BGEO.DPoint3d(100, 200, 100);
            tPoint[2] = new BGEO.DPoint3d(200, 200, 100);
            tPoint[3] = new BGEO.DPoint3d(200, 100, 100);
            DTMFeatureId spotId = nDtmInput.AddPointFeature(tPoint);

            // build the dtm
            nDtmInput.Triangulate();

            // add break line features
            tPoint = new BGEO.DPoint3d[2];
            tPoint[0] = new BGEO.DPoint3d(100, 100, 100);
            tPoint[1] = new BGEO.DPoint3d(200, 200, 100);
            DTMFeatureId id = nDtmInput.AddLinearFeature(tPoint, DTMFeatureType.Breakline);

            // build the dtm
            nDtmInput.Triangulate();
            DTMFeature dtmFeature = nDtmInput.GetFeatureById(id);
            Assert.AreEqual((dtmFeature is DTMLinearFeature), true);

            // 
            DTMFeature feature = nDtmInput.GetFeatureById(id);
            nDtmInput.DeleteFeaturesByType(DTMFeatureType.Breakline);
            id = nDtmInput.AddLinearFeature(tPoint, DTMFeatureType.Breakline);

            // add break line features
            tPoint = new BGEO.DPoint3d[2];
            tPoint[0] = new BGEO.DPoint3d(100, 100, 100);
            tPoint[1] = new BGEO.DPoint3d(200, 200, 100);
            id = nDtmInput.AddLinearFeature(tPoint, DTMFeatureType.Breakline);

            // build the dtm
            nDtmInput.Triangulate();
            // get the breakline feature by id
            dtmFeature = nDtmInput.GetFeatureById(id);
            Assert.AreEqual((dtmFeature is DTMLinearFeature), true);


            // Test DTMDrapedLinearElement
            tPoint = new BGEO.DPoint3d[2];
            tPoint[0] = new BGEO.DPoint3d(100, 150, 100);
            tPoint[1] = new BGEO.DPoint3d(200, 150, 100);

// ToDo...Standalone
            //Bentley.Civil.Geometry.Linear.Line line = new Bentley.Civil.Geometry.Linear.Line(tPoint[0], tPoint[1]);
            //DTMDrapedLinearElement nSection = Bentley.TerrainModelNET.LinearGeometry.Helper.DTMDrapeLinearElement (nDtmInput, line as Bentley.Civil.Geometry.Linear.LinearElement, 0.0001);
            //int nPoint = nSection.Count;

            // Try after the dtm input is cloned
            DTM nDtmInputCloned = nDtmInput.Clone();
            nDtmInputCloned.Triangulate();
            dtmFeature = nDtmInput.GetFeatureById(id);
            Assert.AreEqual((dtmFeature is DTMLinearFeature), true);

            // Test Merge method
            DTM nDtm1 = DTM.CreateFromFile(utilDtm.getFullPathName("t1.tin"));
            DTM nDtm2 = DTM.CreateFromFile(utilDtm.getFullPathName("t2.tin"));
            DTM nDtm3 = nDtm1.Clone();
            nDtm3.Merge(nDtm2);
            nDtm3.Save("t3.tin");
            nDtm3.Dispose();

            // Test DTM properties
            nDtm3 = DTM.CreateFromFile("t3.tin");
            Assert.AreEqual (17, nDtm3.CalculateFeatureStatistics ().TrianglesLinesCount);
            Assert.AreEqual (2, nDtm3.CalculateFeatureStatistics ().BreakLinesCount);
            for (int iIter = 0; iIter < 10; iIter++)
            {
                // Test DTMInput method
                tPoint = new BGEO.DPoint3d[4];
                nDtmInput = new DTM();
                tPoint[0] = new BGEO.DPoint3d(100, 100, 100);
                tPoint[1] = new BGEO.DPoint3d(100, 200, 100);
                tPoint[2] = new BGEO.DPoint3d(200, 200, 100);
                tPoint[3] = new BGEO.DPoint3d(200, 100, 100);
                DTMFeatureId spotId1 = nDtmInput.AddPointFeature(tPoint);
                DTMFeature spotFeature = nDtmInput.GetFeatureById(spotId1);
                DTMSpot dtmSpot = spotFeature as DTMSpot;

                BGEO.DPoint3d[] tPointSpot = dtmSpot.GetPoints();
                Assert.AreEqual(4, tPointSpot.Length);

                int iPoint = 0;
                foreach (BGEO.DPoint3d pSpot in tPointSpot)
                {
                    Assert.AreEqual(tPoint[iPoint++], pSpot);
                }

                tPoint = new BGEO.DPoint3d[2];
                tPoint[0] = new BGEO.DPoint3d(100, 100, 100);
                tPoint[1] = new BGEO.DPoint3d(200, 200, 100);
                DTMFeatureId id1 = nDtmInput.AddLinearFeature(tPoint, DTMFeatureType.Breakline);
                DTM nDtmInputbak = nDtmInput.Clone();
                nDtmInputbak.Triangulate();
                Assert.AreEqual (1, nDtmInputbak.CalculateFeatureStatistics ().BreakLinesCount);
                Assert.AreEqual (nDtmInputbak.CalculateFeatureStatistics ().FeaturesCount, 2);
                nDtmInputbak.Dispose();
                nDtmInputbak = nDtmInput.Clone();
                nDtmInputbak.DeleteFeaturesByType(DTMFeatureType.Breakline);
                nDtmInputbak.Triangulate();
                Assert.AreEqual (nDtmInputbak.CalculateFeatureStatistics ().BreakLinesCount, 0);
                nDtmInput.Dispose();
                nDtmInput = nDtmInputbak.Clone();
                nDtmInputbak.Dispose();
                tPoint[0] = new BGEO.DPoint3d(100, 100, 100);
                tPoint[1] = new BGEO.DPoint3d(200, 200, 100);
                DTMFeatureId blId = nDtmInput.AddLinearFeature(tPoint, DTMFeatureType.Breakline, 101);
                DTMFeature blfeature1 = nDtmInput.GetFeatureById(blId);

                //todo spu: add a method to save as dat.
                //nDtmInput.Save ("tst.dat", true);
                nDtmInput.Triangulate();
                Assert.AreEqual (nDtmInput.CalculateFeatureStatistics ().BreakLinesCount, 1);
                blfeature1 = nDtmInput.GetFeatureById(blId);
                Assert.AreEqual(blId, blfeature1.Id);
                if (blfeature1 is DTMLinearFeature)
                {
                    DTMLinearFeature lFeature = blfeature1 as DTMLinearFeature;
                    Assert.AreEqual(1, lFeature.ElementsCount);

// ToDo...Standalone
                    //BCG.LinearElement[] tElement = Bentley.TerrainModelNET.LinearGeometry.Helper.DTMLinearFeatureGetElements (lFeature);
                    //Assert.AreEqual(1, tElement.Length);
                    //BCG.LinearElement bLine = tElement[0];

                    //Assert.AreEqual(tPoint[0], bLine.StartPoint.Coordinates);
                    //Assert.AreEqual(tPoint[1], bLine.EndPoint.Coordinates);
                }

                nDtmInput.BrowseLinearFeatures(null, DTMFeatureType.Breakline,
                    (LinearFeaturesBrowsingDelegate)delegate(DTMFeatureInfo featureInfo, object oArg)
                        {
// ToDo...Standalone
                            //DTMFeature nFeature = nDtmInput.GetFeatureById(featureInfo.DtmFeatureId);
                            //BCG.LinearElement[] tBlElement = Bentley.TerrainModelNET.LinearGeometry.Helper.DTMLinearFeatureGetElements ((nFeature as DTMLinearFeature));
                            //BCG.LineString breakline = tBlElement[0] as BCG.LineString;
                            //BCG.LinearPointCollection vertices = breakline.StrokeByEndPoints();
                            //BGEO.DPoint3d[] breaklinePoints = vertices.GetVertices();
                            //Assert.AreEqual(tPoint[0], breaklinePoints[0]);
                            //Assert.AreEqual(tPoint[1], breaklinePoints[1]);
                            return true;
                        }, null);

                nDtmInput.BrowseLinearFeatures(null, DTMFeatureType.PointFeature,
                    (LinearFeaturesBrowsingDelegate)delegate(DTMFeatureInfo featureInfo, object oArg)
                        {
                            DTMFeature spotFt = (nDtmInput.GetFeatureById(featureInfo.DtmFeatureId)) as DTMFeature;
                            if (spotFt is DTMSpot)
                            {
                                DTMSpot dtmSpotFt = spotFt as DTMSpot;
                                BGEO.DPoint3d[] spotPoint = dtmSpotFt.GetPoints();
                                Assert.AreEqual(4, spotPoint.Length);

                                foreach (BGEO.DPoint3d pSpot in spotPoint)
                                {
                                    Assert.AreEqual(100.0, pSpot.Z);
                                }
                            }
                            return true;
                        }, null);
                // Test feature enumerator
                //DTMFeatureScanCriteria nDtmScan = new DTMFeatureScanCriteria();
                //nDtmScan.IncludeFeatureType(DTMFeatureType.Breakline);
                //DTMFeatureEnumerator nFeatureEnumerator = nDtmInput.GetFeatures(nDtmScan);
                //nFeatureEnumerator.Reset();
                //nFeatureEnumerator.MoveNext();
                //DTMFeature nFeature = (nFeatureEnumerator.Current) as DTMFeature;
                //BCG.LinearElement[] tBlElement = (nFeature as DTMLinearFeature).GetElements();
                //BCG.LineString breakline = tBlElement[0] as BCG.LineString;
                //BCG.LinearPointCollection vertices = breakline.StrokeByEndPoints();
                //BGEO.DPoint3d[] breaklinePoints = vertices.GetVertices();
                //Assert.AreEqual(tPoint[0], breaklinePoints[0]);
                //Assert.AreEqual(tPoint[1], breaklinePoints[1]);
                //nDtmScan.ExcludeAllFeatureTypes();
                //nDtmScan.IncludeFeatureType(DTMFeatureType.PointFeature);
                //nFeatureEnumerator = nDtmInput.GetFeatures(nDtmScan);
                //nFeatureEnumerator.Reset();
                //nFeatureEnumerator.MoveNext();
                //DTMFeature spotFt = (nFeatureEnumerator.Current) as DTMFeature;
                //if (spotFt is DTMSpot)
                //    {
                //    DTMSpot dtmSpotFt = spotFt as DTMSpot;
                //    BGEO.DPoint3d[] spotPoint = dtmSpotFt.GetPoints();
                //    Assert.AreEqual(4, spotPoint.Length);

                //    foreach (BGEO.DPoint3d pSpot in spotPoint)
                //        {
                //        Assert.AreEqual(100.0, pSpot.Z);
                //        }
                //    }

                // Test : DTMInput.Append method
                tPoint = new BGEO.DPoint3d[5];
                DTM nApendDtmInput = new DTM();
                DTM clonedDtmInput = nDtmInput.Clone();

                tPoint[0] = new BGEO.DPoint3d(125, 125, 125);
                tPoint[1] = new BGEO.DPoint3d(125, 175, 125);
                tPoint[2] = new BGEO.DPoint3d(175, 175, 125);
                tPoint[3] = new BGEO.DPoint3d(175, 125, 125);
                tPoint[4] = new BGEO.DPoint3d(150, 150, 150);

                //                nApendDtmInput.AddPointFeature(tPoint); Commented Out RobC
                clonedDtmInput.AddPointFeature(tPoint);
                //todo spu: add a method to append dtms.
                //clonedDtmInput.Append (nApendDtmInput);
                clonedDtmInput.Triangulate();
                Assert.AreEqual(clonedDtmInput.VerticesCount, 9);
                clonedDtmInput.Dispose();

                // End Test

                // Test SlopeIndicators callback
                DTM nSlpDtmInput = new DTM();
                nSlpDtmInput.AddPointFeature(tPoint);
                SlopeIndicatorsBrowsingDelegate hdlSlpIndic = new SlopeIndicatorsBrowsingDelegate(processSlopeIndicator);
                nSlpDtmInput.Triangulate();
                nSlpDtmInput.BrowseSlopeIndicators(nSlpDtmInput, new SlopeIndicatorsBrowsingCriteria(5, 1), hdlSlpIndic, null);
                nSlpDtmInput.Dispose();

                // Test Browse methods
                tPoint = new BGEO.DPoint3d[5];
                tPoint[0] = new BGEO.DPoint3d(100, 100, 100);
                tPoint[1] = new BGEO.DPoint3d(100, 200, 100);
                tPoint[2] = new BGEO.DPoint3d(200, 200, 100);
                tPoint[3] = new BGEO.DPoint3d(200, 100, 100);
                tPoint[4] = new BGEO.DPoint3d(150, 150, 150);
                nDtmInput = new DTM();
                nDtmInput.AddPointFeature(tPoint);
                nDtmInput.Triangulate();

/*
                BGEO.DPoint3d[] tAscentTrace = Helper.Copy(nDtmInput.GetAscentTrace(1, tPoint[0]));
                tAscentTrace = Helper.Copy(nDtmInput.GetAscentTrace(1, tPoint[4]));
                BGEO.DPoint3d[] tDescentTrace = Helper.Copy(nDtmInput.GetDescentTrace(1, tPoint[0]));
                tDescentTrace = Helper.Copy(nDtmInput.GetDescentTrace(1, tPoint[4]));

                tAscentTrace = Helper.Copy(nDtmInput.GetAscentTrace(1, tPoint[0]));
                tAscentTrace = Helper.Copy(nDtmInput.GetAscentTrace(1, tPoint[4]));
                tDescentTrace = Helper.Copy(nDtmInput.GetDescentTrace(1, tPoint[0]));
                tDescentTrace = Helper.Copy(nDtmInput.GetDescentTrace(1, tPoint[4]));
*/
                DynamicFeaturesBrowsingDelegate traceBrowsingDelegate = new DynamicFeaturesBrowsingDelegate (browseTrace);
                nDtmInput.TraceMaximumDescent(0.1, tPoint[0], traceBrowsingDelegate, null);
                nDtmInput.TraceMaximumAscent(0.1, tPoint[0], traceBrowsingDelegate, null);
                nDtmInput.TraceMaximumDescent(0.1, tPoint[4], traceBrowsingDelegate, null);
                nDtmInput.TraceMaximumAscent(0.1, tPoint[4], traceBrowsingDelegate, null);



                // Test hull property
                BGEO.DPoint3d[] hull = nDtmInput.GetBoundary();
                Assert.AreEqual(5, hull.Length);


                DynamicFeaturesBrowsingDelegate hdl2P = new DynamicFeaturesBrowsingDelegate(processPointsFeatures);
                ArrayList tTriangle = new ArrayList();

                // Triangle  feature
                DynamicFeaturesBrowsingCriteria criteria = new DynamicFeaturesBrowsingCriteria();
                nDtmInput.BrowseDynamicFeatures(criteria, DTMDynamicFeatureType.Triangle, hdl2P, tTriangle);
                Assert.AreEqual(nDtmInput.TrianglesCount, tTriangle.Count);

                // Triangle Edge feature
                nDtmInput.BrowseDynamicFeatures(criteria, DTMDynamicFeatureType.TriangleEdge, hdl2P, tTriangle);

                // Triangle Vertex feature
                nDtmInput.BrowseDynamicFeatures(criteria, DTMDynamicFeatureType.TriangleVertex, hdl2P, tTriangle);


                RidgeLinesBrowsingCriteria ridgeLineCriteria = new RidgeLinesBrowsingCriteria();
                nDtmInput.BrowseRidgeLines(ridgeLineCriteria, hdl2P, null);
                SumpLinesBrowsingCriteria sumpLinesCriteria = new SumpLinesBrowsingCriteria();
                nDtmInput.BrowseSumpLines(sumpLinesCriteria, hdl2P, null);

                SinglePointFeaturesBrowsingDelegate hdlP = new SinglePointFeaturesBrowsingDelegate(processPointFeatures);
                HighPointsBrowsingCriteria highPointsCriteria = new HighPointsBrowsingCriteria();
                nDtmInput.BrowseHighPoints(highPointsCriteria, hdlP, null);
                LowPointsBrowsingCriteria lowPointsCriteria = new LowPointsBrowsingCriteria();
                nDtmInput.BrowseLowPoints(lowPointsCriteria, 1, hdlP, null);

                // Test theme
                //ToDo
                //Bentley.GeometryNET.DInterval1d[] tRange = new Bentley.GeometryNET.DInterval1d[2];
                //tRange[0] = new Bentley.GeometryNET.DInterval1d(100, 120);
                //tRange[1] = new Bentley.GeometryNET.DInterval1d(130, 150);
                //ElevationAnalyzingBrowsingCriteria elevationCriteria = new ElevationAnalyzingBrowsingCriteria();
                //elevationCriteria.PolygonizedResult = true;
                //elevationCriteria.DoubleRange = tRange;
                //nDtmInput.AnalyzeElevation(elevationCriteria, hdl2P, null);
                //elevationCriteria.PolygonizedResult = false;
                //nDtmInput.AnalyzeElevation(elevationCriteria, hdl2P, null);

                //tRange[0] = new Bentley.GeometryNET.DInterval1d(0, 10);
                //tRange[1] = new Bentley.GeometryNET.DInterval1d(20, 30);

                //SlopeAnalyzingBrowsingCriteria slopeCriteria = new SlopeAnalyzingBrowsingCriteria();
                //slopeCriteria.PolygonizedResult = true;
                //slopeCriteria.DoubleRange = tRange;
                //nDtmInput.AnalyzeSlope(slopeCriteria, hdl2P, null);
                //slopeCriteria.PolygonizedResult = false;
                //nDtmInput.AnalyzeSlope(slopeCriteria, hdl2P, null);

                //tRange[0] = new Bentley.GeometryNET.DInterval1d(0, 1.6);
                //tRange[1] = new Bentley.GeometryNET.DInterval1d(1.6, 3.14);
                //AspectAnalyzingBrowsingCriteria aspectCriteria = new AspectAnalyzingBrowsingCriteria();
                //aspectCriteria.PolygonizedResult = true;
                //aspectCriteria.DoubleRange = tRange;
                //nDtmInput.AnalyzeAspect(aspectCriteria, hdl2P, null);
                //aspectCriteria.PolygonizedResult = false;
                //nDtmInput.AnalyzeAspect(aspectCriteria, hdl2P, null);

                // Test Drape methods
                //tPoint[4] = new BGEO.DPoint3d(150,150,150)
                BGEO.DPoint3d tstPoint = new BGEO.DPoint3d(150, 150, 200);
                DTMDrapedPoint drapedPoint = nDtmInput.DrapePoint(tstPoint);
                Assert.AreEqual(tPoint[4], drapedPoint.Coordinates);

                nContourInterval = 0;
                ContoursBrowsingDelegate hdl3P = new ContoursBrowsingDelegate(processContours);
                ContoursBrowsingCriteria contoursCriteria = new ContoursBrowsingCriteria(1);
                nDtmInput.BrowseContours(contoursCriteria, hdl3P, null);

                nDtmInput.Dispose();
            }
        }

        /// <summary>
        /// Browse contour tests
        /// </summary>
        [Test]
        [Category("Claude Test")]
        public void CheckBrowseContourMethod()
        {
            // Test Browse methods
            BGEO.DPoint3d[] tPoint = new BGEO.DPoint3d[5];
            tPoint = new BGEO.DPoint3d[5];
            tPoint[0] = new BGEO.DPoint3d(100, 100, 100);
            tPoint[1] = new BGEO.DPoint3d(100, 200, 100);
            tPoint[2] = new BGEO.DPoint3d(200, 200, 100);
            tPoint[3] = new BGEO.DPoint3d(200, 100, 100);
            tPoint[4] = new BGEO.DPoint3d(150, 150, 150);
            DTM nDtmInput = new DTM();
            nDtmInput.AddPointFeature(tPoint);
            nDtmInput.Triangulate();

            nContourInterval = 0;
            ContoursBrowsingDelegate hdl3P = new ContoursBrowsingDelegate(processContours);
            // BrowseContours methods
            nContourInterval = 0;
            ContoursBrowsingCriteria contoursCriteria = new ContoursBrowsingCriteria(1);
            nDtmInput.BrowseContours(contoursCriteria, hdl3P, null);
            Assert.AreEqual(50, nContourInterval);

            nContourInterval = 0;
            contoursCriteria.SmoothingOption = DTMContourSmoothingMethod.Vertex;
            nDtmInput.BrowseContours(contoursCriteria, hdl3P, null);
            Assert.AreEqual(50, nContourInterval);

            nContourInterval = 0;
            contoursCriteria.SmoothingOption = DTMContourSmoothingMethod.Spline;
            nDtmInput.BrowseContours(contoursCriteria, hdl3P, null);
            Assert.AreEqual(50, nContourInterval);

            // BrowseContours methods with range arguments
            nContourInterval = 0;
            contoursCriteria.ZLow = tPoint[0].Z;
            contoursCriteria.ZHigh = tPoint[4].Z;
            contoursCriteria.SmoothingOption = DTMContourSmoothingMethod.None;
            nDtmInput.BrowseContours(contoursCriteria, hdl3P, null);
            Assert.AreEqual(50, nContourInterval);

            nContourInterval = 0;
            contoursCriteria.SmoothingOption = DTMContourSmoothingMethod.Vertex;
            nDtmInput.BrowseContours(contoursCriteria, hdl3P, null);
            Assert.AreEqual(50, nContourInterval);

            nContourInterval = 0;
            contoursCriteria.SmoothingOption = DTMContourSmoothingMethod.Spline;
            nDtmInput.BrowseContours(contoursCriteria, hdl3P, null);
            Assert.AreEqual(50, nContourInterval);

            // BrowseContours methods with fence arguments

            BGEO.DPoint3d[] hull = nDtmInput.GetBoundary ();

            contoursCriteria.FencePoints = hull;
            nContourInterval = 0;
            contoursCriteria.SmoothingOption = DTMContourSmoothingMethod.None;
            nDtmInput.BrowseContours(contoursCriteria, hdl3P, null);
            Assert.AreEqual(50, nContourInterval);

            nContourInterval = 0;
            contoursCriteria.SmoothingOption = DTMContourSmoothingMethod.Vertex;
            nDtmInput.BrowseContours(contoursCriteria, hdl3P, null);
            Assert.AreEqual(50, nContourInterval);

        }

        private bool processContours(BGEO.DPoint3d[] tPoint, BGEO.DPoint3d direction, System.Object oArg)
        {
            double expectedZ = 100 + nContourInterval * 1;
            if (expectedZ != tPoint[0].Z)
            {
                nContourInterval--;
                expectedZ = 100 + nContourInterval * 1;
            }

            Assert.AreEqual(expectedZ, tPoint[0].Z);
            nContourInterval++;
            return true;
        }

        private bool processSlopeIndicator(bool major, BGEO.DPoint3d startPoint, BGEO.DPoint3d endPoint, System.Object oArg)
        {
            return true;
        }

        private bool processPointFeatures(DTMDynamicFeatureType featureType, BGEO.DPoint3d point, System.Object oArg)
        {
            Assert.AreEqual(150, point.X);
            Assert.AreEqual(150, point.Y);
            Assert.AreEqual(150, point.Z);
            return true;
        }

        private bool processPointsFeatures(DTMDynamicFeatureInfo info,  System.Object oArg)
        {
            if (oArg != null)
            {
                ArrayList al = oArg as ArrayList;
                al.Add (info.FeaturePoints);
            }

            return true;
        }

    }

    /// <summary>
    /// Memory usage DTM tests
    /// </summary>
    [TestFixture]
    public class MemoryUsageTestDTM
    {
        private System.IFormatProvider _formatProvider = System.Globalization.NumberFormatInfo.CurrentInfo;

        /// <summary>
        ///  Constructor
        /// </summary>
        public MemoryUsageTestDTM()
        {
        }


        /// <summary>
        /// Check memory usage
        /// </summary>
        [Test]
        [Category("Claude Test")]
        public void CheckDTM_Memory()
        {
            for (int iIter = 0; iIter < 100; iIter++)
            {
                DTM loopDtmInput = new DTM();
                // Test DTMInput method
                BGEO.DPoint3d[] looptPoint = new BGEO.DPoint3d[4];
                looptPoint[0] = new BGEO.DPoint3d(100, 100, 100);
                looptPoint[1] = new BGEO.DPoint3d(100, 200, 100);
                looptPoint[2] = new BGEO.DPoint3d(200, 200, 100);
                looptPoint[3] = new BGEO.DPoint3d(200, 100, 100);
                DTMFeatureId spotId1 = loopDtmInput.AddPointFeature(looptPoint);
                DTMFeature spotFeature = loopDtmInput.GetFeatureById(spotId1);
                DTMSpot dtmSpot = spotFeature as DTMSpot;

                BGEO.DPoint3d[] tPointSpot = dtmSpot.GetPoints();
                Assert.AreEqual(4, looptPoint.Length);

                loopDtmInput.Dispose();
            }
        }
    }

    internal class utilDtm
    {
        internal static string getFullPathName(string filename)
        {
            string path = Helper.GetTestDataLocation () +"Bentley.Civil.Dtm.Light.NUnit.dll\\";
            return Path.GetFullPath(Path.Combine(path, filename));
        }
    }
}