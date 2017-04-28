using System;
using System.Collections.Generic;
using System.Data;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Bentley.ECObjects.Schema;
using Bentley.Exceptions;
using IndexECPlugin.Source.Helpers;

namespace IndexECPlugin.Source
    {
    /// <summary>
    /// Class used to build an SQL statement to insert or update rows in an SQL server database.
    /// </summary>
    public class SQLServerMergeUpsertStatementBuilder : ISQLMergeUpsertStatementBuilder
        {
        int paramNumber;
        string m_targetTableName;
        string m_targetTableAlias;
        List<string> m_rows;
        string m_sourceTableAlias;
        bool m_columnNamesSet;
        List<ColumnNameTypePair> m_columnNameTypePairs;
        SqlServerParamNameValueMap m_paramNameValueMap;
        string m_onStatement;
        string m_matchedStatement;
        string m_notMatchedStatement;

        /// <summary>
        /// SQLServerMergeUpsertStatementBuilder class constructor
        /// </summary>
        public SQLServerMergeUpsertStatementBuilder ()
            {
            m_targetTableName = null;
            m_targetTableAlias = "t";
            m_sourceTableAlias = "s";
            m_columnNameTypePairs = new List<ColumnNameTypePair>();
            m_paramNameValueMap = new SqlServerParamNameValueMap();
            m_columnNamesSet = false;
            m_rows = new List<string>();

            }

        /// <summary>
        /// Gets the Target table alias. The alias should be used like this: MERGE tableName as alias
        /// </summary>
        public string TargetTableAlias
            {
            get
                {
                return m_targetTableAlias;
                }
            }

        /// <summary>
        /// Gets the Source table name. The name in this example is s: (USING (VALUES(...) ) as s(columnName1,columnName2,...)
        /// </summary>
        public string SourceTableAlias
            {
            get
                {
                return m_sourceTableAlias;
                }
            }

        /// <summary>
        /// Sets the table name (MERGE tableName)
        /// </summary>
        /// <param name="tableName">The table name</param>
        public void SetMergeTableName (string tableName)
            {
            if ( String.IsNullOrWhiteSpace(tableName) )
                {
                throw new ProgrammerException("tableName parameter must be non null and non empty");
                }

            m_targetTableName = tableName;
            }

        /// <summary>
        /// Adds a column (USING (VALUES(...) ) as s(columnName1,columnName2,...). 
        /// Cannot be called after EndSettingColumnNames
        /// </summary>
        /// <param name="columnName">The name of the column in the database</param>
        /// <param name="ecType">The Type of the column</param>
        public void AddColumnName (string columnName, IECType ecType)
            {
            if ( m_columnNamesSet )
                {
                throw new ProgrammerException("AddColumnName cannot be called after EndSettingColumnNames has been called.");
                }
            m_columnNameTypePairs.Add(new ColumnNameTypePair()
            {
                ColumnName = columnName, SqlDbType = ECToSQLMap.ECTypeToSqlDbType(ecType)
            });
            }

        /// <summary>
        /// Adds a spatial column to insert in each row. Cannot be called after EndSettingColumnNames
        /// </summary>
        /// <param name="columnName">The name of the column</param>
        public void AddSpatialColumnName (string columnName)
            {
            if ( m_columnNamesSet )
                {
                throw new ProgrammerException("AddColumnName cannot be called after EndSettingColumnNames has been called.");
                }
            m_columnNameTypePairs.Add(new ColumnNameTypePair()
            {
                ColumnName = columnName, SqlDbType = SqlDbType.Udt
            });
            }

        /// <summary>
        /// Adds a binary column of data to insert in each row. Cannot be called after EndSettingColumnNames
        /// </summary>
        /// <param name="columnName">The name of the column</param>
        public void AddBinaryColumnName (string columnName)
            {
            if ( m_columnNamesSet )
                {
                throw new ProgrammerException("AddColumnName cannot be called after EndSettingColumnNames has been called.");
                }
            m_columnNameTypePairs.Add(new ColumnNameTypePair()
            {
                ColumnName = columnName, SqlDbType = SqlDbType.VarBinary
            });
            }

        /// <summary>
        /// Ends the phase of adding columns to the insert. Add*ColumnName cannot be called after this, 
        /// and AddRow cannot be called before this.
        /// </summary>
        public void EndSettingColumnNames ()
            {
            m_columnNamesSet = true;
            }

        /// <summary>
        /// Set the ON statement (without the ON). Example : (ON) t.IdStr=s.IdStr and t.SubAPI=s.SubAPI
        /// </summary>
        /// <param name="onStatement">The ON statement, without the ON</param>
        public void SetOnStatement (string onStatement)
            {
            m_onStatement = onStatement;
            }

        /// <summary>
        /// Sets the MATCHED conditional statement (without the "WHEN MATCHED AND" and "THEN"). Example: WHEN MATCHED AND (t.Complete='FALSE') THEN
        /// </summary>
        /// <param name="matchedStatement">The MATCHED conditional statement (without the "WHEN MATCHED AND" and "THEN")</param>
        public void SetMatchedStatement (string matchedStatement)
            {
            m_matchedStatement = matchedStatement;
            }

        /// <summary>
        /// Sets the NOT MATCHED conditional statement (without the "WHEN NOT MATCHED AND" and "THEN"). Example: WHEN NOT MATCHED AND (t.Complete='FALSE') THEN
        /// </summary>
        /// /// <param name="notMatchedStatement">The NOT MATCHED conditional statement (without the "WHEN NOT MATCHED AND" and "THEN")</param>
        public void SetNotMatchedStatement (string notMatchedStatement)
            {
            m_notMatchedStatement = notMatchedStatement;
            }

        /// <summary>
        /// Adds a row of values. Cannot be called before EndSettingColumnNames. Not all columns specified before are necessary, others will be null.
        /// </summary>
        /// <param name="rowOfValues">A dictionary containing the names of the column as keys and the value the row must contain as the value linked to the key.</param>
        public void AddRow (Dictionary<string, object> rowOfValues)
            {
            if ( !m_columnNamesSet )
                {
                throw new ProgrammerException("AddRow cannot be called before EndSettingColumnNames has been called.");
                }

            List<string> rowValuesArray = new List<string>();

            foreach ( ColumnNameTypePair columnpair in m_columnNameTypePairs )
                {
                string paramName = GetNewParamName();
                object value;
                if ( rowOfValues.TryGetValue(columnpair.ColumnName, out value) && value != null )
                    {
                    if ( columnpair.SqlDbType == SqlDbType.Udt )
                        {
                        //TODO : Find a better way to manage spatial types. The only way I found yet is to use SqlGeometry objects, but this would require
                        //the use of Microsoft.SqlServer.Types.dll. For now, we put this out of the parameters. There should not be any risks, since we create ourselves the wkt,
                        //and we convert the input into doubles and int, so no unwanted string should end up in the sql statement.
                        string STGeomFromTextString = DbGeometryHelpers.CreateSTGeomFromTextStringFromJson((string) value);

                        //m_paramNameValueMap.AddParamNameValue(paramName, STGeomFromTextString, columnpair.SqlDbType);

                        //The following line should be removed if the one before is uncommented.
                        paramName = STGeomFromTextString;

                        }
                    else
                        {
                        m_paramNameValueMap.AddParamNameValue(paramName, value, columnpair.SqlDbType);
                        }
                    }
                else
                    {
                    m_paramNameValueMap.AddParamNameValue(paramName, null, columnpair.SqlDbType);
                    }
                rowValuesArray.Add(paramName);
                }
            m_rows.Add("(" + String.Join(",", rowValuesArray) + ")");
            }

        /// <summary>
        /// Creates the Sql Server statement according to the columns and rows added. Must be called after EndSettingColumnNames and after having added at least one row.
        /// </summary>
        /// <param name="paramNameValueMap">The map indicating the value and type of each parameter in the parameterized statement returned</param>
        /// <returns>The Sql Server statement</returns>
        public string CreateSqlStatement (out IParamNameValueMap paramNameValueMap)
            {
            if ( !m_columnNamesSet )
                {
                throw new ProgrammerException("CreateSqlStatement cannot be called before EndSettingColumnNames has been called.");
                }
            if ( String.IsNullOrWhiteSpace(m_targetTableName) )
                {
                throw new ProgrammerException("The merge statement cannot be created before the table name has been set.");
                }
            if ( m_rows.Count == 0 )
                {
                throw new ProgrammerException("The merge statement cannot be created if there are no rows added.");
                }
            if ( String.IsNullOrWhiteSpace(m_onStatement) )
                {
                throw new ProgrammerException("The merge statement cannot be created if there is no On statement set.");
                }

            paramNameValueMap = m_paramNameValueMap;

            using ( StringWriter statement = new StringWriter() )
                {
                statement.Write("MERGE " + m_targetTableName + " AS " + m_targetTableAlias + " ");
                statement.Write("USING (VALUES ");
                for ( int i = 0; i < m_rows.Count; i++ )
                    {
                    statement.Write(m_rows[i]);
                    if(i != m_rows.Count - 1)
                        {
                        statement.Write(",");
                        }
                    }
                statement.Write(") AS " + m_sourceTableAlias + " (" + String.Join(",", m_columnNameTypePairs.Select(p => p.ColumnName)) + ") ");
                statement.Write("ON " + m_onStatement);
                if(!String.IsNullOrWhiteSpace(m_matchedStatement))
                    {
                    statement.Write(" WHEN MATCHED AND (" + m_matchedStatement + ") THEN ");
                    }
                else
                    {
                    statement.Write(" WHEN MATCHED THEN ");
                    }
                statement.Write("UPDATE SET " + String.Join(",", m_columnNameTypePairs.Select(p => p.ColumnName + "=" + m_sourceTableAlias + "." + p.ColumnName)));
                if ( !String.IsNullOrWhiteSpace(m_notMatchedStatement) )
                    {
                    statement.Write(" WHEN NOT MATCHED AND (" + m_notMatchedStatement + ") THEN ");
                    }
                else
                    {
                    statement.Write(" WHEN NOT MATCHED THEN ");
                    }

                statement.Write("INSERT (" + String.Join(",", m_columnNameTypePairs.Select(p => p.ColumnName)) + ") ");
                statement.Write("VALUES (" + String.Join(",", m_columnNameTypePairs.Select(p => m_sourceTableAlias + "." + p.ColumnName)) + ");");

                return statement.ToString();
                }
            }
        private string GetNewParamName ()
            {
            paramNumber++;
            return "@p" + Convert.ToString(paramNumber);
            }
        }
    }
