using System;
using System.Text;
using System.IO;
using BGEO = Bentley.GeometryNET;
using NUnit.Framework;

namespace Bentley.TerrainModelNET.NUnit
{
    
    /// <summary>
    /// StoreDTM
    /// </summary>
    /// <category>DTM</category>
    /// <author></author>
    /// <Date></Date>
    [TestFixture]
    public class DTMStoreFeatures  : DTMUNitTest
    {
        //List constant string variables
        static String TinFile = "site.tin";
        static String DTMName = "NEW.dtm";
        static String DataDirectory = @"\Bentley.Civil.Dtm.Light.NUnit.dll\DTMBrowse\";
        static String DataPath = Helper.GetTestDataLocation () + DataDirectory + TinFile;
        double falseLowDepth = 0.00;
        int pointsTraced = 0;


        // Create a new DTM from a given TIN file

        DTM nDTM = new DTM() ;

        /// <summary>
        /// DTMWriteRidgeLinesToDTM
        /// </summary>
        /// <category>DTM Browse</category>
        /// <author>Ian Mackay</author>
        /// <Date>24/Dec/2007</Date>
        [Test]
        [Category("DTMWriteRidgeLinesToDTM")]
        public void DTMWriteRidgeLinesToDTM()
        {
            // Check Dtm Test Level
            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
  //          IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

            DTM xDTM = DTM.CreateFromGeopakTinFile(DataPath);

            DateTime StartTime, EndTime;

            Console.WriteLine("Writing RidgeLines To DTM");

            try
            {
                DynamicFeaturesBrowsingDelegate RidgeLines = new DynamicFeaturesBrowsingDelegate(processTheseFeatures);
                RidgeLinesBrowsingCriteria ridgeCriteria = new RidgeLinesBrowsingCriteria() ;
                StartTime = DateTime.Now;
                xDTM.BrowseRidgeLines(ridgeCriteria,RidgeLines,null);
                EndTime = DateTime.Now;
                TimeSpan ElapsedTime = EndTime - StartTime;

                Console.WriteLine("{0} Ridgeline points traced in {1:0.00} seconds.", pointsTraced, ElapsedTime.TotalSeconds);
                Console.WriteLine("Number Of Dtm Points = {0}",nDTM.VerticesCount);
                pointsTraced = 0;
    //            SaveTheFile();
                xDTM.Dispose();
              
            }
            catch (System.Exception e)
            {
                Console.WriteLine("ERROR: {0}", e);
            }
        }


        /// <summary>
        /// DTMWriteContoursToDTM
        /// </summary>
        /// <category></category>
        /// <author>Ian Mackay</author>
        /// <Date>24/Dec/2007</Date>
        [Test]
        [Category("DTMWriteContoursToDTM")]
        public void DTMWriteContoursToDTM()
        {
            // Check Dtm Test Level
            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
//            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

            DTM xDTM = DTM.CreateFromGeopakTinFile(DataPath);

             DateTime StartTime, EndTime;

            Console.WriteLine("DTMWriteContoursToDTM running");
            ContoursBrowsingDelegate ContourBrowser = new ContoursBrowsingDelegate(processContourFeatures);
            ContoursBrowsingCriteria criteria = new ContoursBrowsingCriteria(1.0)  ;

            criteria.SmoothingOption = DTMContourSmoothingMethod.None;

            StartTime = DateTime.Now;
            xDTM.BrowseContours(criteria, processContourFeatures,null);
            EndTime = DateTime.Now;
            TimeSpan ElapsedTime = EndTime - StartTime;

            Console.WriteLine("{0} Contour points traced in {1:0.00} seconds.", pointsTraced, ElapsedTime.TotalSeconds);
            pointsTraced = 0;
            Console.WriteLine("Number Of Dtm Points = {0}", nDTM.VerticesCount);
            //          SaveTheFile();
            xDTM.Dispose();
        }

        /// <summary>
        /// DTMMaximumAscentPath
        /// </summary>
        /// <category>DTM</category>
        /// <author>Ian Mackay</author>
        /// <Date>24/Dec/07</Date>

        [Test]
        [Category("DTMMaximumAscentPath")]
        public void DTMMaximumAscentPath()
        {
            // Check Dtm Test Level
            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
    //        IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

            DTM xDTM = DTM.CreateFromGeopakTinFile(DataPath);
            
            // Create a new DTM from a given TIN file

             Console.WriteLine("DTMMaximumAscentPath running");
            BGEO.DPoint3d StartPoint = new BGEO.DPoint3d();
            StartPoint.X = 0.0;
            StartPoint.Y = 0.0;
            StartPoint.Z = 0.0;
            DateTime StartTime, EndTime;


            // Trace and time ascent paths from the triangle centroid
            DynamicFeaturesBrowsingDelegate AscentBrowser = new DynamicFeaturesBrowsingDelegate(AscentPaths);

            StartTime = DateTime.Now;
            xDTM.BrowseTriangles(new TrianglesBrowsingCriteria(),AscentBrowser,xDTM);
            EndTime = DateTime.Now;
            TimeSpan ElapsedTime = EndTime - StartTime;

            Console.WriteLine("{0} Ascent paths traced in {1:0.00} seconds.", pointsTraced, ElapsedTime.TotalSeconds);
            pointsTraced = 0;
            Console.WriteLine("Number Of Dtm Points = {0}", nDTM.VerticesCount);
            //            SaveTheFile();
            xDTM.Dispose();
        }

