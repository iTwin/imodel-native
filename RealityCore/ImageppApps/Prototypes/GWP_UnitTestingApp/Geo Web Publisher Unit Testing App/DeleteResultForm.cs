/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/GWP_UnitTestingApp/Geo Web Publisher Unit Testing App/DeleteResultForm.cs $
|    $RCSfile: DeleteResultForm.cs, $
|   $Revision: 1 $
|       $Date: 2013/06/10 $
|     $Author: Julien Rossignol $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace Geo_Web_Publisher_Unit_Testing_App
{
    /// <summary>Form allowing the user to see all registered test to delete them</summary>
    /// <author>Julien Rossignol</author>
    public partial class DeleteResultForm : Form
    {
        #region fields

        List<Test> m_TestList; // all test in the database
        DataGridViewRow m_ExempleRow; // exemple row to ease datagridview initialisation

        #endregion

        #region constructors

        /*------------------------------------------------------------------------------------**/
        /// <summary>Initialize components, and load data into the grid view</summary> 
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public DeleteResultForm()
        {
            InitializeComponent();
            InitializeExempleRow();
            LoadResult();
        }

        #endregion

        #region methods

        /*------------------------------------------------------------------------------------**/
        /// <summary>Load all result into the data grid view</summary> 
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void LoadResult()
        {
            m_TestList = Program.DBManager.LoadAllTests();//get all tests from the database
            foreach( Test test in m_TestList )
            {
                DataGridViewRow row = (DataGridViewRow) m_ExempleRow.Clone(); //clone the exemple row to ease initliasation
                row.Cells[0].Value = test.Id;
                row.Cells[1].Value = GetDate(test.DateTime) + " " + GetTime(test.DateTime);
                row.Cells[2].Value = test.Description;
                row.Cells[3].Value = Program.DBManager.GetNumberOfFailedTest(test.Id);
                row.Cells[4].Value = Program.DBManager.GetNumberOfRecordTest(test.Id);
                TestDataGridView.Rows.Add(row);
            }
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Initialize the exemple row</summary> 
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void InitializeExempleRow()
        {
            m_ExempleRow = new DataGridViewRow();
            m_ExempleRow.Cells.Add(new DataGridViewTextBoxCell());
            m_ExempleRow.Cells.Add(new DataGridViewTextBoxCell());
            m_ExempleRow.Cells.Add(new DataGridViewTextBoxCell());
            m_ExempleRow.Cells.Add(new DataGridViewTextBoxCell());
            m_ExempleRow.Cells.Add(new DataGridViewTextBoxCell());
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Verify and delete the selection when delete button is clicked</summary> 
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void deleteButton_Click(object sender , EventArgs e)
        {

            DialogResult result = MessageBox.Show("Are you sure your want to delete selected test(s)?" , "Warning" , MessageBoxButtons.OKCancel , MessageBoxIcon.Exclamation);
            if( result == DialogResult.OK ) // if the users said yes
            {
                while( TestDataGridView.SelectedRows.Count != 0 )//while there is a selection
                {
                    Program.DBManager.DeleteTest(Convert.ToInt32(TestDataGridView.SelectedRows[0].Cells[0].Value.ToString())); //delete the selected test in the database
                    TestDataGridView.Rows.Remove(TestDataGridView.SelectedRows[0]); //delete it in the grid view
                }
            }
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Activate and desactivate the delete button if there is no selection</summary> 
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void TestDataGridView_SelectionChanged(object sender , EventArgs e)
        {
            if( TestDataGridView.SelectedRows.Count < 1 ) // if there is no selection
                deleteButton.Enabled = false;
            else
                deleteButton.Enabled = true;
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Return the date of the test</summary>
        /// <param name="timestamp">Timestamp to get the date from</param>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private string GetDate(int timestamp)
        {
            DateTime date = new DateTime(1970 , 1 , 1 , 0 , 0 , 0 , 0); // base date for unix timestamp 
            date = date.AddSeconds(timestamp);
            return date.Date.ToShortDateString();
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Return the time of the test</summary> 
        /// <param name="timestamp">Timestamp to get the time from</param>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private string GetTime(int timestamp)
        {
            DateTime time = new DateTime(1970 , 1 , 1 , 0 , 0 , 0 , 0);// base date for unix timestamp 
            time = time.AddSeconds(timestamp);
            return time.TimeOfDay.ToString();
        }

        #endregion
    }
}
