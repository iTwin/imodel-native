using System;
using System.Collections.Generic;
using System.Text;
using BECO = Bentley.ECObjects;
using Bentley.DgnPlatformNET;

namespace Bentley.TerrainModel.ElementTemplate
    {
    public class FlowArrowProperties : GeneralProperties, ICanApplyProperties
        {
        public const string PREFIX_FLOWARROWS = "FlowArrows";
        public const string CATEGORY_FLOWARROWS = "DTMElementExtender_FlowArrowsSettings";

        public FlowArrowProperties () : base (PREFIX_FLOWARROWS)
            {
            }

        public override void IsActivated(Bentley.ECObjects.Instance.IECInstance instance, List<Bentley.ECObjects.Instance.IECPropertyValue> properties)
            {
            BECO.Instance.IECStructValue tempStruct;
            BECO.Instance.IECStructValue structValue;
            BECO.Instance.IECPropertyValue propertyValue = null;

            structValue = instance[PREFIX_FLOWARROWS] as BECO.Instance.IECStructValue;

            propertyValue = structValue[PREFIX_FLOWARROWS + "_Level"];
            if (propertyValue != null)
                {
                propertyValue.StringValue = System.String.Empty;
                properties.Add(propertyValue);
                }

            tempStruct = (structValue[PREFIX_FLOWARROWS + "_LineStyle"]) as BECO.Instance.IECStructValue;
            propertyValue = tempStruct["Value"];
            if (propertyValue != null)
                {
                propertyValue.IntValue = System.Int32.MaxValue - 1;
                properties.Add(propertyValue);
                }

            tempStruct = (structValue[PREFIX_FLOWARROWS + "_LineWeight"]) as BECO.Instance.IECStructValue;
            propertyValue = tempStruct["Value"];
            if (propertyValue != null)
                {
                propertyValue.IntValue = -2;
                properties.Add(propertyValue);
                }

            tempStruct = (structValue[PREFIX_FLOWARROWS + "_Color"]) as BECO.Instance.IECStructValue;
            propertyValue = tempStruct["Value"];
            if (propertyValue != null)
                {
                ElementColor color = new ElementColor((ColorSupports)(ColorSupports.Indexed | ColorSupports.Rgb | ColorSupports.ColorBook));
                color.Source = ColorSource.ByCell;
                tempStruct = (propertyValue as BECO.Instance.IECStructValue);
                color.PopulateStruct (ref tempStruct);
                properties.Add(propertyValue);
                }

            // If Transparency is required.
            propertyValue = structValue[PREFIX_FLOWARROWS + "_Transparency"];
            if (propertyValue != null)
                {
                propertyValue.DoubleValue = 0.0;
                properties.Add(propertyValue);
                }

            tempStruct = structValue[PREFIX_FLOWARROWS + "_PointCellName"] as BECO.Instance.IECStructValue;

            propertyValue = tempStruct["Value"];
            if (propertyValue != null)
                {
                propertyValue.StringValue = "";
                properties.Add(propertyValue);
                }

            propertyValue = tempStruct["Type"];
            if (propertyValue != null)
                {
                propertyValue.IntValue = 0;
                properties.Add(propertyValue);
                }

            propertyValue = structValue[PREFIX_FLOWARROWS + "_CellSize"];
            if (propertyValue != null)
                {
                propertyValue.DoubleValue = 1.0;
                properties.Add(propertyValue);
                }
            }

        public void ApplyProperties(Bentley.TerrainModelNET.Element.DTMElement elem, Bentley.ECObjects.Instance.IECInstance templateInstance)
            {
            Bentley.TerrainModelNET.Element.DTMFlowArrowElement flowArrowElement = elem.FlowArrowElement;

            if (flowArrowElement == null) return;

            BECO.Instance.IECStructValue structValue = templateInstance[PREFIX_FLOWARROWS] as BECO.Instance.IECStructValue;
            ApplyProperties (flowArrowElement, structValue);
            ApplyCellProperties (flowArrowElement, structValue);
            flowArrowElement.Commit (elem);
            }

        }
    }