        bool browseTrace(DTMDynamicFeatureInfo info, object userArg)
        {
            // Console.WriteLine("FeatureType = {0} **** Number Of Points = {1}", featureType, points.Length);
            if (userArg != null)
            {
                DTM nDTM = (DTM)userArg;
                nDTM.AddLinearFeature(info.FeaturePoints, DTMFeatureType.Breakline);
            }
            return true;
        }

        private bool AscentPaths(DTMDynamicFeatureInfo info, System.Object oArg)
        {
            var TrianglePoints = info.FeaturePoints;
            BGEO.DPoint3d Centroid = new BGEO.DPoint3d();
            pointsTraced++;

            DTM xDTM = ( DTM) oArg;

            // Calculate the centroid of the triangle
            Centroid.X = (TrianglePoints[0].X + TrianglePoints[1].X + TrianglePoints[2].X) / 3.0;
            Centroid.Y = (TrianglePoints[0].Y + TrianglePoints[1].Y + TrianglePoints[2].Y) / 3.0;
            Centroid.Z = (TrianglePoints[0].Z + TrianglePoints[1].Z + TrianglePoints[2].Z) / 3.0;
            DynamicFeaturesBrowsingDelegate traceBrowsingDelegate = new DynamicFeaturesBrowsingDelegate(browseTrace);
            xDTM.TraceMaximumAscent(falseLowDepth, Centroid, traceBrowsingDelegate, nDTM );
            return true;
        }

        /// <summary>
        /// DTMMaximumDescentPath
        /// </summary>
        /// <category>DTM</category>
        /// <author>Ian Mackay</author>
        /// <Date>24/Dec/07</Date>
        [Test]
        [Category("DTMMaximumDescentPath")]
        public void DTMMaximumDescentPath()
        {
            // Check Dtm Test Level
            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
      //      IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test


            DTM xDTM = DTM.CreateFromGeopakTinFile(DataPath);

            Console.WriteLine("DTMMaximumDescentPath running");
            BGEO.DPoint3d StartPoint = new BGEO.DPoint3d();
            StartPoint.X = 0.0;
            StartPoint.Y = 0.0;
            StartPoint.Z = 0.0;
            DateTime StartTime, EndTime;


            // Trace and time descent paths from the triangle centroid
            DynamicFeaturesBrowsingDelegate TriangleBrowser = new DynamicFeaturesBrowsingDelegate(DescentPaths);
            TrianglesBrowsingCriteria criteria = new TrianglesBrowsingCriteria();

            StartTime = DateTime.Now;
            xDTM.BrowseTriangles(criteria,TriangleBrowser,xDTM);
            EndTime = DateTime.Now;
            TimeSpan ElapsedTime = EndTime - StartTime;
            SaveTheFile();

            Console.WriteLine("{0} Descent paths traced in {1:0.00} seconds.", pointsTraced, ElapsedTime.TotalSeconds);
            pointsTraced = 0;
            Console.WriteLine("Number Of Dtm Points = {0}", nDTM.VerticesCount);
            //            SaveTheFile();
            xDTM.Dispose();
        }

        private bool DescentPaths (DTMDynamicFeatureInfo info, System.Object oArg)
            {
            var TrianglePoints = info.FeaturePoints;
            BGEO.DPoint3d Centroid = new BGEO.DPoint3d ();
            pointsTraced++;
            DTM xDTM = (DTM)oArg;

            // Calculate the centroid of the triangle
            Centroid.X = (TrianglePoints[0].X + TrianglePoints[1].X + TrianglePoints[2].X) / 3.0;
            Centroid.Y = (TrianglePoints[0].Y + TrianglePoints[1].Y + TrianglePoints[2].Y) / 3.0;
            Centroid.Z = (TrianglePoints[0].Z + TrianglePoints[1].Z + TrianglePoints[2].Z) / 3.0;
            DynamicFeaturesBrowsingDelegate traceBrowsingDelegate = new DynamicFeaturesBrowsingDelegate (browseTrace);
            xDTM.TraceMaximumDescent (falseLowDepth, Centroid, traceBrowsingDelegate, nDTM);
            return true;
            }


        /// <summary>
        /// processContourFeatures
        /// </summary>
        /// <category>DTM</category>
        /// <author>Ian Mackay</author>
        /// <Date>24/Dec/2007</Date>
        /// <param name="points"></param>
        /// <param name="direction"></param>
        /// <param name="oArg"></param>
        /// <returns></returns>
        private bool processContourFeatures(BGEO.DPoint3d[] points, BGEO.DPoint3d direction, System.Object oArg)
        {
            nDTM.AddLinearFeature(points, DTMFeatureType.Breakline);
            pointsTraced++;
            return true;
        }



        /// <summary>
        /// SaveTheFile
        /// </summary>
        /// <category>DTM</category>
        /// <author>Ian Mackay</author>
        /// <Date>24/Dec/2007</Date>
        private void SaveTheFile()
        {
            Console.WriteLine("Saving \"{0}\"", Helper.GetTestDataLocation () + DataDirectory + DTMName);
 //           nDTM.Save(Helper.GetTestDataLocation () + DataDirectory + DTMName);
        }


        /// <summary>
        /// processTheseFeatures
        /// </summary>
        /// <category>DTM Browse</category>
        /// <author>Ian Mackay</author>
        /// <Date>24/Dec/2007</Date>
        /// <param name="featureType"></param>
        /// <param name="points"></param>
        /// <param name="oArg"></param>
        /// <returns></returns>
        private bool processTheseFeatures(DTMDynamicFeatureInfo info, System.Object oArg)
        {
            nDTM.AddLinearFeature(info.FeaturePoints, DTMFeatureType.Breakline);
            pointsTraced++;
            return true;
        }

    }
}
