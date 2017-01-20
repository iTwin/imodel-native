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
    /// Contains extension methods for IECInstance
    /// </summary>
    public static class IECInstanceExtension
        {
        /// <summary>
        /// Initializes all properties of an IECInstance to null
        /// </summary>
        /// <param name="instance">The instance for which we want all properties set to null</param>
        public static void InitializePropertiesToNull (this IECInstance instance)
            {
            if ( instance.ClassDefinition == null)
                {
                return;
                }
            foreach ( IECProperty prop in instance.ClassDefinition )
                {
                instance[prop.Name].SetToNull();
                }
            }
        }
    }
