/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/GWP_UnitTestingApp/Geo Web Publisher Unit Testing App/selectWmsBaselineForm.cs $
|    $RCSfile: SelectWmsBaselineForm.cs, $
|   $Revision: 1 $
|       $Date: 2013/05/27 $
|     $Author: Julien Rossignol $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System;
using System.Timers;
using System.Windows.Forms;

namespace Geo_Web_Publisher_Unit_Testing_App
{
    /// <summary>Manage the baseline form with the embedded web browser</summary>
    /// <author>Julien Rossignol</author>
    public partial class SelectWmsBaselineForm : Form
    {

        #region fields

        private Record m_CurrentRecord; //currently displayed record
        private bool m_IsReady; //indicates the browser is ready
        private System.Timers.Timer m_CheckRequestTimer; //timers indicating when to verify if browser changed
        private bool m_SetOnceRequest; //indicates if the request need to be set in the browser
        private Request m_CurrentRequest; //current displayed request
        private bool m_RequestInfoSetOnce;

        #endregion

        #region parameters

        public bool SetOnceRequest
        {
            get { return m_SetOnceRequest; }
            set { m_SetOnceRequest = value; }
        }

        //intiliaze record info if the edit button was used
        public Record CurrentRecord
        {
            get { return m_CurrentRecord; }
            set
            {
                m_CurrentRecord = value;
                nameTextBox.Text = value.Name;
                m_CurrentRequest = RequestAnalyser.AnalyseRequest(m_CurrentRecord.RecordRequest.RequestString);
                descriptionTextBox.Text = value.Description;
                urlTextBox.Text = Properties.GeoWebPublisherUnitTestingApp.Default.UrlPrefix + value.RecordRequest.RequestString;
                m_SetOnceRequest = false;
                m_RequestInfoSetOnce = false;
                CategoryDropDownList.Text = Program.DBManager.LookForCategoryName(value.CategoryId);
                try
                {
                    UpdateViewer();
                }
                catch
                {
                }
            }
        }

        #endregion

        #region constructors

        /*------------------------------------------------------------------------------------**/
        /// <summary>Initialize form components and timer for request on web browser</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public SelectWmsBaselineForm()
        {
            m_IsReady = false;
            m_SetOnceRequest = false;
            InitializeComponent();
            try
            {
                this.webBrowser1.Url = new System.Uri(this.StripPortNumber(Properties.GeoWebPublisherUnitTestingApp.Default.UrlPrefix)+"/Examples/WmsViewerExample/WmsViewerPage.aspx" , System.UriKind.Absolute); //try to set the browser url
            }
            catch
            {
                System.Windows.Forms.MessageBox.Show("Invalid URL. Please verify protocol, server name and port number" , "Invalid Url" , System.Windows.Forms.MessageBoxButtons.OK , System.Windows.Forms.MessageBoxIcon.Error);
            }
            m_CurrentRecord = new Record();
            m_CheckRequestTimer = new System.Timers.Timer(100);//prepare the timer to check browser info 10 times a second
            m_CheckRequestTimer.Elapsed += new ElapsedEventHandler(OnCheckRequesTimerElapsed);
            LoadCategories();
            CategoryDropDownList.Text = "None";
        }

        #endregion

        #region methods

