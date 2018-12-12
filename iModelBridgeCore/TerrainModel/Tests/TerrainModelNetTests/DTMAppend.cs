using System;
using SD = System.Data;
using SDO = System.Data.OleDb;

using NUnit.Framework;

namespace Bentley.TerrainModelNET.NUnit
   {
    using Bentley.TerrainModelNET;

       /// <summary>
       ///  DTM Append Tests
       /// </summary>
       [TestFixture]
        public class DTMAppend : DTMUNitTest
       {
        /// <summary>
        /// Append Problem Driver
        /// </summary>

       [Test]
       [Category("DTM Append Tests")]
        public void AppendTest()
        {
            // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

            //List constant string variables
            String dbPath = @"Bentley.Civil.Dtm.NUnit.dll\DTMMergeTest\DTMMergeTest001\";
            String DataPath = Helper.GetTestDataLocation () + dbPath;

            //Create the DTMs
            Console.WriteLine("Reading DTM1");
            DTM Dtm1 = DTM.CreateFromFile(DataPath + "merge1.tin");
            Console.WriteLine("Reading DTM2");
            DTM Dtm2 = DTM.CreateFromFile(DataPath + "merge2.tin");
            Console.WriteLine("Cloning DTM1 To DTM3");
            DTM Dtm3 = Dtm1.Clone();
            Console.WriteLine("Cloning DTM2 To DTM4");
            DTM Dtm4 = Dtm2.Clone();

            //Append The DTM's
            Console.WriteLine("Appending Dtm2 To Dtm3");
            Dtm3.Append(Dtm2);
            DateTime EndTime = DateTime.Now;

            //Triangulate
            Console.WriteLine("Triangulating Dtm3");
            Dtm3.Triangulate();

            //Check Append Triangulation - Unit Test Checking Only
            Console.WriteLine("Checking Dtm3 Triangulation");
            if (Dtm3.CheckTriangulation() == false)
            {
                throw new Exception("Dtm3 Triangulation Invalid");
            }
            Console.WriteLine("Dtm3 Triangulation Valid");

            //Append The DTM's
            Console.WriteLine("Appending Dtm1 To Dtm4");
            Dtm4.Append(Dtm1);

            //Triangulate
            Console.WriteLine("Triangulating Dtm4");
            Dtm4.Triangulate();

            //Check Append Triangulation - Unit Test Checking Only
            Console.WriteLine("Checking Dtm4 Triangulation");
            if (Dtm4.CheckTriangulation() == false)
            {
                throw new Exception("Dtm4 Triangulation Invalid");
            }
            Console.WriteLine("Dtm4 Triangulation Valid");

            // Clean up by removing the TINs 
            Dtm1.Dispose();
            Dtm2.Dispose();
            Dtm3.Dispose();
            Dtm4.Dispose();

        }
    }
}