using System;
using System.Text;
using BGEO = Bentley.GeometryNET;
using Bentley.TerrainModelNET;
using NUnit.Framework;


namespace Bentley.TerrainModelNET.NUnit
{
    using Bentley.TerrainModelNET;

    /// <summary>
    /// Class DTMBrowse
    /// </summary>
    /// <category></category>
    [TestFixture]
    public class DTMBrowse : DTMUNitTest
    {
        private System.IFormatProvider _formatProvider = System.Globalization.NumberFormatInfo.CurrentInfo;
        //List constant string variables
        static String TinFile = "site.tin";
        static String DataDirectory = @"Bentley.Civil.Dtm.Light.NUnit.dll\DTMBrowse\";
        static String DataPath = Helper.GetTestDataLocation () + DataDirectory + TinFile;
        static double falseLowDepth = 0.0;
        static Double ContourInterval = 1.0;
        static int pointsTraced = 0;
        static int featuresTraced = 0;
        
        static long totalBrowsePoints = 0 ;
        static long totalBrowseFeatures = 0;
 
        /// <summary>
        /// Constructor
        /// </summary>
        [TestFixtureSetUp]
        public void Setup()
        {
            // Check that the file exists
            if (!System.IO.File.Exists(DataPath))
            {
                Console.WriteLine("DTMBrowse High Points Test failed because the file \"{0}\" does not exist.\n", DataPath);
                throw new Exception("\nCANNOT FIND FILE: The test data file does not exist.\nFilename = " + DataPath);
            }

        }

