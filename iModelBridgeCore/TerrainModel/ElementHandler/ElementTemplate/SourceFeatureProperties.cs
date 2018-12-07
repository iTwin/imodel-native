using System;
using System.Collections.Generic;
using System.Text;
using BECO = Bentley.ECObjects;
using Bentley.DgnPlatformNET;

namespace Bentley.TerrainModel.ElementTemplate
    {
    public class SourceFeatureProperties : GeneralProperties
        {
        public const string CATEGORY_SOURCEFEATURES = "DTMElementExtender_SourceSettings";

        public const string PREFIX_SOURCEFEATURESBREAKLINE = "SourceBreakline";
        public const string PREFIX_SOURCEFEATURESHOLE = "SourceHole";
        public const string PREFIX_SOURCEFEATURESISLAND = "SourceIsland";
        public const string PREFIX_SOURCEFEATURESVOID = "SourceVoid";
        public const string PREFIX_SOURCEFEATURESBOUNDARY = "SourceBoundary";
        public const string PREFIX_SOURCEFEATURESCONTOUR = "SourceContour";
        public const string PREFIX_SOURCEFEATURESSPOT = "SourceSpot";

        public SourceFeatureProperties() : base ("")
            {
            }

        public override void IsActivated (BECO.Instance.IECInstance instance, List<BECO.Instance.IECPropertyValue> properties)
            {
            Activate (PREFIX_SOURCEFEATURESBREAKLINE, instance, properties);
            Activate (PREFIX_SOURCEFEATURESBOUNDARY, instance, properties);
            Activate (PREFIX_SOURCEFEATURESCONTOUR, instance, properties);
            Activate (PREFIX_SOURCEFEATURESISLAND, instance, properties);
            Activate (PREFIX_SOURCEFEATURESHOLE, instance, properties);
            Activate (PREFIX_SOURCEFEATURESVOID, instance, properties);
            Activate (PREFIX_SOURCEFEATURESSPOT, instance, properties);
            }

        public void Activate (string prefix, BECO.Instance.IECInstance instance, List<BECO.Instance.IECPropertyValue> properties)
            {
            BECO.Instance.IECPropertyValue propertyValue;
            BECO.Instance.IECStructValue baseStructValue;
            BECO.Instance.IECStructValue structValue;

            baseStructValue = instance[prefix] as BECO.Instance.IECStructValue;

            propertyValue = baseStructValue.ContainedValues[prefix + "_Level"];
            if (propertyValue != null)
                {
                    propertyValue.StringValue = System.String.Empty;
                properties.Add (propertyValue);
                }

            structValue = (baseStructValue.ContainedValues[prefix + "_LineStyle"]) as BECO.Instance.IECStructValue;
            propertyValue = structValue.ContainedValues["Value"];
            if (propertyValue != null)
                {
                propertyValue.IntValue = System.Int32.MaxValue - 1;
                properties.Add (propertyValue);
                }

            structValue = (baseStructValue.ContainedValues[prefix + "_LineWeight"]) as BECO.Instance.IECStructValue;
            propertyValue = structValue.ContainedValues["Value"];
            if (propertyValue != null)
                {
                propertyValue.IntValue = -2;
                properties.Add (propertyValue);
                }

            propertyValue = baseStructValue[prefix + "_Transparency"];
            if (propertyValue != null)
                {
                propertyValue.DoubleValue = 0.0;
                properties.Add (propertyValue);
                }

            structValue = (baseStructValue.ContainedValues[prefix + "_Color"]) as BECO.Instance.IECStructValue;
            propertyValue = structValue.ContainedValues["Value"];
            if (propertyValue != null)
                {
                ElementColor color = new ElementColor ((ColorSupports)(ColorSupports.Indexed | ColorSupports.Rgb | ColorSupports.ColorBook));
                color.Source = ColorSource.ByCell;
                structValue = (propertyValue as BECO.Instance.IECStructValue);
                color.PopulateStruct (ref structValue);
                properties.Add (propertyValue);
                }

                if (prefix == PREFIX_SOURCEFEATURESSPOT)
                {
                structValue = baseStructValue[prefix + "_SpotSymbol"] as BECO.Instance.IECStructValue;
                propertyValue = structValue["Type"];
                if (propertyValue != null)
                    {
                    propertyValue.IntValue = 0;
                    properties.Add (propertyValue);
                    }

                propertyValue = structValue["Value"];
                if (propertyValue != null)
                    {
                    propertyValue.StringValue = "";
                    properties.Add (propertyValue);
                    }

                propertyValue = baseStructValue[prefix + "_CellScale"];
                if (propertyValue != null)
                    {
                    propertyValue.DoubleValue = 1.0;
                    properties.Add(propertyValue);
                    }

                propertyValue = baseStructValue.ContainedValues[prefix + "_WantText"];
                if (propertyValue != null)
                    {
                        propertyValue.IntValue = System.Convert.ToInt32(true);
                        properties.Add(propertyValue);
                    }

                propertyValue = baseStructValue.ContainedValues[prefix + "_SpotTextStyle"];
                if (propertyValue != null)
                    {
                    propertyValue.StringValue = "";
                    properties.Add (propertyValue);
                    }

                propertyValue = baseStructValue.ContainedValues[prefix + "_SpotTextPrefix"];
                if (propertyValue != null)
                    {
                    propertyValue.StringValue = "";
                    properties.Add (propertyValue);
                    }

                propertyValue = baseStructValue.ContainedValues[prefix + "_SpotTextSuffix"];
                if (propertyValue != null)
                    {
                    propertyValue.StringValue = "";
                    properties.Add (propertyValue);
                    }       
                }
            }

        private void ApplyProperties (Bentley.TerrainModelNET.Element.DTMSubElement subElement, string prefix, BECO.Instance.IECInstance templateInstance)
            {
            if (subElement == null) return;

            BECO.Instance.IECStructValue structValue = templateInstance[prefix] as BECO.Instance.IECStructValue;
            ApplyProperties (subElement, prefix, structValue);

            Bentley.TerrainModelNET.Element.DTMPointElement pointElement = subElement as Bentley.TerrainModelNET.Element.DTMPointElement;
            if (pointElement != null)
                ApplyCellProperties (pointElement, prefix, structValue);
            subElement.Commit (subElement.GetElement());
            }
        public void ApplyProperties(Bentley.TerrainModelNET.Element.DTMElement elem, BECO.Instance.IECInstance templateInstance)
        {
            ApplyProperties (elem.FeatureBreaklineElement, PREFIX_SOURCEFEATURESBREAKLINE, templateInstance);
            ApplyProperties (elem.FeatureBoundaryElement, PREFIX_SOURCEFEATURESBOUNDARY, templateInstance);
            ApplyProperties (elem.FeatureContourElement, PREFIX_SOURCEFEATURESCONTOUR, templateInstance);
            ApplyProperties (elem.FeatureIslandElement, PREFIX_SOURCEFEATURESISLAND, templateInstance);
            ApplyProperties (elem.FeatureHoleElement, PREFIX_SOURCEFEATURESHOLE, templateInstance);
            ApplyProperties (elem.FeatureVoidElement, PREFIX_SOURCEFEATURESVOID, templateInstance);
            ApplyProperties (elem.FeatureSpotElement, PREFIX_SOURCEFEATURESSPOT, templateInstance);
        //    if (elem.FeatureSpotElement != null)
        //    {
        //        structValue = (baseStructValue.ContainedValues[PREFIX_SOURCEFEATURESSPOT + "_SpotSymbol"]) as BECO.Instance.IECStructValue;
        //        propertyValue = structValue.ContainedValues["Type"];
        //        if (propertyValue != null && !propertyValue.IsNull)
        //            elem.FeatureSpotElement.FeatureSpotPointCellType = propertyValue.IntValue;

        //        propertyValue = structValue.ContainedValues["Value"];
        //        if (propertyValue != null && !propertyValue.IsNull)
        //            elem.FeatureSpotElement.FeatureSpotCellName = propertyValue.StringValue;

        //        propertyValue = baseStructValue.ContainedValues[PREFIX_SOURCEFEATURESSPOT + "_SpotTextStyle"];
        //        if (propertyValue != null && !propertyValue.IsNull)
        //            elem.FeatureSpotElement.FeatureSpotTextStyleID = DTMElementTemplateExtender.GetTextStyleID(propertyValue.StringValue);

        //        propertyValue = baseStructValue.ContainedValues[PREFIX_SOURCEFEATURESSPOT + "_SpotTextPrefix"];
        //        if(propertyValue != null && !propertyValue.IsNull)
        //            elem.FeatureSpotElement.FeatureSpotTextPrefix = propertyValue.StringValue;

        //        propertyValue = baseStructValue.ContainedValues[PREFIX_SOURCEFEATURESSPOT + "_SpotTextSuffix"];
        //        if(propertyValue != null && !propertyValue.IsNull)
        //            elem.FeatureSpotElement.FeatureSpotTextSuffix = propertyValue.StringValue;
        //    }
        }

        }
    }
