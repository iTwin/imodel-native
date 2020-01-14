/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#ifndef DWGTOOLKIT_OpenDwg
    #error C3dImporter currently only supports OpenDWG + Civil toolkit!
#endif  // DWGTOOLKIT_OpenDwg

#include <Dwg/DwgImporter.h>

// Namespaces for C3dImporter
#define C3D_NAMESPACE_NAME C3D
#define BEGIN_C3D_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace C3D_NAMESPACE_NAME {
#define END_C3D_NAMESPACE   } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_C3D using namespace BENTLEY_NAMESPACE_NAME::C3D_NAMESPACE_NAME;

// Declare common members for a C3D protocol extension
#define DEFINE_C3DPROTOCOLEXTENSION(__extclassname__)   \
    DEFINE_T_SUPER(DwgProtocolExtension)                \
    DWGRX_DECLARE_MEMBERS(__extclassname__)             \
    DWG_PROTOCOLEXT_DECLARE_MEMBERS(__extclassname__)
    

// C3d schema names
#define C3DSCHEMA_SchemaName    "AdskCivil3dSchema"
#define C3DSCHEMA_SchemaAlias   "C3dSchema"
#define C3DSCHEMA_VERSION_Major 1
#define C3DSCHEMA_VERSION_Write 0
#define C3DSCHEMA_VERSION_Minor 0
#define C3DSCHEMA_FileName      L"C3dSchema.01.00.00.ecschema.xml"
#define C3DSCHEMA(name)         C3DSCHEMA_SchemaAlias "." name

// Civil domain names
#define DESIGNALIGNMENTS_NAME       "Road/Rail Design Alignments"
#define ALIGNMENTS_PARTITION_NAME   "Road/Rail Physical"
#define ROADNETWORK_MODEL_NAME      "Road Network"
#define RAILNETWORK_MODEL_NAME      "Rail Network"
#define CIVIL_ALIGNED_SUBJECT       "Civil Designer Products"

// Entities
#define ECCLASSNAME_AeccAlignment       "AeccAlignment"
#define ECCLASSNAME_AeccVAlignment      "AeccVAlignment"
#define ECCLASSNAME_AeccCorridor        "AeccCorridor"
#define ECCLASSNAME_AeccFeatureLine     "AeccFeatureLine"
// Structs
#define ECCLASSNAME_DesignSpeed         "DesignSpeed"
#define ECCLASSNAME_VAlignment          "VAlignment"
#define ECCLASSNAME_CorridorParameters  "CorridorParameters"
#define ECCLASSNAME_CorridorFeature     "CorridorFeature"
#define ECCLASSNAME_CorridorCode        "CorridorCode"
#define ECCLASSNAME_CorridorRegion      "CorridorRegion"
// Properties
#define ECPROPNAME_Code                 "Code"
#define ECPROPNAME_Comment              "Comment"
#define ECPROPNAME_CorridorParameters   "CorridorParameters"
#define ECPROPNAME_CorridorFeatures     "CorridorFeatures"
#define ECPROPNAME_DesignSpeed          "DesignSpeed"
#define ECPROPNAME_DesignSpeeds         "DesignSpeeds"
#define ECPROPNAME_Description          "Description"
#define ECPROPNAME_EndOffset            "EndOffset"
#define ECPROPNAME_EndStation           "EndStation"
#define ECPROPNAME_FeatureLineStyle     "FeatureLineStyle"
#define ECPROPNAME_HorizontalAlignment  "HorizontalAlignment"
#define ECPROPNAME_Length               "Length"
#define ECPROPNAME_Length2d             "Length2d"
#define ECPROPNAME_Length3d             "Length3d"
#define ECPROPNAME_LinkCodes            "LinkCodes"
#define ECPROPNAME_MaxElevation         "MaxElevation"
#define ECPROPNAME_MinElevation         "MinElevation"
#define ECPROPNAME_Name                 "Name"
#define ECPROPNAME_NumberOfPoints       "NumberOfPoints"
#define ECPROPNAME_PointCodes           "PointCodes"
#define ECPROPNAME_ReferencePoint       "ReferencePoint"
#define ECPROPNAME_ReferenceStation     "ReferenceStation"
#define ECPROPNAME_Regions              "Regions"
#define ECPROPNAME_SampleOffset         "SampleOffset"
#define ECPROPNAME_ShapeCodes           "ShapeCodes"
#define ECPROPNAME_Station              "Station"
#define ECPROPNAME_StartStation         "StartStation"
#define ECPROPNAME_StartOffset          "StartOffset"
#define ECPROPNAME_Style                "Style"
#define ECPROPNAME_VAlignment           "VAlignment"
#define ECPROPNAME_VAlignments          "VAlignments"
#define ECPROPNAME_VerticalAlignment    "VerticalAlignment"