        /// <summary>
        /// DTMBrowsePoints
        /// </summary>
        /// <category>DTM Browse</category>
        /// <author>Rob Cormack</author>
        /// <date>11MAY2009</date>
        [Test]
        [Category("DTMBrowse")]
        public void DTMBrowsePoints()
        {
            IgnoreFirebug();

            DateTime StartTime = DateTime.Now;
            TimeSpan ElapsedTime = DateTime.Now - StartTime;
            Console.WriteLine("Browsing Points");


            String PointPath = @"\Bentley.Civil.Dtm.NUnit.dll\DTMTriangulationTest\DTMTriangulationDatTest\";
            String PointFolder = Helper.GetTestDataLocation () + PointPath;
            String PointFile = PointFolder + "BreakLines01.dat";

            // Check Test File Exists

            if (!System.IO.File.Exists(PointFile)) throw new Exception("Test File " + PointFile + " does not exist");

            // Import Untriangulated Dat File

            Console.WriteLine("Importing {0}", PointFile);
            StartTime = DateTime.Now;
            DTM datDTM = DTM.CreateFromGeopakDatFile(PointFile);
            ElapsedTime = DateTime.Now - StartTime;
            Console.WriteLine("Import Time = {0:0.0000} Seconds", ElapsedTime.TotalSeconds);

            // Set Point Browser 

            PointsBrowsingDelegate pointBrowser = new PointsBrowsingDelegate(pointsBrowser);

            // Browse None Feature Points

            Console.WriteLine("Browsing None Feature Points From Untriangulated DTM");
            PointsBrowsingCriteria pointCriteria = new PointsBrowsingCriteria();
            pointCriteria.MaxPoints = 50000;
            pointCriteria.SelectionOption = PointSelection.NoneFeaturePoints;

            StartTime = DateTime.Now;
            datDTM.BrowsePoints(pointCriteria, pointBrowser, null);
            ElapsedTime = DateTime.Now - StartTime;
            Console.WriteLine("Total None Feature Points Browsed = {0} Time = {1:0.0000} Seconds", totalBrowsePoints, ElapsedTime.TotalSeconds);
            long totalNoneFeaturePointsBrowsed = totalBrowsePoints;
            totalBrowsePoints = 0;

            // Browse Feature Points 

            Console.WriteLine("Browsing Feature Points From Untriangulated DTM");
            pointCriteria.SelectionOption = PointSelection.FeaturePoints;
            StartTime = DateTime.Now;
            datDTM.BrowsePoints(pointCriteria, pointBrowser, null);
            ElapsedTime = DateTime.Now - StartTime;
            Console.WriteLine("Total Feature Points Browsed = {0} Time = {1:0.0000} Seconds", totalBrowsePoints, ElapsedTime.TotalSeconds);
            long totalFeaturePointsBrowsed = totalBrowsePoints;
            totalBrowsePoints = 0;

            // Browse All Points

            Console.WriteLine("Browsing All Points From Untriangulated DTM");
            pointCriteria.SelectionOption = PointSelection.AllPoints;
            StartTime = DateTime.Now;
            datDTM.BrowsePoints(pointCriteria, pointBrowser, null);
            ElapsedTime = DateTime.Now - StartTime;
            Console.WriteLine("Total All Points Browsed = {0}  Time = {1:0.0000} Seconds", totalBrowsePoints, ElapsedTime.TotalSeconds);
            long totalAllPointsBrowsed = totalBrowsePoints;
            totalBrowsePoints = 0;

            // Check totalAllPointsBrowsed = totalNoneFeaturePointsBrowsed + totalFeaturePointsBrowsed

            Assert.AreEqual(totalAllPointsBrowsed, totalNoneFeaturePointsBrowsed + totalFeaturePointsBrowsed);

            // Triangulate

            Console.WriteLine("Triangulating datDTM");
            StartTime = DateTime.Now;
            datDTM.Triangulate();
            ElapsedTime = DateTime.Now - StartTime;
            Console.WriteLine("Triangulation Time = {0:0.0000} Seconds", ElapsedTime.TotalSeconds);

            // Browse Triangulated DTM

            // Browse None Feature Points 

            Console.WriteLine("Browsing None Feature Points From Triangulated DTM");
            pointCriteria.MaxPoints = 20000;
            pointCriteria.SelectionOption = PointSelection.NoneFeaturePoints;
            StartTime = DateTime.Now;
            datDTM.BrowsePoints(pointCriteria, pointBrowser, null);
            ElapsedTime = DateTime.Now - StartTime;
            Console.WriteLine("Total None Feature Points Browsed = {0} Time = {1:0.0000} Seconds", totalBrowsePoints, ElapsedTime.TotalSeconds);
            totalNoneFeaturePointsBrowsed = totalBrowsePoints;
            totalBrowsePoints = 0;

            // Browse Feature Points

            Console.WriteLine("Browsing Feature Points From Triangulated DTM");
            pointCriteria.SelectionOption = PointSelection.FeaturePoints;
            StartTime = DateTime.Now;
            datDTM.BrowsePoints(pointCriteria, pointBrowser, null);
            ElapsedTime = DateTime.Now - StartTime;
            Console.WriteLine("Total Feature Points Browsed = {0}  Time = {1:0.0000} Seconds", totalBrowsePoints, ElapsedTime.TotalSeconds);
            totalFeaturePointsBrowsed = totalBrowsePoints;
            totalBrowsePoints = 0;

            // Browse All Points

            Console.WriteLine("Browsing All Points From Triangulated DTM");
            pointCriteria.SelectionOption = PointSelection.AllPoints;
            StartTime = DateTime.Now;
            datDTM.BrowsePoints(pointCriteria, pointBrowser, null);
            ElapsedTime = DateTime.Now - StartTime;
            Console.WriteLine("Total All Points Browsed = {0}  Time = {1:0.0000} Seconds", totalBrowsePoints, ElapsedTime.TotalSeconds);
            totalAllPointsBrowsed = totalBrowsePoints;
            totalBrowsePoints = 0;

            // Check totalPointsBrowsed = totalNoneFeaturePointsBrowsed + totalFeaturePointsBrowsed

            Assert.AreEqual(totalAllPointsBrowsed, totalNoneFeaturePointsBrowsed + totalFeaturePointsBrowsed);

 
            // Browse Triangle Vertices From Triangulated DTM

            Console.WriteLine("Browsing Triangle Vertices From Triangulated DTM");
            DynamicFeaturesBrowsingCriteria dynamicFeatureCriteria = new DynamicFeaturesBrowsingCriteria();
            dynamicFeatureCriteria.CacheSize = 10000;
            DynamicFeaturesBrowsingDelegate dynamicFeatureBrowser = new DynamicFeaturesBrowsingDelegate(dynamicFeaturesBrowser);
            StartTime = DateTime.Now;
            datDTM.BrowseDynamicFeatures(dynamicFeatureCriteria, DTMDynamicFeatureType.TriangleVertex, dynamicFeatureBrowser, null);
            ElapsedTime = DateTime.Now - StartTime;
            Console.WriteLine("Total Triangle Vertices Browsed = {0} Time = {1:0.0000} Seconds", totalBrowseFeatures, ElapsedTime.TotalSeconds);
            long totalTriangleVerticesBrowsed = totalBrowseFeatures;
            totalBrowseFeatures = 0;

            // Browse Triangles From Triangulated DTM

            Console.WriteLine("Browsing Triangles");
            dynamicFeatureCriteria.CacheSize = 1000;
            StartTime = DateTime.Now;
            datDTM.BrowseDynamicFeatures(dynamicFeatureCriteria, DTMDynamicFeatureType.Triangle, dynamicFeaturesBrowser, null);
            ElapsedTime = DateTime.Now - StartTime;
            Console.WriteLine("Total Triangles Browsed = {0} Time = {1:0.0000} Seconds", totalBrowseFeatures, ElapsedTime.TotalSeconds);
            totalBrowseFeatures = 0;

            // Check totalPointsBrowsed = totalNoneFeaturePointsBrowsed + totalFeaturePointsBrowsed

            Assert.AreEqual(totalAllPointsBrowsed, totalTriangleVerticesBrowsed);

            // Destroy DTM's
            datDTM.Dispose();
        }

