using System;
using System.Collections.Generic;
using System.Text;
using BECO = Bentley.ECObjects;
using Bentley.DgnPlatformNET;

namespace Bentley.TerrainModel.ElementTemplate
    {
    public class TriangleVerticeProperties : GeneralProperties, ICanApplyProperties
        {

        public const string PREFIX_TRIANGLEVERTICE = "TriangleVertice";
        public const string CATEGORY_TRIANGLEVERTICE = "DTMElementExtender_TriangleVerticeSettings";

        public TriangleVerticeProperties() : base (PREFIX_TRIANGLEVERTICE)
            {
            }

        public override void IsActivated(Bentley.ECObjects.Instance.IECInstance instance, List<Bentley.ECObjects.Instance.IECPropertyValue> properties)
            {
            // Set up spot level properties         
            BECO.Instance.IECStructValue valueStruct = null;
            BECO.Instance.IECPropertyValue propertyValue = null;

            BECO.Instance.IECStructValue structValue = instance[PREFIX_TRIANGLEVERTICE] as BECO.Instance.IECStructValue;

            propertyValue = structValue[PREFIX_TRIANGLEVERTICE + "_Level"];
            if (propertyValue != null)
                {
                propertyValue.StringValue = System.String.Empty;
                properties.Add(propertyValue);
                }

            valueStruct = (structValue[PREFIX_TRIANGLEVERTICE + "_Color"]) as BECO.Instance.IECStructValue;
            propertyValue = valueStruct["Value"];
            if (propertyValue != null)
                {
                ElementColor color = new ElementColor((ColorSupports)(ColorSupports.Indexed | ColorSupports.Rgb | ColorSupports.ColorBook));
                color.Source = ColorSource.ByCell;
                valueStruct = (propertyValue as BECO.Instance.IECStructValue);
                color.PopulateStruct (ref valueStruct);
                properties.Add(propertyValue);
                }

            valueStruct = (structValue[PREFIX_TRIANGLEVERTICE + "_LineStyle"]) as BECO.Instance.IECStructValue;
            propertyValue = valueStruct["Value"];
            if (propertyValue != null)
                {
                propertyValue.IntValue = System.Int32.MaxValue - 1;
                properties.Add(propertyValue);
                }

            valueStruct = (structValue[PREFIX_TRIANGLEVERTICE + "_LineWeight"]) as BECO.Instance.IECStructValue;
            propertyValue = valueStruct["Value"];
            if (propertyValue != null)
                {
                propertyValue.IntValue = -2;
                properties.Add(propertyValue);
                }


            // If Transparency is required.
            propertyValue = structValue[PREFIX_TRIANGLEVERTICE + "_Transparency"];
            if (propertyValue != null)
                {
                propertyValue.DoubleValue = 0.0;
                properties.Add(propertyValue);
                }

            valueStruct = structValue[PREFIX_TRIANGLEVERTICE + "_PointCellName"] as BECO.Instance.IECStructValue;

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

            propertyValue = structValue[PREFIX_TRIANGLEVERTICE + "_CellSize"];
            if (propertyValue != null)
                {
                propertyValue.DoubleValue = 1.0;
                properties.Add(propertyValue);
                }

            propertyValue = structValue[PREFIX_TRIANGLEVERTICE + "_DisplaySpotText"];
            if (propertyValue != null)
                {
                propertyValue.IntValue = System.Convert.ToInt32(true);
                properties.Add(propertyValue);
                }

            propertyValue = structValue[PREFIX_TRIANGLEVERTICE + "_SpotTextStyles"];
            if (propertyValue != null)
            {
                propertyValue.StringValue = "";
                properties.Add(propertyValue);
            }

            propertyValue = structValue[PREFIX_TRIANGLEVERTICE + "_SpotTextPrefix"];
            if (propertyValue != null)
                {
                propertyValue.StringValue = "";
                properties.Add(propertyValue);
                }

            propertyValue = structValue[PREFIX_TRIANGLEVERTICE + "_SpotTextSuffix"];
            if (propertyValue != null)
                {
                propertyValue.StringValue = "";
                properties.Add(propertyValue);
                }        
            }

        public void ApplyProperties(Bentley.TerrainModelNET.Element.DTMElement elem, Bentley.ECObjects.Instance.IECInstance templateInstance)
            {
            Bentley.TerrainModelNET.Element.DTMSpotElement spotsElement = elem.SpotsElement;

            if (spotsElement == null) return;

            BECO.Instance.IECStructValue structValue = templateInstance[PREFIX_TRIANGLEVERTICE] as BECO.Instance.IECStructValue;
            ApplyProperties (spotsElement, structValue);
            ApplyCellProperties (spotsElement, structValue);
            spotsElement.Commit (elem);
            }
        }
    }
