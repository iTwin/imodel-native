using System;
using System.IO;
using System.Collections.Generic;
using System.Collections;
using NUnit.Framework;

namespace Bentley.TerrainModelNET.NUnit
{
    using BGEO = Bentley.GeometryNET;
    using Bentley.TerrainModelNET;

    /// <summary>
    /// Summary description for DTM.
    /// </summary>
    [TestFixture]
    public class DTMCrossingFeatures : DTMUNitTest
    {

        class CrossingErrorCriteria
        {
            public double CrossingTolerance=0.0;
            public int NumCrossingErrors=0;
            public CrossingErrorCriteria(double crossingTolerance, int numErrors)
            {
                CrossingTolerance = crossingTolerance;
                NumCrossingErrors = numErrors;
            }
        }

        private bool BrowseCrossingFeatures(DTMCrossingFeatureError crossError, System.Object oArg)
        {
            CrossingErrorCriteria errorCriteria = (CrossingErrorCriteria)oArg;
            double zDifference = crossError.ElevationOnFeature1 - crossError.ElevationOfFeature2;
            if (zDifference < 0.0) zDifference = -zDifference;
            if ( zDifference >= 0.0  /*zDifference > errorCriteria.CrossingTolerance */ )
            {
                ++errorCriteria.NumCrossingErrors;
                BGEO::DPoint2d intersectPoint = crossError.IntersectionPointXY;
                Console.WriteLine("Intersect Point Error = {0} {1} ** Z1 = {2} Z2 = {3} ** Feature1Id = {4} Feature2Id = {5}", intersectPoint.X, intersectPoint.Y, crossError.ElevationOnFeature1, crossError.ElevationOfFeature2, crossError.IdOfFeature1, crossError.IdOfFeature2);
            }
            return true;
        }


        /// <summary>
        /// Scans Tin Files For Crossing DTM Features
        /// As There Are No Crossing Features Errors In A Tin It Simply Reports All Intersecting Features
        /// Simply Testing That The Core Functions Work
        /// </summary>
        [Test]
        [Category("Crossing Features")]
        public void CrossingTinFeatures()
        {
            // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

            // Set The Path 

            String TinDirectory = @"Bentley.Civil.Dtm.NUnit.dll\DTMCrossingFeaturesTest\DTMCrossingTinFeaturesTest\";
            String TinPath = Helper.GetTestDataLocation () + TinDirectory;

            // Check Crossing Features Test Data Directory Exists

            DirectoryInfo testFolder = new DirectoryInfo(TinPath);
            if (!testFolder.Exists)
            {
                throw new DirectoryNotFoundException(" DTM Crossing Tin Features Test Data Folder Not Found " + TinPath);
            }


            // Scan All Tin Files In The Folder And Check For Crossing Features

            foreach (System.IO.FileInfo nextFile in testFolder.GetFiles())
            {
                if (string.Compare(nextFile.Extension, ".tin") == 0)
                {
                    Console.WriteLine("Tin File = {0}", nextFile.Name);

                    // Import The Tin File

                    Console.WriteLine("Importing {0} To DTM", nextFile.Name);
                    DTM testDTM = DTM.CreateFromFile(TinPath + nextFile.Name);

                    // Create Crossing Features Browsing Criteria

                    CrossingFeaturesBrowsingCriteria criteria = new CrossingFeaturesBrowsingCriteria();
                    DTMFeatureType[] FeatureList = new DTMFeatureType[1];
                    FeatureList[0] = DTMFeatureType.Breakline;
                    criteria.FeatureTypes = FeatureList;

                    // Create Crossing Features Browsing Delegate

                    CrossingFeaturesBrowsingDelegate crossingBrowser = new CrossingFeaturesBrowsingDelegate(BrowseCrossingFeatures);

                    // Browse The Crossing Features

                    CrossingErrorCriteria crossErrorCriteria = new CrossingErrorCriteria(0.0,0);
//                    testDTM.BrowseCrossingFeatures(criteria, crossingBrowser, crossErrorCriteria);
                    testDTM.BrowseCrossingFeatures(criteria, BrowseCrossingFeatures, crossErrorCriteria);
                    Console.WriteLine("Number Of Crossing Feature Errors = {0}", crossErrorCriteria.NumCrossingErrors);

                    // Dispose DTM

                    testDTM.Dispose();
                }
            }
        }


