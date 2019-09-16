using System;
using SD = System.Data;
using SDO = System.Data.OleDb;
using BGEO = Bentley.GeometryNET;
using Bentley.TerrainModelNET.Formats;
using NUnit.Framework;

namespace Bentley.TerrainModelNET.NUnit
    {
    /// <summary>
    ///  DTM Lidar Tests
    /// </summary>
    [TestFixture]
    public class DTMLidar : DTMUNitTest
        {
        /// <summary>
        /// Append Problem Driver
        /// </summary>

        static long totalBrowsePoints = 0;
        //     static DTMFeatureId lastGroupSpotId = DTM_NULL_FEATURE_ID;
        //     static long numGroupSpotIds = 0;

        [Test]
        [Category("Lidar Methods")]
        public void ImportLasFeatures()
            {
            IgnoreFirebug ();

            //List constant string variables
            String dbPath = @"Bentley.Civil.Dtm.NUnit.dll\Lidar\";
            String DataPath = Helper.GetTestDataLocation () + dbPath;

            // Import The All The Las Feature Points As Random Spots  

            //Create the Las DTM
            DTM lasDTM = null;

            // Import Las Features
            Console.WriteLine("Importing Las Features");
            LidarImporter importer = LidarImporter.Create(DataPath + "N1430320.las");
            System.Collections.Generic.IEnumerable<ImportedTerrain> terrains = importer.ImportTerrains ();
            foreach (ImportedTerrain terrain in terrains)
                lasDTM = terrain.Terrain;
            Console.WriteLine("Importing Las Features Completed");
            ulong totalLidarPoints = 0;
            foreach (LidarImporter.ClassificationInfo info in importer.GetClassificationInfo())
                {
                ulong numFeatures = info.Count;
                Console.WriteLine("Las Feature = {0} Number Of Points = {1}", info.Classification, numFeatures);
                totalLidarPoints = totalLidarPoints + numFeatures;
                }
            Console.WriteLine("Total Lidar Points Imported = {0}", totalLidarPoints);

            //Triangulate
            Console.WriteLine("Triangulating Lidar Points");
            lasDTM.Triangulate();

            //Check Triangulation - Unit Test Checking Only
            Console.WriteLine("Checking Lidar Triangulation");
            if (lasDTM.CheckTriangulation() == false)
                {
                throw new Exception("Lidar Triangulation Invalid");
                }
            Console.WriteLine("Lidar Triangulation Valid");


            // Browse Lidar Random Spots 

            LinearFeaturesBrowsingCriteria spotCriteria = new LinearFeaturesBrowsingCriteria();
            LinearFeaturesBrowsingDelegate SpotBrowser = new LinearFeaturesBrowsingDelegate(spotBrowser);

            Console.WriteLine("Browsing Lidar Random Spots");
            spotCriteria.MaxSpots = 2000;
            lasDTM.BrowseLinearFeatures(spotCriteria, DTMFeatureType.Point, SpotBrowser, null);
            Console.WriteLine("Total Random Spots Browsed = {0}", totalBrowsePoints);
            totalBrowsePoints = 0;

            // Clean up 
            lasDTM.Dispose();

            // Import The Las Feature Points As Group Spots  

            DTM lidarDTM = new DTM();

            LidarImporter queryLasFile = LidarImporter.Create(DataPath + "N1430320.las");

            foreach (LidarImporter.ClassificationInfo info in importer.GetClassificationInfo ())
                {
                ulong numFeatures = info.Count;
                if (numFeatures >= 0)
                    {
                    LidarImporter.Classification[] filter = new LidarImporter.Classification[] { info.Classification };
                    DTM terrain = importer.ImportTerrain (filter);
                    if (terrain != null)
                        lidarDTM.Append (terrain);
                    }
                }

            //Triangulate
            Console.WriteLine("Triangulating Lidar Points");
            lidarDTM.Triangulate();

            //Check Triangulation - Unit Test Checking Only
            Console.WriteLine("Checking Lidar Triangulation");
            if (lidarDTM.CheckTriangulation() == false)
                {
                throw new Exception("Lidar Triangulation Invalid");
                }
            Console.WriteLine("Lidar Triangulation Valid");

            // Browse Lidar Group Spots 
            Console.WriteLine("Browsing Lidar Group Spots");
            //            numGroupSpotIds = 0 ;
            spotCriteria.MaxSpots = 2000;
            lidarDTM.BrowseLinearFeatures(spotCriteria, DTMFeatureType.PointFeature, SpotBrowser, null);
            //            Console.WriteLine("Total Group Spots Browsed        = {0}",numGroupSpotIds);
            Console.WriteLine("Total Group Spot Points Browsed = {0}", totalBrowsePoints);

            // Clean up 
            lidarDTM.Dispose();

            }

        //     Spot Browser

        private bool spotBrowser(DTMFeatureInfo featureInfo, object oArg)
            {
            //         if( id != lastGroupSpotId )
            //           {
            //            ++numGroupSpotIds ;
            //            lastGroupSpotId = id ;
            //           }
            totalBrowsePoints = totalBrowsePoints + featureInfo.Points.Length;
            return true;
            }

        }
    }