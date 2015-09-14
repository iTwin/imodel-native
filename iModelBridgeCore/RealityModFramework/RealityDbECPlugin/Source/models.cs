using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IndexECPlugin.Source
{
    //These classes are used for json deserialization or various "holding" purposes


    /// <summary>
    /// TO BE DONE
    /// </summary>
    public class PolygonModel
    {
        /// <summary>
        /// TO BE DONE
        /// </summary>
        public PolygonModel()
        {
            points = new List<Double[]>();
        }

        /// <summary>
        /// TO BE DONE
        /// </summary>
        public virtual IEnumerable<double[]> points { get; set; }
        /// <summary>
        /// TO BE DONE
        /// </summary>
        public string coordinate_system { get; set; }
    }

    /// <summary>
    /// TO BE DONE
    /// </summary>
    public class PolygonDescriptor
    {
        /// <summary>
        /// TO BE DONE
        /// </summary>
        public string WKT { get; set; }
        /// <summary>
        /// TO BE DONE
        /// </summary>
        public int SRID { get; set; }
    }

    /// <summary>
    /// TO BE DONE
    /// </summary>
    public class RequestedEntity
    {
        /// <summary>
        /// TO BE DONE
        /// </summary>
        public string ID { get; set; }
        /// <summary>
        /// TO BE DONE
        /// </summary>
        public string SelectedFormat { get; set; }
        /// <summary>
        /// TO BE DONE
        /// </summary>
        public string SelectedStyle { get; set; }

        /// <summary>
        /// TO BE DONE
        /// </summary>
        public IEnumerable<double[]> Footprint { get; set; }
    }

    /// <summary>
    /// TO BE DONE  
    /// </summary>    
    public class MapInfo
    {
        /// <summary>
        /// TO BE DONE  
        /// </summary>
        public string GetMapURL { get; set; }

        /// <summary>
        /// TO BE DONE
        /// </summary>
        public string GetMapURLQuery { get; set; }

        /// <summary>
        /// TO BE DONE
        /// </summary>
        public string Layers { get; set; }

        /// <summary>
        ///   
        /// </summary>
        public string Version { get; set; }

        /// <summary>
        ///   
        /// </summary>
        public string CoordinateSystem { get; set; }

        /// <summary>
        ///   
        /// </summary>
        public string SelectedFormat { get; set; }

        /// <summary>
        ///   
        /// </summary>
        public string SelectedStyle { get; set; }

        /// <summary>
        ///   
        /// </summary>
        public string Legal { get; set; }

        /// <summary>
        ///   
        /// </summary>
        public IEnumerable<double[]> Footprint { get; set; }
    }

    internal class UsgsAPICategory
    {
        /// <summary>
        ///   
        /// </summary>        
        public string Title { get; set; }

        /// <summary>
        ///   
        /// </summary>
        public string SubTitle { get; set; }

        /// <summary>
        ///   
        /// </summary>
        public string Format { get; set; }

        /// <summary>
        ///   
        /// </summary>
        public int Priority { get; set; }

        /// <summary>
        /// 
        /// </summary>
        public String Type { get; set; }
    }

    /// <summary>
    ///   
    /// </summary>
    public class UsgsRequest : IComparable<UsgsRequest>
    {
        /// <summary>
        ///   
        /// </summary>
        public string Dataset { get; set; }

        /// <summary>
        /// 
        /// </summary>
        public string DatasetID { get; set; }

        /// <summary>
        /// 
        /// </summary>
        public string Format { get; set; }

        /// <summary>
        ///   
        /// </summary>
        public string Category { get; set; }

        /// <summary>
        ///   
        /// </summary>
        public int Priority { get; set; }

        /// <summary>
        ///   
        /// </summary>
        public int CompareTo(UsgsRequest req)
        {
            int comp = Category.CompareTo(req.Category);
            if(comp != 0)
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
        public JToken jtoken { get; set; }

        /// <summary>
        /// 
        /// </summary>
        public string DatasetId { get; set; }
    }

    //internal enum ObjectType
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
