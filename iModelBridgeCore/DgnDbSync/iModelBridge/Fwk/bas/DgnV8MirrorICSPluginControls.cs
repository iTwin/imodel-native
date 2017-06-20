using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Xml;
using Bentley.Automation.JobConfiguration;

namespace BentleyB0200.Dgn.DgnV8Mirror.ICS
    {
    public partial class DgnV8MirrorICSPluginControls: UserControl
        {


        public DgnV8MirrorICSPluginControls (JobDefinition jd, Guid DocumentProcessorGuid)
            {
            // This call is required by the Windows.Forms Form Designer.
            InitializeComponent ();

            XmlNode xmlNode = (XmlNode)jd.GetCustomData (DocumentProcessorGuid);
            if (xmlNode != null)
                {
                DgnV8MirrorICSPluginConfigData myDocProcConfigData = Util.GetDgnV8MirrorICSPluginConfigData ((XmlElement)xmlNode);
                }
            }

        internal DgnV8MirrorICSPluginConfigData GetDgnV8MirrorICSPluginConfigData ()
            {
            DgnV8MirrorICSPluginConfigData data = new DgnV8MirrorICSPluginConfigData();
            return data;
            }

        internal XmlElement ToXmlElement ()
            {
            return Util.DgnV8MirrorICSPluginConfigData2XmlElement (GetDgnV8MirrorICSPluginConfigData ());
            } 
        }
    }
