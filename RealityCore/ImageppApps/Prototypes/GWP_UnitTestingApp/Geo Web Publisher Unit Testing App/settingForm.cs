/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/GWP_UnitTestingApp/Geo Web Publisher Unit Testing App/settingForm.cs $
|    $RCSfile: SettingForm.cs, $
|   $Revision: 1 $
|       $Date: 2013/05/27 $
|     $Author: Julien Rossignol $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System;
using System.Diagnostics;
using System.Drawing;
using System.Windows.Forms;

namespace Geo_Web_Publisher_Unit_Testing_App
{
    /// <summary>Manage the setting form</summary>
    /// <author>Julien Rossignol</author>
    public partial class SettingForm : Form
    {

        #region fields

        private int m_RenamingIndex; // index of the category being renamed
        private bool m_IsRenaming; // tells if a category is getting ranamed

        #endregion

        #region constructors

        /*------------------------------------------------------------------------------------**/
        /// <summary>Initialize form components and MRU list</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public SettingForm()
        {
            InitializeComponent();
            testRequestTextBox.Text = Properties.GeoWebPublisherUnitTestingApp.Default.TestRequest;
            databaseTextBox.Text = Properties.GeoWebPublisherUnitTestingApp.Default.DatabaseName;
            LoadCategories();
            backgroundPictureBox.Image = GetBitmapWithColor(Properties.GeoWebPublisherUnitTestingApp.Default.BackgroundColor);
            smallDifferencePictureBox.Image =GetBitmapWithColor(Properties.GeoWebPublisherUnitTestingApp.Default.SmallDifferenceColor);
            bigDifferencePictureBox.Image = GetBitmapWithColor(Properties.GeoWebPublisherUnitTestingApp.Default.BigDifferenceColor);
        }

        #endregion

        #region methods

        #region methods/database

