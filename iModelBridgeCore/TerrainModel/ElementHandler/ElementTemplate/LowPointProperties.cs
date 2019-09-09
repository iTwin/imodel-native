using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using BECO = Bentley.ECObjects;
using Bentley.DgnPlatformNET;

namespace Bentley.TerrainModel.ElementTemplate
    {
    public class LowPointProperties : GeneralProperties, ICanApplyProperties
        {
        public const string PREFIX_LOWPOINTS = "LowPoints";
        public const string CATEGORY_LOWPOINTS = "DTMElementExtender_LowPointSettings";

        public LowPointProperties () : base (PREFIX_LOWPOINTS)
            {
            }

        public override void IsActivated (Bentley.ECObjects.Instance.IECInstance instance, List<Bentley.ECObjects.Instance.IECPropertyValue> properties)
            {
            BECO.Instance.IECStructValue structValue = null;
            BECO.Instance.IECPropertyValue propertyValue = null;

            structValue = instance[PREFIX_LOWPOINTS] as BECO.Instance.IECStructValue;

            Debug.Assert (structValue != null);
            propertyValue = structValue[PREFIX_LOWPOINTS + "_Level"];
            if (propertyValue != null)
                {
                propertyValue.StringValue = System.String.Empty;
                properties.Add(propertyValue);
                }

            BECO.Instance.IECStructValue valueStruct = (structValue[PREFIX_LOWPOINTS + "_LineStyle"]) as BECO.Instance.IECStructValue;
            Debug.Assert (valueStruct != null);
            propertyValue = valueStruct["Value"];
            if (propertyValue != null)
                {
                propertyValue.IntValue = System.Int32.MaxValue - 1;
                properties.Add(propertyValue);
                }

            valueStruct = (structValue[PREFIX_LOWPOINTS + "_LineWeight"]) as BECO.Instance.IECStructValue;
            Debug.Assert (valueStruct != null);
            propertyValue = valueStruct["Value"];
            if (propertyValue != null)
                {
                propertyValue.IntValue = -2;
                properties.Add(propertyValue);
                }

            valueStruct = (structValue[PREFIX_LOWPOINTS + "_Color"]) as BECO.Instance.IECStructValue;
            Debug.Assert (valueStruct != null);
            propertyValue = valueStruct["Value"];
            if (propertyValue != null)
                {
                ElementColor color = new ElementColor((ColorSupports)(ColorSupports.Indexed | ColorSupports.Rgb | ColorSupports.ColorBook))
                {
                    Source = ColorSource.ByCell
                };
                    valueStruct = (propertyValue as BECO.Instance.IECStructValue);
                color.PopulateStruct (ref valueStruct);
                properties.Add(propertyValue);
                }

            // If Transparency is required.
            propertyValue = structValue[PREFIX_LOWPOINTS + "_Transparency"];
            if (propertyValue != null)
                {
                propertyValue.DoubleValue = 0.0;
                properties.Add(propertyValue);
                }

            valueStruct = structValue[PREFIX_LOWPOINTS + "_PointCellName"] as BECO.Instance.IECStructValue;
            Debug.Assert (valueStruct != null);
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

            propertyValue = structValue[PREFIX_LOWPOINTS + "_CellSize"];
            if (propertyValue != null)
                {
                propertyValue.DoubleValue = 1.0;
                properties.Add (propertyValue);
                }

            propertyValue = structValue[PREFIX_LOWPOINTS + "_MinimumDepth"];
            if (propertyValue != null)
                {
                propertyValue.DoubleValue = 0.0;
                properties.Add (propertyValue);
                }

            propertyValue = structValue[PREFIX_LOWPOINTS + "_WantText"];
            if (propertyValue != null)
                {
                propertyValue.IntValue = System.Convert.ToInt32 (true);
                properties.Add (propertyValue);
                }

            propertyValue = structValue[PREFIX_LOWPOINTS + "_TextStyle"];
            if (propertyValue != null)
                {
                propertyValue.StringValue = "";
                properties.Add (propertyValue);
                }

            propertyValue = structValue[PREFIX_LOWPOINTS + "_PrefixText"];
            if (propertyValue != null)
                {
                propertyValue.StringValue = "";
                properties.Add (propertyValue);
                }

            propertyValue = structValue[PREFIX_LOWPOINTS + "_SuffixText"];
            if (propertyValue != null)
                {
                propertyValue.StringValue = "";
                properties.Add (propertyValue);
                }  
            }
        public void ApplyProperties (Bentley.TerrainModelNET.Element.DTMElement elem, Bentley.ECObjects.Instance.IECInstance templateInstance)
            {
            Bentley.TerrainModelNET.Element.DTMLowPointElement lowPointElement = elem.LowPointElement;

            if (lowPointElement == null) return;

            BECO.Instance.IECStructValue structValue = templateInstance[PREFIX_LOWPOINTS] as BECO.Instance.IECStructValue;
            ApplyProperties (lowPointElement, structValue);
            ApplyCellProperties(lowPointElement, structValue);

            Debug.Assert (structValue != null);
            BECO.Instance.IECPropertyValue property = structValue.FindPropertyValue (PREFIX_LOWPOINTS + "_MinimumDepth", false, false, false);

            if (property != null && !property.IsNull)
                {
                double uorsPerMeter = elem.DgnModel.GetModelInfo ().UorPerMeter;
                lowPointElement.MinimumDepth = property.DoubleValue * uorsPerMeter;
                }

            lowPointElement.Commit (elem);
            }

        }
    }
