using System;
using System.Collections.Generic;
using System.Text;
using Bentley.DTM.Element;
using BECO = Bentley.ECObjects;
using BIM = Bentley.Internal.MicroStation;

namespace Bentley.DTM.ElementTemplate
    {
    public class LinearFeatureProperties : ElementTemplateExtenderPropertyProvider
        {
        public const string CATEGORY_LINEARFEATURES = "DTMElementExtender_LinearFeatureSettings";

        public const string PREFIX_LINEARFEATURESBREAKLINE = "LinearFeatureBreakline";
        public const string PREFIX_LINEARFEATURESHOLE = "LinearFeatureHole";
        public const string PREFIX_LINEARFEATURESISLAND = "LinearFeatureIsland";
        public const string PREFIX_LINEARFEATURESVOID = "LinearFeatureVoid";
        public const string PREFIX_LINEARFEATURESBOUNDARY = "LinearFeatureBoundary";
        public const string PREFIX_LINEARFEATURESCONTOUR = "LinearFeatureContour";
        public const string PREFIX_LINEARFEATURESSPOT = "LinearFeatureSpot";

        BECO.Instance.IECInstance m_categoryFeatures = null;

        public LinearFeatureProperties()
            {
            }

        public override void  AddProperties(Bentley.ECObjects.Schema.IECClass ecClass, Bentley.ECObjects.Instance.IECInstance category)
            {
            CreateLinearFeaturesGeneralProperties (PREFIX_LINEARFEATURESBREAKLINE, StringLocalizer.Instance.GetLocalizedString ("Breakline"), ecClass, category, 1000);
            CreateLinearFeaturesGeneralProperties (PREFIX_LINEARFEATURESBOUNDARY, StringLocalizer.Instance.GetLocalizedString ("Boundary"), ecClass, category, 900);
            CreateLinearFeaturesGeneralProperties (PREFIX_LINEARFEATURESCONTOUR, StringLocalizer.Instance.GetLocalizedString ("ImportedContours"), ecClass, category, 800);            
            CreateLinearFeaturesGeneralProperties (PREFIX_LINEARFEATURESISLAND, StringLocalizer.Instance.GetLocalizedString ("Island"), ecClass, category, 700);
            CreateLinearFeaturesGeneralProperties (PREFIX_LINEARFEATURESHOLE, StringLocalizer.Instance.GetLocalizedString ("Hole"), ecClass, category, 600);
            CreateLinearFeaturesGeneralProperties (PREFIX_LINEARFEATURESVOID, StringLocalizer.Instance.GetLocalizedString ("Void"), ecClass, category, 500);
            CreateLinearFeatureSpotProperties (PREFIX_LINEARFEATURESSPOT, StringLocalizer.Instance.GetLocalizedString ("Spot"), ecClass, category, 400);
            }

        public override void CreateCategories(System.Collections.Hashtable categories)
            {
            m_categoryFeatures = Bentley.ECObjects.UI.ECPropertyPane.CreateCategory (
               CATEGORY_LINEARFEATURES,
               StringLocalizer.Instance.GetLocalizedString ("TerrainModelFeatures"),
               StringLocalizer.Instance.GetLocalizedString ("TerrainModelFeatures"),
               BECO.UI.ECPropertyPane.CategorySortPriorityHigh - 1200);
            categories.Add (CATEGORY_LINEARFEATURES, m_categoryFeatures);
            }

        public override void IsActivated (BECO.Instance.IECInstance instance, List<BECO.Instance.IECPropertyValue> properties)
            {
            Activate (PREFIX_LINEARFEATURESBREAKLINE, instance, properties);
            Activate (PREFIX_LINEARFEATURESBOUNDARY, instance, properties);
            Activate (PREFIX_LINEARFEATURESCONTOUR, instance, properties);
            Activate (PREFIX_LINEARFEATURESISLAND, instance, properties);
            Activate (PREFIX_LINEARFEATURESHOLE, instance, properties);
            Activate (PREFIX_LINEARFEATURESVOID, instance, properties);
            Activate (PREFIX_LINEARFEATURESSPOT, instance, properties);
            }


        public void Activate (string prefix, BECO.Instance.IECInstance instance, List<BECO.Instance.IECPropertyValue> properties)
            {
            BECO.Instance.IECPropertyValue propertyValue;
            BECO.Instance.IECStructValue baseStructValue;
            BECO.Instance.IECStructValue structValue;

            baseStructValue = instance[prefix] as BECO.Instance.ECStructValue;

            if (prefix == PREFIX_LINEARFEATURESBREAKLINE)
                instance[DisplayTerrainModelFeatureProperties.PREFIX_LINEARFEATURESBREAKLINEDISPLAYED].IntValue = 0;
            if (prefix == PREFIX_LINEARFEATURESBOUNDARY)
                instance[DisplayTerrainModelFeatureProperties.PREFIX_LINEARFEATURESBOUNDARYDISPLAYED].IntValue = 0;
            if (prefix == PREFIX_LINEARFEATURESCONTOUR)
                instance[DisplayTerrainModelFeatureProperties.PREFIX_LINEARFEATURESCONTOURDISPLAYED].IntValue = 0;
            if (prefix == PREFIX_LINEARFEATURESISLAND)
                instance[DisplayTerrainModelFeatureProperties.PREFIX_LINEARFEATURESISLANDDISPLAYED].IntValue = 0;
            if (prefix == PREFIX_LINEARFEATURESHOLE)
                instance[DisplayTerrainModelFeatureProperties.PREFIX_LINEARFEATURESHOLEDISPLAYED].IntValue = 0;
            if (prefix == PREFIX_LINEARFEATURESVOID)
                instance[DisplayTerrainModelFeatureProperties.PREFIX_LINEARFEATURESVOIDDISPLAYED].IntValue = 0;
            if (prefix == PREFIX_LINEARFEATURESSPOT)
                instance[DisplayTerrainModelFeatureProperties.PREFIX_LINEARFEATURESSPOTDISPLAYED].IntValue = 0;

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
                propertyValue.IntValue = System.Int32.MaxValue;
                properties.Add (propertyValue);
                }

            structValue = (baseStructValue.ContainedValues[prefix + "_LineWeight"]) as BECO.Instance.IECStructValue;
            propertyValue = structValue.ContainedValues["Value"];
            if (propertyValue != null)
                {
                propertyValue.IntValue = -1;
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
                BIM.Elements.ElementColor color = new BIM.Elements.ElementColor ((BIM.Elements.ColorSupport)(BIM.Elements.ColorSupport.ColorSupportIndexed |
                                                                                                                BIM.Elements.ColorSupport.ColorSupportRGB |
                                                                                                                BIM.Elements.ColorSupport.ColorSupportColorBook));
                color.MatchActiveSettings ();
                propertyValue.NativeValue = color;
                properties.Add (propertyValue);
                }
 
            if (prefix == PREFIX_LINEARFEATURESSPOT)
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

        public void ApplyProperties(Bentley.DTM.Element.DTMElement elem, BECO.Instance.IECInstance templateInstance)
        {
            BECO.Instance.IECPropertyValue propertyValue;
            BECO.Instance.IECStructValue baseStructValue;
            BECO.Instance.IECStructValue structValue;

            if (elem.FeatureBreaklineElement != null)
            {
                baseStructValue = templateInstance[PREFIX_LINEARFEATURESBREAKLINE] as BECO.Instance.ECStructValue;
                propertyValue = baseStructValue[PREFIX_LINEARFEATURESBREAKLINE + "_Level"];
                if (propertyValue != null)
                    elem.FeatureBreaklineElement.LevelID = (Bentley.Internal.MicroStation.LevelID)Bentley.Internal.MicroStation.Settings.GetLevelIdFromName(propertyValue.StringValue); ;

                structValue = (baseStructValue.ContainedValues[PREFIX_LINEARFEATURESBREAKLINE + "_LineStyle"]) as BECO.Instance.IECStructValue;
                propertyValue = structValue.ContainedValues["Value"];
                if (propertyValue != null)
                    elem.FeatureBreaklineElement.Style = DTMElementTemplateExtender.GetLineStyle(propertyValue.StringValue);

                structValue = (baseStructValue.ContainedValues[PREFIX_LINEARFEATURESBREAKLINE + "_LineWeight"]) as BECO.Instance.IECStructValue;
                propertyValue = structValue.ContainedValues["Value"];
                if (propertyValue != null)
                {
                    if (propertyValue.IntValue == -1)
                        elem.FeatureBreaklineElement.Weight = 0xffffffff;
                    else
                        elem.FeatureBreaklineElement.Weight = System.Convert.ToUInt32(propertyValue.IntValue);
                }

                structValue = (baseStructValue.ContainedValues[PREFIX_LINEARFEATURESBREAKLINE + "_Color"]) as BECO.Instance.IECStructValue;
                propertyValue = structValue.ContainedValues["Value"];
                if (propertyValue != null)
                    elem.FeatureBreaklineElement.ElementColor = (BIM.Elements.ElementColor)propertyValue.NativeValue;

                propertyValue = baseStructValue.ContainedValues[PREFIX_LINEARFEATURESBREAKLINE + "_Transparency"];
                if (propertyValue != null)
                    elem.FeatureBreaklineElement.Transparency = propertyValue.DoubleValue;
            }

            if (elem.FeatureHoleElement != null)
            {
                baseStructValue = templateInstance[PREFIX_LINEARFEATURESHOLE] as BECO.Instance.ECStructValue;
                propertyValue = baseStructValue[PREFIX_LINEARFEATURESHOLE + "_Level"];
                if (propertyValue != null)
                    elem.FeatureHoleElement.LevelID = (Bentley.Internal.MicroStation.LevelID)Bentley.Internal.MicroStation.Settings.GetLevelIdFromName(propertyValue.StringValue);

                structValue = (baseStructValue.ContainedValues[PREFIX_LINEARFEATURESHOLE + "_LineStyle"]) as BECO.Instance.IECStructValue;
                propertyValue = structValue.ContainedValues["Value"];
                if (propertyValue != null)
                    elem.FeatureHoleElement.Style = DTMElementTemplateExtender.GetLineStyle(propertyValue.StringValue);

                structValue = (baseStructValue.ContainedValues[PREFIX_LINEARFEATURESHOLE + "_LineWeight"]) as BECO.Instance.IECStructValue;
                propertyValue = structValue.ContainedValues["Value"];
                if (propertyValue != null)
                {
                    if (propertyValue.IntValue == -1)
                        elem.FeatureBreaklineElement.Weight = 0xffffffff;
                    else
                        elem.FeatureHoleElement.Weight = System.Convert.ToUInt32(propertyValue.IntValue);
                }

                structValue = (baseStructValue.ContainedValues[PREFIX_LINEARFEATURESHOLE + "_Color"]) as BECO.Instance.IECStructValue;
                propertyValue = structValue.ContainedValues["Value"];
                if (propertyValue != null)
                    elem.FeatureHoleElement.ElementColor = (BIM.Elements.ElementColor)propertyValue.NativeValue;

                propertyValue = baseStructValue.ContainedValues[PREFIX_LINEARFEATURESHOLE + "_Transparency"];
                if (propertyValue != null)
                    elem.FeatureHoleElement.Transparency = propertyValue.DoubleValue;
            }
            if (elem.FeatureIslandElement != null)
            {
                baseStructValue = templateInstance[PREFIX_LINEARFEATURESISLAND] as BECO.Instance.ECStructValue;
                propertyValue = baseStructValue[PREFIX_LINEARFEATURESISLAND + "_Level"];
                if (propertyValue != null)
                    elem.FeatureIslandElement.LevelID = (Bentley.Internal.MicroStation.LevelID)Bentley.Internal.MicroStation.Settings.GetLevelIdFromName(propertyValue.StringValue);

                structValue = (baseStructValue.ContainedValues[PREFIX_LINEARFEATURESISLAND + "_LineStyle"]) as BECO.Instance.IECStructValue;
                propertyValue = structValue.ContainedValues["Value"];
                if (propertyValue != null)
                    elem.FeatureIslandElement.Style = DTMElementTemplateExtender.GetLineStyle(propertyValue.StringValue);

                structValue = (baseStructValue.ContainedValues[PREFIX_LINEARFEATURESISLAND + "_LineWeight"]) as BECO.Instance.IECStructValue;
                propertyValue = structValue.ContainedValues["Value"];
                if (propertyValue != null)
                {
                    if (propertyValue.IntValue == -1)
                        elem.FeatureBreaklineElement.Weight = 0xffffffff;
                    else
                        elem.FeatureIslandElement.Weight = System.Convert.ToUInt32(propertyValue.IntValue);
                }

                structValue = (baseStructValue.ContainedValues[PREFIX_LINEARFEATURESISLAND + "_Color"]) as BECO.Instance.IECStructValue;
                propertyValue = structValue.ContainedValues["Value"];
                if (propertyValue != null)
                    elem.FeatureIslandElement.ElementColor = (BIM.Elements.ElementColor)propertyValue.NativeValue;

                propertyValue = baseStructValue.ContainedValues[PREFIX_LINEARFEATURESISLAND + "_Transparency"];
                if (propertyValue != null)
                    elem.FeatureIslandElement.Transparency = propertyValue.DoubleValue;
            }
            if (elem.FeatureVoidElement != null)
            {
                baseStructValue = templateInstance[PREFIX_LINEARFEATURESVOID] as BECO.Instance.ECStructValue;
                propertyValue = baseStructValue[PREFIX_LINEARFEATURESVOID + "_Level"];
                if (propertyValue != null)
                    elem.FeatureVoidElement.LevelID = (Bentley.Internal.MicroStation.LevelID)Bentley.Internal.MicroStation.Settings.GetLevelIdFromName(propertyValue.StringValue);

                structValue = (baseStructValue.ContainedValues[PREFIX_LINEARFEATURESVOID + "_LineStyle"]) as BECO.Instance.IECStructValue;
                propertyValue = structValue.ContainedValues["Value"];
                if (propertyValue != null)
                    elem.FeatureVoidElement.Style = DTMElementTemplateExtender.GetLineStyle(propertyValue.StringValue);

                structValue = (baseStructValue.ContainedValues[PREFIX_LINEARFEATURESVOID + "_LineWeight"]) as BECO.Instance.IECStructValue;
                propertyValue = structValue.ContainedValues["Value"];
                if (propertyValue != null)
                {
                    if (propertyValue.IntValue == -1)
                        elem.FeatureBreaklineElement.Weight = 0xffffffff;
                    else
                        elem.FeatureVoidElement.Weight = System.Convert.ToUInt32(propertyValue.IntValue);
                }

                structValue = (baseStructValue.ContainedValues[PREFIX_LINEARFEATURESVOID + "_Color"]) as BECO.Instance.IECStructValue;
                propertyValue = structValue.ContainedValues["Value"];
                if (propertyValue != null)
                    elem.FeatureVoidElement.ElementColor = (BIM.Elements.ElementColor)propertyValue.NativeValue;

                propertyValue = baseStructValue.ContainedValues[PREFIX_LINEARFEATURESVOID + "_Transparency"];
                if (propertyValue != null)
                    elem.FeatureVoidElement.Transparency = propertyValue.DoubleValue;
            }
            if (elem.FeatureBoundaryElement != null)
            {
                baseStructValue = templateInstance[PREFIX_LINEARFEATURESBOUNDARY] as BECO.Instance.ECStructValue;
                propertyValue = baseStructValue[PREFIX_LINEARFEATURESBOUNDARY + "_Level"];
                if (propertyValue != null)
                    elem.FeatureBoundaryElement.LevelID = (Bentley.Internal.MicroStation.LevelID)Bentley.Internal.MicroStation.Settings.GetLevelIdFromName(propertyValue.StringValue);

                structValue = (baseStructValue.ContainedValues[PREFIX_LINEARFEATURESBOUNDARY + "_LineStyle"]) as BECO.Instance.IECStructValue;
                propertyValue = structValue.ContainedValues["Value"];
                if (propertyValue != null)
                    elem.FeatureBoundaryElement.Style = DTMElementTemplateExtender.GetLineStyle(propertyValue.StringValue);

                structValue = (baseStructValue.ContainedValues[PREFIX_LINEARFEATURESBOUNDARY + "_LineWeight"]) as BECO.Instance.IECStructValue;
                propertyValue = structValue.ContainedValues["Value"];
                if (propertyValue != null)
                {
                    if (propertyValue.IntValue == -1)
                        elem.FeatureBreaklineElement.Weight = 0xffffffff;
                    else
                        elem.FeatureBoundaryElement.Weight = System.Convert.ToUInt32(propertyValue.IntValue);
                }

                structValue = (baseStructValue.ContainedValues[PREFIX_LINEARFEATURESBOUNDARY + "_Color"]) as BECO.Instance.IECStructValue;
                propertyValue = structValue.ContainedValues["Value"];
                if (propertyValue != null)
                    elem.FeatureBoundaryElement.ElementColor = (BIM.Elements.ElementColor)propertyValue.NativeValue;

                propertyValue = baseStructValue.ContainedValues[PREFIX_LINEARFEATURESBOUNDARY + "_Transparency"];
                if (propertyValue != null)
                    elem.FeatureBoundaryElement.Transparency = propertyValue.DoubleValue;
            }
            if (elem.FeatureContourElement != null)
            {
                baseStructValue = templateInstance[PREFIX_LINEARFEATURESCONTOUR] as BECO.Instance.ECStructValue;
                propertyValue = baseStructValue[PREFIX_LINEARFEATURESCONTOUR + "_Level"];
                if (propertyValue != null)
                    elem.FeatureContourElement.LevelID = (Bentley.Internal.MicroStation.LevelID)Bentley.Internal.MicroStation.Settings.GetLevelIdFromName(propertyValue.StringValue);

                structValue = (baseStructValue.ContainedValues[PREFIX_LINEARFEATURESCONTOUR + "_LineStyle"]) as BECO.Instance.IECStructValue;
                propertyValue = structValue.ContainedValues["Value"];
                if (propertyValue != null)
                    elem.FeatureContourElement.Style = DTMElementTemplateExtender.GetLineStyle(propertyValue.StringValue);

                structValue = (baseStructValue.ContainedValues[PREFIX_LINEARFEATURESCONTOUR + "_LineWeight"]) as BECO.Instance.IECStructValue;
                propertyValue = structValue.ContainedValues["Value"];
                if (propertyValue != null)
                {
                    if (propertyValue.IntValue == -1)
                        elem.FeatureBreaklineElement.Weight = 0xffffffff;
                    else
                        elem.FeatureContourElement.Weight = System.Convert.ToUInt32(propertyValue.IntValue);
                }

                structValue = (baseStructValue.ContainedValues[PREFIX_LINEARFEATURESCONTOUR + "_Color"]) as BECO.Instance.IECStructValue;
                propertyValue = structValue.ContainedValues["Value"];
                if (propertyValue != null)
                    elem.FeatureContourElement.ElementColor = (BIM.Elements.ElementColor)propertyValue.NativeValue;

                propertyValue = baseStructValue.ContainedValues[PREFIX_LINEARFEATURESCONTOUR + "_Transparency"];
                if (propertyValue != null)
                    elem.FeatureContourElement.Transparency = propertyValue.DoubleValue;
            }
            if (elem.FeatureSpotElement != null)
            {
                baseStructValue = templateInstance[PREFIX_LINEARFEATURESSPOT] as BECO.Instance.ECStructValue;
                propertyValue = baseStructValue[PREFIX_LINEARFEATURESSPOT + "_Level"];
                if (propertyValue != null)
                    elem.FeatureSpotElement.LevelID = (Bentley.Internal.MicroStation.LevelID)Bentley.Internal.MicroStation.Settings.GetLevelIdFromName(propertyValue.StringValue);

                structValue = (baseStructValue.ContainedValues[PREFIX_LINEARFEATURESSPOT + "_LineStyle"]) as BECO.Instance.IECStructValue;
                propertyValue = structValue.ContainedValues["Value"];
                if (propertyValue != null)
                    elem.FeatureSpotElement.Style = DTMElementTemplateExtender.GetLineStyle(propertyValue.StringValue);

                structValue = (baseStructValue.ContainedValues[PREFIX_LINEARFEATURESSPOT + "_LineWeight"]) as BECO.Instance.IECStructValue;
                propertyValue = structValue.ContainedValues["Value"];
                if (propertyValue != null)
                {
                    if (propertyValue.IntValue == -1)
                        elem.FeatureSpotElement.Weight = 0xffffffff;
                    else
                        elem.FeatureSpotElement.Weight = System.Convert.ToUInt32(propertyValue.IntValue);
                }

                structValue = (baseStructValue.ContainedValues[PREFIX_LINEARFEATURESSPOT + "_Color"]) as BECO.Instance.IECStructValue;
                propertyValue = structValue.ContainedValues["Value"];
                if (propertyValue != null)
                    elem.FeatureSpotElement.ElementColor = (BIM.Elements.ElementColor)propertyValue.NativeValue;

                propertyValue = baseStructValue.ContainedValues[PREFIX_LINEARFEATURESSPOT + "_Transparency"];
                if (propertyValue != null)
                    elem.FeatureSpotElement.Transparency = propertyValue.DoubleValue;

                propertyValue = baseStructValue.ContainedValues[PREFIX_LINEARFEATURESSPOT + "_WantText"];
                if (!propertyValue.IsNull)
                    elem.FeatureSpotElement.WantFeatureSpotText = System.Convert.ToBoolean(propertyValue.IntValue);

                structValue = (baseStructValue.ContainedValues[PREFIX_LINEARFEATURESSPOT + "_SpotSymbol"]) as BECO.Instance.IECStructValue;
                propertyValue = structValue.ContainedValues["Value"];
                if (!propertyValue.IsNull)
                    elem.FeatureSpotElement.FeatureSpotCellName = propertyValue.StringValue;

                propertyValue = structValue.ContainedValues["Type"];
                if (!propertyValue.IsNull)
                    elem.FeatureSpotElement.FeatureSpotPointCellType = propertyValue.IntValue;

                List<Bentley.Internal.MicroStation.TextStyle> textStyles = Bentley.Internal.MicroStation.TextStyle.GetAll(true);
                foreach (Bentley.Internal.MicroStation.TextStyle textStyle in textStyles)
                {
                    if (textStyle.Name == baseStructValue.ContainedValues[PREFIX_LINEARFEATURESSPOT + "_SpotTextStyle"].StringValue)
                    {
                        elem.FeatureSpotElement.FeatureSpotTextStyleID = (int)textStyle.ID;
                        break;
                    }
                    else
                        elem.FeatureSpotElement.FeatureSpotTextStyleID = 0;
                }

                elem.FeatureSpotElement.FeatureSpotTextPrefix = baseStructValue.ContainedValues[PREFIX_LINEARFEATURESSPOT + "_SpotTextPrefix"].StringValue;
                elem.FeatureSpotElement.FeatureSpotTextSuffix = baseStructValue.ContainedValues[PREFIX_LINEARFEATURESSPOT + "_SpotTextSuffix"].StringValue;
            }
        }

        private void CreateLinearFeaturesGeneralProperties (string name, string displayLabel, BECO.Schema.IECClass ecClass, BECO.Instance.IECInstance category, int priority)
            {
            BECO.Schema.IECProperty generalGroupProperty;
            BECO.Schema.ECClass generalGroupStruct = new Bentley.ECObjects.Schema.ECClass (name + "_Class", null, true);

            generalGroupStruct.Add (CreateLevelProperty (name + "_Level", StringLocalizer.Instance.GetLocalizedString ("Level"), category, 200));
            generalGroupStruct.Add(CreateColorProperty(name + "_Color", StringLocalizer.Instance.GetLocalizedString("Color"), category, 300));
            generalGroupStruct.Add (CreateLineStyleProperty (name + "_LineStyle", StringLocalizer.Instance.GetLocalizedString ("LineStyle"), category, 400));
            generalGroupStruct.Add (CreateLineWeightProperty (name + "_LineWeight", StringLocalizer.Instance.GetLocalizedString ("Weight"), category, 500));
            generalGroupStruct.Add (CreateTransparencyProperty (name + "_Transparency", StringLocalizer.Instance.GetLocalizedString ("Transparency"), category, 600));

            generalGroupProperty = new Bentley.ECObjects.Schema.ECProperty (name, generalGroupStruct);
            generalGroupProperty.DisplayLabel = displayLabel;
            BECO.UI.ECPropertyPane.SetCategory (generalGroupProperty, category);
            BECO.UI.ECPropertyPane.SetPriority (generalGroupProperty, priority);
            ecClass.Add (generalGroupProperty);
            }

        private void CreateLinearFeatureSpotProperties (string name, string displayLabel, BECO.Schema.IECClass ecClass, BECO.Instance.IECInstance category, int priority)
            {
            BECO.Schema.IECProperty spotProperty;
            BECO.Schema.ECClass generalGroupStruct = new Bentley.ECObjects.Schema.ECClass(name + "_Class", null, true);

            generalGroupStruct.Add (CreateLevelProperty (name + "_Level", StringLocalizer.Instance.GetLocalizedString ("Level"), category, priority + 200));
            generalGroupStruct.Add(CreateColorProperty(name + "_Color", StringLocalizer.Instance.GetLocalizedString("Color"), category, priority + 300));
            generalGroupStruct.Add (CreateLineStyleProperty (name + "_LineStyle", StringLocalizer.Instance.GetLocalizedString ("LineStyle"), category, priority + 400));
            generalGroupStruct.Add (CreateLineWeightProperty (name + "_LineWeight", StringLocalizer.Instance.GetLocalizedString ("Weight"), category, priority + 500));
            generalGroupStruct.Add (CreateTransparencyProperty (name + "_Transparency", StringLocalizer.Instance.GetLocalizedString ("Transparency"), category, priority + 600));
            generalGroupStruct.Add (CreatePointCellProperty (name + "_SpotSymbol", StringLocalizer.Instance.GetLocalizedString ("CellName"), category, priority + 700));
            generalGroupStruct.Add (CreateBooleanProperty(name + "_WantText", StringLocalizer.Instance.GetLocalizedString ("DisplayText"), category, priority + 800));
            generalGroupStruct.Add (CreateTextStyleProperty (name + "_SpotTextStyle", StringLocalizer.Instance.GetLocalizedString ("TextStyle"), category, priority + 900));
            generalGroupStruct.Add (CreateStringPropoerty (name + "_SpotTextPrefix", StringLocalizer.Instance.GetLocalizedString ("TextPrefix"), category, priority + 1000));
            generalGroupStruct.Add (CreateStringPropoerty (name + "_SpotTextSuffix", StringLocalizer.Instance.GetLocalizedString ("TextSuffix"), category, priority + 1100));

            spotProperty = new Bentley.ECObjects.Schema.ECProperty (name, generalGroupStruct);
            spotProperty.DisplayLabel = displayLabel;
            BECO.UI.ECPropertyPane.SetCategory (spotProperty, category);
            BECO.UI.ECPropertyPane.SetPriority (spotProperty, priority);
            ecClass.Add (spotProperty);
            }
        }
    }
