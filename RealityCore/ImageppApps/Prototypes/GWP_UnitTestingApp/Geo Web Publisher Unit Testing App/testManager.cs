/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/GWP_UnitTestingApp/Geo Web Publisher Unit Testing App/testManager.cs $
|    $RCSfile: testManager.cs, $
|   $Revision: 1 $
|       $Date: 2013/06/5 $
|     $Author: Julien Rossignol $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Text;
using System.Threading;
using System.Windows.Forms;

namespace Geo_Web_Publisher_Unit_Testing_App
{
    /// <summary>Manage all operation to process a entire test</summary>
    /// <author>Julien Rossignol</author>
    public class TestManager
    {
        #region fields

        private HttpConnector m_HttpConnector; //get the http response to a request
        private ManualResetEvent m_WaitEvent; //allow the main thread to wait for smaller threads
        private volatile int m_CompareCount; //indicates how many comparaison is yet to be done
        private volatile bool m_IsWaiting; //indicates that the main thread is waiting for other thread
        private Test m_Test; //current test
        private string m_RequestTest; //test request used to check if the prefix is ok
        private volatile List<ResultRequest> m_ResultList; // list of result to full
        private volatile string m_Description; // description of the test

        #endregion

        #region parameters

        public string Description
        {
            get { return m_Description; }
            set { m_Description = value; }
        }

        #endregion

        #region constructors

        /*------------------------------------------------------------------------------------**/
        /// <summary>Create the https connector</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public TestManager()
        {
            m_HttpConnector = new HttpConnector();
        }

        #endregion

        #region methods

