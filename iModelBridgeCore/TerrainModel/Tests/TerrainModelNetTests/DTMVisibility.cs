using System;
using System.Text;
using System.IO;
using BGEO = Bentley.GeometryNET;
using NUnit.Framework;


namespace Bentley.TerrainModelNET.NUnit
{
    using Bentley.TerrainModelNET;

    /// <summary>
    /// Class DTMVisibility
    /// </summary>
    /// <category></category>
    [TestFixture]
    public class DTMVisibility : DTMUNitTest
    {

        /// <summary>
        /// DTM  Browse Visibility
        /// </summary>
        /// <category>DTM Visibility</category>
        /// <author>Rob Cormack</author>
        /// <date>17MARCH2010</date>

        class FeatureCounts
        {
            private int featureCount;
            private int visiblePointCount;
            private int invisiblePointCount;
            private int visibleLineCount;
            private int invisibleLineCount;
            private DTM dtm;


            public int FeatureCount { get { return featureCount; } set { featureCount = value; } }
            public int VisiblePointCount { get { return visiblePointCount; } set { visiblePointCount = value; } }
            public int InvisiblePointCount { get { return invisiblePointCount; } set { invisiblePointCount = value; } }
            public int VisibleLineCount { get { return visibleLineCount; } set { visibleLineCount = value; } }
            public int InvisibleLineCount { get { return invisibleLineCount; } set { invisibleLineCount = value; } }
            public DTM FeatureDTM { get { return dtm; } }
            public void AddDtmFeature(BGEO.DPoint3d[] points, DTMFeatureType featureType)
            {
                dtm.AddLinearFeature(points, featureType);
            }
            public void CreateDTM()
            {
                if (this.dtm == null) this.dtm = new DTM();
            }


            public FeatureCounts() 
            {
                this.featureCount = 0;
                this.visiblePointCount = 0;
                this.invisiblePointCount = 0;
                this.visibleLineCount = 0;
                this.invisibleLineCount = 0;
                this.dtm = null;
            }
        }

        bool browseVisibilityFeatures(DTMDynamicFeatureInfo info, object userArg)
        {
            if (userArg != null)
            {
                FeatureCounts featureCounts = (FeatureCounts)userArg;
                ++featureCounts.FeatureCount;
                if (info.FeatureType == DTMDynamicFeatureType.VisiblePoint) ++featureCounts.VisiblePointCount;
                if (info.FeatureType == DTMDynamicFeatureType.InvisiblePoint) ++featureCounts.InvisiblePointCount;
                if (info.FeatureType == DTMDynamicFeatureType.VisibleLine) ++featureCounts.VisibleLineCount;
                if (info.FeatureType == DTMDynamicFeatureType.InvisibleLine) ++featureCounts.InvisibleLineCount;
                if (info.FeatureType == DTMDynamicFeatureType.VisibleLine && featureCounts.FeatureDTM != null)
                {
                    featureCounts.AddDtmFeature(info.FeaturePoints, DTMFeatureType.Breakline);
                }
            }
            return true ;
        }

        [Test]
        [Category("DTMVisibility")]
        public void PointVisibility()
        {
            IgnoreFirebug();

            DateTime startTime = DateTime.Now;
            TimeSpan elapsedTime = DateTime.Now - startTime;
            Console.WriteLine("Determining Point Visibility");

            //  Set Path To Folder Holding DTM Files 

            String DataPath = Helper.GetTestDataLocation () +@"Bentley.Civil.Dtm.NUnit.dll\\DTMDrainageTests\DTMDrainageCatchmentTests\";

            // Get Test Data Directories

            DirectoryInfo testFolder = new DirectoryInfo(DataPath);
            if (!testFolder.Exists)
            {
                throw new DirectoryNotFoundException("DTM Drainage Catchment Test Folder Not Found");
            }

            // Scan And Determine Catchments For All The DTM Files In The Test Folder

            foreach (System.IO.FileInfo nextFile in testFolder.GetFiles())
            {
                if (string.Compare(nextFile.Extension, ".dtm") == 0)
                {
                    Console.WriteLine("DTM File = {0}", nextFile.Name);

                    // Import The DTM File

                    Console.WriteLine("Importing {0}", nextFile.Name);
                    DateTime impstartTime = DateTime.Now;
                    DTM dtm = DTM.CreateFromFile(DataPath + nextFile.Name);

                    //  Set Eye And Point Locations

                    BGEO.DRange3d range = dtm.Range3d;
                    BGEO.DPoint3d eye = new BGEO.DPoint3d(range.Low.X, range.Low.Y, range.High.Z);
                    BGEO.DPoint3d point = new BGEO.DPoint3d();
                    point.X = (range.Low.X + range.High.X) / 2.0;
                    point.Y = (range.Low.Y + range.High.Y) / 2.0;
                    point.Z = (range.Low.Z + range.High.Z) / 2.0;

                    // Create Class To Accumulate Feature Counts

                    // Determine Visibility Of Point From Eye

                    bool visiblity = dtm.PointVisibility(eye, point);

                    // Dispose DTM To Release Allocated Unmanaged Memory

                    dtm.Dispose();
                }
            }
        }

