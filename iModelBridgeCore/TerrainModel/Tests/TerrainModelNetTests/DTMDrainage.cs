using System;
using System.Text;
using System.IO;
using BGEO = Bentley.GeometryNET;
using NUnit.Framework;


namespace Bentley.TerrainModelNET.NUnit
{
    using Bentley.TerrainModelNET;

    /// <summary>
    /// Class DTMDrainage
    /// </summary>
    /// <category></category>
    [TestFixture]
    public class DTMDrainage : DTMUNitTest
    {

        /// <summary>
        /// DTM Drainage Browse Catchments
        /// </summary>
        /// <category>DTM Drainage</category>
        /// <author>Rob Cormack</author>
        /// <date>11MARCH2009</date>

        class FeatureCounts
        {
            private int featureCount;
            private DTM dtm;
            public void AddDtmFeature(BGEO.DPoint3d[] points, DTMFeatureType featureType)
            {
                dtm.AddLinearFeature(points, featureType);
            }
            public int FeatureCount { get { return featureCount; } set { featureCount = value; } }
            public DTM FeatureDTM { get { return dtm; } set { dtm = value; } }
            public FeatureCounts()
            {
                this.featureCount = 0;
                this.dtm = new DTM();
            }
        }

        bool browseCatchments(DTMDynamicFeatureInfo info, object userArg)
        {
            if (userArg != null)
            {
                FeatureCounts featureCounts = (FeatureCounts)userArg;
                ++featureCounts.FeatureCount;
                featureCounts.AddDtmFeature(info.FeaturePoints, DTMFeatureType.Breakline);
            }
            return true;
        }

        bool browseTrace(DTMDynamicFeatureInfo info, object userArg)
        {
            Console.WriteLine("FeatureType = {0} **** Number Of Points = {1}", info.FeatureType, info.FeaturePoints.Length);
            return true;
        }

        [Test]
        [Category("DTMDrainage")]
        public void BrowseCatchments()
        {
            IgnoreFirebug();

            DateTime startTime = DateTime.Now;
            TimeSpan elapsedTime = DateTime.Now - startTime;
            double lowPointDepth = 1.0;
            Console.WriteLine("Browsing Catchments");

            //  Set Path To Folder Holding DTM Files For Determing Catchments

            String DataPath = Helper.GetTestDataLocation () +@"Bentley.Civil.Dtm.NUnit.dll\\DTMDrainageTests\DTMDrainageCatchmentTests\";

            // Get Test Data Directories

            DirectoryInfo testFolder = new DirectoryInfo(DataPath);
            if (!testFolder.Exists)
            {
                throw new DirectoryNotFoundException("DTM Drainage Catchment Test Folder Not Found");
            }

            // Scan And Determine Catchments For All The DTM Files In The Test Folder

            foreach (System.IO.FileInfo nextFile in testFolder.GetFiles())
            {
                if (string.Compare(nextFile.Extension, ".dtm") == 0)
                {
                    Console.WriteLine("DTM File = {0}", nextFile.Name);

                    // Import The DTM File

                    Console.WriteLine("Importing {0}", nextFile.Name);
                    DateTime impstartTime = DateTime.Now;
                    DTM dtm = DTM.CreateFromFile(DataPath + nextFile.Name);

                    // Check The Triangulation - Unit Test Checking Only

                    Console.WriteLine("Checking Triangulation");
                    startTime = DateTime.Now;
                    if (dtm.CheckTriangulation() == false)
                    {
                        throw new Exception("Triangulation Invalid");
                    }
                    Console.WriteLine("Triangulation Valid");
                    elapsedTime = DateTime.Now - startTime;
                    Console.WriteLine("Triangulation Validation Time was {0} seconds.", elapsedTime.TotalSeconds);

                    // Refine The DTM Prior To Browsing Catchments

                    //Console.WriteLine("Refining DTM Prior To Browsing Catchments");
                    //startTime = DateTime.Now;
                    //dtm.RefineForCatchmentAnalysis(lowPointDepth);
                    //elapsedTime = DateTime.Now - startTime;
                    //Console.WriteLine("Time To Refine DTM = {0} seconds", elapsedTime.TotalSeconds);

                    // Check The Triangulation After Refinement - Unit Test Checking Only

                    //Console.WriteLine("Checking Triangulation");
                    //startTime = DateTime.Now;
                    //if (dtm.CheckTriangulation() == false)
                    //{
                    //    throw new Exception("Triangulation Invalid");
                    //}
                    //Console.WriteLine("Triangulation Valid");
                    //elapsedTime = DateTime.Now - startTime;
                    //Console.WriteLine("Triangulation Validation Time was {0} seconds", elapsedTime.TotalSeconds);

                    // Browse The DTM Catchments

                    startTime = DateTime.Now;
                    FeatureCounts featureCounts = new FeatureCounts();
                    featureCounts.FeatureCount = 0;
                    DynamicFeaturesBrowsingDelegate catchmentBrowsingDelegate = new DynamicFeaturesBrowsingDelegate(browseCatchments);
                    CatchmentsBrowsingCriteria catchmentBrowsingCriteria = new CatchmentsBrowsingCriteria();
                    dtm.BrowseCatchments(catchmentBrowsingCriteria, lowPointDepth, catchmentBrowsingDelegate, featureCounts);
                    elapsedTime = DateTime.Now - startTime;
                    Console.WriteLine("Time To Browse Catchments = {0} seconds", elapsedTime.TotalSeconds);
                    Console.WriteLine("Number Of Catchments = {0}", featureCounts.FeatureCount);
                    featureCounts.FeatureDTM.Save(DataPath + "catchments." + nextFile.Name);

                    // Dispose DTM To Release Allocated Unmanaged Memory

                    dtm.Dispose();
                }
            }
        }

