/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Dwg/ProtocalExtensions.h>

BEGIN_C3D_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          11/19
+===============+===============+===============+===============+===============+======*/
class AeccAlignmentExt : public DwgProtocolExtension
{
public:
    DEFINE_C3DPROTOCOLEXTENSION(AeccAlignmentExt)

    BentleyStatus  _ConvertToBim (ProtocolExtensionContext& context, DwgImporterR importer) override;
    static BentleyStatus UpdateElementRepresentedBy (DgnDbR db, DgnElementId civilAlignmentId, DgnElementId aeccAlignmentId);

private:
    mutable Utf8String  m_name;
    mutable Utf8String  m_description;
    mutable C3dImporterP    m_importer;
    mutable AECCDbAlignment*    m_aeccAlignment;
    mutable RoadRailAlignment::AlignmentPtr m_baseAlignment;
    mutable SpatialLocationModelPtr m_alignmentModel;
    mutable DgnModelId  m_verticalAlignmentModelId;
    mutable ProtocolExtensionContext* m_toDgnContext;
    
    // C3D elements
    BentleyStatus   SetVAlignmentProperties (DgnElementR element);
    BentleyStatus   SetDesignSpeedProperties (DgnElementR element);
    BentleyStatus   DetectAndImportAeccVAlignment (DwgImporter::ElementImportInputs& inputs);
    BentleyStatus   CreateOrUpdateAeccVAlignments ();
    BentleyStatus   CreateOrUpdateAeccAlignment ();
    // Civil domain elements
    BentleyStatus   CreateVerticalAlignment (CurveVectorCR curves, DwgImporter::ElementImportResultsR results);
    BentleyStatus   UpdateVerticalAlignment (CurveVectorCR curves, DwgImporter::ElementImportResultsR results);
    BentleyStatus   CreateHorizontalAlignment (CurveVectorCR curves, GeometrySourceCP geomSource, DwgImporter::ElementImportResultsR results, size_t index = 0);
    BentleyStatus   UpdateHorizontalAlignment (CurveVectorCR curves, GeometrySourceCP geomSource, DwgImporter::ElementImportResultsR results, size_t index = 0);
    BentleyStatus   CreateOrUpdateVerticalAlignment (DwgImporter::ElementImportResultsR aeccResults, AECCDbVAlignment* aeccVAlignment);
    BentleyStatus   CreateOrUpdateHorizontalAlignment ();
    // Top level
    BentleyStatus   ImportVAlignments ();
    BentleyStatus   ImportAlignment ();
};  // AeccAlignmentExt

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          11/19
+===============+===============+===============+===============+===============+======*/
class AeccCorridorExt : public DwgProtocolExtension
{
public:
    DEFINE_C3DPROTOCOLEXTENSION(AeccCorridorExt)

    BentleyStatus  _ConvertToBim (ProtocolExtensionContext& context, DwgImporterR importer) override;

private:
    mutable Utf8String  m_name;
    mutable Utf8String  m_description;
    mutable DgnElementP m_importedElement;
    mutable ECN::IECInstancePtr  m_parametersInstance;
    mutable C3dImporterP    m_importer;
    mutable AECCDbCorridor* m_aeccCorridor;
    mutable ProtocolExtensionContext* m_toDgnContext;
    
#ifdef FEATURE_COLLECTIONS
    BentleyStatus ProcessFeatureCollections (AECCCorridorBaseline const& baseline);
#endif
    DgnDbStatus ProcessCode (OdString const& code, AECCSubassemblyEntTraits const& subassentTraits, Utf8StringCR propName, uint32_t index);
    BentleyStatus ProcessRegions (AECCCorridorBaseline const& baseline);
    BentleyStatus ProcessBaseline (AECCCorridorBaseline const& baseline);
    BentleyStatus ProcessBaselines ();
    BentleyStatus ProcessFeatureStyles ();
    BentleyStatus ProcessCodes ();
    BentleyStatus ImportCorridor ();
};  // AeccCorridorExt

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/20
+===============+===============+===============+===============+===============+======*/
class AeccFeatureLineExt : public DwgProtocolExtension
{
public:
    DEFINE_C3DPROTOCOLEXTENSION(AeccFeatureLineExt)

    BentleyStatus  _ConvertToBim (ProtocolExtensionContext& context, DwgImporterR importer) override;

private:
    mutable DgnElementP m_importedElement;
    mutable C3dImporterP m_importer;
    mutable AECCDbFeatureLine* m_aeccFeatureLine;
    mutable ProtocolExtensionContext* m_toDgnContext;

    BentleyStatus ImportFeatureLine ();
};  // AeccFeatureLine

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/20
+===============+===============+===============+===============+===============+======*/
class AeccPipeExt : public DwgProtocolExtension
{
public:
    DEFINE_C3DPROTOCOLEXTENSION(AeccPipeExt)

