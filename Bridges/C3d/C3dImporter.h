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
#define C3DSCHEMA_FileName      L"C3dSchema.ecschema.xml"
#define C3DSCHEMA(name)         C3DSCHEMA_SchemaAlias "." name

// Civil domain names
#define DESIGNALIGNMENTS_NAME       "Road/Rail Design Alignments"
#define ALIGNMENTS_PARTITION_NAME   "Road/Rail Physical"
#define ROADNETWORK_MODEL_NAME      "Road Network"
#define RAILNETWORK_MODEL_NAME      "Rail Network"
#define CIVIL_ALIGNED_SUBJECT       "Civil Alignment"
#define TRANSPORTATIONSYSTEM_NAME   "Transportation System"

// Entities
#define ECCLASSNAME_AeccAlignment       "AeccAlignment"
#define ECCLASSNAME_AeccVAlignment      "AeccVAlignment"
#define ECCLASSNAME_AeccCorridor        "AeccCorridor"
#define ECCLASSNAME_AeccFeatureLine     "AeccFeatureLine"
#define ECCLASSNAME_AeccPipe            "AeccPipe"
#define ECCLASSNAME_AeccStructure       "AeccStructure"
// Structs
#define ECCLASSNAME_DesignSpeed         "DesignSpeed"
#define ECCLASSNAME_VAlignment          "VAlignment"
#define ECCLASSNAME_CorridorParameters  "CorridorParameters"
#define ECCLASSNAME_CorridorFeature     "CorridorFeature"
#define ECCLASSNAME_CorridorCode        "CorridorCode"
#define ECCLASSNAME_CorridorRegion      "CorridorRegion"
// Properties
#define ECPROPNAME_AutoSurfaceAdjustment "AutoSurfaceAdjustment"
#define ECPROPNAME_BarrelPipeClearance  "BarrelPipeClearance"
#define ECPROPNAME_Code                 "Code"
#define ECPROPNAME_ConeHeight           "ConeHeight"
#define ECPROPNAME_Comment              "Comment"
#define ECPROPNAME_ConnectedPipes       "ConnectedPipes"
#define ECPROPNAME_ControlSumpBy        "ControlSumpBy"
#define ECPROPNAME_CorridorParameters   "CorridorParameters"
#define ECPROPNAME_CorridorFeatures     "CorridorFeatures"
#define ECPROPNAME_Cover                "Cover"
#define ECPROPNAME_CrossSectionalShape  "CrossSectionalShape"
#define ECPROPNAME_DesignSpeed          "DesignSpeed"
#define ECPROPNAME_DesignSpeeds         "DesignSpeeds"
#define ECPROPNAME_Description          "Description"
#define ECPROPNAME_EndOffset            "EndOffset"
#define ECPROPNAME_EndStation           "EndStation"
#define ECPROPNAME_EndStructure         "EndStructure"
#define ECPROPNAME_EnergyGradeLineUp    "EnergyGradeLineUp"
#define ECPROPNAME_EnergyGradeLineDown  "EnergyGradeLineDown"
#define ECPROPNAME_FeatureLineStyle     "FeatureLineStyle"
#define ECPROPNAME_FloorThickness       "FloorThickness"
#define ECPROPNAME_FlowDirectionMethod  "FlowDirectionMethod"
#define ECPROPNAME_FlowRate             "FlowRate"
#define ECPROPNAME_Frame                "Frame"
#define ECPROPNAME_FrameDiameter        "FrameDiameter"
#define ECPROPNAME_FrameHeight          "FrameHeight"
#define ECPROPNAME_Grate                "Grate"
#define ECPROPNAME_HorizontalAlignment  "HorizontalAlignment"
#define ECPROPNAME_HydraGradeLineDown   "HydraGradeLineDown"
#define ECPROPNAME_HydraGradeLineUp     "HydraGradeLineUp"
#define ECPROPNAME_InnerPipeDiameter    "InnerPipeDiameter"
#define ECPROPNAME_InnerStructDiameter  "InnerStructDiameter"
#define ECPROPNAME_InsertionElevation   "InsertionElevation"
#define ECPROPNAME_JunctionLoss         "JunctionLoss"
#define ECPROPNAME_Length               "Length"
#define ECPROPNAME_Length2d             "Length2d"
#define ECPROPNAME_Length2dCenterToCenter "Length2dCenterToCenter"
#define ECPROPNAME_Length3d             "Length3d"
#define ECPROPNAME_Length3dCenterToCenter "Length3dCenterToCenter"
#define ECPROPNAME_LinkCodes            "LinkCodes"
#define ECPROPNAME_ManningCoefficient   "ManningCoefficient"
#define ECPROPNAME_MaxElevation         "MaxElevation"
#define ECPROPNAME_Material             "Material"
#define ECPROPNAME_MinElevation         "MinElevation"
#define ECPROPNAME_Name                 "Name"
#define ECPROPNAME_NumberOfPoints       "NumberOfPoints"
#define ECPROPNAME_PartDescription      "PartDescription"
#define ECPROPNAME_PartID               "PartID"
#define ECPROPNAME_PartName             "PartName"
#define ECPROPNAME_PartSizeName         "PartSizeName"
#define ECPROPNAME_PartSubType          "PartSubType"
#define ECPROPNAME_PartType             "PartType"
#define ECPROPNAME_PointCodes           "PointCodes"
#define ECPROPNAME_ReferenceAlignment   "ReferenceAlignment"
#define ECPROPNAME_ReferencePoint       "ReferencePoint"
#define ECPROPNAME_ReferenceStation     "ReferenceStation"
#define ECPROPNAME_ReferenceSurface     "ReferenceSurface"
#define ECPROPNAME_Regions              "Regions"
#define ECPROPNAME_RimToSumpHeight      "RimToSumpHeight"
#define ECPROPNAME_SampleOffset         "SampleOffset"
#define ECPROPNAME_ShapeCodes           "ShapeCodes"
#define ECPROPNAME_SlabThickness        "SlabThickness"
#define ECPROPNAME_Slope                "Slope"
#define ECPROPNAME_StartOffset          "StartOffset"
#define ECPROPNAME_StartStation         "StartStation"
#define ECPROPNAME_StartStructure       "StartStructure"
#define ECPROPNAME_Station              "Station"
#define ECPROPNAME_StructureDiameter    "StructureDiameter"
#define ECPROPNAME_StructureHeight      "StructureHeight"
#define ECPROPNAME_StructureShape       "StructureShape"
#define ECPROPNAME_Style                "Style"
#define ECPROPNAME_SumpDepth            "SumpDepth"
#define ECPROPNAME_SurfaceAdjustment    "SurfaceAdjustment"
#define ECPROPNAME_VAlignment           "VAlignment"
#define ECPROPNAME_VAlignments          "VAlignments"
#define ECPROPNAME_VerticalAlignment    "VerticalAlignment"
#define ECPROPNAME_VerticalPipeClearance "VerticalPipeClearance"
#define ECPROPNAME_WallThickness        "WallThickness"


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
        bool IsAlignedModelPrivate () const { return m_isAlignedModelPrivate; }
        void SetAlignedModelPrivate (bool hide) { m_isAlignedModelPrivate = hide; }
        };  // C3dOptions

