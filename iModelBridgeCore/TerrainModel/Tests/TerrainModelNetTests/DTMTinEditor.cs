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
    ///  DTM Tin Editor Tests
    /// </summary>
    [TestFixture]
    public class DTMEditor : DTMUNitTest
        {

        /// <summary>
        /// DTM Tin Editor Functions 
        /// </summary>
        int totalTrianglesBrowsed = 0;
        int numTriangleCentroids = 0;
        BGEO.DPoint3d[] triangleCentroids = new BGEO.DPoint3d[1000];

        [Test]
        [Category("DTM Interactive Tin Editor")]
        public void DeleteTriangle()
        {
            // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

            String EditDirectory = @"Bentley.Civil.Dtm.NUnit.dll\DTMEditorTest\DTMEditorDeleteTrianglesTest\";
            String EditPath = Helper.GetTestDataLocation () + EditDirectory;

            int beforeTriangleCount = 0;
            int afterTriangleCount = 0;
            long voidCount = 0;
            long islandCount = 0;

            // Import Edit Dtm

            DTM dtm = DTM.CreateFromFile(EditPath + "editDtm00.dtm");

            // Check Triangulation - Unit Test Checking Only

            Console.WriteLine("Checking Triangulation");
            if ( dtm.CheckTriangulation() == false)
                {
                throw new Exception("Triangulation Invalid");
                }
            Console.WriteLine("Triangulation Valid");

            // Browse The Triangles

            Console.WriteLine("Browsing Triangles");
            DynamicFeaturesBrowsingCriteria triangleCriteria = new DynamicFeaturesBrowsingCriteria();
            DynamicFeaturesBrowsingDelegate TriangleBrowser = new DynamicFeaturesBrowsingDelegate(triangleBrowser);
            dtm.BrowseDynamicFeatures(triangleCriteria, DTMDynamicFeatureType.Triangle, triangleBrowser, null);
            Console.WriteLine("Number Of Triangles Browsed = {0}",totalTrianglesBrowsed);
            totalTrianglesBrowsed = 0;

            // Get the dtm editor
            DTMTinEditor editor = dtm.GetTinEditor();

            BGEO.DPoint3d point = new BGEO.DPoint3d(0, 0, 0);

            // Delete Triangles From Centroids

            bool process = true;
            for (int n = 0; n < numTriangleCentroids && process; ++n )
                {
                point.X = triangleCentroids[n].X;
                point.Y = triangleCentroids[n].Y;
                point.Z = triangleCentroids[n].Z;
                Console.WriteLine("Deleting Triangle ** {0} {1} {2}", point.X, point.Y, point.Z);
                if (editor.Select(DTMDynamicFeatureType.Triangle, point) == true)
                   {
                    beforeTriangleCount = dtm.TrianglesCount;
                    editor.Delete();
                    afterTriangleCount = dtm.TrianglesCount;
                    Assert.AreNotEqual(beforeTriangleCount, afterTriangleCount, "Triangle Counts Before And After Deleting Triangle Are The Same");
                    voidCount = dtm.CalculateFeatureStatistics ().VoidsCount;
                    islandCount = dtm.CalculateFeatureStatistics ().IslandsCount;
                    Console.WriteLine("Triangles Count = {0} Voids Count = {1} IslandsCount = {2}", afterTriangleCount, voidCount, islandCount);
                    
                    // Check The Triangulation - Unit Test Checking Only
                    Console.WriteLine("Checking Triangulation");
                    if (dtm.CheckTriangulation() == false)
                        {
                        throw new Exception("Triangulation Invalid After Deleting Triangles");
                        }
                    Console.WriteLine("Triangulation Valid");

                    // Save The Edited Tin
                    dtm.Save(EditPath + "savedEditDtm.dtm");
                   }
                else process = false;
                }

            // Check The Triangulation - Unit Test Checking Only
            Console.WriteLine("Checking Triangulation");
            if (dtm.CheckTriangulation() == false)
                {
                throw new Exception("\nTriangulation Invalid After Deleting Triangle");
                }
            Console.WriteLine("Triangulation Valid");

            }

        //  Triangle Browser - Use This To Get Triangle Centroids To Delete Triangles

        private bool triangleBrowser(DTMDynamicFeatureInfo info,  object oArg)
            {
            var tPoint = info.FeaturePoints;
            double X = (tPoint[0].X + tPoint[1].X + tPoint[2].X) / 3.0;
            double Y = (tPoint[0].Y + tPoint[1].Y + tPoint[2].Y) / 3.0;
            double Z = (tPoint[0].Z + tPoint[1].Z + tPoint[2].Z) / 3.0;
            triangleCentroids[numTriangleCentroids].X = X;
            triangleCentroids[numTriangleCentroids].Y = Y;
            triangleCentroids[numTriangleCentroids].Z = Z;
            ++numTriangleCentroids;
            ++totalTrianglesBrowsed;
            //             Console.WriteLine("triangleCentrod[{0}] = {1} {2} {3}", totalTrianglesBrowsed, X, Y, Z);
            return true;
            }
        }
    }