    BentleyStatus  _ConvertToBim (ProtocolExtensionContext& context, DwgImporterR importer) override;

private:
    mutable C3dImporterP m_importer;
    mutable AECCDbPipe* m_aeccPipe;
    mutable ProtocolExtensionContext* m_toDgnContext;

    ECObjectsStatus SetStringPartProperty (Utf8CP propName, AECCVariant const& var, IECInstanceR ecInstance);
    void SetPartData (IECInstanceR ecInstance);
    BentleyStatus CreateOrUpdateAeccPipe ();
    BentleyStatus ImportAeccPipe ();
    BentleyStatus ImportPipe ();
};  // AeccPipe

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/20
+===============+===============+===============+===============+===============+======*/
class AeccStructureExt : public DwgProtocolExtension
{
public:
    DEFINE_C3DPROTOCOLEXTENSION(AeccStructureExt)

    BentleyStatus  _ConvertToBim (ProtocolExtensionContext& context, DwgImporterR importer) override;

private:
    mutable C3dImporterP m_importer;
    mutable AECCDbStructure* m_aeccStructure;
    mutable ProtocolExtensionContext* m_toDgnContext;

    ECObjectsStatus SetStringPartProperty (Utf8CP propName, AECCVariant const& var, IECInstanceR ecInstance);
    void SetPartRecord (IECInstanceR ecInstance);
    void SetPartDefinition (IECInstanceR ecInstance);
    BentleyStatus CreateOrUpdateAeccStructure ();
    BentleyStatus ImportAeccStructure ();
    BentleyStatus ImportStructure ();
};  // AeccStructure

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          06/19
+===============+===============+===============+===============+===============+======*/
struct C3dProtocolExtensions
{
public:
    OdStaticRxObject<AeccAlignmentExt>  m_aeccAlignmentExt;
    OdStaticRxObject<AeccCorridorExt>   m_aeccCorridorExt;
    OdStaticRxObject<AeccFeatureLineExt> m_aeccFeatureLineExt;
    OdStaticRxObject<AeccPipeExt>   m_aeccPipeExt;
    OdStaticRxObject<AeccStructureExt>  m_aeccStructureExt;
    C3dProtocolExtensions ()
        {
        try
            {
            AECCDbAlignment::rxInit ();
            AeccAlignmentExt::RxInit ();
            DwgRxClass::AddProtocolExtension (AECCDbAlignment::desc(), AeccAlignmentExt::Desc(), &m_aeccAlignmentExt);

            AECCDbCorridor::rxInit ();
            AeccCorridorExt::RxInit ();
            DwgRxClass::AddProtocolExtension (AECCDbCorridor::desc(), DwgProtocolExtension::Desc(), &m_aeccCorridorExt);

            AECCDbFeatureLine::rxInit ();
            AeccFeatureLineExt::RxInit ();
            DwgRxClass::AddProtocolExtension (AECCDbFeatureLine::desc(), DwgProtocolExtension::Desc(), &m_aeccFeatureLineExt);

            AECCDbPipe::rxInit ();
            AeccPipeExt::RxInit ();
            DwgRxClass::AddProtocolExtension (AECCDbPipe::desc(), DwgProtocolExtension::Desc(), &m_aeccPipeExt);

            AECCDbStructure::rxInit ();
            AeccStructureExt::RxInit ();
            DwgRxClass::AddProtocolExtension (AECCDbStructure::desc(), DwgProtocolExtension::Desc(), &m_aeccStructureExt);
            }
        catch (OdError& error)
            {
            LOG.debugv ("Toolkit Exception: %ls", error.description().c_str());
            }
        }

    ~C3dProtocolExtensions ()
        {
        DwgRxClass::DeleteProtocolExtension (AECCDbAlignment::desc(), AeccAlignmentExt::Desc());
        AeccAlignmentExt::RxUnInit ();
        AECCDbAlignment::rxUninit ();

        DwgRxClass::DeleteProtocolExtension (AECCDbCorridor::desc(), AeccCorridorExt::Desc());
        AeccCorridorExt::RxUnInit ();
        AECCDbCorridor::rxUninit ();

        DwgRxClass::DeleteProtocolExtension (AECCDbFeatureLine::desc(), AeccFeatureLineExt::Desc());
        AeccFeatureLineExt::RxUnInit ();
        AECCDbFeatureLine::rxUninit ();

        DwgRxClass::DeleteProtocolExtension (AECCDbPipe::desc(), AeccPipeExt::Desc());
        AeccPipeExt::RxUnInit ();
        AECCDbPipe::rxUninit ();

        DwgRxClass::DeleteProtocolExtension (AECCDbStructure::desc(), AeccStructureExt::Desc());
        AeccStructureExt::RxUnInit ();
        AECCDbStructure::rxUninit ();
        }

};  // C3dProtocolExtensions

END_C3D_NAMESPACE
