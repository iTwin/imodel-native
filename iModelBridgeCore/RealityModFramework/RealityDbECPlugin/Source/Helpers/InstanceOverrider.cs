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

        ///// <summary>
        ///// Communicates with the database to fetch the override information for the instances passed to the Override method and replace it in these instances
        ///// </summary>
        ///// <param name="instances"></param>
        ///// <param name="source"></param>
        ///// <param name="sqlConnection"></param>
        ///// <param name="ecQuerySettings"></param>
        ///// <returns></returns>
        //public void Modify(IEnumerable<IECInstance> instances, DataSource source, IDbConnection sqlConnection, ECQuerySettings ecQuerySettings)
        //    {
        //    m_getStream = false;

        //    if (source == DataSource.All)
        //        {
        //        throw new ArgumentException("The source provided to Override cannot be All");
        //        }

        //    //Determine which ECClasses and associated table(s) to query
        //    //Get all instance Ids for each ECClass

        //    Dictionary<IECClass, List<IECInstance>> classIdMap = GetAllClassesAndIdsRecursive(instances);
            

        //    //Create the SQL Query and query the database for each of these classes
            
        //    foreach (IECClass ecClass in classIdMap.Keys)
        //        {
        //        DataReadingHelper drh;
        //        Dictionary<string, Tuple<string, DbType>> paramNameValueMap;
        //        List<IECInstance> instanceList = classIdMap[ecClass];
        //        string query = CreateOverrideSQLQuery(source, instanceList, ecClass, out drh, out paramNameValueMap);

        //        List<IECInstance> overrideInstances = SqlQueryHelpers.QueryDbForInstances(query, drh, paramNameValueMap, ecClass, ecClass, sqlConnection, false);

        //        //For each instance in the 
        //        //If there is data and stream data (such as thumbnail data) set in instances, replace it if there is an override

        //        ApplyModification(instanceList, overrideInstances);
        //        }
        //    }

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
                if ( GetStream )
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

        //private string CreateOverrideSQLQuery (DataSource source, List<IECInstance> classIdList, IECClass ecClass, out DataReadingHelper drh, out Dictionary<string, Tuple<string, DbType>> paramNameValueMap)
        //    {
        //    TableAliasCreator tac = new TableAliasCreator();

        //    IECInstance sqlEntity = ecClass.GetCustomAttributes("SQLEntity");

        //    TableDescriptor table = new TableDescriptor(sqlEntity.GetPropertyValue("OverrideTableName").StringValue, tac.GetNewTableAlias());
        //    StandardSQLQueryBuilder queryBuilder = new StandardSQLQueryBuilder();

        //    queryBuilder.SpecifyFromClause(table);
        //    foreach ( IECProperty prop in ecClass )
        //        {
        //        IECInstance dbColumn = prop.GetCustomAttributes("DBColumn");
        //        IECInstance overrideDBColumn = prop.GetCustomAttributes("OverrideDBColumn");
        //        if (overrideDBColumn == null)
        //            {
        //            //We want to skip this one, since it's not in the database
        //            continue;
        //            }

        //        if ( overrideDBColumn.GetPropertyValue("OverrideJoinTableName") != null )
        //            {
        //            //Is this is to be implemented one day, see the JoinInternalColumn method. Maybe extract this method, add an override mode and use it. 
        //            //Also add FirstTableKey and NewTableKey properties in OverrideDBColumn
        //            throw new NotImplementedException("The use of the OverrideJoinTableName property in the schema is not implemented yet.");
        //            }
        //        TableDescriptor tempTable1 = table;
        //        if ( ecClass != prop.ClassDefinition )
        //            {
        //            tempTable1 = SqlQueryHelpers.JoinBaseTables(ecClass, tempTable1, prop.ClassDefinition, queryBuilder, tac, true);
        //            }

        //        ColumnCategory category = ColumnCategory.instanceData;
                
        //        if ((dbColumn != null) && (dbColumn.GetPropertyValue("IsSpatial") != null) && (!dbColumn.GetPropertyValue("IsSpatial").IsNull) && (dbColumn.GetPropertyValue("IsSpatial").StringValue.ToLower() == "true") )
        //            {
        //            category = ColumnCategory.spatialInstanceData;
        //            }

        //        string overrideColumnName = overrideDBColumn.GetPropertyValue("OverrideColumnName").StringValue;

        //        queryBuilder.AddSelectClause(tempTable1, overrideColumnName, category, prop);

        //        //If this is the Id property, add it to the where clause
        //        if ( prop.Name == sqlEntity.GetPropertyValue("InstanceIDProperty").StringValue )
        //            {
        //            queryBuilder.AddWhereClause(tempTable1.Alias, overrideColumnName, RelationalOperator.IN, String.Join(",", classIdList.Select(inst => inst.InstanceId)), DbType.String);
        //            }
        //        }
        //    queryBuilder.AddOperatorToWhereClause(LogicalOperator.AND);
        //    queryBuilder.AddWhereClause(table.Alias, "SubAPI", RelationalOperator.EQ, SourceStringMap.SourceToString(source), DbType.String);
        //    paramNameValueMap = queryBuilder.paramNameValueMap;
        //    if ( m_getStream )
        //        {
        //        IECInstance fileHolderAttribute = ecClass.GetCustomAttributes("FileHolder");
        //        if ( (fileHolderAttribute != null) )
        //            {
        //            string streamableColumnName = fileHolderAttribute["LocationHoldingColumn"].StringValue;
        //            queryBuilder.AddSelectClause(table, streamableColumnName, ColumnCategory.streamData, null);
        //            }

        //        }

        //    return queryBuilder.BuildQuery(out drh);
        //    }

        //private Dictionary<IECClass, List<IECInstance>> GetAllClassesAndIdsRecursive (IEnumerable<IECInstance> instances)
        //    {
        //    List<IECInstance> relatedInstances = new List<IECInstance>();
        //    Dictionary<IECClass, List<IECInstance>> classIdMap;
        //    foreach ( var instance in instances )
        //        {

        //        foreach( var relationshipInst in instance.GetRelationshipInstances() )
        //            {
        //            if ( !relationshipInst.ExtendedData.Keys.Contains("Overriden") )
        //                {
        //                //This extended data is set to prevent multiple includes of the same instance
        //                relationshipInst.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("Overriden", true));

        //                var relatedInstance = instance == relationshipInst.Source ? relationshipInst.Target : relationshipInst.Source;
        //                relatedInstances.Add(relatedInstance);
        //                }
        //            }
        //        }
        //    if ( relatedInstances.Any() )
        //        {
        //        classIdMap = GetAllClassesAndIdsRecursive(relatedInstances);
        //        if(m_getStream)
        //            {
        //            //There is more than one instance queried in a stream backed query. This method is obsolete...
        //            throw new NotImplementedException("Override of stream backed data not implemented for multiple instances queries");
        //            }
        //        }
        //    else
        //        {
        //        classIdMap = new Dictionary<IECClass, List<IECInstance>>();
        //        }
        //    foreach ( var instance in instances )
        //        {
        //        StreamBackedDescriptor sbd;
        //        if( StreamBackedDescriptorAccessor.TryGetFrom(instance, out sbd) )
        //            {
        //            if((instances.Count() > 1) || (relatedInstances.Count() > 1))
        //                {
        //                //There is more than one instance queried in a stream backed query. This method is obsolete...
        //                throw new NotImplementedException("Override of stream backed data not implemented for multiple instances queries");
        //                }
        //            m_getStream = true;
        //            }

        //        if( instance.ClassDefinition.GetCustomAttributes("SQLEntity").GetPropertyValue("OverrideTableName") == null )
        //            {
        //            //This class cannot be overriden.
        //            //We skip this instance.
        //            continue;
        //            }
        //        List<IECInstance> singleClassIdList;
        //        if(classIdMap.TryGetValue(instance.ClassDefinition, out singleClassIdList))
        //            {
        //            if ( !singleClassIdList.Any(inst => inst.InstanceId == instance.InstanceId) )
        //                {
        //                singleClassIdList.Add(instance);
        //                }
        //            }
        //        else
        //            {
        //            classIdMap.Add(instance.ClassDefinition, new List<IECInstance>() { instance });
        //            }
        //        }

        //    return classIdMap;
            
        //    }
        }
    }
