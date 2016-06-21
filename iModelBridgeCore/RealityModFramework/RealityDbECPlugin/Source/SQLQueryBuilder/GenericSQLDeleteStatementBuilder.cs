/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/SQLQueryBuilder/GenericSQLDeleteStatementBuilder.cs $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Bentley.EC.Persistence.Query;
using Bentley.ECObjects.Schema;
using Bentley.Exceptions;
using IndexECPlugin.Source.Helpers;

namespace IndexECPlugin.Source
    {
    /// <summary>
    /// Class used to build an SQL statement to delete rows in a generic SQL database.
    /// </summary>
    public class GenericSQLDeleteStatementBuilder
        {

        int paramNumber;
        GenericParamNameValueMap m_paramNameValueMap;
        string m_tableName;
        List<string> m_sqlWhereClauseList;

        /// <summary>
        /// GenericSQLDeleteStatementBuilder constructor
        /// </summary>
        public GenericSQLDeleteStatementBuilder()
            {
            paramNumber = 0;
            m_tableName = null;
            m_sqlWhereClauseList = new List<string>();
            }

        /// <summary>
        /// Sets the table name (DELETE FROM tableName )
        /// </summary>
        /// <param name="tableName">The table name</param>
        public void SetDeleteFromTableName (string tableName)
            {
            if ( String.IsNullOrWhiteSpace(tableName) )
                {
                throw new ProgrammerException("tableName parameter must be non null and non empty");
                }
            m_tableName = tableName;
            }

        /// <summary>
        /// Adds a where clause. 
        /// </summary>
        /// <param name="columnName">The name of the column used in the condition</param>
        /// <param name="op">Relational operator used in the condition</param>
        /// <param name="rightSideString">Right side of the condition encoded as a string</param>
        /// <param name="ecType">The IECType of the data</param>
        void AddWhereClause (string columnName, RelationalOperator op, string rightSideString, IECType ecType)
            {
            string sqlOp = ECToSQLMap.ECRelationalOperatorToSQL(op);

            if ( (op != RelationalOperator.ISNULL) && (op != RelationalOperator.ISNOTNULL) )
                {
                if ( op == RelationalOperator.IN )
                    {
                    List<String> paramNameList = new List<String>();
                    foreach ( string rightSideStringPart in rightSideString.Split(',') )
                        {
                        string paramName = GetNewParamName();
                        paramNameList.Add(paramName);
                        m_paramNameValueMap.AddParamNameValue(paramName, rightSideStringPart, ecType);
                        }
                    m_sqlWhereClauseList.Add(columnName + " " + sqlOp + " (" + String.Join(",", paramNameList.ToArray()) + ") ");
                    }
                else
                    {
                    string paramName = GetNewParamName();
                    m_sqlWhereClauseList.Add(columnName + " " + sqlOp + paramName + " ");
                    m_paramNameValueMap.AddParamNameValue(paramName, rightSideString, ecType);
                    }
                }
            else
                {
                m_sqlWhereClauseList.Add(columnName + " " + sqlOp);
                }
            }

        /// <summary>
        /// Creates the SQL statement according to the information given.
        /// </summary>
        /// <param name="paramNameValueMap">The map indicating the value and type of each parameter in the parameterized statement returned</param>
        /// <returns>The SQL statement</returns>
        public string CreateSqlStatement (out IParamNameValueMap paramNameValueMap)
            {
            if ( String.IsNullOrWhiteSpace(m_tableName) )
                {
                throw new ProgrammerException("The delete statement cannot be created before the table name has been set.");
                }
            if ( m_sqlWhereClauseList.Count == 0 )
                {
                throw new ProgrammerException("The delete statement cannot be created if there are no conditions added.");
                }
            using ( StringWriter deleteStatement = new StringWriter() )
                {
                deleteStatement.Write("DELETE FROM ");
                deleteStatement.Write(m_tableName);
                deleteStatement.Write(" WHERE ");
                deleteStatement.Write(String.Join(" AND ", m_sqlWhereClauseList));

                paramNameValueMap = m_paramNameValueMap;

                return deleteStatement.ToString();
                }
            }
            

        private string GetNewParamName ()
            {
            paramNumber++;
            return "p" + Convert.ToString(paramNumber);
            }
        }
    }