        /*------------------------------------------------------------------------------------**/
        /// <summary>Execute a entire test</summary>
        /// <param name="recordList">program's result list received when the thread is started</param>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void ExecuteTest(object recordList)
        {
            Program.MainForm.ChangeStatusBarMessage("Test initialisation");
            //test initialization
            m_RequestTest = Properties.GeoWebPublisherUnitTestingApp.Default.TestRequest;
            m_Test = new Test();
            m_Test.DateTime = TimeOfTest();
            m_Test.Description = m_Description;
            m_Test.Prefix = Properties.GeoWebPublisherUnitTestingApp.Default.UrlPrefix;
            if( m_Test.Description == null )
                m_Test.Description = "No description";

            try//try to execute the test request
            {
                Program.MainForm.ChangeStatusBarMessage("Test request execution");
                m_HttpConnector.ExecuteRequestWithoutProtection(m_Test.Prefix + m_RequestTest);
            }
            catch //stop the test if the test request failed
            {
                Program.IsTestRunning = false;
                Program.MainForm.ChangeStatusBarMessage("Test request failed");
                MessageBox.Show("Test request failed. Make sure it is a working getmap request and verify protocol, server name and port number" , "Test request failed" , MessageBoxButtons.OK , MessageBoxIcon.Error);
                return;
            }
            //initiliaze class data for the test
            Program.MainForm.ChangeStatusBarMessage("Test request success");
            m_Test.Id = Program.DBManager.AddTest(m_Test);
            m_ResultList = (List<ResultRequest>) recordList;
            m_WaitEvent = new ManualResetEvent(false);
            m_CompareCount = 0;
            m_IsWaiting = false;

            //execute each request
            for( int i = 0 ; i < m_ResultList.Count ; i++ )
            {
                if( Program.IsTestRunning )
                {
                    if( m_ResultList[i].ExecuteTest && m_ResultList[i].ResultRecord.BaselineIsUpToDate ) //check if the baseline is uptodate and if the result is checked
                    {
                        m_CompareCount++;
                        DoRequest(m_ResultList[i]);
                    }
                }
                else
                    break;
            }
            if( Program.IsTestRunning )
            {
                m_IsWaiting = true;
                m_WaitEvent.WaitOne(); //wait till all the thread are finished
            }
            Program.MainForm.ChangeStatusBarMessage("End of test");
            Program.ResultList = m_ResultList;
            Program.MainForm.LoadRecordInGridView();
            Program.IsTestRunning = false;
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Executes one request and compares it to the baseline</summary>
        /// <param name="result">Result with the request to be done</param>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void DoRequest(ResultRequest result)
        {
            //result initiliazation
            result.TestId = m_Test.Id;
            result.LinkedTest = m_Test;
            result.FailureRasterId = 0;
            Program.MainForm.ChangeStatusBarMessage("Executing request Id=" + result.ResultRecord.Id);
            result.ResultBitmap = m_HttpConnector.ExecuteRequest(m_Test.Prefix + result.ResultRecord.RecordRequest.RequestString); //get the response of the request
            Program.MainForm.AdvanceProgressBar();
            Program.MainForm.ChangeStatusBarMessage("Receiving request Id=" + result.ResultRecord.Id);
            if( Program.IsTestRunning )
            {
                ThreadPool.QueueUserWorkItem(new WaitCallback(Compare) , result); //compare the result with the baseline
                Program.MainForm.AdvanceProgressBar();
            }

        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Compare the baseline and the output received</summary>  
        /// <param name="data">Result with both byte[] to compare</param>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void Compare(object data)
        {
            ResultRequest result = (ResultRequest) data;

            Bitmap response = RasterManipulation.ConvertByteArrayToBitmap(result.ResultBitmap); //creates response bitmap
            if( response != null && !result.ResultRecord.BaselineIsXML )
            {
                Program.MainForm.ChangeStatusBarMessage("Comparing bitmap Id=" + result.ResultRecord.Id);
                Bitmap baseline = RasterManipulation.ConvertByteArrayToBitmap(result.ResultRecord.BaselineBitmap); //creates baseline bitmap
                Bitmap difference = new Bitmap(response); //create difference bitmap

                int invalidPixel = 0;
                int bigDifferencePixel = 0;

                for( int i = 0 ; i < response.Height ; i++ ) //compare each pixel one by one
                {
                    for( int j = 0 ; j < response.Width ; j++ )
                    {
                        PixelComparison(ref response , ref baseline , ref invalidPixel , ref bigDifferencePixel , j , i);
                    }
                }
                result.BigDifferencePixels = bigDifferencePixel;
                result.InvalidPixels = invalidPixel;
                if( (double) invalidPixel / ( response.Height * response.Width ) > 0.01 ) //decide what is the result of the comparison
                    result.Status = RequestStatus.FAILED;
                else if( (double) invalidPixel / bigDifferencePixel < 3 )
                    result.Status = RequestStatus.FAILED;
                else if( invalidPixel == 0 )
                    result.Status = RequestStatus.SUCCESS;
                else
                    result.Status = RequestStatus.WARNING;

                response.Dispose();
                baseline.Dispose();
            }
            else if( response == null && result.ResultRecord.BaselineIsXML ) //if output and baseline are string compare both
            {
                Program.MainForm.ChangeStatusBarMessage("Comparing text files Id=" + result.ResultRecord.Id);
                if( Encoding.GetEncoding(28591).GetString(result.ResultBitmap) == Encoding.GetEncoding(28591).GetString(result.ResultRecord.BaselineBitmap) )
                    result.Status = RequestStatus.SUCCESS;
            }

            lock( this ) //make sure no more than one thread modify sensible data at the same time
            {
                Program.MainForm.ChangeStatusBarMessage("Saving result request Id=" + result.ResultRecord.Id);
                if( result.Status == RequestStatus.NONE )
                    result.Status = RequestStatus.ERROR;
                m_CompareCount--;
                Program.DBManager.AddResult(result); //add result in the database
                int position = Program.DBManager.FindRecordInList(result.ResultRecord.Id); //update result list
                m_ResultList.RemoveAt(position);
                m_ResultList.Insert(position , result);
                Program.ResultList = m_ResultList;
                if( m_CompareCount == 0 && m_IsWaiting )
                {
                    this.m_WaitEvent.Set();
                }
            }

        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Compare two pixels</summary>
        /// <param name="bigDifferencePixel">Number of big difference pixels in the bitmap</param>
        /// <param name="comparisonBitmap">Baseline bitmap</param>
        /// <param name="invalidPixel">number of invalid pixel in the bitmap</param>
        /// <param name="responseBitmap">Repsonse bitmap</param>
        /// <param name="j">Y position of the pixel to compare</param>
        /// <param name="i">X position of the pixel to compare</param>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void PixelComparison(ref Bitmap responseBitmap , ref Bitmap comparisonBitmap , ref int invalidPixel , ref int bigDifferencePixel , int j , int i)
        {
            Color responseColor = responseBitmap.GetPixel(j , i);
            Color comparisonColor = comparisonBitmap.GetPixel(j , i);
            if( responseColor != comparisonColor )
            {
                invalidPixel++;

                if( Math.Abs(responseColor.R - comparisonColor.R) > 15 || Math.Abs(responseColor.G - comparisonColor.G) > 15 || Math.Abs(responseColor.B - comparisonColor.B) > 15 )
                    bigDifferencePixel++;
            }
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Get the current time of the test</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public int TimeOfTest()
        {
            TimeSpan span = ( DateTime.Now - new DateTime(1970 , 1 , 1 , 0 , 0 , 0 , 0).ToLocalTime() );

            return (int)( span + System.TimeZoneInfo.Local.BaseUtcOffset).TotalSeconds; // -5*3600 mused to get gmt-5 time
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Get the response for one request to serve a baseline</summary>  
        /// <param name="record">Record to execute</param>
        /// <param name="UpdateDataGrigView">Tells the dbmanager if he must update the datagridview</param>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void FetchBaseline(Record record , bool UpdateDataGrigView = true)
        {
            record.BaselineBitmap = m_HttpConnector.ExecuteRequest(Properties.GeoWebPublisherUnitTestingApp.Default.UpdateServerInfo + record.RecordRequest.RequestString);
            Program.DBManager.UpdateRecord(record , Program.DBManager.FindRecordInList(record.Id) , UpdateDataGrigView , true);
        }

        #endregion
    }
}
