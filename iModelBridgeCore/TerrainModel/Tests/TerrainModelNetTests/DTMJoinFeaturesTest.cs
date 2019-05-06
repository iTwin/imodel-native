#region Using
using System;
using System.Text;
using System.Collections;
using System.IO;
using SD = System.Data;
using SDO = System.Data.OleDb;
using BGEO = Bentley.GeometryNET;
using NUnit.Framework;
#endregion using


namespace Bentley.TerrainModelNET.NUnit
{
    using Bentley.TerrainModelNET;

    /// <summary>
    /// DTMJoinFeaturesTest
    /// </summary>
    /// <category></category>
    [TestFixture]
    public class DTMJoinFeaturesTest
    {
        #region Private Fields
        private System.IFormatProvider _formatProvider = System.Globalization.NumberFormatInfo.CurrentInfo;
        #endregion Private Fields

        /// <summary>
        /// JoinFeatureTest
        /// </summary>
        /// <category></category>
        [Test]
        [Category ("DTMJoinFeatureTest")]
        public void JoinFeatureTest()
        {
            Console.WriteLine("-------------------\nJoinFeatureTest - NO TEST WRITTEN YET\n-------------------");
        }
    }
}
