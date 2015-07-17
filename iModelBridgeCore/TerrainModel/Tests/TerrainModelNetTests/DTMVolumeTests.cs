#region Using
using System;
using System.IO;
using System.Text;
using System.Collections;
using SD = System.Data;
using SDO = System.Data.OleDb;
using BGEO = Bentley.GeometryNET;
using NUnit.Framework;
#endregion


namespace Bentley.TerrainModelNET.NUnit
{
 using Bentley.TerrainModelNET;

    /// <summary>
    /// 
    /// </summary>
    /// <category></category>
    [TestFixture]
    public class DTMVolumeTests : DTMUNitTest
    {

        static String DataDirectory = @"Bentley.Civil.Dtm.NUnit.dll\DTMVolumeTest\";
        static String DataPath = Helper.GetTestDataLocation () + DataDirectory;
        double elevation = 0.0;
        int numGridPoints = 1000000;
        int numRanges = 20;

        /// <summary>
        /// Function To Write The Volume Results To
        /// 
        /// </summary>
        /// <param name="calcType"></param>
        /// <param name="msgLevel"></param>
        /// <param name="volumeResult"></param>

        private void outputVolumeResultsToConsole(int calcType, int msgLevel, VolumeResult volumeResult)
        {

            //  Output Volume Calculation Type

            if (calcType == 1) Console.WriteLine("Prismoidal Surface To Elevation");
            if (calcType == 2) Console.WriteLine("Prismoidal Surface To Surface");
            if (calcType == 3) Console.WriteLine("Prismoidal Surface Balance");
            if (calcType == 4) Console.WriteLine("Grid Surface To Elevation");
            if (calcType == 5) Console.WriteLine("Grid Surface To Surface");

            //  Output Calculation Volumes

            if (calcType == 1 || calcType == 2)
            {
                Console.WriteLine("cut  = {0} fill = {1} balance = {2} area = {3}", volumeResult.CutVolume, volumeResult.FillVolume, volumeResult.BalanceVolume, volumeResult.Area);
            }
            if (calcType == 3)
            {
                Console.WriteLine("fromArea  = {0} toArea = {1} balance = {2}", volumeResult.FromArea, volumeResult.ToArea, volumeResult.BalanceVolume);
            }
            if (calcType == 4 || calcType == 5)
            {
                Console.WriteLine("cut  = {0} fill = {1} balance = {2} area = {3} numCellsUsed = {4} cellArea = {5}", volumeResult.CutVolume, volumeResult.FillVolume, volumeResult.BalanceVolume, volumeResult.Area, volumeResult.NumCellsUsed, volumeResult.CellArea);
            }

            //  Output Volume Polygons - Intersection Of Volume Boundary Polygon And Tin Hull(s)

            VolumePolygon[] volumePolygons = volumeResult.VolumePolygons;
            Console.WriteLine("Number Of Volume Polygons = {0}", volumePolygons.Length);

            if (msgLevel > 1)
            {
                for (int i = 0; i < volumePolygons.Length; ++i)
                {
                    Console.WriteLine("VolumePolygon  = {0} NumPoints = {1}", i, volumePolygons[i].NumPoints);
                    for (int j = 0; j < volumePolygons[i].NumPoints; ++j)
                    {
                        Console.WriteLine("Polygon Point = {0} ** X = {1} Y = {2} Z = {3}", j, volumePolygons[i].Points[j].X, volumePolygons[i].Points[j].Y, volumePolygons[i].Points[j].Z);
                    }
                }
            }

        }

        /// <summary>
        /// Function To Write The Volume Range Results.
        /// The Cut And Fill For Each Elevation Range Is Written
        /// </summary>
        /// <param name="volumeResult">An Instance Of The Volume Result Class</param>
        /// <param name="volumeCriteria">An Instance Of The Volume Criteria Class</param>
 
        private void outputVolumeRangeResultsToConsole( VolumeResult volumeResult , VolumeCriteria volumeCriteria)
        {

            double totalCut = 0.0, totalFill = 0.0;

            Console.WriteLine("Number Of Volume Ranges = {0}", volumeCriteria.RangeTable.Length );
            
            for (int i = 0; i < volumeCriteria.RangeTable.Length ; ++i)
            {
                Console.WriteLine("low = {0} high = {1} ** cut = {2} fill = {3}", volumeCriteria.RangeTable[i].Cut, volumeCriteria.RangeTable[i].Low, volumeCriteria.RangeTable[i].High,volumeCriteria.RangeTable[i].Fill);
                totalCut  = totalCut  + volumeCriteria.RangeTable[i].Cut;
                totalFill = totalFill + volumeCriteria.RangeTable[i].Fill;
            }

            Console.WriteLine("total Range Cut = {0} totalRangeFill = {1}",totalCut,totalFill);
            Console.WriteLine("cut Volume  = {0} fill Volume = {1}", volumeResult.CutVolume, volumeResult.FillVolume);

        }

