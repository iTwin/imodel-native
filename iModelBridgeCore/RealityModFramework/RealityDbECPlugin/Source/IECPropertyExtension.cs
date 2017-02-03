using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;

namespace IndexECPlugin.Source
    {
    /// <summary>
    /// Contains extension methods for IECProperty
    /// </summary>
    public static class IECPropertyExtension
        {
        /// <summary>
        /// Checks if the property is a spatial property (defined in the RealityDbECPlugin ECSchema)
        /// </summary>
        /// <param name="prop">The ECProperty</param>
        /// <returns>True if the property is spatial, false otherwise.</returns>
        public static bool IsSpatial(this IECProperty prop)
            {
            IECInstance dbColumn = prop.GetCustomAttributes("DBColumn");
            return ((dbColumn != null) && (dbColumn.GetPropertyValue("IsSpatial") != null) && (!dbColumn.GetPropertyValue("IsSpatial").IsNull) && (dbColumn.GetPropertyValue("IsSpatial").StringValue.ToLower() == "true"));
            }
        }
    }
