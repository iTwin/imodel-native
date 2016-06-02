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
    /// Class used to build an SQL statement to insert rows in an SQL server database.
    /// </summary>
    public class SQLServerInsertStatementBuilder : ISQLInsertStatementBuilder
        {
        int paramNumber;
        bool m_columnNamesSet;
        string m_tableName;
        SqlServerParamNameValueMap m_paramNameValueMap;
        List<string> m_rows;
        List<string> m_deleteWhereStatements;
        List<ColumnNameTypePair> m_columnNameTypePairs;
        bool m_deleteActivated;


        /// <summary>
        /// SQLServerInsertStatementBuilder class constructor
        /// </summary>
        public SQLServerInsertStatementBuilder()
            {

            m_columnNameTypePairs = new List<ColumnNameTypePair>();
            m_paramNameValueMap = new SqlServerParamNameValueMap();
            m_columnNamesSet = false;
            m_tableName = null;
            paramNumber = 0;
            m_rows = new List<string>();
            m_deleteWhereStatements = null;
            m_deleteActivated = false;
            }

        /// <summary>
        /// Sets the table name in which we want to insert rows
        /// </summary>
        /// <param name="tableName">The table name</param>
        public void SetInsertIntoTableName (string tableName)
            {
            if ( String.IsNullOrWhiteSpace(tableName) )
                {
                throw new ProgrammerException("tableName parameter must be non null and non empty");
                }

            m_tableName = tableName;
            }

        /// <summary>
        /// Calling this method will enable the user to include a delete statement before each insert
        /// </summary>
        public void ActivateDeleteBeforeInsert ()
            {
            if ( m_columnNamesSet )
                {
                throw new ProgrammerException("AddColumnName cannot be called after EndSettingColumnNames has been called.");
                }
            m_deleteActivated = true;
            }

        /// <summary>
        /// Adds a column to insert in each row. Cannot be called after EndSettingColumnNames
        /// </summary>
        /// <param name="columnName">The name of the column</param>
        /// <param name="ecType">The ECType of the data inserted</param>
        public void AddColumnName (string columnName, IECType ecType)
            {
            if ( m_columnNamesSet )
                {
                throw new ProgrammerException("AddColumnName cannot be called after EndSettingColumnNames has been called.");
                }
            m_columnNameTypePairs.Add(new ColumnNameTypePair() { ColumnName = columnName, SqlDbType = ECToSQLMap.ECTypeToSqlDbType(ecType) });
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
        /// Adds a column to insert in each row. Cannot be called after EndSettingColumnNames
        /// </summary>
        /// <param name="columnName">The name of the column</param>
        /// <param name="sqlDbType">The SqlDbType of the data inserted</param>
        public void AddColumnName (string columnName, SqlDbType sqlDbType)
            {
            if ( m_columnNamesSet )
                {
                throw new ProgrammerException("AddColumnName cannot be called after EndSettingColumnNames has been called.");
                }
            m_columnNameTypePairs.Add(new ColumnNameTypePair()
            {
                ColumnName = columnName, SqlDbType = sqlDbType
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

            if(m_deleteActivated)
                {
                m_deleteWhereStatements = new List<string>();
                }
            }

        /// <summary>
        /// Adds a row of values. Cannot be called before EndSettingColumnNames. Not all columns specified before are necessary, others will be null.
        /// </summary>
        /// <param name="rowOfValues">A dictionary containing the names of the column as keys and the value the row must contain as the value linked to the key.</param>
        /// <param name="deleteStatementManager">The delete statement to include before the insert. Must begin after the WHERE (DELETE FROM tableName WHERE ...). 
        /// Can be null if delete is not activated.</param>
        public void AddRow (Dictionary<string, object> rowOfValues, WhereStatementManager deleteStatementManager)
            {
            if ( !m_columnNamesSet )
                {
                throw new ProgrammerException("AddRow cannot be called before EndSettingColumnNames has been called.");
                }

            List<string> rowValuesArray = new List<string>();

            if(m_deleteActivated)
                {
                string deleteStatement = deleteStatementManager.WhereStatement;
                for ( int i = 0; i < deleteStatementManager.ParameterNames.Count; i++ )
                    {
                    string paramName = GetNewParamName();

                    string oldName = deleteStatementManager.ParameterNames[i];
                    IECType type = deleteStatementManager.ParameterTypes[i];
                    object value = deleteStatementManager.ParameterValues[i];

                    deleteStatement = deleteStatement.Replace(oldName, paramName);
                    m_paramNameValueMap.AddParamNameValue(paramName, value, type);
                    }
                    m_deleteWhereStatements.Add(deleteStatement);
                }

            foreach(ColumnNameTypePair columnpair in m_columnNameTypePairs)
                {
                string paramName = GetNewParamName();
                object value;
                if ( rowOfValues.TryGetValue(columnpair.ColumnName, out value) && value != null)
                    {
                    if ( columnpair.SqlDbType == SqlDbType.Udt )
                        {
                        //TODO : Find a better way to manage spatial types. The only way I found yet is to use SqlGeometry objects, but this would require
                        //the use of Microsoft.SqlServer.Types.dll. For now, we put this out of the parameters. There should not be any risks, since we create ourselves the wkt,
                        //and we convert the input into doubles and int, so no unwanted string should end up in the sql statement.
                        string STGeomFromTextString = DbGeometryHelpers.CreateSTGeomFromTextStringFromJson((string)value);
                        
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
            m_rows.Add( "(" + String.Join(",", rowValuesArray) + ")");
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
            if ( String.IsNullOrWhiteSpace(m_tableName) )
                {
                throw new ProgrammerException("The insert statement cannot be created before the table name has been set.");
                }
            if ( m_rows.Count == 0 )
                {
                throw new ProgrammerException("The insert statement cannot be created if there are no rows added.");
                }
            paramNameValueMap = m_paramNameValueMap;

            string deleteFrom = null;

            if(m_deleteActivated)
                {
                deleteFrom = "DELETE FROM " + m_tableName + " WHERE ";
                }
            string beginTry = " BEGIN TRY ";
            string endTry = " END TRY BEGIN CATCH IF ERROR_NUMBER() <> 2627 THROW END CATCH;";
            string insertIntoColumns = "INSERT INTO " + m_tableName + " (";
            insertIntoColumns += String.Join(",", m_columnNameTypePairs.Select(p => p.ColumnName)) + ") VALUES ";
            using ( StringWriter statement = new StringWriter() )
                {
                for (int i = 0; i < m_rows.Count; i++)
                    {
                    if(m_deleteActivated && m_deleteWhereStatements[i] != null)
                        {
                        statement.Write(deleteFrom + m_deleteWhereStatements[i]);
                        }
                    statement.Write(beginTry);
                    statement.Write(insertIntoColumns);
                    statement.Write(m_rows[i] + ";");
                    statement.Write(endTry);
                    }

                return statement.ToString();
                }
            }

        private string GetNewParamName ()
            {
            paramNumber++;
            return "@p" + Convert.ToString(paramNumber);
            }

        private class ColumnNameTypePair
            {
            public String ColumnName
                {
                get;
                set;
                }

            public SqlDbType SqlDbType
                {
                get;
                set;
                }
            }


        }
    }
