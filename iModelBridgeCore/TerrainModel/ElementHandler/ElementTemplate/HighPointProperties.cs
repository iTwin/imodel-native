using System;
using System.Collections.Generic;
using System.Text;
using BECO = Bentley.ECObjects;
using Bentley.DgnPlatformNET;

namespace Bentley.TerrainModel.ElementTemplate
    {
    public class HighPointProperties : GeneralProperties, ICanApplyProperties
        {
        public const string PREFIX_HIGHPOINTS = "HighPoints";
        public const string CATEGORY_HIGHPOINTS = "DTMElementExtender_HighPointSettings";

        public HighPointProperties () : base (PREFIX_HIGHPOINTS)
            {
            }

        public override void IsActivated (Bentley.ECObjects.Instance.IECInstance instance, List<Bentley.ECObjects.Instance.IECPropertyValue> properties)
            {
            BECO.Instance.IECStructValue structValue = null;
            BECO.Instance.IECStructValue valueStruct = null;
            BECO.Instance.IECPropertyValue propertyValue = null;

            structValue = instance[PREFIX_HIGHPOINTS] as BECO.Instance.IECStructValue;

            propertyValue = structValue[PREFIX_HIGHPOINTS + "_Level"];
            if (propertyValue != null)
                {
                propertyValue.StringValue = System.String.Empty;
                properties.Add(propertyValue);
                }

            valueStruct = (structValue[PREFIX_HIGHPOINTS + "_LineStyle"]) as BECO.Instance.IECStructValue;
            propertyValue = valueStruct["Value"];
            if (propertyValue != null)
                {
                propertyValue.IntValue = System.Int32.MaxValue - 1;
                properties.Add(propertyValue);
                }

            valueStruct = (structValue[PREFIX_HIGHPOINTS + "_LineWeight"]) as BECO.Instance.IECStructValue;
            propertyValue = valueStruct["Value"];
            if (propertyValue != null)
                {
                propertyValue.IntValue = -2;
                properties.Add(propertyValue);
                }

            valueStruct = (structValue[PREFIX_HIGHPOINTS + "_Color"]) as BECO.Instance.IECStructValue;
            propertyValue = valueStruct["Value"];
            if (propertyValue != null)
                {
                ElementColor color = new ElementColor((ColorSupports)(ColorSupports.Indexed | ColorSupports.Rgb | ColorSupports.ColorBook));
                color.Source = ColorSource.ByCell;
                valueStruct = (propertyValue as BECO.Instance.IECStructValue);
                color.PopulateStruct (ref valueStruct);
                properties.Add(propertyValue);
                }

            // If Transparency is required.
            propertyValue = structValue[PREFIX_HIGHPOINTS + "_Transparency"];
            if (propertyValue != null)
                {
                propertyValue.DoubleValue = 0.0;
                properties.Add(propertyValue);
                }

            valueStruct = valueStruct = structValue[PREFIX_HIGHPOINTS + "_PointCellName"] as BECO.Instance.IECStructValue;
            propertyValue = valueStruct["Value"];
            if (propertyValue != null)
                {
                propertyValue.StringValue = "";
                properties.Add (propertyValue);
                }

            propertyValue = valueStruct["Type"];
            if (propertyValue != null)
                {
                propertyValue.IntValue = 0;
                properties.Add (propertyValue);
                }

            propertyValue = structValue[PREFIX_HIGHPOINTS + "_CellSize"];
            if (propertyValue != null)
                {
                propertyValue.DoubleValue = 1.0;
                properties.Add (propertyValue);
                }

            propertyValue = structValue[PREFIX_HIGHPOINTS + "_WantText"];
            if (propertyValue != null)
                {
                propertyValue.IntValue = System.Convert.ToInt32(true);
                properties.Add (propertyValue);
                }

            propertyValue = structValue[PREFIX_HIGHPOINTS + "_TextStyle"];
            if (propertyValue != null)
                {
                propertyValue.StringValue = "";
                properties.Add (propertyValue);
                }

            propertyValue = structValue[PREFIX_HIGHPOINTS + "_PrefixText"];
            if (propertyValue != null)
                {
                propertyValue.StringValue = "";
                properties.Add (propertyValue);
                }

            propertyValue = structValue[PREFIX_HIGHPOINTS + "_SuffixText"];
            if (propertyValue != null)
                {
                propertyValue.StringValue = "";
                properties.Add (propertyValue);
                }  
            }

        public void ApplyProperties (Bentley.TerrainModelNET.Element.DTMElement elem, Bentley.ECObjects.Instance.IECInstance templateInstance)
            {
            Bentley.TerrainModelNET.Element.DTMHighPointElement highPointElement = elem.HighPointElement;

            if (highPointElement == null) return;

            BECO.Instance.IECStructValue structValue = templateInstance[PREFIX_HIGHPOINTS] as BECO.Instance.IECStructValue;
            ApplyProperties (highPointElement, structValue);
            ApplyCellProperties(highPointElement, structValue);
            highPointElement.Commit(elem);
            }

        }
    }