        [Test]
        [Category("DTMVisibility")]
        public void LineVisibility()
        {
            IgnoreFirebug();

            DateTime startTime = DateTime.Now;
            TimeSpan elapsedTime = DateTime.Now - startTime;
            Console.WriteLine("Determining Line Visibility");

            //  Set Path To Folder Holding DTM Files 

            String DataPath = Helper.GetTestDataLocation () +@"Bentley.Civil.Dtm.NUnit.dll\\DTMDrainageTests\DTMDrainageCatchmentTests\";

            // Get Test Data Directories

            DirectoryInfo testFolder = new DirectoryInfo(DataPath);
            if (!testFolder.Exists)
            {
                throw new DirectoryNotFoundException("DTM Drainage Catchment Test Folder Not Found");
            }

            // Scan And Determine Catchments For All The DTM Files In The Test Folder

            foreach (System.IO.FileInfo nextFile in testFolder.GetFiles())
            {
                if (string.Compare(nextFile.Extension, ".dtm") == 0)
                {
                    Console.WriteLine("DTM File = {0}", nextFile.Name);

                    // Import The DTM File

                    Console.WriteLine("Importing {0}", nextFile.Name);
                    DateTime impstartTime = DateTime.Now;
                    DTM dtm = DTM.CreateFromFile(DataPath + nextFile.Name);

                    //  Set Eye And Point Locations

                    BGEO.DRange3d range = dtm.Range3d;
                    BGEO.DPoint3d eye = new BGEO.DPoint3d(range.Low.X, range.Low.Y, range.High.Z);
                    BGEO.DPoint3d point1 = new BGEO.DPoint3d();
                    point1.X = range.Low.X;
                    point1.Y = range.Low.Y;
                    point1.Z = range.Low.Z;
                    BGEO.DPoint3d point2 = new BGEO.DPoint3d();
                    point2.X = range.High.X;
                    point2.Y = range.High.Y;
                    point2.Z = range.High.Z;

                    // Determine Visibility Of Point From Eye

                    VisibilityResult lineVisibility = dtm.LineVisibility(eye,point1,point2);

                    // Dispose DTM To Release Allocated Unmanaged Memory

                    dtm.Dispose();
                }
            }
        }
        [Test]
        [Category("DTMVisibility")]
        public void BrowseTinPointsVisibility()
        {
            IgnoreFirebug();

            DateTime startTime = DateTime.Now;
            TimeSpan elapsedTime = DateTime.Now - startTime;
            Console.WriteLine("Determining Tin Point Visibility");

            //  Set Path To Folder Holding DTM Files 

            String DataPath = Helper.GetTestDataLocation () +@"Bentley.Civil.Dtm.NUnit.dll\\DTMDrainageTests\DTMDrainageCatchmentTests\";

            // Get Test Data Directories

            DirectoryInfo testFolder = new DirectoryInfo(DataPath);
            if (!testFolder.Exists)
            {
                throw new DirectoryNotFoundException("DTM Drainage Catchment Test Folder Not Found");
            }

            // Scan And Determine Catchments For All The DTM Files In The Test Folder

            foreach (System.IO.FileInfo nextFile in testFolder.GetFiles())
            {
                if (string.Compare(nextFile.Extension, ".dtm") == 0)
                {
                    Console.WriteLine("DTM File = {0}", nextFile.Name);

                    // Import The DTM File

                    Console.WriteLine("Importing {0}", nextFile.Name);
                    DateTime impstartTime = DateTime.Now;
                    DTM dtm = DTM.CreateFromFile(DataPath + nextFile.Name);

                    //  Set Eye Location

                    BGEO.DRange3d range = dtm.Range3d;
                    BGEO.DPoint3d eye = new BGEO.DPoint3d((range.Low.X + range.High.X) / 2.0, (range.Low.Y + range.High.Y) / 2.0, range.High.Z);

                    // Create Browsing Delegate

                    DynamicFeaturesBrowsingDelegate browsingDelegate = new DynamicFeaturesBrowsingDelegate(browseVisibilityFeatures);

                    // Browse Tin Points Visibility

                    Console.WriteLine("Browsing Tin Points Visibility");
                    startTime = DateTime.Now;
                    FeatureCounts featureCounts = new FeatureCounts();
                    dtm.BrowseTinPointsVisibility(eye, browsingDelegate, featureCounts);
                    Console.WriteLine("Number of Visibility Features = {0}", featureCounts.FeatureCount);
                    Console.WriteLine("Number of Visible Points = {0}", featureCounts.VisiblePointCount);
                    Console.WriteLine("Number of Invisible Points = {0}", featureCounts.InvisiblePointCount);
                    elapsedTime = DateTime.Now - startTime;
                    Console.WriteLine("Time To Browse Tin Points Visibility = {0} seconds", elapsedTime.TotalSeconds);


                    // Dispose DTM To Release Allocated Unmanaged Memory

                    dtm.Dispose();
                }
            }
        }

