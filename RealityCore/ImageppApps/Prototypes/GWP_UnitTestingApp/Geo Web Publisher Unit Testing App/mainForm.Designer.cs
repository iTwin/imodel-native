namespace Geo_Web_Publisher_Unit_Testing_App
{
    partial class MainForm
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.toolStripStatusLabel1 = new System.Windows.Forms.ToolStripStatusLabel();
            this.toolStrip1 = new System.Windows.Forms.ToolStrip();
            this.OptionsButton = new System.Windows.Forms.ToolStripButton();
            this.deleteResultsButton = new System.Windows.Forms.ToolStripButton();
            this.ProtocolLabel = new System.Windows.Forms.Label();
            this.ServerNameLabel = new System.Windows.Forms.Label();
            this.PortNumberLabel = new System.Windows.Forms.Label();
            this.WmsSelectButton = new System.Windows.Forms.Button();
            this.panel1 = new System.Windows.Forms.Panel();
            this.updateAllButton = new System.Windows.Forms.Button();
            this.CopyUrlToClipboard = new System.Windows.Forms.Button();
            this.updateButton = new System.Windows.Forms.Button();
            this.ResultComparisonButton = new System.Windows.Forms.Button();
            this.DeleteButton = new System.Windows.Forms.Button();
            this.EditButton = new System.Windows.Forms.Button();
            this.StopButton = new System.Windows.Forms.Button();
            this.PlayButton = new System.Windows.Forms.Button();
            this.requestDataGridView = new System.Windows.Forms.DataGridView();
            this.ExecuteTest = new System.Windows.Forms.DataGridViewCheckBoxColumn();
            this.ID = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.UpToDate = new System.Windows.Forms.DataGridViewImageColumn();
            this.NameGrid = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Description = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Category = new System.Windows.Forms.DataGridViewComboBoxColumn();
            this.URL = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Status = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.categoryDropDownList = new System.Windows.Forms.ComboBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.statusDropDownList = new System.Windows.Forms.ComboBox();
            this.Tooltip = new System.Windows.Forms.ToolTip(this.components);
            this.comboBoxPortNumber = new System.Windows.Forms.ComboBox();
            this.comboBoxServerName = new System.Windows.Forms.ComboBox();
            this.comboBoxProtocol = new System.Windows.Forms.ComboBox();
            this.progressBarTest = new System.Windows.Forms.ProgressBar();
            this.statusStrip1.SuspendLayout();
            this.toolStrip1.SuspendLayout();
            this.panel1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.requestDataGridView)).BeginInit();
            this.SuspendLayout();
            // 
            // statusStrip1
            // 
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabel1});
            this.statusStrip1.Location = new System.Drawing.Point(0, 658);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Size = new System.Drawing.Size(950, 22);
            this.statusStrip1.TabIndex = 0;
            this.statusStrip1.Text = "statusStrip1";
            // 
            // toolStripStatusLabel1
            // 
            this.toolStripStatusLabel1.Name = "toolStripStatusLabel1";
            this.toolStripStatusLabel1.Size = new System.Drawing.Size(128, 17);
            this.toolStripStatusLabel1.Text = "Waiting for test to start";
            // 
            // toolStrip1
            // 
            this.toolStrip1.GripStyle = System.Windows.Forms.ToolStripGripStyle.Hidden;
            this.toolStrip1.ImageScalingSize = new System.Drawing.Size(24, 24);
            this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.OptionsButton,
            this.deleteResultsButton});
            this.toolStrip1.Location = new System.Drawing.Point(0, 0);
            this.toolStrip1.Name = "toolStrip1";
            this.toolStrip1.RenderMode = System.Windows.Forms.ToolStripRenderMode.System;
            this.toolStrip1.Size = new System.Drawing.Size(950, 31);
            this.toolStrip1.TabIndex = 2;
            this.toolStrip1.Text = "toolStrip1";
            // 
            // OptionsButton
            // 
            this.OptionsButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.OptionsButton.Image = global::Geo_Web_Publisher_Unit_Testing_App.Properties.Resources._2_action_settings;
            this.OptionsButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.OptionsButton.Name = "OptionsButton";
            this.OptionsButton.Size = new System.Drawing.Size(28, 28);
            this.OptionsButton.ToolTipText = "Settings";
            this.OptionsButton.Click += new System.EventHandler(this.OptionsButton_Click);
            // 
            // deleteResultsButton
            // 
            this.deleteResultsButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.deleteResultsButton.Image = global::Geo_Web_Publisher_Unit_Testing_App.Properties.Resources._4_collections_view_as_list;
            this.deleteResultsButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.deleteResultsButton.Name = "deleteResultsButton";
            this.deleteResultsButton.Size = new System.Drawing.Size(28, 28);
            this.deleteResultsButton.ToolTipText = "Test list";
            this.deleteResultsButton.Click += new System.EventHandler(this.deleteResultButton_Click);
            // 
            // ProtocolLabel
            // 
            this.ProtocolLabel.AutoSize = true;
            this.ProtocolLabel.Location = new System.Drawing.Point(9, 39);
            this.ProtocolLabel.Name = "ProtocolLabel";
            this.ProtocolLabel.Size = new System.Drawing.Size(49, 13);
            this.ProtocolLabel.TabIndex = 3;
            this.ProtocolLabel.Text = "Protocol:";
            // 
            // ServerNameLabel
            // 
            this.ServerNameLabel.AutoSize = true;
            this.ServerNameLabel.Location = new System.Drawing.Point(172, 39);
            this.ServerNameLabel.Name = "ServerNameLabel";
            this.ServerNameLabel.Size = new System.Drawing.Size(72, 13);
            this.ServerNameLabel.TabIndex = 5;
            this.ServerNameLabel.Text = "Server Name:";
            // 
            // PortNumberLabel
            // 
            this.PortNumberLabel.AutoSize = true;
            this.PortNumberLabel.Location = new System.Drawing.Point(410, 39);
            this.PortNumberLabel.Name = "PortNumberLabel";
            this.PortNumberLabel.Size = new System.Drawing.Size(69, 13);
            this.PortNumberLabel.TabIndex = 7;
            this.PortNumberLabel.Text = "Port Number:";
            // 
            // WmsSelectButton
            // 
            this.WmsSelectButton.AccessibleDescription = "Add new request";
            this.WmsSelectButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.WmsSelectButton.Image = global::Geo_Web_Publisher_Unit_Testing_App.Properties.Resources._5_content_new_picture;
            this.WmsSelectButton.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.WmsSelectButton.Location = new System.Drawing.Point(844, 87);
            this.WmsSelectButton.Name = "WmsSelectButton";
            this.WmsSelectButton.Size = new System.Drawing.Size(94, 38);
            this.WmsSelectButton.TabIndex = 10;
            this.WmsSelectButton.Text = "WMS...";
            this.WmsSelectButton.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.Tooltip.SetToolTip(this.WmsSelectButton, "Add new wms request");
            this.WmsSelectButton.UseVisualStyleBackColor = true;
            this.WmsSelectButton.Click += new System.EventHandler(this.WmsSelectButton_Click);
            // 
            // panel1
            // 
            this.panel1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.panel1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.panel1.Controls.Add(this.updateAllButton);
            this.panel1.Controls.Add(this.CopyUrlToClipboard);
            this.panel1.Controls.Add(this.updateButton);
            this.panel1.Controls.Add(this.ResultComparisonButton);
            this.panel1.Controls.Add(this.DeleteButton);
            this.panel1.Controls.Add(this.EditButton);
            this.panel1.Location = new System.Drawing.Point(844, 141);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(94, 147);
            this.panel1.TabIndex = 11;
            // 
            // updateAllButton
            // 
            this.updateAllButton.Image = global::Geo_Web_Publisher_Unit_Testing_App.Properties.Resources._6_social_reply_all;
            this.updateAllButton.Location = new System.Drawing.Point(49, 98);
            this.updateAllButton.Name = "updateAllButton";
            this.updateAllButton.Size = new System.Drawing.Size(40, 40);
            this.updateAllButton.TabIndex = 20;
            this.Tooltip.SetToolTip(this.updateAllButton, "Update all");
            this.updateAllButton.UseVisualStyleBackColor = true;
            this.updateAllButton.Click += new System.EventHandler(this.updateAllButton_Click);
            // 
            // CopyUrlToClipboard
            // 
            this.CopyUrlToClipboard.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.CopyUrlToClipboard.Enabled = false;
            this.CopyUrlToClipboard.Image = global::Geo_Web_Publisher_Unit_Testing_App.Properties.Resources._5_content_copy;
            this.CopyUrlToClipboard.Location = new System.Drawing.Point(49, 54);
            this.CopyUrlToClipboard.Name = "CopyUrlToClipboard";
            this.CopyUrlToClipboard.Size = new System.Drawing.Size(40, 40);
            this.CopyUrlToClipboard.TabIndex = 3;
            this.Tooltip.SetToolTip(this.CopyUrlToClipboard, "Copy selected request to clipboard");
            this.CopyUrlToClipboard.UseVisualStyleBackColor = true;
            this.CopyUrlToClipboard.Click += new System.EventHandler(this.CopyUrlToClipboard_Click);
            // 
            // updateButton
            // 
            this.updateButton.Image = global::Geo_Web_Publisher_Unit_Testing_App.Properties.Resources._6_social_reply;
            this.updateButton.Location = new System.Drawing.Point(3, 98);
            this.updateButton.Name = "updateButton";
            this.updateButton.Size = new System.Drawing.Size(40, 40);
            this.updateButton.TabIndex = 19;
            this.Tooltip.SetToolTip(this.updateButton, "Update selection");
            this.updateButton.UseVisualStyleBackColor = true;
            this.updateButton.Click += new System.EventHandler(this.updateButton_Click);
            // 
            // ResultComparisonButton
            // 
            this.ResultComparisonButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.ResultComparisonButton.Enabled = false;
            this.ResultComparisonButton.Image = global::Geo_Web_Publisher_Unit_Testing_App.Properties.Resources._5_content_picture;
            this.ResultComparisonButton.Location = new System.Drawing.Point(3, 54);
            this.ResultComparisonButton.Name = "ResultComparisonButton";
            this.ResultComparisonButton.Size = new System.Drawing.Size(40, 40);
            this.ResultComparisonButton.TabIndex = 2;
            this.Tooltip.SetToolTip(this.ResultComparisonButton, "Open comparison form");
            this.ResultComparisonButton.UseVisualStyleBackColor = true;
            this.ResultComparisonButton.Click += new System.EventHandler(this.ResultComparisonButton_Click);
            // 
            // DeleteButton
            // 
            this.DeleteButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.DeleteButton.Enabled = false;
            this.DeleteButton.Image = global::Geo_Web_Publisher_Unit_Testing_App.Properties.Resources._5_content_discard;
            this.DeleteButton.Location = new System.Drawing.Point(49, 8);
            this.DeleteButton.Name = "DeleteButton";
            this.DeleteButton.Size = new System.Drawing.Size(40, 40);
            this.DeleteButton.TabIndex = 1;
            this.Tooltip.SetToolTip(this.DeleteButton, "Delete current(s) request");
            this.DeleteButton.UseVisualStyleBackColor = true;
            this.DeleteButton.Click += new System.EventHandler(this.DeleteButton_Click);
            // 
            // EditButton
            // 
            this.EditButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.EditButton.Enabled = false;
            this.EditButton.Image = global::Geo_Web_Publisher_Unit_Testing_App.Properties.Resources._5_content_edit;
            this.EditButton.Location = new System.Drawing.Point(3, 8);
            this.EditButton.Name = "EditButton";
            this.EditButton.Size = new System.Drawing.Size(40, 40);
            this.EditButton.TabIndex = 0;
            this.Tooltip.SetToolTip(this.EditButton, "Edit current request");
            this.EditButton.UseVisualStyleBackColor = true;
            this.EditButton.Click += new System.EventHandler(this.EditButton_Click);
            // 
            // StopButton
            // 
            this.StopButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.StopButton.Enabled = false;
            this.StopButton.Image = global::Geo_Web_Publisher_Unit_Testing_App.Properties.Resources._9_av_stop;
            this.StopButton.Location = new System.Drawing.Point(894, 294);
            this.StopButton.Name = "StopButton";
            this.StopButton.Size = new System.Drawing.Size(40, 40);
            this.StopButton.TabIndex = 5;
            this.Tooltip.SetToolTip(this.StopButton, "Stop current execution of request(s)");
            this.StopButton.UseVisualStyleBackColor = true;
            this.StopButton.Click += new System.EventHandler(this.StopButton_Click);
            // 
            // PlayButton
            // 
            this.PlayButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.PlayButton.Image = global::Geo_Web_Publisher_Unit_Testing_App.Properties.Resources._9_av_play;
            this.PlayButton.Location = new System.Drawing.Point(848, 294);
            this.PlayButton.Name = "PlayButton";
            this.PlayButton.Size = new System.Drawing.Size(40, 40);
            this.PlayButton.TabIndex = 4;
            this.Tooltip.SetToolTip(this.PlayButton, "Execute checked request(s)");
            this.PlayButton.UseVisualStyleBackColor = true;
            this.PlayButton.Click += new System.EventHandler(this.PlayButton_Click);
            // 
            // requestDataGridView
            // 
            this.requestDataGridView.AllowUserToDeleteRows = false;
            this.requestDataGridView.AllowUserToOrderColumns = true;
            this.requestDataGridView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.requestDataGridView.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.requestDataGridView.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.ExecuteTest,
            this.ID,
            this.UpToDate,
            this.NameGrid,
            this.Description,
            this.Category,
            this.URL,
            this.Status});
            this.requestDataGridView.Location = new System.Drawing.Point(12, 89);
            this.requestDataGridView.Name = "requestDataGridView";
            this.requestDataGridView.RowHeadersWidth = 40;
            this.requestDataGridView.Size = new System.Drawing.Size(826, 554);
            this.requestDataGridView.TabIndex = 12;
            this.requestDataGridView.RowHeadersWidthChanged += new System.EventHandler(this.requestDataGridView_RowHeadersWidthChanged);
            this.requestDataGridView.CellContentClick += new System.Windows.Forms.DataGridViewCellEventHandler(this.requestDataGridView_CellContentClick);
            this.requestDataGridView.CellEndEdit += new System.Windows.Forms.DataGridViewCellEventHandler(this.requestDataGridView_CellEndEdit);
            this.requestDataGridView.CellLeave += new System.Windows.Forms.DataGridViewCellEventHandler(this.requestDataGridView_CellLeave);
            this.requestDataGridView.ColumnDisplayIndexChanged += new System.Windows.Forms.DataGridViewColumnEventHandler(this.requestDataGridView_ColumnDisplayIndexChanged);
            this.requestDataGridView.ColumnWidthChanged += new System.Windows.Forms.DataGridViewColumnEventHandler(this.requestDataGridView_ColumnWidthChanged);
            this.requestDataGridView.DataError += new System.Windows.Forms.DataGridViewDataErrorEventHandler(this.requestDataGridView_DataError);
            this.requestDataGridView.RowHeaderMouseDoubleClick += new System.Windows.Forms.DataGridViewCellMouseEventHandler(this.requestDataGridView_RowHeaderMouseDoubleClick);
            this.requestDataGridView.RowsAdded += new System.Windows.Forms.DataGridViewRowsAddedEventHandler(this.requestDataGridView_RowsAdded);
            this.requestDataGridView.RowsRemoved += new System.Windows.Forms.DataGridViewRowsRemovedEventHandler(this.requestDataGridView_RowsRemoved);
            this.requestDataGridView.SelectionChanged += new System.EventHandler(this.requestDataGridView_SelectionChanged);
            this.requestDataGridView.UserAddedRow += new System.Windows.Forms.DataGridViewRowEventHandler(this.requestDataGridView_UserAddedRow);
            this.requestDataGridView.KeyDown += new System.Windows.Forms.KeyEventHandler(this.requestDataGridView_KeyDown);
            this.requestDataGridView.Leave += new System.EventHandler(this.requestDataGridView_Leave);
            // 
            // ExecuteTest
            // 
            this.ExecuteTest.FalseValue = false;
            this.ExecuteTest.Frozen = true;
            this.ExecuteTest.HeaderText = "";
            this.ExecuteTest.Name = "ExecuteTest";
            this.ExecuteTest.Resizable = System.Windows.Forms.DataGridViewTriState.False;
            this.ExecuteTest.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.Automatic;
            this.ExecuteTest.TrueValue = true;
            this.ExecuteTest.Width = 35;
            // 
            // ID
            // 
            this.ID.HeaderText = "ID";
            this.ID.Name = "ID";
            this.ID.ReadOnly = true;
            this.ID.Width = 43;
            // 
            // UpToDate
            // 
            this.UpToDate.HeaderText = "";
            this.UpToDate.Name = "UpToDate";
            this.UpToDate.Width = 30;
            // 
            // NameGrid
            // 
            this.NameGrid.HeaderText = "Name";
            this.NameGrid.Name = "NameGrid";
            this.NameGrid.Width = 120;
            // 
            // Description
            // 
            this.Description.HeaderText = "Description";
            this.Description.Name = "Description";
            this.Description.Width = 175;
            // 
            // Category
            // 
            this.Category.HeaderText = "Category";
            this.Category.MaxDropDownItems = 40;
            this.Category.Name = "Category";
            this.Category.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.Automatic;
            // 
            // URL
            // 
            this.URL.HeaderText = "URL";
            this.URL.Name = "URL";
            this.URL.Width = 220;
            // 
            // Status
            // 
            this.Status.HeaderText = "Status";
            this.Status.Name = "Status";
            this.Status.Width = 61;
            // 
            // categoryDropDownList
            // 
            this.categoryDropDownList.FormattingEnabled = true;
            this.categoryDropDownList.Location = new System.Drawing.Point(96, 62);
            this.categoryDropDownList.Name = "categoryDropDownList";
            this.categoryDropDownList.Size = new System.Drawing.Size(131, 21);
            this.categoryDropDownList.TabIndex = 14;
            this.categoryDropDownList.SelectedIndexChanged += new System.EventHandler(this.categoryDropDownList_SelectedIndexChanged);
            this.categoryDropDownList.KeyDown += new System.Windows.Forms.KeyEventHandler(this.categoryDropDownList_KeyDown);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 65);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(77, 13);
            this.label1.TabIndex = 15;
            this.label1.Text = "Category Filter:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(233, 66);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(65, 13);
            this.label2.TabIndex = 16;
            this.label2.Text = "Status Filter:";
            // 
            // statusDropDownList
            // 
            this.statusDropDownList.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.statusDropDownList.FormattingEnabled = true;
            this.statusDropDownList.Items.AddRange(new object[] {
            "All",
            "Success",
            "None",
            "Error",
            "Warning",
            "Failed"});
            this.statusDropDownList.Location = new System.Drawing.Point(301, 62);
            this.statusDropDownList.Name = "statusDropDownList";
            this.statusDropDownList.Size = new System.Drawing.Size(103, 21);
            this.statusDropDownList.TabIndex = 17;
            this.statusDropDownList.SelectedIndexChanged += new System.EventHandler(this.statusDropDownList_SelectedIndexChanged);
            // 
            // comboBoxPortNumber
            // 
            this.comboBoxPortNumber.FormattingEnabled = true;
            this.comboBoxPortNumber.Location = new System.Drawing.Point(485, 36);
            this.comboBoxPortNumber.Name = "comboBoxPortNumber";
            this.comboBoxPortNumber.Size = new System.Drawing.Size(63, 21);
            this.comboBoxPortNumber.TabIndex = 8;
            this.comboBoxPortNumber.TextChanged += new System.EventHandler(this.comboBoxPortNumber_TextChanged);
            // 
            // comboBoxServerName
            // 
            this.comboBoxServerName.FormattingEnabled = true;
            this.comboBoxServerName.Location = new System.Drawing.Point(250, 36);
            this.comboBoxServerName.Name = "comboBoxServerName";
            this.comboBoxServerName.Size = new System.Drawing.Size(154, 21);
            this.comboBoxServerName.TabIndex = 6;
            this.comboBoxServerName.TextChanged += new System.EventHandler(this.comboBoxServerName_TextChanged);
            // 
            // comboBoxProtocol
            // 
            this.comboBoxProtocol.FormattingEnabled = true;
            this.comboBoxProtocol.Location = new System.Drawing.Point(64, 36);
            this.comboBoxProtocol.Name = "comboBoxProtocol";
            this.comboBoxProtocol.Size = new System.Drawing.Size(102, 21);
            this.comboBoxProtocol.TabIndex = 4;
            this.comboBoxProtocol.TextChanged += new System.EventHandler(this.comboBoxProtocol_TextChanged);
            // 
            // progressBarTest
            // 
            this.progressBarTest.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.progressBarTest.Location = new System.Drawing.Point(359, 660);
            this.progressBarTest.Name = "progressBarTest";
            this.progressBarTest.Size = new System.Drawing.Size(420, 18);
            this.progressBarTest.TabIndex = 18;
            this.progressBarTest.Visible = false;
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(950, 680);
            this.Controls.Add(this.progressBarTest);
            this.Controls.Add(this.statusDropDownList);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.categoryDropDownList);
            this.Controls.Add(this.requestDataGridView);
            this.Controls.Add(this.StopButton);
            this.Controls.Add(this.panel1);
            this.Controls.Add(this.PlayButton);
            this.Controls.Add(this.WmsSelectButton);
            this.Controls.Add(this.comboBoxPortNumber);
            this.Controls.Add(this.PortNumberLabel);
            this.Controls.Add(this.comboBoxServerName);
            this.Controls.Add(this.ServerNameLabel);
            this.Controls.Add(this.comboBoxProtocol);
            this.Controls.Add(this.ProtocolLabel);
            this.Controls.Add(this.toolStrip1);
            this.Controls.Add(this.statusStrip1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MinimumSize = new System.Drawing.Size(700, 375);
            this.Name = "MainForm";
            this.Text = "Geo Web Publisher - Unit Testing Application";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainForm_FormClosing);
            this.Shown += new System.EventHandler(this.mainForm_Shown);
            this.LocationChanged += new System.EventHandler(this.mainForm_LocationChanged);
            this.SizeChanged += new System.EventHandler(this.mainForm_SizeChanged);
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            this.toolStrip1.ResumeLayout(false);
            this.toolStrip1.PerformLayout();
            this.panel1.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.requestDataGridView)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.ToolStrip toolStrip1;
        private System.Windows.Forms.Label ProtocolLabel;
        private System.Windows.Forms.ComboBox comboBoxProtocol;
        private System.Windows.Forms.ComboBox comboBoxServerName;
        private System.Windows.Forms.Label ServerNameLabel;
        private System.Windows.Forms.ComboBox comboBoxPortNumber;
        private System.Windows.Forms.Label PortNumberLabel;
        private System.Windows.Forms.Button WmsSelectButton;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Button CopyUrlToClipboard;
        private System.Windows.Forms.Button ResultComparisonButton;
        private System.Windows.Forms.Button DeleteButton;
        private System.Windows.Forms.Button EditButton;
        private System.Windows.Forms.Button StopButton;
        private System.Windows.Forms.Button PlayButton;
        private System.Windows.Forms.DataGridView requestDataGridView;
        private System.Windows.Forms.ComboBox categoryDropDownList;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ComboBox statusDropDownList;
        private System.Windows.Forms.ToolTip Tooltip;
        private System.Windows.Forms.ProgressBar progressBarTest;
        private System.Windows.Forms.Button updateAllButton;
        private System.Windows.Forms.Button updateButton;
        private System.Windows.Forms.DataGridViewCheckBoxColumn ExecuteTest;
        private System.Windows.Forms.DataGridViewTextBoxColumn ID;
        private System.Windows.Forms.DataGridViewImageColumn UpToDate;
        private System.Windows.Forms.DataGridViewTextBoxColumn NameGrid;
        private System.Windows.Forms.DataGridViewTextBoxColumn Description;
        private System.Windows.Forms.DataGridViewComboBoxColumn Category;
        private System.Windows.Forms.DataGridViewTextBoxColumn URL;
        private System.Windows.Forms.DataGridViewTextBoxColumn Status;
        private System.Windows.Forms.ToolStripButton OptionsButton;
        private System.Windows.Forms.ToolStripButton deleteResultsButton;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel1;
    }
}