        private bool pointsBrowser(BGEO.DPoint3d[] tPoint, object oArg)
        {
            totalBrowsePoints = totalBrowsePoints + tPoint.Length;
            return true;
        }

        private bool dynamicFeaturesBrowser(DTMDynamicFeatureInfo info,  object oArg)
        {
            if (info.FeatureType == DTMDynamicFeatureType.Triangle) ++totalBrowseFeatures;
            if (info.FeatureType == DTMDynamicFeatureType.TriangleVertex) totalBrowseFeatures = totalBrowseFeatures + info.FeaturePoints.Length;
            return true;
        }

        /// <summary>
        /// DTMBrowsePointFeatures
        /// </summary>
        /// <category>DTM Browse</category>
        /// <author>Rob Cormack</author>
        /// <date>07MAY2009</date>
        [Test]
        [Category("DTMBrowse")]
        public void DTMBrowsePointFeatures()
        {
            IgnoreFirebug();

            DateTime StartTime = DateTime.Now;
            TimeSpan ElapsedTime = DateTime.Now - StartTime;
            Console.WriteLine("Browsing Point Features Nunit Test");

            String PointPath = @"\Bentley.Civil.Dtm.NUnit.dll\DTMTriangulationTest\DTMTriangulationXyzTest\";
            String PointFolder = Helper.GetTestDataLocation () + PointPath;
            String PointFile = PointFolder + "1m.xyz";

            // Check Test File Exists

            if (!System.IO.File.Exists(PointFile)) throw new Exception("Test File "+ PointFile +" does not exist");


            // Create DTM To Store Point Features
            DTM testDTM = new DTM();

            // Import Untriangulated XYZ File
            Console.WriteLine("Importing {0}",PointFile);
            StartTime = DateTime.Now;
            DTM xyzDTM = DTM.CreateFromXyzFile(PointFile);
            ElapsedTime = DateTime.Now - StartTime;
            Console.WriteLine("Import Time = {0:0.0000} Seconds", ElapsedTime.TotalSeconds);

            // Set Point Browser 

            PointsBrowsingDelegate pointBrowser = new PointsBrowsingDelegate(browsePointsAndAddPointFeatures);

            // Browse None Feature Points In Lots Of 1000 Points and Store the 1000 Point Lots As Point Features In testDTM

            Console.WriteLine("Browsing None Feature Points In Untriangulated xyzDTM");
            PointsBrowsingCriteria pointCriteria = new PointsBrowsingCriteria();
            pointCriteria.MaxPoints = 1000;
            pointCriteria.SelectionOption = PointSelection.NoneFeaturePoints;
            StartTime = DateTime.Now;
            xyzDTM.BrowsePoints(pointCriteria, pointBrowser,testDTM);
            ElapsedTime = DateTime.Now - StartTime;
            Console.WriteLine("Total None Feature Points Browsed = {0} Time = {1:0.0000} Seconds", totalBrowsePoints, ElapsedTime.TotalSeconds);
            long totalNoneFeaturePointsBrowsed = totalBrowsePoints ;
            totalBrowsePoints = 0;

            // Browse Point Features in Untriangulated testDTM 

            totalBrowsePoints = 0;
            totalBrowseFeatures = 0;
            Console.WriteLine("Browsing Point Features In Untriangulated testDTM");
            PointFeaturesBrowsingCriteria pointFeaturesCriteria = new PointFeaturesBrowsingCriteria();
            StartTime = DateTime.Now;
            testDTM.BrowsePointFeatures (pointFeaturesCriteria, pointFeaturesBrowser, null);
            ElapsedTime = DateTime.Now - StartTime;
            Console.WriteLine("Total Point Features Browsed = {0} Total Feature Points Browsed = {1} Time = {2:0.0000} Seconds",totalBrowseFeatures, totalBrowsePoints, ElapsedTime.TotalSeconds);
            long totalPointsBrowsed = totalBrowsePoints ;
            long totalPointFeaturesBrowsed = totalBrowseFeatures ;
            totalBrowsePoints = 0;
            totalBrowseFeatures = 0;

            // Triangulate

            Console.WriteLine("Triangulating testDTM");
            StartTime = DateTime.Now;
            testDTM.Triangulate();
            ElapsedTime = DateTime.Now - StartTime;
            Console.WriteLine("Triangulation Time = {0:0.0000} Seconds", ElapsedTime.TotalSeconds);

            // Browse Point Features in Triangulated testDTM 

            Console.WriteLine("Browsing Point Features In Tiangulated testDTM");
            StartTime = DateTime.Now;
            testDTM.BrowsePointFeatures(pointFeaturesCriteria, pointFeaturesBrowser, null);
            ElapsedTime = DateTime.Now - StartTime;
            Console.WriteLine("Total Point Features Browsed = {0} Total Feature Points Browsed = {1} Time = {2:0.0000} Seconds", totalBrowseFeatures, totalBrowsePoints, ElapsedTime.TotalSeconds);


            // Check The Total Number Of Point Features Browsed In The Same For The Untriangulated And Triangulated testDTM

            Assert.AreEqual(totalPointFeaturesBrowsed, totalBrowseFeatures );
            totalBrowsePoints = 0;
            totalBrowseFeatures = 0;


            // Destroy DTM's

            xyzDTM.Dispose();
            testDTM.Dispose();
        }

