using System;
using System.Collections.Generic;
using System.Text;
using BECO = Bentley.ECObjects;
using Bentley.DgnPlatformNET;

namespace Bentley.TerrainModel.ElementTemplate
    {
    public class TriangleProperties : GeneralProperties, ICanApplyProperties
        {
        public const string PREFIX_TRIANGLES = "Triangle";
        public const string CATEGORY_TRIANGLES = "DTMElementExtender_TriangleSettings";        

        public TriangleProperties() : base (PREFIX_TRIANGLES)
            {
            }

        public void ApplyProperties (Bentley.TerrainModelNET.Element.DTMElement elem, BECO.Instance.IECInstance templateInstance)
            {
            Bentley.TerrainModelNET.Element.DTMTrianglesElement trianglesElement = elem.TrianglesElement;

            if (trianglesElement == null) return;

            BECO.Instance.IECStructValue structValue = templateInstance[PREFIX_TRIANGLES] as BECO.Instance.IECStructValue;

            ApplyProperties (trianglesElement, structValue);

            // Apply material name
            BECO.Instance.IECStructValue structVal;
            BECO.Instance.IECPropertyValue property;
            string paletteName = null;
            string materialName = null;

            structVal = (structValue.ContainedValues[PREFIX_TRIANGLES + "_Materials"]) as BECO.Instance.IECStructValue;
            property = structVal.ContainedValues["Value"];
            if (property != null && !property.IsNull)
                materialName = property.StringValue;

            property = structVal.ContainedValues["Type"];
            if (property != null && !property.IsNull)
                paletteName = property.StringValue;
            trianglesElement.SetMaterialInfo (paletteName, materialName);         

            property = structValue.ContainedValues["Value"];

            if (property != null)
                elem.ThematicDisplayStyle = property.StringValue;
            trianglesElement.Commit (elem);

            }
      
        public override void IsActivated(BECO.Instance.IECInstance instance, List<BECO.Instance.IECPropertyValue> properties)
            {
            BECO.Instance.IECStructValue baseStructValue;
            BECO.Instance.IECStructValue valueStruct;
            BECO.Instance.IECPropertyValue propertyValue;

            baseStructValue = instance[PREFIX_TRIANGLES] as BECO.Instance.IECStructValue;
          
            // Set Thematic settings.
            propertyValue = baseStructValue.ContainedValues["Value"]; 
                if (propertyValue != null)
                {
                    propertyValue.StringValue = "";
                    properties.Add(propertyValue);
                }

                // Set material settings
                valueStruct = baseStructValue.ContainedValues[PREFIX_TRIANGLES + "_Materials"] as BECO.Instance.IECStructValue;

                propertyValue = valueStruct["Value"];
                if (propertyValue != null)
                {
                    propertyValue.StringValue = "";
                    properties.Add(propertyValue);
                }

                propertyValue = valueStruct["Type"];
                if (propertyValue != null)
                {
                    propertyValue.StringValue = "";
                    properties.Add(propertyValue);
                }

                propertyValue = baseStructValue[PREFIX_TRIANGLES + "_Level"];
                if (propertyValue != null)
                {
                    propertyValue.StringValue = System.String.Empty;
                    properties.Add(propertyValue);
                }

                valueStruct = (baseStructValue.ContainedValues[PREFIX_TRIANGLES + "_LineStyle"]) as BECO.Instance.IECStructValue;
                propertyValue = valueStruct.ContainedValues["Value"];
                if (propertyValue != null)
                {
                    propertyValue.IntValue = System.Int32.MaxValue - 1;
                    properties.Add(propertyValue);
                }

                valueStruct = (baseStructValue.ContainedValues[PREFIX_TRIANGLES + "_LineWeight"]) as BECO.Instance.IECStructValue;
                propertyValue = valueStruct.ContainedValues["Value"];
                if (propertyValue != null)
                {
                    propertyValue.IntValue = -2;
                    properties.Add(propertyValue);
                }

                // If Transparency required.
                propertyValue = baseStructValue[PREFIX_TRIANGLES + "_Transparency"];
                if (propertyValue != null)
                {
                    propertyValue.DoubleValue = 0.0;
                    properties.Add(propertyValue);
                }

                valueStruct = (baseStructValue.ContainedValues[PREFIX_TRIANGLES + "_Color"]) as BECO.Instance.IECStructValue;
                propertyValue = valueStruct.ContainedValues["Value"];
                if (propertyValue != null)
                {
                    ElementColor color = new ElementColor((ColorSupports)(ColorSupports.Indexed | ColorSupports.Rgb | ColorSupports.ColorBook));
                    color.Source = ColorSource.ByCell;
                    valueStruct = (propertyValue as BECO.Instance.IECStructValue); 
                    color.PopulateStruct (ref valueStruct);
                    properties.Add(propertyValue);
                }
            }

        }
    }
