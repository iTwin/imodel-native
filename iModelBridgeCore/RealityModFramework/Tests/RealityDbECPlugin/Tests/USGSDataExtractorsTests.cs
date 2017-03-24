using IndexECPlugin.Source.Helpers;
using Newtonsoft.Json.Linq;
using NUnit.Framework;
using System;
using System.IO;
using System.Linq;
using System.Reflection;

namespace IndexECPlugin.Tests
    {
    [TestFixture]
    class USGSDataExtractorsTests
        {

        [Test]
        public void HRODataExtractorTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("TnmApiHRO.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken tnmJson = JToken.Parse(jsonString);

            var hroDataExtractor = new HRODataExtractor();

            string title;
            DateTime? date;
            string resolution;
            string resolutionInMeters;

            hroDataExtractor.ExtractTitleDateAndResolution(tnmJson, out title, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("USGS High Resolution Orthoimagery for Branch County, MI: 16TFM735530 200404 0x6000m BW 1", title);
            Assert.AreEqual(new DateTime(2004, 4, 1), date.Value);
            Assert.AreEqual("0.6000m", resolution);
            Assert.AreEqual("0.6000x0.6000", resolutionInMeters);
            }

        [Test]
        public void HRODataExtractorErrorTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("TnmApiHRO.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken tnmJson = JToken.Parse(jsonString);

            tnmJson["downloadURL"] = "error";

            var hroDataExtractor = new HRODataExtractor();

            string title;
            DateTime? date;
            string resolution;
            string resolutionInMeters;

            hroDataExtractor.ExtractTitleDateAndResolution(tnmJson, out title, out date, out resolution, out resolutionInMeters);

            Assert.IsFalse(date.HasValue);
            Assert.IsNull(resolution);
            Assert.IsNull(resolutionInMeters);

            tnmJson["downloadURL"] = @"http://gisdata.usgs.gov/tdds/downloadfile.php?TYPE\u003dortho\u0026ORIG\u003dSBDDG\u0026FNAME\u003d16TFM735530_20404_0x6000m_BW_1.zip";
            hroDataExtractor.ExtractTitleDateAndResolution(tnmJson, out title, out date, out resolution, out resolutionInMeters);
            Assert.IsFalse(date.HasValue);
            Assert.AreEqual("0.6000m", resolution);
            Assert.AreEqual("0.6000x0.6000", resolutionInMeters);
            }

        [Test]
        public void HRODataExtractorSBTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("ScienceBaseJsonHRO.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken tnmJson = JToken.Parse(jsonString);

            var hroDataExtractor = new HRODataExtractor();

            DateTime? date;
            string resolution;
            string resolutionInMeters;

            hroDataExtractor.ExtractDateAndResolutionSB(tnmJson, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual(new DateTime(2004, 4, 1), date.Value);
            Assert.AreEqual("0.6000m", resolution);
            Assert.AreEqual("0.6000x0.6000", resolutionInMeters);
            }

        [Test]
        public void HRODataExtractorSBErrorTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("ScienceBaseJsonHRO.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken tnmJson = JToken.Parse(jsonString);
            JObject selectedWeblink = null;

            foreach ( JObject weblink in tnmJson["webLinks"] as JArray )
                {
                string[] acceptedFormats = { "JPEG2000", "TIFF", "IMG", "Shapefile", "LAS" };
                if ( (weblink["title"] != null) && (acceptedFormats.Contains(weblink["title"].Value<string>())) )
                    {
                    selectedWeblink = weblink;
                    break;
                    }
                }

            selectedWeblink["uri"] = "error";

            var hroDataExtractor = new HRODataExtractor();

            DateTime? date;
            string resolution;
            string resolutionInMeters;

            hroDataExtractor.ExtractDateAndResolutionSB(tnmJson, out date, out resolution, out resolutionInMeters);

            Assert.IsFalse(date.HasValue);
            Assert.IsNull(resolution);
            Assert.IsNull(resolutionInMeters);

            selectedWeblink["uri"] = @"http://gisdata.usgs.gov/tdds/downloadfile.php?TYPE\u003dortho\u0026ORIG\u003dSBDDG\u0026FNAME\u003d16TFM735530_20404_0x6000m_BW_1.zip";
            hroDataExtractor.ExtractDateAndResolutionSB(tnmJson, out date, out resolution, out resolutionInMeters);
            Assert.IsFalse(date.HasValue);
            Assert.AreEqual("0.6000m", resolution);
            Assert.AreEqual("0.6000x0.6000", resolutionInMeters);
            }

        [Test]
        public void NAIPDataExtractorTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("TnmApiNAIP.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);
            NAIPDataExtractor extractor = new NAIPDataExtractor();

            string title;
            DateTime? date;
            string resolution;
            string resolutionInMeters;

            extractor.ExtractTitleDateAndResolution(jToken, out title, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("FSA 10:1 NAIP Imagery m_3309603_ne_14_1_20150607_20150817 3.75 x 3.75 minute JPEG2000 from The National Map", title, "title invalid.");
            Assert.AreEqual("2015 10 06", String.Format("{0:yyyy MM dd}", date.Value), "date invalid.");
            Assert.AreEqual("1.0m", resolution, "resolution invalid.");
            Assert.AreEqual("1.0x1.0", resolutionInMeters, "resolutionInMeters invalid.");

            jToken["title"] = "FSA 10:1 NAIP Imagery m_3309603_ne_14_h_20150607_20150817 3.75 x 3.75 minute JPEG2000 from The National Map";
            jToken["downloadURL"] = "https://prd-tnm.s3.amazonaws.com/StagedProducts/NAIP/ok_2015/33096/m_3309603_ne_14_h_20150607_20150817.jp2";

            extractor.ExtractTitleDateAndResolution(jToken, out title, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("FSA 10:1 NAIP Imagery m_3309603_ne_14_h_20150607_20150817 3.75 x 3.75 minute JPEG2000 from The National Map", title, "title invalid.");
            Assert.AreEqual("2015 10 06", String.Format("{0:yyyy MM dd}", date.Value), "date invalid.");
            Assert.AreEqual("0.5m", resolution, "resolution invalid.");
            Assert.AreEqual("0.5x0.5", resolutionInMeters, "resolutionInMeters invalid.");

            }

        [Test]
        public void NAIPDataExtractorErrorTest()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("TnmApiNAIP.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);
            NAIPDataExtractor extractor = new NAIPDataExtractor();

            string title;
            DateTime? date;
            string resolution;
            string resolutionInMeters;

            jToken["title"] = "FSA 10:1 NAIP Imagery m_3309603_ne_14_z_20150607_20150817 3.75 x 3.75 minute JPEG2000 from The National Map";
            jToken["downloadURL"] = "https://prd-tnm.s3.amazonaws.com/StagedProducts/NAIP/ok_2015/33096/m_3309603_ne_14_z_20150607_20150817.jp2";

            extractor.ExtractTitleDateAndResolution(jToken, out title, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("FSA 10:1 NAIP Imagery m_3309603_ne_14_z_20150607_20150817 3.75 x 3.75 minute JPEG2000 from The National Map", title, "title invalid.");
            Assert.AreEqual("2015 10 06", String.Format("{0:yyyy MM dd}", date.Value), "date invalid.");
            Assert.IsNull(resolution, "resolution invalid.");
            Assert.IsNull(resolutionInMeters, "resolutionInMeters invalid.");

            jToken["title"] = "FSA 10:1 NAIP Imagery m_3309603_ne_14 3.75 x 3.75 minute JPEG2000 from The National Map";
            jToken["downloadURL"] = "https://prd-tnm.s3.amazonaws.com/StagedProducts/NAIP/ok_2015/33096/m_3309603_ne_14.jp2";

            extractor.ExtractTitleDateAndResolution(jToken, out title, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("FSA 10:1 NAIP Imagery m_3309603_ne_14 3.75 x 3.75 minute JPEG2000 from The National Map", title, "title invalid.");
            Assert.AreEqual("2015 10 06", String.Format("{0:yyyy MM dd}", date.Value), "date invalid.");
            Assert.IsNull(resolution, "resolution invalid.");
            Assert.IsNull(resolutionInMeters, "resolutionInMeters invalid.");

            jToken["publicationDate"] = "error";

            extractor.ExtractTitleDateAndResolution(jToken, out title, out date, out resolution, out resolutionInMeters);

            Assert.IsFalse(date.HasValue);
            Assert.IsNull(resolution, "resolution invalid.");
            Assert.IsNull(resolutionInMeters, "resolutionInMeters invalid.");
            }

        [Test]
        public void NAIPDataExtractorSBTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("ScienceBaseJsonNAIP.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);
            NAIPDataExtractor extractor = new NAIPDataExtractor();

            DateTime? date;
            string resolution;
            string resolutionInMeters;

            extractor.ExtractDateAndResolutionSB(jToken, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("2015 10 06", String.Format("{0:yyyy MM dd}", date.Value), "date invalid.");
            Assert.AreEqual("1.0m", resolution, "resolution invalid.");
            Assert.AreEqual("1.0x1.0", resolutionInMeters, "resolutionInMeters invalid.");

            JObject selectedWeblink = null;

            foreach ( JObject weblink in jToken["webLinks"] as JArray )
                {
                string[] acceptedFormats = { "JPEG2000", "TIFF", "IMG", "Shapefile", "LAS" };
                if ( (weblink["title"] != null) && (acceptedFormats.Contains(weblink["title"].Value<string>())) )
                    {
                    selectedWeblink = weblink;
                    break;
                    }
                }

            selectedWeblink["uri"] = "https://prd-tnm.s3.amazonaws.com/StagedProducts/NAIP/ok_2015/33096/m_3309603_ne_14_h_20150607_20150817.jp2";

            extractor.ExtractDateAndResolutionSB(jToken, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("2015 10 06", String.Format("{0:yyyy MM dd}", date.Value), "date invalid.");
            Assert.AreEqual("0.5m", resolution, "resolution invalid.");
            Assert.AreEqual("0.5x0.5", resolutionInMeters, "resolutionInMeters invalid.");

            JToken datesToken = jToken["dates"];
            JToken pubDateToken = null;
            if ( datesToken != null )
                {
                pubDateToken = datesToken.FirstOrDefault(d => d["type"].Value<string>() == "Publication");
                }

            pubDateToken["dateString"] = "2016";

            extractor.ExtractDateAndResolutionSB(jToken, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("2016 01 01", String.Format("{0:yyyy MM dd}", date.Value), "date invalid.");
            Assert.AreEqual("0.5m", resolution, "resolution invalid.");
            Assert.AreEqual("0.5x0.5", resolutionInMeters, "resolutionInMeters invalid.");

            pubDateToken["dateString"] = "2016-07";

            extractor.ExtractDateAndResolutionSB(jToken, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("2016 07 01", String.Format("{0:yyyy MM dd}", date.Value), "date invalid.");
            Assert.AreEqual("0.5m", resolution, "resolution invalid.");
            Assert.AreEqual("0.5x0.5", resolutionInMeters, "resolutionInMeters invalid.");

            pubDateToken["dateString"] = "2016-07-10";

            extractor.ExtractDateAndResolutionSB(jToken, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("2016 07 10", String.Format("{0:yyyy MM dd}", date.Value), "date invalid.");
            Assert.AreEqual("0.5m", resolution, "resolution invalid.");
            Assert.AreEqual("0.5x0.5", resolutionInMeters, "resolutionInMeters invalid.");
            }

        [Test]
        public void NAIPDataExtractorSBErrorTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("ScienceBaseJsonNAIP.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);
            NAIPDataExtractor extractor = new NAIPDataExtractor();

            DateTime? date;
            string resolution;
            string resolutionInMeters;

            JObject selectedWeblink = null;

            foreach ( JObject weblink in jToken["webLinks"] as JArray )
                {
                string[] acceptedFormats = { "JPEG2000", "TIFF", "IMG", "Shapefile", "LAS" };
                if ( (weblink["title"] != null) && (acceptedFormats.Contains(weblink["title"].Value<string>())) )
                    {
                    selectedWeblink = weblink;
                    break;
                    }
                }

            selectedWeblink["uri"] = "https://prd-tnm.s3.amazonaws.com/StagedProducts/NAIP/ok_2015/33096/m_3309603_ne_14_z_20150607_20150817.jp2";

            extractor.ExtractDateAndResolutionSB(jToken, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("2015 10 06", String.Format("{0:yyyy MM dd}", date.Value), "date invalid.");
            Assert.IsNull(resolution, "resolution invalid.");
            Assert.IsNull(resolutionInMeters, "resolutionInMeters invalid.");

            selectedWeblink["uri"] = "https://prd-tnm.s3.amazonaws.com/StagedProducts/NAIP/ok_2015/33096/m_3309603_ne_14.jp2";

            extractor.ExtractDateAndResolutionSB(jToken, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("2015 10 06", String.Format("{0:yyyy MM dd}", date.Value), "date invalid.");
            Assert.IsNull(resolution, "resolution invalid.");
            Assert.IsNull(resolutionInMeters, "resolutionInMeters invalid.");

            JToken datesToken = jToken["dates"];
            JToken pubDateToken = null;
            if ( datesToken != null )
                {
                pubDateToken = datesToken.FirstOrDefault(d => d["type"].Value<string>() == "Publication");
                }

            pubDateToken["dateString"] = "20x6";

            extractor.ExtractDateAndResolutionSB(jToken, out date, out resolution, out resolutionInMeters);

            Assert.IsFalse(date.HasValue);
            Assert.IsNull(resolution, "resolution invalid.");
            Assert.IsNull(resolutionInMeters, "resolutionInMeters invalid.");

            datesToken = jToken["dates"];
            pubDateToken = null;
            if ( datesToken != null )
                {
                pubDateToken = datesToken.FirstOrDefault(d => d["type"].Value<string>() == "Publication");
                }

            pubDateToken["dateString"] = "error";

            extractor.ExtractDateAndResolutionSB(jToken, out date, out resolution, out resolutionInMeters);

            Assert.IsFalse(date.HasValue);
            Assert.IsNull(resolution, "resolution invalid.");
            Assert.IsNull(resolutionInMeters, "resolutionInMeters invalid.");

            }

        [Test]
        public void NEDDataExtractor1mTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("TnmApiNED1m.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);

            string title;
            DateTime? date;
            string resolution;
            string resolutionInMeters;

            NEDDataExtractor extractor = new NEDDataExtractor("Digital Elevation Model (DEM) 1 meter");

            extractor.ExtractTitleDateAndResolution(jToken, out title, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015", title, "NED one meter title invalid.");
            Assert.AreEqual("2015 03 19", String.Format("{0:yyyy MM dd}", date.Value), "NED one meter date invalid.");
            Assert.AreEqual("1.0m", resolution, "NED one meter resolution invalid.");
            Assert.AreEqual("1.0x1.0", resolutionInMeters, "NED one meter resolutionInMeters invalid.");
            }

        [Test]
        public void NEDDataExtractorDateErrorTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("TnmApiNED1m.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);

            string title;
            DateTime? date;
            string resolution;
            string resolutionInMeters;

            jToken["publicationDate"] = "error";

            NEDDataExtractor extractor = new NEDDataExtractor("Digital Elevation Model (DEM) 1 meter");

            extractor.ExtractTitleDateAndResolution(jToken, out title, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015", title, "NED one meter title invalid.");
            Assert.IsNull(date, "NED one meter date invalid.");
            Assert.AreEqual("1.0m", resolution, "NED one meter resolution invalid.");
            Assert.AreEqual("1.0x1.0", resolutionInMeters, "NED one meter resolutionInMeters invalid.");
            }

        [Test]
        public void NEDDataExtractorDefaultTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("TnmApiNED1m.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);

            string title;
            DateTime? date;
            string resolution;
            string resolutionInMeters;

            NEDDataExtractor extractor = new NEDDataExtractor("Anything");

            extractor.ExtractTitleDateAndResolution(jToken, out title, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015", title, "NED one meter title invalid.");
            Assert.AreEqual("2015 03 19", String.Format("{0:yyyy MM dd}", date.Value), "NED one meter date invalid.");
            Assert.IsNull(resolution, "NED one meter resolution invalid.");
            Assert.IsNull(resolutionInMeters, "NED one meter resolutionInMeters invalid.");
            }

        [Test]
        public void NEDDataExtractorSB1mTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("ScienceBaseJsonNED1m.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);

            DateTime? date;
            string resolution;
            string resolutionInMeters;

            NEDDataExtractor extractor = new NEDDataExtractor("Digital Elevation Model (DEM) 1 meter");

            extractor.ExtractDateAndResolutionSB(jToken, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("2015 03 19", String.Format("{0:yyyy MM dd}", date.Value), "NED one meter date invalid.");
            Assert.AreEqual("1.0m", resolution, "NED one meter resolution invalid.");
            Assert.AreEqual("1.0x1.0", resolutionInMeters, "NED one meter resolutionInMeters invalid.");
            }

        [Test]
        public void NEDDataExtractorSBDateTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("ScienceBaseJsonNED1m.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);

            DateTime? date;
            string resolution;
            string resolutionInMeters;

            JToken datesToken = jToken["dates"];
            JToken pubDateToken = null;
            if ( datesToken != null )
                {
                pubDateToken = datesToken.FirstOrDefault(d => d["type"].Value<string>() == "Publication");
                }

            pubDateToken["dateString"] = "2016";

            NEDDataExtractor extractor = new NEDDataExtractor("Digital Elevation Model (DEM) 1 meter");

            extractor.ExtractDateAndResolutionSB(jToken, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("2016 01 01", String.Format("{0:yyyy MM dd}", date.Value), "NED one meter date invalid.");
            Assert.AreEqual("1.0m", resolution, "NED one meter resolution invalid.");
            Assert.AreEqual("1.0x1.0", resolutionInMeters, "NED one meter resolutionInMeters invalid.");

            pubDateToken["dateString"] = "2016-07";

            extractor.ExtractDateAndResolutionSB(jToken, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("2016 07 01", String.Format("{0:yyyy MM dd}", date.Value), "NED one meter date invalid.");
            Assert.AreEqual("1.0m", resolution, "NED one meter resolution invalid.");
            Assert.AreEqual("1.0x1.0", resolutionInMeters, "NED one meter resolutionInMeters invalid.");

            pubDateToken["dateString"] = "2016-07-10";

            extractor.ExtractDateAndResolutionSB(jToken, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("2016 07 10", String.Format("{0:yyyy MM dd}", date.Value), "NED one meter date invalid.");
            Assert.AreEqual("1.0m", resolution, "NED one meter resolution invalid.");
            Assert.AreEqual("1.0x1.0", resolutionInMeters, "NED one meter resolutionInMeters invalid.");
            }

        [Test]
        public void NEDDataExtractorSBDateErrorTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("ScienceBaseJsonNED1m.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);

            DateTime? date;
            string resolution;
            string resolutionInMeters;

            JToken datesToken = jToken["dates"];
            JToken pubDateToken = null;
            if ( datesToken != null )
                {
                pubDateToken = datesToken.FirstOrDefault(d => d["type"].Value<string>() == "Publication");
                }

            pubDateToken["dateString"] = "20x6";

            NEDDataExtractor extractor = new NEDDataExtractor("Digital Elevation Model (DEM) 1 meter");

            extractor.ExtractDateAndResolutionSB(jToken, out date, out resolution, out resolutionInMeters);

            Assert.IsNull(date, "NED one meter date invalid.");
            Assert.AreEqual("1.0m", resolution, "NED one meter resolution invalid.");
            Assert.AreEqual("1.0x1.0", resolutionInMeters, "NED one meter resolutionInMeters invalid.");

            pubDateToken["dateString"] = "error";
            
            extractor.ExtractDateAndResolutionSB(jToken, out date, out resolution, out resolutionInMeters);

            Assert.IsNull(date, "NED one meter date invalid.");
            Assert.AreEqual("1.0m", resolution, "NED one meter resolution invalid.");
            Assert.AreEqual("1.0x1.0", resolutionInMeters, "NED one meter resolutionInMeters invalid.");
            }

        [Test]
        public void NEDDataExtractor1asTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("TnmApiNED1as.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);

            string title;
            DateTime? date;
            string resolution;
            string resolutionInMeters;

            NEDDataExtractor extractor = new NEDDataExtractor("National Elevation Dataset (NED) 1 arc-second");

            extractor.ExtractTitleDateAndResolution(jToken, out title, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("USGS NED n72w157 1 arc-second 2013 1 x 1 degree IMG", title, "NED 1 arc-second title invalid.");
            Assert.AreEqual("2013 01 01", String.Format("{0:yyyy MM dd}", date.Value), "NED 1 arc-second date invalid.");
            Assert.AreEqual("1\"", resolution, "NED 1 arc-second resolution invalid.");
            Assert.AreEqual("9.7952x30.87", resolutionInMeters, "NED 1 arc-second resolutionInMeters invalid.");

            }

        [Test]
        public void NEDDataExtractorSB1asTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("ScienceBaseJsonNED1as.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);

            DateTime? date;
            string resolution;
            string resolutionInMeters;

            NEDDataExtractor extractor = new NEDDataExtractor("National Elevation Dataset (NED) 1 arc-second");

            extractor.ExtractDateAndResolutionSB(jToken, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("2013 01 01", String.Format("{0:yyyy MM dd}", date.Value), "NED 1 arc-second date invalid.");
            Assert.AreEqual("1\"", resolution, "NED 1 arc-second resolution invalid.");
            Assert.AreEqual("9.7952x30.87", resolutionInMeters, "NED 1 arc-second resolutionInMeters invalid.");

            }

        [Test]
        public void NEDDataExtractor1_3asTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("TnmApiNED1_3as.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);

            string title;
            DateTime? date;
            string resolution;
            string resolutionInMeters;

            NEDDataExtractor extractor = new NEDDataExtractor("National Elevation Dataset (NED) 1/3 arc-second");

            extractor.ExtractTitleDateAndResolution(jToken, out title, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("USGS NED 1/3 arc-second n43w078 1 x 1 degree IMG 2015", title, "NED 1/3 title invalid.");
            Assert.AreEqual("2015 06 03", String.Format("{0:yyyy MM dd}", date.Value), "NED 1/3 date invalid.");
            Assert.AreEqual("1/3\"", resolution, "NED 1/3 resolution invalid.");
            Assert.AreEqual("7.5866x10.29", resolutionInMeters, "NED 1/3 resolutionInMeters invalid.");
            }

        [Test]
        public void NEDDataExtractorSB1_3asTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("ScienceBaseJsonNED1_3as.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);

            DateTime? date;
            string resolution;
            string resolutionInMeters;

            NEDDataExtractor extractor = new NEDDataExtractor("National Elevation Dataset (NED) 1/3 arc-second");

            extractor.ExtractDateAndResolutionSB(jToken, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("2015 06 03", String.Format("{0:yyyy MM dd}", date.Value), "NED 1/3 date invalid.");
            Assert.AreEqual("1/3\"", resolution, "NED 1/3 resolution invalid.");
            Assert.AreEqual("7.5866x10.29", resolutionInMeters, "NED 1/3 resolutionInMeters invalid.");
            }

        [Test]
        public void NEDDataExtractor1_9asTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("TnmApiNED1_9as.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);

            string title;
            DateTime? date;
            string resolution;
            string resolutionInMeters;

            NEDDataExtractor extractor = new NEDDataExtractor("National Elevation Dataset (NED) 1/9 arc-second");

            extractor.ExtractTitleDateAndResolution(jToken, out title, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("USGS NED ned19_n66x75_w145x25_ak_yukonflats_2009 1/9 arc-second 2010 15 x 15 minute IMG", title, "NED 1/9 title invalid.");
            Assert.AreEqual("2010 01 01", String.Format("{0:yyyy MM dd}", date.Value), "NED 1/9 date invalid.");
            Assert.AreEqual("1/9\"", resolution, "NED 1/9 resolution invalid.");
            Assert.AreEqual("1.3608x3.43", resolutionInMeters, "NED 1/9 resolutionInMeters invalid.");
            }

        [Test]
        public void NEDDataExtractorSB1_9asTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("ScienceBaseJsonNED1_9as.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);

            DateTime? date;
            string resolution;
            string resolutionInMeters;

            NEDDataExtractor extractor = new NEDDataExtractor("National Elevation Dataset (NED) 1/9 arc-second");

            extractor.ExtractDateAndResolutionSB(jToken, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("2010 01 01", String.Format("{0:yyyy MM dd}", date.Value), "NED 1/9 date invalid.");
            Assert.AreEqual("1/9\"", resolution, "NED 1/9 resolution invalid.");
            Assert.AreEqual("1.3608x3.43", resolutionInMeters, "NED 1/9 resolutionInMeters invalid.");
            }

        [Test]
        public void NEDDataExtractorSBDefaultTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("ScienceBaseJsonNED1m.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);

            DateTime? date;
            string resolution;
            string resolutionInMeters;

            NEDDataExtractor extractor = new NEDDataExtractor("Anything");

            extractor.ExtractDateAndResolutionSB(jToken, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("2015 03 19", String.Format("{0:yyyy MM dd}", date.Value), "NED one meter date invalid.");
            Assert.IsNull(resolution, "NED one meter resolution invalid.");
            Assert.IsNull(resolutionInMeters, "NED one meter resolutionInMeters invalid.");
            }

        [Test]
        public void DefaultDataExtractorTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("TnmApiLIDAR.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);

            DefaultDataExtractor extractor = new DefaultDataExtractor();

            string title;
            DateTime? date;
            string resolution;
            string resolutionInMeters;

            extractor.ExtractTitleDateAndResolution(jToken, out title, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("USGS Lidar Point Cloud NY FEMA-R2-Seneca 2012 18tum405880 LAS 2015", title, "title invalid.");
            Assert.AreEqual("2015 06 04", String.Format("{0:yyyy MM dd}", date.Value), "date invalid.");
            Assert.IsNull(resolution);
            Assert.IsNull(resolutionInMeters);
            }

        [Test]
        public void DefaultDataExtractorDateErrorTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("TnmApiLIDAR.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);

            jToken["publicationDate"] = "error";

            DefaultDataExtractor extractor = new DefaultDataExtractor();

            string title;
            DateTime? date;
            string resolution;
            string resolutionInMeters;

            extractor.ExtractTitleDateAndResolution(jToken, out title, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("USGS Lidar Point Cloud NY FEMA-R2-Seneca 2012 18tum405880 LAS 2015", title, "title invalid.");
            Assert.IsNull(date, "date invalid.");
            Assert.IsNull(resolution);
            Assert.IsNull(resolutionInMeters);
            }

        [Test]
        public void DefaultDataExtractorSBTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("ScienceBaseJsonLIDAR.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);

            DefaultDataExtractor extractor = new DefaultDataExtractor();

            DateTime? date;
            string resolution;
            string resolutionInMeters;

            extractor.ExtractDateAndResolutionSB(jToken, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("2015 06 04", String.Format("{0:yyyy MM dd}", date.Value), "date invalid.");
            Assert.IsNull(resolution);
            Assert.IsNull(resolutionInMeters);
            }

        [Test]
        public void DefaultDataExtractorSBDateTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("ScienceBaseJsonLIDAR.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);

            DefaultDataExtractor extractor = new DefaultDataExtractor();

            DateTime? date;
            string resolution;
            string resolutionInMeters;

            JToken datesToken = jToken["dates"];
            JToken pubDateToken = null;
            if ( datesToken != null )
                {
                pubDateToken = datesToken.FirstOrDefault(d => d["type"].Value<string>() == "Publication");
                }

            pubDateToken["dateString"] = "2016";

            extractor.ExtractDateAndResolutionSB(jToken, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("2016 01 01", String.Format("{0:yyyy MM dd}", date.Value), "date invalid.");
            Assert.IsNull(resolution);
            Assert.IsNull(resolutionInMeters);

            pubDateToken["dateString"] = "2016-07";

            extractor.ExtractDateAndResolutionSB(jToken, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("2016 07 01", String.Format("{0:yyyy MM dd}", date.Value), "date invalid.");
            Assert.IsNull(resolution);
            Assert.IsNull(resolutionInMeters);

            pubDateToken["dateString"] = "2016-07-10";

            extractor.ExtractDateAndResolutionSB(jToken, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("2016 07 10", String.Format("{0:yyyy MM dd}", date.Value), "date invalid.");
            Assert.IsNull(resolution);
            Assert.IsNull(resolutionInMeters);
            }

        [Test]
        public void DefaultDataExtractorSBDateErrorTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("ScienceBaseJsonLIDAR.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);

            DefaultDataExtractor extractor = new DefaultDataExtractor();

            DateTime? date;
            string resolution;
            string resolutionInMeters;

            JToken datesToken = jToken["dates"];
            JToken pubDateToken = null;
            if ( datesToken != null )
                {
                pubDateToken = datesToken.FirstOrDefault(d => d["type"].Value<string>() == "Publication");
                }

            pubDateToken["dateString"] = "20x6";

            extractor.ExtractDateAndResolutionSB(jToken, out date, out resolution, out resolutionInMeters);

            Assert.IsNull(date, "date invalid.");
            Assert.IsNull(resolution);
            Assert.IsNull(resolutionInMeters);

            pubDateToken["dateString"] = "error";

            extractor.ExtractDateAndResolutionSB(jToken, out date, out resolution, out resolutionInMeters);

            Assert.IsNull(date, "date invalid.");
            Assert.IsNull(resolution);
            Assert.IsNull(resolutionInMeters);
            }

        [Test]
        public void NLCD2011DataExtractorTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("TnmApiNLCD2011.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);

            NLCDDataExtractor extractor = new NLCDDataExtractor("National Land Cover Database (NLCD) - 2011");

            string title;
            DateTime? date;
            string resolution;
            string resolutionInMeters;

            extractor.ExtractTitleDateAndResolution(jToken, out title, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("NLCD 2011 Land Cover (2011 Edition, amended 2014), by State: NLCD2011_LC_Kentucky", title, "NLCD 2011 title invalid.");
            Assert.AreEqual("2011 01 01", String.Format("{0:yyyy MM dd}", date.Value), "NLCD 2011 date invalid.");
            Assert.AreEqual("30.0m", resolution, "NLCD 2011 resolution invalid.");
            Assert.AreEqual("30.0x30.0", resolutionInMeters, "NLCD 2011 resolutionInMeters invalid.");

            }

        [Test]
        public void NLCD2011DataExtractorSBTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("ScienceBaseJsonNLCD2011.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);

            NLCDDataExtractor extractor = new NLCDDataExtractor("National Land Cover Database (NLCD) - 2011");

            DateTime? date;
            string resolution;
            string resolutionInMeters;

            extractor.ExtractDateAndResolutionSB(jToken, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("2011 01 01", String.Format("{0:yyyy MM dd}", date.Value), "NLCD 2011 date invalid.");
            Assert.AreEqual("30.0m", resolution, "NLCD 2011 resolution invalid.");
            Assert.AreEqual("30.0x30.0", resolutionInMeters, "NLCD 2011 resolutionInMeters invalid.");

            }

        [Test]
        public void NLCD2006DataExtractorTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("TnmApiNLCD2006.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);

            NLCDDataExtractor extractor = new NLCDDataExtractor("National Land Cover Database (NLCD) - 2006");

            string title;
            DateTime? date;
            string resolution;
            string resolutionInMeters;

            extractor.ExtractTitleDateAndResolution(jToken, out title, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("NLCD 2006 Land Cover (2011 Edition, amended 2014), by State: NLCD2006_LC_Arizona", title, "NLCD 2006 title invalid.");
            Assert.AreEqual("2006 01 01", String.Format("{0:yyyy MM dd}", date.Value), "NLCD 2006 date invalid.");
            Assert.AreEqual("30.0m", resolution, "NLCD 2006 resolution invalid.");
            Assert.AreEqual("30.0x30.0", resolutionInMeters, "NLCD 2006 resolutionInMeters invalid.");
            }

        [Test]
        public void NLCD2006DataExtractorSBTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("ScienceBaseJsonNLCD2006.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);

            NLCDDataExtractor extractor = new NLCDDataExtractor("National Land Cover Database (NLCD) - 2006");

            DateTime? date;
            string resolution;
            string resolutionInMeters;

            extractor.ExtractDateAndResolutionSB(jToken, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("2006 01 01", String.Format("{0:yyyy MM dd}", date.Value), "NLCD 2006 date invalid.");
            Assert.AreEqual("30.0m", resolution, "NLCD 2006 resolution invalid.");
            Assert.AreEqual("30.0x30.0", resolutionInMeters, "NLCD 2006 resolutionInMeters invalid.");
            }

        [Test]
        public void NLCD2001DataExtractorTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("TnmApiNLCD2001.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);

            NLCDDataExtractor extractor = new NLCDDataExtractor("National Land Cover Database (NLCD) - 2001");

            string title;
            DateTime? date;
            string resolution;
            string resolutionInMeters;

            extractor.ExtractTitleDateAndResolution(jToken, out title, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("NLCD 2001 Land Cover (2011 Edition, amended 2014), by State: NLCD2001_LC_Arizona", title, "NLCD 2001 title invalid.");
            Assert.AreEqual("2001 01 01", String.Format("{0:yyyy MM dd}", date.Value), "NLCD 2001 date invalid.");
            Assert.AreEqual("30.0m", resolution, "NLCD 2001 resolution invalid.");
            Assert.AreEqual("30.0x30.0", resolutionInMeters, "NLCD 2001 resolutionInMeters invalid.");
            }

        [Test]
        public void NLCD2001DataExtractorSBTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("ScienceBaseJsonNLCD2001.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);

            NLCDDataExtractor extractor = new NLCDDataExtractor("National Land Cover Database (NLCD) - 2001");

            DateTime? date;
            string resolution;
            string resolutionInMeters;

            extractor.ExtractDateAndResolutionSB(jToken, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("2001 01 01", String.Format("{0:yyyy MM dd}", date.Value), "NLCD 2001 date invalid.");
            Assert.AreEqual("30.0m", resolution, "NLCD 2001 resolution invalid.");
            Assert.AreEqual("30.0x30.0", resolutionInMeters, "NLCD 2001 resolutionInMeters invalid.");
            }

        [Test]
        public void NLCDDefaultDataExtractorTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("TnmApiNLCD2011.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);

            NLCDDataExtractor extractor = new NLCDDataExtractor("Anything");

            string title;
            DateTime? date;
            string resolution;
            string resolutionInMeters;

            extractor.ExtractTitleDateAndResolution(jToken, out title, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("NLCD 2011 Land Cover (2011 Edition, amended 2014), by State: NLCD2011_LC_Kentucky", title, "NLCD 2011 title invalid.");
            Assert.IsNull(date, "Default NLCD date invalid.");
            Assert.AreEqual("30.0m", resolution, "Default NLCD resolution invalid.");
            Assert.AreEqual("30.0x30.0", resolutionInMeters, "Default NLCD resolutionInMeters invalid.");
            }

        [Test]
        public void NLCDDefaultDataExtractorSBTest ()
            {
            var jsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("ScienceBaseJsonNLCD2011.txt"));
            string jsonString = jsonStreamReader.ReadToEnd();
            JToken jToken = JToken.Parse(jsonString);

            NLCDDataExtractor extractor = new NLCDDataExtractor("Anything");

            DateTime? date;
            string resolution;
            string resolutionInMeters;

            extractor.ExtractDateAndResolutionSB(jToken, out date, out resolution, out resolutionInMeters);

            Assert.IsNull(date, "Default NLCD date invalid.");
            Assert.AreEqual("30.0m", resolution, "Default NLCD resolution invalid.");
            Assert.AreEqual("30.0x30.0", resolutionInMeters, "Default NLCD resolutionInMeters invalid.");
            }
        }
    }
