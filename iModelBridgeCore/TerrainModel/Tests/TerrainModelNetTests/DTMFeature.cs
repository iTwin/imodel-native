
using System;
using System.IO;
using System.Text;
using System.Collections;
using SD = System.Data;
using SDO = System.Data.OleDb;
using BGEO = Bentley.GeometryNET;
using NUnit.Framework;


namespace Bentley.TerrainModelNET.NUnit
    {
    using Bentley.TerrainModelNET;

    /// <summary>
    /// 
    /// </summary>
    /// <category></category>
    [TestFixture]
    public class DTMFeatureTests : DTMUNitTest
        {
        DTMFeatureId m_featureId;
        /// <summary>
        /// DTM Feature Add , Modify And Delete Tests
        /// </summary>
        /// <returns></returns>
        [Test]
        [Category("DTMFeatureTests")]
        public void AddAndDeletePoints()
            {

            // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

            /// <summary>
            /// Create A DTM With 1000 Points
            /// </summary>
            bool reportValidation = true;
            double xMin = 1000.0;
            double xMax = 100000.0;
            double yMin = 1000.0;
            double yMax = 50000.0;
            double zMin = -10.0;
            double zMax = 100.0;
            DTM dtm = new DTM();

            // Store 10000 Points

            for (int n = 0; n < 100; ++n)
                {
                BGEO.DPoint3d[] randomPoints = Helper.CreateRandomPoints(n, 100, xMin, xMax, yMin, yMax, zMin, zMax);
                dtm.AddPoints(randomPoints);
                }

            // Triangulate DTM

            dtm.Triangulate();

            // Validate DTM

            Helper.ValidationResult validationResult = Helper.ValidateDTM(dtm);

            if (reportValidation == true)
                {
                Console.WriteLine("Number Of DTM Points = {0}", validationResult.NumPoints);
                Console.WriteLine("Number Of DTM Lines = {0}", validationResult.NumLines);
                Console.WriteLine("Number Of DTM Triangles = {0}", validationResult.NumTriangles);
                Console.WriteLine("Number Of DTM Features = {0}", validationResult.NumFeatures);
                Console.WriteLine("DTM Area = {0}", validationResult.Area);
                Console.WriteLine("DTM Volume = {0}", validationResult.Volume);
                }

            // Delete All Points

            dtm.DeleteFeaturesByType(DTMFeatureType.Point);

            // Check There Are No Points In The DTM
            // As Deleted Features Are Left In The DTM 
            // And Marked As Deleted When The DTM Is In A Tin State
            // It Must Be Firstly Converted To A Data State 
            // For The Following Assert Test

            DtmState dtmState = dtm.GetDtmState();
            if (dtmState != DtmState.Data)
                dtm.ConvertToDataState();
            Assert.AreEqual(0, dtm.VerticesCount);

            // Dispose DTM

            dtm.Dispose();

            }

        [Test]
        [Category("DTMFeatureTests")]
        public void AddAndDeletePointFeatures()
            {

            // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

            /// <summary>
            /// Create A DTM With 1000 Points
            /// </summary>

            bool reportValidation = true;
            double xMin = 1000.0;
            double xMax = 100000.0;
            double yMin = 1000.0;
            double yMax = 50000.0;
            double zMin = -10.0;
            double zMax = 100.0;
            DTM dtm = new DTM();
            DTMFeatureId[] dtmFeatureId = new DTMFeatureId[100];

            // Store 100 Point Features With 100 Points Per Feature

            for (int n = 0; n < 100; ++n)
                {
                BGEO.DPoint3d[] randomPoints = Helper.CreateRandomPoints(n, 100, xMin, xMax, yMin, yMax, zMin, zMax);
                dtmFeatureId[n] = dtm.AddPointFeature(randomPoints);
                }

            // Triangulate DTM

            dtm.Triangulate();

            // Validate DTM

            Helper.ValidationResult validationResult = Helper.ValidateDTM(dtm);
            if (reportValidation == true)
                {
                Console.WriteLine("Number Of DTM Points = {0}", validationResult.NumPoints);
                Console.WriteLine("Number Of DTM Lines = {0}", validationResult.NumLines);
                Console.WriteLine("Number Of DTM Triangles = {0}", validationResult.NumTriangles);
                Console.WriteLine("Number Of DTM Features = {0}", validationResult.NumFeatures);
                Console.WriteLine("DTM Area = {0}", validationResult.Area);
                Console.WriteLine("DTM Volume = {0}", validationResult.Volume);
                }

            // Delete Every Second Point Feature

            for (int n = 0; n < 100; n += 2)
                {
                dtm.DeleteFeatureById(dtmFeatureId[n]);
                }

            // Triangulate DTM

            dtm.Triangulate();

            // Validate DTM

            Helper.ValidationResult validationResult1 = Helper.ValidateDTM(dtm);

            if (reportValidation == true)
                {
                Console.WriteLine("Number Of DTM Points = {0}", validationResult1.NumPoints);
                Console.WriteLine("Number Of DTM Lines = {0}", validationResult1.NumLines);
                Console.WriteLine("Number Of DTM Triangles = {0}", validationResult1.NumTriangles);
                Console.WriteLine("Number Of DTM Features = {0}", validationResult1.NumFeatures);
                Console.WriteLine("DTM Area = {0}", validationResult1.Area);
                Console.WriteLine("DTM Volume = {0}", validationResult1.Volume);
                }


            // Delete All Remaing Point Features

            dtm.DeleteFeaturesByType(DTMFeatureType.PointFeature);

            // Check There Are No Points In The DTM
            // As Deleted Features Are Left In The DTM 
            // And Marked As Deleted When The DTM Is In A Tin State
            // It Must Be Firstly Converted To A Data State 
            // For The Following Assert Test

            DtmState dtmState = dtm.GetDtmState();
            if (dtmState != DtmState.Data)
                dtm.ConvertToDataState();
            Assert.AreEqual(0, dtm.VerticesCount);

            // Dispose DTM

            dtm.Dispose();

            }


        [Test]
        [Category("DTMFeatureTests")]
        public void AddAndDeleteLinearFeatures()
        {

            // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

            /// <summary>
            /// Create A DTM With 1000 Points
            /// </summary>

            bool reportValidation = true;
            double xMin = 1000.0;
            double xMax = 100000.0;
            double yMin = 1000.0;
            //            double yMax = 50000.0;
            double zMin = -10.0;
            double zMax = 100.0;
            DTM dtm = new DTM();
            DTMFeatureId[] dtmFeatureId = new DTMFeatureId[100];

            // Store 100 Break Line Features 

            for (int n = 0; n < 100; ++n)
            {
                BGEO.DPoint3d[] linearFeature = new BGEO.DPoint3d[2];
                linearFeature[0].X = xMin;
                linearFeature[0].Y = yMin + (double)n * 100.0;
                linearFeature[0].Z = zMin;
                linearFeature[1].X = xMax;
                linearFeature[1].Y = yMin + (double)n * 100.0;
                linearFeature[1].Z = zMax;
                BGEO.DPoint3d[] breakLine = Helper.StrokeLinearFeature(100.0, linearFeature);
                dtmFeatureId[n] = dtm.AddLinearFeature(breakLine, DTMFeatureType.Breakline);
            }

            // Triangulate DTM

            dtm.Triangulate();

            // Validate DTM

            Helper.ValidationResult validationResult = Helper.ValidateDTM(dtm);
            if (reportValidation == true)
            {
                Console.WriteLine("Number Of DTM Points = {0}", validationResult.NumPoints);
                Console.WriteLine("Number Of DTM Lines = {0}", validationResult.NumLines);
                Console.WriteLine("Number Of DTM Triangles = {0}", validationResult.NumTriangles);
                Console.WriteLine("Number Of DTM Features = {0}", validationResult.NumFeatures);
                Console.WriteLine("DTM Area = {0}", validationResult.Area);
                Console.WriteLine("DTM Volume = {0}", validationResult.Volume);
            }

            // Delete Every Second Point Feature

            for (int n = 0; n < 100; n += 2)
            {
                dtm.DeleteFeatureById(dtmFeatureId[n]);
            }

            // Triangulate DTM

            dtm.Triangulate();

            // Validate DTM

            Helper.ValidationResult validationResult1 = Helper.ValidateDTM(dtm);

            if (reportValidation == true)
            {
                Console.WriteLine("Number Of DTM Points = {0}", validationResult1.NumPoints);
                Console.WriteLine("Number Of DTM Lines = {0}", validationResult1.NumLines);
                Console.WriteLine("Number Of DTM Triangles = {0}", validationResult1.NumTriangles);
                Console.WriteLine("Number Of DTM Features = {0}", validationResult1.NumFeatures);
                Console.WriteLine("DTM Area = {0}", validationResult1.Area);
                Console.WriteLine("DTM Volume = {0}", validationResult1.Volume);
            }


            // Delete All Remaing Point Features

            dtm.DeleteFeaturesByType(DTMFeatureType.Breakline);

            // Check There Are No Points In The DTM
            // As Deleted Features Are Left In The DTM 
            // And Marked As Deleted When The DTM Is In A Tin State
            // It Must Be Firstly Converted To A Data State 
            // For The Following Assert Test

            DtmState dtmState = dtm.GetDtmState();
            if (dtmState != DtmState.Data)
                dtm.ConvertToDataState();
            Assert.AreEqual(0, dtm.VerticesCount);

            // Dispose DTM

            dtm.Dispose();

        }

        [Test]
        [Category("DTMFeatureTests")]
        public void AddAndBulkDeleteLinearFeatures()
        {

            // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

            /// <summary>
            /// Create A DTM With 1000 Points
            /// </summary>

            bool reportValidation = true;
            int numBreakLines = 10000;
            double xMin = 1000.0;
            double xMax = 100000.0;
            double yMin = 1000.0;
            //            double yMax = 50000.0;
            double zMin = -10.0;
            double zMax = 100.0;
            DTM dtm = new DTM();
            DTMFeatureId[] dtmFeatureId = new DTMFeatureId[numBreakLines];
            long [] dtmUserTag = new long[numBreakLines];
           
            // Store 10000 Break Line Features 

            for (int n = 0; n < numBreakLines ; ++n)
            {
                dtmUserTag[n] = n;
                BGEO.DPoint3d[] linearFeature = new BGEO.DPoint3d[2];
                linearFeature[0].X = xMin;
                linearFeature[0].Y = yMin + (double)n * 100.0;
                linearFeature[0].Z = zMin;
                linearFeature[1].X = xMax;
                linearFeature[1].Y = yMin + (double)n * 100.0;
                linearFeature[1].Z = zMax;
                BGEO.DPoint3d[] breakLine = Helper.StrokeLinearFeature(1000.0, linearFeature);
                dtmFeatureId[n] = dtm.AddLinearFeature(breakLine, DTMFeatureType.Breakline,dtmUserTag[n]);
            }

            // Triangulate DTM

            Console.WriteLine("Triangulating");
            dtm.Triangulate();
            Console.WriteLine("Triangulating Completed");

            // Validate DTM

            Helper.ValidationResult validationResult = Helper.ValidateDTM(dtm);
            if (reportValidation == true)
            {
                Console.WriteLine("Number Of DTM Points = {0}", validationResult.NumPoints);
                Console.WriteLine("Number Of DTM Lines = {0}", validationResult.NumLines);
                Console.WriteLine("Number Of DTM Triangles = {0}", validationResult.NumTriangles);
                Console.WriteLine("Number Of DTM Features = {0}", validationResult.NumFeatures);
                Console.WriteLine("DTM Area = {0}", validationResult.Area);
                Console.WriteLine("DTM Volume = {0}", validationResult.Volume);
            }

            // Bulk Delete Every Second DTM Feature By Id

            DTMFeatureId[] delDtmFeatureId = new DTMFeatureId[numBreakLines/2];
            for (int n = 0, m = 0 ; n < numBreakLines ; n += 2 , ++m )
            {
                if( m < numBreakLines / 2)
                {
                    delDtmFeatureId[m] = dtmFeatureId[n];
                }
            }

            Console.WriteLine("Bulk Deleting By Feature ID");
            dtm.BulkDeleteFeaturesByFeatureId(delDtmFeatureId);
            Console.WriteLine("Bulk Deleting By Feature ID Completed");

            // Validate DTM

            validationResult = Helper.ValidateDTM(dtm);
            if (reportValidation == true)
            {
                Console.WriteLine("Number Of DTM Points = {0}", validationResult.NumPoints);
                Console.WriteLine("Number Of DTM Lines = {0}", validationResult.NumLines);
                Console.WriteLine("Number Of DTM Triangles = {0}", validationResult.NumTriangles);
                Console.WriteLine("Number Of DTM Features = {0}", validationResult.NumFeatures);
                Console.WriteLine("DTM Area = {0}", validationResult.Area);
                Console.WriteLine("DTM Volume = {0}", validationResult.Volume);
            }

            // Bulk Delete DTM Features By UserTag

            Console.WriteLine("Bulk Deleting By User Tag");
            dtm.BulkDeleteFeaturesByUserTag(dtmUserTag);
            Console.WriteLine("Bulk Deleting By User Tag Completed");

            // Validate DTM

            validationResult = Helper.ValidateDTM(dtm);

            if (reportValidation == true)
            {
                Console.WriteLine("Number Of DTM Points = {0}", validationResult.NumPoints);
                Console.WriteLine("Number Of DTM Lines = {0}", validationResult.NumLines);
                Console.WriteLine("Number Of DTM Triangles = {0}", validationResult.NumTriangles);
                Console.WriteLine("Number Of DTM Features = {0}", validationResult.NumFeatures);
                Console.WriteLine("DTM Area = {0}", validationResult.Area);
                Console.WriteLine("DTM Volume = {0}", validationResult.Volume);
            }

            // Dispose DTM

            dtm.Dispose();

        }

        /// <summary>
        ///  Browsing method For Tin Errors
        /// </summary>


        class FeatureCounts
            {
            private int featureCount = 0;
            public int FeatureCount
                {
                get
                    {
                    return featureCount;
                    }
                set
                    {
                    featureCount = value;
                    }
                }
            }

        struct FeatureValues
            {
            public DTMFeatureId dtmFeatureId;
            public long dtmUserTag;
            public double X;
            public double Y;
            public double Z;
            public FeatureValues(DTMFeatureId featureId, long userTag, double xx, double yy, double zz)
                {
                dtmFeatureId = featureId;
                dtmUserTag = userTag;
                X = xx;
                Y = yy;
                Z = zz;
                }

            }

        private bool browseDtmFeatures(DTMFeatureInfo info, object userArg)
            {
            bool report = false;
            m_featureId = info.DtmFeatureId;
            if (userArg != null)
                {
                FeatureCounts featureCounts = (FeatureCounts)userArg;
                ++featureCounts.FeatureCount;
                };

            if (report)
                Console.WriteLine("Feature Type = {0}", info.DtmFeatureType);
            return true;
            }

        private bool browseDtmPointFeatures(DTMFeatureInfo info, object userArg)
            {
            bool report = false;
            var pts = info.Points;
            if (report)
                Console.WriteLine("Feature Id = {0} User Tag = {1} ** X = {2} Y = {3} Z = {4}", info.DtmFeatureId, info.DtmUserTag, pts[0].X, pts[0].Y, pts[0].Z);

            if (userArg != null)
                {
                ArrayList dtmFeatureValues = (ArrayList)userArg;

                // Scan Array List For Usertags And Check Coordinates

                bool coordinatesCorrect = false;
                foreach (FeatureValues featureValues in dtmFeatureValues)
                    {
                    if (coordinatesCorrect == false && featureValues.dtmUserTag == info.DtmUserTag)
                        {
                        double dx = pts[0].X - featureValues.X;
                        double dy = pts[0].Y - featureValues.Y;
                        double dz = pts[0].Z - featureValues.Z;
                        double ds = Math.Sqrt(dx * dx + dy * dy + dz * dz);
                        if (ds <= 0.000000001)
                            coordinatesCorrect = true;
                        }
                    }

                if (coordinatesCorrect == false)
                    {
                    throw new Exception("UserTag Coordinates Are Not Correct");
                    }


                // Scan Array List For Feature Ids And Check Coordinates

                coordinatesCorrect = false;
                foreach (FeatureValues featureValues in dtmFeatureValues)
                    {

                    if (coordinatesCorrect == false && DTMFeatureId.Equals(featureValues.dtmFeatureId, info.DtmFeatureId))
                        {
                        double dx = pts[0].X - featureValues.X;
                        double dy = pts[0].Y - featureValues.Y;
                        double dz = pts[0].Z - featureValues.Z;
                        double ds = Math.Sqrt(dx * dx + dy * dy + dz * dz);
                        if (ds <= 0.000000001)
                            coordinatesCorrect = true;
                        }
                    }

                if (coordinatesCorrect == false)
                    {
                    throw new Exception("FeatureId Coordinates Are Not Correct");
                    }


                };

            return true;
            }

        /// <summary>
        ///  Check FeatureId And UserTag Coordinates
        /// </summary>
        [Test]
        [Category("DTMFeatureTests")]
        public void CheckFeatureIdAndUserTagCoordinates()
            {
            // Specific test for dtm features
            DTM dtm = new DTM();

            // Stops Initial allocation of memory for 2500000 points 
            dtm.SetMemoryAllocationParameters(10000, 10000);

            bool reportValidation = false;
            double xMin = 1000.0;
            double xMax = 100000.0;
            double yMin = 1000.0;
            double yMax = 50000.0;
            double zMin = -10.0;
            double zMax = 100.0;

            // Create An Array List To Store DTM Feature Id's, UserTags And Coordinate Values

            ArrayList dtmFeatureValues = new ArrayList();

            // Store 10000 Point Features With 1 Point Per Feature

            for (int n = 0; n < 100; ++n)
                {
                BGEO.DPoint3d[] randomPoints = Helper.CreateRandomPoints(n, 1, xMin, xMax, yMin, yMax, zMin, zMax);
                DTMFeatureId dtmFeatureId = dtm.AddPointFeature(randomPoints, n);
                FeatureValues featureValues = new FeatureValues(dtmFeatureId, n, randomPoints[0].X, randomPoints[0].Y, randomPoints[0].Z);
                dtmFeatureValues.Add(featureValues);
                }

            //  Write Feature Values Prior to Triangulation

            if (reportValidation)
                {
                foreach (FeatureValues featureValues in dtmFeatureValues)
                    {
                    Console.WriteLine("Feature Id = {0} User Tag = {1} ** X = {2} Y = {3} Z = {4}", featureValues.dtmFeatureId, featureValues.dtmUserTag, featureValues.X, featureValues.Y, featureValues.Z);
                    }
                }


            // build the dtm

            dtm.Triangulate();

            // Browse Features With Tin Errors

            FeatureCounts featureCount = new FeatureCounts();
            featureCount.FeatureCount = 0;
            LinearFeaturesBrowsingDelegate hdlP = new LinearFeaturesBrowsingDelegate(browseDtmFeatures);
            dtm.BrowseFeaturesWithTinErrors(hdlP, featureCount);
            if (reportValidation == true)
                Console.WriteLine("Number Of Features With Tin Errors = {0}", featureCount.FeatureCount);

            //  Browse Point Features And Check Coordinate Values Are Correct

            featureCount.FeatureCount = 0;
            PointFeaturesBrowsingCriteria pointFeaturesCriteria = new PointFeaturesBrowsingCriteria();
            PointFeaturesBrowsingDelegate pointFeaturesDelegate = new PointFeaturesBrowsingDelegate(browseDtmPointFeatures);
            dtm.BrowsePointFeatures(pointFeaturesCriteria, pointFeaturesDelegate, dtmFeatureValues);
            if (reportValidation == true)
                Console.WriteLine("Number Of Point Features = {0}", featureCount.FeatureCount);

            dtm.Dispose();

            }


        /// <summary>
        ///  Add And Delete Different Features
        /// </summary>
        [Test]
        [Category("Add And Delete Different Features")]
        public void AddAndDelelteDifferentFeatures()
            {

            // Specific test for dtm features
            DTM dtm = new DTM();

            // RobC - Added 05/08/2009 - Stops Initial allocation of memory for 2500000 points 
            dtm.SetMemoryAllocationParameters(100000, 100000);

            bool reportValidation = true;
            double xMin = 1000.0;
            double xMax = 100000.0;
            double yMin = 1000.0;
            double yMax = 50000.0;
            double zMin = -10.0;
            double zMax = 100.0;

            // Store 10000 Points

            BGEO.DPoint3d[] randomPoints = Helper.CreateRandomPoints(1024, 10000, xMin, xMax, yMin, yMax, zMin, zMax);
            dtm.AddPoints(randomPoints);

            // Create An Array List To Store DTM Feature Id's

            ArrayList dtmFeatureIds = new ArrayList();

            // Store 100 Point Features With 100 Points Per Feature

            for (int n = 0; n < 100; ++n)
                {
                randomPoints = Helper.CreateRandomPoints(n, 100, xMin, xMax, yMin, yMax, zMin, zMax);
                dtmFeatureIds.Add(dtm.AddPointFeature(randomPoints, 50));
                }

            // Store 100 Break Line Features 

            for (int n = 0; n < 100; ++n)
                {
                BGEO.DPoint3d[] linearFeature = new BGEO.DPoint3d[2];
                linearFeature[0].X = xMin;
                linearFeature[0].Y = yMin + (double)n * 100.0;
                linearFeature[0].Z = zMin;
                linearFeature[1].X = xMax;
                linearFeature[1].Y = yMin + (double)n * 100.0;
                linearFeature[1].Z = zMax;
                BGEO.DPoint3d[] breakLine = Helper.StrokeLinearFeature(100.0, linearFeature);
                //                if( reportValidation == true ) Console.WriteLine("Number Of Break Line Points = {0}", breakLine.Length);
                dtmFeatureIds.Add(dtm.AddLinearFeature(breakLine, DTMFeatureType.Breakline, 100));
                }

            // build the dtm

            dtm.Triangulate();

            // Browse Features With Tin Errors

            FeatureCounts featureCount = new FeatureCounts();
            featureCount.FeatureCount = 0;
            LinearFeaturesBrowsingDelegate hdlP = new LinearFeaturesBrowsingDelegate(browseDtmFeatures);
            dtm.BrowseFeaturesWithTinErrors(hdlP, featureCount);
            if (reportValidation == true)
                Console.WriteLine("Number Of Features With Tin Errors = {0}", featureCount.FeatureCount);

            //  Browse Features With User Tag == 50

            featureCount.FeatureCount = 0;
            dtm.BrowseFeaturesWithUserTag(50, hdlP, featureCount);
            if (reportValidation == true)
                Console.WriteLine("Number Of Features With UserTag[50] = {0}", featureCount.FeatureCount);

            //  Browse Features With User Tag == 100

            featureCount.FeatureCount = 0;
            dtm.BrowseFeaturesWithUserTag(100, hdlP, featureCount);
            if (reportValidation == true)
                Console.WriteLine("Number Of Features With UserTag[100] = {0}", featureCount.FeatureCount);

            //  Browse Features With Feature Id == 100

            featureCount.FeatureCount = 0;
            dtm.BrowseFeaturesWithFeatureId(DTMFeatureId.FromString("100"), hdlP, featureCount);
            if (reportValidation == true)
                Console.WriteLine("Number Of Features With featureId[100] = {0}", featureCount.FeatureCount);

            //  Browse Break Line Features 

            featureCount.FeatureCount = 0;
            LinearFeaturesBrowsingCriteria linearFeaturesCriteria = new LinearFeaturesBrowsingCriteria();
            dtm.BrowseLinearFeatures(linearFeaturesCriteria, DTMFeatureType.Breakline, hdlP, featureCount);
            if (reportValidation == true)
                Console.WriteLine("Number Of Break Lines = {0}", featureCount.FeatureCount);

            // Validate DTM

            Helper.ValidationResult validationResult1 = Helper.ValidateDTM(dtm);

            if (reportValidation == true)
                {
                Console.WriteLine("Number Of DTM Points = {0}", validationResult1.NumPoints);
                Console.WriteLine("Number Of DTM Lines = {0}", validationResult1.NumLines);
                Console.WriteLine("Number Of DTM Triangles = {0}", validationResult1.NumTriangles);
                Console.WriteLine("Number Of DTM Features = {0}", validationResult1.NumFeatures);
                Console.WriteLine("DTM Area = {0}", validationResult1.Area);
                Console.WriteLine("DTM Volume = {0}", validationResult1.Volume);
                }

            // Count Feature Types

            int breakLineCounts = 0;
            foreach (DTMFeatureId dtmFeatureId in dtmFeatureIds)
                {
                DTMFeatureId featureId = dtmFeatureId;
                DTMFeature dtmFeature = dtm.GetFeatureById(featureId);
                Assert.AreEqual(dtmFeatureId, dtmFeature.Id);
                if (dtmFeature is DTMLinearFeature)
                    {
                    DTMLinearFeature dtmLinearFeature = dtmFeature as DTMLinearFeature;
                    if (dtmLinearFeature.FeatureType == DTMFeatureType.Breakline)
                        ++breakLineCounts;
                    int elementsCount = dtmLinearFeature.ElementsCount;
// ToDo...Standalone
                    //BCG.LinearElement[] dtmElements = Bentley.TerrainModelNET.LinearGeometry.Helper.DTMLinearFeatureGetElements (dtmLinearFeature);
                    //for (int n = 0; n < elementsCount; ++n)
                    //    {
                    //    BCG.LineString breakline = dtmElements[n] as BCG.LineString;
                    //    BCG.LinearPointCollection vertices = breakline.StrokeByEndPoints();
                    //    BGEO.DPoint3d[] breaklinePoints = vertices.GetVertices();
                    //    //                        if (reportValidation == true) Console.WriteLine("DtmFeatureId = {0} DtmFeatureType = {1} Number Of Points = {2}", dtmFeature.Id, dtmFeature.FeatureType, breaklinePoints.Length);
                    //    }
                    }

                }

            Console.WriteLine("Number Of Break Lines = {0}", breakLineCounts);

            // Specific test for dtm features
            DTM dtm1 = new DTM();

            // RobC - Added 05/08/2009 - Stops Initial allocation of memory for 2500000 points 
            dtm1.SetMemoryAllocationParameters(100000, 100000);

            // Store 100 Break Line Features 

            for (int n = 0; n < 100; ++n)
                {
                BGEO.DPoint3d[] linearFeature = new BGEO.DPoint3d[2];
                linearFeature[0].X = xMin;
                linearFeature[0].Y = yMin + (double)n * 100.0;
                linearFeature[0].Z = zMin;
                linearFeature[1].X = xMax;
                linearFeature[1].Y = yMin + (double)n * 100.0;
                linearFeature[1].Z = zMax;
                BGEO.DPoint3d[] breakLine = Helper.StrokeLinearFeature(100.0, linearFeature);
                //                if (reportValidation == true) Console.WriteLine("Number Of Break Line Points = {0}", breakLine.Length);
                dtmFeatureIds.Add(dtm1.AddLinearFeature(breakLine, DTMFeatureType.Contour));
                }

            // build the dtm

            dtm1.Triangulate();

            // merge the DTMs

            dtm.Merge(dtm1);

            validationResult1 = Helper.ValidateDTM(dtm);

            if (reportValidation == true)
                {
                Console.WriteLine("Number Of DTM Points = {0}", validationResult1.NumPoints);
                Console.WriteLine("Number Of DTM Lines = {0}", validationResult1.NumLines);
                Console.WriteLine("Number Of DTM Triangles = {0}", validationResult1.NumTriangles);
                Console.WriteLine("Number Of DTM Features = {0}", validationResult1.NumFeatures);
                Console.WriteLine("DTM Area = {0}", validationResult1.Area);
                Console.WriteLine("DTM Volume = {0}", validationResult1.Volume);
                }


            breakLineCounts = 0;
            int missingBreakLineCounts = 0;
            foreach (DTMFeatureId dtmFeatureId in dtmFeatureIds)
                {
                DTMFeatureId featureId = dtmFeatureId;
                DTMFeature dtmFeature = dtm.GetFeatureById(featureId);
                if (dtmFeature != null)
                    {
                    Assert.AreEqual(dtmFeatureId, dtmFeature.Id);
                    if (dtmFeature is DTMLinearFeature)
                        {
                        DTMLinearFeature dtmLinearFeature = dtmFeature as DTMLinearFeature;
                        if (dtmLinearFeature.FeatureType == DTMFeatureType.Breakline)
                            ++breakLineCounts;
                        int elementsCount = dtmLinearFeature.ElementsCount;
// ToDo...Standalone
                        //BCG.LinearElement[] dtmElements = Bentley.TerrainModelNET.LinearGeometry.Helper.DTMLinearFeatureGetElements (dtmLinearFeature);
                        //for (int n = 0; n < elementsCount; ++n)
                        //    {
                        //    BCG.LineString breakline = dtmElements[n] as BCG.LineString;
                        //    BCG.LinearPointCollection vertices = breakline.StrokeByEndPoints();
                        //    BGEO.DPoint3d[] breaklinePoints = vertices.GetVertices();
                        //    if (reportValidation == true)
                        //        Console.WriteLine("DtmFeatureId = {0} DtmFeatureType = {1} Number Of Points = {2}", dtmFeature.Id, dtmFeature.FeatureType, breaklinePoints.Length);
                        //    }
                        }
                    }
                else
                    {
                    ++missingBreakLineCounts;
                    }
                }
            Console.WriteLine("Number Of missing Break Lines = {0} Number Of Break Lines = {1}", missingBreakLineCounts, breakLineCounts);

            //  Browse Break Line Features 

            featureCount.FeatureCount = 0;
            dtm.BrowseLinearFeatures(linearFeaturesCriteria, DTMFeatureType.Breakline, hdlP, featureCount);
            if (reportValidation == true)
                Console.WriteLine("Number Of Break Lines = {0}", featureCount.FeatureCount);
            dtm1.Dispose();
            dtm.Dispose();
            }
        }
    }
