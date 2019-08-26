using System;
using SD = System.Data;
using SDO = System.Data.OleDb;
using BGEO = Bentley.GeometryNET;
using NUnit.Framework;

namespace Bentley.TerrainModelNET.NUnit
    {
    /// <summary>
    /// 
    /// </summary>
    [TestFixture]
    public class DTMFilterSpots : DTMUNitTest
        {
        //List constant string variables
        static String SpotFile = "10m.xyz";
        static String DataDirectory = @"Bentley.Civil.Dtm.NUnit.dll\DTMTriangulationTest\DTMTriangulationXyzTest\";
        static String DataPath = Helper.GetTestDataLocation () + DataDirectory + SpotFile;

        static String GroupSpotFile = "groupSpot.tin";
        static String GroupSpotDataDirectory = @"Bentley.Civil.Dtm.Light.NUnit.dll\";
        static String GroupSpotDataPath = Helper.GetTestDataLocation () + GroupSpotDataDirectory + GroupSpotFile;


        /// <summary>
        /// Tin Filter Spots
        /// </summary>
        [Test]
        [Category("Thinning")]
        public void TinFilterPoints()
            {
            IgnoreFirebug ();

            Console.WriteLine("Importing XYZ File {0}", SpotFile);
            DTM nDtmInput = DTM.CreateFromXyzFile(DataPath);

            Console.WriteLine("Tin Filtering Random Spots");
            FilterResult res = nDtmInput.TinFilterPoints(new TinFilterCriteria());
            Console.WriteLine("Tin Filter Results ** spotsBefore = {0} spotsAfter = {1} reduction = {2}%", res.NumberOfPointsBeforeFiltering, res.NumberOfPointsAfterFiltering, res.FilterReduction);

            nDtmInput.Dispose();
            nDtmInput = null;
            }

        /// <summary>
        /// Tin Filter Group Spots
        /// </summary>
        [Test]
        [Category("Thinning")]
        public void TinFilterGroupSpot()
            {
            IgnoreFirebug ();

            Console.WriteLine("Importing Tin File {0}", GroupSpotFile);
            DTM nDtmInput = DTM.CreateFromFile(GroupSpotDataPath);

            Console.WriteLine("Tin Filtering Group Spots");
            FilterResult res = nDtmInput.TinFilterSinglePointPointFeatures(new TinFilterCriteria());
            Console.WriteLine("Tin Filter Results ** spotsBefore = {0} spotsAfter = {1} reduction = {2}%", res.NumberOfPointsBeforeFiltering, res.NumberOfPointsAfterFiltering, res.FilterReduction);

            nDtmInput.Dispose();
            nDtmInput = null;
            }

        /// <summary>
        /// Tile Filter Spots
        /// </summary>
        [Test]
        [Category("Thinning")]
        public void TileFilterRandomSpot()
            {
            IgnoreFirebug ();

            Console.WriteLine("Importing XYZ File {0}", SpotFile);
            DTM nDtmInput = DTM.CreateFromXyzFile(DataPath);

            Console.WriteLine("Tile Filtering Random Spots");
            FilterResult res = nDtmInput.TileFilterPoints(new TileFilterCriteria());
            Console.WriteLine("Tile Filter Results ** spotsBefore = {0} spotsAfter = {1} reduction = {2}%", res.NumberOfPointsBeforeFiltering, res.NumberOfPointsAfterFiltering, res.FilterReduction);

            nDtmInput.Dispose();
            nDtmInput = null;
            }

        /// <summary>
        /// Tile Filter Group Spots
        /// </summary>
        [Test]
        [Category("Thinning")]
        public void TileFilterGroupSpots()
            {
            IgnoreFirebug ();

            Console.WriteLine("Importing Tin File {0}", GroupSpotFile);
            DTM nDtmInput = DTM.CreateFromFile(GroupSpotDataPath);

            Console.WriteLine("Tile Filtering Group Spots");
            FilterResult res = nDtmInput.TileFilterSinglePointPointFeatures(new TileFilterCriteria());
            Console.WriteLine("Tile Filter Results ** spotsBefore = {0} spotsAfter = {1} reduction = {2}%", res.NumberOfPointsBeforeFiltering, res.NumberOfPointsAfterFiltering, res.FilterReduction);

            nDtmInput.Dispose();
            nDtmInput = null;
            }
        }
    }