USING_NAMESPACE_DWG

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
    ECN::ECSchemaCP     m_c3dSchema;
    C3dOptions          m_c3dOptions;
    bset<Utf8String>    m_entitiesNeedProxyGeometry;
    
private:
    SubjectCPtr GetAlignmentSubject ();
    void ParseC3dConfigurations ();

public:
    // Constructor
    EXPORT_ATTRIBUTE C3dImporter(DwgImporter::Options& options);
    // Destructor
    EXPORT_ATTRIBUTE ~C3dImporter ();
    // Overridden methods
    EXPORT_ATTRIBUTE BentleyStatus  _MakeSchemaChanges() override;
    EXPORT_ATTRIBUTE BentleyStatus  _ImportEntitySection () override;
    EXPORT_ATTRIBUTE BentleyStatus  _ImportEntity(ElementImportResultsR results, ElementImportInputsR inputs) override;
    EXPORT_ATTRIBUTE Utf8String     _ComputeImportJobName (DwgDbBlockTableRecordCR modelspaceBlock) const override;
    EXPORT_ATTRIBUTE bool           _FilterEntity (ElementImportInputsR inputs) const override;
    EXPORT_ATTRIBUTE bool _CreateObjectProvenance (BentleyApi::MD5::HashVal& hash, DwgDbObjectCR object) override;
    EXPORT_ATTRIBUTE void _SetChangeDetector (bool updating) override;

    // C3dImporter methods
    EXPORT_ATTRIBUTE BentleyStatus  OnBaseBridgeJobInitialized (DgnElementId jobId);
    EXPORT_ATTRIBUTE BentleyStatus  OnBaseBridgeJobFound (DgnElementId jobId);
    PhysicalModelPtr    GetRoadNetworkModel ();
    PhysicalModelPtr    GetRailNetworkModel ();
    SpatialLocationModelPtr GetAlignmentModel () { return m_alignmentModel; }
    ECN::ECSchemaCP     GetC3dSchema () const { return m_c3dSchema; }
    ECN::ECClassCP      GetC3dECClass (Utf8StringCR name) const;
    ECN::StandaloneECInstancePtr CreateC3dECInstance (Utf8StringCR className) const;
    DgnDbStatus InsertArrayProperty (DgnElementR element, Utf8StringCR propertyName, uint32_t arraySize) const;
    IDwgChangeDetector& GetChangeDetector () { return T_Super::_GetChangeDetector(); }
    BentleyStatus   ProcessDetectionResults (IDwgChangeDetector::DetectionResultsR detected, ElementImportResultsR results, ElementImportInputsR inputs) { return T_Super::_ProcessDetectionResults(detected, results, inputs); }
};  // C3dImporter
DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(C3dImporter)


/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          12/19
+===============+===============+===============+===============+===============+======*/
struct C3dUpdaterChangeDetector : public UpdaterChangeDetector
{
public:
    DEFINE_T_SUPER (UpdaterChangeDetector)
    EXPORT_ATTRIBUTE void _DeleteElement (DgnDbR db, DwgSourceAspects::ObjectAspectCR elementAspect) override;
};  // C3dUpdaterChangeDetector

END_C3D_NAMESPACE
