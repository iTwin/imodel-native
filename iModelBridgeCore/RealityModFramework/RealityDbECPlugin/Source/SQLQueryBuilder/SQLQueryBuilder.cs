/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/SQLQueryBuilder/SQLQueryBuilder.cs $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/

using Bentley.EC.Persistence.Query;
using Bentley.ECObjects.Schema;
using Bentley.Exceptions;
using IndexECPlugin.Source.Helpers;
using System;
using System.Collections.Generic;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IndexECPlugin.Source
    {
    /// <summary>
    /// This class is used to build SQL queries by specifying each part of the query.
    /// </summary>
    public abstract class SQLQueryBuilder
        {
        /// <summary>
        /// The SELECT clause of the SQL query
        /// </summary>
        protected List<string> m_sqlSelectClause;

        /// <summary>
        /// The FROM clause of the SQLQuery
        /// </summary>
        protected TableDescriptor m_sqlFromClause;

        /// <summary>
        /// The LeftJoin clause of the SQLQuery
        /// </summary>
        protected List<TableDescriptor> m_sqlLeftJoinClause;

        //TODO : Simplify the use of the whereClause Part of the builder. 
        //       It is the most complex and low level part. It would be nice
        //       to have a way to manage automatically the inner clauses

        /// <summary>
        /// The Where clause of the SQLQuery
        /// </summary>
        protected string m_sqlWhereClause;

        /// <summary>
        /// The OrderBy clause of the SQLQuery
        /// </summary>
        protected List<string> m_sqlOrderByClause;

        /// <summary>
        /// The dataReadingHelper describing what information can be found in each queried column
        /// </summary>
        protected DataReadingHelper m_dataReadingHelper = new DataReadingHelper();

        private GenericParamNameValueMap m_paramNameValueMap;

        private int m_paramNumber;

        /// <summary>
        /// SQLQueryBuilder constructor
        /// </summary>
        protected SQLQueryBuilder ()
            {
            m_paramNameValueMap = new GenericParamNameValueMap();
            m_paramNumber = 0;

            m_sqlSelectClause = new List<string>();
            m_sqlLeftJoinClause = new List<TableDescriptor>();
            m_sqlWhereClause = "";
            m_sqlOrderByClause = new List<string>();
            }

        /// <summary>
        /// Add a column to the select clause
        /// </summary>
        /// <param name="table">The table descriptor containing the information about the table in which is held the selected column</param>
        /// <param name="columnName">The name of the column to add</param>
        /// <param name="columnCategory">The category of the requested data</param>
        /// <param name="property">The property associated to the data. Can be null if the category is not instanceData or spatialInstanceData</param>
        public void AddSelectClause (TableDescriptor table, string columnName, ColumnCategory columnCategory, IECProperty property/*, IECClass ecClass*/)
            {
            if ( columnCategory == ColumnCategory.spatialInstanceData )
                {
                m_sqlSelectClause.Add(AddBrackets(table.Alias) + "." + AddBrackets(columnName) + ".STAsText()");
                m_sqlSelectClause.Add(AddBrackets(table.Alias) + "." + AddBrackets(columnName) + ".STSrid");
                m_dataReadingHelper.AddColumn(columnCategory, property, 2);
                }
            else if (columnCategory == ColumnCategory.nonPropertyData)
                {
                m_sqlSelectClause.Add(AddBrackets(table.Alias) + "." + AddBrackets(columnName));
                m_dataReadingHelper.AddNonPropertyDataColumn(columnName);
                }
            else
                {
                m_sqlSelectClause.Add(AddBrackets(table.Alias) + "." + AddBrackets(columnName));
                m_dataReadingHelper.AddColumn(columnCategory, property, 1);
                }
            
            }

        /// <summary>
        /// Sets the From clause. Replaces the old From clause if it has already been set.
        /// </summary>
        /// <param name="table">The table from which we select from</param>
        public void SpecifyFromClause (TableDescriptor table)
            {

            if ( table.TableKey != null || table.FirstTableKey != null || table.FirstTable != null )
                {
                throw new ProgrammerException("From clause table key parameters and joined table should not be set.");
                }

            //m_sqlFromClause = AddBrackets(table.Name) + " " + AddBrackets(table.Alias);
            m_sqlFromClause = table;
            }

        /// <summary>
        /// Attempts to join the TableDescriptor to the list of tables to join. If a similar table has already
        /// been joined, we do not join the addedTable and we return the similar table. The appropriate table (and its alias)
        /// can then be used after calling this method
        /// </summary>
        /// <param name="addedTable"> The table to add. </param>
        /// <param name="similarTable"> If the join was not successful, this will contain the similar table found in the query. Otherwise, it is null. </param>
        /// <returns>If the join is successful, true, if not, false. </returns>
        public bool AddLeftJoinClause (TableDescriptor addedTable, out TableDescriptor similarTable)
            {

            if ( addedTable.TableKey == null || addedTable.FirstTableKey == null || addedTable.FirstTable == null )
                {
                throw new ProgrammerException("Joined table parameters should have been set before the join.");
                }

            similarTable = m_sqlLeftJoinClause.FirstOrDefault(td => td.IsEqualTo(addedTable));

            if ( similarTable == null )
                {
                //Normally, this if block should never be true, because the fromClause table should not have any keys set,
                //and we just verified that at the beginning of the method
                //if(addedTable.IsEqualTo(m_sqlFromClause))
                //{
                //    similarTable = m_sqlFromClause;
                //    return false;
                //}

                m_sqlLeftJoinClause.Add(addedTable);
                return true;
                }
            return false;

            //m_sqlLeftJoinClause.Add("LEFT JOIN " + AddBrackets(tableName) + " ON " + AddBrackets(foreignKey) + " = " + AddBrackets(tableName) + "." + AddBrackets(primaryKey));
            }

        /// <summary>
        /// Adds an operator (logical Or, logical And) between two conditions of a where clause
        /// </summary>
        /// <param name="op"></param>
        public void AddOperatorToWhereClause (LogicalOperator op)
            {
            if ( op == LogicalOperator.OR )
                {
                m_sqlWhereClause += "OR ";
                }
            else
                {
                m_sqlWhereClause += "AND ";
                }
            }

        /// <summary>
        /// Starts an inner where clause by adding an opening parenthese in the where clause. This is useful for creating complex
        /// where clauses. It is necessary to end this inner clause with EndOfInnerWhereClause.
        /// </summary>
        public void StartOfInnerWhereClause ()
            {
            m_sqlWhereClause += " ( ";
            }

        /// <summary>
        /// Ends an inner where clause by adding a closing parenthese in the where clause.
        /// </summary>
        public void EndOfInnerWhereClause ()
            {
            m_sqlWhereClause += " ) ";
            }

        /// <summary>
        /// Adds a condition to the where clause.
        /// </summary>
        /// <param name="tableName">Name of the table containing the column used in the condition</param>
        /// <param name="columnName">Name of the column used in the condition</param>
        /// <param name="op">Relational operator used in the condition</param>
        /// <param name="rightSideString">Right side of the condition encoded as a string</param>
        /// <param name="dbType">Database type of the right side.</param>
        public void AddWhereClause (string tableName, string columnName, RelationalOperator op, string rightSideString, DbType dbType)
            {
            if ( !String.IsNullOrWhiteSpace(tableName) )
                {
                m_sqlWhereClause += AddBrackets(tableName) + ".";
                }

            string sqlOp = ECToSQLMap.ECRelationalOperatorToSQL(op);

            if ( (op != RelationalOperator.ISNULL) && (op != RelationalOperator.ISNOTNULL) )
                {
                if ( op == RelationalOperator.IN )
                    {
                    List<String> paramNameList = new List<String>();
                    foreach ( string rightSideStringPart in rightSideString.Split(',') )
                        {
                        string paramName = GetNewParamName();
                        //m_sqlWhereClause += paramName + ",";
                        paramNameList.Add(paramName);
                        m_paramNameValueMap.AddParamNameValue(paramName, rightSideStringPart, dbType);
                        }
                    m_sqlWhereClause += AddBrackets(columnName) + " " + sqlOp + " (" + String.Join(",", paramNameList.ToArray()) + ") ";
                    }
                else
                    {
                    string paramName = GetNewParamName();
                    m_sqlWhereClause += AddBrackets(columnName) + " " + sqlOp + paramName + " ";
                    m_paramNameValueMap.AddParamNameValue(paramName, rightSideString, dbType);
                    }
                }
            else
                {
                m_sqlWhereClause += AddBrackets(columnName) + " " + sqlOp + " ";
                }

            }

        /// <summary>
        /// Adds an orderby clause to the query
        /// </summary>
        /// <param name="table">Table descriptor of the table containing the column</param>
        /// <param name="columnName">Name of the column on which the to order the results</param>
        /// <param name="sortAscending">Set to true to sort ascendingly, false otherwise.</param>
        public void AddOrderByClause (TableDescriptor table, string columnName, bool sortAscending)
            {
            string ascOrDesc = sortAscending ? "ASC" : "DESC";

            m_sqlOrderByClause.Add(AddBrackets(table.Alias) + "." + AddBrackets(columnName) + " " + ascOrDesc + " ");
            }

        /// <summary>
        /// Returns a boolean indicating if there is no orderby clause
        /// </summary>
        /// <returns></returns>
        public bool OrderByListIsEmpty ()
            {
            return m_sqlOrderByClause.Count == 0;
            }

        /// <summary>
        /// Does not do anything for now. Should be removed
        /// </summary>
        /// <param name="str"></param>
        /// <returns></returns>
        protected string AddBrackets (string str)
            {
            string tempString = str;
            //TODO : verify if adding brackets is useful!
            //if (!str.StartsWith("["))
            //{
            //    tempString = "[" + tempString;
            //}
            //if (!str.EndsWith("]"))
            //{
            //    tempString = tempString + "]";
            //}

            return tempString;
            }


        /// <summary>
        /// Builds the query according to the clauses added
        /// </summary>
        /// <returns>The SQL query string</returns>
        /// 
        //TODO : Include the query string, the DataReadingHelper and the paramNameValueMap in a single object returned by BuildQuery
        abstract public string BuildQuery (out DataReadingHelper dataReadingHelper);

        /// <summary>
        /// Similar to AddWhereClause, but specialised for obtaining the entries intersecting a polygon.
        /// </summary>
        /// <param name="tableName">Table descriptor of the table containing the column</param>
        /// <param name="columnName">Name of the column used in the condition. Must be a sys.geometry column</param>
        /// <param name="polygonWKT">The WKT polygon</param>
        /// <param name="polygonSRID">The SRID of the polygon</param>
        public void AddSpatialIntersectsWhereClause (string tableName, string columnName, string polygonWKT, int polygonSRID)
            {
            if ( !String.IsNullOrWhiteSpace(tableName) )
                {
                m_sqlWhereClause += AddBrackets(tableName) + ".";
                }

            m_sqlWhereClause += AddBrackets(columnName) + @".STIntersects(geometry::STGeomFromText('" + polygonWKT + @"'," + polygonSRID + @")) = 'true'";
            }

        private string GetNewParamName ()
            {
            return String.Format("@param{0}", m_paramNumber++);
            }

        /// <summary>
        /// The mapping of the parameter names and their values and type.
        /// </summary>
        public GenericParamNameValueMap paramNameValueMap
            {
            get
                {
                return m_paramNameValueMap;
                }
            }

        /// <summary>
        /// Builds a query returning the number of rows following the given criteria.
        /// </summary>
        /// <returns>The query string</returns>
        public string BuildCountQuery ()
            {
            string completeFromStr = "FROM " + m_sqlFromClause.Name + " " + m_sqlFromClause.Alias + " ";

            //string completeLeftJoinClause = String.Join(" ", m_sqlLeftJoinClause.ToArray());
            string completeLeftJoinClause = "";
            foreach ( var table in m_sqlLeftJoinClause )
                {
                completeLeftJoinClause += "LEFT JOIN " + AddBrackets(table.Name) + " " + table.Alias
                    + " ON " + AddBrackets(table.FirstTable.Alias) + "." + AddBrackets(table.FirstTableKey) + " = " + AddBrackets(table.Alias) + "." + AddBrackets(table.TableKey) + " ";
                }


            completeLeftJoinClause += " ";

            string completeWhereClause = "";
            if ( !String.IsNullOrWhiteSpace(m_sqlWhereClause) )
                {
                completeWhereClause = "WHERE " + m_sqlWhereClause + " ";
                }

            return "SELECT COUNT(*) " + completeFromStr + completeLeftJoinClause + completeWhereClause;
            }
        }

    /// <summary>
    /// Enum for the different categories of data that can be requested to the database.
    /// </summary>
    public enum ColumnCategory 
        {
        /// <summary>
        /// Used for data that is transfered in a stream.
        /// </summary>
        streamData = 0,
        /// <summary>
        /// Used for conventional data that will be returned in an IECInstance
        /// </summary>
        instanceData = 1,
        /// <summary>
        /// Used for spatial data that will be returned in an IECInstance 
        /// </summary>
        spatialInstanceData = 2,
        /// <summary>
        /// Used for requesting the related instance Id.
        /// </summary>
        relatedInstanceId = 3,
        /// <summary>
        /// Used for requesting data in a column that is not related to a property.
        /// </summary>
        nonPropertyData = 4
        }
    }



