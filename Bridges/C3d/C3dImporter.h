/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#ifndef DWGTOOLKIT_OpenDwg
    #error C3dImporter currently only supports OpenDWG + Civil toolkit!
#endif  // DWGTOOLKIT_RealDwg

#include <Dwg/DwgImporter.h>
#include <Dwg/DwgHelper.h>
#include <Dwg/DwgBridge.h>
#include <Dwg/ProtocalExtensions.h>

#include <Teigha/Civil/DbEntity/AECCDbAlignment.h>

#include <DgnPlatform/GenericDomain.h>

#include <LinearReferencing/LinearReferencingApi.h>
#include <RoadRailAlignment/RoadRailAlignmentApi.h>
#include <RoadRailPhysical/RoadRailPhysicalApi.h>
//#include <Structural/StructuralPhysicalApi.h>
//#include <StructuralDomain/StructuralPhysicalApi.h>
//#include <BridgeStructural/BridgeStructuralPhysicalApi.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG
USING_NAMESPACE_BENTLEY_LINEARREFERENCING
USING_NAMESPACE_BENTLEY_ROADRAILALIGNMENT

// Use DwgImporter logging to trace import and to report issues
#define LOG (*NativeLogging::LoggingManager::GetLogger(L"DwgImporter"))

// Namespaces for C3dImporter
#define C3D_NAMESPACE_NAME C3D
#define BEGIN_C3D_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace C3D_NAMESPACE_NAME {
#define END_C3D_NAMESPACE   } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_C3D using namespace BENTLEY_NAMESPACE_NAME::C3D_NAMESPACE_NAME;

#define DESIGNALIGNMENTS_NAME       "Road/Rail Design Alignments"
#define ALIGNMENTS_PARTITION_NAME   "Road/Rail Physical"
#define ROADNETWORK_MODEL_NAME      "Road Network"
#define RAILNETWORK_MODEL_NAME      "Rail Network"
#define CIVIL_ALIGNED_SUBJECT       "Civil Designer Products"

#define ECCLASSNAME_AeccAligment    "AeccAlignment"

BEGIN_C3D_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          06/19
+===============+===============+===============+===============+===============+======*/
struct C3dImporter : public DwgImporter
{
    DEFINE_T_SUPER (DwgImporter)

    struct C3dOptions
        {
    public:
        bool    m_isAlignedModelPrivate;
        C3dOptions () : m_isAlignedModelPrivate(true)
            {
            }
        bool IsAlignedModelPrivate () { return m_isAlignedModelPrivate; }
        void SetAlignedModelPrivate (bool hide) { m_isAlignedModelPrivate = hide; }
        };  // C3dOptions

private:
    SpatialLocationModelPtr m_alignmentModel;
    PhysicalModelPtr    m_roadNetworkModel;
    PhysicalModelPtr    m_railNetworkModel;
    ECSchemaCP          m_c3dSchema;
    C3dOptions          m_c3dOptions;
    
private:
    BentleyStatus   ConvertC3d (DgnElementR element, DwgDbEntityCR entity);
    SubjectCPtr GetAlignmentSubject ();
    void ParseC3dConfigurations ();

public:
    // Constructor
    EXPORT_ATTRIBUTE C3dImporter(DwgImporter::Options& options);
    // Initialization
    EXPORT_ATTRIBUTE static void Initialize ();
    // Destructor
    EXPORT_ATTRIBUTE ~C3dImporter ();
    // Overridden methods
    EXPORT_ATTRIBUTE BentleyStatus  _MakeSchemaChanges() override;
    EXPORT_ATTRIBUTE BentleyStatus  _ImportEntitySection () override;
    EXPORT_ATTRIBUTE BentleyStatus  _ImportEntity(ElementImportResults& results, ElementImportInputs& inputs) override;

    // C3dImporter methods
    EXPORT_ATTRIBUTE BentleyStatus  OnBaseBridgeJobFound (DgnElementId jobId);
    PhysicalModelPtr    GetRoadNetworkModel ();
    PhysicalModelPtr    GetRailNetworkModel ();
    SpatialLocationModelPtr GetAlignmentModel () { return m_alignmentModel; }
    ECSchemaCP  GetC3dSchema () const { return m_c3dSchema; }
};  // C3dImporter
DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(C3dImporter)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          11/19
+===============+===============+===============+===============+===============+======*/
class AeccAlignmentExt : public DwgProtocolExtension
{
public:
    DEFINE_T_SUPER (DwgProtocolExtension)
    DWGRX_DECLARE_MEMBERS (AeccAlignmentExt)
    DWG_PROTOCOLEXT_DECLARE_MEMBERS (AeccAlignmentExt)

    virtual BentleyStatus  _ConvertToBim (ProtocolExtensionContext& context, DwgImporterR importer) override;

private:
    mutable Utf8String  m_name;
    mutable Utf8String  m_description;
    mutable C3dImporterP    m_importer;
    mutable AECCDbAlignment*    m_aeccAlignment;
    mutable SpatialLocationModelPtr m_alignmentModel;
    mutable ProtocolExtensionContext* m_toDgnContext;
    
    ECClassCP   GetECClass (Utf8StringCR className) const;
    BentleyStatus   SetDesignSpeeds (DgnElementR element);
    BentleyStatus   CreateAeccAlignment ();
    BentleyStatus   CreateAlignment ();
};  // AeccAlignmentExt

END_C3D_NAMESPACE
