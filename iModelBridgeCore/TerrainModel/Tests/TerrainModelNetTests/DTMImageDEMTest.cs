using System;
using System.Collections;
using System.IO;
using System.Runtime.InteropServices;
using SD = System.Data;
using SDO = System.Data.OleDb;
using BGEO = Bentley.GeometryNET;
using NUnit.Framework;

namespace Bentley.TerrainModelNET.NUnit
{
    using Bentley.TerrainModelNET;

    /// <summary>
    /// DTM Unit testing.
    /// </summary>
    [TestFixture]
    public class DTMImageDtmTest : DTMUNitTest
        {

        //  Delegate To Browse Image Parameters From DTM Core
        //  The Browse Delegate Keeps The Unmanged Base Coordinate System Object In Scope
        //  Until A Managed Base Coordinate System Object Can Be Created
        //  The managed base coordiante system constructor increments a reference to the unmanaged object
        //  that stops it being destroyed on the termination of the browse method  

        //private int imageDtmBrowser (ImagePP.Image image, int imageSize, string projectionName, string projectionDescription, string projectionCode, IntPtr baseGeoCordP, object userData)
        //    {
        //    int success = 0;
        //    //            
        //    //    Create Managed Base Coordinate System From baseGeoCordP
        //    //    
        //    Console.WriteLine ("numPixels = {0} name = {1} description = {2} Code = {3} ** baseGeoCordP = {4} userP = {5}", imageSize, projectionName, projectionDescription, projectionCode, baseGeoCordP, userData);
        //    return success;
        //    }


