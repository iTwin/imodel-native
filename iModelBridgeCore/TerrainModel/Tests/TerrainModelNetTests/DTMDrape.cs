using System;
using System.IO;
using System.Text;
using BGEO = Bentley.GeometryNET;
using Bentley.TerrainModelNET;
using NUnit.Framework;



namespace Bentley.TerrainModelNET.NUnit
{

    [TestFixture]
    public class DTMDrape : DTMUNitTest
    {

        [Test]
        public void SimpleLinearElementTest()
        {
            DTM dtm = new DTM();

            BGEO.DPoint3d[] points = new Bentley.GeometryNET.DPoint3d[5];

            for (int i = 1; i < 10; i++)
            {
                points[0].X = 1000 - (i * 100);
                points[0].Y = 1000 - (i * 100);
                points[2].X = 1000 + (i * 100);
                points[2].Y = 1000 + (i * 100);
                points[1].X = points[0].X;
                points[1].Y = points[2].Y;
                points[3].X = points[2].X;
                points[3].Y = points[0].Y;
                points[4] = points[0];
                points[0].Z = points[1].Z = points[2].Z = points[3].Z = points[4].Z = i * 10;
                dtm.AddLinearFeature(points, DTMFeatureType.Breakline);
            }

            for (int i = 1; i < 10; i++)
            {
                points[0].X = 3000 - (i * 100);
                points[0].Y = 3000 - (i * 100);
                points[2].X = 3000 + (i * 100);
                points[2].Y = 3000 + (i * 100);
                points[1].X = points[0].X;
                points[1].Y = points[2].Y;
                points[3].X = points[2].X;
                points[3].Y = points[0].Y;
                points[4] = points[0];
                points[0].Z = points[1].Z = points[2].Z = points[3].Z = points[4].Z = i * 10;
                dtm.AddLinearFeature(points, DTMFeatureType.Breakline);
            }

            points = new Bentley.GeometryNET.DPoint3d[8];
            points[0] = new Bentley.GeometryNET.DPoint3d(0, 0);
            points[1] = new Bentley.GeometryNET.DPoint3d(2000, 0);
            points[2] = new Bentley.GeometryNET.DPoint3d(2000, 2000);
            points[3] = new Bentley.GeometryNET.DPoint3d(4000, 2000);
            points[4] = new Bentley.GeometryNET.DPoint3d(4000, 4000);
            points[5] = new Bentley.GeometryNET.DPoint3d(2000, 4000);
            points[6] = new Bentley.GeometryNET.DPoint3d(0, 2000);
            points[7] = new Bentley.GeometryNET.DPoint3d(0, 0);
            points[0].Z = points[1].Z = points[2].Z = points[3].Z = points[4].Z = points[5].Z = points[6].Z = points[7].Z = 100;
            dtm.AddLinearFeature(points, DTMFeatureType.Hull);

            dtm.Triangulate();

 //           dtm.Save(@"D:\CivilPlatform\Civil\Bentley.Civil.Dtm.NUnit.dll\drape.tin");
// ToDo...Standalone
            //Bentley.Civil.Geometry.Linear.Line line = new Bentley.Civil.Geometry.Linear.Line(new BGEO.DPoint3d(0, -500), new BGEO.DPoint3d(4500, 4000));
            //DTMDrapedLinearElement drapedElement = Bentley.TerrainModelNET.LinearGeometry.Helper.DTMDrapeLinearElement (dtm, line, 1);
            //int num = 0;
            //Geometry.Linear.ProfileComplex complex = Bentley.TerrainModelNET.LinearGeometry.Helper.DTMDrapedLinearElementGetProfile(drapedElement) as Geometry.Linear.ProfileComplex;
            //Geometry.Linear.ProfileElement[] l = complex.GetSubProfileElements();
            //Assert.IsFalse (Bentley.TerrainModelNET.LinearGeometry.Helper.DTMDrapedLinearElementGetProfile (drapedElement).IsDefinedAtX (10));
            //Assert.IsTrue (Bentley.TerrainModelNET.LinearGeometry.Helper.DTMDrapedLinearElementGetProfile (drapedElement).IsDefinedAtX (1510));
            //Assert.IsFalse (Bentley.TerrainModelNET.LinearGeometry.Helper.DTMDrapedLinearElementGetProfile (drapedElement).IsDefinedAtX (3010));
            //Assert.IsTrue (Bentley.TerrainModelNET.LinearGeometry.Helper.DTMDrapedLinearElementGetProfile (drapedElement).IsDefinedAtX (4010));
            //Assert.IsFalse (Bentley.TerrainModelNET.LinearGeometry.Helper.DTMDrapedLinearElementGetProfile (drapedElement).IsDefinedAtX (5810));

            //foreach (DTMDrapedLinearElementPoint point in drapedElement)
            //{
            //    num++;
            //    System.Console.Out.WriteLine(point.Code.ToString());
            //}
        }