        private bool browsePointsAndAddPointFeatures(BGEO.DPoint3d[] tPoint, object oArg)
        {
            DTM testDTM = (DTM)oArg;
            DTMFeatureId dtmFeatureId = testDTM.AddPointFeature(tPoint);
            totalBrowsePoints = totalBrowsePoints + tPoint.Length;
            return true;
        }

        private bool pointFeaturesBrowser(DTMFeatureInfo dtmFeatureInfo, object oArg)
        {
            ++totalBrowseFeatures;
            totalBrowsePoints = totalBrowsePoints + dtmFeatureInfo.Points.Length;
            return true;
        }


        /// <summary>
        /// DTMBrowseHighPoints
        /// </summary>
        /// <category>DTM Browse</category>
        /// <author>Ian Mackay</author>
        /// <date>12/2007</date>
        [Test]
        [Category("DTMBrowse")]
        public void DTMBrowseHighPoints()
        {
            Console.Write("Browsing High Points");
            try
            {
                // Open DTM file
                DTM nDtm1 = DTM.CreateFromFile(DataPath);

                SinglePointFeaturesBrowsingDelegate hdlP = new SinglePointFeaturesBrowsingDelegate(processTheseFeatures);
                DateTime StartTime = DateTime.Now;
                HighPointsBrowsingCriteria criteria = new HighPointsBrowsingCriteria();
                nDtm1.BrowseHighPoints(criteria, hdlP, null);
                DateTime EndTime = DateTime.Now;
                TimeSpan ElapsedTime = EndTime - StartTime;

                nDtm1.Dispose();
                WriteTheResults(pointsTraced, ElapsedTime.TotalSeconds);
            }
            catch (System.Exception e)
            {
                Console.WriteLine("ERROR: {0}", e);
                throw new Exception(e.ToString());
            }
        }
        
        /// <summary>
        /// DTMBrowseLowPoints
        /// </summary>
        /// <category>DTM Browse</category>
        /// <author>Ian Mackay</author>
        /// <date>12/2007</date>
        [Test]
        [Category("DTMBrowse")]
        public void DTMBrowseLowPoints()
        {
            Console.Write("Browsing Low Points");
            try
            {
                // Open DTM file
                DTM nDtm1 = DTM.CreateFromFile(DataPath);

                SinglePointFeaturesBrowsingDelegate hdlP = new SinglePointFeaturesBrowsingDelegate(processTheseFeatures);

                DateTime StartTime = DateTime.Now;
                LowPointsBrowsingCriteria criteria = new LowPointsBrowsingCriteria();
                nDtm1.BrowseLowPoints(criteria, falseLowDepth, hdlP, null);
                DateTime EndTime = DateTime.Now;
                TimeSpan ElapsedTime = EndTime - StartTime;

                nDtm1.Dispose();

                WriteTheResults(pointsTraced, ElapsedTime.TotalSeconds);
            }
            catch (System.Exception e)
            {
                Console.WriteLine("ERROR: {0}", e);
                throw new Exception(e.ToString());
            }
        }

        /// <summary>
        /// DTMBrowseContours
        /// </summary>
        /// <category>DTM Browse</category>
        /// <author>Ian Mackay</author>
        /// <date>12/2007</date>
        [Test]
        [Category("DTMBrowse")]
        public void DTMBrowseContours()
        {
            try
            {
                // Create a DTM object for testing the smoothings
                DTM nDtm1 = DTM.CreateFromFile(DataPath);

                // Browse The Contours using the various smoothing methods
                BrowseTheContours(nDtm1, DTMContourSmoothingMethod.None);
                BrowseTheContours(nDtm1, DTMContourSmoothingMethod.Vertex);
                BrowseTheContours(nDtm1, DTMContourSmoothingMethod.Spline);
                BrowseTheContours(nDtm1, DTMContourSmoothingMethod.SplineWithoutOverLapDetection);

                // Cleanup
                nDtm1.Dispose();
            }
            catch (System.Exception e)
            {
                Console.WriteLine("ERROR: {0}", e);
                throw new Exception(e.ToString());
            }

        }