        /// <summary>
        /// Test exporting and importing LandXML
        /// </summary>
        [Test]
        [Category ("Import Image Tests")]
        public void DTMImportImageTests ()
            {
            // Check Dtm Test Level

            Console.WriteLine ("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
            IgnoreFirebug (); // If DTM_LEVEL = FIREBUG -> Ignore the test

            // Initialise The Geo Coordinate System By Setting The Path To The MicroStation GeoCoordinateData Folder
            // As The Location Of This Folder Is Machine Dependent It has to be Set By The Application Developer
            // If The Path To The MicroStation GeoCoordinateData Folder Is Not Set The Image Will Not Be Reprojected

            //string geoCoordinateDataPath = "D:\\BSW\\sharedpart_prebuilts\\powerplatform\\v8.11.7\\Bentley\\PseudoStation\\GeoCoordinateData";
            //Console.WriteLine ("geoCoordinateDataPath = {0}", geoCoordinateDataPath);
            //ImagePP.Image.InitialiseGeoCoordinateSystem (geoCoordinateDataPath);

            // Set Path To Image Files

            string imagePATH = Helper.GetTestDataLocation () + "Bentley.Civil.Dtm.NUnit.dll\\DTMImageDEMTests\\DTMImageTests";
            //            string imagePATH = Helper.GetTestDataLocation () +"Bentley.Civil.Dtm.NUnit.dll\\DTMImageDEMTests\\DTMDtedTests\\dtedTest000";
            //            string imagePATH = "C:\\DemRasters\\BIL\\16G00L0";                          
            //            string imagePATH = "C:\\DemRasters\\DEM_s01.dt2\\ECW";                      
            //            string imagePATH = "C:\\DemRasters\\DEM_s01.dt2\\Erdas";
            //            string imagePATH = "C:\\DemRasters\\DEM_s01.dt2\\Geotif";
            //            string imagePATH = "C:\\DemRasters\\DEM_s01.dt2\\NITF";
            //            string imagePATH = "C:\\DemRasters\\DEM_s01.dt2\\USGS_DEM";                
            //            string imagePATH = "C:\\DemRasters\\DTED\\16G00L0";
            //            string imagePATH = "C:\\DemRasters\\DTED\\16INT00L0";
            //            string imagePATH = "C:\\DemRasters\\SPOT Dimap\\16G00C0";
            //            string imagePATH = "C:\\DemRasters\\USGS SDTS DEM\\16INT00L0\\1123";
            //            string imagePATH = "C:\\DemRasters\\USGS SDTS DEM\\16INT00L0\\7397";
            //            string imagePATH = "C:\\DemRasters\\USGS DEM ASCII\\32F00I0";
            //            string imagePATH = "C:\\DemRasters\\USGS DEM ASCII\\16INT00I0";
            //            string imagePATH = "C:\\DemRasters\\ErdasIMG\\32F00R0";
            //            string imagePATH = "C:\\DemRasters\\ErdasIMG\\16INT00R0";                    


            // Test Image Folder Exists

            DirectoryInfo testFolder = new DirectoryInfo (imagePATH);
            if (!testFolder.Exists)
                {
                throw new DirectoryNotFoundException ("DTM Image Test Data Header Folder Not Found + imagePATH");
                }

            // Scan And Triangulate All The Image Files In The Folder

            foreach (System.IO.FileInfo nextFile in testFolder.GetFiles ())
                {
                if (string.Compare (nextFile.Extension, ".DDF") == 0 ||
                    string.Compare (nextFile.Extension, ".TIF") == 0 ||
                    string.Compare (nextFile.Extension, ".dt1") == 0 ||
                    string.Compare (nextFile.Extension, ".dt2") == 0 ||
                    string.Compare (nextFile.Extension, ".dem") == 0 ||
                    string.Compare (nextFile.Extension, ".ntf") == 0 ||
                    string.Compare (nextFile.Extension, ".tif") == 0 ||
                    string.Compare (nextFile.Extension, ".img") == 0 ||
                    string.Compare (nextFile.Extension, ".bil") == 0 ||
                    string.Compare (nextFile.Extension, ".ecw") == 0)
                    {
                    Console.WriteLine ("Image File = {0}", nextFile.Name);

                    // Create Image Instance

                    Bentley.TerrainModelNET.Formats.ImagePPConverter image = Bentley.TerrainModelNET.Formats.ImagePPConverter.Create (imagePATH + "\\" + nextFile.Name);

                    // Browse Image Size And Projection
                    // Use This Method If You Want To create A Managed Base Geocoordinate System Instance
                    // that can be used with the appropriate MicroStation dialogs

                    Console.WriteLine ("**** Browsing Image Size And Projection");
//ToDo                    image.BrowseImageSizeAndProjection (, imageDtmBrowser, null);
                    Console.WriteLine ("**** Image Browse Completed");

                    // Get Image Size And Projection

                    Console.WriteLine ("**** Getting Image Size And Projection");
                    Console.WriteLine ("imageWidth = {0} pixels", image.Width);
                    Console.WriteLine ("imageHeight = {0} pixels", image.Height);
                    Console.WriteLine ("imageSize = {0} pixels", image.NumberOfPixels);
                    if (image.GCS != null)
                        {
                        Console.WriteLine ("imageProjection  = {0}", image.GCS.Name);
                        Console.WriteLine ("imageDescription = {0}", image.GCS.Description);
                        }
                    else
                        {
                        Console.WriteLine ("no gcs");
                        }
                    Console.WriteLine ("**** Image Get Completed");


                    //  Set Image Tin Size And Projection

                    ulong TinMaxSize = 5000000;    // Set Maximum Tin Size In Pixels - ImagePP Filters The Image To This Number Of Pixels
                    string TinProjection = "LL84";  // Set Tin Projection Code

                    // Import Image To A DTM

                    Console.WriteLine ("**** Importing {0} To DTM", nextFile.Name);
                    DateTime impStartTime = DateTime.Now;
                    double imageScaleFactor = image.NumberOfPixels / TinMaxSize;

                    if (imageScaleFactor > 1)
                        imageScaleFactor = 1;
                    DTM imageDTM = image.ImportAndTriangulateImage (imageScaleFactor, TinProjection, 1, 1);
                    DateTime impEndTime = DateTime.Now;
                    TimeSpan impTime = impEndTime - impStartTime;
                    Console.WriteLine ("Import Time was For {0} Points Was {1} seconds.", imageDTM.VerticesCount, impTime.TotalSeconds);
                    Console.WriteLine ("**** Importing Completed");

                    // Check The Triangulation - Unit Test Checking And Development Only

                    Console.WriteLine ("Checking Triangulation");
                    DateTime chkStartTime = DateTime.Now;
                    if (imageDTM.CheckTriangulation () == false)
                        {
                        throw new Exception ("\nTriangulation Invalid");
                        }
                    Console.WriteLine ("Triangulation Valid");
                    DateTime chkEndTime = DateTime.Now;
                    TimeSpan chkTime = chkEndTime - chkStartTime;
                    Console.WriteLine ("Triangulation Validation Time was {0} seconds.", chkTime.TotalSeconds);

                    // Convert Units

                    Console.WriteLine ("Converting Units");
                    imageDTM.ConvertUnits (2.0, 5.0);
                    // Check The Triangulation - Unit Test Checking And Development Only

                    Console.WriteLine ("Checking Triangulation");
                    if (imageDTM.CheckTriangulation () == false)
                        {
                        throw new Exception ("\nTriangulation Invalid");
                        }
                    Console.WriteLine ("Triangulation Valid");


                    // Save DTM To File

                    //                   Console.WriteLine("Writing DTM To File {0}", imagePATH + "\\" + nextFile.Name + ".dtm");
                    //                   imageDTM.Save(imagePATH + "\\" + nextFile.Name + ".dtm");

                    // Dispose DTM To Release Allocated Unmanaged Memory

                    imageDTM.Dispose ();
                    }
                }
            }
        }
}


