using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Bentley.EC.Persistence.Query;
using Bentley.Exceptions;
using IndexECPlugin.Source;
using IndexECPlugin.Source.Helpers;

namespace IndexECPlugin
    {
    /// <summary>
    /// ECQuery class extension methods
    /// </summary>
    public static class ECQueryExtensions
        {

        /// <summary>
        /// Extracts a polygon descriptor object from a spatial query.
        /// </summary>
        /// <param name="query">The ECQuery object</param>
        /// <returns>The polygon descriptor object</returns>
        public static PolygonDescriptor ExtractPolygonDescriptorFromQuery (this ECQuery query)
            {
            if ( !query.ExtendedData.ContainsKey("polygon") )
                {
                throw new UserFriendlyException("This request must contain a \"polygon\" parameter in the form of a WKT polygon string.");
                }

            string polygonString = query.ExtendedData["polygon"].ToString();
            PolygonModel model = DbGeometryHelpers.CreatePolygonModelFromJson(polygonString);

            string polygonWKT = DbGeometryHelpers.CreateWktPolygonString(model.Points);

            return new PolygonDescriptor
            {
                WKT = polygonWKT,
                SRID = model.coordinate_system
            };

            }
        }
    }
