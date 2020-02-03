/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/GWP_UnitTestingApp/Geo Web Publisher Unit Testing App/MessageTextBox.cs $
|    $RCSfile: MessageTextBox.cs, $
|   $Revision: 1 $
|       $Date: 2013/06/5 $
|     $Author: Julien Rossignol $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System;
using System.Windows.Forms;

namespace Geo_Web_Publisher_Unit_Testing_App
{
    /// <summary>Dialog asking for a textual input</summary>
    /// <author>Julien Rossignol</author>
    public partial class MessageTextBox : Form
    {
        #region fields

        private string m_Value; //value of the text box

        #endregion

        #region parameters

        public string Value
        {
            get { return m_Value; }
        }

        #endregion

        #region constructors
        /*------------------------------------------------------------------------------------**/
        /// <summary>Initialize windows form components</summary> 
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public MessageTextBox()
        {
            InitializeComponent();
        }
        #endregion

        #region methods

        /*------------------------------------------------------------------------------------**/
        /// <summary>Create the message box with the specified title and message</summary> 
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public MessageTextBox(string title , string message)  : this()
        {
            this.Text = title;
            this.label1.Text = message;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>return the dialog result "ok" if the ok button is pressed</summary> 
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void okButton_Click(object sender , EventArgs e)
        {
            m_Value = this.inputTextBox.Text;
            this.DialogResult = System.Windows.Forms.DialogResult.OK;
        }

        #endregion
    }
}
