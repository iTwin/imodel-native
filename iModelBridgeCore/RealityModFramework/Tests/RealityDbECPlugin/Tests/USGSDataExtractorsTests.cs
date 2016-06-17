using IndexECPlugin.Source.Helpers;
using Newtonsoft.Json.Linq;
using NUnit.Framework;
using System;


namespace IndexECPlugin.Tests
    {
    [TestFixture]
    class USGSDataExtractorsTests
        {

        [Test]
        public void HRODataExtractorTest ()
            {
            JToken jToken = JToken.Parse(@"{""title"":""USGS High Resolution Orthoimagery for Monterey County, CA: 10SEF910560 200606 0x5000m CL 1"",""sourceId"":""556478f6e4b0afeb7072636d"",""sourceName"":""ScienceBase"",""metaUrl"":""https://www.sciencebase.gov/catalog/item/556478f6e4b0afeb7072636d"",""lastUpdated"":""2015-05-26"",""dateCreated"":""2015-05-26"",""sizeInBytes"":22725252,""extent"":""1500 x 1500 meter"",""format"":""JPEG2000"",""downloadURL"":""http://gisdata.usgs.gov/tdds/downloadfile.php?TYPE\u003dortho\u0026ORIG\u003dSBDDG\u0026FNAME\u003d10SEF910560_200606_0x5000m_CL_1.zip"",""previewGraphicURL"":""http://tdds.cr.usgs.gov/browse/ortho/10S/EF/10SEF910560_200606_0x5000m_CL_1.jpg"",""datasets"":[""High Resolution Orthoimagery""],""boundingBox"":{""minX"":-121.981801,""maxX"":-121.965201,""minY"":36.645083,""maxY"":36.658748},""bestFitIndex"":0.0,""prettyFileSize"":""21.67 MB""}");

            HRODataExtractor extractor = new HRODataExtractor();

            string title;
            DateTime? date;
            string resolution;
            string resolutionInMeters;

            extractor.ExtractTitleDateAndResolution(jToken, out title, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("USGS High Resolution Orthoimagery for Monterey County, CA: 10SEF910560 200606 0x5000m CL 1", title, "title invalid.");
            Assert.AreEqual("2006 06", String.Format("{0:yyyy MM}", date.Value), "date invalid.");
            Assert.AreEqual("0.5000m", resolution, "resolution invalid.");
            Assert.AreEqual("0.5000x0.5000", resolutionInMeters, "resolutionInMeters invalid.");
            }

        [Test]
        public void NAIPDataExtractorTest ()
            {
            JToken jToken = JToken.Parse(@"{""title"":""FSA 10:1 NAIP Imagery m_3309603_ne_14_1_20130612_20130729 3.75 x 3.75 minute JPEG2000 from The National Map"",""sourceId"":""5500ad31e4b02419550fcb62"",""sourceName"":""ScienceBase"",""sourceOriginId"":""6916430"",""sourceOriginName"":""gda"",""metaUrl"":""https://www.sciencebase.gov/catalog/item/5500ad31e4b02419550fcb62"",""publicationDate"":""2013-07-30"",""lastUpdated"":""2015-03-26"",""dateCreated"":""2015-03-11"",""sizeInBytes"":23578651,""extent"":""3.75 x 3.75 minute"",""format"":""JPEG2000"",""downloadURL"":""ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NAIP/ok_2013/m_3309603_ne_14_1_20130612_20130729.jp2"",""datasets"":[""USDA National Agriculture Imagery Program (NAIP)""],""boundingBox"":{""minX"":-96.6875,""maxX"":-96.625,""minY"":33.9375,""maxY"":34.0},""bestFitIndex"":0.0,""prettyFileSize"":""22.49 MB""}");
            NAIPDataExtractor extractor = new NAIPDataExtractor();

            string title;
            DateTime? date;
            string resolution;
            string resolutionInMeters;

            extractor.ExtractTitleDateAndResolution(jToken, out title, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("FSA 10:1 NAIP Imagery m_3309603_ne_14_1_20130612_20130729 3.75 x 3.75 minute JPEG2000 from The National Map", title, "title invalid.");
            Assert.AreEqual("2013 07 30", String.Format("{0:yyyy MM dd}", date.Value), "date invalid.");
            Assert.AreEqual("1.0m", resolution, "resolution invalid.");
            Assert.AreEqual("1.0x1.0", resolutionInMeters, "resolutionInMeters invalid.");
            }

        [Test]
        public void NEDDataExtractorTest ()
            {
            JToken jToken = JToken.Parse(@"{""title"":""USGS NED n72w157 1 arc-second 2013 1 x 1 degree IMG"",""sourceId"":""5317689be4b093796c7b5336"",""sourceName"":""ScienceBase"",""sourceOriginId"":""6024790"",""sourceOriginName"":""gda"",""metaUrl"":""https://www.sciencebase.gov/catalog/item/5317689be4b093796c7b5336"",""publicationDate"":""2013-01-01"",""lastUpdated"":""2014-07-22"",""dateCreated"":""2014-03-05"",""sizeInBytes"":6156171,""extent"":""1 x 1 degree"",""format"":""IMG"",""downloadURL"":""ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/1/IMG/n72w157.zip"",""previewGraphicURL"":""ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/1/IMG/imgn72w157_1_thumb.jpg"",""datasets"":[""National Elevation Dataset (NED) 1 arc-second"",""National Elevation Dataset (NED)""],""boundingBox"":{""minX"":-157.0016666667,""maxX"":-155.9983333334,""minY"":70.99833333333,""maxY"":72.00166666666},""bestFitIndex"":0.0,""prettyFileSize"":""5.87 MB""}");

            NEDDataExtractor extractor = new NEDDataExtractor("National Elevation Dataset (NED) 1 arc-second");

            string title;
            DateTime? date;
            string resolution;
            string resolutionInMeters;

            extractor.ExtractTitleDateAndResolution(jToken, out title, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("USGS NED n72w157 1 arc-second 2013 1 x 1 degree IMG", title, "NED 1 arc-second title invalid.");
            Assert.AreEqual("2013 01 01", String.Format("{0:yyyy MM dd}", date.Value), "NED 1 arc-second date invalid.");
            Assert.AreEqual("1\"", resolution, "NED 1 arc-second resolution invalid.");
            Assert.AreEqual("9.7952x30.87", resolutionInMeters, "NED 1 arc-second resolutionInMeters invalid.");

            jToken = JToken.Parse(@"{""title"":""USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015"",""sourceId"":""553690bfe4b0b22a15807df2"",""sourceName"":""ScienceBase"",""sourceOriginId"":""7085222"",""sourceOriginName"":""gda"",""metaUrl"":""https://www.sciencebase.gov/catalog/item/553690bfe4b0b22a15807df2"",""publicationDate"":""2015-03-19"",""lastUpdated"":""2015-04-21"",""dateCreated"":""2015-04-21"",""sizeInBytes"":262239745,""extent"":""10000 x 10000 meter"",""format"":""IMG"",""downloadURL"":""ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/1m/IMG/USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015.zip"",""previewGraphicURL"":""ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/1m/IMG/USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015_thumb.jpg"",""datasets"":[""Digital Elevation Model (DEM) 1 meter""],""boundingBox"":{""minX"":-90.1111928012935,""maxX"":-89.9874229095346,""minY"":41.32950370684,""maxY"":41.4227313251356},""bestFitIndex"":0.0,""prettyFileSize"":""250.09 MB""}");

            extractor = new NEDDataExtractor("Digital Elevation Model (DEM) 1 meter");

            extractor.ExtractTitleDateAndResolution(jToken, out title, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015", title, "NED one meter title invalid.");
            Assert.AreEqual("2015 03 19", String.Format("{0:yyyy MM dd}", date.Value), "NED one meter date invalid.");
            Assert.AreEqual("1.0m", resolution, "NED one meter resolution invalid.");
            Assert.AreEqual("1.0x1.0", resolutionInMeters, "NED one meter resolutionInMeters invalid.");

            jToken = JToken.Parse(@"{""title"":""USGS NED 1/3 arc-second n43w078 1 x 1 degree IMG 2015"",""sourceId"":""559f5f74e4b0b94a640191b2"",""sourceName"":""ScienceBase"",""sourceOriginId"":""7114921"",""sourceOriginName"":""gda"",""metaUrl"":""https://www.sciencebase.gov/catalog/item/559f5f74e4b0b94a640191b2"",""publicationDate"":""2015-06-03"",""lastUpdated"":""2015-07-10"",""dateCreated"":""2015-07-10"",""sizeInBytes"":355611715,""extent"":""1 x 1 degree"",""format"":""IMG"",""downloadURL"":""ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/Elevation/13/IMG/USGS_NED_13_n43w078_IMG.zip"",""previewGraphicURL"":""ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/Elevation/13/IMG/USGS_NED_13_n43w078_IMG_thumb.jpg"",""datasets"":[""National Elevation Dataset (NED) 1/3 arc-second"",""National Elevation Dataset (NED)""],""boundingBox"":{""minX"":-78.0005555548943,""maxX"":-76.9994444445048,""minY"":41.9994444445064,""maxY"":43.0005555548959},""bestFitIndex"":0.0,""prettyFileSize"":""339.14 MB""}");

            extractor = new NEDDataExtractor("National Elevation Dataset (NED) 1/3 arc-second");

            extractor.ExtractTitleDateAndResolution(jToken, out title, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("USGS NED 1/3 arc-second n43w078 1 x 1 degree IMG 2015", title, "NED 1/3 title invalid.");
            Assert.AreEqual("2015 06 03", String.Format("{0:yyyy MM dd}", date.Value), "NED 1/3 date invalid.");
            Assert.AreEqual("1/3\"", resolution, "NED 1/3 resolution invalid.");
            Assert.AreEqual("7.5866x10.29", resolutionInMeters, "NED 1/3 resolutionInMeters invalid.");

            jToken = JToken.Parse(@"{""title"":""USGS NED ned19_n66x75_w145x25_ak_yukonflats_2009 1/9 arc-second 2010 15 x 15 minute IMG"",""sourceId"":""531741dae4b0cd4cd83be284"",""sourceName"":""ScienceBase"",""sourceOriginId"":""6034701"",""sourceOriginName"":""gda"",""metaUrl"":""https://www.sciencebase.gov/catalog/item/531741dae4b0cd4cd83be284"",""publicationDate"":""2010-01-01"",""lastUpdated"":""2014-07-23"",""dateCreated"":""2014-03-05"",""sizeInBytes"":17219295,""extent"":""15 x 15 minute"",""format"":""IMG"",""downloadURL"":""ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/19/IMG/ned19_n66x75_w145x25_ak_yukonflats_2009.zip"",""previewGraphicURL"":""ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/NED/19/IMG/ned19_n66x75_w145x25_ak_yukonflats_2009_thumb.jpg"",""datasets"":[""National Elevation Dataset (NED) 1/9 arc-second"",""National Elevation Dataset (NED)""],""boundingBox"":{""minX"":-145.25018518518522,""maxX"":-144.99981481481484,""minY"":66.49981481481483,""maxY"":66.7501851851852},""bestFitIndex"":0.0,""prettyFileSize"":""16.42 MB""}");

            extractor = new NEDDataExtractor("National Elevation Dataset (NED) 1/9 arc-second");

            extractor.ExtractTitleDateAndResolution(jToken, out title, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("USGS NED ned19_n66x75_w145x25_ak_yukonflats_2009 1/9 arc-second 2010 15 x 15 minute IMG", title, "NED 1/9 title invalid.");
            Assert.AreEqual("2010 01 01", String.Format("{0:yyyy MM dd}", date.Value), "NED 1/9 date invalid.");
            Assert.AreEqual("1/9\"", resolution, "NED 1/9 resolution invalid.");
            Assert.AreEqual("1.3608x3.43", resolutionInMeters, "NED 1/9 resolutionInMeters invalid.");
            }

        [Test]
        public void DefaultDataExtractorTest ()
            {
            JToken jToken = JToken.Parse(@"{""title"":""USGS Lidar Point Cloud NY FEMA-R2-Seneca 2012 18tum405880 LAS 2015"",""sourceId"":""559fa64de4b0b94a64019781"",""sourceName"":""ScienceBase"",""sourceOriginId"":""7115026"",""sourceOriginName"":""gda"",""metaUrl"":""https://www.sciencebase.gov/catalog/item/559fa64de4b0b94a64019781"",""publicationDate"":""2015-06-04"",""lastUpdated"":""2015-07-10"",""dateCreated"":""2015-07-10"",""sizeInBytes"":33591065,""extent"":""Varies"",""format"":""LAS"",""downloadURL"":""ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/Elevation/LPC/Projects/USGS_LPC_NY_FEMA_R2_Seneca_2012_LAS_2015/las/tiled/USGS_LPC_NY_FEMA_R2_Seneca_2012_18tum405880_LAS_2015.zip"",""previewGraphicURL"":""ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/Elevation/LPC/browse/USGS_LPC_NY_FEMA_R2_Seneca_2012_18tum405880_LAS_2015_thumb.jpg"",""datasets"":[""Lidar Point Cloud (LPC)""],""boundingBox"":{""minX"":-76.9362687275514,""maxX"":-76.9176588703292,""minY"":42.3279143427108,""maxY"":42.3417218212283},""bestFitIndex"":0.0,""prettyFileSize"":""32.03 MB""}");

            DefaultDataExtractor extractor = new DefaultDataExtractor();

            string title;
            DateTime? date;
            string resolution;
            string resolutionInMeters;

            extractor.ExtractTitleDateAndResolution(jToken, out title, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("USGS Lidar Point Cloud NY FEMA-R2-Seneca 2012 18tum405880 LAS 2015", title, "title invalid.");
            Assert.AreEqual("2015 06 04", String.Format("{0:yyyy MM dd}", date.Value), "date invalid.");
            Assert.AreEqual("Unknown", resolution, "resolution invalid.");
            Assert.AreEqual("Unknown", resolutionInMeters, "resolutionInMeters invalid.");
            }

        [Test]
        public void NLCDDataExtractorTest ()
            {
            JToken jToken = JToken.Parse(@"{""title"":""NLCD 2011 Land Cover Alaska, 3 x 3 Degree: NLCD2011_LC_N63W159"",""sourceId"":""553e9538e4b0a658d7938ae0"",""sourceName"":""ScienceBase"",""sourceOriginId"":""7087237"",""sourceOriginName"":""gda"",""metaUrl"":""https://www.sciencebase.gov/catalog/item/553e9538e4b0a658d7938ae0"",""publicationDate"":""2015-01-15"",""lastUpdated"":""2015-04-27"",""dateCreated"":""2015-04-27"",""sizeInBytes"":10930664,""extent"":""3 x 3 degree"",""format"":""GeoTIFF"",""downloadURL"":""http://gisdata.usgs.gov/tdds/downloadfile.php?TYPE\u003dnlcd2011_lc_3x3\u0026ORIG\u003dSBDDG\u0026FNAME\u003dNLCD2011_LC_N63W159.zip"",""previewGraphicURL"":""http://tdds.cr.usgs.gov/browse/nlcd/2011/landcover/3x3/NLCD2011_LC_N63W159.jpg"",""datasets"":[""National Land Cover Database (NLCD) - 2011""],""boundingBox"":{""minX"":-162.0,""maxX"":-159.0,""minY"":63.0,""maxY"":66.0},""bestFitIndex"":0.0,""prettyFileSize"":""10.42 MB""}");

            NLCDDataExtractor extractor = new NLCDDataExtractor("National Land Cover Database (NLCD) - 2011");

            string title;
            DateTime? date;
            string resolution;
            string resolutionInMeters;

            extractor.ExtractTitleDateAndResolution(jToken, out title, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("NLCD 2011 Land Cover Alaska, 3 x 3 Degree: NLCD2011_LC_N63W159", title, "NLCD 2011 title invalid.");
            Assert.AreEqual("2011 01 01", String.Format("{0:yyyy MM dd}", date.Value), "NLCD 2011 date invalid.");
            Assert.AreEqual("30.0m", resolution, "NLCD 2011 resolution invalid.");
            Assert.AreEqual("30.0x30.0", resolutionInMeters, "NLCD 2011 resolutionInMeters invalid.");

            jToken = JToken.Parse(@"{""title"":""NLCD 2006 Land Cover (2011 Edition, amended 2014), by State: NLCD2006_LC_Kentucky"",""sourceId"":""54ed6516e4b02d776a684915"",""sourceName"":""ScienceBase"",""sourceOriginId"":""7086247"",""sourceOriginName"":""gda"",""metaUrl"":""https://www.sciencebase.gov/catalog/item/54ed6516e4b02d776a684915"",""publicationDate"":""2014-10-10"",""lastUpdated"":""2015-04-27"",""dateCreated"":""2015-02-24"",""sizeInBytes"":44842095,""extent"":""State"",""format"":""GeoTIFF"",""downloadURL"":""http://gisdata.usgs.gov/tdds/downloadfile.php?TYPE\u003dnlcd2006_lc_state\u0026ORIG\u003dSBDDG\u0026FNAME\u003dNLCD2006_LC_Kentucky.zip"",""previewGraphicURL"":""http://tdds.cr.usgs.gov/browse/nlcd/2006/landcover/states/NLCD2006_LC_Kentucky.jpg"",""datasets"":[""National Land Cover Database (NLCD) - 2006"",""National Land Cover Database (NLCD) - 2006""],""boundingBox"":{""minX"":-89.5736770628063,""maxX"":-81.9642028810673,""minY"":36.4967193590295,""maxY"":39.1472320542701},""bestFitIndex"":0.0,""prettyFileSize"":""42.76 MB""}");

            extractor = new NLCDDataExtractor("National Land Cover Database (NLCD) - 2006");

            extractor.ExtractTitleDateAndResolution(jToken, out title, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("NLCD 2006 Land Cover (2011 Edition, amended 2014), by State: NLCD2006_LC_Kentucky", title, "NLCD 2006 title invalid.");
            Assert.AreEqual("2006 01 01", String.Format("{0:yyyy MM dd}", date.Value), "NLCD 2006 date invalid.");
            Assert.AreEqual("30.0m", resolution, "NLCD 2006 resolution invalid.");
            Assert.AreEqual("30.0x30.0", resolutionInMeters, "NLCD 2006 resolutionInMeters invalid.");

            jToken = JToken.Parse(@"{""title"":""NLCD 2001 Percent Developed Imperviousness (2011 Edition, amended 2014), by State: NLCD2001_IMP_Arizona"",""sourceId"":""54c66d84e4b043905e0199af"",""sourceName"":""ScienceBase"",""sourceOriginId"":""7084971"",""sourceOriginName"":""gda"",""metaUrl"":""https://www.sciencebase.gov/catalog/item/54c66d84e4b043905e0199af"",""publicationDate"":""2014-10-10"",""lastUpdated"":""2015-05-28"",""dateCreated"":""2015-01-26"",""sizeInBytes"":24362426,""extent"":""State"",""format"":""GeoTIFF"",""downloadURL"":""http://gisdata.usgs.gov/tdds/downloadfile.php?TYPE\u003dnlcd2001_imp_state\u0026ORIG\u003dSBDDG\u0026FNAME\u003dNLCD2001_IMP_Arizona.zip"",""previewGraphicURL"":""http://tdds.cr.usgs.gov/browse/nlcd/2001/impervious/states/NLCD2001_IMP_Arizona.jpg"",""datasets"":[""National Land Cover Database (NLCD) - 2001"",""National Land Cover Database (NLCD) - 2001""],""boundingBox"":{""minX"":-114.815414428974,""maxX"":-109.044883727819,""minY"":31.3291740411292,""maxY"":37.0045852652538},""bestFitIndex"":0.0,""prettyFileSize"":""23.23 MB""}");

            extractor = new NLCDDataExtractor("National Land Cover Database (NLCD) - 2001");

            extractor.ExtractTitleDateAndResolution(jToken, out title, out date, out resolution, out resolutionInMeters);

            Assert.AreEqual("NLCD 2001 Percent Developed Imperviousness (2011 Edition, amended 2014), by State: NLCD2001_IMP_Arizona", title, "NLCD 2001 title invalid.");
            Assert.AreEqual("2001 01 01", String.Format("{0:yyyy MM dd}", date.Value), "NLCD 2001 date invalid.");
            Assert.AreEqual("30.0m", resolution, "NLCD 2001 resolution invalid.");
            Assert.AreEqual("30.0x30.0", resolutionInMeters, "NLCD 2001 resolutionInMeters invalid.");
            }

        }
    }
