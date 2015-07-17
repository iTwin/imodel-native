using System;
using SD = System.Data;
using SDO = System.Data.OleDb;
using BGEO = Bentley.GeometryNET;
using NUnit.Framework;

namespace Bentley.TerrainModelNET.NUnit
{
    /// <summary>
    ///  DTM Import/Export Tests
    /// </summary>
    [TestFixture]
    public class DTMImportExport : DTMUNitTest
    {
        /// <summary>
        /// Append Problem Driver
        /// </summary>

        [Test]
        [Category("DTM Import Export Tests")]
        public void GeopakTinImportExportTest()
        {
            IgnoreFirebug();

            //List constant string variables
            String dbPath = @"Bentley.Civil.Dtm.NUnit.dll\DTMImportExportTest\GeopakTins\";
            String DataPath = Helper.GetTestDataLocation () + dbPath;

            // Import Geopak Tin File To New DTM Instance
            Console.WriteLine("Importing Tin File To New DTM Instance");
            DTM Dtm = DTM.CreateFromGeopakTinFile(DataPath + "geopak.tin");

            // Check The Triangulation
            if (Dtm.CheckTriangulation() == false)
            {
                throw new Exception("Dtm Triangulation Invalid");
            }
            Console.WriteLine("Dtm Triangulation Valid");

            // Expoting DTM Instance To Geopak Tin File
            Console.WriteLine("Exporting DTM Instance To Geopak Tin File");
            Dtm.SaveAsGeopakTinFile(DataPath + "exportedGeopak.tin");

            Console.WriteLine("Importing Tin File To New DTM Instance");
            DTM Dtm1 = DTM.CreateFromGeopakTinFile(DataPath + "exportedGeopak.tin");

            // Check The Triangulation
            if (Dtm1.CheckTriangulation() == false)
            {
                throw new Exception("Dtm Triangulation Invalid");
            }
            Console.WriteLine("Dtm Triangulation Valid");

            Dtm1.Dispose();

            // Populate DTM Instance From Geopak Tin File
            Console.WriteLine("Populating DTM Instance From Geopak Tin File");
            Dtm.PopulateFromGeopakTinFile(DataPath + "geopak.tin");

            // Check The Triangulation
            if (Dtm.CheckTriangulation() == false)
            {
                throw new Exception("Dtm Triangulation Invalid");
            }
            Console.WriteLine("Dtm Triangulation Valid");

            // Expoting DTM Instance To Geopak Tin File
            Console.WriteLine("Exporting DTM Instance To Geopak Tin File");
            Dtm.SaveAsGeopakTinFile(DataPath + "exportedGeopak02.tin");

            // Clean Up

            Dtm.Dispose();

        }