private:
    SpatialLocationModelPtr m_alignmentModel;
    PhysicalModelPtr    m_roadNetworkModel;
    PhysicalModelPtr    m_railNetworkModel;
    ECN::ECSchemaCP     m_c3dSchema;
    C3dOptions          m_c3dOptions;
    bset<Utf8String>    m_entitiesNeedProxyGeometry;
    DwgDbObjectIdArray  m_entitiesForPostImport;
    
private:
    SubjectCPtr GetAlignmentSubject ();
    void ParseC3dConfigurations ();
    void SetRenderableDefaultView ();

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
    EXPORT_ATTRIBUTE bool           _FilterEntity (ElementImportInputsR inputs) override;
    EXPORT_ATTRIBUTE DgnElementPtr  _CreateElement (DgnElement::CreateParams& params, ElementImportInputsR inputs, size_t elementIndex) override;
    EXPORT_ATTRIBUTE DPoint3d       _GetElementPlacementPoint (DwgDbEntityCR entity) override;
    EXPORT_ATTRIBUTE bool _CreateObjectProvenance (BentleyApi::MD5::HashVal& hash, DwgDbObjectCR object) override;
    EXPORT_ATTRIBUTE void _SetChangeDetector (bool updating) override;
    EXPORT_ATTRIBUTE bool _AllowEntityMaterialOverrides (DwgDbEntityCR entity) const override;
    EXPORT_ATTRIBUTE BentleyStatus  _ProcessDetectionResults (IDwgChangeDetector::DetectionResultsR detected, ElementImportResultsR results, ElementImportInputsR inputs) override;
    EXPORT_ATTRIBUTE void           _PostProcessViewports () override;

    // C3dImporter methods
    EXPORT_ATTRIBUTE BentleyStatus  OnBaseBridgeJobInitialized (DgnElementId jobId);
    EXPORT_ATTRIBUTE BentleyStatus  OnBaseBridgeJobFound (DgnElementId jobId);
    PhysicalModelPtr    GetRoadNetworkModel () { return m_roadNetworkModel; }
    PhysicalModelPtr    GetRailNetworkModel () { return m_railNetworkModel; }
    SpatialLocationModelPtr GetAlignmentModel () { return m_alignmentModel; }
    ECN::ECSchemaCP     GetC3dSchema () const { return m_c3dSchema; }
    ECN::ECClassCP      GetC3dECClass (Utf8StringCR name) const;
    ECN::StandaloneECInstancePtr CreateC3dECInstance (Utf8StringCR className) const;
    DgnDbStatus InsertArrayProperty (DgnElementR element, Utf8StringCR propertyName, uint32_t arraySize) const;
    GeometryOptions& GetCurrentGeometryOptions () { return T_Super::_GetCurrentGeometryOptions(); }
    IDwgChangeDetector& GetChangeDetector () { return T_Super::_GetChangeDetector(); }
    C3dOptions const&   GetC3dOptions () const { return m_c3dOptions; }
    ResolvedModelMapping GetRootModel () const { return  T_Super::GetRootModel(); }
    void    RegisterEntityForPostImport (DwgDbObjectIdCR id) { m_entitiesForPostImport.push_back(id); }
    bool    IsUpdating() const { return T_Super::IsUpdating(); }
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
