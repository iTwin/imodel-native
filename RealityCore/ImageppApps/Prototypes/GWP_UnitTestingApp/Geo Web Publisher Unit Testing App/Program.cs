using System;
using System.Collections.Generic;
using System.Threading;
using System.Windows.Forms;

namespace Geo_Web_Publisher_Unit_Testing_App
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>

        #region fields

        //List with all result in the database
        private static volatile List<ResultRequest> m_ResultList;
        //list with all category in the database
        private static List<Category> m_CategoryList;
        //main form of the app, containing the datagridview
        private static MainForm m_MainForm;
        //indicate if the test is running of not
        private static volatile bool m_IsTestRunning;
        //class managing the execution of a test
        private static TestManager m_TestManager;
        //class managing all communications witht the database, making sure that every data is loaded correctly
        private static DatabaseManager m_DBManager;
        //indicates to the comparison form that he needs to recalculate difference bitmap
        private static bool m_ColorSettingsHasChanged;

        #endregion

        #region parameters

        public static List<ResultRequest> ResultList
        {
            get { return m_ResultList; }
            set { m_ResultList = value; }
        }

        public static List<Category> CategoryList
        {
            get { return m_CategoryList; }
            set { m_CategoryList = value; }
        }

        public static MainForm MainForm
        {
            get { return m_MainForm; }
        }

        public static TestManager TestManager
        {
            get { return m_TestManager; }
        }

        public static bool IsTestRunning
        {
            get { return m_IsTestRunning; }
            set
            {
                m_IsTestRunning = value;
                Program.MainForm.TestFinished();
            }
        }

        public static DatabaseManager DBManager
        {
            get { return m_DBManager; }
        }

        public static bool ColorSettingsHasChanged
        {
            get { return m_ColorSettingsHasChanged; }
            set { m_ColorSettingsHasChanged = value; }
        }

        #endregion

        #region methods

        /*------------------------------------------------------------------------------------**/
        /// <summary>Start the thread test and communicate the resultList to the test manager</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public static void StartTest()
        {
            m_IsTestRunning = true;
            Thread testThread = new Thread(TestManager.ExecuteTest); //configure the thread
            testThread.Priority = ThreadPriority.Highest; //makes sure the new test is prioritise
            testThread.Start(Program.m_ResultList); //start the thread 
        }

        [STAThread]
        static void Main()
        {
            m_ResultList = new List<ResultRequest>();
            m_CategoryList = new List<Category>();
            m_TestManager = new TestManager();
            m_DBManager = new DatabaseManager();
            LoadDatabaseInfo();
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            m_MainForm = new MainForm();
            Application.Run(m_MainForm);
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Initilialize and load database data, allowing to change the database while the app is running</summary>  
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public static void LoadDatabaseInfo()
        {
            if( m_MainForm !=null )
                m_MainForm.EmptyGridView();
            m_ResultList.Clear();
            m_CategoryList.Clear();
            m_DBManager.InitializeDatabase(); //makes sure there is a database
            m_DBManager.LoadAllRecord();
            m_DBManager.LoadCategories();
            if( m_MainForm != null )
            {
                m_MainForm.InitializePrefix();
            }

        }

        #endregion
    }
}