        /// <summary>
        /// BrowseTheContours
        /// </summary>
        /// <category></category>
        /// <author>Ian Mackay</author>
        /// <date>20 Dec 2007</date>
        /// <param name="TestDTM"></param>
        /// <param name="Smoothing"></param>
        private void BrowseTheContours(DTM TestDTM, DTMContourSmoothingMethod Smoothing)
        {
            Console.Write("Browsing Contours with Smoothing = {0}", Smoothing.ToString());
            ContoursBrowsingDelegate ContourBrowser = new ContoursBrowsingDelegate(processTheseFeatures);
            ContoursBrowsingCriteria contoursCriteria = new ContoursBrowsingCriteria(ContourInterval);
            contoursCriteria.SmoothingOption = Smoothing ;
            switch ( Smoothing )
              {
               case DTMContourSmoothingMethod.None :
               break ;

               case DTMContourSmoothingMethod.Vertex :
                  contoursCriteria.LinearSmoothingFactor = 0.45 ;
                  break ;

               case DTMContourSmoothingMethod.Spline:
                  contoursCriteria.SplineSmoothingFactor = 2.5;
                  contoursCriteria.SplineDensification = 5;
                  break;

               case DTMContourSmoothingMethod.SplineWithoutOverLapDetection:
                  contoursCriteria.SplineSmoothingFactor = 0.0 ;
                  contoursCriteria.SplineDensification = 5;
                  break;

              }

            DateTime StartTime = DateTime.Now;
                TestDTM.BrowseContours(contoursCriteria, ContourBrowser, null);
            DateTime EndTime = DateTime.Now;
            TimeSpan ElapsedTime = EndTime - StartTime;

            WriteTheResults(pointsTraced, ElapsedTime.TotalSeconds);
        }

        /// <summary>
        /// browsePointContours
        /// </summary>
        /// <category>DTM Browse</category>
        /// <author>Ian Mackay</author>
        /// <date>12/2007</date>

        long contourPointsTraced = 0;
        long contoursTraced = 0;

        private bool browsePointContours(BGEO.DPoint3d[] tPoint, BGEO.DPoint3d contourDirection, object oArg)
        {
             // Used when browsing Point Contours
             ++contoursTraced ;
             contourPointsTraced = contourPointsTraced + tPoint.Length;
            return true;
        }

        /// <summary>
        /// DTMBrowsePointContours
        /// </summary>
        /// <category>DTM Browse</category>
        /// <author>Rob Cormack</author>
        /// <date>7/2010</date>
        [Test]
        [Category("DTMBrowse")]
        public void DTMBrowsePointContours()
        {
            try
            {
                // Create a DTM object for testing the point contouring
                DTM dtm = DTM.CreateFromFile(DataPath);

                ContoursBrowsingDelegate contourBrowser = new ContoursBrowsingDelegate(browsePointContours);
                ContoursBrowsingCriteria contourCriteria = new ContoursBrowsingCriteria(0.0);

                BGEO.DRange3d range = dtm.Range3d;
                double xMin = range.Low.X;
                double xMax = range.High.X;
                double Y = (range.Low.Y + range.High.Y) / 2.0;

                DateTime StartTime = DateTime.Now;
                for (double X = range.Low.X; X <= range.High.X; X = X + 1.0)
                {
                    dtm.ContourAtPoint(contourCriteria, contourBrowser, X, (range.Low.Y + range.High.Y) / 2.0, null);

                }
                DateTime EndTime = DateTime.Now;
                TimeSpan ElapsedTime = EndTime - StartTime;
                WriteTheResults(pointsTraced, ElapsedTime.TotalSeconds);
                Console.WriteLine("Contour Points Traced = {0} Contours Traced = {1} Elapsed Time = {2} seconds", contourPointsTraced,contoursTraced, ElapsedTime);

                // Browse The Contours using the various smoothing methods

                // Cleanup
                dtm.Dispose();
            }
            catch (System.Exception e)
            {
                Console.WriteLine("ERROR: {0}", e);
                throw new Exception(e.ToString());
            }

        }


