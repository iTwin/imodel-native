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
    public class DTMSideSlopeTests : DTMUNitTest
    {

        static String DataDirectory = @"Bentley.Civil.Dtm.NUnit.dll\DTMSideSlopeTest\DTMSideSlopeTest000";
        static String DataPath = Helper.GetTestDataLocation () + DataDirectory;

        /// <summary>
        /// DTMSideSlopeTest
        /// </summary>
        /// <returns></returns>
        [Test]
        [Category("DTMSideSlopeTests")]
        public void SideSlopeTestToSurface()
        {
            /// <summary>
            /// DTMSideSlopeTest1
            /// </summary>

            // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

            double cutSlope = 0.333;
            double fillSlope = 0.3333;

            // Read DTM From File

            DTM groundDTM = DTM.CreateFromFile(DataPath + "\\ground.dtm");

            // Read Side Slope Alignment from File

            BGEO.DPoint3d[] sideSlopeAlignment = DTM.ImportDPoint3DArrayFromXYZFile(DataPath + "\\sideSlopeAlignment.xyz");

            // Create Slope Table

            DTMSlopeTable slopeTable = new DTMSlopeTable();
            slopeTable.AddSlopeRange(0, 10, 1.0);
            slopeTable.AddSlopeRange(10, 20, 5.0);
            slopeTable.AddSlopeRange(20, 30, 10.0);
            slopeTable.AddSlopeRange(30, 40, 1.0);

            // Create Side Slope Input

            double cornerStrokeTolerance = 4.0;       //  Maximum Linear Distance Between Consecutive Corner Side Slope Radial End Points
            double pointToPointTolerance = 0.001;     //  Minimum Linear Distance Between Consecutive Side Slope Radial Start Points
            Int64 radialBreaklineTag = -11111111111;  //  User Tag Assigned To Side Slope Break Lines  
            Int64 sideSlopeElementTag = -22222222222; //  User Tag Assigned To The Side Slope Alignment Break Line
            DTMSideSlopeInput dtmSideSlopeInput = new DTMSideSlopeInput(slopeTable,DTMSideSlopeDirection.Right, DTMSideSlopeCornerOption.Rounded, DTMSideSlopeStrokeCornerOption.Stroke, cornerStrokeTolerance, pointToPointTolerance, radialBreaklineTag, sideSlopeElementTag);

            // Scan Alignment And Populate Side Slope Input

            BGEO.DPoint3d startPoint = new BGEO.DPoint3d();
            for (int n = 0; n < sideSlopeAlignment.Length; ++n)
            {
                startPoint.X = sideSlopeAlignment[n].X;
                startPoint.Y = sideSlopeAlignment[n].Y;
                startPoint.Z = sideSlopeAlignment[n].Z;
                dtmSideSlopeInput.AddRadialToSurface (startPoint, DTMSideSlopeRadialOption.Radial, DTMSideSlopeCutFillOption.CutAndFill, groundDTM, cutSlope, fillSlope);
            }

            // Calculate Side Slope DTMs

            DTM[] sideSlopeDtms = dtmSideSlopeInput.CalculateSideSlopes();

            //  Browse Slope Toe Features And Store In An Array List

            ArrayList dtmSlopeToeFeatures = new ArrayList();
            LinearFeaturesBrowsingCriteria sideSlopeCriteria = new LinearFeaturesBrowsingCriteria();
            LinearFeaturesBrowsingDelegate sideSlopeBrowser = new LinearFeaturesBrowsingDelegate(browseSideSlopeFeatures);
            for (int n = 0; n < sideSlopeDtms.Length; ++n)
            {
                sideSlopeDtms[n].BrowseLinearFeatures(sideSlopeCriteria, DTMFeatureType.SlopeToe, sideSlopeBrowser, dtmSlopeToeFeatures);
            }

            foreach (BGEO.DPoint3d[] slopeToe in dtmSlopeToeFeatures)
            {
                Console.WriteLine("Number Of Slope Toe Points = {0}", slopeToe.Length);
                for (int l = 0; l < slopeToe.Length; ++l)
                {
                    //                   Console.WriteLine("Slope Toe Point[{0}] ** X = {1} Y = {2} Z = {3}",l,slopeToe[l].X,slopeToe[l].Y,slopeToe[l].Z);
                }
            }


            // Merge Ground DTM and Side Slopes DTM To Create Design DTM

            for (int n = 0; n < sideSlopeDtms.Length; ++n)
            {
                // Clone DTMS

                DTM designDTM = groundDTM.Clone();
                DTM sideSlopeDTM = sideSlopeDtms[n].Clone();

                // Triangulate 

                sideSlopeDTM.Triangulate();

//                Console.WriteLine("Saving DTM");
//                designDTM.Save(@"D:\CertData\CivilPlatform\SideSlopes00\design.bcdtm");
//                sideSlopeDTM.Save(@"D:\CertData\CivilPlatform\SideSlopes00\sideSlope.bcdtm");


                // Merge Cloned DTM's

                designDTM.Merge(sideSlopeDTM);

                // Dispose Cloned DTMs

                sideSlopeDTM.Dispose();
                designDTM.Dispose();
            }

            //  Dispose Ground DTM

            groundDTM.Dispose();
        }


        /// <summary>
        /// DTMSideSlopeTest
        /// </summary>
        /// <returns></returns>
        [Test]
        [Category("DTMSideSlopeTests")]
        public void SideSlopeTestToElevation()
        {
            /// <summary>
            /// DTMSideSlopeTest1
            /// </summary>

            // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

            double cutSlope = 0.333;
            double fillSlope = 0.3333;

            // Read DTM From File

            DTM groundDTM = DTM.CreateFromFile(DataPath + "\\ground.dtm");

            // Read Side Slope Alignment from File

            BGEO.DPoint3d[] sideSlopeAlignment = DTM.ImportDPoint3DArrayFromXYZFile(DataPath + "\\sideSlopeAlignment.xyz");

            for (int n = 0; n < sideSlopeAlignment.Length; ++n)
            {
                Console.WriteLine("Point = {0}  ** X = {1} Y = {2} Z = {3}", n, sideSlopeAlignment[n].X, sideSlopeAlignment[n].Y, sideSlopeAlignment[n].Z);
            }


            // Create Side Slope Input

            double cornerStrokeTolerance = 1.0;       //  Maximum Linear Distance Between Consecutive Corner Side Slope Radial End Points
            double pointToPointTolerance = 0.001;     //  Minimum Linear Distance Between Consecutive Side Slope Radial Start Points
            Int64 radialBreaklineTag = -11111111111;  //  User Tag Assigned To Side Slope Break Lines  
            Int64 sideSlopeElementTag = -22222222222; //  User Tag Assigned To The Side Slope Alignment Break Line
            DTMSideSlopeInput dtmSideSlopeInput = new DTMSideSlopeInput(DTMSideSlopeDirection.Right, DTMSideSlopeCornerOption.Rounded, DTMSideSlopeStrokeCornerOption.Stroke, cornerStrokeTolerance, pointToPointTolerance, radialBreaklineTag, sideSlopeElementTag);

            // Scan Alignment And Populate Side Slope Input

            BGEO.DPoint3d startPoint = new BGEO.DPoint3d();
            for (int n = 0; n < sideSlopeAlignment.Length; ++n)
            {
                startPoint.X = sideSlopeAlignment[n].X;
                startPoint.Y = sideSlopeAlignment[n].Y;
                startPoint.Z = sideSlopeAlignment[n].Z;
                dtmSideSlopeInput.AddRadialToElevation(startPoint,DTMSideSlopeRadialOption.Radial, DTMSideSlopeCutFillOption.CutAndFill,groundDTM,cutSlope,fillSlope,startPoint.Z + 1.0);
            }

            // Calculate Side Slope DTMs

            DTM[] sideSlopeDtms = dtmSideSlopeInput.CalculateSideSlopes();

            //  Browse Slope Toe Features And Store In An Array List

            ArrayList dtmSlopeToeFeatures = new ArrayList();
            LinearFeaturesBrowsingCriteria sideSlopeCriteria = new LinearFeaturesBrowsingCriteria();
            LinearFeaturesBrowsingDelegate sideSlopeBrowser = new LinearFeaturesBrowsingDelegate(browseSideSlopeFeatures);
            for (int n = 0; n < sideSlopeDtms.Length; ++n)
            {
                sideSlopeDtms[n].BrowseLinearFeatures(sideSlopeCriteria, DTMFeatureType.SlopeToe, sideSlopeBrowser, dtmSlopeToeFeatures);
            }

            foreach (BGEO.DPoint3d[] slopeToe in dtmSlopeToeFeatures)
            {
                Console.WriteLine("Number Of Slope Toe Points = {0}", slopeToe.Length);
                for (int l = 0; l < slopeToe.Length; ++l)
                {
                    Console.WriteLine("Slope Toe Point[{0}] ** X = {1} Y = {2} Z = {3}", l, slopeToe[l].X, slopeToe[l].Y, slopeToe[l].Z);
                }
            }

            // Merge Ground DTM and Side Slopes DTM To Create Design DTM

            for (int n = 0; n < sideSlopeDtms.Length; ++n)
            {
                // Clone DTMS

                DTM designDTM = groundDTM.Clone();
                DTM sideSlopeDTM = sideSlopeDtms[n].Clone();

                // Triangulate 

                sideSlopeDTM.Triangulate();

//                Console.WriteLine("Saving DTM");
//                designDTM.Save(@"D:\CertData\CivilPlatform\SideSlopes00\design.bcdtm");
//                sideSlopeDTM.Save(@"D:\CertData\CivilPlatform\SideSlopes00\sideSlope.bcdtm");

                // Merge Cloned DTM's

                Console.WriteLine("Merging Side Slope DTM");
                designDTM.Merge(sideSlopeDTM);

                // Dispose Cloned DTMs

                sideSlopeDTM.Dispose();
                designDTM.Dispose();
            }

            //  Dispose Ground DTM

            groundDTM.Dispose();
        }

        /// <summary>
        /// DTMSideSlopeTest
        /// </summary>
        /// <returns></returns>
        [Test]
        [Category("DTMSideSlopeTests")]
        public void SideSlopeTestToDeltaElevation()
        {
            /// <summary>
            /// DTMSideSlopeTest1
            /// </summary>

            // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

            double cutSlope = 0.333;
            double fillSlope = 0.3333;

            // Read DTM From File

            DTM groundDTM = DTM.CreateFromFile(DataPath + "\\ground.dtm");

            // Read Side Slope Alignment from File

            BGEO.DPoint3d[] sideSlopeAlignment = DTM.ImportDPoint3DArrayFromXYZFile(DataPath + "\\sideSlopeAlignment.xyz");

            for (int n = 0; n < sideSlopeAlignment.Length; ++n)
            {
                Console.WriteLine("Point = {0}  ** X = {1} Y = {2} Z = {3}", n, sideSlopeAlignment[n].X, sideSlopeAlignment[n].Y, sideSlopeAlignment[n].Z);
            }


            // Create Side Slope Input

            double cornerStrokeTolerance = 1.0;       //  Maximum Linear Distance Between Consecutive Corner Side Slope Radial End Points
            double pointToPointTolerance = 0.001;     //  Minimum Linear Distance Between Consecutive Side Slope Radial Start Points
            Int64 radialBreaklineTag = -11111111111;  //  User Tag Assigned To Side Slope Break Lines  
            Int64 sideSlopeElementTag = -22222222222; //  User Tag Assigned To The Side Slope Alignment Break Line
            DTMSideSlopeInput dtmSideSlopeInput = new DTMSideSlopeInput(DTMSideSlopeDirection.Right, DTMSideSlopeCornerOption.Rounded, DTMSideSlopeStrokeCornerOption.Stroke, cornerStrokeTolerance, pointToPointTolerance, radialBreaklineTag, sideSlopeElementTag);

            // Scan Alignment And Populate Side Slope Input

            BGEO.DPoint3d startPoint = new BGEO.DPoint3d();
            for (int n = 0; n < sideSlopeAlignment.Length; ++n)
            {
                startPoint.X = sideSlopeAlignment[n].X;
                startPoint.Y = sideSlopeAlignment[n].Y;
                startPoint.Z = sideSlopeAlignment[n].Z;
                dtmSideSlopeInput.AddRadialToDeltaElevation(startPoint,DTMSideSlopeRadialOption.Radial, DTMSideSlopeCutFillOption.CutAndFill,groundDTM, cutSlope,fillSlope, 1.0);
            }

            // Calculate Side Slope DTMs

            DTM[] sideSlopeDtms = dtmSideSlopeInput.CalculateSideSlopes();

            //  Browse Slope Toe Features And Store In An Array List

            ArrayList dtmSlopeToeFeatures = new ArrayList();
            LinearFeaturesBrowsingCriteria sideSlopeCriteria = new LinearFeaturesBrowsingCriteria();
            LinearFeaturesBrowsingDelegate sideSlopeBrowser = new LinearFeaturesBrowsingDelegate(browseSideSlopeFeatures);
            for (int n = 0; n < sideSlopeDtms.Length; ++n)
            {
                sideSlopeDtms[n].BrowseLinearFeatures(sideSlopeCriteria, DTMFeatureType.SlopeToe, sideSlopeBrowser, dtmSlopeToeFeatures);
            }

            foreach (BGEO.DPoint3d[] slopeToe in dtmSlopeToeFeatures)
            {
                Console.WriteLine("Number Of Slope Toe Points = {0}", slopeToe.Length);
                for (int l = 0; l < slopeToe.Length; ++l)
                {
                    Console.WriteLine("Slope Toe Point[{0}] ** X = {1} Y = {2} Z = {3}", l, slopeToe[l].X, slopeToe[l].Y, slopeToe[l].Z);
                }
            }

            // Merge Ground DTM and Side Slopes DTM To Create Design DTM

            for (int n = 0; n < sideSlopeDtms.Length; ++n)
            {
                // Clone DTMS

                DTM designDTM = groundDTM.Clone();
                DTM sideSlopeDTM = sideSlopeDtms[n].Clone();

                // Triangulate 

                sideSlopeDTM.Triangulate();

                Console.WriteLine("Saving DTM");
                //ToDo Do we? designDTM.Save(@"D:\CertData\CivilPlatform\SideSlopes00\design.bcdtm");
                //ToDo Do we? sideSlopeDTM.Save(@"D:\CertData\CivilPlatform\SideSlopes00\sideSlope.bcdtm");

                // Merge Cloned DTM's

                designDTM.Merge(sideSlopeDTM);

                // Dispose Cloned DTMs

                sideSlopeDTM.Dispose();
                designDTM.Dispose();
            }

            //  Dispose Ground DTM

            groundDTM.Dispose();
        }


        /// <summary>
        /// DTMSideSlopeTest
        /// </summary>
        /// <returns></returns>
        [Test]
        [Category("DTMSideSlopeTests")]
        public void SideSlopeTestToSurfaceWolfGang()
        {
            /// <summary>
            /// DTMSideSlopeTest1
            /// </summary>

            // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

            double cutSlope = 0.333;
            double fillSlope = 0.3333;

            // Read DTM From File

            DTM groundDTM = DTM.CreateFromFile(DataPath + "\\Ground2TestSideSlopes.tin");

            // Read Side Slope Alignment from File

//            BGEO.DPoint3d[] sideSlopeAlignment = DTM.ImportDPoint3DArrayFromXYZFile(DataPath + "\\sideSlopeAlignment.xyz");
   
            BGEO.DPoint3d[] sideSlopeAlignment = new BGEO.DPoint3d[23] ;

            sideSlopeAlignment[0].X = 37698.516; sideSlopeAlignment[0].Y = 11203.901; sideSlopeAlignment[0].Z = 41.062;
            sideSlopeAlignment[1].X = 37698.516; sideSlopeAlignment[1].Y = 11213.901; sideSlopeAlignment[1].Z = 40.969;
            sideSlopeAlignment[2].X = 37698.516; sideSlopeAlignment[2].Y = 11223.901; sideSlopeAlignment[2].Z = 40.875;
            sideSlopeAlignment[3].X = 37698.516; sideSlopeAlignment[3].Y = 11233.901; sideSlopeAlignment[3].Z = 40.781;
            sideSlopeAlignment[4].X = 37698.516; sideSlopeAlignment[4].Y = 11243.901; sideSlopeAlignment[4].Z = 40.687;
            sideSlopeAlignment[5].X = 37698.516; sideSlopeAlignment[5].Y = 11253.901; sideSlopeAlignment[5].Z = 40.594;
            sideSlopeAlignment[6].X = 37698.516; sideSlopeAlignment[6].Y = 11263.901; sideSlopeAlignment[6].Z = 40.500;
            sideSlopeAlignment[7].X = 37698.516; sideSlopeAlignment[7].Y = 11273.901; sideSlopeAlignment[7].Z = 40.406;
            sideSlopeAlignment[8].X = 37698.516; sideSlopeAlignment[8].Y = 11283.901; sideSlopeAlignment[8].Z = 40.312;
            sideSlopeAlignment[9].X = 37698.516; sideSlopeAlignment[9].Y = 11293.901; sideSlopeAlignment[9].Z = 40.219;
            sideSlopeAlignment[10].X = 37698.516; sideSlopeAlignment[10].Y = 11303.901; sideSlopeAlignment[10].Z = 40.125;
            sideSlopeAlignment[11].X = 37698.516; sideSlopeAlignment[11].Y = 11313.901; sideSlopeAlignment[11].Z = 40.031;
            sideSlopeAlignment[12].X = 37698.516; sideSlopeAlignment[12].Y = 11323.901; sideSlopeAlignment[12].Z = 41.062;
            sideSlopeAlignment[13].X = 37698.516; sideSlopeAlignment[13].Y = 11333.901; sideSlopeAlignment[13].Z = 39.937;
            sideSlopeAlignment[14].X = 37698.516; sideSlopeAlignment[14].Y = 11343.901; sideSlopeAlignment[14].Z = 39.844;
            sideSlopeAlignment[15].X = 37698.516; sideSlopeAlignment[15].Y = 11353.901; sideSlopeAlignment[15].Z = 39.750;
            sideSlopeAlignment[16].X = 37698.516; sideSlopeAlignment[16].Y = 11363.901; sideSlopeAlignment[16].Z = 39.656;
            sideSlopeAlignment[17].X = 37698.516; sideSlopeAlignment[17].Y = 11373.901; sideSlopeAlignment[17].Z = 39.592;
            sideSlopeAlignment[18].X = 37698.516; sideSlopeAlignment[18].Y = 11383.901; sideSlopeAlignment[18].Z = 39.469;
            sideSlopeAlignment[19].X = 37698.516; sideSlopeAlignment[19].Y = 11393.901; sideSlopeAlignment[19].Z = 39.375;
            sideSlopeAlignment[20].X = 37698.516; sideSlopeAlignment[20].Y = 11403.901; sideSlopeAlignment[20].Z = 39.281;
            sideSlopeAlignment[21].X = 37698.516; sideSlopeAlignment[21].Y = 11413.901; sideSlopeAlignment[21].Z = 39.187;
            sideSlopeAlignment[22].X = 37698.516; sideSlopeAlignment[22].Y = 11423.901; sideSlopeAlignment[22].Z = 39.000;


            // Create Slope Table

            DTMSlopeTable slopeTable = new DTMSlopeTable();
            slopeTable.AddSlopeRange(0, 10, 1.0);
            slopeTable.AddSlopeRange(10, 20, 5.0);
            slopeTable.AddSlopeRange(20, 30, 10.0);
            slopeTable.AddSlopeRange(30, 40, 1.0);

            // Create Side Slope Input

            double cornerStrokeTolerance = 4.0;       //  Maximum Linear Distance Between Consecutive Corner Side Slope Radial End Points
            double pointToPointTolerance = 0.001;     //  Minimum Linear Distance Between Consecutive Side Slope Radial Start Points
            Int64 radialBreaklineTag = -11111111111;  //  User Tag Assigned To Side Slope Break Lines  
            Int64 sideSlopeElementTag = -22222222222; //  User Tag Assigned To The Side Slope Alignment Break Line
            DTMSideSlopeInput dtmSideSlopeInput = new DTMSideSlopeInput(slopeTable, DTMSideSlopeDirection.Right, DTMSideSlopeCornerOption.Rounded, DTMSideSlopeStrokeCornerOption.Stroke, cornerStrokeTolerance, pointToPointTolerance, radialBreaklineTag, sideSlopeElementTag);

            // Scan Alignment And Populate Side Slope Input

            BGEO.DPoint3d startPoint = new BGEO.DPoint3d();
            for (int n = 0; n < sideSlopeAlignment.Length; ++n)
            {
                startPoint.X = sideSlopeAlignment[n].X;
                startPoint.Y = sideSlopeAlignment[n].Y;
                startPoint.Z = sideSlopeAlignment[n].Z;
                dtmSideSlopeInput.AddRadialToSurface (startPoint, DTMSideSlopeRadialOption.Radial, DTMSideSlopeCutFillOption.FillOnly, groundDTM, cutSlope, fillSlope); // was AddRadialToCutFillSurface
            }

            // Calculate Side Slope DTMs

            DTM[] sideSlopeDtms = dtmSideSlopeInput.CalculateSideSlopes();

            //  Browse Slope Toe Features And Store In An Array List

            ArrayList dtmSlopeToeFeatures = new ArrayList();
            LinearFeaturesBrowsingCriteria sideSlopeCriteria = new LinearFeaturesBrowsingCriteria();
            LinearFeaturesBrowsingDelegate sideSlopeBrowser = new LinearFeaturesBrowsingDelegate(browseSideSlopeFeatures);
            for (int n = 0; n < sideSlopeDtms.Length; ++n)
            {
                sideSlopeDtms[n].BrowseLinearFeatures(sideSlopeCriteria, DTMFeatureType.SlopeToe, sideSlopeBrowser, dtmSlopeToeFeatures);
            }

            foreach (BGEO.DPoint3d[] slopeToe in dtmSlopeToeFeatures)
            {
                Console.WriteLine("Number Of Slope Toe Points = {0}", slopeToe.Length);
                for (int l = 0; l < slopeToe.Length; ++l)
                {
                    Console.WriteLine("Slope Toe Point[{0}] ** X = {1} Y = {2} Z = {3}",l,slopeToe[l].X,slopeToe[l].Y,slopeToe[l].Z);
                }
            }


            // Merge Ground DTM and Side Slopes DTM To Create Design DTM

            for (int n = 0; n < sideSlopeDtms.Length; ++n)
            {
                // Clone DTMS

                DTM designDTM = groundDTM.Clone();
                DTM sideSlopeDTM = sideSlopeDtms[n].Clone();

                // Triangulate 

                sideSlopeDTM.Triangulate();

                //                Console.WriteLine("Saving DTM");
                //                designDTM.Save(@"D:\CertData\CivilPlatform\SideSlopes00\design.bcdtm");
                //                sideSlopeDTM.Save(@"D:\CertData\CivilPlatform\SideSlopes00\sideSlope.bcdtm");


                // Merge Cloned DTM's

                designDTM.Merge(sideSlopeDTM);

                // Dispose Cloned DTMs

                sideSlopeDTM.Dispose();
                designDTM.Dispose();
            }

            //  Dispose Ground DTM

            groundDTM.Dispose();
        }

        /// <summary>
        /// DTMSideSlopeTest
        /// </summary>
        /// <returns></returns>
        [Test]
        [Category("DTMSideSlopeTests")]
        public void SideSlopeTestToSurfaceWithElevationLimit()
        {
            /// <summary>
            /// DTMSideSlopeTest1
            /// </summary>

            // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

            double cutSlope = 0.333;
            double fillSlope = 0.3333;

            // Read DTM From File

            DTM groundDTM = DTM.CreateFromFile(DataPath + "\\Ground2TestSideSlopes.tin");

            // Read Side Slope Alignment from File

            //            BGEO.DPoint3d[] sideSlopeAlignment = DTM.ImportDPoint3DArrayFromXYZFile(DataPath + "\\sideSlopeAlignment.xyz");

            BGEO.DPoint3d[] sideSlopeAlignment = new BGEO.DPoint3d[23];

            sideSlopeAlignment[0].X = 37698.516; sideSlopeAlignment[0].Y = 11203.901; sideSlopeAlignment[0].Z = 41.062;
            sideSlopeAlignment[1].X = 37698.516; sideSlopeAlignment[1].Y = 11213.901; sideSlopeAlignment[1].Z = 40.969;
            sideSlopeAlignment[2].X = 37698.516; sideSlopeAlignment[2].Y = 11223.901; sideSlopeAlignment[2].Z = 40.875;
            sideSlopeAlignment[3].X = 37698.516; sideSlopeAlignment[3].Y = 11233.901; sideSlopeAlignment[3].Z = 40.781;
            sideSlopeAlignment[4].X = 37698.516; sideSlopeAlignment[4].Y = 11243.901; sideSlopeAlignment[4].Z = 40.687;
            sideSlopeAlignment[5].X = 37698.516; sideSlopeAlignment[5].Y = 11253.901; sideSlopeAlignment[5].Z = 40.594;
            sideSlopeAlignment[6].X = 37698.516; sideSlopeAlignment[6].Y = 11263.901; sideSlopeAlignment[6].Z = 40.500;
            sideSlopeAlignment[7].X = 37698.516; sideSlopeAlignment[7].Y = 11273.901; sideSlopeAlignment[7].Z = 40.406;
            sideSlopeAlignment[8].X = 37698.516; sideSlopeAlignment[8].Y = 11283.901; sideSlopeAlignment[8].Z = 40.312;
            sideSlopeAlignment[9].X = 37698.516; sideSlopeAlignment[9].Y = 11293.901; sideSlopeAlignment[9].Z = 40.219;
            sideSlopeAlignment[10].X = 37698.516; sideSlopeAlignment[10].Y = 11303.901; sideSlopeAlignment[10].Z = 40.125;
            sideSlopeAlignment[11].X = 37698.516; sideSlopeAlignment[11].Y = 11313.901; sideSlopeAlignment[11].Z = 40.031;
            sideSlopeAlignment[12].X = 37698.516; sideSlopeAlignment[12].Y = 11323.901; sideSlopeAlignment[12].Z = 41.062;
            sideSlopeAlignment[13].X = 37698.516; sideSlopeAlignment[13].Y = 11333.901; sideSlopeAlignment[13].Z = 39.937;
            sideSlopeAlignment[14].X = 37698.516; sideSlopeAlignment[14].Y = 11343.901; sideSlopeAlignment[14].Z = 39.844;
            sideSlopeAlignment[15].X = 37698.516; sideSlopeAlignment[15].Y = 11353.901; sideSlopeAlignment[15].Z = 39.750;
            sideSlopeAlignment[16].X = 37698.516; sideSlopeAlignment[16].Y = 11363.901; sideSlopeAlignment[16].Z = 39.656;
            sideSlopeAlignment[17].X = 37698.516; sideSlopeAlignment[17].Y = 11373.901; sideSlopeAlignment[17].Z = 39.592;
            sideSlopeAlignment[18].X = 37698.516; sideSlopeAlignment[18].Y = 11383.901; sideSlopeAlignment[18].Z = 39.469;
            sideSlopeAlignment[19].X = 37698.516; sideSlopeAlignment[19].Y = 11393.901; sideSlopeAlignment[19].Z = 39.375;
            sideSlopeAlignment[20].X = 37698.516; sideSlopeAlignment[20].Y = 11403.901; sideSlopeAlignment[20].Z = 39.281;
            sideSlopeAlignment[21].X = 37698.516; sideSlopeAlignment[21].Y = 11413.901; sideSlopeAlignment[21].Z = 39.187;
            sideSlopeAlignment[22].X = 37698.516; sideSlopeAlignment[22].Y = 11423.901; sideSlopeAlignment[22].Z = 39.000;


            // Create Slope Table

            DTMSlopeTable slopeTable = new DTMSlopeTable();
            slopeTable.AddSlopeRange(0, 10, 1.0);
            slopeTable.AddSlopeRange(10, 20, 5.0);
            slopeTable.AddSlopeRange(20, 30, 10.0);
            slopeTable.AddSlopeRange(30, 40, 1.0);

            // Create Side Slope Input

            double cornerStrokeTolerance = 4.0;       //  Maximum Linear Distance Between Consecutive Corner Side Slope Radial End Points
            double pointToPointTolerance = 0.001;     //  Minimum Linear Distance Between Consecutive Side Slope Radial Start Points
            Int64 radialBreaklineTag = -11111111111;  //  User Tag Assigned To Side Slope Break Lines  
            Int64 sideSlopeElementTag = -22222222222; //  User Tag Assigned To The Side Slope Alignment Break Line
            DTMSideSlopeInput dtmSideSlopeInput = new DTMSideSlopeInput(slopeTable, DTMSideSlopeDirection.Right, DTMSideSlopeCornerOption.Rounded, DTMSideSlopeStrokeCornerOption.Stroke, cornerStrokeTolerance, pointToPointTolerance, radialBreaklineTag, sideSlopeElementTag);

            // Scan Alignment And Populate Side Slope Input

            BGEO.DPoint3d startPoint = new BGEO.DPoint3d();
            for (int n = 0; n < sideSlopeAlignment.Length; ++n)
            {
                startPoint.X = sideSlopeAlignment[n].X;
                startPoint.Y = sideSlopeAlignment[n].Y;
                startPoint.Z = sideSlopeAlignment[n].Z;
// Added Following 24Oct2012 Needs To Be removed
                cutSlope = cutSlope / 2.0;
                fillSlope = fillSlope / 2.0;
//                dtmSideSlopeInput.AddRadialToSurfaceWithElevationLimit(startPoint, groundDTM, DTMSideSlopeCutFillOption.FillOnly,cutSlope,
//                    fillSlope, 42.0);
            }

            // Calculate Side Slope DTMs

            DTM[] sideSlopeDtms = dtmSideSlopeInput.CalculateSideSlopes();

            //  Browse Slope Toe Features And Store In An Array List

            ArrayList dtmSlopeToeFeatures = new ArrayList();
            LinearFeaturesBrowsingCriteria sideSlopeCriteria = new LinearFeaturesBrowsingCriteria();
            LinearFeaturesBrowsingDelegate sideSlopeBrowser = new LinearFeaturesBrowsingDelegate(browseSideSlopeFeatures);
            for (int n = 0; n < sideSlopeDtms.Length; ++n)
            {
                sideSlopeDtms[n].BrowseLinearFeatures(sideSlopeCriteria, DTMFeatureType.SlopeToe, sideSlopeBrowser, dtmSlopeToeFeatures);
            }

            foreach (BGEO.DPoint3d[] slopeToe in dtmSlopeToeFeatures)
            {
                Console.WriteLine("Number Of Slope Toe Points = {0}", slopeToe.Length);
                for (int l = 0; l < slopeToe.Length; ++l)
                {
                    Console.WriteLine("Slope Toe Point[{0}] ** X = {1} Y = {2} Z = {3}", l, slopeToe[l].X, slopeToe[l].Y, slopeToe[l].Z);
                }
            }


            // Merge Ground DTM and Side Slopes DTM To Create Design DTM

            for (int n = 0; n < sideSlopeDtms.Length; ++n)
            {
                // Clone DTMS

                DTM designDTM = groundDTM.Clone();
                DTM sideSlopeDTM = sideSlopeDtms[n].Clone();

                // Triangulate 

                sideSlopeDTM.Triangulate();

                //                Console.WriteLine("Saving DTM");
                //                designDTM.Save(@"D:\CertData\CivilPlatform\SideSlopes00\design.bcdtm");
                //                sideSlopeDTM.Save(@"D:\CertData\CivilPlatform\SideSlopes00\sideSlope.bcdtm");


                // Merge Cloned DTM's

                designDTM.Merge(sideSlopeDTM);

                // Dispose Cloned DTMs

                sideSlopeDTM.Dispose();
                designDTM.Dispose();
            }

            //  Dispose Ground DTM

            groundDTM.Dispose();
        }

        
        // Side Slope Browse Method

        private bool browseSideSlopeFeatures(DTMFeatureInfo info, object oArg)
        {

            //  Note - Slope Toe Feature In sideSlopeDTM Will have a Null FeatureID - This Can Be changed
            //  Accumulate The Side Slope Features In sideSlopeFeaturesDTM

            bool dbg = false;
            if( dbg ) Console.WriteLine("Dtm Feature Type = {0}", info.DtmFeatureType);
            if (oArg != null)
            {
                ArrayList slopeToeArrayList = oArg as ArrayList;
                BGEO.DPoint3d[] slopeToe = info.Points;
                slopeToeArrayList.Add(slopeToe);
            }
            return true;
        }
    }
}
