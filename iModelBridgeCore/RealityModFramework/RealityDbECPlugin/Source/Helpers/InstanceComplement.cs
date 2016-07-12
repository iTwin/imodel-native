/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/Helpers/InstanceComplement.cs $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using Bentley.Exceptions;

namespace IndexECPlugin.Source.Helpers
    {
    /// <summary>
    /// The instance overrider communicates with the database to fetch the override information for the instances passed to the Override method and add it to the information already present 
    /// in these instances
    /// </summary>
    public class InstanceComplement : InstanceModifier
        {
        /// <summary>
        /// InstanceComplement constructor
        /// </summary>
        public InstanceComplement (IDbQuerier dbQuerier)
            : base(false, "ComplementTableName", "ComplementColumnName", "ComplementJoinTableName", "ComplementTableUniqueIdColumn", dbQuerier)
            {
            }

        /// <summary>
        /// Applies the complement to the original instances. This will take the properties in complement instances and replace the corresponding properties 
        /// of the original instances
        /// </summary>
        /// <param name="instanceList">The list containing the original instances</param>
        /// <param name="complementInstances">The list containing the complement instances</param>
        public override void ApplyModification (List<IECInstance> instanceList, List<IECInstance> complementInstances)
            {
            foreach ( IECInstance overrideInstance in complementInstances )
                {
                IECInstance instanceToModify = instanceList.Find(inst => inst.InstanceId == overrideInstance.InstanceId);
                if ( instanceToModify == null )
                    {
                    throw new ProgrammerException("There was no instance matching the override instance.");
                    }

                string instanceIdPropertyName = overrideInstance.ClassDefinition.GetCustomAttributes("SQLEntity").GetPropertyValue("InstanceIDProperty").StringValue;

                foreach ( IECPropertyValue propValue in overrideInstance )
                    {
                    if (propValue.Property.Name == instanceIdPropertyName)
                        {
                        //We don't want to modify the ID
                        continue;
                        }
                    if ( ECTypeHelper.IsString(propValue.Type) == false )
                        {
                        throw new ProgrammerException("Complement is only allowed on string properties");
                        }
                    IECPropertyValue propValueToModify = instanceToModify.GetPropertyValue(propValue.Property.Name);
                    string value;
                    if ( propValue.TryGetStringValue(out value) && (propValueToModify != null) )
                        {
                        string s;
                        if ( propValueToModify.TryGetStringValue(out s))
                            {
                            //We can't use the out string because it's not actually the string in the instance
                            propValueToModify.StringValue += ", " + value;
                            }
                        else
                            {
                            propValueToModify.StringValue = value;
                            }
                        }
                    }
                }
            }
        }
    }
