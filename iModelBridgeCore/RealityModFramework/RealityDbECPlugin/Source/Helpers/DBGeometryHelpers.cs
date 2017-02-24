/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/Helpers/DBGeometryHelpers.cs $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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

        public static string CreateFootprintString(string minX, string minY, string maxX, string maxY, int srid)
            {
            return String.Format(@"{{""points"":[[{0},{1}],[{2},{1}],[{2},{3}],[{0},{3}],[{0},{1}]],""coordinate_system"":""{4}""}}",minX,minY,maxX, maxY,srid);
            }

        public static string CreateEsriPolygonFromPolyModel (PolygonModel model)
            {
            string esriPoly = "{\"rings\":[";
            
            //This will give [x1,y1],[x2,y2],...[xn,yn]
            esriPoly += String.Join( ",", model.points.Select(p => "[" + p[0] + "," + p[1] + "]"));

            int count = model.points.Count;

            //We have to close the polygon, if it's not done already
            if ( (model.points[0][0] != model.points[count][0]) || model.points[0][1] != model.points[count][1] )
                {
                esriPoly += ",[" + model.points[0][0] + "," + model.points[0][1] + "]";
                }

            esriPoly += "],\"spatialReference\":{\"wkid\":" + model.coordinate_system + "}}";
            return esriPoly;
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
                throw new ArgumentException("Invalid WKT polygon");
                }

            string outerShell = splitStr[1];

            string[] outerShellPoints = outerShell.Split(new char[] { ',' }, StringSplitOptions.RemoveEmptyEntries);

            // A length of 4 is the minimum amount of points in a WKT polygon (triangle), since the last point is supposed to be the same as the first.
            if ( outerShellPoints.Length < 4 )
                {
                throw new ArgumentException("Invalid WKT polygon");
                }

            for ( int i = 0; i < outerShellPoints.Length; i++ )
                {
                string[] coords = outerShellPoints[i].Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);
                if ( coords.Length != 2 )
                    {
                    throw new ArgumentException("Invalid WKT polygon");
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

        public static BBox ExtractBboxFromWKTPolygon (string polygon)
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
            BBox bbox = new BBox();
            bbox.minX = (Double.MaxValue);
            bbox.minY = (Double.MaxValue);
            bbox.maxX = (Double.MinValue);
            bbox.maxY = (Double.MinValue);

            for ( int i = 0; i < coordsList.Count; i++ )
                {
                if ( i % 2 == 0 )
                    {
                    bbox.minX = Math.Min(coordsList[i], bbox.minX);
                    bbox.maxX = Math.Max(coordsList[i], bbox.maxX);
                    }
                else
                    {
                    bbox.minY = Math.Min(coordsList[i], bbox.minY);
                    bbox.maxY = Math.Max(coordsList[i], bbox.maxY);
                    }
                }
            return bbox;
            }

        public static string ExtractBboxStringFromWKTPolygon (string polygon)
            {
            BBox bbox = ExtractBboxFromWKTPolygon(polygon);

            return String.Format("{0},{1},{2},{3}", bbox.minX, bbox.minY, bbox.maxX, bbox.maxY);
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
