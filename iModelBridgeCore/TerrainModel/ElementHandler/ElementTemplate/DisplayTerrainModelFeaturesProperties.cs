using System;
using System.Collections.Generic;
using System.Text;
using BECO = Bentley.ECObjects;
using Bentley.DTM.Element;

namespace Bentley.DTM.ElementTemplate
    {
    public class DisplayTerrainModelFeaturesProperties : ElementTemplateExtenderPropertyProvider, ICanApplyProperties
        {
        public const string PREFIX_TERRAINMODLEFEATURESDISPLAY = "DisplayTerrainModelFeatures";

        public const string PREFIX_TRIANGLESDISPLAYED = "TrianglesDisplayed";
        public const string PREFIX_LINEARFEATURESDISPLAYED = "FeaturesDisplayed";
        public const string PREFIX_CONTOURSDISPLAYED = "ContoursDisplayed";
        public const string PREFIX_SPOTSDISPLAYED = "SpotsDisplayed";
        public const string PREFIX_FLOWARROWSDISPLAYED = "FlowArrowsDisplayed";
        public const string PREFIX_LOWPOINTSDISPLAYED = "LowPointsDisplayed";
        public const string PREFIX_HIGHPOINTSDISPLAYED = "HighPointsDisplayed";
        public const string PREFIX_CATCHMENTAREASDISPLAYED = "CatchmentAreasDisplayed";
        public const string PREFIX_PONDSDISPLAYED = "PondsDisplayed";
        public const string CATEGORY_TERRAINMODELFEATURESDISPLAY = "DTMElementExtender_DisplayTerrainModelFeaturesSettings"; 

        private enum PropertyPriority
        {
            Contours = 100,
            Triangles = 200,
            TriangleVertices = 300,
            FlowArrows = 500,
            LowPoints = 600,
            HighPoints = 700,
            CatchmentAreas = 800,
            Pond = 900,
            TerrainModelFeatures = 1000
        }

        private BECO.Instance.IECInstance m_terrainModelFeaturesCategory = null;

        private bool m_activated;

        public DisplayTerrainModelFeaturesProperties()
            {
            // Do nothing - display properties
            }

        public bool Activated
            {
            get { return m_activated; }
            }

        public override void AddProperties (Bentley.ECObjects.Schema.IECClass ecClass, Bentley.ECObjects.Instance.IECInstance category)
            {
                CreateBooleanProp(PREFIX_CONTOURSDISPLAYED, StringLocalizer.Instance.GetLocalizedString("Contours"), ecClass, category, (int)PropertyPriority.Contours);
                CreateBooleanProp(PREFIX_TRIANGLESDISPLAYED, StringLocalizer.Instance.GetLocalizedString("Triangles"), ecClass, category, (int)PropertyPriority.Triangles);
                CreateBooleanProp(PREFIX_SPOTSDISPLAYED, StringLocalizer.Instance.GetLocalizedString("TriangleVertices"), ecClass, category, (int)PropertyPriority.TriangleVertices);
                CreateBooleanProp(PREFIX_FLOWARROWSDISPLAYED, StringLocalizer.Instance.GetLocalizedString("FlowArrows"), ecClass, category, (int)PropertyPriority.FlowArrows);
                CreateBooleanProp(PREFIX_LOWPOINTSDISPLAYED, StringLocalizer.Instance.GetLocalizedString("LowPoints"), ecClass, category, (int)PropertyPriority.LowPoints);
                CreateBooleanProp(PREFIX_HIGHPOINTSDISPLAYED, StringLocalizer.Instance.GetLocalizedString("HighPoints"), ecClass, category, (int)PropertyPriority.HighPoints);
#if INCLUDE_CATCHMENTS_PONDS
                CreateBooleanProp(PREFIX_CATCHMENTAREASDISPLAYED, StringLocalizer.Instance.GetLocalizedString("CatchmentAreas"), ecClass, category, (int)PropertyPriority.CatchmentAreas);
                CreateBooleanProp(PREFIX_PONDSDISPLAYED, StringLocalizer.Instance.GetLocalizedString("Ponds"), ecClass, category, (int)PropertyPriority.Pond);
#endif
            }

        public void ApplyProperties (Bentley.DTM.Element.DTMElement elem, BECO.Instance.IECInstance templateInstance)
            {
            if (!templateInstance[PREFIX_TRIANGLESDISPLAYED].IsNull)
                elem.WantTriangles = System.Convert.ToBoolean (templateInstance[PREFIX_TRIANGLESDISPLAYED].IntValue);
            else
                elem.WantTriangles = false;

            if(!templateInstance[PREFIX_CONTOURSDISPLAYED].IsNull)
                elem.WantContours = System.Convert.ToBoolean (templateInstance[PREFIX_CONTOURSDISPLAYED].IntValue);
            else
                elem.WantContours = false;

            if(!templateInstance[PREFIX_SPOTSDISPLAYED].IsNull)
                elem.WantSpots = System.Convert.ToBoolean (templateInstance[PREFIX_SPOTSDISPLAYED].IntValue);
            else
                elem.WantSpots = false;
            
            if(!templateInstance[PREFIX_FLOWARROWSDISPLAYED].IsNull)
                elem.WantFlowArrows = System.Convert.ToBoolean (templateInstance[PREFIX_FLOWARROWSDISPLAYED].IntValue);
            else
                elem.WantFlowArrows = false;

            if(!templateInstance[PREFIX_LOWPOINTSDISPLAYED].IsNull)
                elem.WantLowPoints = System.Convert.ToBoolean (templateInstance[PREFIX_LOWPOINTSDISPLAYED].IntValue);
            else
                elem.WantLowPoints = false;

            if(!templateInstance[PREFIX_HIGHPOINTSDISPLAYED].IsNull)
                elem.WantHighPoints = System.Convert.ToBoolean (templateInstance[PREFIX_HIGHPOINTSDISPLAYED].IntValue);
            else
                elem.WantHighPoints = false;

#if INCLUDE_CATCHMENTS_PONDS
            if (!templateInstance[PREFIX_CATCHMENTAREASDISPLAYED].IsNull)
                elem.WantCatchmentAreas = System.Convert.ToBoolean(templateInstance[PREFIX_CATCHMENTAREASDISPLAYED].IntValue);
            else
                elem.WantCatchmentAreas = false;

            if (!templateInstance[PREFIX_PONDSDISPLAYED].IsNull)
                elem.WantPonds = System.Convert.ToBoolean(templateInstance[PREFIX_PONDSDISPLAYED].IntValue);
            else
                elem.WantPonds = false;
#endif
            }

        public override void CreateCategories (System.Collections.Hashtable categories)
            {
            m_terrainModelFeaturesCategory = BECO.UI.ECPropertyPane.CreateCategory (
                CATEGORY_TERRAINMODELFEATURESDISPLAY,
                StringLocalizer.Instance.GetLocalizedString ("CalculatedFeaturesDisplay"),
                StringLocalizer.Instance.GetLocalizedString("CalculatedFeaturesDisplay"),
                BECO.UI.ECPropertyPane.CategorySortPriorityHigh - 100);
            categories.Add(CATEGORY_TERRAINMODELFEATURESDISPLAY, m_terrainModelFeaturesCategory);
            }

        public override void IsActivated (BECO.Instance.IECInstance instance, List<BECO.Instance.IECPropertyValue> properties)
            {
            BECO.Instance.IECPropertyValue propertyValue;

            m_activated = true;

            propertyValue = instance[PREFIX_TRIANGLESDISPLAYED];
            if (propertyValue != null)
                {
                propertyValue.IntValue = System.Convert.ToInt32 (true);
                properties.Add (propertyValue);
                }

            propertyValue = instance[PREFIX_CONTOURSDISPLAYED];
            if (propertyValue != null)
                {
                propertyValue.IntValue = System.Convert.ToInt32 (true);
                properties.Add (propertyValue);
                }

            propertyValue = instance[PREFIX_SPOTSDISPLAYED];
            if (propertyValue != null)
                {
                propertyValue.IntValue = System.Convert.ToInt32 (false);
                properties.Add (propertyValue);
                }

            propertyValue = instance[PREFIX_FLOWARROWSDISPLAYED];
            if (propertyValue != null)
                {
                propertyValue.IntValue = System.Convert.ToInt32 (false);
                properties.Add (propertyValue);
                }

            propertyValue = instance[PREFIX_LOWPOINTSDISPLAYED];
            if (propertyValue != null)
                {
                propertyValue.IntValue = System.Convert.ToInt32 (false);
                properties.Add (propertyValue);
                }

            propertyValue = instance[PREFIX_HIGHPOINTSDISPLAYED];
            if (propertyValue != null)
                {
                propertyValue.IntValue = System.Convert.ToInt32 (false);
                properties.Add (propertyValue);
                }

#if INCLUDE_CATCHMENTS_PONDS
            propertyValue = instance[PREFIX_CATCHMENTAREASDISPLAYED];
                if (propertyValue != null)
                {
                propertyValue.IntValue = System.Convert.ToInt32(false);
                properties.Add(propertyValue);
                }

            propertyValue = instance[PREFIX_PONDSDISPLAYED];
                if (propertyValue != null)
                {
                propertyValue.IntValue = System.Convert.ToInt32(false);
                properties.Add(propertyValue);
                }
#endif
            }
        }
    }
