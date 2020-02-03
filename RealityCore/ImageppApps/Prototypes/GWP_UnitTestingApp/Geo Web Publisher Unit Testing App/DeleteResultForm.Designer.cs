namespace Geo_Web_Publisher_Unit_Testing_App
{
    partial class DeleteResultForm
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
            if( disposing && ( components != null ) )
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(DeleteResultForm));
            this.deleteButton = new System.Windows.Forms.Button();
            this.TestDataGridView = new System.Windows.Forms.DataGridView();
            this.Id = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Datetime = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Description = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.FailedRequest = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.totalRequest = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            ((System.ComponentModel.ISupportInitialize)(this.TestDataGridView)).BeginInit();
            this.SuspendLayout();
            // 
            // deleteButton
            // 
            this.deleteButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.deleteButton.Location = new System.Drawing.Point(347, 386);
            this.deleteButton.Name = "deleteButton";
            this.deleteButton.Size = new System.Drawing.Size(182, 22);
            this.deleteButton.TabIndex = 1;
            this.deleteButton.Text = "Delete selection";
            this.toolTip1.SetToolTip(this.deleteButton, "Delete current selection");
            this.deleteButton.UseVisualStyleBackColor = true;
            this.deleteButton.Click += new System.EventHandler(this.deleteButton_Click);
            // 
            // TestDataGridView
            // 
            this.TestDataGridView.AllowUserToAddRows = false;
            this.TestDataGridView.AllowUserToDeleteRows = false;
            this.TestDataGridView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.TestDataGridView.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.TestDataGridView.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.Id,
            this.Datetime,
            this.Description,
            this.FailedRequest,
            this.totalRequest});
            this.TestDataGridView.Location = new System.Drawing.Point(12, 11);
            this.TestDataGridView.Name = "TestDataGridView";
            this.TestDataGridView.ReadOnly = true;
            this.TestDataGridView.Size = new System.Drawing.Size(516, 369);
            this.TestDataGridView.TabIndex = 2;
            this.TestDataGridView.SelectionChanged += new System.EventHandler(this.TestDataGridView_SelectionChanged);
            // 
            // Id
            // 
            this.Id.Frozen = true;
            this.Id.HeaderText = "Id";
            this.Id.Name = "Id";
            this.Id.ReadOnly = true;
            this.Id.Width = 40;
            // 
            // Datetime
            // 
            this.Datetime.HeaderText = "Datetime";
            this.Datetime.Name = "Datetime";
            this.Datetime.ReadOnly = true;
            this.Datetime.Width = 120;
            // 
            // Description
            // 
            this.Description.HeaderText = "Description";
            this.Description.Name = "Description";
            this.Description.ReadOnly = true;
            this.Description.Width = 212;
            // 
            // FailedRequest
            // 
            this.FailedRequest.HeaderText = "Failed Request";
            this.FailedRequest.Name = "FailedRequest";
            this.FailedRequest.ReadOnly = true;
            this.FailedRequest.Width = 50;
            // 
            // totalRequest
            // 
            this.totalRequest.HeaderText = "Total Request";
            this.totalRequest.Name = "totalRequest";
            this.totalRequest.ReadOnly = true;
            this.totalRequest.Width = 50;
            // 
            // DeleteResultForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(540, 415);
            this.Controls.Add(this.TestDataGridView);
            this.Controls.Add(this.deleteButton);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "DeleteResultForm";
            this.Text = "Delete result";
            ((System.ComponentModel.ISupportInitialize)(this.TestDataGridView)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button deleteButton;
        private System.Windows.Forms.DataGridView TestDataGridView;
        private System.Windows.Forms.DataGridViewTextBoxColumn Id;
        private System.Windows.Forms.DataGridViewTextBoxColumn Datetime;
        private System.Windows.Forms.DataGridViewTextBoxColumn Description;
        private System.Windows.Forms.DataGridViewTextBoxColumn FailedRequest;
        private System.Windows.Forms.DataGridViewTextBoxColumn totalRequest;
        private System.Windows.Forms.ToolTip toolTip1;
    }
}