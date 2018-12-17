using System;
using System.Collections.Generic;
using System.Text;
using BECO = Bentley.ECObjects;
using Bentley.DTM.Element;

namespace Bentley.DTM.ElementTemplate
    {
    public class DisplayTerrainModelFeatureProperties : ElementTemplateExtenderPropertyProvider, ICanApplyProperties
        {
        public const string PREFIX_TERRAINMODLEFEATURESDISPLAY = "DisplayTerrainModelFeatures";

        public const string PREFIX_LINEARFEATURESBREAKLINEDISPLAYED = "LinearFeatureBreaklineDisplayed";
        public const string PREFIX_LINEARFEATURESHOLEDISPLAYED = "LinearFeatureHoleDisplayed";
        public const string PREFIX_LINEARFEATURESISLANDDISPLAYED = "LinearFeatureIslandDisplayed";
        public const string PREFIX_LINEARFEATURESVOIDDISPLAYED = "LinearFeatureVoidDisplayed";
        public const string PREFIX_LINEARFEATURESBOUNDARYDISPLAYED = "LinearFeatureBoundaryDisplayed";
        public const string PREFIX_LINEARFEATURESCONTOURDISPLAYED = "LinearFeatureContourDisplayed";
        public const string PREFIX_LINEARFEATURESSPOTDISPLAYED = "LinearFeatureSpotDisplayed";

        public const string TERRAINMODELFEATURESCATEGORY_DISPLAY = "DTMElementExtender_TerrainModelFeaturesSettings";

        private enum PropertyPriority
        {
            BreakLine = 100,
            Boundary = 200,
            Contour = 300,
            Island = 400,
            Hole = 500,
            Void = 600,
            Spot = 700,
        }

        private BECO.Instance.IECInstance m_terrainModelFeaturesCategory = null;

        private bool m_activated;

        public DisplayTerrainModelFeatureProperties()
            {
            // Do nothing - display properties
            }

        public bool Activated
            {
            get { return m_activated; }
            }

        public override void AddProperties (Bentley.ECObjects.Schema.IECClass ecClass, Bentley.ECObjects.Instance.IECInstance category)
            {
                CreateBooleanProp(PREFIX_LINEARFEATURESBREAKLINEDISPLAYED, StringLocalizer.Instance.GetLocalizedString("Breakline"), ecClass, m_terrainModelFeaturesCategory, (int)PropertyPriority.BreakLine);
                CreateBooleanProp(PREFIX_LINEARFEATURESBOUNDARYDISPLAYED, StringLocalizer.Instance.GetLocalizedString("Boundary"), ecClass, m_terrainModelFeaturesCategory, (int)PropertyPriority.Boundary);
                CreateBooleanProp(PREFIX_LINEARFEATURESCONTOURDISPLAYED, StringLocalizer.Instance.GetLocalizedString("ImportedContours"), ecClass, m_terrainModelFeaturesCategory, (int)PropertyPriority.Contour);
                CreateBooleanProp(PREFIX_LINEARFEATURESISLANDDISPLAYED, StringLocalizer.Instance.GetLocalizedString("Island"), ecClass, m_terrainModelFeaturesCategory, (int)PropertyPriority.Island);
                CreateBooleanProp(PREFIX_LINEARFEATURESHOLEDISPLAYED, StringLocalizer.Instance.GetLocalizedString("Hole"), ecClass, m_terrainModelFeaturesCategory, (int)PropertyPriority.Hole);
                CreateBooleanProp(PREFIX_LINEARFEATURESVOIDDISPLAYED, StringLocalizer.Instance.GetLocalizedString("Void"), ecClass, m_terrainModelFeaturesCategory, (int)PropertyPriority.Void);
                CreateBooleanProp(PREFIX_LINEARFEATURESSPOTDISPLAYED, StringLocalizer.Instance.GetLocalizedString("Spot"), ecClass, m_terrainModelFeaturesCategory, (int)PropertyPriority.Spot);
            }

        public void ApplyProperties (Bentley.DTM.Element.DTMElement elem, BECO.Instance.IECInstance templateInstance)
            {
                if (!templateInstance[PREFIX_LINEARFEATURESBREAKLINEDISPLAYED].IsNull)
                {
                    elem.WantBreaklineFeatures = System.Convert.ToBoolean(templateInstance[PREFIX_LINEARFEATURESBREAKLINEDISPLAYED].IntValue);
                }
                if (!templateInstance[PREFIX_LINEARFEATURESBOUNDARYDISPLAYED].IsNull)
                {
                    elem.WantBoundaryFeatures = System.Convert.ToBoolean(templateInstance[PREFIX_LINEARFEATURESBOUNDARYDISPLAYED].IntValue);
                }
                if (!templateInstance[PREFIX_LINEARFEATURESCONTOURDISPLAYED].IsNull)
                {
                    elem.WantContourFeatures = System.Convert.ToBoolean(templateInstance[PREFIX_LINEARFEATURESCONTOURDISPLAYED].IntValue);
                }
                if (!templateInstance[PREFIX_LINEARFEATURESISLANDDISPLAYED].IsNull)
                {
                    elem.WantIslandFeatures = System.Convert.ToBoolean(templateInstance[PREFIX_LINEARFEATURESISLANDDISPLAYED].IntValue);
                }
                if (!templateInstance[PREFIX_LINEARFEATURESHOLEDISPLAYED].IsNull)
                {
                    elem.WantHoleFeatures = System.Convert.ToBoolean(templateInstance[PREFIX_LINEARFEATURESHOLEDISPLAYED].IntValue);
                }
                if (!templateInstance[PREFIX_LINEARFEATURESVOIDDISPLAYED].IsNull)
                {
                    elem.WantVoidFeatures = System.Convert.ToBoolean(templateInstance[PREFIX_LINEARFEATURESVOIDDISPLAYED].IntValue);
                }
                if (!templateInstance[PREFIX_LINEARFEATURESSPOTDISPLAYED].IsNull)
                {
                    elem.WantFeatureSpots = System.Convert.ToBoolean(templateInstance[PREFIX_LINEARFEATURESSPOTDISPLAYED].IntValue);
                }
            }

        public override void CreateCategories (System.Collections.Hashtable categories)
            {
            m_terrainModelFeaturesCategory = BECO.UI.ECPropertyPane.CreateCategory(
                TERRAINMODELFEATURESCATEGORY_DISPLAY,
                StringLocalizer.Instance.GetLocalizedString("DisplayTerrainModelFeatures"),
                StringLocalizer.Instance.GetLocalizedString("DisplayTerrainModelFeatures"),
                BECO.UI.ECPropertyPane.CategorySortPriorityHigh - 200);
            categories.Add(TERRAINMODELFEATURESCATEGORY_DISPLAY, m_terrainModelFeaturesCategory);

            }

        public override void IsActivated (BECO.Instance.IECInstance instance, List<BECO.Instance.IECPropertyValue> properties)
            {
            BECO.Instance.IECPropertyValue propertyValue;

            m_activated = true;

            propertyValue = instance[PREFIX_LINEARFEATURESBREAKLINEDISPLAYED];
            if (propertyValue != null)
            {
                propertyValue.IntValue = System.Convert.ToInt32(false);
                properties.Add(propertyValue);
            }

            propertyValue = instance[PREFIX_LINEARFEATURESBOUNDARYDISPLAYED];
            if (propertyValue != null)
            {
                propertyValue.IntValue = System.Convert.ToInt32(false);
                properties.Add(propertyValue);
            }

            propertyValue = instance[PREFIX_LINEARFEATURESCONTOURDISPLAYED];
            if (propertyValue != null)
            {
                propertyValue.IntValue = System.Convert.ToInt32(false);
                properties.Add(propertyValue);
            }

            propertyValue = instance[PREFIX_LINEARFEATURESISLANDDISPLAYED];
            if (propertyValue != null)
            {
                propertyValue.IntValue = System.Convert.ToInt32(false);
                properties.Add(propertyValue);
            }

            propertyValue = instance[PREFIX_LINEARFEATURESHOLEDISPLAYED];
            if (propertyValue != null)
            {
                propertyValue.IntValue = System.Convert.ToInt32(false);
                properties.Add(propertyValue);
            }

            propertyValue = instance[PREFIX_LINEARFEATURESVOIDDISPLAYED];
            if (propertyValue != null)
            {
                propertyValue.IntValue = System.Convert.ToInt32(false);
                properties.Add(propertyValue);
            }

            propertyValue = instance[PREFIX_LINEARFEATURESSPOTDISPLAYED];
            if (propertyValue != null)
            {
                propertyValue.IntValue = System.Convert.ToInt32(false);
                properties.Add(propertyValue);
            }
            }
        }
    }