        static String DataDirectory = @"Bentley.Civil.Dtm.NUnit.dll\DTMDrapeTest\";
        static String DataPath = Helper.GetTestDataLocation () + DataDirectory;

        /// <summary>
        /// DTMDrapeTest
        /// </summary>
        /// <returns></returns>
        [Test]
        [Category("DTMDrapeTests")]
        public void DtmDrapeTests()
        {
            // Check Dtm Test Level

            Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
            IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

           // Check Drape Test Data Directory Exists

            DirectoryInfo testFolder = new DirectoryInfo(DataPath);
            if (!testFolder.Exists)
            {
                throw new DirectoryNotFoundException(" DTM Drape Test Data Folder Not Found + DataPath");
            }

            // Scan All The Drape Test Data Directories

            foreach (DirectoryInfo nextFolder in testFolder.GetDirectories())
            {
                Console.WriteLine("Test Data Folder = {0}", nextFolder.Name);

                if (string.Compare(nextFolder.Name, "CVS") != 0)
                {

                    // Check Drape Line File Exists

                    bool drapeTinExists  = System.IO.File.Exists(DataPath + nextFolder.Name + "\\drape.tin");
                    bool drapeLineExists = System.IO.File.Exists(DataPath + nextFolder.Name + "\\drapeLine.xyz");

                    if (drapeTinExists == true && drapeLineExists == true)
                    {
                        // Import Drape DTM

                        DTM drapeDTM = DTM.CreateFromFile(DataPath + nextFolder.Name + "\\drape.tin");

                        //  Read Drape Line From File

                        BGEO.DPoint3d[] drapeLine = DTM.ImportDPoint3DArrayFromXYZFile(DataPath + nextFolder.Name + "\\drapeLine.xyz");

                        Console.WriteLine("drapeLine Length = {0}", drapeLine.Length);

                        // Drape The Line

                        DTMDrapedLinearElement drapedElement = drapeDTM.DrapeLinearPoints(drapeLine);

                        // Write Drape Points To Console

                        foreach (DTMDrapedLinearElementPoint point in drapedElement)
                        {
                            Console.WriteLine("drapeStringPointCode = {0} ** {1},{2},{3}", point.Code, point.Coordinates.X, point.Coordinates.Y, point.Coordinates.Z);
                        }

                       // Write Features At Each Drape Point

                        foreach (DTMDrapedLinearElementPoint point in drapedElement)
                        {
                            DTMDrapePointFeature[] features = point.Features;
                            Console.WriteLine("Number Of Features At Drape Point = {0}",features.Length);
                            for (int n = 0; n < features.Length; ++n)
                            {
                                Console.WriteLine("Drape Point Feature Ind = {0} T = {1} U = {2} Id = {3} Pp = {4} Np = {5} ",features[n].DtmFeatureIndex, features[n].DtmFeatureType, features[n].DtmUserTag,features[n].DtmFeatureId, features[n].DtmFeaturePriorPoint, features[n].DtmFeatureNextPoint);
                            }
                        }


                        //  Dispose DTM 
 
                        drapeDTM.Dispose();
                    }
                }
            }
        }
    }
}
    
