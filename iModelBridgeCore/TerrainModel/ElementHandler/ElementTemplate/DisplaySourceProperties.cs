using System;
using System.Collections.Generic;
using System.Text;
using BECO = Bentley.ECObjects;

namespace Bentley.TerrainModel.ElementTemplate
    {
    public class DisplaySourceProperties : ElementTemplateExtenderPropertyProvider, ICanApplyProperties
        {
        public const string PREFIX_SOURCEDISPLAY = "DisplaySource";

        public const string PREFIX_LINEARFEATURESBREAKLINEDISPLAYED = "LinearFeatureBreaklineDisplayed";
        public const string PREFIX_LINEARFEATURESHOLEDISPLAYED = "LinearFeatureHoleDisplayed";
        public const string PREFIX_LINEARFEATURESISLANDDISPLAYED = "LinearFeatureIslandDisplayed";
        public const string PREFIX_LINEARFEATURESVOIDDISPLAYED = "LinearFeatureVoidDisplayed";
        public const string PREFIX_LINEARFEATURESBOUNDARYDISPLAYED = "LinearFeatureBoundaryDisplayed";
        public const string PREFIX_LINEARFEATURESCONTOURDISPLAYED = "LinearFeatureContourDisplayed";
        public const string PREFIX_LINEARFEATURESSPOTDISPLAYED = "LinearFeatureSpotDisplayed";

        public const string CATEGORY_SOURCEDISPLAY = "DTMElementExtender_DisplaySourceSettings";

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

        private bool m_activated;

        public DisplaySourceProperties()
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
            SetVisibleProperty (elem, templateInstance, PREFIX_LINEARFEATURESBREAKLINEDISPLAYED, elem.FeatureBreaklineElement);
            SetVisibleProperty (elem, templateInstance, PREFIX_LINEARFEATURESBOUNDARYDISPLAYED, elem.FeatureBoundaryElement);
            SetVisibleProperty (elem, templateInstance, PREFIX_LINEARFEATURESCONTOURDISPLAYED, elem.FeatureContourElement);
            SetVisibleProperty (elem, templateInstance, PREFIX_LINEARFEATURESISLANDDISPLAYED, elem.FeatureIslandElement);
            SetVisibleProperty (elem, templateInstance, PREFIX_LINEARFEATURESHOLEDISPLAYED, elem.FeatureHoleElement);
            SetVisibleProperty (elem, templateInstance, PREFIX_LINEARFEATURESVOIDDISPLAYED, elem.FeatureVoidElement);
            SetVisibleProperty (elem, templateInstance, PREFIX_LINEARFEATURESSPOTDISPLAYED, elem.FeatureSpotElement);
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