        [Test]
        [Category("DTMVisibility")]
        public void BrowseTinLinesVisibility()
        {
            IgnoreFirebug();

            DateTime startTime = DateTime.Now;
            TimeSpan elapsedTime = DateTime.Now - startTime;
            Console.WriteLine("Determining Tin Line Visibility");

            //  Set Path To Folder Holding DTM Files 

            String DataPath = Helper.GetTestDataLocation () +@"Bentley.Civil.Dtm.NUnit.dll\\DTMDrainageTests\DTMDrainageCatchmentTests\";

            // Get Test Data Directories

            DirectoryInfo testFolder = new DirectoryInfo(DataPath);
            if (!testFolder.Exists)
            {
                throw new DirectoryNotFoundException("DTM Drainage Catchment Test Folder Not Found");
            }

            // Scan And Determine Catchments For All The DTM Files In The Test Folder

            foreach (System.IO.FileInfo nextFile in testFolder.GetFiles())
            {
                if (string.Compare(nextFile.Extension, ".dtm") == 0)
                {
                    Console.WriteLine("DTM File = {0}", nextFile.Name);

                    // Import The DTM File

                    Console.WriteLine("Importing {0}", nextFile.Name);
                    DateTime impstartTime = DateTime.Now;
                    DTM dtm = DTM.CreateFromFile(DataPath + nextFile.Name);

                    //  Set Eye Location

                    BGEO.DRange3d range = dtm.Range3d;
                    BGEO.DPoint3d eye = new BGEO.DPoint3d((range.Low.X + range.High.X) / 2.0, (range.Low.Y + range.High.Y) / 2.0, range.High.Z);

                    // Create Browsing Delegate

                    DynamicFeaturesBrowsingDelegate browsingDelegate = new DynamicFeaturesBrowsingDelegate(browseVisibilityFeatures);

                    // Browse Tin Lines Visibility

                    Console.WriteLine("Browsing Tin Lines Visibility");
                    startTime = DateTime.Now;
                    FeatureCounts featureCounts = new FeatureCounts();
                    dtm.BrowseTinLinesVisibility(eye, browsingDelegate, featureCounts);
                    Console.WriteLine("Number of Visibility Features = {0}", featureCounts.FeatureCount);
                    Console.WriteLine("Number of Visible Lines = {0}", featureCounts.VisibleLineCount);
                    Console.WriteLine("Number of Invisible Lines = {0}", featureCounts.InvisibleLineCount);
                    elapsedTime = DateTime.Now - startTime;
                    Console.WriteLine("Time To Browse Tin Lines Visibility = {0} seconds", elapsedTime.TotalSeconds);


                    // Dispose DTM To Release Allocated Unmanaged Memory

                    dtm.Dispose();
                }
            }
        }
        [Test]
        [Category("DTMVisibility")]
        public void BrowseRegionViewSheds()
        {
            IgnoreFirebug();

            DateTime startTime = DateTime.Now;
            TimeSpan elapsedTime = DateTime.Now - startTime;
            Console.WriteLine("Browsing Region View Sheds");

            //  Set Path To Folder Holding DTM Files 

            String DataPath = Helper.GetTestDataLocation () +@"Bentley.Civil.Dtm.NUnit.dll\\DTMDrainageTests\DTMDrainageCatchmentTests\";

            // Get Test Data Directories

            DirectoryInfo testFolder = new DirectoryInfo(DataPath);
            if (!testFolder.Exists)
            {
                throw new DirectoryNotFoundException("DTM Drainage Catchment Test Folder Not Found");
            }

            // Scan And Determine Catchments For All The DTM Files In The Test Folder

            foreach (System.IO.FileInfo nextFile in testFolder.GetFiles())
            {
                if (string.Compare(nextFile.Extension, ".dtm") == 0)
                {
                    Console.WriteLine("DTM File = {0}", nextFile.Name);

                    // Import The DTM File

                    Console.WriteLine("Importing {0}", nextFile.Name);
                    DateTime impstartTime = DateTime.Now;
                    DTM dtm = DTM.CreateFromFile(DataPath + nextFile.Name);

                    //  Set Eye Location

                    BGEO.DRange3d range = dtm.Range3d;
                    BGEO.DPoint3d eye = new BGEO.DPoint3d((range.Low.X + range.High.X) / 2.0, (range.Low.Y + range.High.Y) / 2.0, range.High.Z);

                    // Create Browsing Delegate

                    DynamicFeaturesBrowsingDelegate browsingDelegate = new DynamicFeaturesBrowsingDelegate(browseVisibilityFeatures);

                    // Create DTM To Store Visibility Polygons

                    FeatureCounts featureCounts = new FeatureCounts();
                    featureCounts.CreateDTM();

                    // Browse Radial View Sheds

                    Console.WriteLine("Browsing Region View Sheds");
                    startTime = DateTime.Now;
                    dtm.BrowseRegionViewSheds(eye, browsingDelegate, featureCounts);
                    Console.WriteLine("Number of Visibility Features = {0}", featureCounts.FeatureCount);
                    Console.WriteLine("Number of Visible Features = {0}", featureCounts.VisibleLineCount);
                    Console.WriteLine("Number of Invisible Features = {0}", featureCounts.InvisibleLineCount);
                    elapsedTime = DateTime.Now - startTime;
                    Console.WriteLine("Time To Browse Region View Shed = {0} seconds", elapsedTime.TotalSeconds);

                    // Write View Shed Polygons

                    if( featureCounts.FeatureDTM != null ) featureCounts.FeatureDTM.Save(DataPath + "viewShedPolygons.dtm");

                    // Dispose DTM To Release Allocated Unmanaged Memory

                    dtm.Dispose();
                }
            }
        }
        [Test]
        [Category("DTMVisibility")]
        public void BrowseRadialViewSheds()
        {
            IgnoreFirebug();

            DateTime startTime = DateTime.Now;
            TimeSpan elapsedTime = DateTime.Now - startTime;
            Console.WriteLine("Browsing Radial View Sheds");

            //  Set Path To Folder Holding DTM Files 

            String DataPath = Helper.GetTestDataLocation () +@"Bentley.Civil.Dtm.NUnit.dll\\DTMDrainageTests\DTMDrainageCatchmentTests\";

            // Get Test Data Directories

            DirectoryInfo testFolder = new DirectoryInfo(DataPath);
            if (!testFolder.Exists)
            {
                throw new DirectoryNotFoundException("DTM Drainage Catchment Test Folder Not Found");
            }

            // Scan And Determine Catchments For All The DTM Files In The Test Folder

            foreach (System.IO.FileInfo nextFile in testFolder.GetFiles())
            {
                if (string.Compare(nextFile.Extension, ".dtm") == 0)
                {
                    Console.WriteLine("DTM File = {0}", nextFile.Name);

                    // Import The DTM File

                    Console.WriteLine("Importing {0}", nextFile.Name);
                    DateTime impstartTime = DateTime.Now;
                    DTM dtm = DTM.CreateFromFile(DataPath + nextFile.Name);

                    //  Set Eye Location

                    BGEO.DRange3d range = dtm.Range3d;
                    BGEO.DPoint3d eye = new BGEO.DPoint3d((range.Low.X + range.High.X) / 2.0, (range.Low.Y + range.High.Y) / 2.0, range.High.Z);

                    // Create Radial View Shed Criteria And Browsing Delegate

                    RadialViewShedsBrowsingCriteria radialViewShedCriteria = new RadialViewShedsBrowsingCriteria(eye);
                    DynamicFeaturesBrowsingDelegate browsingDelegate = new DynamicFeaturesBrowsingDelegate(browseVisibilityFeatures);

                    // Browse Radial View Sheds

                    Console.WriteLine("Browsing Radial View Sheds");
                    startTime = DateTime.Now;
                    FeatureCounts featureCounts = new FeatureCounts();
                    featureCounts.CreateDTM();
                    radialViewShedCriteria.NumberRadials = 90;
                    dtm.BrowseRadialViewSheds(radialViewShedCriteria, browsingDelegate, featureCounts);
                    Console.WriteLine("Number of Visibility Features = {0}", featureCounts.FeatureCount);
                    Console.WriteLine("Number of Visible Features = {0}", featureCounts.VisibleLineCount);
                    Console.WriteLine("Number of Invisible Features = {0}", featureCounts.InvisibleLineCount);
                    elapsedTime = DateTime.Now - startTime;
                    Console.WriteLine("Time To Browse Radial View Shed = {0} seconds", elapsedTime.TotalSeconds);


                    // Write View Shed Radials 

                    if (featureCounts.FeatureDTM != null) featureCounts.FeatureDTM.Save(DataPath + "viewShedRadials.dtm");

                    // Dispose DTM To Release Allocated Unmanaged Memory

                    dtm.Dispose();
                }
            }
        }
    }
}