        [Test]
        [Category("DTM Import Export Tests")]
        public void MXDtmImportExportTest()
            {
            IgnoreFirebug();
            //List constant string variables
            String dbPath = @"Bentley.Civil.Dtm.NUnit.dll\DTMImportExportTest\MxModelFiles\";
            String DataPath = Helper.GetTestDataLocation () + dbPath;

            Console.WriteLine("Opening Model File");
            using (Bentley.TerrainModelNET.Formats.MXFilImporter importer = Bentley.TerrainModelNET.Formats.MXFilImporter.Create (DataPath + "mxModel000.fil"))
                {

                foreach (Bentley.TerrainModelNET.Formats.TerrainInfo info in importer.Terrains)
                    {
                    Console.WriteLine("Contains Surface " + info.Name);
                    }

                Bentley.TerrainModelNET.Formats.ImportedTerrain terrain = importer.ImportTerrain ("TX00SEEDED");

                if (terrain != null && terrain.Terrain != null)
                    {
                    Console.WriteLine("Checking bclib Dtm Triangulation");
//                    Bentley.Civil.Geometry.Linear.ClosedLineString hull = terrain.Terrain.Hull;
                    if (terrain.Terrain.CheckTriangulation () == false)
                        {
                        throw new Exception("bcLIB Dtm Triangulation Invalid");
                        }
                    }
                }
            }
/*
        [Test]
        [Category("DTM Import Export Tests")]
        public void InroadsDtmImportExportTest()
        {
            IgnoreFirebug();

            //List constant string variables
            String dbPath = @"Bentley.Civil.Dtm.NUnit.dll\DTMImportExportTest\InRoadsDTMs\";
            String DataPath = Helper.GetTestDataLocation () + dbPath;

            //  ******** Round Trip Loop.dtm  *********

            // Import Inroads Dtm File To New DTM Instance
            Console.WriteLine("Importing Inroads Dtm File Loop.dtm To New DTM Instance");
            DTM Dtm1 = DTM.CreateFromInroadsDtmFile(DataPath + "empty.dtm");

            // Check The Triangulation
            Console.WriteLine("Checking bclib Dtm Triangulation");
            if (Dtm1.CheckTriangulation() == false)
            {
                throw new Exception("bcLIB Dtm Triangulation Invalid");
            }
            Console.WriteLine("bclib Dtm Triangulation Valid");

            // Expoting DTM Instance To Inroads DTM File
            Console.WriteLine("Exporting DTM Instance To Inroads DTM File exportedLoop.dtm");
            Dtm1.ExportToInroadsDtmFile(DataPath + "exportedLoop.dtm", "loopName", "loopDescription");

            // Clean Up

            Dtm1.Dispose();

            // Import Inroads Dtm File To New DTM Instance
            Console.WriteLine("Importing Inroads Dtm File exportedLoop.dtm To New DTM Instance");
            DTM Dtm2 = DTM.CreateFromInroadsDtmFile(DataPath + "exportedLoop.dtm");

            // Check The Triangulation
            Console.WriteLine("Checking bclib Dtm Triangulation");
            if (Dtm2.CheckTriangulation() == false)
            {
                throw new Exception("bcLIB Dtm Triangulation Invalid");
            }
            Console.WriteLine("bclib Dtm Triangulation Valid");

            // Clean Up

            Dtm2.Dispose();

            //  ******** Round Trip natural.dtm  *********

            // Import Inroads Dtm File To New DTM Instance
            Console.WriteLine("Importing Dtm File Natural.dtm To New DTM Instance");
            DTM Dtm3 = DTM.CreateFromInroadsDtmFile(DataPath + "natural.dtm");

            // Check The Triangulation
            Console.WriteLine("Checking bclib Dtm Triangulation");
            if (Dtm3.CheckTriangulation() == false)
            {
                throw new Exception("bcLIB Dtm Triangulation Invalid");
            }
            Console.WriteLine("bcLIB Dtm Triangulation Valid");

            // Expoting DTM Instance To Inroads Dtm File
            Console.WriteLine("Exporting bcLIB DTM Instance To Inroads DTM File exportedNatural.dtm ");
            Dtm3.ExportToInroadsDtmFile(DataPath + "exportedNatural.dtm", "naturalName", "naturalDescription");

            // Clean Up

            Dtm3.Dispose();

            // Import Inroads Dtm File To New DTM Instance
            Console.WriteLine("Importing Inroads Dtm File exportedNatural.dtm To New DTM Instance");
            DTM Dtm4 = DTM.CreateFromInroadsDtmFile(DataPath + "exportedNatural.dtm");

            // Check The Triangulation
            Console.WriteLine("Checking bclib Dtm Triangulation");
            if (Dtm4.CheckTriangulation() == false)
            {
                throw new Exception("bcLIB Dtm Triangulation Invalid");
            }
            Console.WriteLine("bclib Dtm Triangulation Valid");

            // Clean Up

            Dtm4.Dispose();

            //  ******** Round Trip clipped.dtm  *********

            // Import Inroads Dtm File To New DTM Instance
            Console.WriteLine("Importing Inroads Dtm File clip.dtm To New DTM Instance");
            DTM Dtm5 = DTM.CreateFromInroadsDtmFile(DataPath + "clipped.dtm");

            // Check The Triangulation
            Console.WriteLine("Checking bclib Dtm Triangulation");
            if (Dtm5.CheckTriangulation() == false)
            {
                throw new Exception("bcLIB Dtm Triangulation Invalid");
            }
            Console.WriteLine("bclib Dtm Triangulation Valid");

            Dtm5.Save(DataPath + "clipped.tin") ;

            // Expoting DTM Instance To Inroads DTM File
            Console.WriteLine("Exporting DTM Instance To Inroads DTM File exportedClipped.dtm");
            Dtm5.ExportToInroadsDtmFile(DataPath + "exportedClipped.dtm", "clipName", "clipDescription");

            // Import Exported Inroads Dtm File To New DTM Instance
            Console.WriteLine("Importing Inroads Dtm File exportedClipped.dtm To New DTM Instance");
            DTM Dtm6 = DTM.CreateFromInroadsDtmFile(DataPath + "exportedClipped.dtm");

            // Check The Triangulation
            Console.WriteLine("Checking bclib Dtm Triangulation");
            if (Dtm6.CheckTriangulation() == false)
            {
                throw new Exception("bcLIB Dtm Triangulation Invalid");
            }
            Console.WriteLine("bclib Dtm Triangulation Valid");


            // Clean Up

            Dtm5.Dispose();
            Dtm6.Dispose();
        }
*/
        [Test]
        [Category("DTM Import Export Tests")]
        public void DtmConversionTests()
        {
            IgnoreFirebug();

            //List constant string variables
            String dbPath = @"Bentley.Civil.Dtm.NUnit.dll\DTMImportExportTest\GeopakTins\";
            String DataPath = Helper.GetTestDataLocation () + dbPath;

            //  ******** Round Trip Geopak.Tin  *********

            Console.WriteLine("Importing Geopak Tin To New DTM Instance");
            DTM gpkDtm1 = DTM.CreateFromGeopakTinFile(DataPath + "geopak.tin");

            // Check The Triangulation
            Console.WriteLine("Checking bclib Dtm Triangulation");
            if (gpkDtm1.CheckTriangulation() == false)
            {
                throw new Exception("bclib Dtm Triangulation Invalid");
            }
            Console.WriteLine("bclib Dtm Triangulation Valid");

            // Expoting DTM Instance To Inroads DTM File
            Console.WriteLine("Exporting DTM Instance To Inroads DTM File geopak.dtm");
// ToDo Vancouver            gpkDtm1.ExportToInroadsDtmFile(DataPath + "geopak.dtm", "geopakName", "geoapkDescription");

            gpkDtm1.Dispose();

            // Import Inroads Dtm File To New DTM Instance
            Console.WriteLine("Importing Dtm File geopak.dtm To New DTM Instance");
            DTM gpkDtm2 = null;
            using (Bentley.TerrainModelNET.Formats.InroadsImporter importer = Bentley.TerrainModelNET.Formats.InroadsImporter.Create (DataPath + "geopak.dtm"))
                {
                string name = null;
                foreach (Bentley.TerrainModelNET.Formats.TerrainInfo info in importer.Terrains)
                    {
                    Console.WriteLine ("Contains Surface " + info.Name);
                    name = info.Name;
                    }

                Bentley.TerrainModelNET.Formats.ImportedTerrain terrain = name != null ? importer.ImportTerrain (name) : null;

                if (terrain != null && terrain.Terrain != null)
                    {
                    gpkDtm2 = terrain.Terrain;
                    }
                }
            

            // Check The Triangulation
            Console.WriteLine("Checking bclib Dtm Triangulation");
            if (gpkDtm2.CheckTriangulation() == false)
            {
                throw new Exception("bcLIB Dtm Triangulation Invalid");
            }
            Console.WriteLine("bcLIB Dtm Triangulation Valid");

            // Expoting DTM Instance To Geopak Tin File
            Console.WriteLine("Exporting DTM Instance To Geopak Tin File");
            gpkDtm2.SaveAsGeopakTinFile(DataPath + "exportedGeopak.tin");

            // Clean Up

            gpkDtm2.Dispose();
        }

