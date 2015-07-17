//using System;
//using System.Collections.Generic;
//using System.Text;
//using BECO = Bentley.ECObjects;

//namespace Bentley.TerrainModel.ElementTemplate
//    {
//    public class BoundaryProperties : ElementTemplateExtenderPropertyProvider, ICanApplyProperties
//        {

//        public const string PREFIX_BOUNDARIES = "Boundary";
//        public const string CATEGORY_BOUNDARIES = "DTMElementExtender_BoundarySettings";

//        private GeneralProperties m_generalProperties = null;

//        public BoundaryProperties()
//            {
//            // create instance of general properties
//            m_generalProperties = new GeneralProperties(PREFIX_BOUNDARIES);
//            }

//        public GeneralProperties GetGeneralProperties()
//            {
//            return m_generalProperties;
//            }

//        public override void IsActivated(BECO.Instance.IECInstance instance, List<BECO.Instance.IECPropertyValue> properties)
//            {
//            // set up default properties            
//            m_generalProperties.IsActivated(instance, properties);
//            }

//        public void ApplyProperties(Bentley.TerrainModelNET.Element.DTMElement elem, BECO.Instance.IECInstance templateInstance)
//            {
//            Bentley.TerrainModelNET.Element.DTMSubElement featureBoundaryElement = elem.FeatureBoundaryElement;

//            if (featureBoundaryElement == null)
//                return;
//            // Apply general properties            
//            m_generalProperties.ApplyProperties (featureBoundaryElement, templateInstance);
//            featureBoundaryElement.Commit (elem);
//            }
//        }
//    }
