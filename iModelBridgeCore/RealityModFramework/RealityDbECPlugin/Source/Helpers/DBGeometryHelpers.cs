﻿/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/Helpers/DBGeometryHelpers.cs $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/

using Bentley.Exceptions;
using System;
using System.Collections.Generic;
//using System.Data.Entity.Spatial;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json.Linq;
using Newtonsoft.Json;

namespace IndexECPlugin.Source.Helpers
    {
    internal static class DbGeometryHelpers
        {
        public static String CreateWktPolygonString (IEnumerable<double[]> enumerationOfPoints)
            {
            using ( StringWriter wktPolygon = new StringWriter(new CultureInfo("en-us")) )
                {
                wktPolygon.Write("POLYGON((");
                int numberOfPoints = enumerationOfPoints.Count();

                if ( numberOfPoints < 3 )
                    {
                    throw new UserFriendlyException("The polygon format is not valid.");
                    }

                for ( int i = 0; i < numberOfPoints; i++ )
                    {
                    wktPolygon.Write("{0} {1}", enumerationOfPoints.ElementAt(i)[0], enumerationOfPoints.ElementAt(i)[1]);
                    if ( i != numberOfPoints - 1 )
                        {
                        wktPolygon.Write(", ");
                        }
                    else
                        {
                        if ( (enumerationOfPoints.ElementAt(i)[0] != enumerationOfPoints.ElementAt(0)[0]) ||
                            (enumerationOfPoints.ElementAt(i)[1] != enumerationOfPoints.ElementAt(0)[1]) )
                            {
                            wktPolygon.Write(", {0} {1}))", enumerationOfPoints.ElementAt(0)[0], enumerationOfPoints.ElementAt(0)[1]);
                            }
                        else
                            {
                            wktPolygon.Write("))");
                            }
                        }
                    }
                return wktPolygon.ToString();
                }
            }

        public static string CreateSTGeomFromTextStringFromJson (string jsonPolygon)
            {
            using ( StringWriter wktPolygon = new StringWriter(new CultureInfo("en-us")) )
                {

                wktPolygon.Write("geometry::STGeomFromText('");

                PolygonModel model = CreatePolygonModelFromJson(jsonPolygon);

                wktPolygon.Write(CreateWktPolygonString(model.points));
                wktPolygon.Write("'," + model.coordinate_system + ")");
                return wktPolygon.ToString();
                }
            }

        public static PolygonModel CreatePolygonModelFromJson(string jsonPolygon)
            {
            try
                {
                return JsonConvert.DeserializeObject<PolygonModel>(jsonPolygon);
                }
            catch ( JsonSerializationException )
                {
                throw new UserFriendlyException("The polygon format is not valid.");
                }
            }

        public static string ExtractOuterShellPointsFromWKTPolygon (string polygon)
            {


            string resultString = "[";

            polygon.Trim();

            if ( !polygon.StartsWith("POLYGON") )
                {
                throw new ArgumentException("Only Polygon WKT are valid");
                }

            string[] splitStr = polygon.Split(new char[] { '(', ')' }, StringSplitOptions.RemoveEmptyEntries);

            if ( splitStr.Length == 0 )
                {
                throw new ArgumentException(String.Format("Invalid WKT polygon : {0}", polygon));
                }

            string outerShell = splitStr[1];

            string[] outerShellPoints = outerShell.Split(new char[] { ',' }, StringSplitOptions.RemoveEmptyEntries);

            // A length of 4 is the minimum amount of points in a WKT polygon (triangle), since the last point is supposed to be the same as the first.
            if ( outerShellPoints.Length < 4 )
                {
                throw new ArgumentException(String.Format("Invalid WKT polygon : {0}", polygon));
                }

            for ( int i = 0; i < outerShellPoints.Length; i++ )
                {
                string[] coords = outerShellPoints[i].Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);
                if ( coords.Length != 2 )
                    {
                    throw new ArgumentException(String.Format("Invalid WKT polygon : {0}", polygon));
                    }

                resultString += "[" + coords[0] + "," + coords[1] + "]";
                if ( i != outerShellPoints.Length - 1 )
                    {
                    resultString += ",";
                    }
                }

            resultString += "]";

            return resultString;
            }

        public static string ExtractBboxFromWKTPolygon (string polygon)
            {

            List<double> coordsList = new List<double>();

            polygon.Trim();

            if ( !polygon.StartsWith("POLYGON") )
                {
                throw new ArgumentException("Only Polygon WKT are valid");
                }

            string[] splitStr = polygon.Split(new char[] { '(', ')' }, StringSplitOptions.RemoveEmptyEntries);

            if ( splitStr.Length == 0 )
                {
                throw new ArgumentException("Invalid WKT");
                }

            string outerShell = splitStr[1];

            string[] outerShellPoints = outerShell.Split(',');

            // A length of 4 is the minimum amount of points in a WKT polygon (triangle), since the last point is supposed to be the same as the first.
            if ( outerShellPoints.Length < 4 )
                {
                throw new ArgumentException("Invalid WKT");
                }

            for ( int i = 0; i < outerShellPoints.Length; i++ )
                {
                string[] coords = outerShellPoints[i].Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);
                if ( coords.Length != 2 )
                    {
                    throw new ArgumentException("Invalid WKT");
                    }

                coordsList.Add(Convert.ToDouble(coords[0]));
                coordsList.Add(Convert.ToDouble(coords[1]));
                }

            //bbox is a capacity 4 list that will contain a bbox in this format : 
            //[xmin, ymin, xmax, ymax]
            List<double> bbox = new List<double>(4);
            bbox.Add(Double.MaxValue);
            bbox.Add(Double.MaxValue);
            bbox.Add(Double.MinValue);
            bbox.Add(Double.MinValue);

            for ( int i = 0; i < coordsList.Count; i++ )
                {
                if ( i % 2 == 0 )
                    {
                    bbox[0] = Math.Min(coordsList[i], bbox[0]);
                    bbox[2] = Math.Max(coordsList[i], bbox[2]);
                    }
                else
                    {
                    bbox[1] = Math.Min(coordsList[i], bbox[1]);
                    bbox[3] = Math.Max(coordsList[i], bbox[3]);
                    }
                }



            return String.Format("{0},{1},{2},{3}", bbox[0], bbox[1], bbox[2], bbox[3]);
            }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="arcsec">number of arsec</param>
        /// <param name="lat">Latitude in degrees</param>
        /// <returns></returns>
        public static double ConvertArcsecLatToMeter (double arcsec, double lat)
            {
            //6371000 * Math.Cos(lat*Math.PI*2/360) * arcsec/206265
            if ( (lat > 90) || (lat < -90) )
                {
                throw new UserFriendlyException("Please use a valid number for latitude");
                }
            return 30.87 * Math.Cos(lat * Math.PI / 180) * arcsec;
            }


        }
    }
