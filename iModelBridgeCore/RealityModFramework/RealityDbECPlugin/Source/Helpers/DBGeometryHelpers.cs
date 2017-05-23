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

                bool polygonIsClosed = enumerationOfPoints.Last()[0] == enumerationOfPoints.First()[0] &&
                                       enumerationOfPoints.Last()[1] == enumerationOfPoints.First()[1];

                if ( numberOfPoints == 3 && polygonIsClosed )
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
                        if ( polygonIsClosed )
                            {
                            wktPolygon.Write("))");
                            }
                        else
                            {
                            wktPolygon.Write(", {0} {1}))", enumerationOfPoints.First()[0], enumerationOfPoints.First()[1]);
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

                wktPolygon.Write(CreateWktPolygonString(model.Points));
                wktPolygon.Write("'," + model.coordinate_system + ")");
                return wktPolygon.ToString();
                }
            }

        public static PolygonModel CreatePolygonModelFromJson (string jsonPolygon)
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

        public static string CreateFootprintString (string minX, string minY, string maxX, string maxY, int srid)
            {
            return String.Format(@"{{""points"":[[{0},{1}],[{2},{1}],[{2},{3}],[{0},{3}],[{0},{1}]],""coordinate_system"":""{4}""}}", minX, minY, maxX, maxY, srid);
            }

        public static string CreateEsriPolygonFromPolyModel (PolygonModel model)
            {
            string esriPoly = "{\"rings\":[[";

            //This will give [x1,y1],[x2,y2],...[xn,yn]
            esriPoly += String.Join(",", model.Points.Select(p => "[" + p[0] + "," + p[1] + "]"));

            int count = model.Points.Count;

            //We have to close the polygon, if it's not done already
            if ( (model.Points[0][0] != model.Points[count - 1][0]) || model.Points[0][1] != model.Points[count - 1][1] )
                {
                esriPoly += ",[" + model.Points[0][0] + "," + model.Points[0][1] + "]";
                }

            esriPoly += "]],\"spatialReference\":{\"wkid\":" + model.coordinate_system + "}}";
            return esriPoly;
            }

        internal static string[][] GetCoordinateArrayFromWKTPolygon (string polygon)
            {
            polygon = polygon.Trim();

            if ( !polygon.StartsWith("POLYGON") )
                {
                throw new ArgumentException("Only Polygon WKT are valid");
                }

            string[] splitStr = polygon.Split(new char[] { '(', ')' }, StringSplitOptions.RemoveEmptyEntries);

            if ( splitStr.Length < 2 )
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

            string[][] coordinateArray = new string[outerShellPoints.Length][];

            for ( int i = 0; i < outerShellPoints.Length; i++ )
                {
                string[] coords = outerShellPoints[i].Split(new[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);
                if ( coords.Length != 2 )
                    {
                    throw new ArgumentException("Invalid WKT polygon");
                    }
                coordinateArray[i] = coords;
                }
            return coordinateArray;
            }

        public static string ExtractOuterShellPointsFromWKTPolygon (string polygon)
            {
            string resultString = "[";

            string[][] coordinateArray = GetCoordinateArrayFromWKTPolygon(polygon);

            foreach ( string[] coordinate in coordinateArray )
                {
                resultString += "[" + coordinate[0] + "," + coordinate[1] + "]";
                if ( coordinate != coordinateArray.Last() )
                    {
                    resultString += ",";
                    }
                }
            resultString += "]";

            return resultString;
            }

        public static BBox ExtractBboxFromWKTPolygon (string polygon)
            {
            string[][] coordinateArray = GetCoordinateArrayFromWKTPolygon(polygon);

            //bbox is a capacity 4 list that will contain a bbox in this format : 
            //[xmin, ymin, xmax, ymax]
            BBox bbox = new BBox();
            bbox.minX = Double.MaxValue;
            bbox.minY = Double.MaxValue;
            bbox.maxX = Double.MinValue;
            bbox.maxY = Double.MinValue;

            foreach ( string[] coordinate in coordinateArray )
                {
                double x = Convert.ToDouble(coordinate[0]);
                double y = Convert.ToDouble(coordinate[1]);

                bbox.minX = Math.Min(x, bbox.minX);
                bbox.maxX = Math.Max(x, bbox.maxX);
                bbox.minY = Math.Min(y, bbox.minY);
                bbox.maxY = Math.Max(y, bbox.maxY);
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
