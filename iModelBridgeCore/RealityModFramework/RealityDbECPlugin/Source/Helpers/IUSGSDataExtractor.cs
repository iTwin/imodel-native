using Bentley.Exceptions;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IndexECPlugin.Source.Helpers
{
    internal interface IUSGSDataExtractor
    {
        void ExtractTitleDateAndResolution(JToken token, out string title, out DateTime? date, out string resolution, out string resolutionInMeters);
    }

    internal class HRODataExtractor : IUSGSDataExtractor
    {
        public HRODataExtractor()
        {
            //Does nothing
        }

        public void ExtractTitleDateAndResolution(JToken token, out string title, out DateTime? date, out string resolution, out string resolutionInMeters)
        {
            string newTokenZipURL = token.TryToGetString("downloadURL");
            string[] newTokenZipURLSplit = newTokenZipURL.Split('/', '\\');

            //The zip name is the last string in the array
            string zipName = newTokenZipURLSplit[newTokenZipURLSplit.Length - 1];

            string[] zipNameSplit = zipName.Split('_');

            //According to the convention, the date of acquisition is located after the first underscore character
            string newDateString = zipNameSplit[1];

            resolution = zipNameSplit[2].Replace('x', '.');

            resolutionInMeters = resolution.TrimEnd('m');
            resolutionInMeters = resolutionInMeters + "x" + resolutionInMeters;

            if (newDateString.Length != 6)
            {
                throw new Bentley.EC.Persistence.Operations.OperationFailedException("Error while filtering results.");
            }

            date = DateTime.ParseExact(newDateString + "01", "yyyyMMdd", System.Globalization.CultureInfo.InvariantCulture);

            title = token.TryToGetString("title");
        }
    }

    internal class NAIPDataExtractor : IUSGSDataExtractor
    {
        public NAIPDataExtractor()
        {
            //Does nothing
        }

        public void ExtractTitleDateAndResolution(JToken token, out string title, out DateTime? date, out string resolution, out string resolutionInMeters)
        {
            string newTokenZipURL = token.TryToGetString("downloadURL");
            string[] newTokenZipURLSplit = newTokenZipURL.Split('/', '\\');

            //The zip name is the last string in the array
            string fileName = newTokenZipURLSplit[newTokenZipURLSplit.Length - 1];

            string[] fileNameSplit = fileName.Split('_');

            string newTokenResolutionChar = fileNameSplit[4];
            if (newTokenResolutionChar == "1")
            {
                resolution = "1.0m";
                resolutionInMeters = "1.0x1.0";
            }
            else
            {
                if (newTokenResolutionChar == "h")
                {
                    resolution = "0.5m";
                    resolutionInMeters = "0.5x0.5";
                }
                else
                {
                    throw new Bentley.EC.Persistence.Operations.OperationFailedException("Error while filtering results.");
                }
            }

            

            ////The last acquisition date is located at the end
            //string dateString = fileNameSplit[fileNameSplit.Count() - 1];
            string dateString = token.TryToGetString("publicationDate");
            if (dateString.Length != 10)
            {
                throw new Bentley.EC.Persistence.Operations.OperationFailedException("Error while filtering results.");
            }
            date = DateTime.ParseExact(dateString, "yyyy-MM-dd", System.Globalization.CultureInfo.InvariantCulture);

            title = token.TryToGetString("title");
        }
    }

    internal class NEDDataExtractor : IUSGSDataExtractor
    {
        string m_dataset;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="Dataset">Name of the dataset (sbDatasetTag)</param>
        public NEDDataExtractor(string Dataset)
        {
            m_dataset = Dataset;
        }

        public void ExtractTitleDateAndResolution(JToken token, out string title, out DateTime? date, out string resolution, out string resolutionInMeters)
        {

            string dateString = token.TryToGetString("publicationDate");
            date = DateTime.ParseExact(dateString, "yyyy-MM-dd", System.Globalization.CultureInfo.InvariantCulture);

            title = token.TryToGetString("title");

            double lat;
            switch (m_dataset)
            {
                
                case "Digital Elevation Model (DEM) 1 meter":
                    resolution = "1.0m";
                    resolutionInMeters = "1.0x1.0";
                    break;
                case "National Elevation Dataset (NED) 1/9 arc-second":
                    lat = (Convert.ToDouble(token["boundingBox"].TryToGetString("minY")) + Convert.ToDouble(token["boundingBox"].TryToGetString("maxY"))) / 2.0;
                    resolution = "1/9\"";
                    resolutionInMeters = String.Format("{0:0.####}", DbGeometryHelpers.ConvertArcsecLatToMeter((1.0/9.0), lat));
                    //It should never be as small as 0.001, so i've put this to prevent errors in case of bad input... 
                    //Anyway 0.001 is way smaller than the error of the ConvertArcsecLatToMeter function.
                    resolutionInMeters = resolutionInMeters != "0" ? resolutionInMeters + "x" + "3.43" : "0.001x3.43"; 
                    break;
                case "National Elevation Dataset (NED) 1/3 arc-second":
                    lat = (Convert.ToDouble(token["boundingBox"].TryToGetString("minY")) + Convert.ToDouble(token["boundingBox"].TryToGetString("maxY"))) / 2.0;
                    resolution = "1/3\"";
                    resolutionInMeters = String.Format("{0:0.####}", DbGeometryHelpers.ConvertArcsecLatToMeter((1.0/3.0), lat));
                    resolutionInMeters = resolutionInMeters != "0" ? resolutionInMeters + "x" + "10.29" : "0.001x10.29"; 
                    break;
                case "National Elevation Dataset (NED) 1 arc-second":
                    lat = (Convert.ToDouble(token["boundingBox"].TryToGetString("minY")) + Convert.ToDouble(token["boundingBox"].TryToGetString("maxY"))) / 2.0;
                    resolution = "1\"";
                    resolutionInMeters = String.Format("{0:0.####}", DbGeometryHelpers.ConvertArcsecLatToMeter((1.0), lat));
                    resolutionInMeters = resolutionInMeters != "0" ? resolutionInMeters + "x" + "30.87" : "0.001x30.87"; 

                    break;
                default:
                    throw new Bentley.EC.Persistence.Operations.OperationFailedException("Error while filtering results.");
            }
        }
    }

    internal class DefaultDataExtractor : IUSGSDataExtractor
    {
        public DefaultDataExtractor()
        {

        }

        public void ExtractTitleDateAndResolution(JToken token, out string title, out DateTime? date, out string resolution, out string resolutionInMeters)
        {
            string dateString = token.TryToGetString("publicationDate");

            if (dateString != null)
            {
                date = DateTime.ParseExact(dateString, "yyyy-MM-dd", System.Globalization.CultureInfo.InvariantCulture);
            }
            else
            {
                date = null;
            }
            title = token.TryToGetString("title");
            resolution = "Unknown";
            resolutionInMeters = "Unknown";
        }
    }

    internal class NLCDDataExtractor : IUSGSDataExtractor
    {
        string m_dataset;
        public NLCDDataExtractor(string Dataset)
        {
            m_dataset = Dataset;
        }

        public void ExtractTitleDateAndResolution(JToken token, out string title, out DateTime? date, out string resolution, out string resolutionInMeters)
        {
            title = token.TryToGetString("title");
            resolution = "30.0m";
            resolutionInMeters = "30.0x30.0";

            switch (m_dataset)
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
                    throw new Bentley.EC.Persistence.Operations.OperationFailedException("Error while filtering results.");
            }

        }
    }

    internal class LidarDataExtractor : IUSGSDataExtractor
    {
        public LidarDataExtractor()
        {

        }

        public void ExtractTitleDateAndResolution(JToken token, out string title, out DateTime? date, out string resolution, out string resolutionInMeters)
        {
            string dateString = token.TryToGetString("publicationDate");

            if (dateString != null)
            {
                date = DateTime.ParseExact(dateString, "yyyy-MM-dd", System.Globalization.CultureInfo.InvariantCulture);
            }
            else
            {
                date = null;
            }
            title = token.TryToGetString("title");
            resolution = "Unknown";
            resolutionInMeters = "Unknown";
        }
    }


}