        private bool triangleMeshBrowser(BGEO.DPoint3d[] meshPoints, int[] meshFaces, object oArg)
        {
            // The Indecies In meshFaces Are 1 based adressing For MDL PolyFace Mesh
            // You May Have To Divide meshFaces / 3 . Can Not remember
            
            Console.WriteLine("numTriangles = {0} numMeshPoints = {1} numMeshFaces = {2}", meshFaces.Length / 3, meshPoints.Length, meshFaces.Length);

            //  Write Out Triangles

            Boolean  dbg = false ;
            if (dbg == true )
            {
                int point1, point2, point3 ;
                for (int face = 0; face < meshFaces.Length; face = face + 3)
                {
                    point1 = meshFaces[face]   - 1;          // subtract 1 from point indice required for mdlMesh_newPolyface
                    point2 = meshFaces[face+1] - 1;          // subtract 1 from point indice required for mdlMesh_newPolyface
                    point3 = meshFaces[face+2] - 1;          // subtract 1 from point indice required for mdlMesh_newPolyface

                    Console.WriteLine(" Triangle[{0}] ** {1} {2} {3} ** {4} {5} {6} ** {7} {8} {9}", face / 3 + 1,
                                        meshPoints[point1].X, meshPoints[point1].Y, meshPoints[point1].Z,
                                        meshPoints[point2].X, meshPoints[point2].Y, meshPoints[point2].Z,
                                        meshPoints[point3].X, meshPoints[point3].Y, meshPoints[point3].Z);
                }
            }
            return true;
        }



        /// <summary>
        /// DTMBrowseTriangles
        /// </summary>
        /// <category>DTM Browse</category>
        /// <author>Ian Mackay</author>
        /// <date>12/2007</date>
        [Test]
        [Category("DTMBrowse")]
        public void DTMBrowseTriangles()
        {
            Console.WriteLine("Browsing Triangle Mesh");

            // Browse Triangle Mesh

            String trgPath = @"Bentley.Civil.Dtm.Light.NUnit.dll\DTMBrowse\";
            String trgFolder = Helper.GetTestDataLocation () + trgPath;
            String trgFile = trgFolder + "problem.dtm";
            
            DTM trgDTM = DTM.CreateFromFile(trgFile);

            Console.WriteLine("Browsing Triangle Mesh From Triangulated DTM");
            TriangleMeshBrowsingCriteria triangleMeshBrowsingCriteria = new TriangleMeshBrowsingCriteria();
            triangleMeshBrowsingCriteria.MaxTriangles = 50000 ;
            TriangleMeshBrowsingDelegate triangleMeshBrowsingDelegate = new TriangleMeshBrowsingDelegate(triangleMeshBrowser);
            trgDTM.BrowseTriangleMesh(triangleMeshBrowsingCriteria, triangleMeshBrowsingDelegate, null);
 
            trgDTM.Dispose();

            // Browse Triangles

            DTM nDtm1 = DTM.CreateFromFile(DataPath);

            Console.WriteLine("Browsing Triangles");
            DynamicFeaturesBrowsingDelegate TriangleBrowser = new DynamicFeaturesBrowsingDelegate(processTheseFeatures);
            DateTime StartTime = DateTime.Now;
            TrianglesBrowsingCriteria criteria = new TrianglesBrowsingCriteria();
            nDtm1.BrowseTriangles(criteria, TriangleBrowser, null);
            DateTime EndTime = DateTime.Now;
            TimeSpan ElapsedTime = EndTime - StartTime;
            WriteTheResults(pointsTraced, ElapsedTime.TotalSeconds);

            // Use Generic Dynamic Features Browsing Delegate

            // Browse Dynamic Triangle Feature
            DynamicFeaturesBrowsingCriteria dymCriteria = new DynamicFeaturesBrowsingCriteria ();
            dymCriteria.CacheSize = 4;
            Console.WriteLine("Browsing Dynamic Triangle Features");
            nDtm1.BrowseDynamicFeatures(dymCriteria, DTMDynamicFeatureType.Triangle, processTheseFeatures, null);
            Console.WriteLine("Number Of Triangles Browsed = {0} ** Points Browsed = {1}",featuresTraced,pointsTraced);
            pointsTraced   = 0;
            featuresTraced = 0; 

            // Browse Dynamic Triangle Edge Feature
            Console.WriteLine("Browsing Dynamic Triangle Edge Features");
            nDtm1.BrowseDynamicFeatures(dymCriteria, DTMDynamicFeatureType.TriangleEdge, processTheseFeatures, null);
            Console.WriteLine("Number Of Triangle Edges Browsed = {0} ** Points Browsed = {1}", featuresTraced,pointsTraced);
            pointsTraced = 0;
            featuresTraced = 0; 

            // Triangle Dynamic Vertex Feature
            Console.WriteLine("Browsing Dynamic Triangle Vertex Features");
            nDtm1.BrowseDynamicFeatures(dymCriteria, DTMDynamicFeatureType.TriangleVertex, processTheseFeatures, null);
            Console.WriteLine("Number Of Triangle Vertices Browsed = {0}", pointsTraced);
            pointsTraced = 0;
            featuresTraced = 0; 

          
            nDtm1.Dispose();

        }