        [Test]
        [Category("DTMDrainage")]
        public void CalculatePondForPoint()
        {
            IgnoreFirebug();

            DateTime startTime = DateTime.Now;
            TimeSpan elapsedTime = DateTime.Now - startTime;
            Console.WriteLine("Calculating Ponds");

            //  Set Path To Folder Holding DTM Files For Determing Catchments

            String DataPath = Helper.GetTestDataLocation () +@"Bentley.Civil.Dtm.NUnit.dll\\DTMDrainageTests\DTMDrainageCatchmentTests\";

            // Get Test Data Directories

            DirectoryInfo testFolder = new DirectoryInfo(DataPath);
            if (!testFolder.Exists)
            {
                throw new DirectoryNotFoundException("DTM Drainage Catchment Test Folder Not Found");
            }

            // Scan And Determine Ponds For All The DTM Files In The Test Folder

            foreach (System.IO.FileInfo nextFile in testFolder.GetFiles())
            {
                if (string.Compare(nextFile.Extension, ".dtm") == 0)
                {
                    Console.WriteLine("DTM File = {0}", nextFile.Name);

                    // Import The DTM File

                    Console.WriteLine("Importing {0}", nextFile.Name);
                    DateTime impstartTime = DateTime.Now;
                    DTM dtm = DTM.CreateFromFile(DataPath + nextFile.Name);

                    // Check The Triangulation - Unit Test Checking Only

                    Console.WriteLine("Checking Triangulation");
                    startTime = DateTime.Now;
                    if (dtm.CheckTriangulation() == false)
                    {
                        throw new Exception("Triangulation Invalid");
                    }
                    Console.WriteLine("Triangulation Valid");
                    elapsedTime = DateTime.Now - startTime;
                    Console.WriteLine("Triangulation Validation Time was {0} seconds.", elapsedTime.TotalSeconds);

                    // Create Pond

                    BGEO.DRange3d range = dtm.Range3d;

                    PondCalculation pondResult = dtm.CalculatePond((range.Low.X + range.High.X) / 2.0, (range.Low.Y + range.High.Y) / 2.0);

                    Console.WriteLine("Pond Calculated = {0}", pondResult.Calculated);
                    Console.WriteLine("Pond Elevation = {0}", pondResult.Elevation);
                    Console.WriteLine("Pond Depth = {0}", pondResult.Depth);
                    Console.WriteLine("Pond Area = {0}", pondResult.Area);
                    Console.WriteLine("Pond Volume = {0}", pondResult.Volume);
                    DTMDynamicFeatureInfo[] pondFeatures = pondResult.PondFeatures;
                    Console.WriteLine("Number Of Pond Features = {0}", pondFeatures.Length);
                    for (int n = 0; n < pondFeatures.Length; ++n)
                    {
                        Console.WriteLine("Feature = {0} ** Number Of Points = {1}", pondFeatures[n].FeatureType, pondFeatures[n].FeaturePoints.Length);
                        BGEO.DPoint3d[] points = pondFeatures[n].FeaturePoints;
                        for (int m = 0; m < pondFeatures[n].FeaturePoints.Length; ++m)
                        {
                            Console.WriteLine("Pond Point[{0}] = {1} {2} {3}", m, points[m].X, points[m].Y, points[m].Z);
                        }
                    }


                    // Dispose DTM To Release Allocated Unmanaged Memory

                    dtm.Dispose();
                }
            }
        }

