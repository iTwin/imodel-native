/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/GWP_UnitTestingApp/Geo Web Publisher Unit Testing App/mainForm.cs $
|    $RCSfile: MainForm.cs, $
|   $Revision: 1 $
|       $Date: 2013/05/27 $
|     $Author: Julien Rossignol $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;


namespace Geo_Web_Publisher_Unit_Testing_App
{
    /// <summary>Manage the main form, also manage when to show the other forms</summary>
    /// <author>Julien Rossignol</author>
    public partial class MainForm : Form
    {
        #region fields

        private SelectWmsBaselineForm m_SelectWmsForm;
        private ResultComparisonForm m_ResultComparisonForm;
        private SettingForm m_SettingForm;
        private DataGridViewRow m_ExempleRow;
        private bool m_IsNotVisibleYet;
        private DeleteResultForm m_DeleteResultForm;

        #endregion

        #region constructors

        /*------------------------------------------------------------------------------------**/
        /// <summary>Initialize form components and MRU list</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public MainForm()
        {
            m_IsNotVisibleYet = true; //prevent the constructor of the form to change sizes specified in properties
            InitializeComponent();
            LoadCategories();
            statusDropDownList.SelectedIndex = 0;
            InitializePrefix();//intiliaze protocol/servername/portnumber combobox
            EvaluatePrefix(); //get all combobox value together in a prefix
            LoadRecordInGridView(); // load the grid view with data
        }

        #endregion

        #region methods

        #region methods/initiliaze

