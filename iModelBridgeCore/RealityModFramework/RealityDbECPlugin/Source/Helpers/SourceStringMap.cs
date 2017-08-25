/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/Helpers/SourceStringMap.cs $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IndexECPlugin.Source.Helpers
    {
    internal static class SourceStringMap
        {

        public static bool IsValidId (DataSource source, string id)
            {
            switch ( source )
                {
                case DataSource.Index:
                    int num;
                    return int.TryParse(id, out num);
                case DataSource.USGS:
                    return (id.Length == IndexConstants.USGSIdLenght);
                case DataSource.RDS:
                    Guid guid;
                    return Guid.TryParse(id, out guid);
                //case DataSource.AU:
                //    string[] splitId = id.Split('_');
                //    if ( splitId.Length < 3 )
                //        {
                //        return false;
                //        }
                //    if ( splitId[0] != "AU" )
                //        {
                //        return false;
                //        }
                //    return true;
                case DataSource.USGSEE:
                    string[] splitId = id.Split(new string[]{"__"}, StringSplitOptions.RemoveEmptyEntries);
                    if ( splitId.Length < 3 )
                        {
                        return false;
                        }
                    if ( splitId[0] != "USGSEE" )
                        {
                        return false;
                        }
                    return true;
                case DataSource.All:
                    return true;
                default:
                    throw new NotImplementedException("This source is not implemented yet");
                }
            }

        public static string SourceToString (DataSource source)
            {
            switch ( source )
                {
                case DataSource.Index:
                    return "index";
                case DataSource.USGS:
                    return "usgsapi";
                case DataSource.RDS:
                    return "rds";
                case DataSource.USGSEE:
                    return "usgsee";
                //case DataSource.AU:
                //    return "au";
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
                case "rds":
                    return DataSource.RDS;
                //case "au":
                //    return DataSource.AU;
                case "usgsee":
                    return DataSource.USGSEE;
                case "all":
                    return DataSource.All;
                default:
                    throw new NotImplementedException("This source is not mapped yet.");
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
            //return "\"index\", \"usgsAPI\", \"rds\", and \"all\"";
            return "\"index\", \"usgsAPI\", \"rds\", \"usgsee\" and \"all\"";
            //return "\"index\", \"usgsAPI\", \"rds\", \"au\" and \"all\"";
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
        /// The Reality Data Service datasource
        /// </summary>
        RDS,
        /// <summary>
        /// The australian API datasource
        /// </summary> 
        AU,
        /// <summary>
        /// Usgs Earth Explorer API
        /// </summary>
        USGSEE,
        /// <summary>
        /// Represents all datasources
        /// </summary> 
        All
        }
    }
