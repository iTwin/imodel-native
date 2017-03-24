//using System;
//using System.Collections.Generic;
//using System.IO;
//using System.Linq;
//using System.Reflection;
//using System.Text;
//using System.Threading.Tasks;
//using IndexECPlugin.Source.Helpers;
//using Newtonsoft.Json.Linq;
//using NUnit.Framework;
//using Rhino.Mocks;

//namespace IndexECPlugin.Tests.Tests
//    {
//    [TestFixture]
//    class DataExtractorTests
//        {
//        [SetUp]
//        public void SetUp ()
//            {
//            }
//        [Test]
//        public void HRODataExtractorTest()
//            {
//            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("TnmApiHRO.txt"));
//            string jsonString = jsonStreamReader.ReadToEnd();
//            JToken tnmJson = JToken.Parse(jsonString);

//            var hroDataExtractor = new HRODataExtractor();

//            string title;
//            DateTime? date;
//            string resolution;
//            string resolutionInMeters;

//            hroDataExtractor.ExtractTitleDateAndResolution(tnmJson, out title, out date, out resolution, out resolutionInMeters);

//            Assert.AreEqual("USGS High Resolution Orthoimagery for Branch County, MI: 16TFM735530 200404 0x6000m BW 1", title);
//            Assert.AreEqual(new DateTime(2004,4,1), date.Value);
//            Assert.AreEqual("0.6000m", resolution);
//            Assert.AreEqual("0.6000x0.6000", resolutionInMeters);
//            }

//        [Test]
//        public void HRODataExtractorErrorTest ()
//            {
//            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("TnmApiHRO.txt"));
//            string jsonString = jsonStreamReader.ReadToEnd();
//            JToken tnmJson = JToken.Parse(jsonString);

//            tnmJson["downloadURL"] = "error";

//            var hroDataExtractor = new HRODataExtractor();

//            string title;
//            DateTime? date;
//            string resolution;
//            string resolutionInMeters;

//            hroDataExtractor.ExtractTitleDateAndResolution(tnmJson, out title, out date, out resolution, out resolutionInMeters);

//            Assert.IsFalse(date.HasValue);
//            Assert.IsNull(resolution);
//            Assert.IsNull(resolutionInMeters);

//            tnmJson["downloadURL"] = @"http://gisdata.usgs.gov/tdds/downloadfile.php?TYPE\u003dortho\u0026ORIG\u003dSBDDG\u0026FNAME\u003d16TFM735530_20404_0x6000m_BW_1.zip";
//            hroDataExtractor.ExtractTitleDateAndResolution(tnmJson, out title, out date, out resolution, out resolutionInMeters);
//            Assert.IsFalse(date.HasValue);
//            Assert.AreEqual("0.6000m", resolution);
//            Assert.AreEqual("0.6000x0.6000", resolutionInMeters);
//            }

//        [Test]
//        public void HRODataExtractorSBTest ()
//            {
//            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("ScienceBaseJsonHRO.txt"));
//            string jsonString = jsonStreamReader.ReadToEnd();
//            JToken tnmJson = JToken.Parse(jsonString);

//            var hroDataExtractor = new HRODataExtractor();

//            DateTime? date;
//            string resolution;
//            string resolutionInMeters;

//            hroDataExtractor.ExtractDateAndResolutionSB(tnmJson, out date, out resolution, out resolutionInMeters);

//            Assert.AreEqual(new DateTime(2004, 4, 1), date.Value);
//            Assert.AreEqual("0.6000m", resolution);
//            Assert.AreEqual("0.6000x0.6000", resolutionInMeters);
//            }

//        [Test]
//        public void HRODataExtractorSBErrorTest ()
//            {
//            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("ScienceBaseJsonHRO.txt"));
//            string jsonString = jsonStreamReader.ReadToEnd();
//            JToken tnmJson = JToken.Parse(jsonString);
//            JObject selectedWeblink = null;

//            foreach ( JObject weblink in tnmJson["webLinks"] as JArray )
//                    {
//                    string[] acceptedFormats = { "JPEG2000", "TIFF", "IMG", "Shapefile", "LAS" };
//                    if ( (weblink["title"] != null) && (acceptedFormats.Contains(weblink["title"].Value<string>())) )
//                        {
//                        selectedWeblink = weblink;
//                        break;
//                        }
//                    }
                
//            selectedWeblink["uri"] = "error";

//            var hroDataExtractor = new HRODataExtractor();

//            DateTime? date;
//            string resolution;
//            string resolutionInMeters;

//            hroDataExtractor.ExtractDateAndResolutionSB(tnmJson, out date, out resolution, out resolutionInMeters);

//            Assert.IsFalse(date.HasValue);
//            Assert.IsNull(resolution);
//            Assert.IsNull(resolutionInMeters);

//            selectedWeblink["uri"] = @"http://gisdata.usgs.gov/tdds/downloadfile.php?TYPE\u003dortho\u0026ORIG\u003dSBDDG\u0026FNAME\u003d16TFM735530_20404_0x6000m_BW_1.zip";
//            hroDataExtractor.ExtractDateAndResolutionSB(tnmJson, out date, out resolution, out resolutionInMeters);
//            Assert.IsFalse(date.HasValue);
//            Assert.AreEqual("0.6000m", resolution);
//            Assert.AreEqual("0.6000x0.6000", resolutionInMeters);
//            }
//        }
//    }
