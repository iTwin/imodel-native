using System;
using System.Collections.Generic;
//using System.Data.Entity.Spatial;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IndexECPlugin.Source.Helpers
{
    internal static class DbGeometryHelpers
    {
        public static String CreateWktPolygonString(IEnumerable<double[]> enumerationOfPoints)
        {
            StringWriter wktPolygon = new StringWriter(new CultureInfo("en-us"));
            wktPolygon.Write("POLYGON((");
            int numberOfPoints = enumerationOfPoints.Count();
            for (int i = 0; i < numberOfPoints; i++)
            {
                wktPolygon.Write("{0} {1}", enumerationOfPoints.ElementAt(i)[0], enumerationOfPoints.ElementAt(i)[1]);
                if (i != numberOfPoints - 1)
                {
                    wktPolygon.Write(", ");
                }
                else
                {
                    if ((enumerationOfPoints.ElementAt(i)[0] != enumerationOfPoints.ElementAt(0)[0]) ||
                        (enumerationOfPoints.ElementAt(i)[1] != enumerationOfPoints.ElementAt(0)[1]))
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

        //public static string ExtractPointsLatLong(DbGeometry geom)
        //{
        //    string jsonListOfPoints = "[";

        //    if (geom.ExteriorRing == null)
        //    {
        //        for (int j = 1; j <= geom.Envelope.PointCount.Value; j++)
        //        {
        //            if (j != 1)
        //            {
        //                jsonListOfPoints += ",";
        //            }
        //            jsonListOfPoints += "[" + geom.Envelope.PointAt(j).YCoordinate.Value + "," + geom.Envelope.PointAt(j).XCoordinate.Value + "]";
        //        }
        //    }
        //    else
        //    {
        //        for (int j = 1; j <= geom.ExteriorRing.PointCount.Value; j++)
        //        {
        //            if (j != 1)
        //            {
        //                jsonListOfPoints += ",";
        //            }
        //            jsonListOfPoints += "[" + geom.ExteriorRing.PointAt(j).YCoordinate.Value + "," + geom.ExteriorRing.PointAt(j).XCoordinate.Value + "]";
        //        }
        //    }

        //    jsonListOfPoints += "]";
        //    return jsonListOfPoints;
        //}

        //public static string ExtractPointsLongLat(DbGeometry geom)
        //{
        //    string jsonListOfPoints = "[";

        //    if (geom.ExteriorRing == null)
        //    {
        //        for (int j = 1; j <= geom.Envelope.PointCount.Value; j++)
        //        {
        //            if (j != 1)
        //            {
        //                jsonListOfPoints += ",";
        //            }
        //            jsonListOfPoints += "[" + geom.Envelope.PointAt(j).XCoordinate.Value + "," + geom.Envelope.PointAt(j).YCoordinate.Value + "]";
        //        }
        //    }
        //    else
        //    {
        //        for (int j = 1; j <= geom.ExteriorRing.PointCount.Value; j++)
        //        {
        //            if (j != 1)
        //            {
        //                jsonListOfPoints += ",";
        //            }
        //            jsonListOfPoints += "[" + geom.ExteriorRing.PointAt(j).XCoordinate.Value + "," + geom.ExteriorRing.PointAt(j).YCoordinate.Value + "]";
        //        }
        //    }

        //    jsonListOfPoints += "]";
        //    return jsonListOfPoints;
        //}

        public static string ExtractOuterShellPointsFromWKTPolygon(string polygon)
        {


            string resultString = "[";

            polygon.Trim();

            if(!polygon.StartsWith("POLYGON"))
            {
                throw new ArgumentException("Only Polygon WKT are valid");
            }

            string[] splitStr = polygon.Split(new char[] {'(', ')'}, StringSplitOptions.RemoveEmptyEntries);

            if(splitStr.Length == 0)
            {
                throw new ArgumentException(String.Format("Invalid WKT polygon : {0}", polygon));
            }

            string outerShell = splitStr[1];

            string[] outerShellPoints = outerShell.Split(new char[] { ',' }, StringSplitOptions.RemoveEmptyEntries);

            // A length of 4 is the minimum amount of points in a WKT polygon (triangle), since the last point is supposed to be the same as the first.
            if(outerShellPoints.Length < 4)
            {
                throw new ArgumentException(String.Format("Invalid WKT polygon : {0}", polygon));
            }

            for (int i = 0; i < outerShellPoints.Length; i++)
            {
                string[] coords = outerShellPoints[i].Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);
                if(coords.Length != 2)
                {
                    throw new ArgumentException(String.Format("Invalid WKT polygon : {0}", polygon));
                }

                resultString += "[" + coords[0] + "," + coords[1] + "]";
                if (i != outerShellPoints.Length - 1)
                {
                    resultString += ",";
                }
            }

            resultString += "]";

            return resultString;
        }

        public static string ExtractBboxFromWKTPolygon(string polygon)
        {

            List<double> coordsList = new List<double>();

            polygon.Trim();

            if (!polygon.StartsWith("POLYGON"))
            {
                throw new ArgumentException("Only Polygon WKT are valid");
            }

            string[] splitStr =  polygon.Split(new char[] { '(', ')' }, StringSplitOptions.RemoveEmptyEntries);

            if (splitStr.Length == 0)
            {
                throw new ArgumentException("Invalid WKT");
            }

            string outerShell = splitStr[1];

            string[] outerShellPoints = outerShell.Split(',');

            // A length of 4 is the minimum amount of points in a WKT polygon (triangle), since the last point is supposed to be the same as the first.
            if (outerShellPoints.Length < 4)
            {
                throw new ArgumentException("Invalid WKT");
            }

            for (int i = 0; i < outerShellPoints.Length; i++)
            {
                string[] coords = outerShellPoints[i].Split(new char[] {' '}, StringSplitOptions.RemoveEmptyEntries);
                if (coords.Length != 2)
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

            for (int i = 0; i < coordsList.Count; i++ )
            {
                if(i % 2 == 0)
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
    }



}
