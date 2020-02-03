namespace Geo_Web_Publisher_Unit_Testing_App
{
    partial class SettingForm
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(SettingForm));
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.renameButton = new System.Windows.Forms.Button();
            this.deleteCategoryButton = new System.Windows.Forms.Button();
            this.AddCategoryButton = new System.Windows.Forms.Button();
            this.CategoryNameTextBox = new System.Windows.Forms.TextBox();
            this.CategoryListView = new System.Windows.Forms.ListView();
            this.closeButton = new System.Windows.Forms.Button();
            this.outputFolderBrowser = new System.Windows.Forms.FolderBrowserDialog();
            this.comparisonFolderBrowser = new System.Windows.Forms.FolderBrowserDialog();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.databaseTextBox = new System.Windows.Forms.TextBox();
            this.createNewDatabaseButton = new System.Windows.Forms.Button();
            this.chooseDatabaseButton = new System.Windows.Forms.Button();
            this.createNewDatabaseDialog = new System.Windows.Forms.SaveFileDialog();
            this.openDatabaseDialog = new System.Windows.Forms.OpenFileDialog();
            this.differencePixelColoPickerGroupBox = new System.Windows.Forms.GroupBox();
            this.label3 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.backgroundColorButton = new System.Windows.Forms.Button();
            this.backgroundPictureBox = new System.Windows.Forms.PictureBox();
            this.bigDifferencePictureBox = new System.Windows.Forms.PictureBox();
            this.smallDifferencePictureBox = new System.Windows.Forms.PictureBox();
            this.smallDifferenceButton = new System.Windows.Forms.Button();
            this.bigDifferenceButton = new System.Windows.Forms.Button();
            this.bigDifferenceColorDialog = new System.Windows.Forms.ColorDialog();
            this.smallDifferenceColorDialog = new System.Windows.Forms.ColorDialog();
            this.backgroundColorDialog = new System.Windows.Forms.ColorDialog();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.testRequestTextBox = new System.Windows.Forms.TextBox();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.serverInfoTextBox = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.autoFetchUpdate = new System.Windows.Forms.CheckBox();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.groupBox3.SuspendLayout();
            this.groupBox4.SuspendLayout();
            this.differencePixelColoPickerGroupBox.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.backgroundPictureBox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.bigDifferencePictureBox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.smallDifferencePictureBox)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.renameButton);
            this.groupBox3.Controls.Add(this.deleteCategoryButton);
            this.groupBox3.Controls.Add(this.AddCategoryButton);
            this.groupBox3.Controls.Add(this.CategoryNameTextBox);
            this.groupBox3.Controls.Add(this.CategoryListView);
            this.groupBox3.Location = new System.Drawing.Point(13, 351);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(430, 196);
            this.groupBox3.TabIndex = 4;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Categories";
            // 
            // renameButton
            // 
            this.renameButton.Enabled = false;
            this.renameButton.Location = new System.Drawing.Point(236, 131);
            this.renameButton.Name = "renameButton";
            this.renameButton.Size = new System.Drawing.Size(191, 25);
            this.renameButton.TabIndex = 4;
            this.renameButton.Text = "Rename Category";
            this.toolTip1.SetToolTip(this.renameButton, "Rename selected category");
            this.renameButton.UseVisualStyleBackColor = true;
            this.renameButton.Click += new System.EventHandler(this.renameButton_Click);
            // 
            // deleteCategoryButton
            // 
            this.deleteCategoryButton.Enabled = false;
            this.deleteCategoryButton.Location = new System.Drawing.Point(236, 162);
            this.deleteCategoryButton.Name = "deleteCategoryButton";
            this.deleteCategoryButton.Size = new System.Drawing.Size(191, 26);
            this.deleteCategoryButton.TabIndex = 3;
            this.deleteCategoryButton.Text = "Delete Category";
            this.toolTip1.SetToolTip(this.deleteCategoryButton, "Delete selected category(ies)");
            this.deleteCategoryButton.UseVisualStyleBackColor = true;
            this.deleteCategoryButton.Click += new System.EventHandler(this.deleteCategoryButton_Click);
            // 
            // AddCategoryButton
            // 
            this.AddCategoryButton.Location = new System.Drawing.Point(236, 45);
            this.AddCategoryButton.Name = "AddCategoryButton";
            this.AddCategoryButton.Size = new System.Drawing.Size(191, 27);
            this.AddCategoryButton.TabIndex = 2;
            this.AddCategoryButton.Text = "Add Category";
            this.toolTip1.SetToolTip(this.AddCategoryButton, "Save change in database");
            this.AddCategoryButton.UseVisualStyleBackColor = true;
            this.AddCategoryButton.Click += new System.EventHandler(this.AddCategoryButton_Click);
            // 
            // CategoryNameTextBox
            // 
            this.CategoryNameTextBox.Location = new System.Drawing.Point(236, 19);
            this.CategoryNameTextBox.MaxLength = 50;
            this.CategoryNameTextBox.Name = "CategoryNameTextBox";
            this.CategoryNameTextBox.Size = new System.Drawing.Size(186, 20);
            this.CategoryNameTextBox.TabIndex = 1;
            // 
            // CategoryListView
            // 
            this.CategoryListView.Location = new System.Drawing.Point(7, 19);
            this.CategoryListView.Name = "CategoryListView";
            this.CategoryListView.Size = new System.Drawing.Size(217, 170);
            this.CategoryListView.TabIndex = 0;
            this.CategoryListView.UseCompatibleStateImageBehavior = false;
            this.CategoryListView.View = System.Windows.Forms.View.List;
            this.CategoryListView.SelectedIndexChanged += new System.EventHandler(this.CategoryListView_SelectedIndexChanged);
            this.CategoryListView.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.CategoryListView_MouseDoubleClick);
            // 
            // closeButton
            // 
            this.closeButton.Location = new System.Drawing.Point(249, 558);
            this.closeButton.Name = "closeButton";
            this.closeButton.Size = new System.Drawing.Size(190, 24);
            this.closeButton.TabIndex = 5;
            this.closeButton.Text = "Close";
            this.closeButton.UseVisualStyleBackColor = true;
            this.closeButton.Click += new System.EventHandler(this.closeButton_Click);
            // 
            // outputFolderBrowser
            // 
            this.outputFolderBrowser.Description = "Select output folder";
            // 
            // comparisonFolderBrowser
            // 
            this.comparisonFolderBrowser.Description = "Select folder for baseline comparison";
            // 
            // groupBox4
            // 
            this.groupBox4.Controls.Add(this.databaseTextBox);
            this.groupBox4.Controls.Add(this.createNewDatabaseButton);
            this.groupBox4.Controls.Add(this.chooseDatabaseButton);
            this.groupBox4.Location = new System.Drawing.Point(13, 258);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Size = new System.Drawing.Size(431, 87);
            this.groupBox4.TabIndex = 6;
            this.groupBox4.TabStop = false;
            this.groupBox4.Text = "Database";
            // 
            // databaseTextBox
            // 
            this.databaseTextBox.Location = new System.Drawing.Point(7, 22);
            this.databaseTextBox.Name = "databaseTextBox";
            this.databaseTextBox.ReadOnly = true;
            this.databaseTextBox.Size = new System.Drawing.Size(393, 20);
            this.databaseTextBox.TabIndex = 10;
            // 
            // createNewDatabaseButton
            // 
            this.createNewDatabaseButton.Location = new System.Drawing.Point(242, 48);
            this.createNewDatabaseButton.Name = "createNewDatabaseButton";
            this.createNewDatabaseButton.Size = new System.Drawing.Size(185, 27);
            this.createNewDatabaseButton.TabIndex = 9;
            this.createNewDatabaseButton.Text = "Create Database";
            this.toolTip1.SetToolTip(this.createNewDatabaseButton, "Create new database");
            this.createNewDatabaseButton.UseVisualStyleBackColor = true;
            this.createNewDatabaseButton.Click += new System.EventHandler(this.createNewDatabaseButton_Click);
            // 
            // chooseDatabaseButton
            // 
            this.chooseDatabaseButton.Image = global::Geo_Web_Publisher_Unit_Testing_App.Properties.Resources.open_folder;
            this.chooseDatabaseButton.Location = new System.Drawing.Point(403, 20);
            this.chooseDatabaseButton.Name = "chooseDatabaseButton";
            this.chooseDatabaseButton.Size = new System.Drawing.Size(23, 23);
            this.chooseDatabaseButton.TabIndex = 8;
            this.toolTip1.SetToolTip(this.chooseDatabaseButton, "Choose database");
            this.chooseDatabaseButton.UseVisualStyleBackColor = true;
            this.chooseDatabaseButton.Click += new System.EventHandler(this.chooseDatabaseButton_Click);
            // 
            // createNewDatabaseDialog
            // 
            this.createNewDatabaseDialog.Filter = "Database file| *.db";
            this.createNewDatabaseDialog.OverwritePrompt = false;
            this.createNewDatabaseDialog.Title = "Database save";
            // 
            // openDatabaseDialog
            // 
            this.openDatabaseDialog.Filter = "Database file| *.db";
            this.openDatabaseDialog.Title = "Open database";
            // 
            // differencePixelColoPickerGroupBox
            // 
            this.differencePixelColoPickerGroupBox.Controls.Add(this.label3);
            this.differencePixelColoPickerGroupBox.Controls.Add(this.label2);
            this.differencePixelColoPickerGroupBox.Controls.Add(this.label1);
            this.differencePixelColoPickerGroupBox.Controls.Add(this.backgroundColorButton);
            this.differencePixelColoPickerGroupBox.Controls.Add(this.backgroundPictureBox);
            this.differencePixelColoPickerGroupBox.Controls.Add(this.bigDifferencePictureBox);
            this.differencePixelColoPickerGroupBox.Controls.Add(this.smallDifferencePictureBox);
            this.differencePixelColoPickerGroupBox.Controls.Add(this.smallDifferenceButton);
            this.differencePixelColoPickerGroupBox.Controls.Add(this.bigDifferenceButton);
            this.differencePixelColoPickerGroupBox.Location = new System.Drawing.Point(13, 5);
            this.differencePixelColoPickerGroupBox.Name = "differencePixelColoPickerGroupBox";
            this.differencePixelColoPickerGroupBox.Size = new System.Drawing.Size(431, 104);
            this.differencePixelColoPickerGroupBox.TabIndex = 7;
            this.differencePixelColoPickerGroupBox.TabStop = false;
            this.differencePixelColoPickerGroupBox.Text = "Different pixel color";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(17, 75);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(68, 13);
            this.label3.TabIndex = 8;
            this.label3.Text = "Background:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(17, 47);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(135, 13);
            this.label2.TabIndex = 7;
            this.label2.Text = "Pixels with large difference:";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(17, 19);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(135, 13);
            this.label1.TabIndex = 6;
            this.label1.Text = "Pixels with small difference:";
            // 
            // backgroundColorButton
            // 
            this.backgroundColorButton.Location = new System.Drawing.Point(178, 71);
            this.backgroundColorButton.Name = "backgroundColorButton";
            this.backgroundColorButton.Size = new System.Drawing.Size(67, 21);
            this.backgroundColorButton.TabIndex = 5;
            this.backgroundColorButton.Text = "Pick Color";
            this.toolTip1.SetToolTip(this.backgroundColorButton, "Pick background color");
            this.backgroundColorButton.UseVisualStyleBackColor = true;
            this.backgroundColorButton.Click += new System.EventHandler(this.backgroundColorButton_Click);
            // 
            // backgroundPictureBox
            // 
            this.backgroundPictureBox.Location = new System.Drawing.Point(251, 71);
            this.backgroundPictureBox.Name = "backgroundPictureBox";
            this.backgroundPictureBox.Size = new System.Drawing.Size(172, 22);
            this.backgroundPictureBox.TabIndex = 4;
            this.backgroundPictureBox.TabStop = false;
            this.backgroundPictureBox.Click += new System.EventHandler(this.backgroundPictureBox_Click);
            // 
            // bigDifferencePictureBox
            // 
            this.bigDifferencePictureBox.Location = new System.Drawing.Point(251, 43);
            this.bigDifferencePictureBox.Name = "bigDifferencePictureBox";
            this.bigDifferencePictureBox.Size = new System.Drawing.Size(172, 22);
            this.bigDifferencePictureBox.TabIndex = 3;
            this.bigDifferencePictureBox.TabStop = false;
            this.bigDifferencePictureBox.Click += new System.EventHandler(this.bigDifferencePictureBox_Click);
            // 
            // smallDifferencePictureBox
            // 
            this.smallDifferencePictureBox.Location = new System.Drawing.Point(251, 15);
            this.smallDifferencePictureBox.Name = "smallDifferencePictureBox";
            this.smallDifferencePictureBox.Size = new System.Drawing.Size(172, 22);
            this.smallDifferencePictureBox.TabIndex = 2;
            this.smallDifferencePictureBox.TabStop = false;
            this.smallDifferencePictureBox.Click += new System.EventHandler(this.smallDifferencePictureBox_Click);
            // 
            // smallDifferenceButton
            // 
            this.smallDifferenceButton.Location = new System.Drawing.Point(178, 15);
            this.smallDifferenceButton.Name = "smallDifferenceButton";
            this.smallDifferenceButton.Size = new System.Drawing.Size(67, 21);
            this.smallDifferenceButton.TabIndex = 0;
            this.smallDifferenceButton.Text = "Pick Color";
            this.toolTip1.SetToolTip(this.smallDifferenceButton, "Pick small difference pixel color");
            this.smallDifferenceButton.UseVisualStyleBackColor = true;
            this.smallDifferenceButton.Click += new System.EventHandler(this.smallDifferenceButton_Click);
            // 
            // bigDifferenceButton
            // 
            this.bigDifferenceButton.Location = new System.Drawing.Point(178, 43);
            this.bigDifferenceButton.Name = "bigDifferenceButton";
            this.bigDifferenceButton.Size = new System.Drawing.Size(67, 21);
            this.bigDifferenceButton.TabIndex = 1;
            this.bigDifferenceButton.Text = "Pick Color";
            this.toolTip1.SetToolTip(this.bigDifferenceButton, "Pick large difference pixel color");
            this.bigDifferenceButton.UseVisualStyleBackColor = true;
            this.bigDifferenceButton.Click += new System.EventHandler(this.bigDifferenceButton_Click);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.testRequestTextBox);
            this.groupBox1.Location = new System.Drawing.Point(13, 115);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(431, 49);
            this.groupBox1.TabIndex = 8;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Test request";
            // 
            // testRequestTextBox
            // 
            this.testRequestTextBox.Location = new System.Drawing.Point(6, 19);
            this.testRequestTextBox.Name = "testRequestTextBox";
            this.testRequestTextBox.Size = new System.Drawing.Size(420, 20);
            this.testRequestTextBox.TabIndex = 1;
            this.toolTip1.SetToolTip(this.testRequestTextBox, "Test request excecuted before a test to check if the server connection info is va" +
        "lid. Must be a valid getmap request\r\n");
            this.testRequestTextBox.TextChanged += new System.EventHandler(this.testRequestTextBox_TextChanged);
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.serverInfoTextBox);
            this.groupBox2.Controls.Add(this.label4);
            this.groupBox2.Controls.Add(this.autoFetchUpdate);
            this.groupBox2.Location = new System.Drawing.Point(13, 170);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(430, 82);
            this.groupBox2.TabIndex = 9;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Request update";
            // 
            // serverInfoTextBox
            // 
            this.serverInfoTextBox.Location = new System.Drawing.Point(74, 49);
            this.serverInfoTextBox.Name = "serverInfoTextBox";
            this.serverInfoTextBox.Size = new System.Drawing.Size(352, 20);
            this.serverInfoTextBox.TabIndex = 1;
            this.serverInfoTextBox.Text = "http://localhost:8087/";
            this.toolTip1.SetToolTip(this.serverInfoTextBox, "Server info for baseline update ");
            this.serverInfoTextBox.TextChanged += new System.EventHandler(this.serverInfoTextBox_TextChanged);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(7, 52);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(61, 13);
            this.label4.TabIndex = 1;
            this.label4.Text = "Server info:";
            // 
            // autoFetchUpdate
            // 
            this.autoFetchUpdate.AutoSize = true;
            this.autoFetchUpdate.Checked = true;
            this.autoFetchUpdate.CheckState = System.Windows.Forms.CheckState.Checked;
            this.autoFetchUpdate.Location = new System.Drawing.Point(10, 19);
            this.autoFetchUpdate.Name = "autoFetchUpdate";
            this.autoFetchUpdate.Size = new System.Drawing.Size(196, 17);
            this.autoFetchUpdate.TabIndex = 0;
            this.autoFetchUpdate.Text = "Automaticaly fetch update if needed";
            this.autoFetchUpdate.UseVisualStyleBackColor = true;
            this.autoFetchUpdate.CheckedChanged += new System.EventHandler(this.autoFetchUpdate_CheckedChanged);
            // 
            // SettingForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(457, 590);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.differencePixelColoPickerGroupBox);
            this.Controls.Add(this.groupBox4);
            this.Controls.Add(this.closeButton);
            this.Controls.Add(this.groupBox3);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MaximumSize = new System.Drawing.Size(473, 628);
            this.MinimizeBox = false;
            this.MinimumSize = new System.Drawing.Size(473, 539);
            this.Name = "SettingForm";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.Text = "Settings";
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            this.groupBox4.ResumeLayout(false);
            this.groupBox4.PerformLayout();
            this.differencePixelColoPickerGroupBox.ResumeLayout(false);
            this.differencePixelColoPickerGroupBox.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.backgroundPictureBox)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.bigDifferencePictureBox)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.smallDifferencePictureBox)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.Button renameButton;
        private System.Windows.Forms.Button deleteCategoryButton;
        private System.Windows.Forms.Button AddCategoryButton;
        private System.Windows.Forms.TextBox CategoryNameTextBox;
        private System.Windows.Forms.ListView CategoryListView;
        private System.Windows.Forms.Button closeButton;
        private System.Windows.Forms.FolderBrowserDialog outputFolderBrowser;
        private System.Windows.Forms.FolderBrowserDialog comparisonFolderBrowser;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.Button createNewDatabaseButton;
        private System.Windows.Forms.Button chooseDatabaseButton;
        private System.Windows.Forms.SaveFileDialog createNewDatabaseDialog;
        private System.Windows.Forms.OpenFileDialog openDatabaseDialog;
        private System.Windows.Forms.GroupBox differencePixelColoPickerGroupBox;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button backgroundColorButton;
        private System.Windows.Forms.PictureBox backgroundPictureBox;
        private System.Windows.Forms.PictureBox bigDifferencePictureBox;
        private System.Windows.Forms.PictureBox smallDifferencePictureBox;
        private System.Windows.Forms.Button smallDifferenceButton;
        private System.Windows.Forms.Button bigDifferenceButton;
        private System.Windows.Forms.ColorDialog bigDifferenceColorDialog;
        private System.Windows.Forms.ColorDialog smallDifferenceColorDialog;
        private System.Windows.Forms.ColorDialog backgroundColorDialog;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.TextBox testRequestTextBox;
        private System.Windows.Forms.TextBox databaseTextBox;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.CheckBox autoFetchUpdate;
        private System.Windows.Forms.TextBox serverInfoTextBox;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.ToolTip toolTip1;
    }
}