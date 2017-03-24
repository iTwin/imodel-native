/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/Helpers/IUSGSDataExtractor.cs $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/

using Bentley.EC.Persistence.Operations;
using Bentley.Exceptions;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IndexECPlugin.Source.Helpers
    {
    /// <summary>
    /// Interface for USGSDataExtractors, used to extract data from the USGS API datasets
    /// </summary>
    public interface IUSGSDataExtractor
        {
        /// <summary>
        /// Extract data from the USGS API Json token
        /// </summary>
        /// <param name="token">The json token obtained from the USGS API</param>
        /// <param name="title">The title extracted from the token</param>
        /// <param name="date">The date of publication of the data</param>
        /// <param name="resolution">The resolution of the data, if applicable</param>
        /// <param name="resolutionInMeters">The resolution in meters of the data, if applicable</param>
        void ExtractTitleDateAndResolution (JToken token, out string title, out DateTime? date, out string resolution, out string resolutionInMeters);

        /// <summary>
        /// Extract data from the USGS ScienceBase Json token
        /// </summary>
        /// <param name="token">The json token obtained from ScienceBase</param>
        /// <param name="date">The date of publication of the data</param>
        /// <param name="resolution">The resolution of the data, if applicable</param>
        /// <param name="resolutionInMeters">The resolution in meters of the data, if applicable</param>
        void ExtractDateAndResolutionSB (JToken token, out DateTime? date, out string resolution, out string resolutionInMeters);
        }

    /// <summary>
    /// This class is used to extract data from the USGS API High Resolution Orthoimagery results
    /// </summary>
    public class HRODataExtractor : IUSGSDataExtractor
        {
        /// <summary>
        /// HRODataExtractor constructor
        /// </summary>
        public HRODataExtractor ()
            {
            //Does nothing
            }

        /// <summary>
        /// Extract data from the USGS API Json token
        /// </summary>
        /// <param name="token">The json token obtained from the USGS API</param>
        /// <param name="title">The title extracted from the token</param>
        /// <param name="date">The date of publication of the data</param>
        /// <param name="resolution">The resolution of the data, if applicable</param>
        /// <param name="resolutionInMeters">The resolution in meters of the data, if applicable</param>
        public void ExtractTitleDateAndResolution (JToken token, out string title, out DateTime? date, out string resolution, out string resolutionInMeters)
            {
            title = token.TryToGetString("title");

            string newTokenZipURL = token.TryToGetString("downloadURL");
            string[] newTokenZipURLSplit = newTokenZipURL.Split('/', '\\');

            //The zip name is the last string in the array
            string zipName = newTokenZipURLSplit[newTokenZipURLSplit.Length - 1];

            string[] zipNameSplit = zipName.Split('_');

            if ( zipNameSplit.Count() < 3 )
                {
                resolution = null;
                resolutionInMeters = null;
                date = null;
                }
            else
                {
                //According to the convention, the date of acquisition is located after the first underscore character
                string newDateString = zipNameSplit[1];

                resolution = zipNameSplit[2].Replace('x', '.');

                resolutionInMeters = resolution.TrimEnd('m');
                resolutionInMeters = resolutionInMeters + "x" + resolutionInMeters;

                try
                    {
                    date = DateTime.ParseExact(newDateString + "01", "yyyyMMdd", System.Globalization.CultureInfo.InvariantCulture);
                    }
                catch ( System.FormatException )
                    {
                    Log.Logger.error(String.Format("Error while filtering USGS High Resolution Orthoimagery entry {0}.", title));
                    date = null;
                    }
                }
            }

        /// <summary>
        /// Extract data from the USGS ScienceBase Json token
        /// </summary>
        /// <param name="token">The json token obtained from ScienceBase</param>
        /// <param name="date">The date of publication of the data</param>
        /// <param name="resolution">The resolution of the data, if applicable</param>
        /// <param name="resolutionInMeters">The resolution in meters of the data, if applicable</param>
        public void ExtractDateAndResolutionSB (JToken token, out DateTime? date, out string resolution, out string resolutionInMeters)
            {
            string title = token.TryToGetString("title");

            JArray weblinks = token["webLinks"] as JArray;
            string newTokenZipURL = null;

            if ( weblinks != null )
                {
                foreach ( JObject weblink in weblinks )
                    {

                    string[] acceptedFormats = { "JPEG2000", "TIFF", "IMG", "Shapefile", "LAS" };
                    if ( (weblink["title"] != null) && (acceptedFormats.Contains(weblink["title"].Value<string>())) )
                        {
                        newTokenZipURL = weblink.TryToGetString("uri");
                        break;
                        }
                    }
                }

            string[] newTokenZipURLSplit = newTokenZipURL.Split('/', '\\');

            //The zip name is the last string in the array
            string zipName = newTokenZipURLSplit[newTokenZipURLSplit.Length - 1];

            string[] zipNameSplit = zipName.Split('_');

            if ( zipNameSplit.Count() < 3 )
                {
                resolution = null;
                resolutionInMeters = null;
                date = null;
                }
            else
                {
                //According to the convention, the date of acquisition is located after the first underscore character
                string newDateString = zipNameSplit[1];

                resolution = zipNameSplit[2].Replace('x', '.');

                resolutionInMeters = resolution.TrimEnd('m');
                resolutionInMeters = resolutionInMeters + "x" + resolutionInMeters;

                try
                    {
                    date = DateTime.ParseExact(newDateString + "01", "yyyyMMdd", System.Globalization.CultureInfo.InvariantCulture);
                    }
                catch ( System.FormatException )
                    {
                    Log.Logger.error(String.Format("Error while filtering USGS High Resolution Orthoimagery entry {0}.", title));
                    date = null;
                    }
                }
            }
        }

    /// <summary>
    /// This class is used to extract data from the USGS API National Agriculture Imagery Program
    /// </summary>
    public class NAIPDataExtractor : IUSGSDataExtractor
        {
        /// <summary>
        /// NAIPDataExtractor constructor
        /// </summary>
        public NAIPDataExtractor ()
            {
            //Does nothing
            }

        /// <summary>
        /// Extract data from the USGS API Json token
        /// </summary>
        /// <param name="token">The json token obtained from the USGS API</param>
        /// <param name="title">The title extracted from the token</param>
        /// <param name="date">The date of publication of the data</param>
        /// <param name="resolution">The resolution of the data, if applicable</param>
        /// <param name="resolutionInMeters">The resolution in meters of the data, if applicable</param>
        public void ExtractTitleDateAndResolution (JToken token, out string title, out DateTime? date, out string resolution, out string resolutionInMeters)
            {
            title = token.TryToGetString("title");

            string newTokenZipURL = token.TryToGetString("downloadURL");
            string[] newTokenZipURLSplit = newTokenZipURL.Split('/', '\\');

            //The zip name is the last string in the array
            string fileName = newTokenZipURLSplit[newTokenZipURLSplit.Length - 1];

            string[] fileNameSplit = fileName.Split('_');

            if ( fileNameSplit.Count() < 5 )
                {
                resolution = null;
                resolutionInMeters = null;
                }
            else
                {
                string newTokenResolutionChar = fileNameSplit[4];
                if ( newTokenResolutionChar == "1" )
                    {
                    resolution = "1.0m";
                    resolutionInMeters = "1.0x1.0";
                    }
                else
                    {
                    if ( newTokenResolutionChar == "h" )
                        {
                        resolution = "0.5m";
                        resolutionInMeters = "0.5x0.5";
                        }
                    else
                        {
                        resolution = null;
                        resolutionInMeters = null;
                        }
                    }
                }
            string dateString = token.TryToGetString("publicationDate");
            date = null;
            if ( dateString != null )
                {

                try
                    {
                    date = DateTime.ParseExact(dateString, "yyyy-MM-dd", System.Globalization.CultureInfo.InvariantCulture);
                    }
                catch ( FormatException )
                    {
                    Log.Logger.error(String.Format("Error while filtering USGS National Agriculture Imagery Program entry {0}.", title));
                    date = null;
                    }
                }

            }

        /// <summary>
        /// Extract data from the USGS ScienceBase Json token
        /// </summary>
        /// <param name="token">The json token obtained from ScienceBase</param>
        /// <param name="date">The date of publication of the data</param>
        /// <param name="resolution">The resolution of the data, if applicable</param>
        /// <param name="resolutionInMeters">The resolution in meters of the data, if applicable</param>
        public void ExtractDateAndResolutionSB (JToken token, out DateTime? date, out string resolution, out string resolutionInMeters)
            {
            date = null;

            JToken datesToken = token["dates"];
            if ( datesToken != null )
                {
                JToken pubDateToken = datesToken.FirstOrDefault(d => d.TryToGetString("type") == "Publication");
                string pubDateString = pubDateToken.TryToGetString("dateString");
                if ( pubDateString != null )
                    {
                    try
                        {
                        if ( pubDateString.Length == 4 )
                            {
                            date = DateTime.ParseExact(pubDateString, "yyyy", System.Globalization.CultureInfo.InvariantCulture);
                            }
                        else if ( pubDateString.Length == 7 )
                            {
                            date = DateTime.ParseExact(pubDateString, "yyyy-MM", System.Globalization.CultureInfo.InvariantCulture);
                            }
                        else
                            {
                            date = DateTime.ParseExact(pubDateString, "yyyy-MM-dd", System.Globalization.CultureInfo.InvariantCulture);
                            }
                        }
                    catch ( FormatException )
                        {
                        date = null;
                        }
                    }
                }

            JArray weblinks = token["webLinks"] as JArray;
            string newTokenZipURL = null;

            if ( weblinks != null )
                {
                foreach ( JObject weblink in weblinks )
                    {

                    string[] acceptedFormats = { "JPEG2000", "TIFF", "IMG", "Shapefile", "LAS" };
                    if ( (weblink["title"] != null) && (acceptedFormats.Contains(weblink["title"].Value<string>())) )
                        {
                        newTokenZipURL = weblink.TryToGetString("uri");
                        break;
                        }
                    }
                }

            string[] newTokenZipURLSplit = newTokenZipURL.Split('/', '\\');

            //The zip name is the last string in the array
            string fileName = newTokenZipURLSplit[newTokenZipURLSplit.Length - 1];

            string[] fileNameSplit = fileName.Split('_');

            if ( fileNameSplit.Count() < 5 )
                {
                resolution = null;
                resolutionInMeters = null;
                }
            else
                {
                string newTokenResolutionChar = fileNameSplit[4];
                if ( newTokenResolutionChar == "1" )
                    {
                    resolution = "1.0m";
                    resolutionInMeters = "1.0x1.0";
                    }
                else
                    {
                    if ( newTokenResolutionChar == "h" )
                        {
                        resolution = "0.5m";
                        resolutionInMeters = "0.5x0.5";
                        }
                    else
                        {
                        resolution = null;
                        resolutionInMeters = null;
                        }
                    }
                }
            }

        }

    /// <summary>
    /// This class is used to extract data from the different USGS API National Elevation Datasets
    /// </summary>
    public class NEDDataExtractor : IUSGSDataExtractor
        {
        string m_dataset;

        /// <summary>
        /// NEDDataExtractor constructor
        /// </summary>
        /// <param name="Dataset">Name of the dataset (sbDatasetTag)</param>
        public NEDDataExtractor (string Dataset)
            {
            m_dataset = Dataset;
            }

        /// <summary>
        /// Extract data from the USGS API Json token
        /// </summary>
        /// <param name="token">The json token obtained from the USGS API</param>
        /// <param name="title">The title extracted from the token</param>
        /// <param name="date">The date of publication of the data</param>
        /// <param name="resolution">The resolution of the data, if applicable</param>
        /// <param name="resolutionInMeters">The resolution in meters of the data, if applicable</param>
        public void ExtractTitleDateAndResolution (JToken token, out string title, out DateTime? date, out string resolution, out string resolutionInMeters)
            {
            title = token.TryToGetString("title");
            string dateString = token.TryToGetString("publicationDate");

            date = null;
            if ( dateString != null )
                {
                try
                    {
                    date = DateTime.ParseExact(dateString, "yyyy-MM-dd", System.Globalization.CultureInfo.InvariantCulture);
                    }
                catch ( FormatException )
                    {
                    Log.Logger.error(String.Format("Error while filtering USGS National Elevation Dataset entry {0}.", title));
                    date = null;
                    }
                }

            double lat;
            switch ( m_dataset )
                {

                case "Digital Elevation Model (DEM) 1 meter":
                    resolution = "1.0m";
                    resolutionInMeters = "1.0x1.0";
                    break;
                case "National Elevation Dataset (NED) 1/9 arc-second":
                    lat = (Convert.ToDouble(token["boundingBox"].TryToGetString("minY")) + Convert.ToDouble(token["boundingBox"].TryToGetString("maxY"))) / 2.0;
                    resolution = "1/9\"";
                    resolutionInMeters = String.Format("{0:0.####}", DbGeometryHelpers.ConvertArcsecLatToMeter((1.0 / 9.0), lat));
                    //It should never be as small as 0.001, so i've put this to prevent errors in case of bad input... 
                    //Anyway 0.001 is way smaller than the error of the ConvertArcsecLatToMeter function.
                    resolutionInMeters = resolutionInMeters != "0" ? resolutionInMeters + "x" + "3.43" : "0.001x3.43";
                    break;
                case "National Elevation Dataset (NED) 1/3 arc-second":
                    lat = (Convert.ToDouble(token["boundingBox"].TryToGetString("minY")) + Convert.ToDouble(token["boundingBox"].TryToGetString("maxY"))) / 2.0;
                    resolution = "1/3\"";
                    resolutionInMeters = String.Format("{0:0.####}", DbGeometryHelpers.ConvertArcsecLatToMeter((1.0 / 3.0), lat));
                    resolutionInMeters = resolutionInMeters != "0" ? resolutionInMeters + "x" + "10.29" : "0.001x10.29";
                    break;
                case "National Elevation Dataset (NED) 1 arc-second":
                    lat = (Convert.ToDouble(token["boundingBox"].TryToGetString("minY")) + Convert.ToDouble(token["boundingBox"].TryToGetString("maxY"))) / 2.0;
                    resolution = "1\"";
                    resolutionInMeters = String.Format("{0:0.####}", DbGeometryHelpers.ConvertArcsecLatToMeter((1.0), lat));
                    resolutionInMeters = resolutionInMeters != "0" ? resolutionInMeters + "x" + "30.87" : "0.001x30.87";
                    break;
                default:
                    resolution = null;
                    resolutionInMeters = null;
                    break;
                }
            }

        /// <summary>
        /// Extract data from the USGS ScienceBase Json token
        /// </summary>
        /// <param name="token">The json token obtained from ScienceBase</param>
        /// <param name="date">The date of publication of the data</param>
        /// <param name="resolution">The resolution of the data, if applicable</param>
        /// <param name="resolutionInMeters">The resolution in meters of the data, if applicable</param>
        public void ExtractDateAndResolutionSB (JToken token, out DateTime? date, out string resolution, out string resolutionInMeters)
            {
            string title = token.TryToGetString("title");

            date = null;

            JToken datesToken = token["dates"];
            if ( datesToken != null )
                {
                JToken pubDateToken = datesToken.FirstOrDefault(d => d.TryToGetString("type") == "Publication");
                string pubDateString = pubDateToken.TryToGetString("dateString");
                if ( pubDateString != null )
                    {
                    try
                        {
                        if ( pubDateString.Length == 4 )
                            {
                            date = DateTime.ParseExact(pubDateString, "yyyy", System.Globalization.CultureInfo.InvariantCulture);
                            }
                        else if ( pubDateString.Length == 7 )
                            {
                            date = DateTime.ParseExact(pubDateString, "yyyy-MM", System.Globalization.CultureInfo.InvariantCulture);
                            }
                        else
                            {
                            date = DateTime.ParseExact(pubDateString, "yyyy-MM-dd", System.Globalization.CultureInfo.InvariantCulture);
                            }
                        }
                    catch ( FormatException )
                        {
                        date = null;
                        }
                    }
                }

            double lat;
            resolution = null;
            resolutionInMeters = null;
            JToken bbox = token["spatial"] != null ? token["spatial"]["boundingBox"] : null;
            if ( bbox != null )
                {
                switch ( m_dataset )
                    {
                    case "Digital Elevation Model (DEM) 1 meter":
                        resolution = "1.0m";
                        resolutionInMeters = "1.0x1.0";
                        break;
                    case "National Elevation Dataset (NED) 1/9 arc-second":
                        lat = (Convert.ToDouble(bbox.TryToGetString("minY")) + Convert.ToDouble(bbox.TryToGetString("maxY"))) / 2.0;
                        resolution = "1/9\"";
                        resolutionInMeters = String.Format("{0:0.####}", DbGeometryHelpers.ConvertArcsecLatToMeter((1.0 / 9.0), lat));
                        //It should never be as small as 0.001, so i've put this to prevent errors in case of bad input... 
                        //Anyway 0.001 is way smaller than the error of the ConvertArcsecLatToMeter function.
                        resolutionInMeters = resolutionInMeters != "0" ? resolutionInMeters + "x" + "3.43" : "0.001x3.43";
                        break;
                    case "National Elevation Dataset (NED) 1/3 arc-second":
                        lat = (Convert.ToDouble(bbox.TryToGetString("minY")) + Convert.ToDouble(bbox.TryToGetString("maxY"))) / 2.0;
                        resolution = "1/3\"";
                        resolutionInMeters = String.Format("{0:0.####}", DbGeometryHelpers.ConvertArcsecLatToMeter((1.0 / 3.0), lat));
                        resolutionInMeters = resolutionInMeters != "0" ? resolutionInMeters + "x" + "10.29" : "0.001x10.29";
                        break;
                    case "National Elevation Dataset (NED) 1 arc-second":
                        lat = (Convert.ToDouble(bbox.TryToGetString("minY")) + Convert.ToDouble(bbox.TryToGetString("maxY"))) / 2.0;
                        resolution = "1\"";
                        resolutionInMeters = String.Format("{0:0.####}", DbGeometryHelpers.ConvertArcsecLatToMeter((1.0), lat));
                        resolutionInMeters = resolutionInMeters != "0" ? resolutionInMeters + "x" + "30.87" : "0.001x30.87";
                        break;
                    default:
                        resolution = null;
                        resolutionInMeters = null;
                        break;
                    }
                }
            }
        }

    /// <summary>
    /// This class is used to extract data from the USGS API Datasets that do not have any resolution and follow standard date specification.
    /// </summary>
    public class DefaultDataExtractor : IUSGSDataExtractor
        {
        /// <summary>
        /// DefaultDataExtractor constructor
        /// </summary>
        public DefaultDataExtractor ()
            {

            }

        /// <summary>
        /// Extract data from the USGS API Json token
        /// </summary>
        /// <param name="token">The json token obtained from the USGS API</param>
        /// <param name="title">The title extracted from the token</param>
        /// <param name="date">The date of publication of the data</param>
        /// <param name="resolution">The resolution of the data, if applicable</param>
        /// <param name="resolutionInMeters">The resolution in meters of the data, if applicable</param>
        public void ExtractTitleDateAndResolution (JToken token, out string title, out DateTime? date, out string resolution, out string resolutionInMeters)
            {
            title = token.TryToGetString("title");
            date = null;

            string dateString = token.TryToGetString("publicationDate");
            if ( dateString != null )
                {
                try
                    {
                    date = DateTime.ParseExact(dateString, "yyyy-MM-dd", System.Globalization.CultureInfo.InvariantCulture);
                    }
                catch ( FormatException )
                    {
                    Log.Logger.error(String.Format("Error while filtering USGS entry {0}.", title));
                    date = null;
                    }
                }
                
            resolution = null;
            resolutionInMeters = null;
            }

        /// <summary>
        /// Extract data from the USGS ScienceBase Json token
        /// </summary>
        /// <param name="token">The json token obtained from ScienceBase</param>
        /// <param name="date">The date of publication of the data</param>
        /// <param name="resolution">The resolution of the data, if applicable</param>
        /// <param name="resolutionInMeters">The resolution in meters of the data, if applicable</param>
        public void ExtractDateAndResolutionSB (JToken token, out DateTime? date, out string resolution, out string resolutionInMeters)
            {
            string title = token.TryToGetString("title");

            date = null;

            JToken datesToken = token["dates"];
            if ( datesToken != null )
                {
                JToken pubDateToken = datesToken.FirstOrDefault(d => d.TryToGetString("type") == "Publication");
                string pubDateString = pubDateToken.TryToGetString("dateString");
                if ( pubDateString != null )
                    {
                    try
                        {
                        if ( pubDateString.Length == 4 )
                            {
                            date = DateTime.ParseExact(pubDateString, "yyyy", System.Globalization.CultureInfo.InvariantCulture);
                            }
                        else if ( pubDateString.Length == 7 )
                            {
                            date = DateTime.ParseExact(pubDateString, "yyyy-MM", System.Globalization.CultureInfo.InvariantCulture);
                            }
                        else
                            {
                            date = DateTime.ParseExact(pubDateString, "yyyy-MM-dd", System.Globalization.CultureInfo.InvariantCulture);
                            }
                        }
                    catch ( FormatException )
                        {
                        date = null;
                        }
                    }
                }

            resolution = null;
            resolutionInMeters = null;
            }
        }

    /// <summary>
    /// This class is used to extract data from the different USGS API National Land Cover Database Datasets
    /// </summary>
    public class NLCDDataExtractor : IUSGSDataExtractor
        {
        string m_dataset;
        /// <summary>
        /// NLCDDataExtractor constructor
        /// </summary>
        /// <param name="Dataset">Name of the dataset (sbDatasetTag)</param>
        public NLCDDataExtractor (string Dataset)
            {
            m_dataset = Dataset;
            }

        /// <summary>
        /// Extract data from the USGS API Json token
        /// </summary>
        /// <param name="token">The json token obtained from the USGS API</param>
        /// <param name="title">The title extracted from the token</param>
        /// <param name="date">The date of publication of the data</param>
        /// <param name="resolution">The resolution of the data, if applicable</param>
        /// <param name="resolutionInMeters">The resolution in meters of the data, if applicable</param>
        public void ExtractTitleDateAndResolution (JToken token, out string title, out DateTime? date, out string resolution, out string resolutionInMeters)
            {
            title = token.TryToGetString("title");
            resolution = "30.0m";
            resolutionInMeters = "30.0x30.0";

            switch ( m_dataset )
                {

                case "National Land Cover Database (NLCD) - 2001":
                    date = new DateTime(2001, 1, 1);
                    break;
                case "National Land Cover Database (NLCD) - 2006":
                    date = new DateTime(2006, 1, 1);
                    break;
                case "National Land Cover Database (NLCD) - 2011":
                    date = new DateTime(2011, 1, 1);
                    break;
                default:
                    Log.Logger.error(String.Format("Error while filtering USGS National Land Cover Database entry {0}.", title));
                    date = null;
                    break;
                }

            }

        /// <summary>
        /// Extract data from the USGS ScienceBase Json token
        /// </summary>
        /// <param name="token">The json token obtained from ScienceBase</param>
        /// <param name="date">The date of publication of the data</param>
        /// <param name="resolution">The resolution of the data, if applicable</param>
        /// <param name="resolutionInMeters">The resolution in meters of the data, if applicable</param>
        public void ExtractDateAndResolutionSB (JToken token, out DateTime? date, out string resolution, out string resolutionInMeters)
            {
            string title = token.TryToGetString("title");
            resolution = "30.0m";
            resolutionInMeters = "30.0x30.0";

            switch ( m_dataset )
                {

                case "National Land Cover Database (NLCD) - 2001":
                    date = new DateTime(2001, 1, 1);
                    break;
                case "National Land Cover Database (NLCD) - 2006":
                    date = new DateTime(2006, 1, 1);
                    break;
                case "National Land Cover Database (NLCD) - 2011":
                    date = new DateTime(2011, 1, 1);
                    break;
                default:
                    date = null;
                    break;
                }

            }
        }

    ///// <summary>
    ///// This class is used to extract data from the USGS API Lidar Point Cloud Dataset
    ///// </summary>
    //public class LidarDataExtractor : IUSGSDataExtractor
    //{
    //    /// <summary>
    //    /// LidarDataExtractor constructor
    //    /// </summary>
    //    public LidarDataExtractor()
    //    {

    //    }

    //    /// <summary>
    //    /// Extract data from the USGS API Json token
    //    /// </summary>
    //    /// <param name="token">The json token obtained from the USGS API</param>
    //    /// <param name="title">The title extracted from the token</param>
    //    /// <param name="date">The date of publication of the data</param>
    //    /// <param name="resolution">The resolution of the data, if applicable</param>
    //    /// <param name="resolutionInMeters">The resolution in meters of the data, if applicable</param>
    //    public void ExtractTitleDateAndResolution(JToken token, out string title, out DateTime? date, out string resolution, out string resolutionInMeters)
    //    {
    //        string dateString = token.TryToGetString("publicationDate");
    //        title = token.TryToGetString("title");


    //        if (dateString != null)
    //        {
    //            try
    //            {
    //                date = DateTime.ParseExact(dateString, "yyyy-MM-dd", System.Globalization.CultureInfo.InvariantCulture);
    //            }
    //            catch(FormatException)
    //            {
    //                Log.Logger.error(String.Format("Error while filtering USGS Lidar entry {0}.", title));
    //                throw new Bentley.EC.Persistence.Operations.OperationFailedException("Error while filtering USGS Lidar results.");
    //            }
    //        }
    //        else
    //        {
    //            date = null;
    //        }
    //        resolution = null;
    //        resolutionInMeters = null;
    //    }
    //}


    }