        /*------------------------------------------------------------------------------------**/
        /// <summary>Ask for a new database and reload new informations when choosedatabase button is clicked</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void chooseDatabaseButton_Click(object sender , EventArgs e)
        {
            openDatabaseDialog.ShowDialog();
            if( openDatabaseDialog.FileName != "" )
            {
                if( openDatabaseDialog.FileName != Properties.GeoWebPublisherUnitTestingApp.Default.DatabaseName )//if the user choosed a different database
                {
                    Properties.GeoWebPublisherUnitTestingApp.Default.DatabaseName = openDatabaseDialog.FileName;
                    Properties.GeoWebPublisherUnitTestingApp.Default.Save();
                    Program.DBManager.ConnectionToDatabase();
                    databaseTextBox.Text = openDatabaseDialog.FileName;
                    Program.LoadDatabaseInfo(); //reload all informations
                }
            }
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Create a new database</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void createNewDatabaseButton_Click(object sender , EventArgs e)
        {
            CreateNewDatabase();
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Create a new database and load it</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void CreateNewDatabase()
        {
            createNewDatabaseDialog.ShowDialog(); //ask for a filename 
            if( createNewDatabaseDialog.FileName != "" )
            {
                try
                {
                    if( createNewDatabaseDialog.FileName.EndsWith(".db") ) //check if it is a database file
                    {
                        if( System.IO.File.Exists(createNewDatabaseDialog.FileName) ) //check if it already exists
                        {
                            MessageBox.Show("Cannot overwrite an existing database, please enter a new file name" , "Database already exists" , MessageBoxButtons.OK , MessageBoxIcon.Asterisk);
                        }
                        else
                        {
                            //execute the batch file to crea te new database
                            Process process = new Process();
                            process.StartInfo.WorkingDirectory = string.Format("..\\..\\SqLite");
                            process.StartInfo.FileName = "database.bat";
                            process.StartInfo.Arguments = string.Format(createNewDatabaseDialog.FileName.Insert(0 , "\"") + "\"");
                            process.StartInfo.CreateNoWindow = true;
                            process.Start();
                            process.WaitForExit();
                            databaseTextBox.Text = createNewDatabaseDialog.FileName;
                            //change database info and reload it;
                            Properties.GeoWebPublisherUnitTestingApp.Default.DatabaseName = createNewDatabaseDialog.FileName;
                            Properties.GeoWebPublisherUnitTestingApp.Default.Save();
                            Program.DBManager.ConnectionToDatabase();
                            Program.LoadDatabaseInfo();
                        }
                    }
                    else
                    {
                        DialogResult result = MessageBox.Show("Invalid filename, please enter a valid filename" , "Invalid filename" , MessageBoxButtons.OK , MessageBoxIcon.Exclamation);
                        CreateNewDatabase();
                    }
                }
                catch
                {
                }
            }
        }

        #endregion

        #region methods/category

        /*------------------------------------------------------------------------------------**/
        /// <summary>Load new categories into the list view</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public void LoadCategories()
        {
            foreach( Category category in Program.CategoryList )
            {
                CategoryListView.Items.Add(category.Name);
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Add the category to the list or rename it</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void AddCategoryButton_Click(object sender , EventArgs e)
        {
            if( CategoryNameTextBox.Text != "" )
            {
                Program.MainForm.EndEdit();
                if( m_IsRenaming )
                {
                    Program.DBManager.UpdateCategoryList(m_RenamingIndex , CategoryNameTextBox.Text); //update category in database
                    CategoryListView.Items[m_RenamingIndex].Text = CategoryNameTextBox.Text;
                    m_IsRenaming = false;
                    AddCategoryButton.Text = "Add category";
                    CategoryNameTextBox.Text = "";
                    m_RenamingIndex = -1;

                }
                else
                {
                    Program.DBManager.InsertCategory(CategoryNameTextBox.Text); //insert category in database
                    CategoryListView.Items.Add(CategoryNameTextBox.Text);
                    CategoryNameTextBox.Text = "";
                }
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>take the selected category and place it in the rename textobx</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void renameButton_Click(object sender , EventArgs e)
        {
            AddCategoryButton.Text = "Save change";
            m_RenamingIndex = CategoryListView.SelectedIndices[0];
            m_IsRenaming = true;
            CategoryNameTextBox.Text = CategoryListView.SelectedItems[0].Text;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Delete selected categories</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void deleteCategoryButton_Click(object sender , EventArgs e)
        {
            if( m_IsRenaming )
            {
                m_IsRenaming = false;
                m_RenamingIndex = -1;
                CategoryNameTextBox.Text = "";
                AddCategoryButton.Text = "Add category";
            }

            DialogResult result = MessageBox.Show("Are you sure your want to delete this/these category(ies)?" , "Warning" , MessageBoxButtons.OKCancel , MessageBoxIcon.Exclamation);
            if( result == DialogResult.OK )
            {
                foreach( int index in CategoryListView.SelectedIndices )
                {
                    Program.DBManager.DeleteCategory(index);
                    CategoryListView.Items[CategoryListView.SelectedIndices[0]].Remove();
                }
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Decide which action can be done on the selected categories</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void CategoryListView_SelectedIndexChanged(object sender , EventArgs e)
        {

            if( CategoryListView.SelectedIndices.Count == 1 )
            {
                renameButton.Enabled = true;
                deleteCategoryButton.Enabled = true;
            }
            else if( CategoryListView.SelectedIndices.Count > 1 )
            {
                renameButton.Enabled = false;
                deleteCategoryButton.Enabled = true;
            }
            else
            {
                renameButton.Enabled = false;
                deleteCategoryButton.Enabled = false;
            }

            if( m_IsRenaming )
            {
                m_IsRenaming = false;
                m_RenamingIndex = -1;
                CategoryNameTextBox.Text = "";
                AddCategoryButton.Text = "Add category";
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Allow renaming of a category by double clicking</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void CategoryListView_MouseDoubleClick(object sender , MouseEventArgs e)
        {
            if( this.CategoryListView.SelectedIndices.Count == 1 )
            {
                AddCategoryButton.Text = "Save change";
                m_RenamingIndex = CategoryListView.SelectedIndices[0];
                m_IsRenaming = true;
                CategoryNameTextBox.Text = CategoryListView.SelectedItems[0].Text;
            }
        }

        #endregion

        #region methods/colorSelection

        /*------------------------------------------------------------------------------------**/
        /// <summary>Allow the user to chosse the small difference color</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void smallDifferenceButton_Click(object sender , EventArgs e)
        {
            ChooseSmallDifferenceColor();
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Allow the user to chosse the big difference color</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void bigDifferenceButton_Click(object sender , EventArgs e)
        {
            ChooseBigDifferenceColor();
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Allow the user to chosse the background difference color</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void backgroundColorButton_Click(object sender , EventArgs e)
        {
            ChooseBackgroundColor();
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Return a little bitmap filled with the specified color</summary>
        /// <param name="newColor">Color to fill the bitmap with</param>
        /// <returns>Bitmap filled with the color</returns>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private Bitmap GetBitmapWithColor(Color newColor)
        {
            Bitmap newBitmap = new Bitmap(251 , 71);
            for( int i = 0 ; i < newBitmap.Width ; i++ )
            {
                for( int j = 0 ; j < newBitmap.Height ; j++ )
                {
                    newBitmap.SetPixel(i , j , newColor);
                }
            }
            return newBitmap;
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Allow the user to chosse the small difference color</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void smallDifferencePictureBox_Click(object sender , EventArgs e)
        {
            ChooseSmallDifferenceColor();
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Allow the user to chosse the big difference color</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void bigDifferencePictureBox_Click(object sender , EventArgs e)
        {
            ChooseBigDifferenceColor();
        }
        /// <summary>Allow the user to chosse the background difference color</summary>   
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void backgroundPictureBox_Click(object sender , EventArgs e)
        {
            ChooseBackgroundColor();
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Open the background color selection dialog and process the choice</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void ChooseBackgroundColor()
        {
            DialogResult result = backgroundColorDialog.ShowDialog();
            if( result == System.Windows.Forms.DialogResult.OK )
            {
                Properties.GeoWebPublisherUnitTestingApp.Default.BackgroundColor = backgroundColorDialog.Color;
                Properties.GeoWebPublisherUnitTestingApp.Default.Save();
                backgroundPictureBox.Image = GetBitmapWithColor(backgroundColorDialog.Color);
                Program.ColorSettingsHasChanged = true;
            }
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Open the small difference color selection dialog and process the choice</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void ChooseSmallDifferenceColor()
        {
            DialogResult result = smallDifferenceColorDialog.ShowDialog();
            if( result == System.Windows.Forms.DialogResult.OK )
            {
                Properties.GeoWebPublisherUnitTestingApp.Default.SmallDifferenceColor = smallDifferenceColorDialog.Color;
                Properties.GeoWebPublisherUnitTestingApp.Default.Save();
                smallDifferencePictureBox.Image = GetBitmapWithColor(smallDifferenceColorDialog.Color);
                Program.ColorSettingsHasChanged = true;
            }
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Open the big difference color selection dialog and process the choice</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void ChooseBigDifferenceColor()
        {
            DialogResult result =  bigDifferenceColorDialog.ShowDialog();
            if( result == System.Windows.Forms.DialogResult.OK )
            {
                Properties.GeoWebPublisherUnitTestingApp.Default.BigDifferenceColor = bigDifferenceColorDialog.Color;
                Properties.GeoWebPublisherUnitTestingApp.Default.Save();
                bigDifferencePictureBox.Image = GetBitmapWithColor(bigDifferenceColorDialog.Color);
                Program.ColorSettingsHasChanged = true;
            }
        }

        #endregion

        #region methods/others

        /*------------------------------------------------------------------------------------**/
        /// <summary>hide form</summary>  
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void closeButton_Click(object sender , EventArgs e)
        {
            this.Hide();
        }
      
        /*------------------------------------------------------------------------------------**/
        /// <summary>Change the test request propertie to fit changes in the text box</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void testRequestTextBox_TextChanged(object sender , EventArgs e)
        {
            Properties.GeoWebPublisherUnitTestingApp.Default.TestRequest = testRequestTextBox.Text;
            Properties.GeoWebPublisherUnitTestingApp.Default.Save();
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Change the autocheckupdate propertie to fit the check box</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void autoFetchUpdate_CheckedChanged(object sender , EventArgs e)
        {
            Properties.GeoWebPublisherUnitTestingApp.Default.AutoCheckUpdate = autoFetchUpdate.Checked;
            Properties.GeoWebPublisherUnitTestingApp.Default.Save();
            if( autoFetchUpdate.Checked )
                Program.MainForm.UpdateAllRecord();
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Change the update server info propertie to fit the change in the text box</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void serverInfoTextBox_TextChanged(object sender , EventArgs e)
        {
            Properties.GeoWebPublisherUnitTestingApp.Default.UpdateServerInfo = serverInfoTextBox.Text;
            Properties.GeoWebPublisherUnitTestingApp.Default.Save();
        }

        #endregion

        #endregion
    }
}