        /// <summary>
        /// DTMBrowseTrianglesByRange
        /// </summary>
        /// <category>DTM Browse</category>
        /// <author>Ian Mackay</author>
        /// <date>12/2007</date>
        [Test]
        [Category("DTMBrowse")]
        public void DTMBrowseTrianglesByRange()
        {
            Console.Write("Browsing Triangles by Range");
            try
            {

                DTM nDtm1 = DTM.CreateFromFile(DataPath);
                DynamicFeaturesBrowsingDelegate TriangleBrowser = new DynamicFeaturesBrowsingDelegate(processTheseFeatures);

                DateTime StartTime = DateTime.Now;
                TrianglesBrowsingCriteria criteria = new TrianglesBrowsingCriteria();
                nDtm1.BrowseTriangles(criteria, TriangleBrowser, null);
                DateTime EndTime = DateTime.Now;
                TimeSpan ElapsedTime = EndTime - StartTime;

                nDtm1.Dispose();

                WriteTheResults(pointsTraced, ElapsedTime.TotalSeconds);
            }
            catch (System.Exception e)
            {
                Console.WriteLine("ERROR: {0}", e);
                throw new Exception(e.ToString());
            }

        }

        /// <summary>
        /// DTMBrowseTriangleEdges
        /// </summary>
        /// <category>DTM Browse</category>
        /// <author>Ian Mackay</author>
        /// <date>12/2007</date>
        [Test]
        [Category("DTMBrowse")]
        public void DTMBrowseTriangleEdges()
        {
            Console.Write("Browsing Triangle Edges");
            try
            {
                DTM nDtm1 = DTM.CreateFromFile(DataPath);
                
                DynamicFeaturesBrowsingDelegate TriangleEdgeBrowser = new DynamicFeaturesBrowsingDelegate (processTheseFeatures);

                DateTime StartTime = DateTime.Now;
                TriangleEdgesBrowsingCriteria criteria = new TriangleEdgesBrowsingCriteria();
                nDtm1.BrowseTriangleEdges(criteria, TriangleEdgeBrowser, null);
                DateTime EndTime = DateTime.Now;
                TimeSpan ElapsedTime = EndTime - StartTime;

                nDtm1.Dispose();

                WriteTheResults(pointsTraced, ElapsedTime.TotalSeconds);
            }
            catch (System.NullReferenceException e)
            {
                Console.WriteLine("\nERROR: {0}\nThe file \"{1}\" could not be found.", e, DataPath);
            }
        }

