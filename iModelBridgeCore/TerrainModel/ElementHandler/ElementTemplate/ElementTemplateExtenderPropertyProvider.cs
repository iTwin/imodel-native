extern alias ustation;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing.Design;
using System.Globalization;
using System.Linq;
using System.Windows.Forms;
using Bentley.DgnPlatformNET;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.UI;
using Bentley.MstnPlatformNET.Templates.Support;
using Bentley.MstnPlatformNET.WinForms.Controls;
using Bentley.MstnPlatformNET.WinForms.ECPropertyPane;
using BECO = Bentley.ECObjects;
using SWFD = System.Windows.Forms.Design;
using SCG = System.Collections.Generic;
using SWF = System.Windows.Forms;
using DPN = Bentley.DgnPlatformNET;
using BIM = ustation::Bentley.Internal.MstnPlatformNET;
using SD = System.Drawing;
using BMW = Bentley.MstnPlatformNET.WinForms;
using BMG = Bentley.MstnPlatformNET.GUI;
using BUICWF = Bentley.UI.Controls.WinForms;
namespace Bentley.TerrainModel.ElementTemplate
    {

    public class WeightStructEditorWithParent : Bentley.TerrainModelNET.WeightEditor
        {
            // Methods
            public override object EditValue(ITypeDescriptorContext context, IServiceProvider provider, object value, IECPropertyValue propertyValue, ECEnumerablePropertyDescriptor propertyDescriptor)
            {
                IEnumerator<IECPropertyValue> enumerator = null;
                IECPropertyValue value2 = null;
                object obj2 = null;
                IECPropertyValue weightPropertyValue = null;
                IECStructValue weightStructPropertyValue = null;
                IECPropertyValue current = null;
                weightPropertyValue = this.GetWeightPropertyValue(propertyValue);
                if (null != weightPropertyValue)
                {
                    obj2 = base.EditValue(context, provider, value, weightPropertyValue, propertyDescriptor);
                }
                if (null != obj2)
                {
                    ECPropertyPane.CallMultiInstanceBeforeChangeEvent(propertyDescriptor);
                    enumerator = propertyDescriptor.GetEnumerator();
                    try
                    {
                        while (enumerator.MoveNext())
                        {
                            current = enumerator.Current;
                            weightStructPropertyValue = (IECStructValue) current;
                            value2 = this.GetWeightPropertyValue(weightStructPropertyValue);
                            if (null != value2)
                            {
                                value2.NativeValue = obj2;
                            }
                        }
                    }
                    finally
                    {
                        IDisposable disposable = enumerator;
                        if (disposable != null)
                        {
                            disposable.Dispose();
                        }
                    }
                    ECPropertyPane.CallMultiInstanceAfterChangeEvent(propertyDescriptor);
                }
                return null;
            }

            public override bool GetPaintValueSupported(ITypeDescriptorContext context, IECPropertyValue propertyValue, ECEnumerablePropertyDescriptor propertyDescriptor)
            {
                IECPropertyValue weightPropertyValue = null;
                weightPropertyValue = this.GetWeightPropertyValue(propertyValue);
                return ((null != weightPropertyValue) && base.GetPaintValueSupported(context, weightPropertyValue, propertyDescriptor));
            }

            private IECPropertyValue GetWeightPropertyValue(IECPropertyValue weightStructPropertyValue)
            {
                IECStructValue value2 = null;
                value2 = (IECStructValue) weightStructPropertyValue;
                return value2["Value"];
            }

            public override void PaintValue(PaintValueEventArgs args, IECPropertyValue propertyValue, ECEnumerablePropertyDescriptor propertyDescriptor)
            {
                IECPropertyValue weightPropertyValue = null;
                weightPropertyValue = this.GetWeightPropertyValue(propertyValue);
                if (null != weightPropertyValue)
                {
                    base.PaintValue(args, weightPropertyValue, propertyDescriptor);
                }
            }
        }

    public class WeightCriteriaStructTypeConverterWithParent : CriteriaStructTypeConverter
        {
        // Methods
        public override bool CanConvertFromString (IECPropertyValue propVal, ECEnumerablePropertyDescriptor propertyDescriptor, int unusedIndex)
            {
            return false;
            }

        public override object ConvertFromString (IECPropertyValue propVal, ECEnumerablePropertyDescriptor propertyDescriptor, int unusedIndex, CultureInfo culture, string externalString)
            {
            return null;
            }

        public override string ConvertToString (IECPropertyValue propVal, ECEnumerablePropertyDescriptor propertyDescriptor, int unusedIndex, CultureInfo culture, object value)
            {
            string localizedString = null;
            IECPropertyValue value2 = null;
            IECPropertyValue value3 = null;
            IECStructValue value4 = null;
            value4 = (IECStructValue)propVal;
            value2 = value4["Value"];
            if ((null != value2) && !value2.IsNull)
                {
                localizedString = string.Empty;
                int intValue = value2.IntValue;
                if (-2 == intValue)
                    {
                    localizedString = Bentley.TerrainModelNET.SymbologyPickerOptions.Instance.ByCellDisplayString;
                    }
                else if (-1 == intValue)
                    {
                    localizedString = TemplateExtensionManager.GetLocalizedString ("ByLevel");
                    }
                else
                    {
                    localizedString = value2.StringValue;
                    }
                value3 = value4["Criteria"];
                if (value3.IsNull)
                    {
                    return localizedString;
                    }
                return string.Format ("{0};{1}", localizedString, value3.StringValue);
                }
            return string.Empty;
            }
        }

    public class LineStyleStructEditorWithParent : LineStyleNameEditor
    {
        public override SymbologyPickerOptions PickerOptions
            {
            get
                {
                return Bentley.TerrainModelNET.SymbologyPickerOptions.Instance;
                }
            }
        // Methods
        public override object EditValue(ITypeDescriptorContext context, IServiceProvider provider, object value, IECPropertyValue propertyValue, ECEnumerablePropertyDescriptor propertyDescriptor)
        {
            IEnumerator<IECPropertyValue> enumerator = null;
            IECPropertyValue value2 = null;
            object obj2 = null;
            IECPropertyValue lineStylePropertyValue = null;
            IECStructValue lineStyleStructPropertyValue = null;
            IECPropertyValue current = null;
            lineStylePropertyValue = this.GetLineStylePropertyValue(propertyValue);
            if (null != lineStylePropertyValue)
            {
                obj2 = base.EditValue(context, provider, value, lineStylePropertyValue, propertyDescriptor);
            }
            if (null != obj2)
            {
                ECPropertyPane.CallMultiInstanceBeforeChangeEvent(propertyDescriptor);
                enumerator = propertyDescriptor.GetEnumerator();
                try
                {
                    while (enumerator.MoveNext())
                    {
                        current = enumerator.Current;
                        lineStyleStructPropertyValue = (IECStructValue) current;
                        value2 = this.GetLineStylePropertyValue(lineStyleStructPropertyValue);
                        if (null != value2)
                        {
                            value2.NativeValue = obj2;
                        }
                    }
                }
                finally
                {
                    IDisposable disposable = enumerator;
                    if (disposable != null)
                    {
                        disposable.Dispose();
                    }
                }
                ECPropertyPane.CallMultiInstanceAfterChangeEvent(propertyDescriptor);
            }
            return null;
        }

        private IECPropertyValue GetLineStylePropertyValue(IECPropertyValue lineStyleStructPropertyValue)
        {
            IECStructValue value2 = null;
            value2 = (IECStructValue) lineStyleStructPropertyValue;
            return value2["Value"];
        }

        public override bool GetPaintValueSupported(ITypeDescriptorContext context, IECPropertyValue propertyValue, ECEnumerablePropertyDescriptor propertyDescriptor)
        {
            IECPropertyValue lineStylePropertyValue = null;
            lineStylePropertyValue = this.GetLineStylePropertyValue(propertyValue);
            return ((null != lineStylePropertyValue) && base.GetPaintValueSupported(context, lineStylePropertyValue, propertyDescriptor));
        }

        public override void PaintValue(PaintValueEventArgs args, IECPropertyValue propertyValue, ECEnumerablePropertyDescriptor propertyDescriptor)
        {
            IECPropertyValue lineStylePropertyValue = null;
            lineStylePropertyValue = this.GetLineStylePropertyValue(propertyValue);
            if (null != lineStylePropertyValue)
            {
                base.PaintValue(args, lineStylePropertyValue, propertyDescriptor);
            }
        }
    }

    public class LineStyleCriteriaStructTypeConverterWithParent : CriteriaStructTypeConverter
    {
        // Methods
        public override bool CanConvertFromString(IECPropertyValue propVal, ECEnumerablePropertyDescriptor propertyDescriptor, int unusedIndex)
        {
            return false;
        }

        public override object ConvertFromString(IECPropertyValue propVal, ECEnumerablePropertyDescriptor propertyDescriptor, int unusedIndex, CultureInfo culture, string externalString)
        {
            return null;
        }

        public override string ConvertToString(IECPropertyValue propVal, ECEnumerablePropertyDescriptor propertyDescriptor, int unusedIndex, CultureInfo culture, object value)
        {
            string localizedString = null;
            IECPropertyValue value2 = null;
            IECPropertyValue value3 = null;
            IECStructValue value4 = null;
            value4 = (IECStructValue) propVal;
            value2 = value4["Value"];
            if ((null != value2) && !value2.IsNull)
            {
                localizedString = string.Empty;
                int num2 = 0x7fffffff;
                int num4 = 0x7ffffffe;
                if (value2.StringValue.Equals(num4.ToString()))
                {
                localizedString = Bentley.TerrainModelNET.SymbologyPickerOptions.Instance.ByCellDisplayString;
                }
                else
                {
                    int num3 = num2;
                    if (value2.StringValue.Equals(num3.ToString()))
                    {
                        localizedString = TemplateExtensionManager.GetLocalizedString("ByLevel");
                    }
                    else
                    {
                        localizedString = value2.StringValue;
                    }
                }
                value3 = value4["Criteria"];
                if (value3.IsNull)
                {
                    return localizedString;
                }
                return string.Format("{0};{1}", localizedString, value3.StringValue);
            }
            return string.Empty;
        }
    }

    public class ColorStructEditorWithParent : ColorTypeEditor
    {
        public override SymbologyPickerOptions PickerOptions
            {
            get
                {
                return Bentley.TerrainModelNET.SymbologyPickerOptions.Instance;
                }
            }
        // Methods
        public override bool AddUniqueTabPage(TabControl tabControl)
        {
            return false;
        }

        public override object EditValue(ITypeDescriptorContext context, IServiceProvider provider, object value, IECPropertyValue propertyValue, ECEnumerablePropertyDescriptor propertyDescriptor)
        {
            IEnumerator<IECPropertyValue> enumerator = null;
            ElementColor color = null;
            IECPropertyValue colorPropertyValue = null;
            IECPropertyValue current = null;
            object obj2 = null;
            IECStructValue iecStruct = null;
            colorPropertyValue = this.GetColorPropertyValue(propertyValue);
            if (null != colorPropertyValue)
            {
                obj2 = base.EditValue(context, provider, value, colorPropertyValue, propertyDescriptor);
            }
            color = obj2 as ElementColor;
            if (null != color)
            {
                ECPropertyPane.CallMultiInstanceBeforeChangeEvent(propertyDescriptor);
                enumerator = propertyDescriptor.GetEnumerator();
                try
                {
                    while (enumerator.MoveNext())
                    {
                        current = enumerator.Current;
                        iecStruct = (IECStructValue)current;
                        if (null != iecStruct)
                            iecStruct = (IECStructValue)iecStruct["Value"];
                        if (null != iecStruct)
                            color.PopulateStruct (ref iecStruct);
                    }
                }
                finally
                {
                    IDisposable disposable = enumerator;
                    if (disposable != null)
                    {
                        disposable.Dispose();
                    }
                }
                ECPropertyPane.CallMultiInstanceAfterChangeEvent(propertyDescriptor);
            }
            return null;
        }

        private IECPropertyValue GetColorPropertyValue(IECPropertyValue colorStructPropertyValue)
        {
            IECStructValue value2 = null;
            value2 = (IECStructValue) colorStructPropertyValue;
            return value2["Value"];
        }

        public override bool GetPaintValueSupported(ITypeDescriptorContext context, IECPropertyValue propertyValue, ECEnumerablePropertyDescriptor propertyDescriptor)
        {
            IECPropertyValue colorPropertyValue = null;
            colorPropertyValue = this.GetColorPropertyValue(propertyValue);
            return ((null != colorPropertyValue) && base.GetPaintValueSupported(context, colorPropertyValue, propertyDescriptor));
        }

        public override void PaintValue(PaintValueEventArgs args, IECPropertyValue propertyValue, ECEnumerablePropertyDescriptor propertyDescriptor)
        {
            IECPropertyValue colorPropertyValue = null;
            colorPropertyValue = this.GetColorPropertyValue(propertyValue);
            if (null != colorPropertyValue)
            {
                base.PaintValue(args, colorPropertyValue, propertyDescriptor);
            }
        }
    }

    public class ReadOnlyCriteriaStructTypeConverterWithParent : CriteriaStructTypeConverter
        {
        // Methods
        public override bool CanConvertFromString (IECPropertyValue propVal, ECEnumerablePropertyDescriptor propertyDescriptor, int unusedIndex)
            {
            return false;
            }

        public override object ConvertFromString (IECPropertyValue propVal, ECEnumerablePropertyDescriptor propertyDescriptor, int unusedIndex, CultureInfo culture, string externalString)
            {
            return null;
            }

        public override string ConvertToString (IECPropertyValue propVal, ECEnumerablePropertyDescriptor propertyDescriptor, int unusedIndex, CultureInfo culture, object value)
            {
            // convert the members to string and concatenate them.
            IECStructValue     structVal = (IECStructValue)propVal;
            IECPropertyValue   criteriaVal;
            IECPropertyValue   valueVal;
            if (!((valueVal = structVal["Value"]).IsNull))
                {
                string valueString = string.Empty;

                if (!valueVal.IsPrimitive)
                    {
                    ElementColor elementColor = ustation.Bentley.MstnPlatformNET.XDataTree.ElementTemplateMgr.LoadElementColorFromPropertyValue (valueVal as IECStructValue);
                    if (null != elementColor && ColorSource.ByCell == elementColor.Source)
                        {
                        valueString = Bentley.TerrainModelNET.SymbologyPickerOptions.Instance.ByCellDisplayString;
                        if ((criteriaVal = structVal["Criteria"]).IsNull)
                            {
                            return valueString;
                            }
                        else
                            {
                            return string.Format ("{0};{1}", valueString, criteriaVal.StringValue);
                            }                        
                        }
                    }
                }

            return base.ConvertToString (propVal, propertyDescriptor, unusedIndex, culture, value);
            }

        }


    public class DisplayStyleEditor : DgnECUITypeEditor
        {
        private class StyleInfo
            {
            private int m_listIndex;
            private int m_styleId;
            private string m_styleName;
            private string m_iconName;

            public StyleInfo (int listIndex, int styleId, string name, string iconName)
                {
                m_listIndex = listIndex;
                m_styleId = styleId;
                m_styleName = name;
                m_iconName = iconName;
                }

            public StyleInfo (int listIndex, DPN.DisplayStyle style) : this (listIndex, style.Index, style.Name, BIM.DisplayStyleExtensions.GetIconName (style)) { }

            public int ListIndex { get { return m_listIndex; } }
            public int StyleId { get { return m_styleId; } }
            public string Name { get { return m_styleName; } }
            public string IconName { get { return m_iconName; } }
            };

        private SWFD.IWindowsFormsEditorService m_edSvc;
        private string m_selectedName;
        private BIM.DisplayStyleList m_displayStyles;
        private SCG.List<StyleInfo> m_styleInfos;
        private SWF.ImageList m_iconList;
        private bool m_usableForView;
        private bool m_usableForClip;
        private bool m_usableAddFromView;

        /*------------------------------------------------------------------------------------**/
        /// <summary>Constructor</summary>
        /// <author>Mukesh.Pant</author>                                <date>06/2010</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public DisplayStyleEditor (bool usableForView, bool usableForClip)
            {
            m_usableForView = usableForView;
            m_usableAddFromView = m_usableForClip = usableForClip;
            }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Constructor</summary>
        /// <author>Mukesh.Pant</author>                                <date>06/2010</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public DisplayStyleEditor (bool usableForView, bool usableForClip, bool usableAddFromView)
            {
            m_usableForView = usableForView;
            m_usableForClip = usableForClip;
            m_usableAddFromView = usableAddFromView;
            }

        /*------------------------------------------------------------------------------------**/
        /// <summary>!!! Describe Method Here !!!</summary>
        /// <author>Mukesh.Pant</author>                                <date>03/2010</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public override bool GetPaintValueSupported
        (
        ITypeDescriptorContext context,
        IECPropertyValue propertyValue,
        ECEnumerablePropertyDescriptor propertyDescriptor
        )
            {
            return true;
            }

        /*------------------------------------------------------------------------------------**/
        /// <summary>!!! Describe Method Here !!!</summary>
        /// <author>Mukesh.Pant</author>                                <date>03/2010</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void BuildIconList (System.String iconName)
            {
            SD.Size iconSize = new SD.Size (16, 16);

            BMW.Icon icon = new BMW.Icon (iconName, BMG.IconType.WindowsIcon, BMG.IconStyle.Normal, BMG.ColorIndex.Default, ref iconSize);
            System.Drawing.Bitmap original = icon.GetBitmap ();
            if (null == original)
                original = icon.GetIcon ().ToBitmap ();

            m_iconList.ImageSize = original.Size;
            m_iconList.ColorDepth = System.Windows.Forms.ColorDepth.Depth32Bit;
            System.Drawing.Bitmap bmp = new System.Drawing.Bitmap (original, original.Size);
            m_iconList.Images.Add (iconName, bmp); // must copy bitmap
            icon.Dispose ();
            }

        /*------------------------------------------------------------------------------------**/
        /// <summary>override method EditValue</summary>
        /// <author>Mukesh.Pant</author>                                 <date>03/2010</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public override object EditValue
        (
        ITypeDescriptorContext context,
        IServiceProvider provider,
        object value,
        IECPropertyValue propertyValue,
        ECEnumerablePropertyDescriptor propertyDescriptor
        )
            {
            m_selectedName = null;
            IECInstance instance;
            DPN.DgnFile dgnfile;
            if (null != (instance = propertyValue.Instance))
                {
                dgnfile = GetInstanceDgnFile (propertyValue);
                if (null == dgnfile)
                    dgnfile = ustation::Bentley.MstnPlatformNET.Session.Instance.GetActiveDgnFile ();

                // Obtain an IWindowsFormsEditorService.
                SWFD.IWindowsFormsEditorService edSvc;
                if (null != (edSvc = (SWFD.IWindowsFormsEditorService)provider.GetService (typeof (SWFD.IWindowsFormsEditorService))))
                    {
                    // D-112582: ECQuery form may hand us a null property value - choose a default
                    string myStyleName = null != value ? (string)value : "";
                    BUICWF.EnterKeyImageListBox listBox = new BUICWF.EnterKeyImageListBox ();
                    foreach (StyleInfo style in m_styleInfos)
                        {
                        if ("-" == style.Name)
                            {
                            BUICWF.SeparatorListBoxItem sepItem = new BUICWF.SeparatorListBoxItem ();
                            listBox.Items.Add (sepItem);
                            }
                        else
                            {
                            BUICWF.ImageListBoxItem item = new BUICWF.ImageListBoxItem (style.Name);

                            item.ImageIndex = -1;
                            if (m_iconList.Images.ContainsKey (style.IconName))
                                item.ImageIndex = m_iconList.Images.IndexOfKey (style.IconName);

                            item.UserData = style;

                            listBox.Items.Add (item);
                            if (style.Name == myStyleName)
                                {
                                if (style.StyleId < 0)  // "from view", not a real DisplayStyle
                                    {
                                    m_selectedName = style.Name;
                                    listBox.SelectedItem = item;
                                    }
                                else
                                    {
                                    using (DPN.DisplayStyle dispStyle = m_displayStyles[style.ListIndex])
                                        {
                                        if (dispStyle.GetFile () == dgnfile)
                                            {
                                            m_selectedName = style.Name;
                                            listBox.SelectedItem = item;
                                            }
                                        }
                                    }
                                }
                            }
                        }

                    listBox.ImageList = m_iconList;

                    // Wire the ListBox events and display the drop-down UI. The selection process
                    // is then handled inside the event handler methods.
                    listBox.MouseUp += new SWF.MouseEventHandler (OnMouseUpInList);
                    listBox.EnterPressed += new EventHandler (OnEnterPressedInList);

                    m_edSvc = edSvc;
                    edSvc.DropDownControl (listBox);

                    // NOTE: We must ensure the style is imported into the element's DgnFile before it can be applied
                    foreach (StyleInfo info in m_styleInfos)
                        {
                        if (info.Name == m_selectedName)
                            {
                            if (0 <= info.ListIndex)
                                {
                                if (null != dgnfile)
                                    {
                                    DPN.DisplayStyle style = m_displayStyles[info.ListIndex];
                                    DPN.DisplayStyle importedStyle = DPN.DisplayStyleManager.EnsureDisplayStyleIsInFile (style, dgnfile);
                                    if (null != importedStyle)
                                        {
                                        value = importedStyle.Name;
                                        if (importedStyle != style)
                                            {
                                            importedStyle.Dispose ();
                                            BuildDisplayStyleAndIconList (propertyValue);
                                            }
                                        }

                                    style.Dispose ();
                                    }
                                }
                            else
                                {
                                value = "";   // "From View"
                                }

                            break;
                            }
                        }
                    }
                }

            return value;
            }

        /*------------------------------------------------------------------------------------**/
        /// <summary>!!! Describe Method Here !!!</summary>
        /// <author>Mukesh.Pant</author>                                <date>03/2010</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void BuildDisplayStyleAndIconList
        (
        IECPropertyValue propertyValue
        )
            {
            DPN.DgnFile file = ustation::Bentley.MstnPlatformNET.Session.Instance.GetActiveDgnFile ();
            bool includeLibs = null != file;

            m_displayStyles = new BIM.DisplayStyleList (file, m_usableForView, m_usableForClip);
            m_iconList = new SWF.ImageList ();
            m_styleInfos = new SCG.List<StyleInfo> ();

            if (m_usableAddFromView)
                {
                // ----- From View -------
                StyleInfo fromViewStyle = new StyleInfo (-1, -1, BIM.DisplayStyleList.GetFromViewName (), "View");
                m_styleInfos.Add (fromViewStyle);
                BuildIconList (fromViewStyle.IconName);

                // It is just a dummy style holder so that it can be reconginized as separator when we insert
                // items in listbox.
                StyleInfo separator = new StyleInfo (-1, -2, "-", "");
                m_styleInfos.Add (separator);
                }

            // ------ Get rest of the styles -------

            // Sort names alphabetically (only for the Elements Display Styles)
            if (m_usableAddFromView && !m_usableForClip && m_usableForView)
                {
                System.Collections.ArrayList names = new System.Collections.ArrayList ();

                // Add names to be sorted
                foreach (DPN.DisplayStyle style in m_displayStyles)
                    {
                    if (includeLibs || style.GetFile () == file)
                        names.Add (style.Name);

                    style.Dispose ();
                    }

                names.Sort ();

                // Add the styles in alphabetical order
                foreach (String name in names)
                    {
                    int listIndex = 0;
                    foreach (DPN.DisplayStyle style in m_displayStyles)
                        {
                        if (name.Equals (style.Name))
                            {
                            m_styleInfos.Add (new StyleInfo (listIndex, style));
                            BuildIconList (BIM.DisplayStyleExtensions.GetIconName (style));
                            style.Dispose ();
                            break;
                            }
                        else
                            {
                            listIndex++;
                            style.Dispose ();
                            }
                        }
                    }

                }
            else
                {
                // Add them normally
                int listIndex = 0;
                foreach (DPN.DisplayStyle style in m_displayStyles)
                    {
                    if (includeLibs || style.GetFile () == file)
                        {
                        m_styleInfos.Add (new StyleInfo (listIndex++, style));
                        BuildIconList (BIM.DisplayStyleExtensions.GetIconName (style));
                        }

                    style.Dispose ();
                    }
                }
            }

        /*------------------------------------------------------------------------------------**/
        /// <summary>override method PaintValue</summary>
        /// <author>Mukesh.Pant</author>                                 <date>03/2010</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public override void PaintValue
        (
        PaintValueEventArgs args,
        IECPropertyValue propertyValue,
        ECEnumerablePropertyDescriptor propertyDescriptor
        )
            {
            // if any of the list is null.. build new list for both.
            if (null == m_styleInfos || null == m_iconList)
                BuildDisplayStyleAndIconList (propertyValue);

            // D-112582: ECQuery form may hand us a null property value - don't try to get an integer out of it
            if (!propertyValue.IsNull)
                {
                string styleName = propertyValue.StringValue;
                foreach (StyleInfo style in m_styleInfos)
                    {
                    if (styleName != style.Name)
                        continue;

                    if (m_iconList.Images.ContainsKey (style.IconName))
                        {
                        System.Drawing.Graphics graphics = args.Graphics;
                        graphics.DrawImage (m_iconList.Images[style.IconName], args.Bounds.Location);
                        }
                    }
                }
            }

        /*------------------------------------------------------------------------------------**/
        /// <summary>override method GeteEditStyle</summary>
        /// <author>Mukesh.Pant</author>                                 <date>03/2010</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public override UITypeEditorEditStyle GetEditStyle
        (
        ITypeDescriptorContext context
        )
            {
            return UITypeEditorEditStyle.DropDown;
            }

        /*------------------------------------------------------------------------------------**/
        /// <summary>!!! Describe Method Here !!!</summary>
        /// <author>Mukesh.Pant</author>                                <date>03/2010</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void ProcessSelectedListItem
        (
        SWF.ListBox listBox
        )
            {
            // Set the LineStyle
            BUICWF.ImageListBoxItem item;

            if (null != (item = listBox.SelectedItem as BUICWF.ImageListBoxItem))
                {
                StyleInfo style;
                if (null != (style = item.UserData as StyleInfo))
                    m_selectedName = style.Name;

                m_edSvc.CloseDropDown ();
                }
            }

        /*------------------------------------------------------------------------------------**/
        /// <summary>!!! Describe Method Here !!!</summary>
        /// <author>Mukesh.Pant</author>                                <date>03/2010</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void OnMouseUpInList
        (
        Object sender,
        SWF.MouseEventArgs e
        )
            {
            // Process/accept the selected list box item.
            ProcessSelectedListItem (sender as SWF.ListBox);
            }

        /*------------------------------------------------------------------------------------**/
        /// <summary>!!! Describe Method Here !!!</summary>
        /// <author>Mukesh.Pant</author>                                <date>03/2010</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        private void OnEnterPressedInList
        (
        Object sender,
        EventArgs e
        )
            {
            // Process/accept the selected list box item.
            ProcessSelectedListItem (sender as SWF.ListBox);
            }
        } // DisplayStyleEditor

    public class DisplayStyleEditorWithParent : DisplayStyleEditor
        {
        public DisplayStyleEditorWithParent ()
            : base (true, false) //, true)
            {
            }
//        public override object EditValue(ITypeDescriptorContext context, IServiceProvider provider, object value)
//            {
//            DgnPlatformNET.DgnFile instanceDgnFile = ustation::Bentley.MstnPlatformNET.Session.Instance.GetActiveDgnFile ();
//            ustation::Bentley.Internal.MstnPlatformNET.DisplayStyleList list = new ustation::Bentley.Internal.MstnPlatformNET.DisplayStyleList (instanceDgnFile, true, false);
//
//            int indexValue = (from ds in list where ds.Name == (string) value select ds.Index).FirstOrDefault();
//            value = base.EditValue(context, provider, indexValue);
//            indexValue = (int)value;
//            string name = (from ds in list where ds.Index == indexValue select ds.Name).FirstOrDefault ();
//            return name;
//            }
//
//        public override void PaintValue (PaintValueEventArgs args, IECPropertyValue propertyValue, ECEnumerablePropertyDescriptor propertyDescriptor)
//            {
//            base.PaintValue (args, propertyValue, propertyDescriptor);
//            }
        }

    public class DisplayStyleTypeConverter : ECCustomFormatTypeConverter
        {     
        public override StandardValuesCollection GetStandardValues(ITypeDescriptorContext context)
            {
            ustation::Bentley.Internal.MstnPlatformNET.DisplayStyleList displayStyles = new ustation::Bentley.Internal.MstnPlatformNET.DisplayStyleList (ustation::Bentley.MstnPlatformNET.Session.Instance.GetActiveDgnFile(), true, false);
            List<string> values = displayStyles.Select(ds => ds.Name).ToList();
            return new StandardValuesCollection (values);
            }

        public override object ConvertFromString (IECPropertyValue propVal, ECEnumerablePropertyDescriptor propertyDescriptor, int coordinateIndex, CultureInfo culture, string value)
            {
            return value;
            }

        public override string ConvertToString (IECPropertyValue propVal, ECEnumerablePropertyDescriptor propertyDescriptor, int coordinateIndex, CultureInfo culture, object value)
            {
            return value.ToString ();
            }
        }

 

 


    public interface ICanApplyProperties
        {
        void ApplyProperties(Bentley.TerrainModelNET.Element.DTMElement elem, BECO.Instance.IECInstance templateInstance);
        }

    public abstract class ElementTemplateExtenderPropertyProvider
        {

        /// <summary>
        /// set up the extended type that shows arrays as strings - for level names
        /// </summary> 
        protected static BECO.Instance.IECInstance m_showArrayAsStringET = null;

        /// <summary>
        /// set up the extended type that shows arrays as strings - for line styles
        /// </summary> 
        protected static BECO.Instance.IECInstance m_showLineStyleArrayAsStringET = null;

        /// <summary>
        /// set up the extended type that shows arrays as strings - for line weight
        /// </summary> 
        protected static BECO.Instance.IECInstance m_showLineWeightArrayAsStringET = null;

        /// <summary>
        /// set up the extended type that shows arrays as strings - for point3d
        /// </summary> 
        protected static BECO.Instance.IECInstance m_nonZeroPointStructET = null;

        protected static BECO.Instance.IECInstance m_showActivePointArrayAsStringET = null;
        /// <summary>
        /// Criteria struct - whatever?
        /// </summary>
        protected static BECO.Schema.ECClass m_criteriaStruct = null;

        public ElementTemplateExtenderPropertyProvider()
            {
            // Create the extended required types
            CreateExtendedTypes();
            }

        /// <summary>
        /// Create any extended types that have not been created yet
        /// </summary>
        private void CreateExtendedTypes()
            {
            // Create any extended types that have not yet been created
/*
 *          if (m_criteriaStruct == null)
                {
                m_criteriaStruct = new Bentley.ECObjects.Schema.ECClass("CriteriaStruct", true);
                m_criteriaStruct.Add(new BECO.Schema.ECProperty("Criteria", BECO.ECObjects.StringType));
                }

            if (m_showArrayAsStringET == null)
                {
                // set up the extended type that shows arrays as strings.
                m_showArrayAsStringET = BECO.UI.ECPropertyPane.CreateExtendedType("ShowArrayAsString");
                BECO.UI.ECPropertyPane.SetExtendedTypeTypeConverter(m_showArrayAsStringET, typeof(ValueArrayTypeConverter));
                }

            if (m_showLineStyleArrayAsStringET == null)
                {
                // set up the extended type that shows arrays as strings.
                m_showLineStyleArrayAsStringET = BECO.UI.ECPropertyPane.CreateExtendedType("ShowLineStyleArrayAsString");
                BECO.UI.ECPropertyPane.SetExtendedTypeTypeConverter(m_showLineStyleArrayAsStringET, typeof(LineStyleValueArrayTypeConverter));
                }

            if (m_showLineWeightArrayAsStringET == null)
                {
                // set up the extended type that shows arrays as strings.
                m_showLineWeightArrayAsStringET = BECO.UI.ECPropertyPane.CreateExtendedType("ShowLineWeightArrayAsString");
                BECO.UI.ECPropertyPane.SetExtendedTypeTypeConverter(m_showLineWeightArrayAsStringET, typeof(LineWeightValueArrayTypeConverter));
                }

            if (m_nonZeroPointStructET == null)
                {
                // set up the extended type that shows arrays of points as strings.
                m_nonZeroPointStructET = BECO.UI.ECPropertyPane.CreateExtendedType("NonZeroPointStructType");
                BECO.UI.ECPropertyPane.SetExtendedTypeTypeConverter(m_nonZeroPointStructET, typeof(NonZeroPointStructTypeConverter));
                }
            if (m_showActivePointArrayAsStringET == null)
                {
                m_showActivePointArrayAsStringET = BECO.UI.ECPropertyPane.CreateExtendedType ("ShowActivePointArrayAsString");
                BECO.UI.ECPropertyPane.SetExtendedTypeTypeConverter (m_showActivePointArrayAsStringET, typeof (ActivePointValueTypeConverter));
                }
*/


            // --- used to display value in Reports ---
            //BECO.Instance.IECInstance weightExtendedType = BECO.UI.ECPropertyPane.CreateExtendedType ("LineWeight_ExtendedType1");
            //BECO.UI.ECPropertyPane.SetExtendedTypeTypeConverter (weightExtendedType, typeof (WeightTypeConverter));

            // --- used to display and edit values in property pane ---e
            BECO.Instance.IECInstance weightStructExtendedType = BECO.UI.ECPropertyPane.CreateExtendedType ("LineWeight_ExtendedType");
            BECO.UI.ECPropertyPane.SetExtendedTypeWinFormUIEditor (weightStructExtendedType, typeof (WeightStructEditorWithParent));
            BECO.UI.ECPropertyPane.SetExtendedTypeTypeConverter (weightStructExtendedType, typeof (WeightCriteriaStructTypeConverterWithParent));

            BECO.Instance.IECInstance lineStyleExtendedType = BECO.UI.ECPropertyPane.CreateExtendedType ("TMNameStyle");
            BECO.UI.ECPropertyPane.SetExtendedTypeTypeConverter (lineStyleExtendedType, typeof (LineStyleTypeConverter));

            BECO.Instance.IECInstance lineStyleStructExtendedType = BECO.UI.ECPropertyPane.CreateExtendedType ("LineStyle_ExtendedType");
            BECO.UI.ECPropertyPane.SetExtendedTypeWinFormUIEditor (lineStyleStructExtendedType, typeof (LineStyleStructEditorWithParent));
            BECO.UI.ECPropertyPane.SetExtendedTypeTypeConverter (lineStyleStructExtendedType, typeof (LineStyleCriteriaStructTypeConverterWithParent));

            BECO.Instance.IECInstance colorStructExtendedType = BECO.UI.ECPropertyPane.CreateExtendedType ("Color_ExtendedType");
            BECO.UI.ECPropertyPane.SetExtendedTypeWinFormUIEditor (colorStructExtendedType, typeof (ColorStructEditorWithParent));
            BECO.UI.ECPropertyPane.SetExtendedTypeTypeConverter (colorStructExtendedType, typeof (ReadOnlyCriteriaStructTypeConverterWithParent));

            BECO.Instance.IECInstance transparencyStructExtendedType = BECO.UI.ECPropertyPane.CreateExtendedType ("Transparency_ExtendedType");
            BECO.UI.ECPropertyPane.SetExtendedTypeWinFormUIEditor (transparencyStructExtendedType, typeof (TransparencyEditor));
            BECO.UI.ECPropertyPane.SetExtendedTypeTypeConverter (transparencyStructExtendedType, typeof (TransparencyTypeConverter));

            IECInstance extendedType = Bentley.ECObjects.UI.ECPropertyPane.CreateExtendedType ("ElementParams_TMLevelStruct");
            BECO.UI.ECPropertyPane.SetExtendedTypeWinFormUIEditor (extendedType, typeof (Bentley.MstnPlatformNET.Templates.Support.LevelStructEditor));
            BECO.UI.ECPropertyPane.SetExtendedTypeTypeConverter (extendedType, typeof (Bentley.MstnPlatformNET.Templates.Support.LevelTypeConverter));

            extendedType = Bentley.ECObjects.UI.ECPropertyPane.CreateExtendedType ("ElementParams_TMTextStyleStruct");
            BECO.UI.ECPropertyPane.SetExtendedTypeWinFormUIEditor (extendedType, typeof (Bentley.MstnPlatformNET.Templates.Support.TextStyleStructEditor));
            BECO.UI.ECPropertyPane.SetExtendedTypeTypeConverter (extendedType, typeof (Bentley.MstnPlatformNET.Templates.Support.TextStyleTypeConverter));

            extendedType = Bentley.ECObjects.UI.ECPropertyPane.CreateExtendedType ("ElementParams_TMDisplayStyle");
            BECO.UI.ECPropertyPane.SetExtendedTypeWinFormUIEditor (extendedType, typeof (DisplayStyleEditorWithParent));
            BECO.UI.ECPropertyPane.SetExtendedTypeTypeConverter (extendedType, typeof (DisplayStyleTypeConverter));
            }

        /// <summary>
        /// Called when the property group is selected in the context menu
        /// </summary>
        /// <param name="instance">The instance of the properties</param>
        /// <param name="properties">List of properties to add to to show</param>
        public abstract void IsActivated(BECO.Instance.IECInstance instance, System.Collections.Generic.List<BECO.Instance.IECPropertyValue> properties);

        }

    }