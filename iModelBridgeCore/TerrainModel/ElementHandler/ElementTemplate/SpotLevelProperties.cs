using System;
using System.Collections.Generic;
using System.Text;
using BECO = Bentley.ECObjects;
using Bentley.DTM.Element;

namespace Bentley.DTM.ElementTemplate
    {
    public class SpotLevelProperties : ElementTemplateExtenderPropertyProvider
        {

        public const string PREFIX_SPOTLEVEL = "SpotLevel";
        public const string CATEGORY_SPOTLEVEL = "DTMElementExtender_SpotLevelSettings";

        private BECO.Instance.IECInstance m_category = null;
        private GeneralProperties m_generalProperties = null;

        public SpotLevelProperties()
            {
            // Create general properties
            m_generalProperties = new GeneralProperties(PREFIX_SPOTLEVEL, true, true, false, false, false);
            }

        public override void AddProperties(Bentley.ECObjects.Schema.IECClass ecClass, Bentley.ECObjects.Instance.IECInstance category)
            {
            // Add the general properties
            m_generalProperties.AddProperties(ecClass, m_category);

            CreatePointCellProp (PREFIX_SPOTLEVEL + "_PointCellName", StringLocalizer.Instance.GetLocalizedString ("PointCellName"), ecClass, m_category, 600);

            // Display Spot text - T/F
            CreateBooleanProp(PREFIX_SPOTLEVEL + "_DisplaySpotText", StringLocalizer.Instance.GetLocalizedString("DisplayText"), ecClass, m_category, 600);

            // Create Text Style entries     
            CreateTextStyleProp(PREFIX_SPOTLEVEL + "_SpotTextStyles", StringLocalizer.Instance.GetLocalizedString("TextStyle"), ecClass, m_category, 700);
            
            // Spot text prefix - string
            CreateStringProp(PREFIX_SPOTLEVEL + "_SpotTextPrefix", StringLocalizer.Instance.GetLocalizedString("TextPrefix"), ecClass, m_category, 800);

            // Spot text Suffix - string
            CreateStringProp(PREFIX_SPOTLEVEL + "_SpotTextSuffix", StringLocalizer.Instance.GetLocalizedString("TextSuffix"), ecClass, m_category, 900);

            }

        public override void CreateCategories(System.Collections.Hashtable categories)
            {
            // Create spot level category
            m_category = Bentley.ECObjects.UI.ECPropertyPane.CreateCategory(
             CATEGORY_SPOTLEVEL,
             StringLocalizer.Instance.GetLocalizedString("TriangleVerticesSettings"),
             StringLocalizer.Instance.GetLocalizedString("TriangleVerticesSettings"),
             BECO.UI.ECPropertyPane.CategorySortPriorityHigh - 500);
            categories.Add(CATEGORY_SPOTLEVEL, m_category);
            }

        public override void IsActivated(Bentley.ECObjects.Instance.IECInstance instance, List<Bentley.ECObjects.Instance.IECPropertyValue> properties)
            {
            instance[DisplayTerrainModelFeaturesProperties.PREFIX_SPOTSDISPLAYED].IntValue = 1;
            // Set the general properties
            m_generalProperties.IsActivated(instance, properties);

            // Set up spot level properties         
            BECO.Instance.IECArrayValue arrayProperty = null;
            BECO.Instance.IECStructValue valueStruct = null;
            BECO.Instance.IECPropertyValue propertyValue = null;
            int arrayIndex = 0;

            arrayProperty = instance[PREFIX_SPOTLEVEL + "_PointCellName"] as BECO.Instance.IECArrayValue;
            arrayIndex = arrayProperty.Count;
            if (arrayIndex >= 1) return;

            valueStruct = (arrayProperty[arrayIndex]) as BECO.Instance.IECStructValue;
            propertyValue = valueStruct["Type"];
            if (propertyValue != null)
                {
                propertyValue.IntValue = 0;
                properties.Add (propertyValue);
                }

            propertyValue = valueStruct["Value"];
            if (propertyValue != null)
                {
                propertyValue.StringValue = "";
                properties.Add(propertyValue);
                }

            propertyValue = instance[PREFIX_SPOTLEVEL + "_DisplaySpotText"];
            if (propertyValue != null)
                {
                propertyValue.IntValue = System.Convert.ToInt32(true);
                properties.Add(propertyValue);
                }

            propertyValue = instance[PREFIX_SPOTLEVEL + "_SpotTextStyles"];
            if (propertyValue != null)
            {
                propertyValue.StringValue = "";
                properties.Add(propertyValue);
            }         

            propertyValue = instance[PREFIX_SPOTLEVEL + "_SpotTextPrefix"];
            if (propertyValue != null)
                {
                propertyValue.StringValue = "";
                properties.Add(propertyValue);
                }

            propertyValue = instance[PREFIX_SPOTLEVEL + "_SpotTextSuffix"];
            if (propertyValue != null)
                {
                propertyValue.StringValue = "";
                properties.Add(propertyValue);
                }        
            }

        public void ApplyProperties(Bentley.DTM.Element.DTMElement elem, Bentley.ECObjects.Instance.IECInstance templateInstance)
            {
            // Apply general properties
            if (!templateInstance[DisplayTerrainModelFeaturesProperties.PREFIX_SPOTSDISPLAYED].IsNull)
                elem.WantSpots = System.Convert.ToBoolean (templateInstance[DisplayTerrainModelFeaturesProperties.PREFIX_SPOTSDISPLAYED].IntValue);
            else
                elem.WantSpots = false;

            if ((elem.SpotsElement == null) || (elem.SpotsElement== null)) return;

            m_generalProperties.ApplyProperties(elem.SpotsElement, templateInstance);

            // Apply spot level properties
            elem.SpotsElement.SpotPointCellType = GetArrayProperty (templateInstance, PREFIX_SPOTLEVEL + "_PointCellName", "Type").IntValue;
            elem.SpotsElement.SpotCellName = GetArrayProperty (templateInstance, PREFIX_SPOTLEVEL + "_PointCellName", "Value").StringValue;
            elem.SpotsElement.WantSpotText = System.Convert.ToBoolean(templateInstance[PREFIX_SPOTLEVEL + "_DisplaySpotText"].IntValue);

            List<Bentley.Internal.MicroStation.TextStyle> textStyles = Bentley.Internal.MicroStation.TextStyle.GetAll(true);
            foreach (Bentley.Internal.MicroStation.TextStyle textStyle in textStyles)
            {
                if (textStyle.Name == templateInstance[PREFIX_SPOTLEVEL + "_SpotTextStyles"].StringValue)
                {
                    elem.SpotsElement.SpotTextStyleID = (int)textStyle.ID;
                    break;
                }
                else
                    elem.SpotsElement.SpotTextStyleID = 0;
            }

            elem.SpotsElement.SpotTextPrefix = templateInstance[PREFIX_SPOTLEVEL + "_SpotTextPrefix"].StringValue;
            elem.SpotsElement.SpotTextSuffix = templateInstance[PREFIX_SPOTLEVEL + "_SpotTextSuffix"].StringValue;

            }
        }
    }
