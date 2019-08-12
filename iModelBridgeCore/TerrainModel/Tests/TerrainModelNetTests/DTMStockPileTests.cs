
using System;
using System.IO;
using System.Text;
using System.Collections;
using SD = System.Data;
using SDO = System.Data.OleDb;
using NUnit.Framework;
using BGEO = Bentley.GeometryNET;



namespace Bentley.TerrainModelNET.NUnit
{
 using Bentley.TerrainModelNET;
 
    /// <summary>
    /// 
    /// </summary>
    /// <category></category>
    [TestFixture]
    public class DTMStockPileTests : DTMUNitTest 
    {

        static String DataDirectory = @"Bentley.Civil.Dtm.NUnit.dll\DTMStockPileTest\";
        static String DataPath = Helper.GetTestDataLocation () + DataDirectory;
 
        /// <summary>
        /// DTMStockPile
        /// </summary>
        /// <returns></returns>
        /// 

        private double dtmDistance(double X1, double Y1, double X2, double Y2)
        {
            double dx = X2 - X1;
            double dy = Y2 - Y1;
            return (Math.Sqrt(dx * dx + dy * dy));
        }


        private double dtmAngle(double X1, double Y1, double X2, double Y2)
        {
            double dx = X2 - X1;
            double dy = Y2 - Y1;
            return (Math.Atan2(dy,dx));
        } 


