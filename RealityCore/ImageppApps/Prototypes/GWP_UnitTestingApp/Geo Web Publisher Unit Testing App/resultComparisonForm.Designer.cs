namespace Geo_Web_Publisher_Unit_Testing_App
{
    partial class ResultComparisonForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(ResultComparisonForm));
            this.FirstButton = new System.Windows.Forms.Button();
            this.PreviousButton = new System.Windows.Forms.Button();
            this.NextButton = new System.Windows.Forms.Button();
            this.LastButton = new System.Windows.Forms.Button();
            this.numberLabel = new System.Windows.Forms.Label();
            this.nameLabel = new System.Windows.Forms.Label();
            this.descriptionLabel = new System.Windows.Forms.Label();
            this.baselinePictureBox = new System.Windows.Forms.PictureBox();
            this.SaveAsBaselineButton = new System.Windows.Forms.Button();
            this.CloseButton = new System.Windows.Forms.Button();
            this.label4 = new System.Windows.Forms.Label();
            this.UrlTextBox = new System.Windows.Forms.TextBox();
            this.resultPictureBox = new System.Windows.Forms.PictureBox();
            this.ResultCheckBox = new System.Windows.Forms.CheckBox();
            this.pixelComparisonCheckBox = new System.Windows.Forms.CheckBox();
            this.statusLabel = new System.Windows.Forms.Label();
            this.StatusMarkerLabel = new System.Windows.Forms.Label();
            this.OldResultComboBox = new System.Windows.Forms.ComboBox();
            this.skipSuccesCheckBox = new System.Windows.Forms.CheckBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.failedRequestCount = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.testPrefixLabel = new System.Windows.Forms.Label();
            this.testDescriptionLabel = new System.Windows.Forms.Label();
            this.TimeLabel = new System.Windows.Forms.Label();
            this.DateLabel = new System.Windows.Forms.Label();
            this.recordGroupdBox = new System.Windows.Forms.GroupBox();
            this.largeDiffPixelCount = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.pixelCountLabel = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.saveBaselineButton = new System.Windows.Forms.Button();
            this.saveComparisonButton = new System.Windows.Forms.Button();
            this.baselineSaveFileDialog = new System.Windows.Forms.SaveFileDialog();
            this.comparisonSaveFileDialog = new System.Windows.Forms.SaveFileDialog();
            this.outputTextBox = new System.Windows.Forms.RichTextBox();
            this.baselineTextBox = new System.Windows.Forms.RichTextBox();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            ((System.ComponentModel.ISupportInitialize)(this.baselinePictureBox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.resultPictureBox)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.recordGroupdBox.SuspendLayout();
            this.SuspendLayout();
            // 
            // FirstButton
            // 
            this.FirstButton.Image = global::Geo_Web_Publisher_Unit_Testing_App.Properties.Resources._9_av_previous;
            this.FirstButton.Location = new System.Drawing.Point(12, 6);
            this.FirstButton.Name = "FirstButton";
            this.FirstButton.Size = new System.Drawing.Size(34, 34);
            this.FirstButton.TabIndex = 0;
            this.toolTip1.SetToolTip(this.FirstButton, "First");
            this.FirstButton.UseVisualStyleBackColor = true;
            this.FirstButton.Click += new System.EventHandler(this.FirstButton_Click);
            // 
            // PreviousButton
            // 
            this.PreviousButton.Image = global::Geo_Web_Publisher_Unit_Testing_App.Properties.Resources._9_av_rewind;
            this.PreviousButton.Location = new System.Drawing.Point(52, 6);
            this.PreviousButton.Name = "PreviousButton";
            this.PreviousButton.Size = new System.Drawing.Size(34, 34);
            this.PreviousButton.TabIndex = 1;
            this.toolTip1.SetToolTip(this.PreviousButton, "Previous");
            this.PreviousButton.UseVisualStyleBackColor = true;
            this.PreviousButton.Click += new System.EventHandler(this.PreviousButton_Click);
            // 
            // NextButton
            // 
            this.NextButton.Image = global::Geo_Web_Publisher_Unit_Testing_App.Properties.Resources._9_av_fast_forward;
            this.NextButton.Location = new System.Drawing.Point(92, 6);
            this.NextButton.Name = "NextButton";
            this.NextButton.Size = new System.Drawing.Size(34, 34);
            this.NextButton.TabIndex = 2;
            this.toolTip1.SetToolTip(this.NextButton, "Next");
            this.NextButton.UseVisualStyleBackColor = true;
            this.NextButton.Click += new System.EventHandler(this.NextButton_Click);
            // 
            // LastButton
            // 
            this.LastButton.Image = global::Geo_Web_Publisher_Unit_Testing_App.Properties.Resources._9_av_next;
            this.LastButton.Location = new System.Drawing.Point(132, 6);
            this.LastButton.Name = "LastButton";
            this.LastButton.Size = new System.Drawing.Size(34, 34);
            this.LastButton.TabIndex = 3;
            this.toolTip1.SetToolTip(this.LastButton, "Last");
            this.LastButton.UseVisualStyleBackColor = true;
            this.LastButton.Click += new System.EventHandler(this.LastButton_Click);
            // 
            // numberLabel
            // 
            this.numberLabel.AutoSize = true;
            this.numberLabel.Location = new System.Drawing.Point(172, 17);
            this.numberLabel.Name = "numberLabel";
            this.numberLabel.Size = new System.Drawing.Size(24, 13);
            this.numberLabel.TabIndex = 4;
            this.numberLabel.Text = "0/0";
            // 
            // nameLabel
            // 
            this.nameLabel.AutoSize = true;
            this.nameLabel.Location = new System.Drawing.Point(6, 16);
            this.nameLabel.Name = "nameLabel";
            this.nameLabel.Size = new System.Drawing.Size(59, 13);
            this.nameLabel.TabIndex = 5;
            this.nameLabel.Text = "Test Name";
            // 
            // descriptionLabel
            // 
            this.descriptionLabel.AutoSize = true;
            this.descriptionLabel.Location = new System.Drawing.Point(6, 32);
            this.descriptionLabel.Name = "descriptionLabel";
            this.descriptionLabel.Size = new System.Drawing.Size(84, 13);
            this.descriptionLabel.TabIndex = 6;
            this.descriptionLabel.Text = "Test Description";
            // 
            // baselinePictureBox
            // 
            this.baselinePictureBox.Location = new System.Drawing.Point(7, 79);
            this.baselinePictureBox.Name = "baselinePictureBox";
            this.baselinePictureBox.Size = new System.Drawing.Size(600, 400);
            this.baselinePictureBox.TabIndex = 7;
            this.baselinePictureBox.TabStop = false;
            this.baselinePictureBox.Paint += new System.Windows.Forms.PaintEventHandler(this.baselinePictureBox_Paint);
            this.baselinePictureBox.MouseDown += new System.Windows.Forms.MouseEventHandler(this.PictureBox_MouseDown);
            this.baselinePictureBox.MouseMove += new System.Windows.Forms.MouseEventHandler(this.PictureBox_MouseMove);
            this.baselinePictureBox.MouseUp += new System.Windows.Forms.MouseEventHandler(this.PictureBox_MouseUp);
            // 
            // SaveAsBaselineButton
            // 
            this.SaveAsBaselineButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.SaveAsBaselineButton.Location = new System.Drawing.Point(1120, 483);
            this.SaveAsBaselineButton.Name = "SaveAsBaselineButton";
            this.SaveAsBaselineButton.Size = new System.Drawing.Size(99, 22);
            this.SaveAsBaselineButton.TabIndex = 10;
            this.SaveAsBaselineButton.Text = "Save As Baseline";
            this.SaveAsBaselineButton.UseVisualStyleBackColor = true;
            this.SaveAsBaselineButton.Click += new System.EventHandler(this.SaveAsBaselineButton_Click);
            // 
            // CloseButton
            // 
            this.CloseButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.CloseButton.Location = new System.Drawing.Point(1035, 483);
            this.CloseButton.Name = "CloseButton";
            this.CloseButton.Size = new System.Drawing.Size(79, 22);
            this.CloseButton.TabIndex = 11;
            this.CloseButton.Text = "Close";
            this.toolTip1.SetToolTip(this.CloseButton, "Save the current comparison as the new baseline");
            this.CloseButton.UseVisualStyleBackColor = true;
            this.CloseButton.Click += new System.EventHandler(this.CloseButton_Click);
            // 
            // label4
            // 
            this.label4.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(12, 488);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(32, 13);
            this.label4.TabIndex = 12;
            this.label4.Text = "URL:";
            // 
            // UrlTextBox
            // 
            this.UrlTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.UrlTextBox.Location = new System.Drawing.Point(46, 485);
            this.UrlTextBox.Name = "UrlTextBox";
            this.UrlTextBox.ReadOnly = true;
            this.UrlTextBox.Size = new System.Drawing.Size(983, 20);
            this.UrlTextBox.TabIndex = 13;
            // 
            // resultPictureBox
            // 
            this.resultPictureBox.Location = new System.Drawing.Point(619, 79);
            this.resultPictureBox.Name = "resultPictureBox";
            this.resultPictureBox.Size = new System.Drawing.Size(600, 400);
            this.resultPictureBox.TabIndex = 14;
            this.resultPictureBox.TabStop = false;
            this.resultPictureBox.Paint += new System.Windows.Forms.PaintEventHandler(this.resultPictureBox_Paint);
            this.resultPictureBox.MouseDown += new System.Windows.Forms.MouseEventHandler(this.PictureBox_MouseDown);
            this.resultPictureBox.MouseMove += new System.Windows.Forms.MouseEventHandler(this.PictureBox_MouseMove);
            this.resultPictureBox.MouseUp += new System.Windows.Forms.MouseEventHandler(this.PictureBox_MouseUp);
            // 
            // ResultCheckBox
            // 
            this.ResultCheckBox.AutoSize = true;
            this.ResultCheckBox.Checked = true;
            this.ResultCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.ResultCheckBox.Location = new System.Drawing.Point(619, 12);
            this.ResultCheckBox.Name = "ResultCheckBox";
            this.ResultCheckBox.Size = new System.Drawing.Size(74, 17);
            this.ResultCheckBox.TabIndex = 15;
            this.ResultCheckBox.Text = "Output file";
            this.ResultCheckBox.UseVisualStyleBackColor = true;
            this.ResultCheckBox.CheckedChanged += new System.EventHandler(this.ResultCheckBox_CheckedChanged);
            // 
            // pixelComparisonCheckBox
            // 
            this.pixelComparisonCheckBox.AutoSize = true;
            this.pixelComparisonCheckBox.Location = new System.Drawing.Point(619, 33);
            this.pixelComparisonCheckBox.Name = "pixelComparisonCheckBox";
            this.pixelComparisonCheckBox.Size = new System.Drawing.Size(105, 17);
            this.pixelComparisonCheckBox.TabIndex = 16;
            this.pixelComparisonCheckBox.Text = "Pixel comparison";
            this.pixelComparisonCheckBox.UseVisualStyleBackColor = true;
            this.pixelComparisonCheckBox.CheckedChanged += new System.EventHandler(this.pixelComparisonCheckBox_CheckedChanged);
            // 
            // statusLabel
            // 
            this.statusLabel.AutoSize = true;
            this.statusLabel.Location = new System.Drawing.Point(766, 12);
            this.statusLabel.Name = "statusLabel";
            this.statusLabel.Size = new System.Drawing.Size(33, 13);
            this.statusLabel.TabIndex = 17;
            this.statusLabel.Text = "None";
            // 
            // StatusMarkerLabel
            // 
            this.StatusMarkerLabel.AutoSize = true;
            this.StatusMarkerLabel.Location = new System.Drawing.Point(720, 12);
            this.StatusMarkerLabel.Name = "StatusMarkerLabel";
            this.StatusMarkerLabel.Size = new System.Drawing.Size(40, 13);
            this.StatusMarkerLabel.TabIndex = 18;
            this.StatusMarkerLabel.Text = "Status:";
            // 
            // OldResultComboBox
            // 
            this.OldResultComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.OldResultComboBox.FormattingEnabled = true;
            this.OldResultComboBox.Location = new System.Drawing.Point(619, 52);
            this.OldResultComboBox.Name = "OldResultComboBox";
            this.OldResultComboBox.Size = new System.Drawing.Size(208, 21);
            this.OldResultComboBox.TabIndex = 20;
            this.OldResultComboBox.SelectedIndexChanged += new System.EventHandler(this.OldResutComboBox_SelectedIndexChanged);
            // 
            // skipSuccesCheckBox
            // 
            this.skipSuccesCheckBox.AutoSize = true;
            this.skipSuccesCheckBox.Location = new System.Drawing.Point(723, 32);
            this.skipSuccesCheckBox.Name = "skipSuccesCheckBox";
            this.skipSuccesCheckBox.Size = new System.Drawing.Size(133, 17);
            this.skipSuccesCheckBox.TabIndex = 22;
            this.skipSuccesCheckBox.Text = "Skip successful record";
            this.skipSuccesCheckBox.UseVisualStyleBackColor = true;
            this.skipSuccesCheckBox.CheckedChanged += new System.EventHandler(this.skipSuccesCheckBox_CheckedChanged);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.failedRequestCount);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.testPrefixLabel);
            this.groupBox1.Controls.Add(this.testDescriptionLabel);
            this.groupBox1.Controls.Add(this.TimeLabel);
            this.groupBox1.Controls.Add(this.DateLabel);
            this.groupBox1.Location = new System.Drawing.Point(854, 1);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(365, 71);
            this.groupBox1.TabIndex = 23;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Test informations";
            // 
            // failedRequestCount
            // 
            this.failedRequestCount.AutoSize = true;
            this.failedRequestCount.Location = new System.Drawing.Point(154, 54);
            this.failedRequestCount.Name = "failedRequestCount";
            this.failedRequestCount.Size = new System.Drawing.Size(100, 13);
            this.failedRequestCount.TabIndex = 29;
            this.failedRequestCount.Text = "failedRequestCount";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(6, 54);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(142, 13);
            this.label2.TabIndex = 28;
            this.label2.Text = "Unsuccessful request count:";
            // 
            // testPrefixLabel
            // 
            this.testPrefixLabel.AutoSize = true;
            this.testPrefixLabel.Location = new System.Drawing.Point(6, 36);
            this.testPrefixLabel.Name = "testPrefixLabel";
            this.testPrefixLabel.Size = new System.Drawing.Size(35, 13);
            this.testPrefixLabel.TabIndex = 27;
            this.testPrefixLabel.Text = "label1";
            // 
            // testDescriptionLabel
            // 
            this.testDescriptionLabel.AutoSize = true;
            this.testDescriptionLabel.Location = new System.Drawing.Point(6, 16);
            this.testDescriptionLabel.Name = "testDescriptionLabel";
            this.testDescriptionLabel.Size = new System.Drawing.Size(35, 13);
            this.testDescriptionLabel.TabIndex = 26;
            this.testDescriptionLabel.Text = "label1";
            // 
            // TimeLabel
            // 
            this.TimeLabel.AutoSize = true;
            this.TimeLabel.Location = new System.Drawing.Point(301, 36);
            this.TimeLabel.Name = "TimeLabel";
            this.TimeLabel.Size = new System.Drawing.Size(30, 13);
            this.TimeLabel.TabIndex = 25;
            this.TimeLabel.Text = "Time";
            // 
            // DateLabel
            // 
            this.DateLabel.AutoSize = true;
            this.DateLabel.Location = new System.Drawing.Point(301, 19);
            this.DateLabel.Name = "DateLabel";
            this.DateLabel.Size = new System.Drawing.Size(30, 13);
            this.DateLabel.TabIndex = 24;
            this.DateLabel.Text = "Date";
            // 
            // recordGroupdBox
            // 
            this.recordGroupdBox.Controls.Add(this.largeDiffPixelCount);
            this.recordGroupdBox.Controls.Add(this.label3);
            this.recordGroupdBox.Controls.Add(this.pixelCountLabel);
            this.recordGroupdBox.Controls.Add(this.label1);
            this.recordGroupdBox.Controls.Add(this.descriptionLabel);
            this.recordGroupdBox.Controls.Add(this.nameLabel);
            this.recordGroupdBox.Location = new System.Drawing.Point(214, 1);
            this.recordGroupdBox.Name = "recordGroupdBox";
            this.recordGroupdBox.Size = new System.Drawing.Size(393, 72);
            this.recordGroupdBox.TabIndex = 24;
            this.recordGroupdBox.TabStop = false;
            this.recordGroupdBox.Text = "Record informations";
            // 
            // largeDiffPixelCount
            // 
            this.largeDiffPixelCount.AutoSize = true;
            this.largeDiffPixelCount.Location = new System.Drawing.Point(320, 50);
            this.largeDiffPixelCount.Name = "largeDiffPixelCount";
            this.largeDiffPixelCount.Size = new System.Drawing.Size(35, 13);
            this.largeDiffPixelCount.TabIndex = 10;
            this.largeDiffPixelCount.Text = "label2";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(183, 50);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(141, 13);
            this.label3.TabIndex = 9;
            this.label3.Text = "Large difference pixel count:";
            // 
            // pixelCountLabel
            // 
            this.pixelCountLabel.AutoSize = true;
            this.pixelCountLabel.Location = new System.Drawing.Point(98, 50);
            this.pixelCountLabel.Name = "pixelCountLabel";
            this.pixelCountLabel.Size = new System.Drawing.Size(35, 13);
            this.pixelCountLabel.TabIndex = 8;
            this.pixelCountLabel.Text = "label2";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(6, 50);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(95, 13);
            this.label1.TabIndex = 7;
            this.label1.Text = "Invalid pixel count:";
            // 
            // saveBaselineButton
            // 
            this.saveBaselineButton.Image = global::Geo_Web_Publisher_Unit_Testing_App.Properties.Resources._5_content_save;
            this.saveBaselineButton.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.saveBaselineButton.Location = new System.Drawing.Point(12, 46);
            this.saveBaselineButton.Name = "saveBaselineButton";
            this.saveBaselineButton.Size = new System.Drawing.Size(84, 27);
            this.saveBaselineButton.TabIndex = 25;
            this.saveBaselineButton.Text = " Baseline";
            this.saveBaselineButton.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.toolTip1.SetToolTip(this.saveBaselineButton, "Save baseline on disk");
            this.saveBaselineButton.UseVisualStyleBackColor = true;
            this.saveBaselineButton.Click += new System.EventHandler(this.saveBaselineButton_Click);
            // 
            // saveComparisonButton
            // 
            this.saveComparisonButton.Image = global::Geo_Web_Publisher_Unit_Testing_App.Properties.Resources._5_content_save;
            this.saveComparisonButton.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.saveComparisonButton.Location = new System.Drawing.Point(102, 46);
            this.saveComparisonButton.Name = "saveComparisonButton";
            this.saveComparisonButton.Size = new System.Drawing.Size(97, 27);
            this.saveComparisonButton.TabIndex = 26;
            this.saveComparisonButton.Text = "Comparison";
            this.saveComparisonButton.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.toolTip1.SetToolTip(this.saveComparisonButton, "Save comparison on disk");
            this.saveComparisonButton.UseVisualStyleBackColor = true;
            this.saveComparisonButton.Click += new System.EventHandler(this.saveComparisonButton_Click);
            // 
            // baselineSaveFileDialog
            // 
            this.baselineSaveFileDialog.Title = "Save file dialog";
            // 
            // comparisonSaveFileDialog
            // 
            this.comparisonSaveFileDialog.Title = "Save file dialog";
            // 
            // outputTextBox
            // 
            this.outputTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left)));
            this.outputTextBox.BackColor = System.Drawing.SystemColors.ControlLightLight;
            this.outputTextBox.Location = new System.Drawing.Point(619, 79);
            this.outputTextBox.Name = "outputTextBox";
            this.outputTextBox.ReadOnly = true;
            this.outputTextBox.Size = new System.Drawing.Size(600, 400);
            this.outputTextBox.TabIndex = 27;
            this.outputTextBox.Text = "";
            this.outputTextBox.Visible = false;
            // 
            // baselineTextBox
            // 
            this.baselineTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left)));
            this.baselineTextBox.BackColor = System.Drawing.SystemColors.ControlLightLight;
            this.baselineTextBox.Location = new System.Drawing.Point(7, 79);
            this.baselineTextBox.Name = "baselineTextBox";
            this.baselineTextBox.ReadOnly = true;
            this.baselineTextBox.Size = new System.Drawing.Size(600, 400);
            this.baselineTextBox.TabIndex = 28;
            this.baselineTextBox.Text = "";
            this.baselineTextBox.Visible = false;
            // 
            // ResultComparisonForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1228, 508);
            this.Controls.Add(this.baselineTextBox);
            this.Controls.Add(this.outputTextBox);
            this.Controls.Add(this.saveComparisonButton);
            this.Controls.Add(this.saveBaselineButton);
            this.Controls.Add(this.recordGroupdBox);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.skipSuccesCheckBox);
            this.Controls.Add(this.OldResultComboBox);
            this.Controls.Add(this.StatusMarkerLabel);
            this.Controls.Add(this.statusLabel);
            this.Controls.Add(this.pixelComparisonCheckBox);
            this.Controls.Add(this.ResultCheckBox);
            this.Controls.Add(this.resultPictureBox);
            this.Controls.Add(this.UrlTextBox);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.CloseButton);
            this.Controls.Add(this.SaveAsBaselineButton);
            this.Controls.Add(this.baselinePictureBox);
            this.Controls.Add(this.numberLabel);
            this.Controls.Add(this.LastButton);
            this.Controls.Add(this.NextButton);
            this.Controls.Add(this.PreviousButton);
            this.Controls.Add(this.FirstButton);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MinimumSize = new System.Drawing.Size(1244, 546);
            this.Name = "ResultComparisonForm";
            this.Text = "Result Comparison";
            this.SizeChanged += new System.EventHandler(this.ResultComparisonForm_SizeChanged);
            ((System.ComponentModel.ISupportInitialize)(this.baselinePictureBox)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.resultPictureBox)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.recordGroupdBox.ResumeLayout(false);
            this.recordGroupdBox.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button FirstButton;
        private System.Windows.Forms.Button PreviousButton;
        private System.Windows.Forms.Button NextButton;
        private System.Windows.Forms.Button LastButton;
        private System.Windows.Forms.Label numberLabel;
        private System.Windows.Forms.Label nameLabel;
        private System.Windows.Forms.Label descriptionLabel;
        private System.Windows.Forms.PictureBox baselinePictureBox;
        private System.Windows.Forms.Button SaveAsBaselineButton;
        private System.Windows.Forms.Button CloseButton;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TextBox UrlTextBox;
        private System.Windows.Forms.PictureBox resultPictureBox;
        private System.Windows.Forms.CheckBox ResultCheckBox;
        private System.Windows.Forms.CheckBox pixelComparisonCheckBox;
        private System.Windows.Forms.Label statusLabel;
        private System.Windows.Forms.Label StatusMarkerLabel;
        private System.Windows.Forms.ComboBox OldResultComboBox;
        private System.Windows.Forms.CheckBox skipSuccesCheckBox;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Label TimeLabel;
        private System.Windows.Forms.Label DateLabel;
        private System.Windows.Forms.GroupBox recordGroupdBox;
        private System.Windows.Forms.Label testDescriptionLabel;
        private System.Windows.Forms.Button saveBaselineButton;
        private System.Windows.Forms.Button saveComparisonButton;
        private System.Windows.Forms.SaveFileDialog baselineSaveFileDialog;
        private System.Windows.Forms.SaveFileDialog comparisonSaveFileDialog;
        private System.Windows.Forms.RichTextBox outputTextBox;
        private System.Windows.Forms.RichTextBox baselineTextBox;
        private System.Windows.Forms.Label testPrefixLabel;
        private System.Windows.Forms.Label largeDiffPixelCount;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label pixelCountLabel;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label failedRequestCount;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ToolTip toolTip1;
    }
}