        [Test]
        [Category("DTM Import Export Tests")]
        public void DtmConversionTest631DTM()
        {
            IgnoreFirebug();

            //List constant string variables
            String dbPath = @"Bentley.Civil.Dtm.NUnit.dll\DTMImportExportTest\";
            String DataPath = Helper.GetTestDataLocation () + dbPath;

            //  ******** Round Trip Inroads exist631.dtm  *********

            // Import Inroads Dtm File To New DTM Instance
            Console.WriteLine("Importing Dtm File exist631.dtm To New DTM Instance");
            DTM dtm1 = null;
            using (Bentley.TerrainModelNET.Formats.InroadsImporter importer = Bentley.TerrainModelNET.Formats.InroadsImporter.Create (DataPath + "exist631.dtm"))
                {
                string name = null;
                foreach (Bentley.TerrainModelNET.Formats.TerrainInfo info in importer.Terrains)
                    {
                    Console.WriteLine ("Contains Surface " + info.Name);
                    name = info.Name;
                    }

                Bentley.TerrainModelNET.Formats.ImportedTerrain terrain = name != null ? importer.ImportTerrain (name) : null;

                if (terrain != null && terrain.Terrain != null)
                    {
                    dtm1 = terrain.Terrain;
                    }
                }
            
            Console.WriteLine("Importing Dtm File exist631.dtm Completed");

            // Check The Triangulation
            Console.WriteLine("Checking bclib Dtm Triangulation");
            if (dtm1.CheckTriangulation() == false)
            {
                throw new Exception("bcLIB Dtm Triangulation Invalid");
            }
            Console.WriteLine("bcLIB Dtm Triangulation Valid");

            // Expoting DTM Instance To Geopak Tin File
            Console.WriteLine("Exporting DTM Instance To Geopak Tin File exist631.tin");
            dtm1.SaveAsGeopakTinFile(DataPath + "exist631.tin");

            // Clean Up

            dtm1.Dispose();

        }

    }
}