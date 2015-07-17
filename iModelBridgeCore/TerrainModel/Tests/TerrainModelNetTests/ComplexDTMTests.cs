using System;
using System.IO;
using System.Text;
using BGEO = Bentley.GeometryNET;
using Bentley.TerrainModelNET;
using NUnit.Framework;


namespace Bentley.TerrainModelNET.NUnit
    {
    using Bentley.TerrainModelNET;
    //public class TestDTMProvider : IDTMProvider, IDisposable
    //    {
    //    private System.Collections.Generic.Dictionary<string, CommonDTM> m_dtms = new System.Collections.Generic.Dictionary<string, CommonDTM>();

    //    #region IDTMProvider Members

    //    public CommonDTM GetDTMFromId(string id, CommonDTM parentDTM)
    //        {
    //        return m_dtms[id];
    //        }

    //    public System.Collections.Generic.IEnumerable<string> GetListOfDTMs()
    //        {
    //        return m_dtms.Keys;
    //        }

    //    #endregion

    //    public void AddDTM(CommonDTM dtm)
    //        {
    //        string id = Guid.NewGuid().ToString();
    //        m_dtms.Add(id, dtm);
    //        dtm.SetID(id);
    //        }

    //    #region IDisposable Members

    //    public void Dispose()
    //        {
    //        CleanDTMS();
    //        }

    //    #endregion
    //    public void CleanDTMS()
    //        {
    //        foreach (CommonDTM dtm in m_dtms.Values)
    //            {
    //            dtm.Dispose();
    //            }
    //        m_dtms.Clear();
    //        }
    //    }

    //[TestFixture]
    //public class ComplexDTMTests : DTMUNitTest
    //    {
    //    static String DataDirectory = @"Bentley.Civil.Dtm.NUnit.dll\DTMMergeTest\";
    //    static String DataPath = Helper.GetTestDataLocation () + DataDirectory;

    //    /// <summary>
    //    /// DTMDrapeTest
    //    /// </summary>
    //    /// <returns></returns>
    //    [Test]
    //    [Category("ComplexDTMTests")]
    //    public void ComplexDTMContour()
    //        {
    //        TestDTMProvider provider = new TestDTMProvider();

    //        DTMProviderManager.Instance.RegisterProvider(provider);

    //        // Check Dtm Test Level
    //        Console.WriteLine("DTM_NUNIT_TEST_LEVEL = {0}", this.UnitTest_Level);
    //        IgnoreFirebug(); // If DTM_LEVEL = FIREBUG -> Ignore the test

    //        // Check Drape Test Data Directory Exists

    //        DirectoryInfo testFolder = new DirectoryInfo(DataPath);
    //        if (!testFolder.Exists)
    //            {
    //            throw new DirectoryNotFoundException(" DTM Drape Test Data Folder Not Found + DataPath");
    //            }

    //        // Scan All The Drape Test Data Directories

    //        foreach (DirectoryInfo nextFolder in testFolder.GetDirectories())
    //            {
    //            Console.WriteLine("Test Data Folder = {0}", nextFolder.Name);
    //            ComplexDTM complexDTM = new ComplexDTM();

    //            if (string.Compare(nextFolder.Name, "CVS") != 0)
    //                {
    //                // Check Drape Line File Exists
    //                foreach (FileInfo nextFile in nextFolder.GetFiles("*.tin"))
    //                    {
    //                    DTM drapeDTM = DTM.CreateFromFile(nextFile.FullName);

    //                    provider.AddDTM(drapeDTM);
    //                    complexDTM.AddDTM(drapeDTM);
    //                    }

    //                ContoursBrowsingCriteria contoursCriteria = new ContoursBrowsingCriteria(10);

    //                complexDTM.GetContouring().BrowseContours(contoursCriteria, ContourCallback, null);

    //                complexDTM.GetDraping().DrapeLinearElement(new Bentley.Civil.Geometry.Linear.Line(new Bentley.GeometryNET.DPoint3d(0, 0), new Bentley.GeometryNET.DPoint3d(100, 100)), 0.0);
    //                complexDTM.Dispose();
    //                provider.CleanDTMS();
    //                }
    //            }
    //        DTMProviderManager.Instance.UnregisterProvider(provider);
    //        }
    //    public bool ContourCallback(BGEO.DPoint3d[] tPoint, BGEO.DPoint3d contourDirection, object oArg)
    //        {
    //        return true;
    //        }

    //    }
    }