        [Test]
        [Category("DTMStockPile")]
        public void StockPileTests()
        {
         /// <summary>
         /// Create Point Stock Piles
         /// </summary>

         // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}",this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

         // Get Test Data Directories

            Console.WriteLine("DataPath = {0}",DataPath);

            DirectoryInfo testFolder = new DirectoryInfo(DataPath);
            if (! testFolder.Exists)
            {
                throw new DirectoryNotFoundException("DTM Stock Pile Nunit Test Data Header Folder Not Found + DataPath");
            }

         //  Read Ground DTM

            DTM groundDTM = DTM.CreateFromFile(DataPath + "\\groundSurface.tin");

         // Validate DTM

            if (groundDTM.CheckTriangulation() == false)
            {
                throw new Exception("groundDTM Invalid");
            }
            Console.WriteLine("groundDTM Valid");

  
         //  Calculate Value Of 2 PYE

            double DTM_2PYE = Math.Atan2(0.0, -1.0) * 2.0;

         //  Generate Coordinates For Creating Stock Piles

            int numIncs = 0, numHeadCoordinates = 0;
            double rsx, rsy, rex, rey, tx, ty, distance, dd, ddInc, ddLength, headOffsetDistance;
            double angle, offsetAngle;
            bool createAlignmentStockpile = true;
            bool createPointStockpile = true;

            BGEO.DPoint3d[] headCoordinates = new Bentley.GeometryNET.DPoint3d[100];

            headOffsetDistance = 30.0;
            rsx = 2138540.0; rsy = 221240.0;
            rex = 2138850.0; rey = 221240.0;
            numHeadCoordinates = 0;
            distance = dtmDistance(rsx, rsy, rex, rey);
            angle = dtmAngle(rsx,rsy,rex,rey);
            Console.WriteLine("Conveyor Rail ** Angle = {0} ** Distance = {1}", angle, distance);

            // Calculate Conveyor Head Cordinates Up Rail Track

            offsetAngle = angle + DTM_2PYE / 4.0;
            while (offsetAngle > DTM_2PYE) offsetAngle = -DTM_2PYE;
            Console.WriteLine("Conveyor Head Offset Angle = {0} ** Head Offset Distance = {1},", offsetAngle, headOffsetDistance);
            dd = 0;
            ddLength = 15.0;
            ddInc = distance / ddLength;
            numIncs = (int)(distance / ddInc) + 1;
            Console.WriteLine("Up Track Coordinates");
            for (int n = 0; n < numIncs; ++n)
            {
                tx = rsx + dd * Math.Cos(angle);
                ty = rsy + dd * Math.Sin(angle);
                headCoordinates[numHeadCoordinates].X = tx + headOffsetDistance * Math.Cos(offsetAngle);
                headCoordinates[numHeadCoordinates].Y = ty + headOffsetDistance * Math.Sin(offsetAngle);
                headCoordinates[numHeadCoordinates].Z = 680.0;
                Console.WriteLine("Coordinate[{0}] ** Rail Coordinate = {1} -- {2} ** Head Coordinate = {3} -- {4}", numHeadCoordinates, tx, ty, headCoordinates[numHeadCoordinates].X, headCoordinates[numHeadCoordinates].Y);
                ++numHeadCoordinates;
                dd = dd + ddInc;
            }

            // Calculate Conveyor Head Coordinates Around End Of Track

            Console.WriteLine("Track End Corner Coordinates");
            for (int n = 0; n < 3; ++n )
            {
                offsetAngle = offsetAngle - DTM_2PYE / 8.0;
                headCoordinates[numHeadCoordinates].X = rex + headOffsetDistance * Math.Cos(offsetAngle);
                headCoordinates[numHeadCoordinates].Y = rey + headOffsetDistance * Math.Sin(offsetAngle);
                headCoordinates[numHeadCoordinates].Z = 680.0;
                Console.WriteLine("Coordinate[{0}] ** Rail Coordinate = {1} -- {2} ** Head Coordinate = {3} -- {4}", numHeadCoordinates, rex, rey, headCoordinates[numHeadCoordinates].X, headCoordinates[numHeadCoordinates].Y);
                ++numHeadCoordinates;
            }

            // Calculate Conveyor Head Cordinates Down Rail Track

            angle = dtmAngle(rex, rey, rsx, rsy);
            offsetAngle = angle + DTM_2PYE / 4.0;
            while (offsetAngle > DTM_2PYE) offsetAngle = -DTM_2PYE;
            dd = 0;
            ddLength = 15.0;
            ddInc = distance / ddLength;
            numIncs = (int)(distance / ddInc) + 1;
            Console.WriteLine("Down Track Coordinates");
            for (int n = 0; n < numIncs; ++n)
            {
                tx = rex + dd * Math.Cos(angle);
                ty = rey + dd * Math.Sin(angle);
                headCoordinates[numHeadCoordinates].X = tx + headOffsetDistance * Math.Cos(offsetAngle);
                headCoordinates[numHeadCoordinates].Y = ty + headOffsetDistance * Math.Sin(offsetAngle);
                headCoordinates[numHeadCoordinates].Z = 680.0;
                Console.WriteLine("Coordinate[{0}] ** Rail Coordinate = {1} -- {2} ** Head Coordinate = {3} -- {4}", numHeadCoordinates, tx, ty, headCoordinates[numHeadCoordinates].X, headCoordinates[numHeadCoordinates].Y);
                ++numHeadCoordinates;
                dd = dd + ddInc;
            }


          // Create Alignment 

            if ( createAlignmentStockpile )
            {
                Console.WriteLine("Creating A Single Alignment Stockpile");
                BGEO.DPoint3d[] coordinates = new Bentley.GeometryNET.DPoint3d[numHeadCoordinates];
                for (int n = 0; n < numHeadCoordinates; ++n)
                {
                    coordinates[n] = headCoordinates[n];
                }
                StockPileCriteria stockPileCriteria = new StockPileCriteria();
                stockPileCriteria.MergeOption = false ;
                stockPileCriteria.StockPileType = StockPileFeature.AlignmentStockpile ;
                stockPileCriteria.StockPileSlope = 2.0 / 1.0;  // run / rise
                stockPileCriteria.StockPilePoints = coordinates;

                // Create Alignment Stock Pile

                StockPileResult stockPileResult = groundDTM.CreateStockPile(stockPileCriteria);
                Console.WriteLine("Alignment Stock Pile[{0}] Volume = {0}",stockPileResult.StockPileVolume);

                // Validate Stock Pile DTM - Development Only

                if (stockPileResult.StockPileDTM.CheckTriangulation() == false)
                {
                    throw new Exception("StockPileDTM Invalid");
                }
                Console.WriteLine("StockPileDTM Valid");

            }

         //  Create Point Stock Piles

            if (createPointStockpile)
            {
                Console.WriteLine("Creating Multiple Point Stockpiles");
                DTM groundSurfaceDTM = groundDTM.Clone();
                for (int n = 0; n < numHeadCoordinates; ++n)
                {
                    BGEO.DPoint3d[] coordinates = new Bentley.GeometryNET.DPoint3d[1];
                    coordinates[0] = headCoordinates[n];
                    StockPileCriteria stockPileCriteria = new StockPileCriteria();
                    stockPileCriteria.MergeOption = true;
                    stockPileCriteria.StockPileType = StockPileFeature.PointStockPile;
                    stockPileCriteria.StockPileSlope = 2.0 / 1.0;  // run / rise
                    stockPileCriteria.StockPilePoints = coordinates;
                    StockPileResult stockPileResult = groundSurfaceDTM.CreateStockPile(stockPileCriteria);
                    Console.WriteLine("Point Stock Pile[{0}] Volume = {1}", n, stockPileResult.StockPileVolume);

                    // Validate Stock Pile DTM - Development Only

                    if (stockPileResult.StockPileDTM.CheckTriangulation() == false)
                    {
                        throw new Exception("StockPileDTM Invalid");
                    }
                    Console.WriteLine("StockPileDTM Valid");

                    //  Dispose And Create New Ground Surface DTM From Merged DTM

                    if (stockPileCriteria.MergeOption)
                    {
                        // Validate Merged DTM - Development Only

                        if (stockPileResult.MergedDTM.CheckTriangulation() == false)
                        {
                            throw new Exception("MergedDTM Invalid");
                        }
                        Console.WriteLine("MergedDTM Valid");
                        groundSurfaceDTM.Dispose();
                        groundSurfaceDTM = stockPileResult.MergedDTM.Clone();
                    }
                }
            }
        }
    }
}
