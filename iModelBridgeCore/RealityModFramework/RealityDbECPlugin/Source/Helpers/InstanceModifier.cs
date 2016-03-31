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

        /// <summary>
        /// The name of the property containing the name of the table storing the modifying instances. This property is located in the SQLEntity custom attribute class.
        /// </summary>
        protected string ModifierTableNameProp
            {
            get;
            private set;
            }

        /// <summary>
        /// The name of the property containing the name of the column storing the modifier for a single property.
        /// </summary>
        protected string ModifierColumnNameProp
            {
            get;
            private set;
            }

        /// <summary>
        /// The name of the property containing the name of a table to join to access a property that is not stored in the main table of a class. Currently not implemented
        /// </summary>
        protected string ModifierJoinTableNameProp
            {
            get;
            private set;
            }

        /// <summary>
        /// The UniqueId column name of the modifier table
        /// </summary>
        protected string ModifierUniqueIdColumn
            {
            get;
            private set;
            }
        
        /// <summary>
        /// The property indicating if we have to query or if we have queried stream (binary) data from the database. Will be true if an instance to modify contains stream data. Is relevant when
        /// CanModifyStreamData is set to true.
        /// </summary>
        protected bool GetStream
            {
            get;
            private set;
            }

        /// <summary>
        /// Property set at the creation of the modifier to indicate if it can modify stream data of instances to modify.
        /// </summary>
        protected bool CanModifyStreamData
            {
            get;
            private set;
            }

        private const string ModifierDbColumn = "ModifierDBColumn";

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
            CanModifyStreamData = canModifyStreamData;
            ModifierTableNameProp = modifierTableNameProp;
            ModifierColumnNameProp = modifierColumnNameProp;
            ModifierJoinTableNameProp = modifierJoinTableNameProp;
            ModifierUniqueIdColumn = modifierUniqueIdColumn;
            }

        /// <summary>
        /// Communicates with the database to fetch the modifying information for the instances passed this method and modify thses instances accordingly to the ApplyModification method
        /// </summary>
        /// <param name="instances"></param>
        /// <param name="source"></param>
        /// <param name="sqlConnection"></param>
        /// <param name="ecQuerySettings"></param>
        /// <returns></returns>
        public void Modify (IEnumerable<IECInstance> instances, DataSource source, IDbConnection sqlConnection, ECQuerySettings ecQuerySettings)
            {
            GetStream = false;

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
                Dictionary<string, Tuple<string, DbType>> paramNameValueMap;
                List<IECInstance> instanceList = classIdMap[ecClass];
                string query = CreateModifierSQLQuery(source, instanceList, ecClass, out drh, out paramNameValueMap);

                List<IECInstance> modifyInstances = SqlQueryHelpers.QueryDbForInstances(query, drh, paramNameValueMap, ecClass, drh.GetProperties(), sqlConnection, false);

                //For each instance in the 
                //If there is data and stream data (such as thumbnail data) set in instances, replace it if there is an override

                ApplyModification(instanceList, modifyInstances);
                }
            }

        private string CreateModifierSQLQuery (DataSource source, List<IECInstance> classIdList, IECClass ecClass, out DataReadingHelper drh, out Dictionary<string, Tuple<string, DbType>> paramNameValueMap)
            {
            
            TableAliasCreator tac = new TableAliasCreator();

            IECInstance sqlEntity = ecClass.GetCustomAttributes("SQLEntity");

            TableDescriptor table = new TableDescriptor(sqlEntity.GetPropertyValue(ModifierTableNameProp).StringValue, tac.GetNewTableAlias());
            StandardSQLQueryBuilder queryBuilder = new StandardSQLQueryBuilder();

            queryBuilder.SpecifyFromClause(table);

            bool idWhereClauseAdded = false;
            
            foreach ( IECProperty prop in ecClass )
                {
                IECInstance dbColumn = prop.GetCustomAttributes("DBColumn");
                IECInstance modifierDBColumn = prop.GetCustomAttributes(ModifierDbColumn);
                if ( (modifierDBColumn == null) || (modifierDBColumn.GetPropertyValue(ModifierColumnNameProp) == null) )
                    {
                    //We want to skip this one, since it's not in the database
                    continue;
                    }

                if ( modifierDBColumn.GetPropertyValue(ModifierJoinTableNameProp) != null )
                    {
                    //Is this is to be implemented one day, see the JoinInternalColumn method. Maybe extract this method, add a modifier mode and use it. 
                    //Also add FirstTableKey and NewTableKey properties in ModifierDBColumn
                    throw new NotImplementedException(String.Format("The use of the {0} property in the schema is not implemented yet.", ModifierJoinTableNameProp));
                    }
                TableDescriptor tempTable1 = table;
                if ( ecClass != prop.ClassDefinition )
                    {
                    tempTable1 = SqlQueryHelpers.JoinBaseTables(ecClass, tempTable1, prop.ClassDefinition, queryBuilder, tac, ModifierTableNameProp, ModifierUniqueIdColumn);
                    }

                ColumnCategory category = ColumnCategory.instanceData;

                if ( (dbColumn != null) && (dbColumn.GetPropertyValue("IsSpatial") != null) && (!dbColumn.GetPropertyValue("IsSpatial").IsNull) && (dbColumn.GetPropertyValue("IsSpatial").StringValue.ToLower() == "true") )
                    {
                    category = ColumnCategory.spatialInstanceData;
                    }

                string modifierColumnName = modifierDBColumn.GetPropertyValue(ModifierColumnNameProp).StringValue;

                queryBuilder.AddSelectClause(tempTable1, modifierColumnName, category, prop);

                //If this is the Id property, add it to the where clause
                if ( prop.Name == sqlEntity.GetPropertyValue("InstanceIDProperty").StringValue )
                    {
                    queryBuilder.AddWhereClause(tempTable1.Alias, modifierColumnName, RelationalOperator.IN, String.Join(",", classIdList.Select(inst => inst.InstanceId)), DbType.String);
                    queryBuilder.AddOperatorToWhereClause(LogicalOperator.AND);
                    idWhereClauseAdded = true;
                    }
                }

            if( !idWhereClauseAdded )
                {
                throw new ProgrammerException("The id where clause should have been added at this point");
                }

            queryBuilder.AddWhereClause(table.Alias, "SubAPI", RelationalOperator.EQ, SourceStringMap.SourceToString(source), DbType.String);
            paramNameValueMap = queryBuilder.paramNameValueMap;
            if ( GetStream && CanModifyStreamData )
                {
                IECInstance fileHolderAttribute = ecClass.GetCustomAttributes("FileHolder");
                if ( (fileHolderAttribute != null) )
                    {
                    string streamableColumnName = fileHolderAttribute["LocationHoldingColumn"].StringValue;
                    queryBuilder.AddSelectClause(table, streamableColumnName, ColumnCategory.streamData, null);
                    }

                }

            return queryBuilder.BuildQuery(out drh);
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
                if ( GetStream && CanModifyStreamData )
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
                    if ( CanModifyStreamData )
                        {
                        GetStream = true;
                        }
                    }

                if ( instance.ClassDefinition.GetCustomAttributes("SQLEntity").GetPropertyValue(ModifierTableNameProp) == null )
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
