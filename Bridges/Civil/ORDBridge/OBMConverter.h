/*--------------------------------------------------------------------------------------+
|
|     $Source: ORDBridge/OBMConverter.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "ORDBridgeInternal.h"
#include "ORDConverter.h"

BEGIN_ORDBRIDGE_NAMESPACE


struct ObmApplicationSchemaCreator
    {
    //This is an internal tool used in the development of the OBM Application schema. It recognizes OBM data
    //that doesn't exist in the schema, and creates an updated version of the schema with the added elements/properties.
    //This tool should only be used while developing the OBM Application schema, it's not a necessary part of 
    //converting OBM data.
    public:
        ObmApplicationSchemaCreator() {};
        ObmApplicationSchemaCreator(BentleyM0200::BeFileName schemaFileName);
        BeXmlDomPtr m_xmlDom;
        BeXmlNodeP m_rootNodeP;
        BeXmlNodeP m_currentNodeP;
        BentleyM0200::BeFileName m_schemaFileName;
        BentleyM0200::BeFileName m_updatedSchemaFileName;
        Bentley::bset<Bentley::Utf8String> m_elementTypeNames;
        BentleyM0200::bmap<BentleyM0200::Utf8String, BeXmlNodeP> m_aspectNameToSchemaNodeMap;
        BentleyM0200::bmap<BentleyM0200::Utf8String, BentleyM0200::Utf8String> m_localizableStringsMap;
        BentleyM0200::bset<BentleyM0200::Utf8String> m_propertiesToIgnore;

        void InitializeSchema();
        void InitializeLocalizableStringsMap();
        BeXmlNodeP AddNewAspect(BeXmlNodeP parentNodeP, Bentley::Utf8String aspectName, Bentley::Utf8String aspectModifier, Bentley::Utf8String aspectDisplayLabel, Bentley::Utf8String aspectDescription);
        void AddNewAspectProperty(BeXmlNodeP parentNodeP, Bentley::Utf8String propertyName, Bentley::Utf8String propertyTypeName, Bentley::Utf8String propertyDisplayLabel, Bentley::Utf8String kindOfQuantity);
        BeXmlNodeP FindAspectByName(BentleyM0200::Utf8String aspectName);
        void ReadSchemaFromFile();
        void WriteXmlDomToFile();
        bool DoesSchemaContainAspect(Bentley::WString aspectName);
        bool DoesAspectContainProperty(BentleyM0200::Utf8String propertyName, BeXmlNodeP aspectNodeP);
        Bentley::Utf8String DetermineKindOfQuantity(BentleyM0200::Utf8String propertyName, BentleyM0200::Utf8String propertyTypeName);
        Bentley::Utf8String DetermineDisplayLabel(BentleyM0200::Utf8String propertyName);
        void ApplySchemaFormatting();
        void ParseSerializedProperties(Bentley::WString serializedProperties);
        void ParseSerializedProperties(Bentley::WString serializedProperties, BentleyM0200::Utf8String parentAspectName, BeXmlNodeP parentAspectNodeP);
        void UpdateOBMApplicationSchema(Bentley::DgnECInstanceCR ecInstanceCR);

        static BentleyM0200::bset<BentleyM0200::Utf8String> s_doublePropertyPhrases;
        static BentleyM0200::bset<BentleyM0200::Utf8String> s_stringPropertyPhrases;
        static void InitializePhrases();
        static Bentley::Utf8String DeterminePropertyType(BentleyM0200::Utf8String propertyName);

    }; //OBMApplicationSchemaCreator

struct OBMConverter : ORDConverterExtension
{
protected:
    virtual Dgn::DgnDbSync::DgnV8::XDomain::Result _PreConvertElement(DgnV8EhCR, ORDConverter&, Dgn::DgnDbSync::DgnV8::ResolvedModelMapping const&) override;
    virtual void _DetermineElementParams(Dgn::DgnClassId&, Dgn::DgnCode&, Dgn::DgnCategoryId&, DgnV8EhCR, ORDConverter&, ECObjectsV8::IECInstance const* primaryV8Instance, Dgn::DgnDbSync::DgnV8::ResolvedModelMapping const&) override;
    virtual void _ProcessResults(Dgn::DgnDbSync::DgnV8::ElementConversionResults&, DgnV8EhCR, Dgn::DgnDbSync::DgnV8::ResolvedModelMapping const&, ORDConverter&) override;
    virtual void _OnConversionComplete(ORDConverter&) override;
    virtual BentleyStatus _MakeSchemaChanges() override;

public:
    OBMConverter(ORDConverter& converter);

private:
    
    enum StructuralElementType
        {
        StructuralElementType_Default,
        StructuralElementType_Beam,
        StructuralElementType_Brace,
        StructuralElementType_Column,
        StructuralElementType_ConcreteMaterialProperties,
        StructuralElementType_FoundationMember,
        StructuralElementType_MaterialProperties,
        StructuralElementType_Pile,
        StructuralElementType_PileCap,
        StructuralElementType_Slab,
        StructuralElementType_SpreadFooting,
        StructuralElementType_SteelMaterialProperties,
        StructuralElementType_StripFooting,
        StructuralElementType_StructuralAddition,
        StructuralElementType_StructuralElement,
        StructuralElementType_StructuralMember,
        StructuralElementType_StructuralPhysicalDomain,
        StructuralElementType_StructuralPhysicalModel,
        StructuralElementType_StructuralSubtraction,
        StructuralElementType_Wall
        };

    ORDConverter& m_converter;
    bmap <Bentley::ObmNET::GeometryModel::SDK::BeamSegmentType, StructuralElementType> m_BeamSegmentMap;
    bmap <Bentley::ObmNET::GeometryModel::SDK::PierCapType, StructuralElementType> m_PierCapMap;
    bmap<Bentley::ObmNET::GeometryModel::SDK::PierFootingType, StructuralElementType> m_PierFootingMap;
    bmap<Bentley::ObmNET::GeometryModel::SDK::CrossFrameComponentType, StructuralElementType> m_CrossFrameMap;
    bmap<Dgn::DgnCategoryId, Dgn::DgnCategoryId> m_obmToBimCategoryMap;
    bmap<Bentley::ElementRefP, BridgeStructuralPhysical::GenericSubstructureElementPtr> m_SupportLineRefToConvertedSubstructureMap;
    bset<Dgn::DgnCategoryId> m_dgnCategoriesSeen;
    bset<Bentley::ElementRefP> m_bridgeV8RefSet;
    bset<Bentley::ElementRefP> m_bmSolidRefSet;
    bset<Bentley::ElementRefP> m_elementsSeen;
    Bentley::Cif::ConsensusConnectionPtr m_consensusConnectionPtr;
    ObmApplicationSchemaCreator m_obmSchemaCreator;
    ObmApplicationSchemaCreator* m_obmSchemaCreatorP;
    
    bool CheckShouldUpdateOBMSchema();
    void InitializeObmElementMappings();

    void CreateBimBridges();
    void CreateBimBridge(Bentley::ObmNET::BridgeR bridgeR, bmap<Bentley::ElementRefP, Dgn::DgnElementId> cifAlignmentToBimID);
    void CreateBimBridgeComponents(Bentley::ObmNET::BridgeR obmBridgeR, BridgeStructuralPhysical::BridgeR bimBridgeR, RoadRailAlignment::AlignmentCR bimAlignmentCR);
    void CreateBimBridgeSubstructures(Bentley::ObmNET::BridgeUnitR bridgeUnitR, BridgeStructuralPhysical::BridgeR bimBridgeR, RoadRailAlignment::AlignmentCR bimAlignmentCR, Dgn::PhysicalModelCP structuralSystemModelCP);
    
    void ProcessObmSupportLines(Bentley::ObmNET::BridgeUnitR bridgeUnitR, BridgeStructuralPhysical::BridgeR bimBridgeR, RoadRailAlignment::AlignmentCR bimAlignmentCR, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ProcessObmPierComponents(Bentley::ObmNET::PierR pierR, BridgeStructuralPhysical::GenericSubstructureElementR bimSubstructureR, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ProcessObmBearingGroups(Bentley::ObmNET::BridgeUnitR bridgeUnitR, BridgeStructuralPhysical::BridgeR bimBridgeR, RoadRailAlignment::AlignmentCR bimAlignmentCR, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ProcessObmBearingSubcomponents(Bentley::ObmNET::BearingComponentCollectionR bearingComponentsR, BridgeStructuralPhysical::GenericSubstructureElementPtr bimSubstructurePtr, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ProcessObmAbutmentWingWalls(Bentley::ObmNET::BridgeUnitR bridgeUnitR, BridgeStructuralPhysical::BridgeR bimBridgeR, RoadRailAlignment::AlignmentCR bimAlignmentCR, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ProcessObmAbutmentWingWallComponents(Bentley::ObmNET::AbutmentWingWallR abutmentWingWallR, BridgeStructuralPhysical::BridgeR bimBridgeR, RoadRailAlignment::AlignmentCR bimAlignmentCR, Dgn::PhysicalModelCP structuralSystemModelCP);
    
    void ConvertObmBearings(Bentley::ObmNET::BearingComponentCollectionR bearingComponentCollectionR, BridgeStructuralPhysical::GenericSubstructureElementPtr bimSubstructurePtr, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ConvertObmBeamSeats(Bentley::ObmNET::BearingR bearingR, BridgeStructuralPhysical::GenericSubstructureElementPtr bimSubstructurePtr, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ConvertObmWingWalls(Bentley::ObmNET::AbutmentWingWallCmpCollectionSubR abutmentWingWallSubcomponentCollectionR, BridgeStructuralPhysical::GenericSubstructureElementPtr bimSubstructurePtr, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ConvertObmWingWallFootings(Bentley::ObmNET::AbutmentWingWallCmpCollectionSubR abutmentWingWallSubcomponentCollectionR, BridgeStructuralPhysical::GenericSubstructureElementPtr bimSubstructurePtr, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ConvertObmWingWallPiles(Bentley::ObmNET::AbutmentWingWallCmpCollectionSubR abutmentWingWallSubcomponentCollectionR, BridgeStructuralPhysical::GenericSubstructureElementPtr bimSubstructurePtr, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ConvertObmPierCaps(Bentley::ObmNET::PierComponentCollectionSubR pierSubcomponentR, BridgeStructuralPhysical::GenericSubstructureElementR bimSubstructureR, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ConvertObmPierCheekWalls(Bentley::ObmNET::PierComponentCollectionSubR pierSubcomponentR, BridgeStructuralPhysical::GenericSubstructureElementR bimSubstructureR, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ConvertObmPierColumns(Bentley::ObmNET::PierComponentCollectionSubR pierSubcomponentR, BridgeStructuralPhysical::GenericSubstructureElementR bimSubstructureR, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ConvertObmPierCrashWalls(Bentley::ObmNET::PierComponentCollectionSubR pierSubcomponentR, BridgeStructuralPhysical::GenericSubstructureElementR bimSubstructureR, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ConvertObmPierFootings(Bentley::ObmNET::PierComponentCollectionSubR pierSubcomponentR, BridgeStructuralPhysical::GenericSubstructureElementR bimSubstructureR, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ConvertObmPierPiles(Bentley::ObmNET::PierComponentCollectionSubR pierSubcomponentR, BridgeStructuralPhysical::GenericSubstructureElementR bimSubstructureR, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ConvertObmPierRockSockets(Bentley::ObmNET::PierComponentCollectionSubR pierSubcomponentR, BridgeStructuralPhysical::GenericSubstructureElementR bimSubstructureR, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ConvertObmPierStruts(Bentley::ObmNET::PierComponentCollectionSubR pierSubcomponentR, BridgeStructuralPhysical::GenericSubstructureElementR bimSubstructureR, Dgn::PhysicalModelCP structuralSystemModelCP);
    
    void CreateBimBridgeSuperstructures(Bentley::ObmNET::BridgeUnitR bridgeUnitR, BridgeStructuralPhysical::BridgeR bimBridgeR, RoadRailAlignment::AlignmentCR bimAlignmentCR, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ProcessObmCrossFrameGroups(Bentley::ObmNET::BridgeUnitR bridgeUnitR, BridgeStructuralPhysical::GenericSuperstructureElementR bimSuperstructureR, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ProcessObmCrossFrames(Bentley::ObmNET::CrossFrameGroupR crossFrameGroupR, BridgeStructuralPhysical::GenericSuperstructureElementR bimSuperstructureR, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ProcessObmSegmentalBalances(Bentley::ObmNET::BridgeUnitR bridgeUnitR, BridgeStructuralPhysical::GenericSuperstructureElementR bimSuperstructureR, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ProcessObmSegmentalSpans(Bentley::ObmNET::BridgeUnitR bridgeUnitR, BridgeStructuralPhysical::GenericSuperstructureElementR bimSuperstructureR, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ProcessObmSegmentCollectionSubcomponents(Bentley::ObmNET::SegmentalBalancePtr segmentalBalancePtr, Bentley::ObmNET::SegmentalSpanPtr segmentalSpanPtr, BridgeStructuralPhysical::GenericSuperstructureElementR bimSuperstructureR, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ProcessObmBeamGroups(Bentley::ObmNET::BridgeUnitR bridgeUnitR, BridgeStructuralPhysical::GenericSuperstructureElementR bimSuperstructureR, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ConvertObmCrossFrameComponents(Bentley::ObmNET::CrossFrameR, BridgeStructuralPhysical::GenericSuperstructureElementR bimSuperstructureR, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ConvertObmSegments(Bentley::ObmNET::SegmentCollectionSubR segmentCollectionSubR, BridgeStructuralPhysical::GenericSuperstructureElementR bimSuperstructureR, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ConvertObmDecks(Bentley::ObmNET::BridgeUnitR bridgeUnitR, BridgeStructuralPhysical::GenericSuperstructureElementR bimSuperstructureR, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ConvertObmBarriers(Bentley::ObmNET::BridgeUnitR bridgeUnitR, BridgeStructuralPhysical::GenericSuperstructureElementR bimSuperstructureR, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ConvertObmTendons(Bentley::ObmNET::BridgeUnitR bridgeUnitR, BridgeStructuralPhysical::GenericSuperstructureElementR bimSuperstructureR, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ProcessObmBeamSegmentCollectionSub(Bentley::ObmNET::BeamGroupR beamGroupR, BridgeStructuralPhysical::GenericSuperstructureElementR bimSuperstructureR, Dgn::PhysicalModelCP structuralSystemModelCP);
    void ConvertObmBeamSegments(Bentley::ObmNET::BeamSegmentCollectionSubR beamSegmentSubComponentR, BridgeStructuralPhysical::GenericSuperstructureElementR bimSuperstructureR, Dgn::PhysicalModelCP structuralSystemModelCP);
    
    void DetermineElementMappingAndCreate(StructuralElementType structuralElementType, Bentley::ObmNET::BMSolidR obmSolidR, Dgn::PhysicalElementR parentStructureR, Dgn::PhysicalModelCP structuralSystemModelCP, Bentley::WStringP serializedPropertiesP = nullptr);
    template<typename BimElementType> void CreateBimStructuralElement(Bentley::ObmNET::BMSolidR obmSolidR, Dgn::PhysicalModelCP structuralSystemModelCP, Dgn::PhysicalElementPtr parentStructureP = nullptr, Bentley::WStringP serializedPropertiesP = nullptr);
    template<typename BimElementType, typename BimElementTypePtr> BimElementTypePtr CreateBimStructuralElement1(Bentley::ObmNET::BMSolidR obmSolidR, Dgn::PhysicalElementP parentStructureP, Dgn::DgnCategoryId categoryId, Dgn::PhysicalModelCP structuralSystemModelCP);
    template<typename BimElementType, typename BimElementTypePtr> BimElementTypePtr CreateBimStructuralElement2(Bentley::ObmNET::BMSolidR obmSolidR, Dgn::PhysicalModelCP structuralSystemModelCP, Dgn::PhysicalElementPtr parentStructureP = nullptr, Bentley::WStringP serializedPropertiesP = nullptr);
    void UpdateCategorySelector(Dgn::PhysicalModelCP structuralSystemModelCP);
    void CreateBimElementGeometry(Bentley::ObmNET::BMSolidR, BentleyM0200::Dgn::PhysicalElementR, Dgn::DgnElementId& graphicsElementId, BentleyM0200::Utf8String& relClassName);
    void CreateGraphicsRelationship(BentleyM0200::Dgn::PhysicalElementR structuralBimElementR, Dgn::DgnElementId graphicsElementId, BentleyM0200::Utf8String relClassName);
    void GetBimBridgeFromToParams(Bentley::ObmNET::BridgeR bridgeR, double& start, double& end);
    
    void ConvertSerializedProperties(Bentley::WString serializedProperties, Dgn::GeometricElement3d& geometricElementR);
    void ConvertSerializedProperties(Bentley::WString serializedProperties, Dgn::GeometricElement3d& geometricElementR, BentleyM0200::Utf8String parentAspectName, ECN::StandaloneECInstancePtr parentAspectInstancePtr);
    void ConvertProperties(Bentley::DgnECInstanceCR ecInstanceCR, Dgn::GeometricElement3d& geometricElementR);
    BentleyM0200::ECN::ECValue CreatePropertyECValue(BentleyM0200::Utf8String propertyName, BentleyM0200::Utf8String propertyValue);

    CurveVectorPtr CreateCuttingShapeByPointAndNormal(DPoint3dCR pt, DVec3dCR normalDir);
    BentleyM0200::Dgn::IBRepEntityPtr CreateCuttingToolAlongPath(ICurvePrimitivePtr curvePrimitivePtr, double const& distanceFromStart);
    ICurvePrimitivePtr GetLongestEdge(BentleyM0200::Dgn::IBRepEntityCR brepEntity);
    void SplitBarrierIntoSmallerParts(Dgn::DgnDbSync::DgnV8::ElementConversionResults& elRes);

    Bentley::ECN::ECRelationshipClassCP FindRelationshipClass(DgnFileP dgnFile, WCharCP schemaName, WCharCP className);
    Bentley::ECN::ECClassCP FindClass(DgnFileP dgnFile, WCharCP schemaName, WCharCP className);
    DgnPlatform::DgnECRelationshipIterable FindRelationships(DgnECInstanceCR instance, Bentley::ECN::ECRelationshipClassCP relClass, Bentley::ECN::ECRelatedInstanceDirection dir);
    
}; // OBMConverter


END_ORDBRIDGE_NAMESPACE
