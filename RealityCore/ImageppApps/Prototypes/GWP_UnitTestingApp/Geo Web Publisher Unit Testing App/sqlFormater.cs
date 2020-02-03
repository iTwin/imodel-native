/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/GWP_UnitTestingApp/Geo Web Publisher Unit Testing App/sqlFormater.cs $
|    $RCSfile: sqlFormater.cs, $
|   $Revision: 1 $
|       $Date: 2013/05/27 $
|     $Author: Julien Rossignol $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

namespace Geo_Web_Publisher_Unit_Testing_App
{

    class SqlFormater
    {
        #region methods

        #region methods/query

        /*------------------------------------------------------------------------------------**/
        /// <summary>Format a query to load a particuliar element in a table by is id</summary> 
        /// <param name="table">Table to load from</param>
        /// <param name="id">Id to look in the table</param>
        /// <returns>Sql command/query string</returns>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public string QueryById(DBTable table , int id)
        {
            string query = "Select * from " + GetTableName(table) + " Where " + GetTableName(table , false) + ".Id = " + id + ";";
            return query;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Format a query to load all record corresponding to a precise category</summary> 
        /// <param name="id">Id of the category to load</param>
        /// <returns>Sql command/query string</returns>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public string RecordByCategory(int id)
        {
            string query = "Select Record.Id , Record.Name, Record.Description, Record.Request, Record.CategoryId, Category.Name, Record.MRBaselineId, Baseline.Raster from Record Inner Join Category On Record.CategoryId = Category.Id Left Join Baseline On Record.MRBaselineId = Baseline.Id Where Record.CategoryId = " + id + ";";
            return query;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Format a query to load all record corresponding to a precise category</summary> 
        /// <param name="category">Name of the category to load</param>
        /// <returns>Sql command/query string</returns>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public string RecordByCategory(string category)
        {
            string query = "Select Record.Id , Record.Name, Record.Description, Record.Request, Record.CategoryId, Category.Name Record.MRBaselineId, Baseline.Raster from Record Inner Join Category On Record.CategoryId = Category.Id Left Join Baseline On Record.MRBaselineId = Baseline.Id where Category.Name = " + "'" + EscapeCaracter(category) + "';";
            return query;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Format a query to load all element in a table</summary> 
        /// <param name="table">Table name</param>
        /// <returns>Sql command/query string</returns>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public string QueryAll(DBTable table)
        {
            if( table == DBTable.RECORD )
            {
                return "Select Record.Id , Record.Name, Record.Description, Record.Request, Record.CategoryId, Category.Name, Record.MRBaselineId, Baseline.Raster from " + GetTableName(table) + ";";
            }
            else
            {
                return "Select * from " + GetTableName(table) + ";";
            }

        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Format a query to load all ResultRequest before/after a certain date</summary>
        /// <param name="date">Timestamp of the date to look</param>
        /// <param name="IsOlder">Determines if it must look for older or more recent result</param>
        /// <returns>Sql command/query string</returns>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public string ResultByDate(int date , bool IsOlder = false)
        {
            string comparator = "<= ";
            if( IsOlder )
                comparator = ">= ";

            string query =  "Select * from RequestResult Where DateTime " + comparator + date + ";";
            return query;
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Format a query to load a particuliar category by is name</summary>
        /// <param name="name">Name of the category</param>
        /// <returns>Sql command/query string</returns>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public string CategoryByName(string name)
        {
            string query = "Select * from Category where Name = '" + EscapeCaracter(name) + "';";
            return query;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Format a query to load all result corresponding to a record</summary> 
        /// <returns>Sql command/query string</returns>
        /// <param name="index">Index of the record</param>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public string QueryResultByRecord(int index)
        {
            string query = "Select * from RequestResult Left Join FailureRaster On FailureRaster.Id = RequestResult.FailureRasterId Where RecordId = " + index+";";
            return query;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Format a query to load all failed result corresponding to a record</summary> 
        /// <returns>Sql command/query string</returns>
        /// <param name="index">Index of the record</param>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public string QueryFailedResultByRecord(int index)
        {
            string query = "Select * from RequestResult Where RecordId = " + index + " AND RequestResult.Status > 0;";
            return query;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Format a query to load all prefix in the database</summary> 
        /// <returns>Sql command/query string</returns>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public string QueryAllPrefix()
        {
            string query = "Select ServerConnection from Test";
            return query;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Format a query to load all result linked to a test</summary>
        /// <param name="index">Index of the test</param>
        /// <returns>Sql command/query string</returns>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public string QueryResultByTest(int index)
        {
            string query = "Select * from RequestResult Where TestId = " + index + ";";
            return query;
        }

        #endregion

        #region methods/delete

        /*------------------------------------------------------------------------------------**/
        /// <summary>Format a delete an element in a table by is id</summary> 
        /// <param name="table">Table in which to delete the element</param>
        /// <param name="id">If of the element to delete</param>
        /// <returns>Sql command/query string</returns>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public string deleteById(DBTable table , int id)
        {
            string command = "Delete From " + GetTableName(table , false) + " where id = " + id +";";
            return command;
        }


        /*------------------------------------------------------------------------------------**/
        /// <summary>Format a query to delete everything in a table linked to a specified record</summary> 
        /// <returns>Sql command/query string</returns>
        /// <param name="dBTable">Table in which to delete record</param>
        /// <param name="recordId">Id of the record</param>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public string deleteByRecord(DBTable dBTable , int recordId)
        {
            string command = "delete from " + GetTableName(dBTable) + " where RecordId = " + recordId + ";";
            return command;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Format a command to delete all result linked to a test</summary>
        /// <param name="index">Index of the test</param>
        /// <returns>Sql command/query string</returns>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public string DeleteResultByTest(int index)
        {
            string command = "Delete from RequestResult Where TestId = " + index + ";";
            return command;
        }

        #endregion

        #region methods/insert


        /*------------------------------------------------------------------------------------**/
        /// <summary>Format a command to insert a record in the database</summary>
        /// <param name="record">Record to insert into the database</param>
        /// <returns>Sql command/query string</returns>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public string InsertRecord(Record record)
        {
            string command = "Insert into Record(Name, Description, Request, CategoryId, MrBaselineId) VALUES('" + EscapeCaracter(record.Name) + "','" + EscapeCaracter(record.Description) + "','" + EscapeCaracter(record.RecordRequest.RequestString) + "'," + record.CategoryId + "," + record.MRBaselineId + ");";
            return command;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Format a command to insert a category</summary>
        /// <param name="category">Category to insert</param>
        /// <returns>Sql command/query string</returns>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public string InsertCategory(Category category)
        {
            string command = "Insert into Category(Name) VALUES('" + EscapeCaracter(category.Name) + "');";
            return command;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Format a command to insert a result into the database</summary> 
        /// <returns>Sql command/query string</returns>
        /// <param name="category">Result to save</param>
        /// <param name="rasterID">Id of the failure raster</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public string InsertResult(ResultRequest result , int rasterID)
        {
            string command = "Insert into RequestResult(RecordId, Status, TestId, FailureRasterId, InvalidPixel, BigDifferencePixel) VALUES("+ result.ResultRecord.Id + "," + (int) result.Status + "," + result.TestId + "," + rasterID + "," + result.InvalidPixels + "," + result.BigDifferencePixels + ");";
            return command;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Format a command to insert a test</summary> 
        /// <returns>Sql command/query string</returns>
        /// <param name="category">Test to insert</param>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public string InsertTest(Test test)
        {
            string command = "Insert into Test(DateTime, Description, ServerConnection) VALUES(" + test.DateTime + ",'" + EscapeCaracter(test.Description) + "','" + test.Prefix + "');";
            return command;
        }

        #endregion

        #region methods/update

        /*------------------------------------------------------------------------------------**/
        /// <summary>Format a command to update a record in the database</summary> 
        /// <param name="record">Record to insert into the database</param>
        /// <returns>Sql command/query string</returns>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public string UpdateRecord(Record record)
        {
            string command = "Update Record Set Name = '" + EscapeCaracter(record.Name) + "', Description = '" + EscapeCaracter(record.Description) + "', Request = '" + EscapeCaracter(record.RecordRequest.RequestString) + "', CategoryId = " + record.CategoryId + ", MRBaselineID = " + record.MRBaselineId + " Where Id = " + record.Id + ";";
            return command;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Format a command to update a category</summary> 
        /// <returns>Sql command/query string</returns>
        /// <param name="category">Category to update</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public string UpdateCategory(Category category)
        {
            string command = "Update Category set Name = '" + EscapeCaracter(category.Name) + "' where id =" + category.Id + ";";
            return command;
        }

        #endregion

        #region methods/others

        /*------------------------------------------------------------------------------------**/
        /// <summary>Return the name of the table base on the DBTable input</summary>
        /// <param name="table">Table to get the name</param>
        /// <param name="withModifier">Indicates wheter or not modifiers like Inner Join must be append to the table name</param>
        /// <returns>Sql command/query string</returns>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public string GetTableName(DBTable table , bool withModifier = true)
        {
            string TableName;

            switch( table )
            {
                case DBTable.CATEGORY:
                    TableName = "Category";
                    break;
                case DBTable.RECORD:
                    if( withModifier )
                        TableName = "Record Left Join Category On Record.CategoryId = Category.Id Left Join Baseline On Record.MRBaselineId = Baseline.Id";
                    else
                        TableName = "Record";
                    break;
                case DBTable.REQUESTRESULT:
                    TableName = "RequestResult";
                    break;
                case DBTable.BASELINE:
                    TableName = "Baseline";
                    break;
                case DBTable.FAILURERASTER:
                    TableName = "FailureRaster";
                    break;
                case DBTable.TEST:
                    TableName = "Test";
                    break;
                default:
                    TableName = "";
                    break;
            }
            return TableName;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Escape apostrophes of the input string</summary> 
        /// <returns>string with all apostrophes escaped</returns>
        /// <param name="stringToEscape">String from which apostrophee needs to be escape</param>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public string EscapeCaracter(string stringToEscape)
        {
            string withReplacement = stringToEscape.Replace("'" , "''");
            return withReplacement;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Format a query to count the number of failed result for a test</summary>
        /// <param name="testIndex">Index of the test</param>
        /// <returns>Sql command/query string</returns>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public string GetNumberOfFailedRecordTest(int testIndex)
        {
            string query = "Select Count(*) AS NumberOfFailedRecord From RequestResult Where TestId =" + testIndex + " AND Status > 0 ;";
            return query;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Format a query to count the number of result for a test</summary>
        /// <param name="testIndex">Index of the test</param>
        /// <returns>Sql command/query string</returns>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public string GetNumberOfRecordTest(int testIndex)
        {
            string query = "Select Count(*) AS NumberOfRecord From RequestResult Where TestId =" + testIndex + ";";
            return query;
        }

        #endregion

        #endregion
    }
}