        /*------------------------------------------------------------------------------------**/
        /// <summary>Each 100 miliseconds, checks for an update of the wmsviewer</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void OnCheckRequesTimerElapsed(object sender , ElapsedEventArgs e)
        {
            CheckUrlTextBoxCallback callback = new CheckUrlTextBoxCallback(CheckUrlTextBox);
            Invoke(callback);
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Check if there is a change in the wmsviewer, truncate it, an set it to urlTextBox if needed</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private delegate void CheckUrlTextBoxCallback();
        private void CheckUrlTextBox()
        {
            if( m_IsReady && !urlTextBox.Focused ) //does not check if urltextbox is being changed or if rowser is not ready yet
            {
                object browserInfo = webBrowser1.Document.InvokeScript("eval" , new object[] { "document.viewerForm.Log.value" }); //gets the text in the request text box of the browser
                if( browserInfo != null )
                {
                    string requestFromBrowser = browserInfo.ToString();
                    if( requestFromBrowser != "" )
                    {
                        int spacePosition = requestFromBrowser.LastIndexOf(" - "); // truncate the reload stamp and datime of the text box
                        requestFromBrowser = requestFromBrowser.Remove(0 , spacePosition + 3);
                        int reloadStampPosition = requestFromBrowser.LastIndexOf('&');
                        requestFromBrowser = requestFromBrowser.Remove(reloadStampPosition);

                        if( urlTextBox.Text != requestFromBrowser ) // if textbox and browser info are not the same
                        {
                            if( m_CurrentRecord.RecordRequest.RequestString != "" && !m_SetOnceRequest && m_CurrentRequest.RequestString != null ) //if info were not send yet yo the browser
                            {
                                webBrowser1.Document.InvokeScript("SetMap" , new object[] { m_CurrentRequest.Map }); //change the map combobox to the right one
                                webBrowser1.Document.InvokeScript("SetSRS" , new object[] { m_CurrentRequest.SRS }); //change the srs combobox to the right one
                                webBrowser1.Document.InvokeScript("FitViewWithNewExtent" , new object[] { m_CurrentRequest.BBox.XMin , m_CurrentRequest.BBox.YMin , m_CurrentRequest.BBox.XMax , m_CurrentRequest.BBox.YMax }); //fit the navigator to the current extent
                                webBrowser1.Document.InvokeScript("ClearLayer"); // clear the selected layer
                                foreach( string layer in m_CurrentRequest.LayerList )
                                {
                                    webBrowser1.Document.InvokeScript("CheckLayer" , new object[] { layer }); //select the layer
                                }
                                webBrowser1.Document.InvokeScript("UpdateLayers"); // update the layer list
                                webBrowser1.Document.InvokeScript("UpdateViewer"); // update the image
                                m_SetOnceRequest = true;
                            }
                            else
                            {
                                if( m_CurrentRecord.RecordRequest.Type == RequestType.GETFEATUREINFO && !m_RequestInfoSetOnce )
                                {
                                    m_RequestInfoSetOnce = true;
                                }
                                else
                                {
                                    urlTextBox.Text = requestFromBrowser;
                                    m_CurrentRequest = RequestAnalyser.AnalyseRequest(StripPrefix(urlTextBox.Text));
                                }
                                webBrowser1.Document.InvokeScript("ClearLog"); // empty the request text box of the browser
                            }
                        }
                    }
                }
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Strip the prefix from the url</summary>
        /// <param name="url">String to strip from</param>
        /// <returns>Stripped string</returns>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private string StripPrefix(string url)
        {
            string StrippedUrl = url.Replace(StripPortNumber(Properties.GeoWebPublisherUnitTestingApp.Default.UrlPrefix) , "");
            return StrippedUrl.Substring(StrippedUrl.IndexOf('/')+1);
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Load new categories into the drop down list, also change m_CurrentRecord category if needed</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void LoadCategories()
        {
            CategoryDropDownList.Items.Clear();
            foreach( Category category in Program.CategoryList )
            {
                CategoryDropDownList.Items.Add(category.Name);
            }
            CategoryDropDownList.Items.Add("None");

            if( m_CurrentRecord.Id != 0 )
                CategoryDropDownList.Text = Program.DBManager.LookForCategoryName(m_CurrentRecord.CategoryId);
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Save the record and reinitialise the form</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void SaveButton_Click(object sender , EventArgs e)
        {
            if( nameTextBox.Text == "" )
                MessageBox.Show("Request cannot be save : you must enter a name" , "Cannot save" , MessageBoxButtons.OK , MessageBoxIcon.Error);
            else
            {
                if( urlTextBox.Text == "" )
                    MessageBox.Show("Request cannot be save : you must enter an url" , "Cannot save" , MessageBoxButtons.OK , MessageBoxIcon.Error);
                else
                {
                    //change current record to fit entered value 
                    m_CurrentRecord.Name = nameTextBox.Text;
                    m_CurrentRecord.CategoryId = Program.DBManager.LookForCategoryId(CategoryDropDownList.Text);
                    if( Properties.GeoWebPublisherUnitTestingApp.Default.UrlPrefix + m_CurrentRecord.RecordRequest.RequestString != urlTextBox.Text )
                    {
                        m_CurrentRecord.BaselineIsUpToDate = false;
                        m_CurrentRecord.MRBaselineId = 0;
                    }
                    m_CurrentRecord.RecordRequest = m_CurrentRequest;
                    m_CurrentRecord.Description = descriptionTextBox.Text;
                    if( m_CurrentRecord.Id == 0 )//add new record in the database
                    {
                        Program.DBManager.InsertRecord(m_CurrentRecord);
                        MessageBox.Show("Request successfully saved" , "Success" , MessageBoxButtons.OK , MessageBoxIcon.None);
                    }
                    else //update record in the database
                    {
                        int positionInList = Program.DBManager.FindRecordInList(m_CurrentRecord.Id);
                        Program.DBManager.UpdateRecord(m_CurrentRecord , positionInList , true);
                        this.Dispose();
                    }
                }
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Close the form</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void CancelButton_Click(object sender , EventArgs e)
        {
            this.Hide();
            CurrentRecord = new Record();
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Prepares injected javascript function to be used later.</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void PrepareNewJavascriptFonction()
        {
            // Easier to read and commented version of those function are available in the programming guide
            string FitViewWithNewExtentFunction = "function FitViewWithNewExtent(xmin, ymin, xmax, ymax){var newExtent = new CExtent(xmin, ymin, xmax, ymax);objNavigation.SetExtent(newExtent);	objNavigation.Fit(); UpdateViewer(); writeLogToScreen(xmin + ymin + xmax + ymax);}";
            webBrowser1.Document.InvokeScript("execScript" , new object[] { FitViewWithNewExtentFunction , "JavaScript" });
            string CheckLayerFunction = "function CheckLayer(layerName)	{var table4 = document.getElementById(\"TreeView\");if(table4){var checkboxes = table4.getElementsByTagName(\"input\");var labels = table4.getElementsByTagName(\"label\");if ( checkboxes && labels ){for(var i = 0; i < labels.length ; i++){if(labels[i].innerText == layerName){checkboxes[i].checked = true;break;}}}}}";
            webBrowser1.Document.InvokeScript("execScript" , new object[] { CheckLayerFunction , "JavaScript" });
            string ClearLayerFunction = "function ClearLayer(){var table4 = document.getElementById(\"TreeView\");if(table4){var checkboxes = table4.getElementsByTagName(\"input\");if ( checkboxes ){for(var i = 0; i < checkboxes.length ; i++){checkboxes[i].checked = false;}}}}";
            webBrowser1.Document.InvokeScript("execScript" , new object[] { ClearLayerFunction , "JavaScript" });
            string SetMapFunction = "function SetMap(GUID){var MapDropDown = document.getElementsByName(\"DropDownListMapID\")[0];if(MapDropDown){for(var i = 0; i < MapDropDown.options.length ; i++){if(MapDropDown.options[i].value == GUID){if(i != MapDropDown.selectedIndex){MapDropDown.selectedIndex = i;__doPostBack(\"DropDownListMapID\",\"\");}}}}}";
            webBrowser1.Document.InvokeScript("execScript" , new object[] { SetMapFunction , "JavaScript" });
            string SetSRSFunction = "function SetSRS(newSrs){var SRSDropDownList = document.getElementsByName(\"DropDownListMapSRS\")[0];if(SRSDropDownList){for(var i = 0;i < SRSDropDownList.options.length ; i++){if(SRSDropDownList.options[i].value == newSrs){SRSDropDownList.selectedIndex = i;UpdateViewer();break;}}}}";
            webBrowser1.Document.InvokeScript("execScript" , new object[] { SetSRSFunction , "JavaScript" });

        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>when webbrowser is ready, prepare javascript function</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void webBrowser1_DocumentCompleted(object sender , WebBrowserDocumentCompletedEventArgs e)
        {
            m_IsReady = true;
            PrepareNewJavascriptFonction();
            if( !m_SetOnceRequest )
                UpdateViewer();
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Start and stop timer if the form is shown/hidden</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void SelectWmsBaselineForm_VisibleChanged(object sender , EventArgs e)
        {
            if( this.Visible == true )
                m_CheckRequestTimer.Start();
            else
                m_CheckRequestTimer.Stop();
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>When enter is pressed in url text box, update the viewer</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void urlTextBox_KeyDown(object sender , KeyEventArgs e)
        {
            if( e.KeyCode == Keys.Enter )
            {
                UpdateViewer();
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Strip from the prefix the port number</summary>
        /// <param name="prefix">Prefix with prt number</param>
        /// <returns>String without port number</returns>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private string StripPortNumber(string prefix)
        {
            return prefix.Remove(prefix.LastIndexOf(":"));
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>When enter is pressed in url text box, update the viewer</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void UpdateViewer()
        {
            Request request = RequestAnalyser.AnalyseRequest(StripPrefix(urlTextBox.Text)); // analyse the new url 
            webBrowser1.Document.InvokeScript("SetMap" , new object[] { m_CurrentRequest.Map }); // set map combobox
            webBrowser1.Document.InvokeScript("SetSRS" , new object[] { m_CurrentRequest.SRS }); //set srs combobox
            webBrowser1.Document.InvokeScript("FitViewWithNewExtent" , new object[] { request.BBox.XMin , request.BBox.YMin , request.BBox.XMax , request.BBox.YMax }); //set navigator extend 
            webBrowser1.Document.InvokeScript("ClearLayer"); // empty selected layers list
            foreach( string layer in request.LayerList ) //select correct layers
            {
                webBrowser1.Document.InvokeScript("CheckLayer" , new object[] { layer });
            }
            webBrowser1.Document.InvokeScript("UpdateLayers"); //update layers list
            webBrowser1.Document.InvokeScript("UpdateViewer"); //update viewer
            m_CurrentRequest = request;

        }

        #endregion
    }
}
