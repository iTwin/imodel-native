using System;
using System.Collections.Generic;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Bentley.EC.Persistence.Query;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using Bentley.Exceptions;

namespace IndexECPlugin.Source.Helpers
    {

    /// <summary>
    /// Base class for reading mimic tables in the database
    /// </summary>
    public class MimicTableAccessor
        {
        /// <summary>
        /// The name of the property containing the name of the table storing the mimic instances. This property is located in the SQLEntity custom attribute class.
        /// </summary>
        public string MimicTableNameProp
            {
            get;
            private set;
            }

        /// <summary>
        /// The name of the property containing the name of the column mimicking a single property.
        /// </summary>
        public string MimicColumnNameProp
            {
            get;
            private set;
            }

        /// <summary>
        /// The name of the property containing the name of a table to join to access a property that is not stored in the main table of a class. Currently not implemented
        /// </summary>
        public string MimicJoinTableNameProp
            {
            get;
            private set;
            }

        /// <summary>
        /// The UniqueId column name of the mimic table. The Unique Id Column is only used here for joining a base table to the derived one. It can be null if joinBaseTable is never used in the mimic.
        /// </summary>
        public string MimicUniqueIdColumn
            {
            get;
            private set;
            }

        /// <summary>
        /// The property indicating if we have to query or if we have queried stream (binary) data from the database. Will be true if an instance to mimic contains stream data. Is relevant when
        /// CanMimicStreamData is set to true.
        /// </summary>
        public bool GetStream
            {
            get;
            set;
            }

        /// <summary>
        /// Property set at the creation of the accessor to indicate if it can mimic stream data of instances to mimic.
        /// </summary>
        public bool CanMimicStreamData
            {
            get;
            private set;
            }

        /// <summary>
        /// Constant indicating the name of the custom attribute containing the information about the mimic tables
        /// </summary>
        public const string MimicDbColumnCustomClassName = "MimicDBColumn";

                /// <summary>
        /// Constructor for instance mimic accessor. Sets the names of the properties of custom attributes used to query the database.
        /// </summary>
        /// <param name="canMimicStreamData">Sets the CanModifyStreamData property.</param>
        /// <param name="mimicTableNameProp">Sets the ModifierTableNameProp</param>
        /// <param name="mimicColumnNameProp">Sets the ModifierColumnNameProp</param>
        /// <param name="mimicJoinTableNameProp">Sets the ModifierJoinTableNameProp</param>
        /// <param name="mimicUniqueIdColumn">Sets the ModifierUniqueIdColumn</param>
        public MimicTableAccessor (bool canMimicStreamData, string mimicTableNameProp, string mimicColumnNameProp, string mimicJoinTableNameProp, string mimicUniqueIdColumn)
            {
            CanMimicStreamData = canMimicStreamData;
            MimicTableNameProp = mimicTableNameProp;
            MimicColumnNameProp = mimicColumnNameProp;
            MimicJoinTableNameProp = mimicJoinTableNameProp;
            MimicUniqueIdColumn = mimicUniqueIdColumn;
            }

        /// <summary>
        /// Creates an sql query command to retrieve a set of mimicked instances.
        /// </summary>
        /// <param name="source">The source of the instances mimicked</param>
        /// <param name="classIdList">The ids of the instances to retrieve</param>
        /// <param name="ecClass">The class of the instances retrieved</param>
        /// <param name="propertyList">The properties to retrieve</param>
        /// <param name="drh">The data reading helper object. Used by the database reading method.</param>
        /// <param name="paramNameValueMap">The parameter name value map object. Used by the database reading method</param>
        /// <param name="nonInstanceDataColumnList">Additional columns to query and that are not contained in properties. These columns must be in the ecClass' sql table. This data will be put in the extended data of the instances.</param>
        /// <returns>The sql query. </returns>
        public string CreateMimicSQLQuery (DataSource source, IEnumerable<string> classIdList, IECClass ecClass, IEnumerable<IECProperty> propertyList, out DataReadingHelper drh, out IParamNameValueMap paramNameValueMap, IEnumerable<string> nonInstanceDataColumnList = null)
            {

            TableAliasCreator tac = new TableAliasCreator();

            IECInstance sqlEntity = ecClass.GetCustomAttributes("SQLEntity");

            TableDescriptor table = new TableDescriptor(sqlEntity.GetPropertyValue(MimicTableNameProp).StringValue, tac.GetNewTableAlias());
            StandardSQLQueryBuilder queryBuilder = new StandardSQLQueryBuilder();

            queryBuilder.SpecifyFromClause(table);

            SetSelectClause(ecClass, propertyList, tac, table, queryBuilder, nonInstanceDataColumnList);

            IECProperty idProp = ecClass.FindProperty(sqlEntity.GetPropertyValue("InstanceIDProperty").StringValue);
            IECInstance mimicIdDBColumn = idProp.GetCustomAttributes(MimicDbColumnCustomClassName);
            string mimicIdColumnName = mimicIdDBColumn.GetPropertyValue(MimicColumnNameProp).StringValue;

            queryBuilder.AddWhereClause(table.Alias, mimicIdColumnName, RelationalOperator.IN, String.Join(",", classIdList), DbType.String);
            queryBuilder.AddOperatorToWhereClause(LogicalOperator.AND);

            queryBuilder.AddWhereClause(table.Alias, "SubAPI", RelationalOperator.EQ, SourceStringMap.SourceToString(source), DbType.String);
            paramNameValueMap = queryBuilder.paramNameValueMap;
            if ( GetStream && CanMimicStreamData )
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

        private void SetSelectClause (IECClass ecClass, IEnumerable<IECProperty> propertyList, TableAliasCreator tac, TableDescriptor table, StandardSQLQueryBuilder queryBuilder, IEnumerable<string> nonInstanceDataColumnList)
            {
            foreach ( IECProperty prop in propertyList )
                {
                IECInstance dbColumn = prop.GetCustomAttributes("DBColumn");
                IECInstance mimicDBColumn = prop.GetCustomAttributes(MimicDbColumnCustomClassName);
                if ( (mimicDBColumn == null) || (mimicDBColumn.GetPropertyValue(MimicColumnNameProp) == null) )
                    {
                    //We want to skip this one, since it's not in the database
                    continue;
                    }

                if ( mimicDBColumn.GetPropertyValue(MimicJoinTableNameProp) != null )
                    {
                    //Is this is to be implemented one day, see the JoinInternalColumn method. Maybe extract this method, add a mimic mode and use it. 
                    //Also add FirstTableKey and NewTableKey properties in MimicDBColumn
                    throw new NotImplementedException(String.Format("The use of the {0} property in the schema is not implemented yet.", MimicJoinTableNameProp));
                    }
                TableDescriptor tempTable1 = table;
                if ( ecClass != prop.ClassDefinition )
                    {
                    tempTable1 = SqlQueryHelpers.JoinBaseTables(ecClass, tempTable1, prop.ClassDefinition, queryBuilder, tac, MimicTableNameProp, MimicUniqueIdColumn);
                    }

                ColumnCategory category = ColumnCategory.instanceData;

                if ( (dbColumn != null) && (dbColumn.GetPropertyValue("IsSpatial") != null) && (!dbColumn.GetPropertyValue("IsSpatial").IsNull) && (dbColumn.GetPropertyValue("IsSpatial").StringValue.ToLower() == "true") )
                    {
                    category = ColumnCategory.spatialInstanceData;
                    }

                string mimicColumnName = mimicDBColumn.GetPropertyValue(MimicColumnNameProp).StringValue;

                queryBuilder.AddSelectClause(tempTable1, mimicColumnName, category, prop);

                }
            if ( nonInstanceDataColumnList != null )
                {
                foreach ( string columnName in nonInstanceDataColumnList )
                    {
                    queryBuilder.AddSelectClause(table, columnName, ColumnCategory.nonPropertyData, null);
                    }
                }
            }

        /// <summary>
        /// Creates an sql query command to retrieve mimicked instances inside a spatial boundary.
        /// </summary>
        /// <param name="source">The source of the instances mimicked</param>
        /// <param name="polygonDescriptor">The polygon in which to search the instances.</param>
        /// <param name="ecClass">The class of the instances retrieved</param>
        /// <param name="propertyList">The properties to retrieve</param>
        /// <param name="drh">The data reading helper object. Used by the database reading method.</param>
        /// <param name="paramNameValueMap">The parameter name value map object. Used by the database reading method</param>
        /// <param name="nonInstanceDataColumnList">Additional columns to query and that are not contained in properties. This data will be put in the extended data of the instances.</param>
        /// <param name="whereCriteriaList">The where criteria List, in the form of a list of criterias linked by AND logical operators</param>
        /// <returns>The sql query.</returns>
        public string CreateMimicSQLSpatialQuery (DataSource source, PolygonDescriptor polygonDescriptor, IECClass ecClass, IEnumerable<IECProperty> propertyList, out DataReadingHelper drh, out IParamNameValueMap paramNameValueMap, IEnumerable<string> nonInstanceDataColumnList = null, List<SingleWhereCriteriaHolder> whereCriteriaList = null)
            {
            TableAliasCreator tac = new TableAliasCreator();

            IECInstance sqlEntity = ecClass.GetCustomAttributes("SQLEntity");

            TableDescriptor table = new TableDescriptor(sqlEntity.GetPropertyValue(MimicTableNameProp).StringValue, tac.GetNewTableAlias());
            StandardSQLQueryBuilder queryBuilder = new StandardSQLQueryBuilder();

            queryBuilder.SpecifyFromClause(table);

            SetSelectClause(ecClass, propertyList, tac, table, queryBuilder, nonInstanceDataColumnList);

            extractSpatialWhereClause(ecClass, table, queryBuilder, polygonDescriptor, tac);

            paramNameValueMap = queryBuilder.paramNameValueMap;

            queryBuilder.AddOperatorToWhereClause(LogicalOperator.AND);

            queryBuilder.AddWhereClause(table.Alias, "SubAPI", RelationalOperator.EQ, SourceStringMap.SourceToString(source), DbType.String);

            if(whereCriteriaList != null)
                {
                foreach (SingleWhereCriteriaHolder whereCriteria in whereCriteriaList)
                    {
                    var mimicDbColumnAttribute = whereCriteria.Property.GetCustomAttributes(MimicDbColumnCustomClassName);
                    if(mimicDbColumnAttribute.GetPropertyValue(MimicJoinTableNameProp) != null)
                        {
                        throw new NotImplementedException("CreateMimicSQLSpatialQuery does not support JoinTables in the where criteria List.");
                        }
                    if(mimicDbColumnAttribute != null)
                        {
                        var propval = mimicDbColumnAttribute.GetPropertyValue(MimicColumnNameProp);
                        if(propval != null && !propval.IsNull)
                            {
                            queryBuilder.AddOperatorToWhereClause(LogicalOperator.AND);
                            queryBuilder.AddWhereClause(table.Alias, propval.StringValue, whereCriteria.Operator, whereCriteria.Value, ECToSQLMap.ECTypeToDbType(whereCriteria.Property.Type));
                            }
                        }
                    }
                }

            paramNameValueMap = queryBuilder.paramNameValueMap;
            if ( GetStream && CanMimicStreamData )
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

            private void extractSpatialWhereClause (IECClass queriedClass, TableDescriptor tableToJoin, SQLQueryBuilder sqlQueryBuilder, PolygonDescriptor polygonDescriptor, TableAliasCreator tac)
            {

            TableDescriptor propertyTable = tableToJoin;
            string columnName = null;
            foreach ( var prop in queriedClass )
                {
                IECInstance dbColumn = prop.GetCustomAttributes("DBColumn");
                if ( dbColumn == null )
                    {
                    //This is not a column in the sql table. We skip it...
                    continue;
                    }
                var isSpatialProperty = dbColumn["IsSpatial"];
                if ( !isSpatialProperty.IsNull && isSpatialProperty.StringValue.ToLower() == "true" )
                    {
                    columnName = dbColumn.GetPropertyValue("ColumnName").StringValue;

                    if ( queriedClass != prop.ClassDefinition )
                        {
                        propertyTable = SqlQueryHelpers.JoinBaseTables(queriedClass, propertyTable, prop.ClassDefinition, sqlQueryBuilder, tac);
                        }

                    break;
                    }
                }
            if ( String.IsNullOrWhiteSpace(columnName) )
                {
                throw new UserFriendlyException("The class for which you requested a spatial query does not have any spatial property.");
                }

            sqlQueryBuilder.AddSpatialIntersectsWhereClause(propertyTable.Alias, columnName, polygonDescriptor.WKT, polygonDescriptor.SRID);

            }

        }
    }