        /// <summary>
        /// Scans Geopak Dat Files For Crossing DTM Features
        /// </summary>
        [Test]
        [Category("Crossing Features")]
        public void CrossingDatFeatures()
        {

            // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

            // Set Path To Triangulation Dat Files

            String DatDirectory = @"Bentley.Civil.Dtm.NUnit.dll\DTMTriangulationTest\DTMTriangulationDatTest\";
            String DatPath = Helper.GetTestDataLocation () + DatDirectory;

            // Check Crossing Features Test Data Directory Exists

            DirectoryInfo testFolder = new DirectoryInfo(DatPath);
            if (!testFolder.Exists)
            {
                throw new DirectoryNotFoundException(" DTM Crossing Features Test Data Folder Not Found " + DatPath);
            }

            // Scan All Dat Files In The Folder And Check For Crossing Features

            foreach (System.IO.FileInfo nextFile in testFolder.GetFiles())
            {
                if (string.Compare(nextFile.Extension, ".dat") == 0)
                {
                    Console.WriteLine("Dat File = {0}", nextFile.Name);

                    // Import The Dat File

                    Console.WriteLine("Importing {0} To DTM", nextFile.Name);
                    DTM testDTM = DTM.CreateFromGeopakDatFile(DatPath + nextFile.Name);

 
                    // Create Crossing Features Browsing Criteria

                    CrossingFeaturesBrowsingCriteria criteria = new CrossingFeaturesBrowsingCriteria();
                    DTMFeatureType[] FeatureList = new DTMFeatureType[1];
                    FeatureList[0] = DTMFeatureType.Breakline;
                    criteria.FeatureTypes = FeatureList;

                    // Create Crossing Features Browsing Delegate

                    CrossingFeaturesBrowsingDelegate crossingBrowser = new CrossingFeaturesBrowsingDelegate(BrowseCrossingFeatures);

                    // Browse The Crossing Features

                    CrossingErrorCriteria crossErrorCriteria = new CrossingErrorCriteria(0.001, 0);
//                    testDTM.BrowseCrossingFeatures(criteria, crossingBrowser, crossErrorCriteria);
                    testDTM.BrowseCrossingFeatures(criteria, BrowseCrossingFeatures, crossErrorCriteria);
                    Console.WriteLine("Number Of Crossing Feature Errors = {0}", crossErrorCriteria.NumCrossingErrors);

                    // Dispose DTM

                    testDTM.Dispose(); 

                }
            }
        }


        /// <summary>
        /// Scans Untriangulated DTM Files For Crossing DTM Features
        /// </summary>
        [Test]
        [Category("Crossing Features")]
        public void CrossingDtmFeatures()
        {

            // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

            // Set Path To Triangulation Dtm Files

            String DtmDirectory = @"Bentley.Civil.Dtm.NUnit.dll\DTMCrossingFeaturesTest\DTMCrossingDtmFeaturesTest\";
            String DtmPath = Helper.GetTestDataLocation () + DtmDirectory;

            // Check Crossing Features Test Dtma Directory Exists

            DirectoryInfo testFolder = new DirectoryInfo(DtmPath);
            if (!testFolder.Exists)
            {
                throw new DirectoryNotFoundException(" DTM Crossing Features Test Dtm Folder Not Found " + DtmPath);
            }

            // Scan All Dtm Files In The Folder And Check For Crossing Features

            foreach (System.IO.FileInfo nextFile in testFolder.GetFiles())
            {
                if (string.Compare(nextFile.Extension, ".dtm") == 0)
                {
                    Console.WriteLine("DTM File = {0}", nextFile.Name);

                    // Import The Dtm File

                    Console.WriteLine("Importing {0} To DTM", nextFile.Name);
                    DTM testDTM = DTM.CreateFromFile(DtmPath + nextFile.Name);

                    // Create Crossing Features Browsing Criteria

                    CrossingFeaturesBrowsingCriteria criteria = new CrossingFeaturesBrowsingCriteria();
                    DTMFeatureType[] FeatureList = new DTMFeatureType[1];
                    FeatureList[0] = DTMFeatureType.Breakline;
                    criteria.FeatureTypes = FeatureList;

                    // Create Crossing Features Browsing Delegate

                    CrossingFeaturesBrowsingDelegate crossingBrowser = new CrossingFeaturesBrowsingDelegate(BrowseCrossingFeatures);

                    // Browse The Crossing Features

                    CrossingErrorCriteria crossErrorCriteria = new CrossingErrorCriteria(0.001, 0);
//                    testDTM.BrowseCrossingFeatures(criteria, crossingBrowser, crossErrorCriteria);
                    testDTM.BrowseCrossingFeatures(criteria, BrowseCrossingFeatures, crossErrorCriteria);
                    Console.WriteLine("Number Of Crossing Feature Errors = {0}", crossErrorCriteria.NumCrossingErrors);

                    // Dispose DTM

                    testDTM.Dispose();
                }
            }
        }
    }

}
