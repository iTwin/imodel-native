#region using
using System;
using System.IO;
using System.Collections;
using SD = System.Data;
using SDO = System.Data.OleDb;
using NUnit.Framework;

#endregion using

namespace Bentley.TerrainModelNET.NUnit
{
    using Bentley.TerrainModelNET;


    struct dbStrings
    {
        public String _dbPath;
        public String _dbName;
        public String _dbTable;
        public string _theTest;
    }
    

    /// <summary>
    /// Database Access
    /// </summary>
        class DatabaseAccess
        {
            private System.IFormatProvider _formatProvider = System.Globalization.NumberFormatInfo.CurrentInfo;
            private SDO.OleDbConnection _dbConnection;
            private dbStrings accessDb = new dbStrings();
            private SDO.OleDbDataAdapter adapter;
            private SD.DataTable schemaTable;
            private String SQL;
            private String SQLWhere;

            


        /// <summary>
        /// Constructor
        /// </summary>
            public DatabaseAccess(string dbPath, string dbName, string dbTable, string theTest)
            {
                // Package the database access strings for easier usage
                accessDb._dbPath = dbPath;
                accessDb._dbName = dbName;
                accessDb._dbTable = dbTable;
                accessDb._theTest = theTest;

                // Create a basic SQL Select command string. The "WHERE" clause will be added later
                SQL = "SELECT * FROM " + dbTable;
            }

            /// <summary>
            /// ReadDatabaseSchema
            /// </summary>
            private void ReadDatabaseSchema()
            {
                // Create the variables
                SDO.OleDbDataReader schemaReader;
                SDO.OleDbCommand cmd = new SDO.OleDbCommand();

                // Create a connection and read the database schema
                cmd.Connection = _dbConnection;
                cmd.CommandText = SQL;
                schemaReader = cmd.ExecuteReader(SD.CommandBehavior.KeyInfo);
                schemaTable = schemaReader.GetSchemaTable();

                //Update the SQL command by adding a "WHERE" clause and reading the name of the first database column.
                SD.DataRow dbColumnNames = schemaTable.Rows[0];
                SQLWhere= " WHERE " + dbColumnNames[0].ToString() + " = '" + accessDb._theTest + "'";
                SQL = SQL + SQLWhere;
            }

            /// <summary>
            /// Database open
            /// </summary>
            public void OpenTheDatabase()
            {
                _dbConnection = new SDO.OleDbConnection(getConnectionString(accessDb));
                if (_dbConnection.State == SD.ConnectionState.Closed)
                    _dbConnection.Open();
                ReadDatabaseSchema();
            }


            /// <summary>
            /// ReadFromTheDatabase
            /// </summary>
            /// <returns></returns>
            public String ReadFromTheDatabase(String WhichData)
            {
                SDO.OleDbCommand GetCmd = _dbConnection.CreateCommand();
                GetCmd.CommandText = SQL;
                SDO.OleDbDataReader ThisReader = GetCmd.ExecuteReader();
                adapter = new SDO.OleDbDataAdapter(SQL, _dbConnection);
                ThisReader.Read();
                String Info = ThisReader[WhichData].ToString();
                ThisReader.Dispose();
                return Info;
            }

            /// <summary>
            /// Create the Database connection string
            /// </summary>
            /// <param name="dbs"></param>
            /// <returns></returns>
            private String getConnectionString(dbStrings dbs )
            {
                // Make the Connection string
                return "Provider=Microsoft.Jet.OLEDB.4.0; Data Source=" + dbs._dbPath + dbs._dbName;
            }

            /// <summary>
            /// UpdateTheDatabase
            /// </summary>
            /// <param name="NewData"></param>
            public void UpdateTheDatabase(String[] NewData)
            {
                // Create the variables
                String SQLUpdate = "";
                String SQLSet = " SET ";

                //Create a DataSet and fill it from the Database
                SD.DataSet ds = new SD.DataSet();
                adapter.Fill(ds, accessDb._dbTable);

                // Fill a new Table from the data adapter
                SD.DataTable NewTestDataTable = ds.Tables[accessDb._dbTable];

                // Load  the dataset with the new data
                for (int i=1; i < NewTestDataTable.Columns.Count; i++)
                    ds.Tables[accessDb._dbTable].Rows[0][i]=NewData[i];

                // Loop starts at 1 to miss the "WhichTest" entry. Remove the comma from the last entry
                for (int i = 1; i < NewTestDataTable.Columns.Count-1; i++)
                {
                    SQLSet = SQLSet + NewTestDataTable.Columns[i] + " = ?, ";
                }
                SQLSet = SQLSet + NewTestDataTable.Columns[NewTestDataTable.Columns.Count - 1] + " = ?";

                // Put the Update command together
                SQLUpdate = "UPDATE " + accessDb._dbTable + SQLSet + SQLWhere;
                adapter.UpdateCommand = new SDO.OleDbCommand(SQLUpdate, _dbConnection);

                // Loop through to create the Update parameters, again ignore the first column
                SD.DataColumn ColumnInfo = schemaTable.Columns["ColumnSize"];
                bool notFirstColumn = false;
                foreach (SD.DataRow RowInfo in schemaTable.Rows)
                {
                    if (notFirstColumn)
                    {
                        adapter.UpdateCommand.Parameters.Add("@" + RowInfo[0], SDO.OleDbType.Char, Convert.ToInt16(RowInfo[ColumnInfo]), RowInfo[0].ToString());
                    }
                    notFirstColumn = true;
                }

                // Update the database
                adapter.Update(ds, accessDb._dbTable);
            }

            /// <summary>
            /// CloseTheDatabase
            /// </summary>
            public void CloseTheDatabase()
            {
                _dbConnection.Close();
            }

        }
    }
