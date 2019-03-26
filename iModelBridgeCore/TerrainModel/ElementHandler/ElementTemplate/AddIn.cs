extern alias ustation;
using System;
using ustation;

namespace Bentley.TerrainModel.ElementTemplate
    {       

    /// <summary>
    /// Addin for tests of the ModelSurface library.
    /// </summary>
    [ustation.Bentley.MstnPlatformNET.AddInAttribute(MdlTaskID = "DTMELEMENTTEMPLATEEXTENDER", Password = "HV1107B1B77Y")]
    public class DTMElementTemplateExtenderAddin : ustation.Bentley.MstnPlatformNET.AddIn
        {
        // Make this private and expose IMessageCenter as a CadContext interface.
        public static DTMElementTemplateExtenderAddin App = null;
        private static DTMElementTemplateExtender s_extender = null;
      
        private static global::System.Resources.ResourceManager resourceMan;
        
        private static global::System.Globalization.CultureInfo resourceCulture;
        
        /// <summary>
        ///   Returns the cached ResourceManager instance used by this class.
        /// </summary>
        [global::System.ComponentModel.EditorBrowsableAttribute(global::System.ComponentModel.EditorBrowsableState.Advanced)]
        internal static global::System.Resources.ResourceManager ResourceManager {
            get {
                if (object.ReferenceEquals(resourceMan, null)) {
                global::System.Resources.ResourceManager temp = new global::System.Resources.ResourceManager ("Bentley.TerrainModel.ElementTemplate.LocalizableStrings", typeof (DTMElementTemplateExtenderAddin).Assembly);
                    resourceMan = temp;
                }
                return resourceMan;
            }
        }
        
        /// <summary>
        ///   Overrides the current thread's CurrentUICulture property for all
        ///   resource lookups using this strongly typed resource class.
        /// </summary>
        [global::System.ComponentModel.EditorBrowsableAttribute(global::System.ComponentModel.EditorBrowsableState.Advanced)]
        internal static global::System.Globalization.CultureInfo Culture {
            get {
                return resourceCulture;
            }
            set {
                resourceCulture = value;
            }
        }

        public DTMElementTemplateExtenderAddin(IntPtr mdlDesc)
            : base(mdlDesc)
            {
            App = this;
            }

        protected override int Run(string[] commandLine)
            {            
            // Create the template extender
            if (s_extender == null)
                s_extender = new DTMElementTemplateExtender();
            return 0;
            }        
        }
    class StringLocalizer
        {
        static StringLocalizer s_instance;

        public static StringLocalizer Instance
            {
            get
                {
                if (s_instance == null)
                    s_instance = new StringLocalizer ();
                return s_instance;
                }
            }
        public string GetLocalizedString (string value)
            {
            return DTMElementTemplateExtenderAddin.ResourceManager.GetString (value, DTMElementTemplateExtenderAddin.Culture);
            }


        }
    
    
    }

