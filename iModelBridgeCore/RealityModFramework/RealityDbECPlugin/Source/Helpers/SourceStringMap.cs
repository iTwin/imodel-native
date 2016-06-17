using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IndexECPlugin.Source.Helpers
    {
    internal static class SourceStringMap
        {
        public static string SourceToString (DataSource source)
            {
            switch ( source )
                {
                case DataSource.Index:
                    return "index";
                case DataSource.USGS:
                    return "usgsapi";
                case DataSource.All:
                    return "all";
                default:
                    throw new NotImplementedException("This source map is not implemented yet");
                }
            }

        public static DataSource StringToSource (string sourceStr)
            {
            switch ( sourceStr.ToLower() )
                {
                case "index":
                    return DataSource.Index;
                case "usgsapi":
                    return DataSource.USGS;
                case "all":
                    return DataSource.All;
                default:
                    throw new NotImplementedException(String.Format("The source {0} is not mapped yet", sourceStr));
                }
            }
        public static string GetAllSourceStrings ()
            {
            //String result = "";
            //foreach ( Source source in Enum.GetValues(typeof(Source)) )
            //    {
            //    result += "\"" + SourceToString(source) + "\", ";
            //    }

            //return result.TrimEnd(' ', ',');

            //I hard coded it, because I feared that the commented method could throw exceptions if the map was not updated at the same time as the Source enum.
            return "\"index\", \"usgsAPI\" and \"all\"";
            }
        }

    /// <summary>
    /// Represents the datasources the ECPlugin uses
    /// </summary>
    public enum DataSource
        {
        /// <summary>
        /// Index represents the database maintained by this server's administrators
        /// </summary>
        Index = 0,
        /// <summary>
        /// The USGS API datasource
        /// </summary>
        USGS,
        /// <summary>
        /// Represents all datasources
        /// </summary>
        All
        }
    }
