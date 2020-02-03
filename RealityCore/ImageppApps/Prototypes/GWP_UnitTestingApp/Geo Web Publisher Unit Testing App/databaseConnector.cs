/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/GWP_UnitTestingApp/Geo Web Publisher Unit Testing App/databaseConnector.cs $
|    $RCSfile: databaseConnector.cs, $
|   $Revision: 1 $
|       $Date: 2013/05/27 $
|     $Author: Julien Rossignol $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System;
using System.Data;
using System.Data.SQLite;

namespace Geo_Web_Publisher_Unit_Testing_App
{
    /// <summary>Manage the connexion and the execution of query/command to the database</summary>
    /// <author>Julien Rossignol</author>
    class DatabaseConnector
    {
        #region fields

        public string m_ConnectionString; // string used to connect to the database
        
        #endregion

        #region methods

        /*------------------------------------------------------------------------------------**/
        /// <summary>update the connection string</summary> 
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void ConnectionToDatabase()
        {
            m_ConnectionString = "Data Source=" + Properties.GeoWebPublisherUnitTestingApp.Default.DatabaseName; //set the conenction string to the one specified in settings file
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Execute the sql request</summary>
        /// <param name="sql">Sql code to execute</param>
        /// <returns>Return the result, if the command is a query</returns>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public DataTable Execute(string sql)
        {
            DataTable queryResponse = new DataTable();
            if( sql.StartsWith("Select" , StringComparison.CurrentCultureIgnoreCase) ) // if the request start with select
                queryResponse = ExecuteQuery(sql);
            else //if the request is update / insert / delete
                ExecuteNonQuery(sql);

            return queryResponse;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Execute the sql query</summary>
        /// <param name="sqlQuery">Query to be excute</param>
        /// <returns>Datatable containing the result fo the request</returns>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private DataTable ExecuteQuery(string sqlQuery)
        {
            DataTable queryResponse = new DataTable();
            try//try to connect to the database and execute the query
            {
                SQLiteConnection connection = new SQLiteConnection(m_ConnectionString);
                connection.Open();
                SQLiteCommand query = new SQLiteCommand(connection);
                query.CommandText = sqlQuery;
                SQLiteDataReader reader = query.ExecuteReader();//read the query response
                queryResponse.Load(reader); //load query response into data table
                reader.Close();
                connection.Close();
            }
            catch
            {
            }
            return queryResponse;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Execute the sql command (insert/update/delete)</summary>
        /// <param name="sqlCommand">Sql command to be execute</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void ExecuteNonQuery(string sqlCommand)
        {
            try // try to connect to the database and execute command
            {
                SQLiteConnection connection = new SQLiteConnection(m_ConnectionString);
                connection.Open();
                SQLiteCommand command = new SQLiteCommand(connection);
                command.CommandText = sqlCommand;
                command.ExecuteNonQuery();
                connection.Close();
            }
            catch // TO DO
            {

            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Insert a baseline into a database. We need a special function since converting it to string would be difficult</summary>
        /// <param name="baseline">Byte array containing the baseline to save</param>
        /// <param name="recordId">Id of the record associate with the baseline</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void InsertBaselineCommand(byte[] baseline , int recordId)
        {
            SQLiteConnection connection = new SQLiteConnection(m_ConnectionString);
            connection.Open();
            SQLiteCommand command = new SQLiteCommand("Insert into [Baseline] (RecordId, Raster) Values(@RecordId, @Baseline)" , connection); // prepare the SQL command
            command.Parameters.Add("RecordId" , DbType.Int32 , 0).Value = recordId;
            command.Parameters.Add("Baseline" , DbType.Object , 0).Value = baseline;
            command.ExecuteNonQuery(); // execute the command
            connection.Close();
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Insert a output image into a database. We need a special function since converting it to string would be difficult</summary>
        /// <param name="result">Result containing the image to save and other needed information</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void InsertOutputImage(ResultRequest result)
        {
            SQLiteConnection connection = new SQLiteConnection(m_ConnectionString);
            connection.Open();
            SQLiteCommand command = new SQLiteCommand("Insert into [FailureRaster] (Raster) Values(@Raster)" , connection); //prepare the command
            command.Parameters.Add("Raster" , DbType.Object , 0).Value = result.ResultBitmap;
            command.ExecuteNonQuery(); //execute the command
            connection.Close();
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Check if the current database exists</summary>
        /// <param name="result">returns true if the file exist</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public bool CheckIfDatabaseExist()
        {
            return System.IO.File.Exists(Properties.GeoWebPublisherUnitTestingApp.Default.DatabaseName);
        }

        #endregion
    }
}
