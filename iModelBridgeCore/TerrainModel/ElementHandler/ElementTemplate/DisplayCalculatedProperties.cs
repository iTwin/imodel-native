using System;
using System.Collections.Generic;
using System.Text;
using BECO = Bentley.ECObjects;

namespace Bentley.TerrainModel.ElementTemplate
    {
    public class DisplayCalculatedProperties : ElementTemplateExtenderPropertyProvider, ICanApplyProperties
        {
        public const string PREFIX_TERRAINMODLEFEATURESDISPLAY = "DisplayTerrainModelFeatures";

        public const string PREFIX_TRIANGLESDISPLAYED = "TrianglesDisplayed";
        public const string PREFIX_LINEARFEATURESDISPLAYED = "FeaturesDisplayed";
        public const string PREFIX_CONTOURSDISPLAYED = "ContoursDisplayed";
        public const string PREFIX_TRIANGLEVERTICEDISPLAYED = "TriangleVerticeDisplayed";
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

        private bool m_activated;

        public DisplayCalculatedProperties()
            {
            // Do nothing - display properties
            }

        public bool Activated
            {
            get { return m_activated; }
            }

        private void SetVisibleProperty (Bentley.TerrainModelNET.Element.DTMElement elem, BECO.Instance.IECInstance templateInstance, string propertyName, Bentley.TerrainModelNET.Element.DTMSubElement subElement)
            {
            if (subElement != null)
                {
                if (!templateInstance[propertyName].IsNull)
                    {
                    bool newVis = System.Convert.ToBoolean (templateInstance[propertyName].IntValue);

                    if (subElement.Visible != newVis)
                        {
                        subElement.Visible = newVis;
                        subElement.Commit (elem);
                        }
                    }
                }
            }

          public void ApplyProperties (Bentley.TerrainModelNET.Element.DTMElement elem, BECO.Instance.IECInstance templateInstance)
            {
            SetVisibleProperty (elem, templateInstance, PREFIX_TRIANGLESDISPLAYED, elem.TrianglesElement);
            SetVisibleProperty (elem, templateInstance, PREFIX_CONTOURSDISPLAYED, elem.MinorContourElement);
            SetVisibleProperty (elem, templateInstance, PREFIX_CONTOURSDISPLAYED, elem.MajorContourElement);
            SetVisibleProperty (elem, templateInstance, PREFIX_TRIANGLEVERTICEDISPLAYED, elem.SpotsElement);
            SetVisibleProperty (elem, templateInstance, PREFIX_FLOWARROWSDISPLAYED, elem.FlowArrowElement);
            SetVisibleProperty (elem, templateInstance, PREFIX_LOWPOINTSDISPLAYED, elem.LowPointElement);
            SetVisibleProperty (elem, templateInstance, PREFIX_HIGHPOINTSDISPLAYED, elem.HighPointElement);
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

            propertyValue = instance[PREFIX_TRIANGLEVERTICEDISPLAYED];
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

#if INCLUDE_CATCHMENT
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
