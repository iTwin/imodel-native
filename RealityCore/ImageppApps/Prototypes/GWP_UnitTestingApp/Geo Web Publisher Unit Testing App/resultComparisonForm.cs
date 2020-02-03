/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/GWP_UnitTestingApp/Geo Web Publisher Unit Testing App/resultComparisonForm.cs $
|    $RCSfile: ResultComparisonForm.cs, $
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
    /// <summary>Manage the comparison form</summary>
    /// <author>Julien Rossignol</author>
    public partial class ResultComparisonForm : Form
    {

        #region fields

        private int m_IteratorList; //indicates where we are currently in the list
        private bool m_IsMouseDown; //used for picture box panning, tells if the left mouse button is pressed 
        private Point m_MouseStartingPoint; //indicates where the mouse was first pressed down 
        private Point m_PointToPaint; //indicate where the new raster should be paint
        private Bitmap m_ResultImage; //contains the image to show in the right picture box
        private Bitmap m_BaseResultImage; //contains the base output raster 
        private Bitmap m_BaselineImage; //containt the current baseline
        private double m_ZoomFactor; //tells how much the image is zoomed
        private bool m_SkipSuccess; //indicates if the iterator must skip successful request 
        private List<ResultRequest> m_OldResult; //contains all old results for the current record
        private Bitmap m_DifferenceBitmap; //contains the current difference bitmap
        private String m_ErrorMessage; //containt the output error message
        private bool m_IsError; //indicates that the output is an text file
        private bool m_BaselineIsXML; //indicate the baseline is an text file
        private double m_ZoomFactorMin; // limits the minimum value of the zoomFactor
        private bool m_ImageErrorBaseline; //indicates there is an error in baseline loading
        private bool m_ImageErrorResult; //indicates there is an error in result loading
        private bool m_OutputIsnothing; //indicates if the output image is blank

        #endregion

        #region parameters

        public int IteratorList
        {
            get { return m_IteratorList; }
            set
            {
                m_IteratorList = value;
                UpdateInfo();
            }
        }

        #endregion

        #region constructors

        /*------------------------------------------------------------------------------------**/
        /// <summary>Initialize form components</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public ResultComparisonForm()
        {
            InitializeComponent();
            m_IteratorList = 1;
            m_IsMouseDown = false;
            m_MouseStartingPoint = Point.Empty;
            m_PointToPaint = Point.Empty;
            m_ZoomFactor = 1;
            m_SkipSuccess = false;
            m_ErrorMessage = "";
            m_ImageErrorBaseline = false;
            m_ImageErrorResult = false;
        }

        #endregion

        #region methods

        #region methods/informationDisplay

        /*------------------------------------------------------------------------------------**/
        /// <summary>Initialize record labels and test labels</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void InitializeRecordInfo()
        {
            this.UrlTextBox.Text = Program.ResultList[m_IteratorList - 1].ResultRecord.RecordRequest.RequestString;
            this.nameLabel.Text = Program.ResultList[m_IteratorList - 1].ResultRecord.Name;
            this.descriptionLabel.Text = Program.ResultList[m_IteratorList - 1].ResultRecord.Description;
            this.numberLabel.Text = m_IteratorList + "/" + Program.ResultList.Count;
            this.recordGroupdBox.Text ="Record informations       Id = " + Program.ResultList[m_IteratorList - 1].ResultRecord.Id.ToString();
            m_BaselineImage = RasterManipulation.ConvertByteArrayToBitmap(Program.ResultList[m_IteratorList - 1].ResultRecord.BaselineBitmap);
            m_PointToPaint = Point.Empty;

            m_BaselineIsXML = Program.ResultList[m_IteratorList - 1].ResultRecord.BaselineIsXML;
            statusLabel.Text = Program.ResultList[m_IteratorList - 1].Status.ToString();
            testDescriptionLabel.Text = Program.ResultList[m_IteratorList - 1].LinkedTest.Description;
            testPrefixLabel.Text = Program.ResultList[m_IteratorList - 1].LinkedTest.Prefix;
            failedRequestCount.Text = Program.DBManager.GetNumberOfFailedTest(Program.ResultList[m_IteratorList - 1].LinkedTest.Id) + "/" + Program.DBManager.GetNumberOfRecordTest(Program.ResultList[m_IteratorList - 1].LinkedTest.Id);
            DateLabel.Text = GetDate(Program.ResultList[m_IteratorList - 1].LinkedTest.DateTime + (int)System.TimeZoneInfo.Local.BaseUtcOffset.TotalSeconds);
            if( DateLabel.Text.StartsWith("1") )
                DateLabel.Text = "----/--/--";
            TimeLabel.Text = GetTime(Program.ResultList[m_IteratorList - 1].LinkedTest.DateTime + (int) System.TimeZoneInfo.Local.BaseUtcOffset.TotalSeconds);

        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Get old results for the current record</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void InitializeOldResult()
        {
            m_OldResult = Program.DBManager.GetOldResult(Convert.ToInt32(Program.ResultList[m_IteratorList - 1].ResultRecord.Id.ToString()));
            if( Program.ResultList[m_IteratorList - 1].LinkedTest.DateTime != 0 && Program.ResultList[m_IteratorList - 1].Status == RequestStatus.SUCCESS )//add the most recent result if it is a success
                m_OldResult.Insert(0 , Program.ResultList[m_IteratorList - 1]);
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Initiliaze baseline data</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void InitiliazeBaseline()
        {
            if( m_BaselineIsXML ) //enable text box and disable picture box
            {
                baselinePictureBox.Visible = false;
                baselineTextBox.Visible = true;
                baselineTextBox.Text = System.Text.Encoding.GetEncoding(28591).GetString(Program.ResultList[m_IteratorList - 1].ResultRecord.BaselineBitmap);
            }
            else if( Program.ResultList[m_IteratorList - 1].ResultRecord.BaselineIsUpToDate )//enable picture box and disable text box
            {
                baselinePictureBox.Visible = true;
                baselineTextBox.Visible = false;
                m_ZoomFactorMin = baselinePictureBox.Width/m_BaselineImage.Width; //initiliaze m_ZoomFactorMin to fit the width and height of the raster
                if( m_ZoomFactorMin < baselinePictureBox.Height/m_BaselineImage.Height )
                    m_ZoomFactorMin = baselinePictureBox.Height/m_BaselineImage.Height;

                m_ZoomFactor = m_ZoomFactorMin;
            }
            else //disable both, only happens if baseline is not up to date
            {
                baselinePictureBox.Visible = false;
                baselineTextBox.Visible = false;
            }
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Initiliaze right picturebox/text box to fit the baseResult output </summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void InitializeOutput()
        {
            if( Program.ResultList[m_IteratorList - 1].Status == RequestStatus.ERROR || RasterManipulation.ConvertByteArrayToBitmap(Program.ResultList[m_IteratorList - 1].ResultBitmap) == null && Program.ResultList[m_IteratorList - 1].Status == RequestStatus.SUCCESS )//if the output is a text file
            {
                m_OutputIsnothing = false;
                m_IsError = true;
                m_ErrorMessage = System.Text.Encoding.GetEncoding(28591).GetString(Program.ResultList[m_IteratorList - 1].ResultBitmap);
                resultPictureBox.Visible = false;
                outputTextBox.Visible = true;
                outputTextBox.Text = m_ErrorMessage;
                pixelCountLabel.Text = "0 (0%)";
                largeDiffPixelCount.Text = "0 (0%)";
            }
            else if( RasterManipulation.ConvertByteArrayToBitmap(Program.ResultList[m_IteratorList - 1].ResultBitmap) != null )//if the output is a raster
            {
                m_OutputIsnothing = false;
                m_IsError = false;
                m_BaseResultImage = RasterManipulation.ConvertByteArrayToBitmap(Program.ResultList[m_IteratorList - 1].ResultBitmap);
                m_DifferenceBitmap = RasterManipulation.GetDifferenceBitmap(Program.ResultList[m_IteratorList - 1].ResultBitmap , Program.ResultList[m_IteratorList - 1].ResultRecord.BaselineBitmap);
                pixelComparisonCheckBox.Enabled = true;
                resultPictureBox.Visible = true;
                outputTextBox.Visible = false;
                pixelCountLabel.Text = Program.ResultList[m_IteratorList - 1].InvalidPixels + " (" + ( (double) Program.ResultList[m_IteratorList - 1].InvalidPixels / m_BaseResultImage.Height*m_BaseResultImage.Width ) + "%)";
                largeDiffPixelCount.Text = Program.ResultList[m_IteratorList - 1].BigDifferencePixels + " (" + ( (double) Program.ResultList[m_IteratorList - 1].BigDifferencePixels / m_BaseResultImage.Height*m_BaseResultImage.Width ) + "%)";
                ResultPictureChange();
            }
            else if( m_OldResult.Count == 0 ) //if there is no output nor oldresult
            {
                m_OutputIsnothing = true;
                m_BaseResultImage = new Bitmap(600 , 400);
                m_IsError = false;
                pixelComparisonCheckBox.Checked = false;
                pixelComparisonCheckBox.Enabled = false;
                resultPictureBox.Visible = true;
                outputTextBox.Visible = false;
                pixelCountLabel.Text = "0 (0%)";
                largeDiffPixelCount.Text = "0 (0%)";
                ResultPictureChange();
            }
            else //if there is no output but at least one old result
            {
                m_OutputIsnothing = false;
                m_IsError = false;
                pixelComparisonCheckBox.Enabled = true;
                resultPictureBox.Visible = true;
                outputTextBox.Visible = false;
                pixelCountLabel.Text = "0 (0%)";
                largeDiffPixelCount.Text = "0 (0%)";
            }
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Update all info in the data form to fit a change in the printed record</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void UpdateInfo()
        {
            if( m_IteratorList > 0 && m_IteratorList <= Program.ResultList.Count )
            {
                InitializeRecordInfo();
                InitializeOldResult();
                InitiliazeBaseline();
                InitializeOutput();
                UpdateOldResult();
            }
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Update the old results drop down list</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void UpdateOldResult()
        {
            OldResultComboBox.Items.Clear();
            foreach( ResultRequest result in m_OldResult )
            {
                OldResultComboBox.Items.Add(GetDate(result.LinkedTest.DateTime) + " " + GetTime(result.LinkedTest.DateTime));
            }
            if( OldResultComboBox.Items.Count != 0 )
                OldResultComboBox.SelectedIndex = 0;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Change the the output and test information when a old result is selected </summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void OldResutComboBox_SelectedIndexChanged(object sender , EventArgs e)
        {
            //initliaze labels
            statusLabel.Text = m_OldResult[OldResultComboBox.SelectedIndex].Status.ToString();
            testDescriptionLabel.Text = m_OldResult[OldResultComboBox.SelectedIndex].LinkedTest.Description;
            DateLabel.Text = GetDate(m_OldResult[OldResultComboBox.SelectedIndex].LinkedTest.DateTime);
            TimeLabel.Text = GetTime(m_OldResult[OldResultComboBox.SelectedIndex].LinkedTest.DateTime);
            testPrefixLabel.Text = ( m_OldResult[OldResultComboBox.SelectedIndex].LinkedTest.Prefix );
            failedRequestCount.Text = Program.DBManager.GetNumberOfFailedTest(m_OldResult[OldResultComboBox.SelectedIndex].LinkedTest.Id) + "/" + Program.DBManager.GetNumberOfRecordTest(m_OldResult[OldResultComboBox.SelectedIndex].LinkedTest.Id);

            //initiliaze result image
            byte[] resultByte;
            if( m_OldResult[OldResultComboBox.SelectedIndex].ResultBitmap == null )
                resultByte = Program.DBManager.GetFailureBitmap(m_OldResult[OldResultComboBox.SelectedIndex].FailureRasterId);
            else
                resultByte = m_OldResult[OldResultComboBox.SelectedIndex].ResultBitmap;

            if( resultByte != null )
            {
                if( RasterManipulation.ConvertByteArrayToBitmap(resultByte) == null )//if output is text file
                {
                    m_IsError = true;
                    m_ErrorMessage = System.Text.Encoding.GetEncoding(28591).GetString(resultByte);
                    resultPictureBox.Visible = false;
                    outputTextBox.Visible = true;
                    outputTextBox.Text = m_ErrorMessage;
                    pixelComparisonCheckBox.Checked = false;
                    pixelComparisonCheckBox.Enabled = false;
                    pixelCountLabel.Text = "0 (0%)";
                    largeDiffPixelCount.Text = "0 (0%)";
                }
                else //if it is a image
                {
                    m_OutputIsnothing = false;
                    m_BaseResultImage = RasterManipulation.ConvertByteArrayToBitmap(resultByte);
                    if( !m_BaselineIsXML )
                        m_DifferenceBitmap = RasterManipulation.GetDifferenceBitmap(resultByte , Program.ResultList[m_IteratorList - 1].ResultRecord.BaselineBitmap);
                    else
                        m_DifferenceBitmap = new Bitmap(600 , 400);
                    resultPictureBox.Visible = true;
                    outputTextBox.Visible = false;
                    pixelComparisonCheckBox.Enabled = true;
                    pixelCountLabel.Text = m_OldResult[OldResultComboBox.SelectedIndex].InvalidPixels + "(" + decimal.Round(( (decimal) m_OldResult[OldResultComboBox.SelectedIndex].InvalidPixels / ( m_BaseResultImage.Height*m_BaseResultImage.Width )*100 ) , 2 , MidpointRounding.AwayFromZero) + "%)";
                    largeDiffPixelCount.Text = m_OldResult[OldResultComboBox.SelectedIndex].BigDifferencePixels + "(" + decimal.Round(( (decimal) m_OldResult[OldResultComboBox.SelectedIndex].BigDifferencePixels / ( m_BaseResultImage.Height*m_BaseResultImage.Width )*100 ) , 2 , MidpointRounding.AwayFromZero) + "%)";
                    ResultPictureChange();
                }
            }
            this.resultPictureBox.Focus();
        }

        #endregion

        #region methods/iterator

        /*------------------------------------------------------------------------------------**/
        /// <summary>Show the first item of the list</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void FirstButton_Click(object sender , EventArgs e)
        {
            int lastValue = m_IteratorList;
            m_IteratorList = 1;
            if( m_SkipSuccess )
            {
                while( Program.ResultList[m_IteratorList-1].Status == RequestStatus.SUCCESS ) //go back into the list if there is a success
                {
                    if( m_IteratorList == Program.ResultList.Count ) //if the last element is reached and was a success
                    {
                        m_IteratorList = lastValue;
                        return;
                    }
                    m_IteratorList++;

                }

            }
            Program.MainForm.ChangeSelectedIndex(m_IteratorList); //update the main form datagridview to select the current element
            UpdateInfo();
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Show the previous item in the list</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void PreviousButton_Click(object sender , EventArgs e)
        {
            int lastValue = m_IteratorList;
            if( m_IteratorList != 1 )
            {
                m_IteratorList--;
                if( m_SkipSuccess )
                {
                    while( Program.ResultList[m_IteratorList - 1].Status == RequestStatus.SUCCESS ) //go back into the list if there is a success
                    {
                        if( m_IteratorList == 1 )//if the last element is reached and was a success
                        {
                            m_IteratorList = lastValue;
                            return;
                        }
                        m_IteratorList--;
                    }
                }
                Program.MainForm.ChangeSelectedIndex(m_IteratorList);//update the main form datagridview to select the current element
                UpdateInfo();
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Show the next item in the list</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void NextButton_Click(object sender , EventArgs e)
        {
            int lastValue = m_IteratorList;
            if( m_IteratorList != Program.ResultList.Count )
            {
                m_IteratorList++;
                if( m_SkipSuccess )
                {
                    while( Program.ResultList[m_IteratorList - 1].Status == RequestStatus.SUCCESS ) //advance into the list if next result is a success
                    {
                        if( m_IteratorList == Program.ResultList.Count ) //if the last result has been reached
                        {
                            m_IteratorList = lastValue;
                            return;
                        }
                        m_IteratorList++;
                    }
                }
                Program.MainForm.ChangeSelectedIndex(m_IteratorList);
                UpdateInfo();
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Show the last item in the list</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void LastButton_Click(object sender , EventArgs e)
        {
            int lastValue = m_IteratorList;
            m_IteratorList = Program.ResultList.Count;
            if( m_SkipSuccess )
            {
                while( Program.ResultList[m_IteratorList - 1].Status == RequestStatus.SUCCESS ) //rewind into the list if last element is a success
                {
                    if( m_IteratorList == 1 ) // if first element is reached
                    {
                        m_IteratorList = lastValue;
                        return;
                    }
                    m_IteratorList--;
                }

            }
            Program.MainForm.ChangeSelectedIndex(m_IteratorList);
            UpdateInfo();
        }

        #endregion
        
        #region methods/bitmapManipulation


        /*------------------------------------------------------------------------------------**/
        /// <summary>Change the output image when needed</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void ResultPictureChange()
        {
            if( Program.ColorSettingsHasChanged ) //if color settings were change, recalculate difference bitmap
                m_DifferenceBitmap = RasterManipulation.GetDifferenceBitmap(RasterManipulation.ConvertImageToByteArray(m_BaseResultImage) , RasterManipulation.ConvertImageToByteArray(m_BaselineImage));

            if( ResultCheckBox.Checked && pixelComparisonCheckBox.Checked )
                m_ResultImage = new Bitmap(PictureWithTransparency()); //both difference and output image printed
            else if( ResultCheckBox.Checked )
                m_ResultImage = new Bitmap(m_BaseResultImage); //only baseline image printed
            else if( pixelComparisonCheckBox.Checked ) //only pixel comparison image printed
                m_ResultImage = new Bitmap(m_DifferenceBitmap);
            else //make sure there is always an image
                ResultCheckBox.Checked = true;

            Program.ColorSettingsHasChanged = false;
            resultPictureBox.Invalidate();//update both picture box image to fit the changes
            baselinePictureBox.Invalidate();
            baselinePictureBox.Update();
            resultPictureBox.Update();
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Create a bitmap containing both the difference bitmap and the result bitmap</summary>
        /// <returns>Bitmap with both bitmap overlapping</returns>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private Bitmap PictureWithTransparency()
        {
            Bitmap resultImage = new Bitmap(m_BaseResultImage); //creates copie of both bitmap to avoid modifying them directly 
            Bitmap differenceImage = new Bitmap(m_DifferenceBitmap);
            differenceImage.MakeTransparent(Properties.GeoWebPublisherUnitTestingApp.Default.BackgroundColor); //makes the difference bitmpap background transparent
            Graphics picture = Graphics.FromImage(resultImage);
            picture.CompositingMode = System.Drawing.Drawing2D.CompositingMode.SourceOver;
            picture.DrawImage(differenceImage , new Point(0 , 0)); //blit the difference bitmap on top of the other
            return resultImage;
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Save the point where the left mouse button was pressed</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void PictureBox_MouseDown(object sender , MouseEventArgs e)
        {
            m_IsMouseDown = true;
            m_MouseStartingPoint = new Point(e.Location.X - m_PointToPaint.X , e.Location.Y - m_PointToPaint.Y);
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Indicates that the mouse button is not pressed anymore</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void PictureBox_MouseUp(object sender , MouseEventArgs e)
        {
            m_IsMouseDown = false;
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Calculate the new starting point of both image when one of the images are moved</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void PictureBox_MouseMove(object sender , MouseEventArgs e)
        {
            if( m_IsMouseDown && !m_BaselineIsXML && !m_ImageErrorBaseline && !m_ImageErrorResult ) //don't do anything if mouse is not pressed or if baseline is a text file
            {
                m_PointToPaint = new Point(e.Location.X - m_MouseStartingPoint.X , e.Location.Y - m_MouseStartingPoint.Y); //calculate new point to paint
                //next four if are used to make sure there is no blanck spot in the output image
                if( m_PointToPaint.X > 0 )
                    m_PointToPaint.X = 0;
                if( m_PointToPaint.Y > 0 )
                    m_PointToPaint.Y = 0;
                if( m_PointToPaint.X + m_ZoomFactor * m_BaselineImage.Width < baselinePictureBox.Width )
                    m_PointToPaint.X = baselinePictureBox.Width - (int) ( m_ZoomFactor * m_BaselineImage.Width );
                if( m_PointToPaint.Y + m_ZoomFactor * m_BaselineImage.Height < baselinePictureBox.Height )
                    m_PointToPaint.Y = baselinePictureBox.Height - (int) ( m_ZoomFactor * m_BaselineImage.Height );

                //refresh both images
                baselinePictureBox.Invalidate();
                resultPictureBox.Invalidate();
                baselinePictureBox.Update();
                resultPictureBox.Update();
            }
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>calculates the bitmap to print when the baseline picture box is refreshed</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void baselinePictureBox_Paint(object sender , PaintEventArgs e)
        {
            Size sizeToPrint = new Size((int) ( m_BaselineImage.Width * m_ZoomFactor ) , (int) ( m_BaselineImage.Height * m_ZoomFactor )); //size of the image to print

            try
            {
                Bitmap toPrint = new Bitmap(m_BaselineImage , sizeToPrint);//create a new bitamp smaller/larger than the original
                e.Graphics.Clear(Color.White);
                e.Graphics.DrawImage(toPrint , m_PointToPaint); //draw the bitmap at the specified location
                m_ImageErrorBaseline = false;
            }
            catch // reload the m_Baseline image if it gets corrupt
            {
                m_ImageErrorBaseline = true;
                m_BaselineImage = RasterManipulation.ConvertByteArrayToBitmap(Program.ResultList[m_IteratorList - 1].ResultRecord.BaselineBitmap);
                baselinePictureBox.Invalidate();
                baselinePictureBox.Update();
            }
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>calculates the bitmap to print when the output picture box is refreshed</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void resultPictureBox_Paint(object sender , PaintEventArgs e)
        {
            try
            {
                if( m_ResultImage != null )
                {
                    Size sizeToPrint = new Size((int) ( m_ResultImage.Width * m_ZoomFactor ) , (int) ( m_ResultImage.Height * m_ZoomFactor )); //size of the image to print
                    Bitmap toPrint = new Bitmap(m_ResultImage , sizeToPrint);//create a new bitamp smaller/larger than the original

                    e.Graphics.Clear(Color.White);
                    e.Graphics.DrawImage(toPrint , m_PointToPaint); //draw the bitmap at the specified location
                    m_ImageErrorResult = false;
                }
            }
            catch// reload the m+_ResultImage if it gets corrupt
            {
                m_ImageErrorResult = true;
                ResultPictureChange();
            }
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Calculate a new zoom factor when mouse wheel is used</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        protected override void OnMouseWheel(MouseEventArgs e)
        {
            if( !m_BaselineIsXML && !m_ImageErrorBaseline && !m_ImageErrorResult ) //prevent anything to be done if baseline is a text file or if one of the image are corrupted
            {
                double ancientZoomFactor = m_ZoomFactor;
                if( e.Delta > 1 ) //if mouse wheel was pressed frontward
                    m_ZoomFactor += 0.5;
                else //if mouse wheel was pressed backward
                    m_ZoomFactor -= 0.5;

                if( m_ZoomFactor < m_ZoomFactorMin ) //prevent the zoomfactor from getting too small
                    m_ZoomFactor = m_ZoomFactorMin;

                if( m_ZoomFactor > 10 ) //prevent the zoomfactor from getting too big
                    m_ZoomFactor = 10;

                m_PointToPaint.X = (int) ( ( m_PointToPaint.X - ( baselinePictureBox.Width/2 ) ) * ( m_ZoomFactor / ancientZoomFactor ) + ( baselinePictureBox.Width/2 ) ); // makes sure the zoom is made in the center of the image
                m_PointToPaint.Y = (int) ( ( m_PointToPaint.Y - ( baselinePictureBox.Height/2 ) ) * ( m_ZoomFactor / ancientZoomFactor ) + ( baselinePictureBox.Height/2 ) );

                //prevent  blanck spot from gettint into the image
                if( m_PointToPaint.X > 0 )
                    m_PointToPaint.X = 0;
                if( m_PointToPaint.Y > 0 )
                    m_PointToPaint.Y = 0;
                if( m_PointToPaint.X + m_ZoomFactor * m_BaselineImage.Width < baselinePictureBox.Width )
                    m_PointToPaint.X = baselinePictureBox.Width - (int) ( m_ZoomFactor * m_BaselineImage.Width );
                if( m_PointToPaint.Y + m_ZoomFactor * m_BaselineImage.Height < baselinePictureBox.Height )
                    m_PointToPaint.Y = baselinePictureBox.Height - (int) ( m_ZoomFactor * m_BaselineImage.Height );

                //refresh both picture box
                baselinePictureBox.Invalidate();
                resultPictureBox.Invalidate();
                baselinePictureBox.Update();
                resultPictureBox.Update();
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Change the output image if the result check box is changed</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void ResultCheckBox_CheckedChanged(object sender , EventArgs e)
        {
            if( m_BaseResultImage != null )
                ResultPictureChange();
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Change the output image if the pixel comparison check box is changed</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void pixelComparisonCheckBox_CheckedChanged(object sender , EventArgs e)
        {
            if( m_BaseResultImage != null )
                ResultPictureChange();
        }

        #endregion

        #region methods/otherEventHandler

        /*------------------------------------------------------------------------------------**/
        /// <summary>Replace the current baseline in the database with the current base result image</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void SaveAsBaselineButton_Click(object sender , EventArgs e)
        {
            if( !Program.IsTestRunning )
            {
                if( !m_OutputIsnothing && !m_IsError )
                {
                    Record record = Program.ResultList[m_IteratorList - 1].ResultRecord; //get the record to modify
                    record.BaselineBitmap = RasterManipulation.ConvertImageToByteArray(m_BaseResultImage); //assign the new bitmap
                    m_BaselineImage = new Bitmap(m_BaseResultImage); //change the baseline in the comparison form
                    Program.DBManager.UpdateRecord(record , m_IteratorList - 1 , false , true); //update the record in the database
                    Program.DBManager.LoadLastResult(m_IteratorList - 1); //reload current result to load new difference bitmap
                    MessageBox.Show("Save as baseline succesful");
                }
                else if( outputTextBox.Text != "" && m_IsError )
                {
                    Record record = Program.ResultList[m_IteratorList - 1].ResultRecord; //get the record to modify
                    record.BaselineBitmap = System.Text.Encoding.GetEncoding(28591).GetBytes(outputTextBox.Text);
                    Program.DBManager.UpdateRecord(record , m_IteratorList - 1 , false , true); //update the record in the database
                    Program.DBManager.LoadLastResult(m_IteratorList - 1); //reload current result to load new difference bitmap
                    MessageBox.Show("Save as baseline succesful");
                }
                else
                    MessageBox.Show("Cannot save nothing as a baseline");
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Close form</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void CloseButton_Click(object sender , EventArgs e)
        {
            this.Hide();
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Change the skipsucess parameter when the check box is changed</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void skipSuccesCheckBox_CheckedChanged(object sender , EventArgs e)
        {
            m_SkipSuccess = skipSuccesCheckBox.Checked;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Save the current baseline to an image/text file</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void saveBaselineButton_Click(object sender , EventArgs e)
        {
            if( m_BaselineIsXML )
                baselineSaveFileDialog.Filter = "XML Files(*.xml)|*.xml|Text Files(*.txt)|*.txt";
            else
                baselineSaveFileDialog.Filter = "Image Files(*.png, *.jpg, *.gif)|*.png;*.jpeg;*.jpg.;*gif";

            DialogResult dialogResult = baselineSaveFileDialog.ShowDialog();
            if( dialogResult == System.Windows.Forms.DialogResult.OK )
            {
                if( m_BaselineIsXML )
                    baselineTextBox.SaveFile(baselineSaveFileDialog.FileName , RichTextBoxStreamType.PlainText); //save to a text file
                else
                {
                    Bitmap toSave = new Bitmap(m_BaselineImage);
                    toSave.Save(baselineSaveFileDialog.FileName); //save to an image file
                }
            }
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Save the current output to a image/text file</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void saveComparisonButton_Click(object sender , EventArgs e)
        {
            if( m_IsError )
                comparisonSaveFileDialog.Filter = "XML Files(*.xml)|*.xml|Text Files(*.txt)|*.txt";
            else
                comparisonSaveFileDialog.Filter = "Image Files(*.png, *.jpg, *.gif)|*.png;*.jpeg;*.jpg.;*gif";

            DialogResult dialogResult = comparisonSaveFileDialog.ShowDialog();
            if( dialogResult == System.Windows.Forms.DialogResult.OK )
            {
                if( m_IsError )
                    baselineTextBox.SaveFile(comparisonSaveFileDialog.FileName , RichTextBoxStreamType.PlainText);//save to a text file
                else
                {
                    Bitmap toSave = new Bitmap(m_ResultImage);
                    toSave.Save(comparisonSaveFileDialog.FileName);//save to an image file
                }
            }
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Allow text box to resize correctly when form's size is changed</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void ResultComparisonForm_SizeChanged(object sender , EventArgs e)
        {
            int width = ( this.Width - 44 )/2;
            int location = width+19;
            baselineTextBox.Width = width;
            outputTextBox.Location = new Point(location , resultPictureBox.Location.Y);
            outputTextBox.Width = width;
        }

        #endregion
        
        #region methods/others

        /*------------------------------------------------------------------------------------**/
        /// <summary>Returns the date associate with a timestamp</summary>
        /// <param name="timestamp">Timestamp to evaluate</param>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private string GetDate(int timestamp)
        {
            DateTime date = new DateTime(1970 , 1 , 1 , 0 , 0 , 0 , 0);
            date = date.AddSeconds(timestamp);
            return date.Date.ToShortDateString();
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Returns the time associate with a timestamp</summary>
        /// <param name="timestamp">Timestamp to evaluate</param>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private string GetTime(int timestamp)
        {
            DateTime time = new DateTime(1970 , 1 , 1 , 0 , 0 , 0 , 0);
            time = time.AddSeconds(timestamp);
            return time.TimeOfDay.ToString();
        }

        #endregion

        #endregion
    }
}