        [Test]
        [Category("DTMDrainage")]
        public void TraceCatchmentForPoint()
        {
            IgnoreFirebug();

            DateTime startTime = DateTime.Now;
            TimeSpan elapsedTime = DateTime.Now - startTime;
            Console.WriteLine("Calculating Ponds");

            //  Set Path To Folder Holding DTM Files For Determing Catchments

            String DataPath = Helper.GetTestDataLocation () +@"Bentley.Civil.Dtm.NUnit.dll\\DTMDrainageTests\DTMDrainageCatchmentTests\";

            // Get Test Data Directories

            DirectoryInfo testFolder = new DirectoryInfo(DataPath);
            if (!testFolder.Exists)
            {
                throw new DirectoryNotFoundException("DTM Drainage Catchment Test Folder Not Found");
            }

            // Scan And Determine Ponds For All The DTM Files In The Test Folder

            foreach (System.IO.FileInfo nextFile in testFolder.GetFiles())
            {
                if (string.Compare(nextFile.Extension, ".dtm") == 0)
                {
                    Console.WriteLine("DTM File = {0}", nextFile.Name);

                    // Import The DTM File

                    Console.WriteLine("Importing {0}", nextFile.Name);
                    DateTime impstartTime = DateTime.Now;
                    DTM dtm = DTM.CreateFromFile(DataPath + nextFile.Name);

                    // Check The Triangulation - Unit Test Checking Only

                    //                    Console.WriteLine("Checking Triangulation");
                    //                    startTime = DateTime.Now;
                    //                    if (dtm.CheckTriangulation() == false)
                    //                    {
                    //                        throw new Exception("Triangulation Invalid");
                    //                    }
                    //                    Console.WriteLine("Triangulation Valid");
                    //                    elapsedTime = DateTime.Now - startTime;
                    //                    Console.WriteLine("Triangulation Validation Time was {0} seconds.", elapsedTime.TotalSeconds);

                    // Create Pond

                    BGEO.DRange3d range = dtm.Range3d;
                    double maxPondDepth = 0.0;
                    DTMDynamicFeatureInfo[] catchmentFeatures = dtm.TraceCatchmentForPoint ((range.Low.X + range.High.X) / 2.0, (range.Low.Y + range.High.Y) / 2.0, maxPondDepth).Catchment;
                    if (catchmentFeatures == null)
                    {
                        Console.WriteLine("Catchment Not Determined");
                    }
                    else
                    {
                        DTM catchment = new DTM();
                        Console.WriteLine("Number Of Catchment Features = {0}", catchmentFeatures.Length);
                        for (int n = 0; n < catchmentFeatures.Length; ++n)
                        {
                            catchment.AddLinearFeature(catchmentFeatures[n].FeaturePoints, DTMFeatureType.Breakline);
                            Console.WriteLine("Feature = {0} ** Number Of Points = {1}", catchmentFeatures[n].FeatureType, catchmentFeatures[n].FeaturePoints.Length);
                            BGEO.DPoint3d[] points = catchmentFeatures[n].FeaturePoints;
                            for (int m = 0; m < catchmentFeatures[n].FeaturePoints.Length; ++m)
                            {
                                Console.WriteLine("Catchment Point[{0}] = {1} {2} {3}", m, points[m].X, points[m].Y, points[m].Z);
                            }
                        }
                        //                        catchment.Save(DataPath + "catchment.dtm");

                        startTime = DateTime.Now;
                        RidgeLinesBrowsingCriteria ridgeCriteria = new RidgeLinesBrowsingCriteria();
                        FeatureCounts ridgeFeatures = new FeatureCounts();
                        Console.WriteLine("cachSize = {0}", ridgeCriteria.CacheSize);
                        Console.WriteLine("Browsing Ridge Lines");
                        dtm.BrowseRidgeLines(ridgeCriteria, browseCatchments, ridgeFeatures);
                        Console.WriteLine("Number Of Ridge Features {0}", ridgeFeatures.FeatureCount);
                        elapsedTime = DateTime.Now - startTime;
                        Console.WriteLine("Time To Browse Ridge Lines = {0} Seconds", elapsedTime.TotalSeconds);
                        //                        ridgeFeatures.FeatureDTM.Save(DataPath + "ridge.dtm");
                    }


                    // Dispose DTM To Release Allocated Unmanaged Memory

                    dtm.Dispose();
                }
            }
        }

