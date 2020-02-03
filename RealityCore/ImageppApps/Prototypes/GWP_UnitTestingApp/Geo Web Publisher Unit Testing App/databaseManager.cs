/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/GWP_UnitTestingApp/Geo Web Publisher Unit Testing App/databaseManager.cs $
|    $RCSfile: DatabaseManager.cs, $
|   $Revision: 1 $
|       $Date: 2013/05/27 $
|     $Author: Julien Rossignol $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System;
using System.Collections.Generic;
using System.Data;
using System.Diagnostics;

namespace Geo_Web_Publisher_Unit_Testing_App
{
    /// <summary>Manage all query/command to the database, modify Program lists and call for updates on form when needed</summary>
    /// <author>Julien Rossignol</author>
    class DatabaseManager
    {

        #region fields

        private DatabaseConnector m_dbConnect; // Class used for connection to database and executing request
        private SqlFormater m_SqlFormat; // Class used to format sql command

        #endregion

        #region constructors

        public DatabaseManager() // base constructor
        {
            m_dbConnect = new DatabaseConnector();
            m_SqlFormat = new SqlFormater();
        }

        #endregion

        #region methods

        #region methods/category

        /*------------------------------------------------------------------------------------**/
        /// <summary>Load all categories into Program.CategoryList, update Main form to fit the new categories</summary> 
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void LoadCategories()
        {
            Program.CategoryList.Clear();
            DataTable categoryTable = m_dbConnect.Execute(m_SqlFormat.QueryAll(DBTable.CATEGORY)); //Fetch all category from the database
            foreach( DataRow row in categoryTable.Rows ) //add each category in Program.CategoryList
            {
                Category toAdd = new Category();
                toAdd.Id = Convert.ToInt32(row["Id"].ToString());
                toAdd.Name = row["Name"].ToString();
                Program.CategoryList.Add(toAdd);
            }
            if( Program.MainForm != null ) //if MainForm exist, reload categories 
            {
                Program.MainForm.LoadCategories();
                Program.MainForm.LoadRecordInGridView();
            }
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Update the category list a the specify index</summary>
        /// <param name="index">Index in Program.CategoryList where to do the update</param>
        /// <param name="name">New name of the category</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void UpdateCategoryList(int index , string name)
        {
            int id = Program.CategoryList[index].Id;
            Category category = new Category();
            category.Id = id;
            category.Name = name;
            m_dbConnect.Execute(m_SqlFormat.UpdateCategory(category)); // update category with new name
            LoadCategories(); //reload categories to apply changes
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Delete the specified category</summary> 
        /// <param name="index">Index of the category to delete</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void DeleteCategory(int index)
        {
            int id = Program.CategoryList[index].Id;
            m_dbConnect.Execute(m_SqlFormat.deleteById(DBTable.CATEGORY , id));//delete specified category
            LoadCategories(); //reload categories to apply changes
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Insert a new category</summary>
        /// <param name="name">Name of the new category</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void InsertCategory(string name)
        {
            Category category = new Category();
            category.Name = name;
            m_dbConnect.Execute(m_SqlFormat.InsertCategory(category));  //insert category 
            LoadCategories(); // reload categories to apply changes
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Returns the id associate with the category name</summary>
        /// <param name="name">Name of the category to look for</param>
        /// <returns>Category Id</returns>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public int LookForCategoryId(string name)
        {
            foreach( Category category in Program.CategoryList )
            {
                if( category.Name == name )
                    return category.Id;
            }
            return 0; // returns 0 if nothing is found
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Returns the name of the category associatie with the specified id</summary>
        /// <param name="id">Id to look for</param>
        /// <returns>Category name</returns>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public string LookForCategoryName(int id)
        {
            foreach( Category category in Program.CategoryList )
            {
                if( category.Id == id )
                    return category.Name;
            }
            return ""; // return an empty string if nothing is found
        }

        #endregion

        #region methods/record

        /*------------------------------------------------------------------------------------**/
        /// <summary>Load the specified DataTable in Program.ResultList</summary> 
        /// <param name="recordTable">DataTable containing records to loadd</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void LoadRecord(DataTable recordTable)
        {
            Program.ResultList.Clear();
            foreach( DataRow row in recordTable.Rows )
            {
                AddRecord(row);
            }
            if( Program.MainForm != null )
                Program.MainForm.LoadRecordInGridView();
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Load all record in Program.ResultList</summary> 
        /// <param name="recordTable">DataTable containing records to loadd</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void LoadAllRecord()
        {
            DataTable recordTable = m_dbConnect.Execute(m_SqlFormat.QueryAll(DBTable.RECORD)); // Fetch all record in the database
            LoadRecord(recordTable);
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Add a record to the the record list</summary>
        /// <param name="row">Row to add in the record list</param>
        /// <param name="insertPosition">Position to insert the new record</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void AddRecord(DataRow row , int insertPosition = -1 , bool executetest = false)
        {
            //initialisation of the new record
            Record toAdd = new Record();
            toAdd.Id = Convert.ToInt32(row["Id"].ToString());
            toAdd.MRBaselineId = Convert.ToInt32(row["MRBaselineId"].ToString());
            if( row["Raster"].ToString() != "" ) // if baseline raster exist
            {
                byte[] bitmapBytes = (byte[]) row["Raster"];
                toAdd.BaselineBitmap = bitmapBytes;
                toAdd.BaselineIsUpToDate = true;

                if( RasterManipulation.ConvertByteArrayToBitmap(toAdd.BaselineBitmap) == null ) //if it starts with <, it's an xml file
                    toAdd.BaselineIsXML = true;
            }
            else
                toAdd.BaselineIsUpToDate = false; //if baseline doesn't exist, the MainForm will ask for an update

            toAdd.Description = row["Description"].ToString();
            toAdd.Name = row["Name"].ToString();
            toAdd.RecordRequest = RequestAnalyser.AnalyseRequest(row["Request"].ToString());
            if( row["CategoryId"].ToString() != "" )
                toAdd.CategoryId = Convert.ToInt32(row["CategoryId"].ToString());
            //end of initialisation

            //initialisation of result to put the record in
            ResultRequest result;
            if( insertPosition == -1 )
            {
                result = new ResultRequest();
                result.Status = RequestStatus.NONE;
                result.ExecuteTest = executetest;
            }
            else
            {
                result = Program.ResultList[insertPosition];
                Program.ResultList.RemoveAt(insertPosition);
            }
            result.ResultRecord = toAdd;

            if( insertPosition == -1 )
                Program.ResultList.Add(result);//append the record to the list
            else
                Program.ResultList.Insert(insertPosition , result);//insert in at the specified position
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Does an update of the specified record</summary>
        /// <param name="record">Record to update</param>
        /// <param name="indexInList">Position in current list</param>
        /// <param name="updateGridView">Indicate if grid view must be update</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void UpdateRecord(Record record , int indexInList , bool updateGridView = false , bool updateBaseline = false)
        {
            if( updateBaseline )
            {
                m_dbConnect.InsertBaselineCommand(record.BaselineBitmap , record.Id); // insert baseline in database
                DataTable data = m_dbConnect.Execute(m_SqlFormat.QueryAll(DBTable.BASELINE)); // get all baseline from database
                record.MRBaselineId = Convert.ToInt32(data.Rows[data.Rows.Count - 1]["Id"].ToString()); //fin the new baseline Id and assign it to the record
            }

            m_dbConnect.Execute(m_SqlFormat.UpdateRecord(record)); //update record into database
            AddRecord(m_dbConnect.Execute(m_SqlFormat.QueryById(DBTable.RECORD , record.Id)).Rows[0] , indexInList); //reload record 

            if( updateGridView && Program.MainForm != null )
                Program.MainForm.UpdateRecordInGridView(record , indexInList);
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Reload the specified record at the specified position</summary>
        /// <param name="id">Id of the record to be reload</param>
        /// <param name="indexInList">Position in the list to insert remove/insert it</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void ReloadRecord(int id , int indexInList)
        {
            AddRecord(m_dbConnect.Execute(m_SqlFormat.QueryById(DBTable.RECORD , id)).Rows[0] , indexInList);//Reload
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>return the position in the list of the specified record</summary>
        /// <param name="id">Id of the record to look</param>
        /// <returns>Position in the list</returns>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public int FindRecordInList(int id)
        {
            for( int i = 0 ; i < Program.ResultList.Count ; i++ )
            {
                if( Program.ResultList[i].ResultRecord.Id == id )
                    return i;
            }
            return -1; //return -1 if record is not in the list 
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Insert specified record in list and database</summary>
        /// <param name="record">Record to insert</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void InsertRecord(Record record , bool insertInGridView = true)
        {
            m_dbConnect.Execute(m_SqlFormat.InsertRecord(record)); //insert record into database
            DataTable data = m_dbConnect.Execute(m_SqlFormat.QueryAll(DBTable.RECORD));
            record.Id = Convert.ToInt32(data.Rows[data.Rows.Count - 1]["Id"].ToString()); //assign the id to the record

            ResultRequest result = new ResultRequest(); //add the record to Program.ResultList 
            result.ResultRecord = record;
            result.Status = RequestStatus.NONE;
            result.ExecuteTest = true;
            Program.ResultList.Add(result);
            if( insertInGridView )
                Program.MainForm.AddToGridView(record);
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Delete specified record</summary>
        /// <param name="indexInList">Index of the record to delete in the program.recordList</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void DeleteRecord(int indexInList)
        {
            int recordId = Program.ResultList[indexInList].ResultRecord.Id;
            m_dbConnect.Execute(m_SqlFormat.deleteById(DBTable.RECORD , recordId));//delete the record
            DataTable data = m_dbConnect.Execute(m_SqlFormat.QueryResultByRecord(recordId));
            foreach( DataRow row in data.Rows )//delete all failure raster associate with request result to be delete
            {
                if( Convert.ToInt32(row["FailureRasterId"].ToString()) > 0 )
                    m_dbConnect.Execute(m_SqlFormat.deleteById(DBTable.FAILURERASTER , Convert.ToInt32(row["FailureRasterId"].ToString())));
            }
            m_dbConnect.Execute(m_SqlFormat.deleteByRecord(DBTable.REQUESTRESULT , recordId)); //delete all the request result associate with record
            m_dbConnect.Execute(m_SqlFormat.deleteByRecord(DBTable.BASELINE , recordId)); // delete all the baselines of the record
            Program.ResultList.RemoveAt(indexInList); //removes the record from the list
        }

        #endregion

        #region methods/result

        /*------------------------------------------------------------------------------------**/
        /// <summary>Add the specified result into the database</summary>
        /// <param name="result">Result to save in the database</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void AddResult(ResultRequest result)
        {
            Program.MainForm.LoadRecordInGridView(); //reload record in grid view
            int rasterId = 0;
            if( result.Status == RequestStatus.FAILED || result.Status == RequestStatus.WARNING || result.Status == RequestStatus.ERROR ) // if it is not a success, save the output file
            {
                m_dbConnect.InsertOutputImage(result); //insert raster in database
                DataTable data = m_dbConnect.Execute(m_SqlFormat.QueryAll(DBTable.FAILURERASTER));
                rasterId = Convert.ToInt32(data.Rows[data.Rows.Count - 1]["Id"].ToString());
            }

            m_dbConnect.Execute(m_SqlFormat.InsertResult(result , rasterId)); //insert result in database
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Add the specified result into the database</summary>
        /// <param name="result">Result to save in the database</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void LoadLastResult(int indexInList)
        {
            DataTable data = m_dbConnect.Execute(m_SqlFormat.QueryResultByRecord(Program.ResultList[indexInList].ResultRecord.Id)); //Query results with record id
            ResultRequest result = new ResultRequest();
            //find the latest result in the datatable and assign is value to result
            result.Id = Convert.ToInt32(data.Rows[data.Rows.Count - 1]["Id"].ToString());
            if( data.Rows[data.Rows.Count - 1]["Raster"].ToString() != "" )
            {
                byte[] bitmapBytes = (byte[]) data.Rows[data.Rows.Count - 1]["Raster"];
                result.ResultBitmap = bitmapBytes;
            }
            result.Status = (RequestStatus) Convert.ToInt32(data.Rows[data.Rows.Count - 1]["Status"].ToString());
            result.TestId = Convert.ToInt32(data.Rows[data.Rows.Count - 1]["TestId"].ToString());
            result.LinkedTest = LoadTestById(result.TestId);
            result.ResultRecord = Program.ResultList[indexInList].ResultRecord;
            result.InvalidPixels = Convert.ToInt32(data.Rows[data.Rows.Count - 1]["InvalidPixel"].ToString());
            result.BigDifferencePixels = Convert.ToInt32(data.Rows[data.Rows.Count - 1]["BigDifferencePixel"].ToString());
            //if the test was sucessful, set the result to the baseline
            if( result.Status == RequestStatus.SUCCESS )
                result.ResultBitmap = result.ResultRecord.BaselineBitmap;
            if( !result.ResultRecord.BaselineIsXML )
                result.DifferenceBitmap = RasterManipulation.GetDifferenceBitmap(result.ResultBitmap , result.ResultRecord.BaselineBitmap);

            Program.ResultList.RemoveAt(indexInList);
            Program.ResultList.Insert(indexInList , result); //update ResultList
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Get the list of all OldResult for the record</summary>
        /// <param name="recordIndex">Index of the record to look for</param>
        /// <returns>List of all old results</returns>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public List<ResultRequest> GetOldResult(int recordIndex)
        {
            List<ResultRequest> listResult = new List<ResultRequest>();
            DataTable data = m_dbConnect.Execute(m_SqlFormat.QueryFailedResultByRecord(recordIndex)); //get all result linked to this record
            foreach( DataRow row in data.Rows ) //initialized all value
            {
                ResultRequest result = new ResultRequest();
                result.Id = Convert.ToInt32(row["Id"].ToString());
                result.Status = (RequestStatus) Convert.ToInt32(row["Status"].ToString());
                result.TestId = Convert.ToInt32(row["TestId"].ToString());
                result.LinkedTest = LoadTestById(result.TestId);
                result.FailureRasterId = Convert.ToInt32(row["FailureRasterId"].ToString());
                result.InvalidPixels = Convert.ToInt32(row["InvalidPixel"].ToString());
                result.BigDifferencePixels = Convert.ToInt32(row["BigDifferencePixel"].ToString());
                listResult.Insert(0 , result);
            }
            return listResult;
        }

        #endregion

        #region methods/test

        /*------------------------------------------------------------------------------------**/
        /// <summary>Insert a test in the database</summary>
        /// <param name="test">Test to add in the database</param>
        /// <returns>Returns the id of the test in the database</returns>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public int AddTest(Test test)
        {
            m_dbConnect.Execute(m_SqlFormat.InsertTest(test)); //insert the test in the database

            DataTable data = m_dbConnect.Execute(m_SqlFormat.QueryAll(DBTable.TEST));
            return Convert.ToInt32(data.Rows[data.Rows.Count - 1]["Id"].ToString());
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Get a the test by is ID</summary>
        /// <param name="index">Index of the test to load</param>
        /// <returns>Test corresponding with the index</returns>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public Test LoadTestById(int index)
        {
            Test test = new Test();
            DataTable data = m_dbConnect.Execute(m_SqlFormat.QueryById(DBTable.TEST , index));//query the test by is Id 
            test.DateTime = Convert.ToInt32(data.Rows[0]["DateTime"].ToString());
            test.Id = index;
            test.Description = data.Rows[0]["Description"].ToString();
            test.Prefix =  data.Rows[0]["ServerConnection"].ToString();
            return test;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Get all Test in the database</summary>
        /// <returns>List of all tests</returns>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public List<Test> LoadAllTests()
        {
            DataTable data = m_dbConnect.Execute(m_SqlFormat.QueryAll(DBTable.TEST));
            List<Test> testList = new List<Test>();
            foreach( DataRow row in data.Rows )
            {
                Test test = new Test();
                test.DateTime = Convert.ToInt32(row["DateTime"].ToString());
                test.Description = row["Description"].ToString();
                test.Id = Convert.ToInt32(row["Id"].ToString());
                test.Prefix = row["ServerConnection"].ToString();
                testList.Add(test);
            }
            return testList;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Delete a test in the database, with all failureraster and requestresult linked to it</summary>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void DeleteTest(int index)
        {
            m_dbConnect.Execute(m_SqlFormat.deleteById(DBTable.TEST , index));//delete the Test
            DataTable data = m_dbConnect.Execute(m_SqlFormat.QueryResultByTest(index));
            foreach( DataRow row in data.Rows ) //delete all failure raster associate with request result to be delete
            {
                if( Convert.ToInt32(row["FailureRasterId"].ToString()) > 0 )
                    m_dbConnect.Execute(m_SqlFormat.deleteById(DBTable.FAILURERASTER , Convert.ToInt32(row["FailureRasterId"].ToString())));
            }
            m_dbConnect.Execute(m_SqlFormat.DeleteResultByTest(index)); //delete all the requestresult
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Returns the number of result in a test</summary>
        /// <param name="testIndex">Test to check</param>
        /// <returns>Number of result associate with the test</returns>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public int GetNumberOfRecordTest(int testIndex)
        {
            DataTable data = m_dbConnect.Execute(m_SqlFormat.GetNumberOfRecordTest(testIndex));
            return Convert.ToInt32(data.Rows[0]["NumberOfRecord"].ToString());
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Returns the number of unsuccessful result in a test</summary>
        /// <param name="testIndex">Test to check</param>
        /// <returns>Number of unsuccessful result associate with the test</returns>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public int GetNumberOfFailedTest(int testIndex)
        {
            DataTable data = m_dbConnect.Execute(m_SqlFormat.GetNumberOfFailedRecordTest(testIndex));
            return Convert.ToInt32(data.Rows[0]["NumberOfFailedRecord"].ToString());
        }

        #endregion

        #region methods/others

        /*------------------------------------------------------------------------------------**/
        /// <summary>Get the byte[] representing the bitmap of a failed request</summary>
        /// <param name="index">Index of the bitmap in the database</param>
        /// <returns>Failed bitmap as a byte[]</returns>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public Byte[] GetFailureBitmap(int index)
        {
            DataTable data = m_dbConnect.Execute(m_SqlFormat.QueryById(DBTable.FAILURERASTER , index));  //get the raster
            if( data.Rows[0]["Raster"].ToString() != "" )
            {
                byte[] bitmapBytes = (byte[]) data.Rows[data.Rows.Count - 1]["Raster"];
                return bitmapBytes;
            }
            return null;
        }
    
        /*------------------------------------------------------------------------------------**/
        /// <summary>Reload the connection string of the database</summary>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void ConnectionToDatabase()
        {
            m_dbConnect.ConnectionToDatabase();
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Get the list containing all prefix in current database</summary>
        /// <returns>List of all prefix in database</returns>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public List<string> GetAllPrefix()
        {
            List<string> PrefixList = new List<string>();
            DataTable data = m_dbConnect.Execute(m_SqlFormat.QueryAllPrefix()); // query all prefix
            foreach( DataRow row in data.Rows ) //add them to the list
            {
                string prefix = row[0].ToString();
                PrefixList.Add(prefix);
            }
            return PrefixList;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Get three list containing all protocol, server name and port number used in a request in current database</summary>
        /// <returns>Array of three list of string, [0]-> list of protocol, [1]-> list of server name, [2]-> list of port number</returns>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public List<string>[] GetSeparatePrefix()
        {
            List<string>[] decomposePrefix = new List<string>[3]; // initialise all three list
            decomposePrefix[0] = new List<string>();
            decomposePrefix[1] = new List<string>();
            decomposePrefix[2] = new List<string>();

            foreach( string prefix in GetAllPrefix() ) // for each prefix in the database
            {
                int ProtocolIndex = prefix.IndexOf("//")+2; // fin the end of the protocol
                string protocol = prefix.Remove(ProtocolIndex); //remove everything after //

                string serverName = prefix.Remove(prefix.LastIndexOf(":")); //find the end of the servername and delete everything after
                serverName = serverName.Replace(protocol , ""); // remmoves the protocol name

                string portNumber =   ( prefix.Replace(protocol , "") ).Replace(serverName , ""); //remove the protocol and server nam
                portNumber =  portNumber.Remove(portNumber.LastIndexOf("/")).Replace(":" , "");//find the end of the port number

                //Check if extracted protocol, server name and port number are in the list
                int listIndexOfProtocol = decomposePrefix[0].FindIndex(c => c == protocol);
                int listIndexOfServerName = decomposePrefix[1].FindIndex(c => c ==serverName);
                int listIndexOfPortNumber = decomposePrefix[2].FindIndex(c => c ==portNumber);

                //if they are, delete them and add them back to have them in order of last use
                if( listIndexOfProtocol >= 0 )
                    decomposePrefix[0].RemoveAt(listIndexOfProtocol);
                decomposePrefix[0].Add(protocol);

                if( listIndexOfServerName >= 0 )
                    decomposePrefix[1].RemoveAt(listIndexOfServerName);
                decomposePrefix[1].Add(serverName);

                if( listIndexOfPortNumber >= 0 )
                    decomposePrefix[2].RemoveAt(listIndexOfPortNumber);
                decomposePrefix[2].Add(portNumber);
            }
            return decomposePrefix;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Initialize connection string and makes sur the database exist, create one or connect to a default one if needed</summary>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void InitializeDatabase()
        {
            m_dbConnect.ConnectionToDatabase(); //initialize connection string
            if( !m_dbConnect.CheckIfDatabaseExist() ) // check if the db exist
            {
                try
                {
                    if( !System.IO.File.Exists(System.Windows.Forms.Application.UserAppDataPath + "\\database.db") ) //check if default database exists
                    {
                        //execute a batch file that creat a new database at a default location
                        Process process = new Process();
                        process.StartInfo.WorkingDirectory = string.Format("..\\..\\SqLite");
                        process.StartInfo.FileName = "database.bat";
                        process.StartInfo.Arguments = string.Format("\"" + System.Windows.Forms.Application.UserAppDataPath + "\\database.db" + "\"");
                        process.StartInfo.CreateNoWindow = false;
                        process.Start();
                        process.WaitForExit();
                        Properties.GeoWebPublisherUnitTestingApp.Default.DatabaseName = System.Windows.Forms.Application.UserAppDataPath + "\\database.db";
                        Properties.GeoWebPublisherUnitTestingApp.Default.Save();
                        Program.DBManager.ConnectionToDatabase();
                    }
                    else
                    {
                        //set the current database to the default one
                        Properties.GeoWebPublisherUnitTestingApp.Default.DatabaseName = System.Windows.Forms.Application.UserAppDataPath + "\\database.db";
                        Properties.GeoWebPublisherUnitTestingApp.Default.Save();
                        Program.DBManager.ConnectionToDatabase();
                    }
                }
                catch
                {
                }

            }
        }

        #endregion

        #endregion
    }
}
