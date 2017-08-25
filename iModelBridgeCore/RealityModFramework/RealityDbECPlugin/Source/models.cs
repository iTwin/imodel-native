/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/models.cs $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/

using Bentley.EC.Persistence.Query;
using Bentley.ECObjects.Schema;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using RealityPackageNet;
using System.Data;
using Bentley.ECObjects;
using Bentley.Exceptions;

namespace IndexECPlugin.Source
    {
    //These classes are used for json deserialization or various "holding" purposes


    /// <summary>
    /// TO BE DONE
    /// </summary>
    public class PolygonModel
        {
        private List<Double[]> m_points;

        /// <summary>
        /// TO BE DONE
        /// </summary>
        public PolygonModel (List<Double[]> points)
            {
            Points = points;
            }

        /// <summary>
        /// TO BE DONE
        /// </summary>
        public List<double[]> Points
            {
            get
                {
                return m_points;
                }
            set
                {
                if ( !isListOfPointsValid(value) )
                    {
                    throw new ArgumentException("The polygon format is not valid.");
                    }
                m_points = value;
                }
            }
        /// <summary>
        /// TO BE DONE
        /// </summary>
        public int coordinate_system
            {
            get;
            set;
            }

        private static bool isListOfPointsValid (List<Double[]> listOfPoints)
            {
            bool isValid = true;
            int numberOfPoints = listOfPoints.Count();

            if ( numberOfPoints < 3 )
                {
                isValid = false;
                }
            else
                {
                bool polygonIsClosed = listOfPoints.Last()[0] == listOfPoints.First()[0] &&
                                       listOfPoints.Last()[1] == listOfPoints.First()[1];

                if ( numberOfPoints == 3 && polygonIsClosed )
                    {
                    isValid = false;
                    }
                }
            return isValid;
            }
        }

    /// <summary>
    /// 
    /// </summary>
    public class BBox
        {
        /// <summary>
        /// 
        /// </summary>
        public double minX
            {
            get;
            set;
            }
        /// <summary>
        /// 
        /// </summary>
        public double maxX
            {
            get;
            set;
            }
        /// <summary>
        /// 
        /// </summary>
        public double minY
            {
            get;
            set;
            }
        /// <summary>
        /// 
        /// </summary>
        public double maxY
            {
            get;
            set;
            }

        }

    /// <summary>
    /// TO BE DONE
    /// </summary>
    public class PolygonDescriptor
        {
        /// <summary>
        /// TO BE DONE
        /// </summary>
        public string WKT
            {
            get;
            set;
            }
        /// <summary>
        /// TO BE DONE
        /// </summary>
        public int SRID
            {
            get;
            set;
            }
        }

    /// <summary>
    /// TO BE DONE
    /// </summary>
    public class RequestedEntity
        {
        /// <summary>
        /// TO BE DONE
        /// </summary>
        public string ID
            {
            get;
            set;
            }
        /// <summary>
        /// TO BE DONE
        /// </summary>
        public string SelectedFormat
            {
            get;
            set;
            }
        /// <summary>
        /// TO BE DONE
        /// </summary>
        public string SpatialDataSourceID
            {
            get;
            set;
            }
        /// <summary>
        /// TO BE DONE
        /// </summary>
        public string SelectedStyle
            {
            get;
            set;
            }

        /// <summary>
        /// TO BE DONE
        /// </summary>
        public IEnumerable<double[]> Footprint
            {
            get;
            set;
            }

        ///// <summary>
        ///// 
        ///// </summary>
        //public string Type { get; set; }
        }

    /// <summary>
    /// TO BE DONE  
    /// </summary>    
    public class MapInfo
        {
        /// <summary>
        /// TO BE DONE  
        /// </summary>
        public string GetMapURL
            {
            get;
            set;
            }

        /// <summary>
        /// TO BE DONE
        /// </summary>
        public string GetMapURLQuery
            {
            get;
            set;
            }

        /// <summary>
        /// TO BE DONE
        /// </summary>
        public string Layers
            {
            get;
            set;
            }

        /// <summary>
        ///   
        /// </summary>
        public string Version
            {
            get;
            set;
            }

        /// <summary>
        ///   
        /// </summary>
        public string CoordinateSystem
            {
            get;
            set;
            }

        /// <summary>
        ///   
        /// </summary>
        public string SelectedFormat
            {
            get;
            set;
            }

        /// <summary>
        ///   
        /// </summary>
        public string SelectedStyle
            {
            get;
            set;
            }

        /// <summary>
        ///   
        /// </summary>
        public string Legal
            {
            get;
            set;
            }

        /// <summary>
        ///   
        /// </summary>
        public IEnumerable<double[]> Footprint
            {
            get;
            set;
            }
        }

    /// <summary>
    /// 
    /// </summary>
    public class GenericInfo
        {
        /// <summary>
        /// 
        /// </summary>
        public string URI
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public bool ParameterizedURI
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string Type
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string Copyright
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string TermsOfUse
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string Provider
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string SpatialEntityID
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string Name
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string Footprint
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string SpatialDataSourceID
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string FileInCompound
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string CoordinateSystem
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string Classification
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string NoDataValue
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public ulong FileSize
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string Metadata
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string Dataset
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public List<UriNet> SisterFiles
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public bool Streamed
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string ServerLoginKey
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string ServerLoginMethod
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string ServerOrganisationPage
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string ServerRegistrationPage
            {
            get;
            set;
            }

        }

    /// <summary>
    /// 
    /// </summary>
    public class UsgsAPICategory
        {
        /// <summary>
        ///   
        /// </summary>        
        public string Title
            {
            get;
            set;
            }

        /// <summary>
        ///   
        /// </summary>
        public string SubTitle
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string SbDatasetTag
            {
            get;
            set;
            }

        /// <summary>
        ///   
        /// </summary>
        public string Format
            {
            get;
            set;
            }

        /// <summary>
        ///   
        /// </summary>
        public int Priority
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public String Classification
            {
            get;
            set;
            }
        }

    /// <summary>
    /// 
    /// </summary>
    public class AustraliaLayer
        {
        /// <summary>
        /// 
        /// </summary>
        public string Id
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string Classification
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string IdField
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string CopyrightText
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string Type
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string GeometryType
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string Dataset
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public Dictionary<string, string> FieldMap
            {
            get;
            set;
            }
        }

    /// <summary>
    ///   
    /// </summary>
    public class UsgsRequest : IComparable<UsgsRequest>
        {
        /// <summary>
        ///   
        /// </summary>
        public string Dataset
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string DatasetID
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string Format
            {
            get;
            set;
            }

        /// <summary>
        ///   
        /// </summary>
        public string Category
            {
            get;
            set;
            }

        /// <summary>
        ///   
        /// </summary>
        public int Priority
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string Classification
            {
            get;
            set;
            }

        /// <summary>
        ///   
        /// </summary>
        public int CompareTo (UsgsRequest req)
            {
            int comp = Category.CompareTo(req.Category);
            if ( comp != 0 )
                {
                return comp;
                }
            return Priority.CompareTo(req.Priority);
            }
        }

    /// <summary>
    /// 
    /// </summary>
    public class USGSRequestBundle
        {
        /// <summary>
        /// 
        /// </summary>
        public IEnumerable<JToken> jtokenList
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string DatasetId
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string Dataset
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string Classification
            {
            get;
            set;
            }
        }

    /// <summary>
    /// 
    /// </summary>
    public class AURequestBundle
        {
        /// <summary>
        /// 
        /// </summary>
        public JObject LayerContent
            {
            get;
            set;
            }
        /// <summary>
        /// 
        /// </summary>
        public AustraliaLayer Layer
            {
            get;
            set;
            }
        }

    /// <summary>
    /// 
    /// </summary>
    public class USGSExtractedResult
        {
        /// <summary>
        /// 
        /// </summary>
        public JToken jToken
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string Title
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public DateTime? Date
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string Resolution
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string ResolutionInMeters
            {
            get;
            set;
            }
        }

    /// <summary>
    /// 
    /// </summary>
    public class SingleWhereCriteriaHolder
        {
        /// <summary>
        /// 
        /// </summary>
        public IECProperty Property
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string Value
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public RelationalOperator Operator
            {
            get;
            set;
            }
        }

    internal class ColumnNameTypePair
        {
        public String ColumnName
            {
            get;
            set;
            }

        public SqlDbType SqlDbType
            {
            get;
            set;
            }
        }

    /// <summary>
    /// 
    /// </summary>
    public class UsgsEEDataset
        {

        /// <summary>
        /// 
        /// </summary>
        public string DatasetName
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string DatasetId
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string DataFormat
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public string Classification
            {
            get;
            set;
            }
        }

    /// <summary>
    /// 
    /// </summary>
    public class EERequestBundle
        {
        /// <summary>
        /// 
        /// </summary>
        public UsgsEEDataset Dataset
            {
            get;
            set;
            }

        /// <summary>
        /// 
        /// </summary>
        public IEnumerable<JToken> jtokenList
            {
            get;
            set;
            }
        }

    //internal enum ObjectClassification
    //{
    //    None = 0,
    //    Roadway,
    //    Bridge,
    //    Terrain,
    //    Building,
    //    WaterBody,
    //    Property,
    //    OffLimitZone,
    //    Imagery,
    //    PointCloud
    //}
    }
