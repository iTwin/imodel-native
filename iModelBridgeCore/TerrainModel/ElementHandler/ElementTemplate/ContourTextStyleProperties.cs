using System;
using System.Collections.Generic;
using System.Text;
using BECO = Bentley.ECObjects;

namespace Bentley.TerrainModel.ElementTemplate
    {
    public class ContourTextStyleProperties : ElementTemplateExtenderPropertyProvider
        {

        private string m_prefix = string.Empty;                

        public ContourTextStyleProperties(string prefix)
            {
            // Set prefix
            m_prefix = prefix;
            }

        public override void IsActivated(Bentley.ECObjects.Instance.IECInstance instance, List<Bentley.ECObjects.Instance.IECPropertyValue> properties)
            {
            // Set default contour text style properties
            BECO.Instance.IECPropertyValue property = instance[m_prefix + "_TextInterval"];
            if (property != null)
                {
                property.DoubleValue = 10;
                properties.Add(property);
                }

            property = instance[m_prefix + "_TextDontShow"];
            if (property != null)
                {
                property.IntValue = System.Convert.ToInt32(true);
                properties.Add(property);
                }

            property = instance[m_prefix + "_TextSmallContourLength"];
            if (property != null)
                {
                property.DoubleValue = 5;
                properties.Add(property);
                }

            }

//        public void ApplyProperties (Bentley.TerrainModelNET.Element.DTMElement elem, Bentley.TerrainModelNET.Element.DTMContourElement textStyleElement, Bentley.ECObjects.Instance.IECInstance templateInstance)
//            {
        //    BECO.Instance.IECPropertyValue property;
        //    // Apply text style properties

        //    property = templateInstance[m_prefix + "_TextDontShow"];
        //    if (property != null && !property.IsNull)
        //        contourElement.NoTextForSmallContours = System.Convert.ToBoolean(property.IntValue);

        //    property = templateInstance[m_prefix + "_TextSmallContourLength"];
        //    if (property != null && !property.IsNull)
        //        contourElement.SmallContourLength = property.DoubleValue * elem.Element.GetModelReference ().UORsPerMeter;

        //    property = templateInstance[m_prefix + "_TextInterval"];
        //    if (property != null && !property.IsNull)
        //        contourElement.TextInterval = property.DoubleValue * elem.Element.GetModelReference().UORsPerMeter;
//            }

        public void ApplyProperties (Bentley.TerrainModelNET.Element.DTMElement elem, string prefix, Bentley.TerrainModelNET.Element.DTMContourElement contourElement, Bentley.ECObjects.Instance.IECStructValue strucValue)
            {
            BECO.Instance.IECPropertyValue property;
            // Apply text style properties

            property = strucValue[m_prefix + "_TextInterval"];
            if (property != null && !property.IsNull)
                contourElement.TextInterval = property.IntValue * elem.DgnModel.GetModelInfo().UorPerMeter;
            }
        }
    }
