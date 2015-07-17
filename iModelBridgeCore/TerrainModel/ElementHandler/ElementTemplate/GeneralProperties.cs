using System;
using System.Collections.Generic;
using System.Text;
using BECO = Bentley.ECObjects;
using Bentley.DgnPlatformNET;

namespace Bentley.TerrainModel.ElementTemplate
    {

    public class GeneralProperties : ElementTemplateExtenderPropertyProvider
        {
        private string m_prefix = string.Empty;

        public GeneralProperties (string prefix)
            : base ()
            {
            m_prefix = prefix;
            }

        public void ApplyProperties (Bentley.TerrainModelNET.Element.DTMSubElement element, BECO.Instance.IECStructValue structValue)
            {
            ApplyProperties (element, m_prefix, structValue);
            }

        protected System.Int32 GetLineStyleIndex (Bentley.TerrainModelNET.Element.DTMElement dtmElement, string lineStyleString)
            {
            Int32 byLevel = (Int32)Bentley.DgnPlatformNET.LsKnownStyleNumber.ByLevel;
            Int32 byCell = (Int32)Bentley.DgnPlatformNET.LsKnownStyleNumber.ByCell;
            Int32 lineStyleValue = 0;

            if (1 == lineStyleString.Length && (lineStyleString[0] >= '0') &&
                (lineStyleString[0] <= '7'))
                {
                lineStyleValue = (Int32)(lineStyleString[0] - '0');
                }
            else if (0 == string.Compare (lineStyleString, byLevel.ToString ()))
                {
                lineStyleValue = byLevel;
                }
            else if (0 == string.Compare (lineStyleString, byCell.ToString ()))
                {
                lineStyleValue = byCell;
                }
            else
                {
                LsEntry ls = dtmElement.DgnModel.GetDgnFile ().GetLineStyleMap ().GetLineStyleEntry (lineStyleString);
                if (ls != null)
                    lineStyleValue = ls.StyleNumber;
                }

            return lineStyleValue;
            }

        public void ApplyProperties (Bentley.TerrainModelNET.Element.DTMSubElement element, string prefix, BECO.Instance.IECStructValue structValue)
            {
            // Apply the properties to the correct element
            if (element == null) return;
            BECO.Instance.IECStructValue structVal;
            BECO.Instance.IECPropertyValue property;

            structVal = (structValue.ContainedValues[prefix + "_Color"]) as BECO.Instance.IECStructValue;
            property = structVal.ContainedValues["Value"];
            if (property != null && !property.IsNull)
                {
                BECO.Instance.IECStructValue propertyValue = property as BECO.Instance.IECStructValue;
                if (!propertyValue["Source"].IsNull)
                    {
                    ElementColor color = new ElementColor ((ColorSupports)(ColorSupports.Indexed | ColorSupports.Rgb | ColorSupports.ColorBook));
                    color.LoadFromStruct (propertyValue, element.GetElement ().DgnModel);

                    element.Color = (uint)color.GetRawColorIndex (element.GetElement ().DgnModel);
                    }
                }

            structVal = (structValue.ContainedValues[prefix + "_LineStyle"]) as BECO.Instance.IECStructValue;
            property = structVal.ContainedValues["Value"];
            if (property != null && !property.IsNull)
                {
                string styleValue = property.StringValue;
                int lineStyle = GetLineStyleIndex (element.GetElement (), styleValue);
                element.LineStyle = lineStyle;
                }

            structVal = (structValue.ContainedValues[prefix + "_LineWeight"]) as BECO.Instance.IECStructValue;
            property = structVal.ContainedValues["Value"];
            if (property != null && !property.IsNull)
                {
                if (property.IntValue == -1)
                    element.Weight = 0xffffffff;
                else if (property.IntValue == -2)
                    element.Weight = 0xfffffffe;
                else
                    element.Weight = System.Convert.ToUInt32 (property.IntValue);
                }

            property = structValue.ContainedValues[prefix + "_Level"];

            if (property != null && !property.IsNull)
                {
                LevelHandle level;

                if (property.StringValue == "")
                    level = element.GetElement ().DgnModel.GetFileLevelCache ().GetLevel (64);
                else
                    level = element.GetElement ().DgnModel.GetFileLevelCache ().GetLevelByName (property.StringValue, true);

                if (level == null || !level.IsValid)
                    level = element.GetElement ().DgnModel.GetFileLevelCache ().CreateLevel (property.StringValue);
                if (level != null && level.IsValid)
                    element.LevelId = level.LevelId;  // ToDo need to work out if Level doesn't exist.
                }

            // If Transparency is required.
            property = structValue[prefix + "_Transparency"];
            if (property != null && !property.IsNull)
                element.Transparency = property.DoubleValue / 100;

            }
        public void ApplyCellProperties (Bentley.TerrainModelNET.Element.DTMPointElement element, BECO.Instance.IECStructValue structValue)
            {
            ApplyCellProperties (element, m_prefix, structValue);
            }

        public void ApplyCellProperties (Bentley.TerrainModelNET.Element.DTMPointElement element, string prefix, BECO.Instance.IECStructValue structValue)
            {
            BECO.Instance.IECStructValue structVal;
            BECO.Instance.IECPropertyValue property;

            structVal = (structValue.FindPropertyValue (prefix + "_PointCellName", false, false, false)) as BECO.Instance.IECStructValue;
            if (structVal == null)
                structVal = (structValue.FindPropertyValue (prefix + "_SpotSymbol", false, false, false)) as BECO.Instance.IECStructValue;
            if (structVal != null)
                {
                property = structVal.ContainedValues["Type"];
                if (property != null && !property.IsNull)
                    element.CellType = property.IntValue;

                property = structVal.ContainedValues["Value"];
                if (property != null && !property.IsNull)
                    element.CellName = property.StringValue;
                }

            property = structValue.FindPropertyValue (prefix + "_TextStyle", false, false, false);
            if (property == null)
                property = structValue.FindPropertyValue (prefix + "_TextStyles", false, false, false);
            if (property == null)
                property = structValue.FindPropertyValue (prefix + "_SpotTextStyle", false, false, false);
            if (property == null)
                property = structValue.FindPropertyValue (prefix + "_SpotTextStyles", false, false, false);
            if (property != null && !property.IsNull)
                element.TextStyle = DgnTextStyle.GetByName (property.StringValue, element.GetElement ().DgnModelRef.GetDgnFile ());

            property = structValue.FindPropertyValue (prefix + "_PrefixText", false, false, false);
            if (property == null)
                property = structValue.FindPropertyValue (prefix + "_SpotPrefixText", false, false, false);
            if (property == null)
                property = structValue.FindPropertyValue (prefix + "_SpotTextPrefix", false, false, false);
            if (property != null && !property.IsNull)
                element.TextPrefix = property.StringValue;

            property = structValue.FindPropertyValue (prefix + "_SuffixText", false, false, false);
            if (property == null)
                property = structValue.FindPropertyValue (prefix + "_SpotSuffixText", false, false, false);
            if (property == null)
                property = structValue.FindPropertyValue (prefix + "_SpotTextSuffix", false, false, false);
            if (property != null && !property.IsNull)
                element.TextSuffix = property.StringValue;

            property = structValue.FindPropertyValue (prefix + "_CellSize", false, false, false);
            if (property != null && !property.IsNull)
                {
                Bentley.GeometryNET.DPoint3d point = new Bentley.GeometryNET.DPoint3d (property.DoubleValue, property.DoubleValue, property.DoubleValue);
                element.CellSize = point;
                }

            property = structValue.FindPropertyValue (prefix + "_WantText", false, false, false);
            if (property == null)
                property = structValue.FindPropertyValue (prefix + "_DisplaySpotText", false, false, false);
            if (property != null && !property.IsNull)
                element.DisplayText = System.Convert.ToBoolean (property.IntValue);
            }

        public override void IsActivated (BECO.Instance.IECInstance instance, List<BECO.Instance.IECPropertyValue> properties)
            {
            BECO.Instance.IECArrayValue arrayProperty;
            int arrayIndex;
            BECO.Instance.IECStructValue valueStruct;
            BECO.Instance.IECPropertyValue propertyValue;

            arrayProperty = instance[m_prefix + "_Level"] as BECO.Instance.IECArrayValue;
            arrayIndex = arrayProperty.Count;
            if (arrayIndex >= 1) return;
            valueStruct = (arrayProperty[arrayIndex]) as BECO.Instance.IECStructValue;
            propertyValue = valueStruct["Value"];
            if (propertyValue != null)
                {
                propertyValue.StringValue = System.String.Empty;
                properties.Add (propertyValue);
                }

            arrayProperty = instance[m_prefix + "_Color"] as BECO.Instance.IECArrayValue;
            arrayIndex = arrayProperty.Count;

            valueStruct = (arrayProperty[arrayIndex]) as BECO.Instance.IECStructValue;
            propertyValue = valueStruct["Value"];
            if (propertyValue != null)
                {
                ElementColor color = new ElementColor ((ColorSupports)(ColorSupports.Indexed | ColorSupports.Rgb | ColorSupports.ColorBook));
                color.Source = ColorSource.ByCell;
                valueStruct = (propertyValue as BECO.Instance.IECStructValue);
                color.PopulateStruct (ref valueStruct);
                properties.Add (propertyValue);
                }

            arrayProperty = instance[m_prefix + "_LineStyle"] as BECO.Instance.IECArrayValue;
            arrayIndex = arrayProperty.Count;

            valueStruct = (arrayProperty[arrayIndex]) as BECO.Instance.IECStructValue;
            propertyValue = valueStruct["Value"];
            if (propertyValue != null)
                {
                propertyValue.IntValue = System.Int32.MaxValue - 1;
                properties.Add (propertyValue);
                }

            arrayProperty = instance[m_prefix + "_LineWeight"] as BECO.Instance.IECArrayValue;
            arrayIndex = arrayProperty.Count;

            valueStruct = (arrayProperty[arrayIndex]) as BECO.Instance.IECStructValue;
            propertyValue = valueStruct["Value"];
            if (propertyValue != null)
                {
                propertyValue.IntValue = -2;
                properties.Add (propertyValue);
                }
            }
        }

    }
