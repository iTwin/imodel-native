/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/Helpers/InstanceOverrider.cs $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/

using System;
using System.Collections.Generic;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Bentley.EC.Persistence;
using Bentley.EC.Persistence.Query;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using Bentley.Exceptions;

namespace IndexECPlugin.Source.Helpers
    {

    /// <summary>
    /// The instance overrider communicates with the database to fetch the override information for the instances passed to the Override method and replace it in these instances
    /// </summary>
    public class InstanceOverrider : InstanceModifier
        {

        //This boolean is used to indicate if we have to override stream data (for now, only thumbnails). If more classes gain streambackable policy and if more than one instance
        //can be queried with stream data (this should not happen in WSG), this boolean will have to be replaced and our method updated.
        //bool m_getStream;

        /// <summary>
        /// InstanceOverrider constructor
        /// </summary>
        public InstanceOverrider()
            : base(true, "OverrideTableName", "OverrideColumnName", "OverrideJoinTableName", "OverrideTableUniqueIdColumn")
            {
            }

        /// <summary>
        /// Applies the override to the original instances. This will take the properties in override instances and replace the corresponding properties 
        /// of the original instances
        /// </summary>
        /// <param name="instanceList">The list containing the original instances</param>
        /// <param name="overrideInstances">The list containing the override instances</param>
        public override void ApplyModification (List<IECInstance> instanceList, List<IECInstance> overrideInstances)
            {
            foreach ( IECInstance overrideInstance in overrideInstances )
                {
                IECInstance instanceToModify = instanceList.Find(inst => inst.InstanceId == overrideInstance.InstanceId);
                if ( instanceToModify == null )
                    {
                    throw new ProgrammerException("There was no instance matching the override instance.");
                    }
                foreach ( IECPropertyValue propValue in overrideInstance )
                    {
                    object value;
                    IECPropertyValue propValueToModify = instanceToModify.GetPropertyValue(propValue.Property.Name);
                    if ( (propValue.TryGetNativeValue(out value)) && (propValueToModify != null) )
                        {
                        instanceToModify.GetPropertyValue(propValue.Property.Name).NativeValue = value;
                        }
                    }
                if ( MimicTableAccessor.GetStream )
                    {
                    StreamBackedDescriptor desc;
                    if ( StreamBackedDescriptorAccessor.TryGetFrom(overrideInstance, out desc) )
                        {
                        StreamBackedDescriptorAccessor.RemoveFrom(instanceToModify);
                        StreamBackedDescriptorAccessor.SetIn(instanceToModify, desc);
                        }
                    }
                }
            }
        }
    }