        [Test]
        [Category("DTMDrainage")]
        public void MaximumAscentDescentTrace()
        {
            IgnoreFirebug();

            DateTime startTime = DateTime.Now;
            TimeSpan elapsedTime = DateTime.Now - startTime;
            Console.WriteLine("Calculating Ponds");

            //  Set Path To Folder Holding DTM Files For Determing Catchments

            String DataPath = Helper.GetTestDataLocation () +@"Bentley.Civil.Dtm.NUnit.dll\\DTMDrainageTests\DTMDrainageCatchmentTests\";

            // Get Test Data Directories

            DirectoryInfo testFolder = new DirectoryInfo(DataPath);
            if (!testFolder.Exists)
            {
                throw new DirectoryNotFoundException("DTM Drainage Catchment Test Folder Not Found");
            }

            // Scan And Determine Ponds For All The DTM Files In The Test Folder

            foreach (System.IO.FileInfo nextFile in testFolder.GetFiles())
            {
                if (string.Compare(nextFile.Extension, ".dtm") == 0)
                {
                    Console.WriteLine("DTM File = {0}", nextFile.Name);

                    // Import The DTM File

                    Console.WriteLine("Importing {0}", nextFile.Name);
                    DateTime impstartTime = DateTime.Now;
                    DTM dtm = DTM.CreateFromFile(DataPath + nextFile.Name);

                    // Check The Triangulation - Unit Test Checking Only

                    //                    Console.WriteLine("Checking Triangulation");
                    //                    startTime = DateTime.Now;
                    //                    if (dtm.CheckTriangulation() == false)
                    //                    {
                    //                        throw new Exception("Triangulation Invalid");
                    //                    }
                    //                    Console.WriteLine("Triangulation Valid");
                    //                    elapsedTime = DateTime.Now - startTime;
                    //                    Console.WriteLine("Triangulation Validation Time was {0} seconds.", elapsedTime.TotalSeconds);

                    // Trace Maximim Descent

                    DynamicFeaturesBrowsingDelegate traceBrowsingDelegate = new DynamicFeaturesBrowsingDelegate(browseTrace);
                    BGEO.DPoint3d traceFromPoint = new BGEO.DPoint3d(315035.441,295497.472) ;
                    double minLowDepth = 0.1;
                    double minHighElevation = 0.1;
                    Console.WriteLine("**** Tracing Maximum Descent");
                    dtm.TraceMaximumDescent(minLowDepth, traceFromPoint, traceBrowsingDelegate, null);
                    Console.WriteLine("**** Tracing Maximum Ascent");
                    dtm.TraceMaximumAscent(minHighElevation, traceFromPoint, traceBrowsingDelegate, null);
 
                    // Dispose DTM To Release Allocated Unmanaged Memory

                    dtm.Dispose();
                }
            }
        }
    }
}