        /*------------------------------------------------------------------------------------**/
        /// <summary>Initialize protocol, server name and port number combo box</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void InitializePrefix()
        {
            //make sure all combobox are empty
            comboBoxProtocol.Items.Clear();
            comboBoxPortNumber.Items.Clear();
            comboBoxServerName.Items.Clear();
            List<string>[] prefixList = Program.DBManager.GetSeparatePrefix();//get all value for all combo box
            foreach( string protocol in prefixList[0] )
            {
                comboBoxProtocol.Items.Insert(0 , protocol); //add all protocol in protocol list 
            }
            foreach( string serverName in prefixList[1] )
            {
                comboBoxServerName.Items.Insert(0 , serverName); //add all server name in server name list
            }
            foreach( string portNumber in prefixList[2] )
            {
                comboBoxPortNumber.Items.Insert(0 , portNumber); //add all portNumber in port number list
            }
            if( comboBoxProtocol.Items.Count == 0 )
                comboBoxProtocol.Items.Insert(0 , "http://"); //if empty, insert a default value
            comboBoxProtocol.SelectedIndex = 0;
            if( comboBoxServerName.Items.Count == 0 )
                comboBoxServerName.Items.Insert(0 , "localhost"); //if empty, insert a default value
            comboBoxServerName.SelectedIndex = 0;
            if( comboBoxPortNumber.Items.Count == 0 )
                comboBoxPortNumber.Items.Insert(0 , "8087"); //if empty, insert a default value
            comboBoxPortNumber.SelectedIndex = 0;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Initilize the SelectAll check box on top of the grid view</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void InitiliazeSelectAll()
        {
            System.Windows.Forms.CheckBox SelectAllCheckBox = new System.Windows.Forms.CheckBox();
            System.Drawing.Rectangle SelectAllRect = requestDataGridView.GetCellDisplayRectangle(0 , -1 , false); //get the location of the header
            SelectAllRect.X = SelectAllRect.Location.X + 10;//center the check box horizontaly
            SelectAllRect.Y = SelectAllRect.Location.Y + 4; //center the check box vertically
            SelectAllCheckBox.Location = SelectAllRect.Location;
            SelectAllCheckBox.Size = new System.Drawing.Size(13 , 13); // 13*13 is slightly bigger than other check box in the column, but using a smaller size causes the check box border to glitch
            requestDataGridView.Controls.Add(SelectAllCheckBox);
            SelectAllCheckBox.CheckedChanged += new EventHandler(SelectAllCheckBox_CheckChanged); //link the check box to to correct event
            SelectAllCheckBox.Checked = true;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Initialize an exemple row to append record to the gridview more easily</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void InitilializeExempleRow()
        {
            m_ExempleRow = new DataGridViewRow();
            m_ExempleRow.Cells.Add(new System.Windows.Forms.DataGridViewCheckBoxCell());
            m_ExempleRow.Cells.Add(new System.Windows.Forms.DataGridViewTextBoxCell());
            m_ExempleRow.Cells.Add(new System.Windows.Forms.DataGridViewImageCell());
            m_ExempleRow.Cells.Add(new System.Windows.Forms.DataGridViewTextBoxCell());
            m_ExempleRow.Cells.Add(new System.Windows.Forms.DataGridViewTextBoxCell());
            System.Windows.Forms.DataGridViewComboBoxCell categoryCell = new System.Windows.Forms.DataGridViewComboBoxCell();
            foreach( Category category in Program.CategoryList )//initialize category drop-down
            {
                categoryCell.Items.Add(category.Name);
            }
            categoryCell.Items.Add("None");
            m_ExempleRow.Cells.Add(categoryCell);
            m_ExempleRow.Cells.Add(new System.Windows.Forms.DataGridViewTextBoxCell());
            m_ExempleRow.Cells.Add(new System.Windows.Forms.DataGridViewTextBoxCell());
            m_ExempleRow.Cells[1].ReadOnly = true;
            m_ExempleRow.Cells[7].ReadOnly = true;

        }


        /*------------------------------------------------------------------------------------**/
        /// <summary>Load categories and modifies datagrieview combobox column to show changes in categories</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void LoadCategories()
        {
            try
            {
                InitilializeExempleRow(); // ask for another initiliazation of the exemple row since it load categories into the row
                foreach( DataGridViewRow row in requestDataGridView.Rows )
                {
                    if( !row.IsNewRow )
                        row.Cells[5] = (DataGridViewComboBoxCell) m_ExempleRow.Cells[5].Clone();
                }
                if( m_SelectWmsForm != null )
                    m_SelectWmsForm.LoadCategories(); //load the category for SelectWmsForm if needed

                categoryDropDownList.Items.Clear(); // clear and full the category filter drop down
                categoryDropDownList.Items.Add("All");
                foreach( Category category in Program.CategoryList )
                {
                    categoryDropDownList.Items.Add(category.Name);
                }
                categoryDropDownList.Items.Add("None");
                categoryDropDownList.SelectedIndex = 0;//select the "All" choice
            }
            catch
            {
                categoryDropDownList.SelectedIndex = 0;
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Load previous session properties, windows size, location and grid view parameters</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void LoadWindowsProperties()
        {
            int Y = Properties.GeoWebPublisherUnitTestingApp.Default.WindowLocationY;
            int X= Properties.GeoWebPublisherUnitTestingApp.Default.WindowLocationX;

            this.DesktopLocation = new Point(X , Y);

            //load all other properties(mostly datagridview display properties)
            this.Size = new Size(Properties.GeoWebPublisherUnitTestingApp.Default.WindowSizeW , Properties.GeoWebPublisherUnitTestingApp.Default.WindowSizeH);
            this.requestDataGridView.Columns["ID"].DisplayIndex = Properties.GeoWebPublisherUnitTestingApp.Default.GridViewIdOrder;
            this.requestDataGridView.Columns["NameGrid"].DisplayIndex = Properties.GeoWebPublisherUnitTestingApp.Default.GridViewNameOrder;
            this.requestDataGridView.Columns["Description"].DisplayIndex = Properties.GeoWebPublisherUnitTestingApp.Default.GridViewDescriptionOrder;
            this.requestDataGridView.Columns["Category"].DisplayIndex = Properties.GeoWebPublisherUnitTestingApp.Default.GridViewCategoryOrder;
            this.requestDataGridView.Columns["URL"].DisplayIndex = Properties.GeoWebPublisherUnitTestingApp.Default.GridViewUrlOrder;
            this.requestDataGridView.Columns["Status"].DisplayIndex = Properties.GeoWebPublisherUnitTestingApp.Default.GridViewStatusOrder;
            this.requestDataGridView.Columns["UpToDate"].DisplayIndex = Properties.GeoWebPublisherUnitTestingApp.Default.GridViewUpToDateOrder;
            this.requestDataGridView.Columns["ID"].Width = Properties.GeoWebPublisherUnitTestingApp.Default.GridViewIdWidth;
            this.requestDataGridView.Columns["NameGrid"].Width = Properties.GeoWebPublisherUnitTestingApp.Default.GridViewNameWidth;
            this.requestDataGridView.Columns["Description"].Width = Properties.GeoWebPublisherUnitTestingApp.Default.GridViewDescriptionWidth;
            this.requestDataGridView.Columns["Category"].Width = Properties.GeoWebPublisherUnitTestingApp.Default.GridViewCategoryWidth;
            this.requestDataGridView.Columns["URL"].Width = Properties.GeoWebPublisherUnitTestingApp.Default.GridViewUrlWidth;
            this.requestDataGridView.Columns["Status"].Width = Properties.GeoWebPublisherUnitTestingApp.Default.GridViewStatusWidth;
            this.requestDataGridView.Columns["UpToDate"].Width = Properties.GeoWebPublisherUnitTestingApp.Default.GridViewUpToDateWidth;
        }

        #endregion

        #region methods/eventsHandler

        #region methods/eventsHandler/gridViewEvents

        /*------------------------------------------------------------------------------------**/
        /// <summary>Show a comparisonform already initialized on row header double-click</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void requestDataGridView_RowHeaderMouseDoubleClick(object sender , DataGridViewCellMouseEventArgs e)
        {
            try
            {
                m_ResultComparisonForm.BringToFront();
                m_ResultComparisonForm.Show();
            }
            catch // if the comparison form has not yet been created
            {
                m_ResultComparisonForm = new ResultComparisonForm();
                m_ResultComparisonForm.Show();
            }
            finally // set the comparison form data to the selected row
            {
                if( requestDataGridView.SelectedRows.Count == 1 )
                    m_ResultComparisonForm.IteratorList = requestDataGridView.SelectedRows[0].Index + 1;
                else
                    m_ResultComparisonForm.IteratorList = 1;
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>prevent the row header width to be changed(preventing the select all check box from being out of position)</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void requestDataGridView_RowHeadersWidthChanged(object sender , EventArgs e)
        {
            this.requestDataGridView.RowHeadersWidth = 40;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Change the selected record to fit the change in select all check box</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void SelectAllCheckBox_CheckChanged(object sender , EventArgs e)
        {
            if( !Program.IsTestRunning ) // prevent the user to deselect record during a test
            {
                foreach( DataGridViewRow row in requestDataGridView.Rows )
                {
                    if( !row.IsNewRow )
                    {
                        ResultRequest resultToChange = Program.ResultList[row.Index]; //get the result
                        row.Cells[0].Value = ( (CheckBox) sender ).Checked; // get the value of the check box
                        requestDataGridView.UpdateCellValue(0 , row.Index);
                        resultToChange.ExecuteTest = ( (CheckBox) sender ).Checked; // set the value of the result
                        Program.ResultList[row.Index] = resultToChange; // mutate result in the result list
                    }
                }
                requestDataGridView.RefreshEdit(); // this line is necessary to make sure that the cell currently being edit is correctly updated 
            }
            else
                ( (CheckBox) sender ).Checked = !( (CheckBox) sender ).Checked; // revert changes on the select all check box
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Change the result executeTest value to fit the check box in the row</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void requestDataGridView_CellContentClick(object sender , DataGridViewCellEventArgs e)
        {
            if( e.RowIndex >=0 && e.ColumnIndex == 0 && !Program.IsTestRunning )//only catches event of the check box column
            {
                ResultRequest resultToChange = Program.ResultList[e.RowIndex];
                resultToChange.ExecuteTest = !resultToChange.ExecuteTest;
                Program.ResultList[e.RowIndex] = resultToChange;
                requestDataGridView.Rows[e.RowIndex].Cells[0].Selected = false;
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>End data grid view edit if focus is lost</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void requestDataGridView_Leave(object sender , EventArgs e)
        {
            requestDataGridView.EndEdit();
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>End data grid view edit if focus is lost</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void requestDataGridView_CellLeave(object sender , DataGridViewCellEventArgs e)
        {
            requestDataGridView.EndEdit();
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Initialize properly a new row when the user add one in the datagridview</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void requestDataGridView_UserAddedRow(object sender , DataGridViewRowEventArgs e)
        {
            requestDataGridView.CurrentRow.Cells[3].Value = "";
            requestDataGridView.CurrentRow.Cells[4].Value = "";
            requestDataGridView.CurrentRow.Cells[5] = (DataGridViewCell) m_ExempleRow.Cells[5].Clone();
            requestDataGridView.CurrentRow.Cells[5].Value = "None";
            requestDataGridView.CurrentRow.Cells[6].Value = "";
            Record record = new Record();
            record.Name = "";
            record.Description = "";
            record.RecordRequest = RequestAnalyser.AnalyseRequest("");
            Program.DBManager.InsertRecord(record , false);
            requestDataGridView.CurrentRow.Cells[1].Value = Program.ResultList[Program.ResultList.Count-1].ResultRecord.Id;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Check how many record are selected in the grid view and enabled/disabled buttons accordinly</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void requestDataGridView_SelectionChanged(object sender , EventArgs e)
        {
            if( requestDataGridView.SelectedRows.Count == 1)
            {
                if( !requestDataGridView.SelectedRows[0].IsNewRow )
                {
                    EditButton.Enabled = true;
                    DeleteButton.Enabled = true;
                    CopyUrlToClipboard.Enabled = true;
                    updateButton.Enabled = true;
                }
            }
            else if( requestDataGridView.SelectedRows.Count > 1 )
            {
                EditButton.Enabled = false;
                DeleteButton.Enabled = true;
                CopyUrlToClipboard.Enabled = false;
                updateButton.Enabled = true;
            }
            else
            {
                EditButton.Enabled = false;
                DeleteButton.Enabled = false;
                CopyUrlToClipboard.Enabled = false;
                updateButton.Enabled = false;
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Save datagridview column display index when modified</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void requestDataGridView_ColumnDisplayIndexChanged(object sender , DataGridViewColumnEventArgs e)
        {
            if( !m_IsNotVisibleYet )// this if statement prevent the constructor of the form to save data into the program settings
            {
                Properties.GeoWebPublisherUnitTestingApp.Default.GridViewIdOrder = requestDataGridView.Columns["ID"].DisplayIndex;
                Properties.GeoWebPublisherUnitTestingApp.Default.GridViewNameOrder = requestDataGridView.Columns["NameGrid"].DisplayIndex;
                Properties.GeoWebPublisherUnitTestingApp.Default.GridViewDescriptionOrder = requestDataGridView.Columns["Description"].DisplayIndex;
                Properties.GeoWebPublisherUnitTestingApp.Default.GridViewCategoryOrder = requestDataGridView.Columns["Category"].DisplayIndex;
                Properties.GeoWebPublisherUnitTestingApp.Default.GridViewUrlOrder = requestDataGridView.Columns["URL"].DisplayIndex;
                Properties.GeoWebPublisherUnitTestingApp.Default.GridViewStatusOrder = requestDataGridView.Columns["Status"].DisplayIndex;
                Properties.GeoWebPublisherUnitTestingApp.Default.GridViewUpToDateOrder = requestDataGridView.Columns["UpToDate"].DisplayIndex;
                Properties.GeoWebPublisherUnitTestingApp.Default.Save();
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Save datagridview columns width when modified</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void requestDataGridView_ColumnWidthChanged(object sender , DataGridViewColumnEventArgs e)
        {
            if( !m_IsNotVisibleYet )// this if statement prevent the constructor of the form to save data into the program settings
            {
                Properties.GeoWebPublisherUnitTestingApp.Default.GridViewIdWidth = requestDataGridView.Columns["ID"].Width;
                Properties.GeoWebPublisherUnitTestingApp.Default.GridViewNameWidth = requestDataGridView.Columns["NameGrid"].Width;
                Properties.GeoWebPublisherUnitTestingApp.Default.GridViewDescriptionWidth = requestDataGridView.Columns["Description"].Width;
                Properties.GeoWebPublisherUnitTestingApp.Default.GridViewCategoryWidth = requestDataGridView.Columns["Category"].Width;
                Properties.GeoWebPublisherUnitTestingApp.Default.GridViewUrlWidth = requestDataGridView.Columns["URL"].Width;
                Properties.GeoWebPublisherUnitTestingApp.Default.GridViewStatusWidth = requestDataGridView.Columns["Status"].Width;
                Properties.GeoWebPublisherUnitTestingApp.Default.GridViewUpToDateWidth = requestDataGridView.Columns["UpToDate"].Width;
                Properties.GeoWebPublisherUnitTestingApp.Default.Save();
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Delete selection when the delete key is pressed</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void requestDataGridView_KeyDown(object sender , KeyEventArgs e)
        {
            if( e.KeyCode == Keys.Delete && requestDataGridView.SelectedRows.Count != 0)
            {
                DeleteSelection();
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Cancel Datagridcombobox error message, if you want to know what it means, delete 
        /// comment this function and rename a category the cancelled error has been prooved to be a 
        /// .net internal mistake</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void requestDataGridView_DataError(object sender , DataGridViewDataErrorEventArgs e)
        {
            if( e.Context == DataGridViewDataErrorContexts.Formatting || e.Context == DataGridViewDataErrorContexts.PreferredSize )
                e.ThrowException = false;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Save modifications made to a cell</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void requestDataGridView_CellEndEdit(object sender , DataGridViewCellEventArgs e)
        {
            if( !Program.IsTestRunning )//forbid modification while test is running
            {
                int selectedRow = e.RowIndex;
                if( !requestDataGridView.Rows[selectedRow].IsNewRow )
                {
                    int recordInList = Program.DBManager.FindRecordInList(Convert.ToInt32(requestDataGridView.Rows[selectedRow].Cells[1].Value.ToString()));//position of the record in the list
                    if( recordInList != -1 )
                    {
                        Record recordModified = Program.ResultList[recordInList].ResultRecord;
                        try
                        {
                            recordModified.Name = requestDataGridView.Rows[selectedRow].Cells[3].Value.ToString();
                        }
                        catch
                        {
                            requestDataGridView.Rows[selectedRow].Cells[3].Value = recordModified.Name;
                        }
                        try
                        {
                            recordModified.Description = requestDataGridView.Rows[selectedRow].Cells[4].Value.ToString();
                        }
                        catch
                        {
                            recordModified.Description = "";
                        }

                        recordModified.CategoryId = Program.DBManager.LookForCategoryId(requestDataGridView.Rows[selectedRow].Cells[5].Value.ToString());
                        try
                        {
                            if( recordModified.RecordRequest.RequestString !=requestDataGridView.Rows[selectedRow].Cells[6].Value.ToString() )//if url was changed
                            {
                                recordModified.RecordRequest = RequestAnalyser.AnalyseRequest(requestDataGridView.Rows[selectedRow].Cells[6].Value.ToString());

                                if( Properties.GeoWebPublisherUnitTestingApp.Default.AutoCheckUpdate ) // update the baseline if proper option is selected
                                {
                                    recordModified.BaselineIsUpToDate = true;
                                    Program.TestManager.FetchBaseline(recordModified , false);
                                }
                                else // indicate that the baseline is not up to date
                                {
                                    recordModified.BaselineIsUpToDate = false;
                                    recordModified.MRBaselineId = 0;
                                    requestDataGridView.Rows[selectedRow].Cells[2].Value = global::Geo_Web_Publisher_Unit_Testing_App.Properties.Resources.Warning;
                                    Program.DBManager.UpdateRecord(recordModified , recordInList);//update the record in the database
                                }
                            }
                            else
                            {
                                Program.DBManager.UpdateRecord(recordModified , recordInList);//update the record in the database
                            }
                        }
                        catch
                        {
                            requestDataGridView.Rows[selectedRow].Cells[6].Value = recordModified.RecordRequest.RequestString;
                        }
                        if( m_ResultComparisonForm != null )//update resultcomparison if needed
                            m_ResultComparisonForm.UpdateInfo();
                    }
                }
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Enable the comparison button when there is a record in the data grid view</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void requestDataGridView_RowsAdded(object sender , DataGridViewRowsAddedEventArgs e)
        {
            ResultComparisonButton.Enabled = true;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Disabled the comparison button when there is no record in the data grid view</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void requestDataGridView_RowsRemoved(object sender , DataGridViewRowsRemovedEventArgs e)
        {
            if( requestDataGridView.RowCount >= 1 )
                ResultComparisonButton.Enabled = false;
        }
        #endregion

        #region methods/eventsHandler/formEvents

        /*------------------------------------------------------------------------------------**/
        /// <summary>Makes sur the user really want to quit if he close the app while a test is running</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void MainForm_FormClosing(object sender , FormClosingEventArgs e)
        {
            if( Program.IsTestRunning )
            {
                DialogResult result = MessageBox.Show("Test currently in progress, are you sure you want to close the application?" , "Test in progress" , MessageBoxButtons.OKCancel , MessageBoxIcon.Exclamation);
                if( result == System.Windows.Forms.DialogResult.Cancel )
                    e.Cancel = true;
                else
                    e.Cancel = false;
            }
            else
                e.Cancel = false;
        }


        /*------------------------------------------------------------------------------------**/
        /// <summary>Store the size on the windows when modified</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void mainForm_SizeChanged(object sender , EventArgs e)
        {
            if( !m_IsNotVisibleYet ) // this if statement prevent the constructor of the form to save data into the program settings
            {
                Properties.GeoWebPublisherUnitTestingApp.Default.WindowSizeH = Size.Height;
                Properties.GeoWebPublisherUnitTestingApp.Default.WindowSizeW = Size.Width;
                Properties.GeoWebPublisherUnitTestingApp.Default.Save();
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Store le location of the window when modified</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void mainForm_LocationChanged(object sender , EventArgs e)
        {
            if( !m_IsNotVisibleYet ) // this if statement prevent the constructor of the form to save data into the program settings
            {
                Properties.GeoWebPublisherUnitTestingApp.Default.WindowLocationX = DesktopLocation.X;
                Properties.GeoWebPublisherUnitTestingApp.Default.WindowLocationY = DesktopLocation.Y;
                Properties.GeoWebPublisherUnitTestingApp.Default.Save();
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Make sure no base initialisation override user properties by setting them after construction</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void mainForm_Shown(object sender , EventArgs e)
        {
            LoadWindowsProperties();
            InitiliazeSelectAll();
            m_IsNotVisibleYet = false;
        }

        #endregion

        #region methods/eventsHandler/sidePanelEvents

        /*------------------------------------------------------------------------------------**/
        /// <summary>Open the delete result form</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void deleteResultButton_Click(object sender , EventArgs e)
        {
            if( !Program.IsTestRunning )
            {
                try
                {
                    m_DeleteResultForm.BringToFront();
                    m_DeleteResultForm.Show();
                }
                catch // if the delete result form was not create yet
                {
                    m_DeleteResultForm = new DeleteResultForm();
                    m_DeleteResultForm.Show();
                }
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Update all selected baseline if needed</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void updateButton_Click(object sender , EventArgs e)
        {
            foreach( DataGridViewRow row in requestDataGridView.SelectedRows )
            {
                ResultRequest result = Program.ResultList[Program.DBManager.FindRecordInList(Convert.ToInt32(row.Cells[1].Value.ToString()))];
                if( !result.ResultRecord.BaselineIsUpToDate )
                {
                    result.ResultRecord.BaselineIsUpToDate = true;
                    Program.TestManager.FetchBaseline(result.ResultRecord);
                }
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Update all baseline not up to date</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void updateAllButton_Click(object sender , EventArgs e)
        {
            UpdateAllRecord();
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Open an empty wms selection form</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void WmsSelectButton_Click(object sender , EventArgs e)
        {
            if( !Program.IsTestRunning )
            {
                try
                {
                    m_SelectWmsForm.BringToFront();
                    m_SelectWmsForm.Show();
                    m_SelectWmsForm.SetOnceRequest = true; //indicates to the form that it is a empty request
                }
                catch//if the selectbaseline form is closed
                {
                    m_SelectWmsForm = new SelectWmsBaselineForm();
                    m_SelectWmsForm.Show();
                    m_SelectWmsForm.SetOnceRequest = true;
                }
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Load an filled wms selection form to alllow edition</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void EditButton_Click(object sender , EventArgs e)
        {
            if( !Program.IsTestRunning )
            {
                try
                {
                    m_SelectWmsForm.BringToFront();
                    m_SelectWmsForm.CurrentRecord = Program.ResultList[Program.DBManager.FindRecordInList(Convert.ToInt32(requestDataGridView.SelectedRows[0].Cells[1].Value.ToString()))].ResultRecord;
                    m_SelectWmsForm.Show();
                }
                catch//if the selectbaseline form is closed
                {
                    m_SelectWmsForm = new SelectWmsBaselineForm();
                    m_SelectWmsForm.CurrentRecord = Program.ResultList[Program.DBManager.FindRecordInList(Convert.ToInt32(requestDataGridView.SelectedRows[0].Cells[1].Value.ToString()))].ResultRecord;
                    m_SelectWmsForm.Show();
                }
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Call the delete selection fonction when the delete button is clicked</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void DeleteButton_Click(object sender , EventArgs e)
        {
            DeleteSelection();
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Open the result comparison form</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void ResultComparisonButton_Click(object sender , EventArgs e)
        {
            try
            {
                m_ResultComparisonForm.BringToFront();
                m_ResultComparisonForm.Show();
            }
            catch//if the m_ResultComparisonForm does not exist
            {
                m_ResultComparisonForm = new ResultComparisonForm();
                m_ResultComparisonForm.Show();
            }
            finally
            {
                if( requestDataGridView.SelectedRows.Count == 1 )
                    m_ResultComparisonForm.IteratorList = requestDataGridView.SelectedRows[0].Index + 1;
                else
                    m_ResultComparisonForm.IteratorList = 1;
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Copy the select url to clipboard</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void CopyUrlToClipboard_Click(object sender , EventArgs e)
        {
            if( requestDataGridView.SelectedRows[0].Cells[6].Value.ToString() != "" && requestDataGridView.SelectedRows[0].Cells[6].Value != null )
                Clipboard.SetText(requestDataGridView.SelectedRows[0].Cells[6].Value.ToString());
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Execute all selected the request when the play button is clicked</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void PlayButton_Click(object sender , EventArgs e)
        {
            if( requestDataGridView.RowCount > 0 && !Program.IsTestRunning ) //if there is a request and if the test is not already running
            {
                MessageTextBox descriptionForm = new MessageTextBox("Description needed" , "Please enter a description for the test"); //ask for a user description
                DialogResult descriptionFormResult = descriptionForm.ShowDialog(this);

                if( descriptionFormResult == System.Windows.Forms.DialogResult.OK )
                {
                    Program.TestManager.Description = descriptionForm.Value;
                    progressBarTest.Value = 0;
                    progressBarTest.Maximum = GetNumberOfTickProgressBar(); // get the max value of the progress bar
                    if( progressBarTest.Maximum !=0 ) //makes sure there is a selected request
                    {
                        progressBarTest.Visible = true;
                        Program.StartTest();
                        requestDataGridView.Columns[0].ReadOnly = true;
                        StopButton.Enabled = true;
                        if( m_SelectWmsForm != null ) //hide all form to prevent the user to modify important data
                            m_SelectWmsForm.Hide();
                        if( m_SettingForm != null )
                            m_SettingForm.Hide();
                        if( m_DeleteResultForm!= null )
                            m_DeleteResultForm.Hide();
                    }
                }
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Stop the request</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void StopButton_Click(object sender , EventArgs e)
        {
            Program.IsTestRunning = false;
        }

        #endregion

        #region methods/eventsHandler/topPanelEvents

        /*------------------------------------------------------------------------------------**/
        /// <summary>Show a comparisonform already initialized on row header double-click</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void statusDropDownList_SelectedIndexChanged(object sender , EventArgs e)
        {
            FilterRecordList();
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Show a comparisonform already initialized on row header double-click</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void categoryDropDownList_SelectedIndexChanged(object sender , EventArgs e)
        {
            FilterRecordList();
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Evaluate the new prefix when the one of the combobox is changer</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void comboBoxProtocol_TextChanged(object sender , EventArgs e)
        {
            EvaluatePrefix();
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Evaluate the new prefix when the one of the combobox is changer</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void comboBoxServerName_TextChanged(object sender , EventArgs e)
        {
            EvaluatePrefix();
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Evaluate the new prefix when the one of the combobox is changer</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void comboBoxPortNumber_TextChanged(object sender , EventArgs e)
        {
            EvaluatePrefix();
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Allow the user to directly add a category in the category filter combobox</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void categoryDropDownList_KeyDown(object sender , KeyEventArgs e)
        {
            if( e.KeyCode == Keys.Enter )
            {
                if( Program.DBManager.LookForCategoryId(categoryDropDownList.Text) == 0 && categoryDropDownList.Text != "All" )
                {
                    categoryDropDownList.Items.Add(categoryDropDownList.Text);
                    Program.DBManager.InsertCategory(categoryDropDownList.Text); //insert category in database
                    categoryDropDownList.SelectedIndex = categoryDropDownList.Items.Count -1;
                    if( m_SettingForm != null )
                        m_SettingForm.LoadCategories();
                }
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Open the setting form</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void OptionsButton_Click(object sender , EventArgs e)
        {
            if( !Program.IsTestRunning )
            {
                try
                {
                    m_SettingForm.BringToFront();
                    m_SettingForm.Show();
                }
                catch//if the m_SettingForm is closed
                {
                    m_SettingForm = new SettingForm();
                    m_SettingForm.Show();
                }
            }
        }

        #endregion

        #endregion

        #region methods/gridViewOperation

        /*------------------------------------------------------------------------------------**/
        /// <summary>Add all Program.RecordList to the grid view (and clear previously added record)</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private delegate void LoadRecordInGridViewCallback();
        public void LoadRecordInGridView()
        {
            if( requestDataGridView.InvokeRequired ) //make sure no thread, other than the main one modifies the value directly(this causes an exception on all windows forms control)
            {
                LoadRecordInGridViewCallback callback = new LoadRecordInGridViewCallback(LoadRecordInGridView);
                Invoke(callback);//asynchronously call this fonction again
            }
            else
            {
                if( requestDataGridView.Rows.Count - 1 == Program.ResultList.Count )
                {
                    foreach( DataGridViewRow row in requestDataGridView.Rows )
                    {
                        if( !row.IsNewRow )
                            UpdateRecordInGridView(Program.ResultList[Program.DBManager.FindRecordInList(Convert.ToInt16(row.Cells[1].Value.ToString()))].ResultRecord , row.Index);
                    }
                }
                else
                {
                    requestDataGridView.Rows.Clear();
                    for( int i = 0 ; i < Program.ResultList.Count ; i++ )
                    {
                        AddToGridView(Program.ResultList[i].ResultRecord); //reload all records
                    }
                }
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Append a record to the grid view</summary>  
        /// <param name="record">Record to be added</param>
        /// <param name="index">Indicate where to insert the record, if nothing is specified, the record is append to the list</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void AddToGridView(Record record , int index = -1)
        {

            //insert all value in data grid view
            DataGridViewRow row = (DataGridViewRow) m_ExempleRow.Clone();
            if( record.BaselineIsUpToDate )
                row.Cells[2].Value = global::Geo_Web_Publisher_Unit_Testing_App.Properties.Resources.check;
            else
            {
                if( Properties.GeoWebPublisherUnitTestingApp.Default.AutoCheckUpdate )
                {
                    record.BaselineIsUpToDate = true;
                    Program.TestManager.FetchBaseline(record , false);
                    row.Cells[2].Value = global::Geo_Web_Publisher_Unit_Testing_App.Properties.Resources.check;
                }
                else
                    row.Cells[2].Value = global::Geo_Web_Publisher_Unit_Testing_App.Properties.Resources.Warning;
            }
            row.Cells[1].Value = record.Id;
            row.Cells[3].Value = record.Name;
            row.Cells[7].Value = Program.ResultList[Program.DBManager.FindRecordInList(record.Id)].Status;
            row.Cells[0].Value = Program.ResultList[Program.DBManager.FindRecordInList(record.Id)].ExecuteTest;
            row.Cells[4].Value = record.Description;
            string CategoryName = Program.DBManager.LookForCategoryName(record.CategoryId);

            if( CategoryName != "" )
                row.Cells[5].Value = CategoryName;
            else
                row.Cells[5].Value = "None";

            row.Cells[6].Value = record.RecordRequest.RequestString;
            //update resultComparisonForm if needed
            if( m_ResultComparisonForm != null )
                m_ResultComparisonForm.UpdateInfo();

            //add the row at the selected index, or at the end if index =-1
            if( index == -1 )
                requestDataGridView.Rows.Add(row);
            else
                requestDataGridView.Rows.Insert(index , row);
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Delete selected element in the grid view</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void DeleteSelection()
        {
            if( !Program.IsTestRunning )
            {
                //ask if the user really wants to delete the request
                DialogResult result = MessageBox.Show("Are you sure your want to delete selected request(s)?" , "Warning" , MessageBoxButtons.OKCancel , MessageBoxIcon.Exclamation);
                if( result == DialogResult.OK )
                {
                    while( requestDataGridView.SelectedRows.Count != 0 )//delete in the database and in the grid view all selected request
                    {
                        if( !requestDataGridView.SelectedRows[0].IsNewRow )
                        {
                            Program.DBManager.DeleteRecord(Program.DBManager.FindRecordInList(Convert.ToInt32(requestDataGridView.SelectedRows[0].Cells[1].Value)));
                            requestDataGridView.Rows.Remove(requestDataGridView.SelectedRows[0]);
                        }
                        else
                            requestDataGridView.SelectedRows[0].Selected = false;
                    }
                }
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Update one record in the grid view</summary>
        /// <param name="record">Record to update</param>
        /// <param name="indexInGridView">Index in gridview where the record is</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void UpdateRecordInGridView(Record record , int indexInGridView)
        {
            requestDataGridView.Rows.RemoveAt(indexInGridView);
            AddToGridView(record , indexInGridView);
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Allow the comparison form to change the selected index according to its current record</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void ChangeSelectedIndex(int index)
        {
            requestDataGridView.ClearSelection();
            foreach( DataGridViewRow row in requestDataGridView.Rows )
            {
                if( !row.IsNewRow )
                {
                    if( Convert.ToInt32(row.Cells[1].Value.ToString()) == Program.ResultList[index-1].ResultRecord.Id )
                        row.Selected = true;
                    else
                        row.Selected = false;
                }
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Allow an external form to end the edition of the request datagridview</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        internal void EndEdit()
        {
            requestDataGridView.EndEdit();
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Update all baseline not up to date</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void UpdateAllRecord()
        {
            foreach( DataGridViewRow row in requestDataGridView.Rows )
            {
                if( !row.IsNewRow )
                {
                    ResultRequest result = Program.ResultList[Program.DBManager.FindRecordInList(Convert.ToInt32(row.Cells[1].Value.ToString()))];
                    if( !result.ResultRecord.BaselineIsUpToDate )
                    {
                        result.ResultRecord.BaselineIsUpToDate = true;
                        Program.TestManager.FetchBaseline(result.ResultRecord);

                    }
                }
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Empty the datagridview</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void EmptyGridView()
        {
            requestDataGridView.Rows.Clear();
        }

        #endregion

        #region methods/others

        /*------------------------------------------------------------------------------------**/
        /// <summary>Evaluate the prefix in a single string</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void EvaluatePrefix()
        {
            Properties.GeoWebPublisherUnitTestingApp.Default.UrlPrefix = comboBoxProtocol.Text+comboBoxServerName.Text+":"+comboBoxPortNumber.Text+"/";
            Properties.GeoWebPublisherUnitTestingApp.Default.Save();
        }
       
        /*------------------------------------------------------------------------------------**/
        /// <summary>Advances progress bar of 1 part</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private delegate void AdvanceProgressBarCallBack();
        public void AdvanceProgressBar()
        {
            if( toolStrip1.InvokeRequired )//makes sure the control is thread safe
            {
                AdvanceProgressBarCallBack callback = new AdvanceProgressBarCallBack(AdvanceProgressBar);
                Invoke(callback);
            }
            else
            {
                progressBarTest.Value++;
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Get the number of events in the current test to make increase the progress bar correctly</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private int GetNumberOfTickProgressBar()
        {
            int tick = 0;
            foreach( DataGridViewRow row in requestDataGridView.Rows ) // check wich row are selected 
            {
                if( !row.IsNewRow )
                {
                    DataGridViewCheckBoxCell executeTestCheckBox = (DataGridViewCheckBoxCell) row.Cells[0];
                    if( Convert.ToBoolean(executeTestCheckBox.Value) && Program.ResultList[Program.DBManager.FindRecordInList(Convert.ToInt32(row.Cells[1].Value.ToString()))].ResultRecord.BaselineIsUpToDate )
                        tick++;
                }
            }

            return 2*tick;
        }
            
        /*------------------------------------------------------------------------------------**/
        /// <summary>Allow the test manager to indicate the end of the test to reenabled disabled capabilities</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private delegate void TestFinishedCallBack();
        public void TestFinished()
        {
            if( this.InvokeRequired ) // prevent the windows form controls from sending an exception because it has been modified by another thread
            {
                TestFinishedCallBack callback = new TestFinishedCallBack(TestFinished);
                Invoke(callback);
            }
            else
            {
                requestDataGridView.Columns[0].ReadOnly = false;
                progressBarTest.Visible = false;
                InitializePrefix(); //add the most recent prefix in the database
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Show a comparisonform already initialized on row header double-click</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void FilterRecordList()
        {
            if( !Program.IsTestRunning )
            {
                string category = categoryDropDownList.Text;
                string stringStatus = statusDropDownList.Text.ToUpper();

                foreach( DataGridViewRow row in requestDataGridView.Rows )
                {
                    if( !row.IsNewRow )
                    {
                        row.Cells[0].Value = ( ( row.Cells[5].Value.ToString() == category || category == "All" ) && ( ( row.Cells[7].Value.ToString() == stringStatus || stringStatus == "ALL" ) ) );
                        requestDataGridView.UpdateCellValue(0 , row.Index);
                        ResultRequest resultToChange = Program.ResultList[row.Index];
                        resultToChange.ExecuteTest = ( ( row.Cells[5].Value.ToString() == category || category == "All" ) && ( ( row.Cells[7].Value.ToString() == stringStatus || stringStatus == "ALL" ) ) );
                        Program.ResultList[row.Index] = resultToChange;
                    }
                }
                requestDataGridView.RefreshEdit();
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Change the statusbar message</summary>
        /// <param name="message">Message to put in the status bar</param>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private delegate void ChangeStatusBarMessageCallBack(string message);
        public void ChangeStatusBarMessage(string message)
        {
            if( toolStrip1.InvokeRequired )//makes sur the windows control does not throw an exception
            {
                ChangeStatusBarMessageCallBack callback = new ChangeStatusBarMessageCallBack(ChangeStatusBarMessage);
                Invoke(callback , message);
            }
            else
                toolStripStatusLabel1.Text = message;
        }

        #endregion

        #endregion
    }
}