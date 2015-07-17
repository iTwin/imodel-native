extern alias ustation;
using System;
using System.Collections.Generic;
using System.Text;
using Bentley.DgnPlatformNET;
using Bentley.DgnPlatformNET.XDataTree;
using Bentley.ECObjects.Instance;
using Bentley.MstnPlatformNET.Templates.Support;
using ustation.Bentley.MstnPlatformNET.XDataTree;
using BECO = Bentley.ECObjects;
using Bentley.ECObjects.UI;
using System.ComponentModel;
using Bentley.DgnPlatformNET.DgnEC;
using Bentley.ECObjects.Schema;

namespace Bentley.TerrainModel.ElementTemplate
    {
    /// <summary>
    /// DTMElementTemplateExtender - provides and Element Extension Template for a Civil DTM element
    /// </summary>
    /// <author>james.goode</author>
    /// <date>5/6/2008</date>
    [InstanceOnLoad]
    public partial class DTMElementTemplateExtender : IElementParamsExtender, IApplyElementParamsToHandle, BECO.UI.IECContextMenuProvider
        {
        private static DTMElementTemplateExtender s_instance = null;
        private static DgnModelRef s_modelReference = null;

        /// <summary>
        /// Deafult constructor
        /// </summary>
        public DTMElementTemplateExtender()
            {
            if (s_instance != null)
                {
                return;
                }

            s_instance = this;
            // Create the schema used by this extension manager
            CreateSchema();

            // Add this class as a template extender and a context menu provider
            XDataTreeSessionMgr.AddElementParamsExtender (this);
            XDataTreeSessionMgr.AddContextMenuProvider (this);

            // Add the schema too...
            //XDataTreeManager.RegisterSchema (m_schema);

//            XDataTreeSessionMgrAddSchema (m_schema, s_className);
           }

        #region IElementParamsExtender Members

        public void ActivateElementParams(XDataTreeNode templateToActivate)
            {
            // Do nothing
            }

        public unsafe bool ApplyElementParams (ustation.Bentley.DgnPlatform.EditElementHandle* element, Bentley.DgnPlatformNET.XDataTree.XDataTreeNode templateToActivate, ECInstanceList modifiedECInstanceList, bool mustReferenceMatchingTemplate)
            {
            Bentley.DgnPlatformNET.Elements.Element managedElement = Bentley.TerrainModelNET.Element.Helper.FromEditElementHandleIntPtr (new System.IntPtr(element));
            Bentley.TerrainModelNET.Element.DTMElement dtmElement = managedElement as Bentley.TerrainModelNET.Element.DTMElement;

            if (dtmElement != null)
                {
                BECO.Instance.IECInstance dtmInstance = null;

                for (int index = 0; index < templateToActivate.ECInstanceList.Count; index++)
                    {
                    BECO.Instance.IECInstance instance = templateToActivate.ECInstanceList[index];
                    if (instance.ClassDefinition.Name == CLASS_NAME)
                        {
                        dtmInstance = instance;
                        break;
                        }
                    }

                if (dtmInstance != null)
                    {
                    s_modelReference = dtmElement.DgnModelRef;

                    ApplyTemplate (dtmElement, dtmInstance);
                    Bentley.TerrainModelNET.Element.Helper.ReleaseElementHandler (dtmElement);
                    return true;
                    }
            }
            if (null != managedElement)
                managedElement.Dispose ();

            return false;
            }

        public bool ApplyElementParamsToSelectedElements(XDataTreeNode templateToApply)
            {
            return false;
            }
        public void CopyTemplateToFile(XDataTreeNode copiedTemplate, XDataTreeNode dgnLibTemplate)
            {
            }
        public unsafe bool OnAddTemplateReference (ustation.Bentley.DgnPlatform.EditElementHandle* element, XDataTreeNode templateDataNode, bool scheduled)
            {
            return false;
            }



        #endregion

        #region ICustomizeExtender Members

        public string ClassName
            {
            get 
                {
                return CLASS_NAME;
                }
            }

        public void CreateNewInstanceData(XDataTreeNode node)
    {
            // We dont need to add anything as we add when they want.
            //BECO.Schema.IECClass ecClass = Schema.GetClass(CLASS_NAME);
            //BECO.Instance.IECInstance ecInstance = ecClass.CreateInstance();

            //if (ecInstance != null)
            //    {
            //    node.ECInstanceList.Add (ecInstance);
            //    }
            //node.Owner.Write (node);
            }

        public void HandleNewDesignFileEvent(bool fileOpened)
            {
            
            }

        public Bentley.ECObjects.Schema.IECSchema Schema
            {
            get 
                {
                // Get schema and return it
                if (m_schema == null)
                    {
                    DgnECManager mgr = DgnECManager.Manager;
                    m_schema = mgr.LocateSchema (SCHEMA_NAME, 1, 0, SchemaMatchType.LatestCompatible, null, ustation.Bentley.MstnPlatformNET.Session.Instance.GetActiveDgnFile ());
                    }
                XDataTreeManager.RegisterSchema (m_schema);

                return m_schema;
                }
            }

        public void SessionShutdown()
            {
            // Nothing to do
            m_schema = null;
            }

        public void SessionStart()
            {
            // Nothing to do
            m_schema = null;
            }
        #endregion          
        
        #region IECContextMenuProvider Members

        public IECContextMenuEntry[] GetMenuItems (string requestor, string controlType, bool isEditingValue, string category, ICustomTypeDescriptor typeDescriptor, IECPropertyValue propertyValue, ECEnumerablePropertyDescriptor propertyDescriptor, object control, ContextMenuEntryArray currentMenuEntries)
            {
            // Create the menu entries in the right context
            ustation.Bentley.MstnPlatformNET.XDataTree.CustomizeDataGrid dataGridControl= control as ustation.Bentley.MstnPlatformNET.XDataTree.CustomizeDataGrid;
            if (dataGridControl==null)
                return null;
           
            if ((dataGridControl.SelectedDataTreeNodes==null) || (dataGridControl.SelectedDataTreeNodes.Count==0))
                return null;

            if (isEditingValue)
                return null;
                
            if (requestor != "ElementParams_Editor")
                return null;

            bool enableEntries = true;

            // If any of the selected items in the list are readonly then show disabled context menu entries
            foreach (XDataTreeNode dataNode in dataGridControl.SelectedDataTreeNodes)
                {
                if (dataNode.Owner.IsReadOnly)
                    enableEntries = false;
                }

            // if we right click outside of any property grid, we show all menu items, allowing you to add anything.
            // if we right click on the header of a particular category grid, we show only the menu items for that category.
            // if we right click on a particular property, we show only the menu item for that particular property.
            BECO.UI.ContextMenuEntryArray menuEntries = new Bentley.ECObjects.UI.ContextMenuEntryArray();

            if ((typeDescriptor == null))
                {
                menuEntries = AddAllMenuItems(currentMenuEntries, propertyDescriptor, dataGridControl, enableEntries);      // outside of all grids            
                }
            else if (typeDescriptor != null) 
                {
                if (category.StartsWith("DTM"))
                    currentMenuEntries.RemoveAt(0);
                return null;
                }

            if (menuEntries!=null)
                return menuEntries.ToArray();
            else
                return null;

            }
        private BECO.UI.ContextMenuEntryArray AddAllMenuItems (
            BECO.UI.ContextMenuEntryArray menuEntries,
            BECO.UI.ECEnumerablePropertyDescriptor propDescr,
            ustation.Bentley.MstnPlatformNET.XDataTree.CustomizeDataGrid control,
            bool enableEntries)
            {
            object firstValue = null;
            // Find the Add menu
            BECO.UI.IECContextMenuEntry addMenu = BECO.UI.ECPropertyPane.FindContextMenuEntryByName (menuEntries, "Add");
            if (addMenu == null)
                return null;

            BECO.UI.ContextMenuEntryArray newMenus = new Bentley.ECObjects.UI.ContextMenuEntryArray ();

            BECO.UI.IECContextMenuEntry dtmMenu = new ContextMenuEntry ("DTMElementExtender_CivilDTM", StringLocalizer.Instance.GetLocalizedString ("TerrainModel"), 0, enableEntries, true, false, addMenu, firstValue, propDescr, control);
            newMenus.Add (dtmMenu);

            BECO.UI.IECContextMenuEntry menuEntry = new ContextMenuEntry ("DTMElementExtender_Display", StringLocalizer.Instance.GetLocalizedString ("Display"), 1100, enableEntries, false, true, dtmMenu, firstValue, propDescr, control);
            newMenus.Add (menuEntry);

            menuEntry = new ContextMenuEntry ("DTMElementExtender_SourceFeatures", StringLocalizer.Instance.GetLocalizedString ("SourceFeatures"), 1000, enableEntries, false, true, dtmMenu, firstValue, propDescr, control);
            newMenus.Add (menuEntry);

            menuEntry = new ContextMenuEntry ("DTMElementExtender_CalculatedFeatures", StringLocalizer.Instance.GetLocalizedString ("CalculatedFeatures"), 1000, enableEntries, false, true, dtmMenu, firstValue, propDescr, control);
            newMenus.Add (menuEntry);

            //menuEntry = new ContextMenuEntry ("DTMElementExtender_Contours", StringLocalizer.Instance.GetLocalizedString ("Contours"), 900, enableEntries, false, false, dtmMenu, firstValue, propDescr, control);
            //newMenus.Add (menuEntry);

            //menuEntry = new ContextMenuEntry ("DTMElementExtender_Triangles", StringLocalizer.Instance.GetLocalizedString ("Triangles"), 800, enableEntries, false, false, dtmMenu, firstValue, propDescr, control);
            //newMenus.Add (menuEntry);

            //menuEntry = new ContextMenuEntry ("DTMElementExtender_TriangleVertices", StringLocalizer.Instance.GetLocalizedString ("TriangleVertices"), 700, enableEntries, false, false, dtmMenu, firstValue, propDescr, control);
            //newMenus.Add (menuEntry);

            //menuEntry = new ContextMenuEntry ("DTMElementExtender_FlowArrowSettings", StringLocalizer.Instance.GetLocalizedString ("FlowArrows"), 600, enableEntries, false, false, dtmMenu, firstValue, propDescr, control);
            //newMenus.Add (menuEntry);

            //menuEntry = new ContextMenuEntry ("DTMElementExtender_LowPointSettings", StringLocalizer.Instance.GetLocalizedString ("LowPoints"), 500, enableEntries, false, false, dtmMenu, firstValue, propDescr, control);
            //newMenus.Add (menuEntry);

            //menuEntry = new ContextMenuEntry ("DTMElementExtender_HighPointSettings", StringLocalizer.Instance.GetLocalizedString ("HighPoints"), 400, enableEntries, false, false, dtmMenu, firstValue, propDescr, control);
            //newMenus.Add (menuEntry);

#if INCLUDE_CATCHMENT
            menuEntry = new ContextMenuEntry ("DTMElementExtender_CatchmentAreaSettings", StringLocalizer.Instance.GetLocalizedString ("CatchmentAreas"), 100, enableEntries, true, false, dtmMenu, firstValue, propDescr, control);
            newMenus.Add (menuEntry);

            menuEntry = new ContextMenuEntry("DTMElementExtender_PondSettings", StringLocalizer.Instance.GetLocalizedString("Ponds"), 100, enableEntries, true, false, dtmMenu, firstValue, propDescr, control);
            newMenus.Add(menuEntry);
#endif

            // Add event handlers for all menus
            foreach (ContextMenuEntry menu in newMenus)
                {
                menu.IsActivated += new ContextMenuEntry.ContextMenuEntryActivatedDelegate (menu_IsActivated);
                }

            return newMenus;

            }

        void menu_IsActivated (object sender)
            {
            // One of the menu entries has been activate
            ContextMenuEntry menu = sender as ContextMenuEntry;
            if (menu == null) return;

            if ((menu.DataGrid == null) || (menu.DataGrid.SelectedDataTreeNodes == null))
                return;

            if (menu.Name == "DTMElementExtender_Remove")
                {
                // Remove 
                string displayLabel;
                int priority;
                bool defaultExpand;

                BECO.Schema.IECClass ecClass = Schema.GetClass (this.ClassName);

                if (menu.PropertyDecriptor != null)
                    {
                    // Property has been select - get the category it's in and then remove all proeprties in the category
                    foreach (BECO.Instance.IECPropertyValue propertyValue in menu.PropertyDecriptor)
                        {
                        BECO.UI.ECPropertyPane.GetCategoryInformation (out displayLabel, out priority, out defaultExpand, propertyValue.Property, ecClass);
                        string accessString = propertyValue.AccessString;

                        System.Collections.Generic.List<string> categoryNames = new List<string> ();

                        if (displayLabel == "Triangle Settings")
                            {
                            categoryNames.Add (GetCategoryDisplayLabel (m_categories[TriangleProperties.CATEGORY_TRIANGLES] as BECO.Instance.IECInstance));
                            }
                        if (displayLabel == "Contour Settings")
                            {
                            categoryNames.Add (GetCategoryDisplayLabel (m_categories[ContourProperties.CATEGORY_CONTOURS] as BECO.Instance.IECInstance));
                            }
                        if (displayLabel == "Source Feature Settings")
                            {
                            categoryNames.Add (GetCategoryDisplayLabel (m_categories[SourceFeatureProperties.CATEGORY_SOURCEFEATURES] as BECO.Instance.IECInstance));
                            }
                        if (displayLabel == "Low Point Settings")
                            {
                            categoryNames.Add (GetCategoryDisplayLabel (m_categories[LowPointProperties.CATEGORY_LOWPOINTS] as BECO.Instance.IECInstance));
                            }
                        if (displayLabel == "High Point Settings")
                            {
                            categoryNames.Add (GetCategoryDisplayLabel (m_categories[HighPointProperties.CATEGORY_HIGHPOINTS] as BECO.Instance.IECInstance));
                            }
                        if (displayLabel == "Flow Arrow Settings")
                            {
                            categoryNames.Add (GetCategoryDisplayLabel (m_categories[FlowArrowProperties.CATEGORY_FLOWARROWS] as BECO.Instance.IECInstance));
                            }
                        if (displayLabel == "Spot Level Settings")
                            {
                            categoryNames.Add (GetCategoryDisplayLabel (m_categories[TriangleVerticeProperties.CATEGORY_TRIANGLEVERTICE] as BECO.Instance.IECInstance));
                            }
#if INCLUDE_CATHMENTS_PONDS
                        if (displayLabel == "Catchment Area Settings")
                            {
                            categoryNames.Add (GetCategoryDisplayLabel (m_categories[CatchmentAreaProperties.CATEGORY_CATCHMENTAREAS] as BECO.Instance.IECInstance));
                            }
                        if (displayLabel == "Pond Settings")
                            {
                            categoryNames.Add(GetCategoryDisplayLabel(m_categories[PondProperties.CATEGORY_PONDS] as BECO.Instance.IECInstance));
                            }
#endif
                        foreach (string categoryName in categoryNames)
                            {
                            foreach (BECO.Instance.IECPropertyValue propertyValue2 in propertyValue.Instance)
                                {
                                string displayLabel2;
                                int priority2;
                                bool defaultExpand2;

                                BECO.UI.ECPropertyPane.GetCategoryInformation (out displayLabel2, out priority2, out defaultExpand2, propertyValue2.Property, propertyValue.Instance.ClassDefinition);

                                if (displayLabel2 == categoryName)
                                    {
                                    menu.DataGrid.AllowRefreshes = false;

                                    bool valueDeleted = BECO.Instance.ECInstanceHelper.RemovePropertyValue (propertyValue2);

                                    menu.DataGrid.AllowRefreshes = true;

                                    if (valueDeleted)
                                        {
                                        BECO.Instance.ECPropertyValueChangedEventArgs args = new Bentley.ECObjects.Instance.ECPropertyValueChangedEventArgs (propertyValue2);
                                        menu.DataGrid.InstanceDataChanged (null, args);
                                        }
                                    }
                                }
                            }
                        break;
                        }
                    }
                }
            else
                {

                System.Collections.Generic.List<BECO.Instance.IECPropertyValue> properties = new List<Bentley.ECObjects.Instance.IECPropertyValue> ();

                // --- adding properties ---
                foreach (XDataTreeNode dataNode in menu.DataGrid.SelectedDataTreeNodes)
                    {
                    if (dataNode.Owner.IsReadOnly)
                        continue;

                    menu.DataGrid.AllowRefreshes = false;

                    BECO.Instance.IECInstance instance = dataNode.GetECInstanceByClassName (this.ClassName);
                    if (instance == null)
                        {
                        IECSchema s_schema = this.Schema;

                        BECO.Schema.IECClass ecClass = s_schema.GetClass (this.ClassName);
                        instance = ecClass.CreateInstance ();
                        dataNode.ECInstanceList.Add (instance);
                        }

                    if (instance != null)
                        {
                        // Which one is it?
                        switch (menu.Name)
                            {
                            case "DTMElementExtender_Display":
                                    {
                                    m_displayCalculatedProperties.IsActivated (instance, properties);
//                                    m_calculatedCategoryContainer.Activate (instance, properties);
                                    m_displaySourceProperties.IsActivated (instance, properties);
//                                    m_sourceFeatureProperties.IsActivated (instance, properties);
                                    break;
                                    }

                            case "DTMElementExtender_SourceFeatures":
                                    {
                                    m_sourceFeatureProperties.IsActivated (instance, properties);
                                    break;
                                    }
                            case "DTMElementExtender_CalculatedFeatures":
                                m_calculatedCategoryContainer.Activate (instance, properties, Constants.TRIANGLES);
                                m_calculatedCategoryContainer.Activate (instance, properties, Constants.CONTOURS);
                                m_calculatedCategoryContainer.Activate (instance, properties, Constants.TRIANGLEVERTICES);
                                m_calculatedCategoryContainer.Activate (instance, properties, Constants.HIGHPOINTS);
                                m_calculatedCategoryContainer.Activate (instance, properties, Constants.LOWPOINTS);
                                m_calculatedCategoryContainer.Activate (instance, properties, Constants.FLOWARROWS);
                                break;
                            case "DTMElementExtender_Triangles":
                                    {
                                    m_calculatedCategoryContainer.Activate (instance, properties, Constants.TRIANGLES);
                                    break;
                                    }
                            case "DTMElementExtender_Contours":
                                    {
                                    m_calculatedCategoryContainer.Activate (instance, properties, Constants.CONTOURS);
                                    break;
                                    }
                            case "DTMElementExtender_TriangleVertices":
                                    {
                                    m_calculatedCategoryContainer.Activate (instance, properties, Constants.TRIANGLEVERTICES);
                                    break;
                                    }
                            case "DTMElementExtender_HighPointSettings":
                                    {
                                    m_calculatedCategoryContainer.Activate (instance, properties, Constants.HIGHPOINTS);
                                    break;
                                    }
                            case "DTMElementExtender_LowPointSettings":
                                    {
                                    m_calculatedCategoryContainer.Activate (instance, properties, Constants.LOWPOINTS);
                                    break;
                                    }
                            case "DTMElementExtender_FlowArrowSettings":
                                    {
                                    m_calculatedCategoryContainer.Activate (instance, properties, Constants.FLOWARROWS);
                                    break;
                                    }
#if INCLUDE_CATCHMENT
                        case "DTMElementExtender_CatchmentAreaSettings":
                                {
                                m_calculatedCategoryContainer.Activate(instance, properties, Constants.CATCHMENTAREAS);
                                break;
                                }
                        case "DTMElementExtender_PondSettings":
                                {
                                m_calculatedCategoryContainer.Activate(instance, properties, Constants.PONDS);
                                break;
                                }
#endif
                            default:
                                    {
                                    break;
                                    }
                            }
                        }
                    dataNode.Owner.Write (dataNode);
                    }

                menu.DataGrid.AllowRefreshes = true;
                if (properties.Count > 0)
                    {
                    foreach (BECO.Instance.IECPropertyValue property in properties)
                        {
                        BECO.Instance.ECPropertyValueChangedEventArgs args = new Bentley.ECObjects.Instance.ECPropertyValueChangedEventArgs (property);
                        menu.DataGrid.InstanceDataChanged (null, args);
                        }
                    }
                }

            menu.DataGrid.RefreshData ();
            }

//        private BECO.Instance.IECPropertyValue GetProperty(System.Collections.Generic.List<BECO.Instance.IECPropertyValue> properties, string accessString)
//            {
//            // Find the right property in the proties list
//            foreach (BECO.Instance.IECPropertyValue property in properties)
//                {
//                if (property.AccessString == accessString)
//                    return property;
//                }

//            return null;
//            }

        private string GetCategoryDisplayLabel (BECO.Instance.IECInstance category)
            {
            // Get a category's display label
            string displayLabel = category["DisplayLabel"].StringValue;
            return displayLabel;
            }
    
        private void ApplyTemplate(Bentley.TerrainModelNET.Element.DTMElement elem, BECO.Instance.IECInstance templateInstance)
            {            
            try
              {
//                // Apply template to the dtm element
                m_displayCalculatedProperties.ApplyProperties(elem, templateInstance);
                m_calculatedCategoryContainer.ApplyProperties(elem, templateInstance);
                m_displaySourceProperties.ApplyProperties(elem, templateInstance);
                m_sourceFeatureProperties.ApplyProperties(elem, templateInstance);
                }
            catch (System.Exception)
                {

                }
            }
                     
        #endregion

        /// <summary>
        /// Private class representing each context menu entry added to the Element Template GUI
        /// </summary>
        /// <author>James.Goode</author>
        /// <date>5/6/2008</date>
        private class ContextMenuEntry : Bentley.ECObjects.UI.ECContextMenuEntry
            {

            public delegate void ContextMenuEntryActivatedDelegate (object sender);
            public event ContextMenuEntryActivatedDelegate IsActivated;

            /*private BECO.Instance.IECPropertyValue*/ object m_firstValue;
            private BECO.UI.ECEnumerablePropertyDescriptor m_propDescr;
            private ustation.Bentley.MstnPlatformNET.XDataTree.CustomizeDataGrid m_dataGrid;

            public ContextMenuEntry (
                string name,
                string displayLabel,
                int priority,
                bool enabled,
                bool breakBefore,
                bool breakAfter,
                Bentley.ECObjects.UI.IECContextMenuEntry parentMenu,
                /*BECO.Instance.IECPropertyValue*/ object firstValue,
                BECO.UI.ECEnumerablePropertyDescriptor propDescr,
                ustation.Bentley.MstnPlatformNET.XDataTree.CustomizeDataGrid control)
                : base (name, displayLabel, priority, enabled, breakBefore, breakAfter, parentMenu)
                {
                m_firstValue = firstValue;
                m_propDescr = propDescr;
                m_dataGrid = control;
                }

            public override void Activated ()
                {
                // Raise out that the menu has just activated
                if (this.IsActivated != null)
                    this.IsActivated (this);
                }

            public ustation.Bentley.MstnPlatformNET.XDataTree.CustomizeDataGrid DataGrid
                {
                get
                    {
                    return m_dataGrid;
                    }
                }

            public /*BECO.Instance.IECPropertyValue*/ object FirstValue
                {
                get
                    {
                    return m_firstValue;
                    }
                }

            public BECO.UI.ECEnumerablePropertyDescriptor PropertyDecriptor
                {
                get
                    {
                    return m_propDescr;
                    }
                }


            }


            #region IApplyElementParamsToHandle Members

        public unsafe bool ApplySymbologyToElement (ustation.Bentley.DgnPlatform.EditElementHandle* ehandle, XDataTreeNode templateNode)
            {
            return ApplyElementParams (ehandle, templateNode, null, false);
            }

        public unsafe bool ProcessApplyElementParamsToHandle (ustation.Bentley.DgnPlatform.EditElementHandle* ehandle, XDataTreeNode templateNode, Bentley.ECObjects.Instance.ECInstanceList originalECInstanceList, bool mustReferenceMatchingTemplate)
            {
            bool elementModified = false;

            elementModified = ApplyElementParams (ehandle, templateNode, originalECInstanceList, mustReferenceMatchingTemplate);

            return elementModified;
            }

            #endregion
        }

    }

