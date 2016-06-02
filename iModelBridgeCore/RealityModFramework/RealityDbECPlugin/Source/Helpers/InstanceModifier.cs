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
    /// Base class for instance modifiers
    /// </summary>
    abstract public class InstanceModifier
        {

        MimicTableAccessor m_mimicTableAccessor;

        /// <summary>
        /// The mimicTableAccessor object.
        /// </summary>
        protected MimicTableAccessor MimicTableAccessor
            {
            get
                {
                return m_mimicTableAccessor;
                }
            }

        /// <summary>
        /// Constructor for instance modifier. Sets the names of the properties of custom attributes used to query the database.
        /// </summary>
        /// <param name="canModifyStreamData">Sets the CanModifyStreamData property.</param>
        /// <param name="modifierTableNameProp">Sets the ModifierTableNameProp</param>
        /// <param name="modifierColumnNameProp">Sets the ModifierColumnNameProp</param>
        /// <param name="modifierJoinTableNameProp">Sets the ModifierJoinTableNameProp</param>
        /// <param name="modifierUniqueIdColumn">Sets the ModifierUniqueIdColumn</param>
        protected InstanceModifier (bool canModifyStreamData, string modifierTableNameProp, string modifierColumnNameProp, string modifierJoinTableNameProp, string modifierUniqueIdColumn)
            {
            m_mimicTableAccessor = new MimicTableAccessor(canModifyStreamData, modifierTableNameProp, modifierColumnNameProp, modifierJoinTableNameProp, modifierUniqueIdColumn);
            }

        /// <summary>
        /// Communicates with the database to fetch the modifying information for the instances passed this method and modify thses instances accordingly to the ApplyModification method
        /// </summary>
        /// <param name="instances"></param>
        /// <param name="source"></param>
        /// <param name="sqlConnection"></param>
        /// <returns></returns>
        public void Modify (IEnumerable<IECInstance> instances, DataSource source, IDbConnection sqlConnection/*, ECQuerySettings ecQuerySettings*/)
            {
            m_mimicTableAccessor.GetStream = false;

            if ( source == DataSource.All )
                {
                throw new ArgumentException("The source provided to Modify cannot be All");
                }

            //Determine which ECClasses and associated table(s) to query
            //Get all instance Ids for each ECClass

            Dictionary<IECClass, List<IECInstance>> classIdMap = GetAllClassesAndIdsRecursive(instances);


            //Create the SQL Query and query the database for each of these classes

            foreach ( IECClass ecClass in classIdMap.Keys )
                {
                DataReadingHelper drh;
                IParamNameValueMap paramNameValueMap;
                List<IECInstance> instanceList = classIdMap[ecClass];
                string query = m_mimicTableAccessor.CreateMimicSQLQuery(source, instanceList.Select(instance => instance.InstanceId), ecClass, ecClass, out drh, out paramNameValueMap, null);

                List<IECInstance> modifyInstances = SqlQueryHelpers.QueryDbForInstances(query, drh, paramNameValueMap, ecClass, drh.GetProperties(), sqlConnection, null, false);

                //For each instance in the 
                //If there is data and stream data (such as thumbnail data) set in instances, replace it if there is an override

                ApplyModification(instanceList, modifyInstances);
                }
            }

        private Dictionary<IECClass, List<IECInstance>> GetAllClassesAndIdsRecursive (IEnumerable<IECInstance> instances)
            {
            List<IECInstance> relatedInstances = new List<IECInstance>();
            Dictionary<IECClass, List<IECInstance>> classIdMap;
            foreach ( var instance in instances )
                {

                foreach ( var relationshipInst in instance.GetRelationshipInstances() )
                    {
                    if ( !relationshipInst.ExtendedData.Keys.Contains("Modified") )
                        {
                        //This extended data is set to prevent multiple includes of the same instance
                        relationshipInst.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("Modified", true));

                        var relatedInstance = instance == relationshipInst.Source ? relationshipInst.Target : relationshipInst.Source;
                        relatedInstances.Add(relatedInstance);
                        }
                    }
                }
            if ( relatedInstances.Any() )
                {
                classIdMap = GetAllClassesAndIdsRecursive(relatedInstances);
                if ( m_mimicTableAccessor.GetStream && m_mimicTableAccessor.CanMimicStreamData )
                    {
                    //There is more than one instance queried in a stream backed query. This method is obsolete...
                    throw new NotImplementedException("Modification of stream backed data not implemented for multiple instances queries");
                    }
                }
            else
                {
                classIdMap = new Dictionary<IECClass, List<IECInstance>>();
                }
            foreach ( var instance in instances )
                foreach ( var relationshipInst in instance.GetRelationshipInstances() )
                    {
                    //We clean up the extended data we added before.
                    if ( relationshipInst.ExtendedData.Keys.Contains("Modified") )
                        {
                        relationshipInst.ExtendedData.Remove("Modified");
                        }
                    }
            foreach ( var instance in instances )
                {
                StreamBackedDescriptor sbd;
                if ( StreamBackedDescriptorAccessor.TryGetFrom(instance, out sbd) )
                    {
                    if ( (instances.Count() > 1) || (relatedInstances.Count() > 1) )
                        {
                        //There is more than one instance queried in a stream backed query. This method is obsolete...
                        throw new NotImplementedException("Modification of stream backed data not implemented for multiple instances queries");
                        }
                    if ( m_mimicTableAccessor.CanMimicStreamData )
                        {
                        m_mimicTableAccessor.GetStream = true;
                        }
                    }

                if ( instance.ClassDefinition.GetCustomAttributes("SQLEntity").GetPropertyValue(m_mimicTableAccessor.MimicTableNameProp) == null )
                    {
                    //This class cannot be modified.
                    //We skip this instance.
                    continue;
                    }
                List<IECInstance> singleClassIdList;
                if ( classIdMap.TryGetValue(instance.ClassDefinition, out singleClassIdList) )
                    {
                    if ( !singleClassIdList.Any(inst => inst.InstanceId == instance.InstanceId) )
                        {
                        singleClassIdList.Add(instance);
                        }
                    }
                else
                    {
                    classIdMap.Add(instance.ClassDefinition, new List<IECInstance>() { instance });
                    }
                }

            return classIdMap;

            }

        /// <summary>
        /// Method determining how to modify the original instances according to the modifying instances that were retrieved from the database.
        /// </summary>
        /// <param name="instanceList">The original instances</param>
        /// <param name="modifyInstances">The modifying instances</param>
        public abstract void ApplyModification (List<IECInstance> instanceList, List<IECInstance> modifyInstances);
        }
    }
