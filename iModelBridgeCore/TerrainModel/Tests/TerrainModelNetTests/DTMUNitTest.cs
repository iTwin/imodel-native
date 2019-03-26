using System;
using NUnit.Framework;

namespace Bentley.TerrainModelNET.NUnit
    {
    /// <summary>
    /// 
    /// </summary>
    public class DTMUNitTest
        {
        /// <summary>
        /// Enum to determine the level of the DTM Unit Test
        /// </summary>
        public enum DTM_UNIT_TEST_LEVEL
            {
            /// <summary>
            /// Default Level - Launch all tests
            /// </summary>
            DEFAULT_LEVEL = 0,
            /// <summary>
            /// Firebug Level - Use by the firebug machine to launch only "light" tests
            /// </summary>
            FIREBUG = 1 
            } ;

        /// <summary>
        /// Private property
        /// </summary>
        private DTM_UNIT_TEST_LEVEL m_level = DTM_UNIT_TEST_LEVEL.DEFAULT_LEVEL;

        /// <summary>
        /// Check if the DTM_NUNIT_TEST_LEVEL == FIREBUG.
        /// </summary>
        public void IgnoreFirebug ()
            {
            if (m_level == DTM_UNIT_TEST_LEVEL.FIREBUG)
                Assert.Ignore ("Not running on the firebug machine.");
            }

        /// <summary>
        /// Before All Test - Check the DTM Level
        /// </summary>
        [SetUp]
        public void DTM_Unit_Test_SetUp ()
            {
            try
                {
                String level = Environment.GetEnvironmentVariable ("DTM_NUNIT_TEST_LEVEL");
                if (level != null)
                    m_level = (DTM_UNIT_TEST_LEVEL)Convert.ToInt32 (level);
                }
            catch (Exception)
                {
                Assert.Ignore ("Cannot read the DTM_NUNIT_TEST_LEVEL");
                }
            }

        /// <summary>
        /// Property to know the level of the UnitTest
        /// </summary>
        public DTM_UNIT_TEST_LEVEL UnitTest_Level
            {
            get { return m_level; }
            set { m_level = value;  }
            }
        }
    }
