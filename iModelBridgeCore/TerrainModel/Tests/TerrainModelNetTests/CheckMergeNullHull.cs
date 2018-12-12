using System;
using System.IO;
using NUnit.Framework;

namespace Bentley.TerrainModelNET.NUnit
    {
    using BGEO = Bentley.GeometryNET;
    using Bentley.TerrainModelNET;
    using Bentley.GeometryNET;


    /// <summary>
    /// Summary description for DTM.
    /// </summary>
    [TestFixture]
    public class Check_MergeNullHull : DTMUNitTest
    {
        private bool gFoundIt = false;
        private DTMFeatureId gFeatureId;

        private bool BrowseToGetLinearFeatures(DTMFeatureInfo featureInfo, object oArg)
        {
            gFoundIt = true;
            gFeatureId = featureInfo.DtmFeatureId;
            return true;
        }

        /// <summary>
        /// Test add and find a spot features
        /// </summary>
        [Test]
        [Category("Just a Test")]
//        [Ignore("Need to be review / Memory Corrupted")]
        public void CheckMergeNullHull()
        {
            DTM tin1 = new DTM();
            DTMFeatureId id;
            BGEO.DPoint3d[] testPoints = new BGEO.DPoint3d[2];
            BGEO.DPoint3d[] exPoints = new BGEO.DPoint3d[5];
            BGEO.DPoint3d[] hullPoints = new BGEO.DPoint3d[6];
            BGEO.DPoint3d aPoint = new BGEO.DPoint3d(0, 0, 0);

            aPoint.XX = 150;
            aPoint.YY = 150;
            aPoint.ZZ = 100;
            id = tin1.AddPointFeature(aPoint);

            aPoint.XX = 120;
            aPoint.YY = 120;
            aPoint.ZZ = 80;
            id = tin1.AddPointFeature(aPoint);

            aPoint.XX = 130;
            aPoint.YY = 130;
            aPoint.ZZ = 90;
            id = tin1.AddPointFeature(aPoint);

            aPoint.XX = 100;
            aPoint.YY = 100;
            aPoint.ZZ = 100;
            id = tin1.AddPointFeature(aPoint);

            aPoint.XX = 100;
            aPoint.YY = 200;
            aPoint.ZZ = 100;
            id = tin1.AddPointFeature(aPoint);

            aPoint.XX = 200;
            aPoint.YY = 200;
            aPoint.ZZ = 100;
            id = tin1.AddPointFeature(aPoint);

            aPoint.XX = 200;
            aPoint.YY = 100;
            aPoint.ZZ = 100;
            id = tin1.AddPointFeature(aPoint);

            aPoint.XX = 130;
            aPoint.YY = 140;
            aPoint.ZZ = 100;
            id = tin1.AddPointFeature(aPoint);

            testPoints[0].XX = 120;
            testPoints[0].YY = 120;
            testPoints[0].ZZ = 100;

            testPoints[1].XX = 130;
            testPoints[1].YY = 130;
            testPoints[1].ZZ = 100;
            id = tin1.AddPointFeature(testPoints);

            testPoints[0].XX = 100;
            testPoints[0].YY = 100;
            testPoints[0].ZZ = 100;

            testPoints[1].XX = 200;
            testPoints[1].YY = 200;
            testPoints[1].ZZ = 100;

            id = tin1.AddLinearFeature(testPoints, DTMFeatureType.Breakline);

            // Create one that crosses....

            testPoints[0].XX = 75;
            testPoints[0].YY = 150;
            testPoints[0].ZZ = 225;

            testPoints[1].XX = 165;
            testPoints[1].YY = 125;
            testPoints[1].ZZ = 280;

            id = tin1.AddLinearFeature(testPoints, DTMFeatureType.Breakline);

            exPoints[0].XX = 135;
            exPoints[0].YY = 190;
            exPoints[0].ZZ = 100;

            exPoints[1].XX = 147;
            exPoints[1].YY = 173;
            exPoints[1].ZZ = 90;

            exPoints[2].XX = 160;
            exPoints[2].YY = 180;
            exPoints[2].ZZ = 90;

            exPoints[3].XX = 168;
            exPoints[3].YY = 187;
            exPoints[3].ZZ = 90;

            exPoints[4].XX = 135;
            exPoints[4].YY = 190;
            exPoints[4].ZZ = 100;

            id = tin1.AddLinearFeature(exPoints, DTMFeatureType.Void);

            exPoints[0].XX = 130;
            exPoints[0].YY = 65;
            exPoints[0].ZZ = 100;

            exPoints[1].XX = 130;
            exPoints[1].YY = 110;
            exPoints[1].ZZ = 90;

            exPoints[2].XX = 230;
            exPoints[2].YY = 110;
            exPoints[2].ZZ = 90;

            exPoints[3].XX = 230;
            exPoints[3].YY = 75;
            exPoints[3].ZZ = 90;

            exPoints[4].XX = 130;
            exPoints[4].YY = 65;
            exPoints[4].ZZ = 100;

            id = tin1.AddLinearFeature(exPoints, DTMFeatureType.Void);

            exPoints[0].XX = 140;
            exPoints[0].YY = 80;
            exPoints[0].ZZ = 100;

            exPoints[1].XX = 140;
            exPoints[1].YY = 100;
            exPoints[1].ZZ = 90;

            exPoints[2].XX = 215;
            exPoints[2].YY = 100;
            exPoints[2].ZZ = 75;

            exPoints[3].XX = 215;
            exPoints[3].YY = 80;
            exPoints[3].ZZ = 90;

            exPoints[4].XX = 140;
            exPoints[4].YY = 80;
            exPoints[4].ZZ = 100;

            id = tin1.AddLinearFeature(exPoints, DTMFeatureType.Island);

            exPoints[0].XX = 50;
            exPoints[0].YY = 50;
            exPoints[0].ZZ = 100;

            exPoints[1].XX = 50;
            exPoints[1].YY = 250;
            exPoints[1].ZZ = 90;

            exPoints[2].XX = 250;
            exPoints[2].YY = 250;
            exPoints[2].ZZ = 90;

            exPoints[3].XX = 250;
            exPoints[3].YY = 50;
            exPoints[3].ZZ = 100;

            exPoints[4].XX = 50;
            exPoints[4].YY = 50;
            exPoints[4].ZZ = 100;

            id = tin1.AddLinearFeature(exPoints, DTMFeatureType.Hull);

            // triangulate it...
            Console.WriteLine("Triangulating Tin1");
            tin1.Triangulate();

           // Check The Triangulation - Unit Test Checking And Development Only

            Console.WriteLine("Checking Tin1 Triangulation");
            if (tin1.CheckTriangulation() == false)
            {
                throw new Exception("Tin1 Triangulation Invalid");
            }
            Console.WriteLine("Tin1 Triangulation Valid");


            // Create second one...
            DTM tin2 = new DTM();

            aPoint.XX = 650;
            aPoint.YY = 650;
            aPoint.ZZ = 600;
            id = tin2.AddPointFeature(aPoint);

            aPoint.XX = 720;
            aPoint.YY = 720;
            aPoint.ZZ = 680;
            id = tin2.AddPointFeature(aPoint);

            aPoint.XX = 730;
            aPoint.YY = 730;
            aPoint.ZZ = 690;
            id = tin2.AddPointFeature(aPoint);

            aPoint.XX = 700;
            aPoint.YY = 700;
            aPoint.ZZ = 700;
            id = tin2.AddPointFeature(aPoint);

            aPoint.XX = 700;
            aPoint.YY = 800;
            aPoint.ZZ = 700;
            id = tin2.AddPointFeature(aPoint);

            aPoint.XX = 800;
            aPoint.YY = 800;
            aPoint.ZZ = 700;
            id = tin2.AddPointFeature(aPoint);

            aPoint.XX = 800;
            aPoint.YY = 700;
            aPoint.ZZ = 700;
            id = tin2.AddPointFeature(aPoint);

            aPoint.XX = 730;
            aPoint.YY = 740;
            aPoint.ZZ = 700;
            id = tin2.AddPointFeature(aPoint);

            testPoints[0].XX = 720;
            testPoints[0].YY = 720;
            testPoints[0].ZZ = 700;

            testPoints[1].XX = 730;
            testPoints[1].YY = 730;
            testPoints[1].ZZ = 700;
            id = tin2.AddPointFeature(testPoints);

            testPoints[1].XX = 533;
            testPoints[1].YY = 460;
            testPoints[1].ZZ = 500;
            id = tin2.AddPointFeature(testPoints);

            testPoints[1].XX = 198;
            testPoints[1].YY = 216;
            testPoints[1].ZZ = 400;
            id = tin2.AddPointFeature(testPoints);

            testPoints[0].XX = 700;
            testPoints[0].YY = 700;
            testPoints[0].ZZ = 700;

            testPoints[1].XX = 800;
            testPoints[1].YY = 800;
            testPoints[1].ZZ = 800;

            id = tin2.AddLinearFeature(testPoints, DTMFeatureType.Breakline);

            exPoints[0].XX = 735;
            exPoints[0].YY = 790;
            exPoints[0].ZZ = 700;

            exPoints[1].XX = 747;
            exPoints[1].YY = 773;
            exPoints[1].ZZ = 790;

            exPoints[2].XX = 760;
            exPoints[2].YY = 780;
            exPoints[2].ZZ = 690;

            exPoints[3].XX = 768;
            exPoints[3].YY = 787;
            exPoints[3].ZZ = 690;

            exPoints[4].XX = 735;
            exPoints[4].YY = 790;
            exPoints[4].ZZ = 700;

            id = tin2.AddLinearFeature(exPoints, DTMFeatureType.Void);

            hullPoints[0].XX = 175;
            hullPoints[0].YY = 175;
            hullPoints[0].ZZ = 700;

            hullPoints[1].XX = 650;
            hullPoints[1].YY = 850;
            hullPoints[1].ZZ = 690;

            hullPoints[2].XX = 850;
            hullPoints[2].YY = 850;
            hullPoints[2].ZZ = 690;

            hullPoints[3].XX = 850;
            hullPoints[3].YY = 650;
            hullPoints[3].ZZ = 700;

            hullPoints[4].XX = 600;
            hullPoints[4].YY = 175;
            hullPoints[3].ZZ = 600;

            hullPoints[5].XX = 175;
            hullPoints[5].YY = 175;
            hullPoints[5].ZZ = 700;

            id = tin2.AddLinearFeature(hullPoints, DTMFeatureType.Hull);

            Console.WriteLine("Triangulating Tin2");
            tin2.Triangulate();

            // Check The Triangulation - Unit Test Checking And Development Only

            Console.WriteLine("Checking Tin2 Triangulation");
            if (tin2.CheckTriangulation() == false)
            {
                throw new Exception("Tin2 Triangulation Invalid");
            }
            Console.WriteLine("Tin2 Triangulation Valid");

            // Merge...
            DTM tin3 = tin2.Clone();

            Console.WriteLine("Merging Tin2 ==> Tin1");
            tin3.Merge(tin1);

            // Check The Triangulation - Unit Test Checking And Development Only

            Console.WriteLine("Checking Tin3 Triangulation");
            if (tin3.CheckTriangulation() == false)
            {
                throw new Exception("Tin3 Triangulation Invalid");
            }
            Console.WriteLine("Tin3 Triangulation Valid");

            // Look for the hull
            gFoundIt = false;

            // Browse for the hull....
            LinearFeaturesBrowsingCriteria criteria = new LinearFeaturesBrowsingCriteria ();
            LinearFeaturesBrowsingDelegate browser = new LinearFeaturesBrowsingDelegate (BrowseToGetLinearFeatures);

            tin3.BrowseLinearFeatures(criteria, DTMFeatureType.Hull, BrowseToGetLinearFeatures, null);

            DTMFeature feature = null;

            if (true == gFoundIt)
            {
                feature = tin3.GetFeatureById(gFeatureId);
            }

            if (null == feature)
            {
                Assert.Fail ("Hull is Null");
            }
            else if (feature is Bentley.TerrainModelNET.DTMComplexLinearFeature)
            {
        // ToDo...Standalone
            //BCG.LinearElement[] tBlElement = Bentley.TerrainModelNET.LinearGeometry.Helper.DTMLinearFeatureGetElements ((feature as DTMComplexLinearFeature));

            //    if (tBlElement.Length > 1)
            //    {
            //        Assert.Fail("Hull has more than 1 element.");
            //    }
            }
        }
    }
}