        /// <summary>
        /// DTMVolumeTest
        /// </summary>
        /// <returns></returns>
        [Test]
        [Category("DTMVolumeTests")]
        public void VolumeTest1()
        {
         /// <summary>
         /// DTMVolumeTest1
         /// Volume Tests Without A Volume Boundary Polygon Or Set Of Volume Range Values
         /// The Volume Boundary Polygon Defaults To The Tin Hull For The To Elevation Volume Calculations
         /// And To The Intersection Of The Tin Hulls For The To Surface Volume Calculations
         /// </summary>

         // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}",this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

         // Get Test Data Directories

            DirectoryInfo testFolder = new DirectoryInfo (DataPath);
            if (! testFolder.Exists)
            {
                throw new DirectoryNotFoundException(" DTM Volumes Top Nunit Test Data Folder Not Found + DataPath");
            }

         // List All The Test Data Directories

            foreach (DirectoryInfo nextFolder in testFolder.GetDirectories())
            {
                Console.WriteLine("Test Data Folder = {0}", nextFolder.Name);

                if (string.Compare(nextFolder.Name, "CVS") != 0)
                {

                    // Read DTM's From File

                    DTM toDTM = DTM.CreateFromFile(DataPath + nextFolder.Name + "\\to.tin");
                    DTM fromDTM = DTM.CreateFromFile(DataPath + nextFolder.Name + "\\from.tin");

                    if (toDTM.CheckTriangulation() == false)
                    {
                        throw new Exception("toDTM Triangulation Invalid");
                    }
                    Console.WriteLine("toDTM Triangulation Valid");


                    if (fromDTM.CheckTriangulation() == false)
                    {
                        throw new Exception("fromDTM Triangulation Invalid");
                    }
                    Console.WriteLine("fromDTM Triangulation Valid");


                    //  Create Volume Criteria  

                    VolumeCriteria volumeCriteria = new VolumeCriteria();

                    //  Prismoidal Volumes
                    Console.WriteLine("Test1");
                    VolumeResult volumeResult0 = toDTM.CalculatePrismoidalVolumeToElevation(elevation, volumeCriteria);
                    outputVolumeResultsToConsole(1, 1, volumeResult0);

                    Console.WriteLine("Test2");
                    VolumeResult volumeResult1 = fromDTM.CalculatePrismoidalVolumeToElevation(elevation, volumeCriteria);
                    outputVolumeResultsToConsole(1, 1, volumeResult1);

                    Console.WriteLine("Test3");
                    VolumeResult volumeResult2 = fromDTM.CalculatePrismoidalVolumeToSurface(toDTM, volumeCriteria);
                    outputVolumeResultsToConsole(2, 1, volumeResult2);

                    //  Prismoidal Surface Balance - Validation Only Purpose 

                    Console.WriteLine("Test4");
                    VolumeResult volumeResult3 = fromDTM.CalculatePrismoidalVolumeBalanceToSurface(toDTM, volumeCriteria);
                    outputVolumeResultsToConsole(3, 1, volumeResult3);

                    //  Grid Volumes

                    Console.WriteLine("Test5");
                    VolumeResult volumeResult4 = toDTM.CalculateGridVolumeToElevation(elevation, numGridPoints, volumeCriteria);
                    outputVolumeResultsToConsole(4, 1, volumeResult4);

                    Console.WriteLine("Test6");
                    VolumeResult volumeResult5 = fromDTM.CalculateGridVolumeToElevation(elevation, numGridPoints, volumeCriteria);
                    outputVolumeResultsToConsole(4, 1, volumeResult5);

                    Console.WriteLine("Test7");
                    VolumeResult volumeResult6 = fromDTM.CalculateGridVolumeToSurface(toDTM, numGridPoints, volumeCriteria);
                    outputVolumeResultsToConsole(5, 1, volumeResult6);

                    // Dispose DTMs

                    toDTM.Dispose();
                    fromDTM.Dispose();
                }
            }

        }
        /// <summary>
        /// 
        /// </summary>
        [Test]
        [Category("DTMVolumeTests")]
        public void VolumeTest2()
        {
            /// <summary>
            /// DTMVolumeTest2
            /// Volume Tests With A Volume Boundary Polygon
            /// </summary>

            // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

            // Get Test Data Directories

            DirectoryInfo testFolder = new DirectoryInfo(DataPath);
            if (!testFolder.Exists)
            {
                throw new DirectoryNotFoundException(" DTM Volumes Top Nunit Test Data Folder Not Found + DataPath");
            }

            // Scan All The Test Data Directories

            foreach (DirectoryInfo nextFolder in testFolder.GetDirectories())
            {
                Console.WriteLine("Test Data Folder = {0}", nextFolder.Name);

                if (string.Compare(nextFolder.Name, "CVS") != 0)
                {

                    // Check Volume Polygon File Exists

                    bool volumePolygonExists = System.IO.File.Exists(DataPath + nextFolder.Name + "\\volumePolygon.xyz");

                    if (volumePolygonExists == true)
                    {

                        // Read DTM's From File

                        DTM toDTM = DTM.CreateFromFile(DataPath + nextFolder.Name + "\\to.tin");
                        DTM fromDTM = DTM.CreateFromFile(DataPath + nextFolder.Name + "\\from.tin");

                        //  Get Volume Polygon From File

                        BGEO.DPoint3d[] volumePolygon = DTM.ImportDPoint3DArrayFromXYZFile(DataPath + nextFolder.Name + "\\volumePolygon.xyz");

                        //  Create Volume Criteria  

                        VolumeCriteria volumeCriteria = new VolumeCriteria();

                        //  Store Volume Polygon In Volume Criteria

                        volumeCriteria.VolumePolygon = volumePolygon;

                        //  Prismoidal Volumes

                        VolumeResult volumeResult0 = toDTM.CalculatePrismoidalVolumeToElevation(elevation, volumeCriteria);
                        outputVolumeResultsToConsole(1, 1, volumeResult0);

                        VolumeResult volumeResult1 = fromDTM.CalculatePrismoidalVolumeToElevation(elevation, volumeCriteria);
                        outputVolumeResultsToConsole(1, 1, volumeResult1);

                        VolumeResult volumeResult2 = fromDTM.CalculatePrismoidalVolumeToSurface(toDTM, volumeCriteria);
                        outputVolumeResultsToConsole(2, 1, volumeResult2);

                        //  Prismoidal Surface Balance - Validation Only Purpose 

                        VolumeResult volumeResult3 = fromDTM.CalculatePrismoidalVolumeBalanceToSurface(toDTM, volumeCriteria);
                        outputVolumeResultsToConsole(3, 1, volumeResult3);

                        //  Grid Volumes

                        VolumeResult volumeResult4 = toDTM.CalculateGridVolumeToElevation(elevation, numGridPoints, volumeCriteria);
                        outputVolumeResultsToConsole(4, 1, volumeResult4);

                        VolumeResult volumeResult5 = fromDTM.CalculateGridVolumeToElevation(elevation, numGridPoints, volumeCriteria);
                        outputVolumeResultsToConsole(4, 1, volumeResult5);

                        VolumeResult volumeResult6 = fromDTM.CalculateGridVolumeToSurface(toDTM, numGridPoints, volumeCriteria);
                        outputVolumeResultsToConsole(5, 1, volumeResult6);

                        // Dispose DTMs

                        toDTM.Dispose();
                        fromDTM.Dispose();
                    }

                }

            }
        }


        
        [Test]
        [Category("DTMVolumeTests")]
        public void VolumeTest3()
        {
            /// <summary>
            /// DTMVolumeTest3
            /// Volume Tests With A Set Of Volume Range Values
            /// The Volume Boundary Polygon Defaults To The Tin Hull For The To Elevation Volume Calculations
            /// And To The Intersection Of The Tin Hulls For The To Surface Volume Calculations
            /// </summary>

            // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

 
         // Get Test Data Directories

            DirectoryInfo testFolder = new DirectoryInfo (DataPath);
            if (! testFolder.Exists)
            {
                throw new DirectoryNotFoundException(" DTM Volumes Top Nunit Test Data Folder Not Found + DataPath");
            }

         // List All The Test Data Directories

            foreach (DirectoryInfo nextFolder in testFolder.GetDirectories())
            {
                Console.WriteLine("Test Data Folder = {0}", nextFolder.Name);

                if (string.Compare(nextFolder.Name, "CVS") != 0)
                {

                    // Read DTM's From File

                    DTM toDTM = DTM.CreateFromFile(DataPath + nextFolder.Name + "\\to.tin");
                    DTM fromDTM = DTM.CreateFromFile(DataPath + nextFolder.Name + "\\from.tin");

                    // Get Dtm Ranges

                    double zMin, zMax, zRangeInc;
                    BGEO.DRange3d toDtmRange = toDTM.Range3d;
                    BGEO.DRange3d fromDtmRange = fromDTM.Range3d;
                    zMin = toDtmRange.Low.Z;
                    zMax = toDtmRange.High.Z;
                    if (fromDtmRange.Low.Z < zMin) zMin = fromDtmRange.Low.Z;
                    if (fromDtmRange.High.Z > zMax) zMax = fromDtmRange.High.Z;
                    Console.WriteLine("zMin = {0} zMax = {1}", zMin, zMax);
                    elevation = (zMin + zMax) / 2.0;
                    zRangeInc = (zMax - zMin) / numRanges;
                    //            Console.WriteLine("elevation = {0} zRangeInc = {1}", elevation,zRangeInc);

                    // Create And Populate Volume Range Table

                    VolumeRange[] volumeRange = new VolumeRange[numRanges];
                    for (int i = 0; i < numRanges; ++i)
                    {
                        zMax = zMin + zRangeInc;
                        volumeRange[i] = new VolumeRange(zMin, zMax);
                        zMin = zMax;
                        //                Console.WriteLine("low = {0} high = {1}", volumeRange[i].Low,volumeRange[i].High );
                    }

                    //  Create Volume Criteria  

                    VolumeCriteria volumeCriteria = new VolumeCriteria();
                    volumeCriteria.RangeTable = volumeRange;

                    //  Prismoidal Volumes

                    VolumeResult volumeResult0 = toDTM.CalculatePrismoidalVolumeToElevation(elevation, volumeCriteria);
                    outputVolumeResultsToConsole(1, 1, volumeResult0);
                    outputVolumeRangeResultsToConsole(volumeResult0, volumeCriteria);

                    VolumeResult volumeResult1 = fromDTM.CalculatePrismoidalVolumeToElevation(elevation, volumeCriteria);
                    outputVolumeResultsToConsole(1, 1, volumeResult1);
                    outputVolumeRangeResultsToConsole(volumeResult1, volumeCriteria);

                    VolumeResult volumeResult2 = fromDTM.CalculatePrismoidalVolumeToSurface(toDTM, volumeCriteria);
                    outputVolumeResultsToConsole(2, 1, volumeResult2);
                    outputVolumeRangeResultsToConsole(volumeResult2, volumeCriteria);

                    //  Prismoidal Surface Balance - Validation Only Purpose 

                    VolumeResult volumeResult3 = fromDTM.CalculatePrismoidalVolumeBalanceToSurface(toDTM, volumeCriteria);
                    outputVolumeResultsToConsole(3, 1, volumeResult3);
                    outputVolumeRangeResultsToConsole(volumeResult3, volumeCriteria);

                    //  Grid Volumes

                    VolumeResult volumeResult4 = toDTM.CalculateGridVolumeToElevation(elevation, numGridPoints, volumeCriteria);
                    outputVolumeResultsToConsole(4, 1, volumeResult4);
                    outputVolumeRangeResultsToConsole(volumeResult4, volumeCriteria);

                    VolumeResult volumeResult5 = fromDTM.CalculateGridVolumeToElevation(elevation, numGridPoints, volumeCriteria);
                    outputVolumeResultsToConsole(4, 1, volumeResult5);
                    outputVolumeRangeResultsToConsole(volumeResult5, volumeCriteria);

                    VolumeResult volumeResult6 = fromDTM.CalculateGridVolumeToSurface(toDTM, numGridPoints, volumeCriteria);
                    outputVolumeResultsToConsole(5, 1, volumeResult6);
                    outputVolumeRangeResultsToConsole(volumeResult6, volumeCriteria);

                    // Dispose DTMs

                    toDTM.Dispose();
                    fromDTM.Dispose();
                }
            }
        }
    }
}
