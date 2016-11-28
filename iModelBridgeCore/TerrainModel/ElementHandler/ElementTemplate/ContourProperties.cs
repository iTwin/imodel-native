using System;
using System.Collections.Generic;
using System.Text;
using BECO = Bentley.ECObjects;
using Bentley.DgnPlatformNET;
using Bentley.ECObjects.Instance;

namespace Bentley.TerrainModel.ElementTemplate
    {

    public class ContourProperties : GeneralProperties, ICanApplyProperties
        {
        public const string CATEGORY_CONTOURS = "DTMElementExtender_ContourSettings";

        public const string PREFIX_CONTOURS = "Contour";
        public const string PREFIX_MAJORCONTOUR = "ContourMajor";
        public const string PREFIX_MINORCONTOUR = "ContourMinor";

        public const string PREFIX_MAJORDEPRESSIONCONTOUR = "ContourMajorDepression";
        public const string PREFIX_MINORDEPRESSIONCONTOUR = "ContourMinorDepression";

        private ContourTextStyleProperties m_minorTextProperties = null;
        private ContourTextStyleProperties m_majorTextProperties = null;

        public ContourProperties()
            : base("")
            {
            m_majorTextProperties = new ContourTextStyleProperties(PREFIX_MAJORCONTOUR);
            m_minorTextProperties = new ContourTextStyleProperties(PREFIX_MINORCONTOUR);
            }

        public override void IsActivated(BECO.Instance.IECInstance instance, List<BECO.Instance.IECPropertyValue> properties)
            {
            Activate (PREFIX_CONTOURS, instance, properties);
            Activate (PREFIX_MAJORCONTOUR, instance, properties);
            Activate (PREFIX_MINORCONTOUR, instance, properties);
            }

        private void Activate (string prefix, BECO.Instance.IECInstance instance, List<BECO.Instance.IECPropertyValue> properties)
            {
            BECO.Instance.IECPropertyValue propertyValue;
            BECO.Instance.IECStructValue baseStructValue;
            BECO.Instance.IECStructValue depressionStructValue;
            BECO.Instance.IECStructValue structValue;

            baseStructValue = instance[PREFIX_CONTOURS] as BECO.Instance.IECStructValue;

            if (prefix == "Contour")
                {
                propertyValue = baseStructValue.ContainedValues[prefix + "_MaxSlopeOption"];
                if (propertyValue != null)
                    {
                    propertyValue.IntValue = 0;
                    properties.Add(propertyValue);
                    }
                
                propertyValue = baseStructValue.ContainedValues[prefix + "_MaxSlopeValue"];
                if (propertyValue != null)
                    {
                    propertyValue.DoubleValue = 0.0;
                    properties.Add(propertyValue);
                    }

                propertyValue = baseStructValue.ContainedValues[prefix + "_ContourLabelPrecision"];
                if (propertyValue != null)
                    {
                    propertyValue.IntValue = 2;
                    properties.Add(propertyValue);
                    }

                propertyValue = baseStructValue.ContainedValues[prefix + "_SmoothingOptions"];
                if (propertyValue != null)
                    {
                    propertyValue.IntValue = 0;
                    properties.Add (propertyValue);
                    }

                propertyValue = baseStructValue.ContainedValues[prefix + "_SplineSmoothingFactor"];
                if (propertyValue != null)
                    {
                    propertyValue.DoubleValue = 5.0;
                    properties.Add (propertyValue);
                    }

                propertyValue = baseStructValue.ContainedValues[prefix + "_MajorInterval"];
                if (propertyValue != null)
                    {
                    propertyValue.DoubleValue = 10.0;
                    properties.Add(propertyValue);
                    }

                propertyValue = baseStructValue.ContainedValues[prefix + "_MinorInterval"];
                if (propertyValue != null)
                    {
                    propertyValue.DoubleValue = 1.0;
                    properties.Add(propertyValue);
                    }

                }
            else
                {
                BECO.Instance.IECStructValue tempStruct = baseStructValue[prefix] as BECO.Instance.IECStructValue;

                propertyValue = tempStruct[prefix + "_Display"];
                if (propertyValue != null)
                    {
                    propertyValue.IntValue = 1;
                    properties.Add (propertyValue);
                    }

                propertyValue = tempStruct[prefix + "_Level"];
                if (propertyValue != null)
                    {
                    propertyValue.StringValue = System.String.Empty;
                    properties.Add (propertyValue);
                    }

                structValue = tempStruct[prefix + "_LineStyle"] as BECO.Instance.IECStructValue;
                propertyValue = structValue["Value"];
                if (propertyValue != null)
                    {
                    propertyValue.IntValue = System.Int32.MaxValue - 1;
                    properties.Add (propertyValue);
                    }

                structValue = tempStruct[prefix + "_LineWeight"] as BECO.Instance.IECStructValue;
                propertyValue = structValue["Value"];
                if (propertyValue != null)
                    {
                    propertyValue.IntValue = -2;
                    properties.Add (propertyValue);
                    }

                propertyValue = tempStruct[prefix + "_Transparency"];
                if (propertyValue != null)
                    {
                    propertyValue.DoubleValue = 0.0;
                    properties.Add (propertyValue);
                    }

                structValue = tempStruct[prefix + "_Color"] as BECO.Instance.IECStructValue;
                propertyValue = structValue["Value"];
                if (propertyValue != null)
                    {
                    ElementColor color = new ElementColor ((ColorSupports)(ColorSupports.Indexed | ColorSupports.Rgb | ColorSupports.ColorBook));
                    color.Source = ColorSource.ByCell;
                    structValue = (propertyValue as BECO.Instance.IECStructValue);
                    color.PopulateStruct (ref structValue);
                    properties.Add (propertyValue);
                    }

                // Depressions.
                depressionStructValue = tempStruct[prefix + "Depression"] as BECO.Instance.IECStructValue;

                structValue = depressionStructValue[prefix + "Depression_LineStyle"] as BECO.Instance.IECStructValue;
                propertyValue = structValue["Value"];
                if (propertyValue != null)
                    {
                    propertyValue.IntValue = System.Int32.MaxValue - 1;
                    properties.Add (propertyValue);
                    }

                structValue = depressionStructValue[prefix + "Depression_LineWeight"] as BECO.Instance.IECStructValue;
                propertyValue = structValue["Value"];
                if (propertyValue != null)
                    {
                    propertyValue.IntValue = -2;
                    properties.Add (propertyValue);
                    }

                structValue = depressionStructValue[prefix + "Depression_Color"] as BECO.Instance.IECStructValue;
                propertyValue = structValue["Value"];
                if (propertyValue != null)
                    {
                    ElementColor color = new ElementColor ((ColorSupports)(ColorSupports.Indexed | ColorSupports.Rgb | ColorSupports.ColorBook));
                    color.Source = ColorSource.ByCell;
                    structValue = (propertyValue as BECO.Instance.IECStructValue);
                    color.PopulateStruct (ref structValue);
                    }

                propertyValue = tempStruct.ContainedValues[prefix + "_DisplayText"];
                if (propertyValue != null)
                    {
                    propertyValue.IntValue = 0;
                    properties.Add (propertyValue);
                    }

                propertyValue = tempStruct.ContainedValues[prefix + "_TextLevel"];
                if (propertyValue != null)
                    {
                    propertyValue.StringValue = "Default";
                    properties.Add (propertyValue);
                    }

                propertyValue = tempStruct.ContainedValues[prefix + "_TextStyles"];
                if (propertyValue != null)
                    {
                    propertyValue.StringValue = "";
                    properties.Add (propertyValue);
                    }



                propertyValue = tempStruct.ContainedValues[prefix + "_TextInterval"];
                if (propertyValue != null)
                    {
                    propertyValue.DoubleValue = 10;
                    properties.Add (propertyValue);
                    }
                }
            }

        public void ApplyProperties(Bentley.TerrainModelNET.Element.DTMElement elem, BECO.Instance.IECInstance templateInstance)
            {
            Bentley.TerrainModelNET.Element.DTMContourElement majorContourElement = elem.MajorContourElement;
            Bentley.TerrainModelNET.Element.DTMContourElement minorContourElement = elem.MinorContourElement;
            BECO.Instance.IECStructValue majorStructValue;
            BECO.Instance.IECStructValue minorStructValue;
            BECO.Instance.IECStructValue majorDepressionStructValue;
            BECO.Instance.IECStructValue minorDepressionStructValue;
            BECO.Instance.IECStructValue generalStructValue;

            if (majorContourElement == null && minorContourElement == null) return;

            generalStructValue = templateInstance[PREFIX_CONTOURS] as BECO.Instance.IECStructValue;
            majorStructValue = generalStructValue[PREFIX_MAJORCONTOUR] as BECO.Instance.IECStructValue;
            minorStructValue = generalStructValue[PREFIX_MINORCONTOUR] as BECO.Instance.IECStructValue;
            majorDepressionStructValue = majorStructValue.ContainedValues[PREFIX_MAJORDEPRESSIONCONTOUR] as BECO.Instance.IECStructValue;
            minorDepressionStructValue = minorStructValue.ContainedValues[PREFIX_MINORDEPRESSIONCONTOUR] as BECO.Instance.IECStructValue;

            if (majorContourElement != null)
                {
                ApplyProperties (majorContourElement, PREFIX_MAJORCONTOUR, majorStructValue);
                ApplyGeneralProperties (majorContourElement, generalStructValue, PREFIX_MAJORCONTOUR, majorStructValue, "Major", majorDepressionStructValue, PREFIX_MAJORDEPRESSIONCONTOUR);
                majorContourElement.Commit (elem);
                }

            if (minorContourElement != null)
                {
                ApplyProperties (minorContourElement, PREFIX_MINORCONTOUR, minorStructValue);
                ApplyGeneralProperties (minorContourElement, generalStructValue, PREFIX_MINORCONTOUR, minorStructValue, "Minor", minorDepressionStructValue, PREFIX_MINORDEPRESSIONCONTOUR);
                minorContourElement.Commit (elem);
                }
            }

        void ApplyGeneralProperties (Bentley.TerrainModelNET.Element.DTMContourElement contourElement, BECO.Instance.IECStructValue generalStructValue, string prefix, BECO.Instance.IECStructValue contourStructValue, string majorMinorString, BECO.Instance.IECStructValue depressionStructValue, string depressionPrefix)
            {
            double uorsPerMeter = contourElement.GetElement().DgnModel.GetModelInfo ().UorPerMeter;

            // Apply contour interval.
            BECO.Instance.IECPropertyValue property = generalStructValue [PREFIX_CONTOURS + "_" + majorMinorString + "Interval"];
            if (property != null && !property.IsNull)
                contourElement.ContourInterval = property.DoubleValue * uorsPerMeter;

            property = generalStructValue[PREFIX_CONTOURS + "_SmoothingOptions"];
            if (property != null && !property.IsNull)
                contourElement.ContourSmoothing = (Bentley.TerrainModelNET.Element.DTMContourSmoothingMethod) property.IntValue;

            property = generalStructValue[PREFIX_CONTOURS + "_SplineSmoothingFactor"];
            if (property != null && !property.IsNull)
                contourElement.SmoothingFactor = property.DoubleValue;

            // Apply text interval.
            property = contourStructValue[prefix + "_TextInterval"];
            if (property != null && !property.IsNull)
                contourElement.TextInterval = property.DoubleValue * uorsPerMeter;

            //Apply Annotation.
            property = contourStructValue[prefix + "_DisplayText"];
            if (property != null && !property.IsNull)
                contourElement.DrawTextOption = (Bentley.TerrainModelNET.Element.DTMContourElement.ContourDrawTextOption)(property.IntValue);

            property = contourStructValue[prefix + "_TextLevel"];
            if (property != null && !property.IsNull)
                {
                LevelHandle level;

                if (property.StringValue == "")
                    level = contourElement.GetElement ().DgnModel.GetFileLevelCache ().GetLevel (64);
                else
                    level = contourElement.GetElement ().DgnModel.GetFileLevelCache ().GetLevelByName (property.StringValue, true);

                if (level == null || !level.IsValid)
                    level = contourElement.GetElement ().DgnModel.GetFileLevelCache ().CreateLevel (property.StringValue);
                if (level != null && level.IsValid)
                    contourElement.TextLevelId = level.LevelId;
                }

            // Apply text style.
            property = contourStructValue[prefix + "_TextStyles"];
            if (property != null && !property.IsNull)
                contourElement.TextStyle = DgnTextStyle.GetByName (property.StringValue, contourElement.GetElement ().DgnModelRef.GetDgnFile ());

            // Apply Max Slope Option.
            property = generalStructValue[PREFIX_CONTOURS + "_MaxSlopeOption"];
            if (property != null && !property.IsNull)
                contourElement.MaxSlopeOption = (byte)property.IntValue;

            // Apply Max Slope value.
            property = generalStructValue[PREFIX_CONTOURS + "_MaxSlopeValue"];
            if (property != null && !property.IsNull)
                contourElement.MaxSlopeValue = property.DoubleValue;

            // Apply Contour Label Precision.
            property = generalStructValue[PREFIX_CONTOURS + "_ContourLabelPrecision"];
            if (property != null && !property.IsNull)
                contourElement.ContourLabelPrecision = property.IntValue;

            // Apply Depression Symbology.
            BECO.Instance.IECStructValue structVal = (depressionStructValue.ContainedValues[depressionPrefix + "_Color"]) as BECO.Instance.IECStructValue;
            property = structVal.ContainedValues["Value"];
            if (property != null && !property.IsNull)
                {
                BECO.Instance.IECStructValue propertyValue = property as BECO.Instance.IECStructValue;
                if (!propertyValue["Source"].IsNull)
                    {
                    ElementColor color = new ElementColor ((ColorSupports)(ColorSupports.Indexed | ColorSupports.Rgb | ColorSupports.ColorBook));
                    color.LoadFromStruct (propertyValue, contourElement.GetElement ().DgnModel);

                    contourElement.DepressionColor = (uint)color.GetRawColorIndex (contourElement.GetElement ().DgnModel);
                    }
                }

            structVal = (depressionStructValue.ContainedValues[depressionPrefix + "_LineStyle"]) as BECO.Instance.IECStructValue;
            property = structVal.ContainedValues["Value"];
            if (property != null && !property.IsNull)
                {
                string styleValue = property.StringValue;
                int lineStyle = GetLineStyleIndex (contourElement.GetElement (), styleValue);
                contourElement.DepressionLineStyle = lineStyle;
                }

            structVal = (depressionStructValue.ContainedValues[depressionPrefix + "_LineWeight"]) as BECO.Instance.IECStructValue;
            property = structVal.ContainedValues["Value"];
            if (property != null && !property.IsNull)
                {
                if (property.IntValue == -1)
                    contourElement.DepressionWeight = 0xffffffff;
                else if (property.IntValue == -2)
                    contourElement.DepressionWeight = 0xfffffffe;
                else
                    contourElement.DepressionWeight = System.Convert.ToUInt32 (property.IntValue);
                }
            }

        }
    }