        /// <summary>
        /// DTMBrowseRidgeLines
        /// </summary>
        /// <category>DTM Browse</category>
        /// <author>Ian Mackay</author>
        /// <date>12/2007</date>
        [Test]
        [Category("DTMBrowse")]
        public void DTMBrowseRidgeLines()
        {
            Console.Write("Browsing Ridge Lines");
            try
            {
                DTM nDtm1 = DTM.CreateFromFile(DataPath);

                DynamicFeaturesBrowsingDelegate RidgeLines = new DynamicFeaturesBrowsingDelegate(processTheseFeatures);

                DateTime StartTime = DateTime.Now;
                RidgeLinesBrowsingCriteria criteria = new RidgeLinesBrowsingCriteria();
                nDtm1.BrowseRidgeLines(criteria, RidgeLines, null);
                DateTime EndTime = DateTime.Now;
                TimeSpan ElapsedTime = EndTime - StartTime;

                nDtm1.Dispose();

                WriteTheResults(pointsTraced, ElapsedTime.TotalSeconds);
            }
            catch (System.NullReferenceException e)
            {
                Console.WriteLine("\nERROR: {0}\nThe file \"{1}\" could not be found.", e, DataPath);
            }

        }
        /// <summary>
        /// DTMBrowseSumpLines
        /// </summary>
        /// <category>DTM Browse</category>
        /// <author>Ian Mackay</author>
        /// <date>12/2007</date>
        [Test]
        [Category("DTMBrowse")]
        public void DTMBrowseSumpLines()
        {
            Console.Write("Browsing Sump Lines");
            try
            {
                DTM nDtm1 = DTM.CreateFromFile(DataPath);
                DynamicFeaturesBrowsingDelegate SumpLines = new DynamicFeaturesBrowsingDelegate(processTheseFeatures);

                DateTime StartTime = DateTime.Now;
                SumpLinesBrowsingCriteria criteria = new SumpLinesBrowsingCriteria();
                nDtm1.BrowseSumpLines(criteria, SumpLines, null);
                DateTime EndTime = DateTime.Now;
                TimeSpan ElapsedTime = EndTime - StartTime;

                nDtm1.Dispose();

                WriteTheResults(pointsTraced, ElapsedTime.TotalSeconds);
            }
            catch (System.NullReferenceException e)
            {
                Console.WriteLine("\nERROR: {0}\nThe file \"{1}\" could not be found.", e, DataPath);
            }
        }


/*
        /// <summary>
        /// DTMBrowsePonds
        /// </summary>
        /// <category>DTM Browse</category>
        /// <author>Rob Cormack</author>
        /// <date>01/2011</date>
        [Test]
        [Category("DTMBrowse")]
        public void DTMBrowsePonds()
        {
            Console.WriteLine("Browsing Ponds");
            try
            {
                DTM nDtm1 = DTM.CreateFromFile(DataPath);

                DynamicFeaturesBrowsingDelegate PondsBrowser = new DynamicFeaturesBrowsingDelegate(processPonds);

                DateTime StartTime = DateTime.Now;
                nDtm1.BrowsePonds(PondsBrowser,null);
                DateTime EndTime = DateTime.Now;
                TimeSpan ElapsedTime = EndTime - StartTime;

                nDtm1.Dispose();

                WriteTheResults(pointsTraced, ElapsedTime.TotalSeconds);
            }
            catch (System.NullReferenceException e)
            {
                Console.WriteLine("\nERROR: {0}\nThe file \"{1}\" could not be found.", e, DataPath);
            }
        }
 */
        /// <summary>
        /// processPonds
        /// </summary>
        /// <param name="featureType"></param>
        /// <param name="points"></param>
        /// <param name="oArg"></param>
        /// <category>DTM Browse</category>
        /// <author>Rob.Cormack</author>
        /// <date>01/2011</date>
        /// <returns></returns>
        private bool processPonds(DTMDynamicFeatureType featureType, BGEO.DPoint3d[] points, System.Object oArg)
        {
            // Used for browsing triangles, triangle edges, sump and ridge lines

            // Each call to the BrowseTriangleEdges will return 5 points, these include the Vertical faces coords. 
            // We will only use the the first 2 of these points.
            // The Browse Sump/Ridge Lines routine will only return 2 points - CHECK WITH ROB

            bool dbg = false;
            if (dbg)
            {
                Console.WriteLine("featureType = {0} ** numPoints = {1} ",featureType,points.Length) ;
            }
            ++featuresTraced;
            pointsTraced = pointsTraced + points.Length;
            return true;
        }

        /// <summary>
        /// processTheseFeatures
        /// </summary>
        /// <param name="featureType"></param>
        /// <param name="points"></param>
        /// <param name="oArg"></param>
        /// <category>DTM Browse</category>
        /// <author>Ian Mackay</author>
        /// <date>12/2007</date>
        /// <returns></returns>
        private bool processTheseFeatures(DTMDynamicFeatureInfo info, System.Object oArg)
        {
            // Used for browsing triangles, triangle edges, sump and ridge lines

            // Each call to the BrowseTriangleEdges will return 5 points, these include the Vertical faces coords. 
            // We will only use the the first 2 of these points.
            // The Browse Sump/Ridge Lines routine will only return 2 points - CHECK WITH ROB
            ++featuresTraced ;
            pointsTraced = pointsTraced + info.FeaturePoints.Length;
            return true;
        }


        /// <summary>
        /// processPointFeatures
        /// </summary>
        /// <category>DTM Browse</category>
        /// <author>Ian Mackay</author>
        /// <date>12/2007</date>
        private bool processTheseFeatures(DTMDynamicFeatureType featureType, BGEO.DPoint3d point, System.Object oArg)
        {
            // Used when browsing Point features
            pointsTraced++;
            return true ;
        }

        /// <summary>
        /// processContourFeatures
        /// </summary>
        /// <param name="Points"></param>
        /// <param name="direction"></param>
        /// <param name="oArg"></param>
        /// <category>DTM Browse</category>
        /// <author>Ian Mackay</author>
        /// <date>12/2007</date>
        /// <returns></returns>
        private bool processTheseFeatures(BGEO.DPoint3d[] Points, BGEO.DPoint3d direction, System.Object oArg)
        {
            // Used when browsing contour features
            pointsTraced++;
            return true;
        }

        private void WriteTheResults(int Number, double RunTime)
        {
            Console.WriteLine(" - Completed tracing {0} points in {1:0.000} seconds", Number, RunTime);
            Console.WriteLine("-----------------------");
            pointsTraced = 0;
        }

    }
}
