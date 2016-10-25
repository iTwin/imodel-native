/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnBRep/SolidKernelUtil.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#if defined (BENTLEYCONFIG_OPENCASCADE) 
#include <DgnPlatform/DgnBRep/OCBRep.h>
#elif defined (BENTLEYCONFIG_PARASOLID) 
#include <PSolid/frustrum_tokens.h>
#include <PSolid/kernel_interface.h>
#include <PSolid/parasolid_kernel.h>
#include <PSolid/parasolid_debug.h>
#include <PSolid/frustrum_ifails.h>
#endif

enum SolidDataVersion
    {
    DataVersion_PSolid = 120, //!< Transmit schema version used to persist Parasolid data in a dgn file.
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/12
+---------------+---------------+---------------+---------------+---------------+------*/
FaceAttachment::FaceAttachment ()
    {
    m_useColor = m_useMaterial = false;
    m_color = ColorDef::Black();
    m_transparency = 0.0;
    m_uv.Init(0.0, 0.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/12
+---------------+---------------+---------------+---------------+---------------+------*/
FaceAttachment::FaceAttachment (GeometryParamsCR sourceParams)
    {
    m_categoryId    = sourceParams.GetCategoryId();
    m_subCategoryId = sourceParams.GetSubCategoryId();
    m_transparency  = sourceParams.GetTransparency();

    m_useColor = !sourceParams.IsLineColorFromSubCategoryAppearance();
    m_color = m_useColor ? sourceParams.GetLineColor() : ColorDef::Black();

    if (m_useMaterial = !sourceParams.IsMaterialFromSubCategoryAppearance())
        m_material = sourceParams.GetMaterialId();

    m_uv.Init(0.0, 0.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/12
+---------------+---------------+---------------+---------------+---------------+------*/
void FaceAttachment::ToGeometryParams (GeometryParamsR elParams) const
    {
    elParams.SetCategoryId(m_categoryId);
    elParams.SetSubCategoryId(m_subCategoryId);
    elParams.SetTransparency(m_transparency);

    if (m_useColor)
        elParams.SetLineColor(m_color);

    if (m_useMaterial)
        elParams.SetMaterialId(m_material);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool FaceAttachment::operator==(struct FaceAttachment const& rhs) const
    {
    if (m_useColor      != rhs.m_useColor ||
        m_useMaterial   != rhs.m_useMaterial ||
        m_categoryId    != rhs.m_categoryId ||
        m_subCategoryId != rhs.m_subCategoryId ||
        m_color         != rhs.m_color ||
        m_transparency  != rhs.m_transparency ||
        m_material      != rhs.m_material ||
        m_uv.x          != rhs.m_uv.x || 
        m_uv.y          != rhs.m_uv.y)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool FaceAttachment::operator< (struct FaceAttachment const& rhs) const
    {
    return (m_useColor         < rhs.m_useColor ||
            m_useMaterial      < rhs.m_useMaterial ||
            m_categoryId       < rhs.m_categoryId ||
            m_subCategoryId    < rhs.m_subCategoryId ||
            m_color.GetValue() < rhs.m_color.GetValue() ||
            m_transparency     < rhs.m_transparency ||
            m_material         < rhs.m_material ||
            m_uv.x             < rhs.m_uv.x || 
            m_uv.y             < rhs.m_uv.y);
    }

#if defined (BENTLEYCONFIG_PARASOLID)
/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  12/12
+===============+===============+===============+===============+===============+======*/
struct FaceMaterialAttachments : RefCounted <IFaceMaterialAttachments>
{
private:

T_FaceAttachmentsVec    m_faceAttachmentsVec;
T_FaceToSubElemIdMap    m_faceToSubElemIdMap;

public:

FaceMaterialAttachments() {}

virtual T_FaceAttachmentsVec const& _GetFaceAttachmentsVec() const override {return m_faceAttachmentsVec;}
virtual T_FaceToSubElemIdMap const& _GetFaceToSubElemIdMap() const override {return m_faceToSubElemIdMap;}

virtual T_FaceAttachmentsVec& _GetFaceAttachmentsVecR() override {return m_faceAttachmentsVec;}
virtual T_FaceToSubElemIdMap& _GetFaceToSubElemIdMapR() override {return m_faceToSubElemIdMap;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
static IFaceMaterialAttachmentsPtr Create (ISolidKernelEntityCR entity, Render::GeometryParamsCR baseParams)
    {
#if defined (NOT_NOW)
    int         nFaces;
    PK_FACE_t*  faces = NULL;

    if (SUCCESS != PK_BODY_ask_faces (PSolidUtil::GetEntityTag (entity), &nFaces, &faces))
        return NULL;

    FaceMaterialAttachments* attachments = new FaceMaterialAttachments ();

    attachments->_GetFaceAttachmentsVecR().push_back(FaceAttachment(baseParams));

    for (int iFace = 0; iFace < nFaces; iFace++)
        attachments->_GetFaceToSubElemIdMapR()[faces[iFace]] = make_bpair(iFace + 1, 0);
    
    PK_MEMORY_free (faces);

    return attachments;
#else
    return nullptr;
#endif
    }

}; // FaceMaterialAttachments

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  04/09
+===============+===============+===============+===============+===============+======*/
struct PSolidKernelEntity : public RefCounted <ISolidKernelEntity>
{
private:

PK_ENTITY_t                 m_entityTag;
Transform                   m_transform;
bool                        m_owned;
mutable PK_MEMORY_block_t   m_block;
IFaceMaterialAttachmentsPtr m_faceAttachments;

protected:

virtual Transform _GetEntityTransform() const override {return m_transform;}
virtual bool _SetEntityTransform(TransformCR transform) override {m_transform = transform; return true;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
EntityType _GetEntityType() const
    {
    PK_BODY_type_t  bodyType = PK_BODY_type_unspecified_c;

    PK_BODY_ask_type(m_entityTag, &bodyType);

    switch (bodyType)
        {
        case PK_BODY_type_solid_c:
            return ISolidKernelEntity::EntityType::Solid;

        case PK_BODY_type_sheet_c:
            return ISolidKernelEntity::EntityType::Sheet;

        case PK_BODY_type_wire_c:
            return ISolidKernelEntity::EntityType::Wire;

        default:
            return ISolidKernelEntity::EntityType::Minimal;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d _GetEntityRange() const
    {
    PK_BOX_t    box;

    if (PK_ERROR_no_errors != PK_TOPOL_find_box(m_entityTag, &box))
        return DRange3d::NullRange();

    DRange3d    range = DRange3d::From(box.coord[0], box.coord[1], box.coord[2], box.coord[3], box.coord[4], box.coord[5]);

    m_transform.Multiply(range, range);

    return range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsEqual(ISolidKernelEntityCR entity) const override
    {
    PSolidKernelEntity const* psEntity;

    if (NULL == (psEntity = dynamic_cast <PSolidKernelEntity const*> (&entity)))
        return false;

    if (m_entityTag != psEntity->GetEntityTag())
        return false;

    if (!m_transform.IsEqual(psEntity->GetEntityTransform(), 1.0e-8, 1.0e-8))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual IFaceMaterialAttachmentsCP _GetFaceMaterialAttachments() const override
    {
    return m_faceAttachments.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _InitFaceMaterialAttachments(Render::GeometryParamsCP baseParams) override
    {
    if (nullptr == baseParams)
        {
        m_faceAttachments = nullptr;

        return true;
        }

    IFaceMaterialAttachmentsPtr attachments = FaceMaterialAttachments::Create(*this, *baseParams);

    if (!attachments.IsValid())
        return false;

    m_faceAttachments = attachments;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ISolidKernelEntityPtr _Clone() const override
    {
    PK_ENTITY_t entityTag = PK_ENTITY_null;

    PK_ENTITY_copy(m_entityTag, &entityTag);

    if (PK_ENTITY_null == entityTag)
        return nullptr;

    return PSolidKernelEntity::CreateNewEntity(entityTag, m_transform);
    }

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
PSolidKernelEntity(PK_ENTITY_t entityTag, TransformCR transform, bool owned)
    {
    m_entityTag = entityTag;
    m_transform = transform;
    m_owned     = owned;

    memset (&m_block, 0, sizeof(m_block));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
~PSolidKernelEntity()
    {
    PK_MEMORY_block_f(&m_block);

    if (!m_owned)
        return;

    PK_ENTITY_delete(1, &m_entityTag);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
PK_MEMORY_block_t* GetPrivateMemoryBlock() const
    {
    PK_MEMORY_block_f(&m_block); // Avoid leak if _SaveEntityToMemory called twice on same entity...

    return &m_block;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsOwnedEntity() const
    {
    return m_owned;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ENTITY_t GetEntityTag() const
    {
    return m_entityTag;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ENTITY_t ExtractEntityTag()
    {
    if (!m_owned)
        return PK_ENTITY_null;
        
    PK_ENTITY_t entityTag = m_entityTag;
    m_entityTag = PK_ENTITY_null;
    
    return entityTag;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
static PSolidKernelEntity* CreateNewEntity(PK_ENTITY_t entityTag, TransformCR transform, bool owned = true)
    {
    return new PSolidKernelEntity(entityTag, transform, owned);
    }

}; // PSolidKernelEntity

static const double TOLERANCE_ChoordAngle       = 0.4;
static const double MESH_TOLERANCE_DIVISOR      = 2.5E2;    // Range diagonal divisor
static const double HATCH_TOLERANCE_DIVISOR     = 2.5E4;
static const double MIN_MESH_TOLERANCE          = 1e-5;
static const double MIN_STROKE_TOLERANCE        = 0.25;
static const double MAX_STROKE_TOLERANCE        = 1.0;
static const double DEF_STROKE_TOLERANCE        = 0.5;

static const int    MAX_FacetsPerStrip          = 4096;     // Max facets per strip returned by ParaSolid facetter.
static const int    QV_HIDEFIN                  = -3;       // this finFin value inhibits marking of open edge

static double       s_sizeToToleranceRatio      = .5;
static double       s_smallFeaturePixels        = 4.0;
static double       s_minRangeRelTol            = 1.0e-5;
static double       s_maxRangeIgnoreRelTol      = .3;       // Don't ignore entire body (causes facetter failure).

static double       s_maxFacetAngleTol          = 1.00;     // radians
static double       s_minFacetAngleTol          = 0.10;     // radians
static double       s_defaultFacetAngleTol      = 0.39;     // radians
static double       s_facetAngleCurveFactor     = 0.5;
static double       s_maxToleranceRatio         = 50000.0;

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  04/09
+===============+===============+===============+===============+===============+======*/
struct PSolidFacetTopologyTable : public RefCounted <IFacetTopologyTable>
{
private:

PK_TOPOL_facet_2_r_t    m_table;
T_FaceAttachmentsVec    m_faceAttachmentsVec;
T_FaceToSubElemIdMap    m_faceToSubElemIdMap;
bset<int32_t>           m_hiddenFaces;
bset<int32_t>           m_hiddenEdges;

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/09
+---------------+---------------+---------------+---------------+---------------+------*/
PSolidFacetTopologyTable::PSolidFacetTopologyTable (ISolidKernelEntityCR in, double pixelSize)
    {
    FacetEntity (in, pixelSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
PSolidFacetTopologyTable::PSolidFacetTopologyTable (ISolidKernelEntityCR in, IFacetOptionsR options)
    {
    FacetEntity (in, options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/09
+---------------+---------------+---------------+---------------+---------------+------*/
PSolidFacetTopologyTable::~PSolidFacetTopologyTable ()
    {
    PK_TOPOL_facet_2_r_f (&m_table);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual T_FaceAttachmentsVec const* PSolidFacetTopologyTable::_GetFaceAttachmentsVec() override {return (m_faceAttachmentsVec.empty() ? NULL : &m_faceAttachmentsVec);};
virtual T_FaceToSubElemIdMap const* PSolidFacetTopologyTable::_GetFaceToSubElemIdMap() override {return (m_faceToSubElemIdMap.empty() ? NULL : &m_faceToSubElemIdMap);};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool    PSolidFacetTopologyTable::_IsTableValid () override {return m_table.number_of_facets > 0;}
virtual int     PSolidFacetTopologyTable::_GetFacetCount () override {return m_table.number_of_facets;}
virtual int     PSolidFacetTopologyTable::_GetFinCount () override {return m_table.number_of_fins;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/09
+---------------+---------------+---------------+---------------+---------------+------*/
int             PSolidFacetTopologyTable::FindTableIndex (PK_TOPOL_fctab_t tableId)
    {
    for (int iTable=0; iTable < m_table.number_of_tables; iTable++)
        if (tableId == m_table.tables[iTable].fctab)
            return iTable;

    return -1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual DPoint3dCP  PSolidFacetTopologyTable::_GetPoint () override
    {
    int         iTable;
    
    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_point_vec_c)))
        return NULL;

    return (DPoint3dCP) m_table.tables[iTable].table.point_vec->vec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual int     PSolidFacetTopologyTable::_GetPointCount () override
    {
    int         iTable;
    
    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_point_vec_c)))
        return 0;

    return m_table.tables[iTable].table.point_vec->length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual int const*  PSolidFacetTopologyTable::_GetPointIndex () override
    {
    int         iTable;
    
    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_data_point_c)))
        return NULL;

    return m_table.tables[iTable].table.data_point_idx->point;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual int     PSolidFacetTopologyTable::_GetPointIndexCount () override
    {
    int         iTable;
    
    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_data_point_c)))
        return 0;

    return m_table.tables[iTable].table.data_point_idx->length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual DVec3dCP    PSolidFacetTopologyTable::_GetNormal () override
    {
    int         iTable;
    
    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_normal_vec_c)))
        return NULL;

    return (DVec3dP) m_table.tables[iTable].table.normal_vec->vec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual int     PSolidFacetTopologyTable::_GetNormalCount () override
    {
    int         iTable;
    
    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_normal_vec_c)))
        return 0;

    return m_table.tables[iTable].table.normal_vec->length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual int const*  PSolidFacetTopologyTable::_GetNormalIndex () override
    {
    int         iTable;
    
    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_data_normal_c)))
        return NULL;

    return m_table.tables[iTable].table.data_normal_idx->normal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual int     PSolidFacetTopologyTable::_GetNormalIndexCount () override
    {
    int         iTable;
    
    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_data_normal_c)))
        return 0;

    return m_table.tables[iTable].table.data_normal_idx->length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual DPoint2dCP  PSolidFacetTopologyTable::_GetParamUV () override
    {
    int         iTable;
    
    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_param_uv_c)))
        return NULL;

    return (DPoint2dP) m_table.tables[iTable].table.param_uv->data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual int     PSolidFacetTopologyTable::_GetParamUVCount () override
    {
    int         iTable;
    
    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_param_uv_c)))
        return 0;

    return m_table.tables[iTable].table.param_uv->length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual int const*  PSolidFacetTopologyTable::_GetParamUVIndex () override
    {
    int         iTable;
    
    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_data_param_c)))
        return NULL;

    return m_table.tables[iTable].table.data_param_idx->param;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual int     PSolidFacetTopologyTable::_GetParamUVIndexCount () override
    {
    int         iTable;
    
    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_data_param_c)))
        return 0;

    return m_table.tables[iTable].table.data_param_idx->length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual int const*  PSolidFacetTopologyTable::_GetFinData () override // fin vertex...
    {
    int         iTable;
    
    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_fin_data_c)))
        return NULL;

    return m_table.tables[iTable].table.fin_data->data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual int     PSolidFacetTopologyTable::_GetFinDataCount () override
    {
    int         iTable;
    
    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_fin_data_c)))
        return 0;

    return m_table.tables[iTable].table.fin_data->length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual int const*  PSolidFacetTopologyTable::_GetFinFin () override
    {
    int         iTable;
    
    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_fin_fin_c)))
        return NULL;

    return m_table.tables[iTable].table.fin_fin->fin;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual int     PSolidFacetTopologyTable::_GetFinFinCount () override
    {
    int         iTable;
    
    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_fin_fin_c)))
        return 0;

    return m_table.tables[iTable].table.fin_fin->length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual Point2dCP   PSolidFacetTopologyTable::_GetFacetFin () override
    {
    int         iTable;
    
    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_facet_fin_c)))
        return NULL;

    return (Point2dCP) m_table.tables[iTable].table.facet_fin->data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual int     PSolidFacetTopologyTable::_GetFacetFinCount () override
    {
    int         iTable;
    
    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_facet_fin_c)))
        return 0;

    return m_table.tables[iTable].table.facet_fin->length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual Point2dCP   PSolidFacetTopologyTable::_GetStripFin () override
    {
    int         iTable;
    
    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_strip_boundary_c)))
        return NULL;

    return (Point2dCP) m_table.tables[iTable].table.strip_boundary->data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual int     PSolidFacetTopologyTable::_GetStripFinCount () override
    {
    int         iTable;
    
    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_strip_boundary_c)))
        return 0;

    return m_table.tables[iTable].table.strip_boundary->length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual int const*  PSolidFacetTopologyTable::_GetStripFaceId () override
    {
    int         iTable;
    
    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_strip_face_c)))
        return NULL;

    return m_table.tables[iTable].table.strip_face->face;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual int     PSolidFacetTopologyTable::_GetStripFaceIdCount () override
    {
    int         iTable;
    
    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_strip_face_c)))
        return 0 ;

    return m_table.tables[iTable].table.strip_face->length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual Point2dCP   PSolidFacetTopologyTable::_GetFinEdge () override
    {
    int         iTable;
    
    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_fin_edge_c)))
        return NULL;

    return (Point2dCP) m_table.tables[iTable].table.fin_edge->data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual int     PSolidFacetTopologyTable::_GetFinEdgeCount () override
    {
    int         iTable;
    
    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_fin_edge_c)))
        return 0;

    return m_table.tables[iTable].table.fin_edge->length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual int const*  PSolidFacetTopologyTable::_GetFacetFace () override
    {
    int         iTable;
    
    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_facet_face_c)))
        return NULL;

    return m_table.tables[iTable].table.facet_face->face;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual int     PSolidFacetTopologyTable::_GetFacetFaceCount () override
    {
    int         iTable;
    
    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_facet_face_c)))
        return 0;

    return m_table.tables[iTable].table.facet_face->length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2012
* 
*     Note:  This method  assumes that the body still exists.   This is true
*            for the instances where they are currently used, but if that changes
*            the curve Ids and hidden states will have to actually be stored in the
*            topology table.
*
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool    PSolidFacetTopologyTable::_GetEdgeCurveId (CurveTopologyId& curveTopologyId, int32_t edge, bool useHighestId) override
    {
    return false;//SUCCESS == PSolidUtil::CurveTopologyIdFromEdge (curveTopologyId, (PK_EDGE_t) edge, useHighestId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool    PSolidFacetTopologyTable::_IsHiddenEdge (int32_t entity) override  { return m_hiddenEdges.find(entity) != m_hiddenEdges.end(); }
virtual bool    PSolidFacetTopologyTable::_IsHiddenFace (int32_t entity) override  { return m_hiddenFaces.find(entity) != m_hiddenFaces.end(); }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            PSolidFacetTopologyTable::GetFacetTolerances (double *choordTolP, double *angleTolP, DPoint3dCP lowP, DPoint3dCP highP, double pixelSize)
    {
    if (0.0 == pixelSize)
        {
        double  strokeTolerance = DEF_STROKE_TOLERANCE; // tcb->strokeTolerance;

        LIMIT_RANGE (MIN_STROKE_TOLERANCE, MAX_STROKE_TOLERANCE, strokeTolerance);

        *angleTolP  = (strokeTolerance * TOLERANCE_ChoordAngle);
        *choordTolP = 0.0;

        if (lowP && highP)
            *choordTolP = (strokeTolerance * lowP->Distance (*highP) / MESH_TOLERANCE_DIVISOR);

        if (*choordTolP <= 0.0)
            *choordTolP = 0.0;
        else
            *choordTolP = (*choordTolP < MIN_MESH_TOLERANCE ? MIN_MESH_TOLERANCE : *choordTolP);
        }
    else
        {
        *choordTolP = s_sizeToToleranceRatio * pixelSize;
        *angleTolP  = msGeomConst_2pi / 7.0;
        }

    // Don't allow a tolerance that is really small compared to the range diagonal.
    // This is redundant of the strokeTol * distance / MESH_TOLERANCE_DIVISOR test...
    // if strokeTol is "relative" that one is ok, if "absolute" that one is confused.
    if (lowP != NULL && highP != NULL)
        {
        double diag = lowP->Distance (*highP);
        double minTol = s_minRangeRelTol * diag;

        if (*choordTolP < minTol)
            *choordTolP = minTol;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     11/04
+---------------+---------------+---------------+---------------+---------------+------*/
double          PSolidFacetTopologyTable::RestrictAngleTol (double radians, double defaultRadians, double minRadians, double maxRadians)
    {
    if (radians <= 0.0)
        radians = defaultRadians;

    else if (radians < minRadians)
        radians = minRadians;

    else if (radians > maxRadians)
        radians = maxRadians;

    return radians;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            PSolidFacetTopologyTable::RemoveHiddenEdges (PK_ENTITY_t entityTag)
    {
    int         iTable;
    
    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_fin_edge_c)))
        return;

    PK_TOPOL_fcstr_fin_edge_t*  finEdgeP = m_table.tables[iTable].table.fin_edge->data;
    int                         numFinEdge = m_table.tables[iTable].table.fin_edge->length;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_fin_fin_c)))
        return;

    int*        finFinIdxP = m_table.tables[iTable].table.fin_fin->fin;
    int         fin, coFin;

    for (int iFinEdge = 0; iFinEdge < numFinEdge; iFinEdge++, finEdgeP++)
        {
        // if this fin's exterior edge is hidden, mark both co-fin entries so that QV knows not to infer an edge
        fin   = finEdgeP->fin;
        coFin = finFinIdxP[fin];

        if (coFin != QV_HIDEFIN && false)//PSolidUtil::IsEntityHidden (finEdgeP->edge))
            {
            finFinIdxP[fin] = QV_HIDEFIN;

            if (coFin >= 0)
                finFinIdxP[coFin] = QV_HIDEFIN;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            PSolidFacetTopologyTable::RemoveHiddenFaces (PK_ENTITY_t entityTag)
    {
    int         iTable;
    
    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_strip_boundary_c)))
        return;

    PK_TOPOL_fcstr_strip_boundary_t*    stripFinP = m_table.tables[iTable].table.strip_boundary->data;
    int                                 numStripFin = m_table.tables[iTable].table.strip_boundary->length;
    int*                                numStripFinP = &m_table.tables[iTable].table.strip_boundary->length;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_strip_face_c)))
        return;

    int*        stripFaceP = m_table.tables[iTable].table.strip_face->face;
    int*        numStripFaceP = &m_table.tables[iTable].table.strip_face->length;

    if (-1 == (iTable = FindTableIndex (PK_TOPOL_fctab_fin_fin_c)))
        return;

    int*        finFinIdxP = m_table.tables[iTable].table.fin_fin->fin;
    int         prevStrip = -1, prevFace = -1, currStrip, currFace, numStripFinNew = 0, numStripFaceNew = 0, fin, coFin;
    bool        stripIsHidden = false;

    for (int iStripFin = 0; iStripFin < numStripFin; iStripFin++)
        {
        currStrip = stripFinP[iStripFin].strip;
        currFace  = stripFaceP[currStrip];

        // find a new strip...
        if (prevStrip != currStrip)
            {
            // ...in a new face...
            if (prevFace != currFace)
                {
                stripIsHidden = false;//PSolidUtil::IsEntityHidden (currFace);
                prevFace = currFace;
                }

            // if strip is visible, reassign its face (overwrite hidden face)
            if (!stripIsHidden)
                {
                stripFaceP[numStripFaceNew] = currFace;
                numStripFaceNew++;
                }

            prevStrip = currStrip;
            }

        if (stripIsHidden)
            {
            // if this hidden fin's co-fin has a co-fin, remove it
            fin   = stripFinP[iStripFin].fin;
            coFin = finFinIdxP[fin];

            if (coFin >= 0)
                {
                finFinIdxP[fin] = -1;

                if (finFinIdxP[coFin] >= 0)
                    finFinIdxP[coFin] = -1;
                }
            }
        else
            {
            // append a new fin to the visible strip (overwrite hidden strip)
            stripFinP[numStripFinNew].strip = numStripFaceNew - 1;
            stripFinP[numStripFinNew].fin   = stripFinP[iStripFin].fin;
            numStripFinNew++;
            }
        }

    // Update counts to reflect removed faces...
    *numStripFinP  = numStripFinNew;
    *numStripFaceP = numStripFaceNew;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidUtil_GetBodyFaces (bvector<PK_FACE_t>& faces, PK_BODY_t body)
    {
    int             faceCount = 0;
    PK_FACE_t*      pFaceTagArray = NULL;

    if (SUCCESS != PK_BODY_ask_faces (body, &faceCount, &pFaceTagArray))
        return ERROR;

    faces.resize (faceCount);
    for (int i=0; i<faceCount; i++)
        faces[i] = pFaceTagArray[i];

    if (pFaceTagArray)
        PK_MEMORY_free (pFaceTagArray);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    PSolidFacetTopologyTable::CompleteTable (PK_ENTITY_t entityTag, IFaceMaterialAttachmentsCP attachments, bool hasHiddenEdge, bool hasHiddenFace)
    {
    // Process hidden edges so they don't display...
    if (hasHiddenEdge)
        {
        RemoveHiddenEdges (entityTag);
//        PSolidUtil::GetHiddenBodyEdges (m_hiddenEdges, entityTag);
        }

    // Process hidden faces so they don't display...
    if (hasHiddenFace)
        {
        RemoveHiddenFaces (entityTag);
//        PSolidUtil::GetHiddenBodyFaces (m_hiddenFaces, entityTag);
        }

    if (nullptr == attachments)
        {
        bvector<PK_FACE_t> faces;

        PSolidUtil_GetBodyFaces (faces, entityTag);

        for (size_t i=0; i<faces.size(); i++)
            m_faceToSubElemIdMap[faces[i]] = make_bpair((int32_t) (i + 1), 0); // 0 is invalid attachment index...
        }
    else
        {
        m_faceAttachmentsVec = attachments->_GetFaceAttachmentsVec ();
        m_faceToSubElemIdMap = attachments->_GetFaceToSubElemIdMap ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            PSolidFacetTopologyTable::FacetEntity (ISolidKernelEntityCR in, double pixelSize)
    {
    PK_TOPOL_facet_2_o_t    options;

    PK_TOPOL_facet_mesh_2_o_m (options.control);

    double      choordTol, angleTol;
    DRange3d    range = DRange3d::NullRange();

    PK_BOX_t    box;
//    PK_ENTITY_t entityTag = PSolidUtil::GetEntityTag (in);
    PK_ENTITY_t entityTag = SolidKernelUtil::GetEntityTag (in);

    if (0.0 != pixelSize)
        {
        DVec3d  xColumn;
    
        in.GetEntityTransform ().GetMatrixColumn (xColumn, 0);
        pixelSize /= xColumn.Magnitude (); // Scale pixelSize to kernel units
        }

    if (PK_ERROR_no_errors == PK_TOPOL_find_box (entityTag, &box))
        {
        range.InitFrom (box.coord[0], box.coord[1], box.coord[2], box.coord[3], box.coord[4], box.coord[5]);
        GetFacetTolerances (&choordTol, &angleTol, &range.low, &range.high, pixelSize);
        }
    else
        {
        GetFacetTolerances (&choordTol, &angleTol, NULL, NULL, pixelSize);
        }

    bool        hasHiddenEdge = false;//PSolidUtil::HasHiddenEdge (entityTag);
    bool        hasHiddenFace = false;//PSolidUtil::HasHiddenFace (entityTag);

    if (0.0 != choordTol)
        {
        options.control.is_curve_chord_tol   = PK_LOGICAL_true;
        options.control.curve_chord_tol      = choordTol;

        options.control.is_surface_plane_tol = PK_LOGICAL_true;
        options.control.surface_plane_tol    = choordTol;
        }

    if (0.0 != angleTol)
        {
        options.control.is_surface_plane_ang = PK_LOGICAL_true;
        options.control.surface_plane_ang    = angleTol;

        options.control.is_curve_chord_ang   = PK_LOGICAL_true;
        options.control.curve_chord_ang      = angleTol;
        }

    if (0.0 != pixelSize && !range.IsNull())
        {
        double      maximumIgnore = s_maxRangeIgnoreRelTol * range.low.Distance (range.high);

        options.control.ignore = PK_facet_ignore_absolute_c;
        options.control.ignore_value = s_smallFeaturePixels * pixelSize;

        if (options.control.ignore_value > maximumIgnore)
            options.control.ignore_value = maximumIgnore;
        }

    options.control.max_facet_sides = 3;
    options.control.match = PK_facet_match_topol_c;

    PK_TOPOL_facet_choice_2_o_m (options.choice);

    if (hasHiddenEdge)
        options.choice.fin_edge = PK_LOGICAL_true;

    options.choice.strip_face           = PK_LOGICAL_true;
    options.choice.strip_boundary       = PK_LOGICAL_true;
    options.choice.fin_fin              = PK_LOGICAL_true;
    options.choice.fin_data             = PK_LOGICAL_true;
    options.choice.point_vec            = PK_LOGICAL_true;
    options.choice.normal_vec           = PK_LOGICAL_true;
    options.choice.data_point_idx       = PK_LOGICAL_true;
    options.choice.data_normal_idx      = PK_LOGICAL_true;
    options.choice.data_param_idx       = PK_LOGICAL_true;
    options.choice.param_uv             = PK_LOGICAL_true;
    options.choice.consistent_parms     = PK_facet_consistent_parms_yes_c;
    options.choice.max_facets_per_strip = MAX_FacetsPerStrip;
    options.choice.split_strips         = PK_facet_split_strip_yes_c; // TR# 269038.

    memset (&m_table, 0, sizeof (m_table)); // Not initialized if error?!?

    if (SUCCESS != PK_TOPOL_facet_2 (1, &entityTag, NULL, &options, &m_table) || !_IsTableValid ())
        return;

    IFaceMaterialAttachmentsCP attachments = in.GetFaceMaterialAttachments();

    CompleteTable (entityTag, attachments, hasHiddenEdge, hasHiddenFace);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            PSolidFacetTopologyTable::FacetEntity (ISolidKernelEntityCR in, IFacetOptionsR facetOptions)
    {
    PK_TOPOL_facet_2_o_t    options;

    PK_TOPOL_facet_mesh_2_o_m (options.control);

    Transform   solidToOutputTransform = in.GetEntityTransform (), outputToSolidTransform;
    DPoint3d    tolerancePoint;

    tolerancePoint.Init (facetOptions.GetChordTolerance (), 0.0, 0.0);
    outputToSolidTransform.InverseOf (solidToOutputTransform);
    outputToSolidTransform.MultiplyMatrixOnly (tolerancePoint);

    double      solidTolerance = tolerancePoint.Magnitude ();
    double      angleTol = RestrictAngleTol (facetOptions.GetAngleTolerance (), s_defaultFacetAngleTol, s_minFacetAngleTol, s_maxFacetAngleTol);
    DRange3d    range;
    PK_BOX_t    box;
//    PK_ENTITY_t entityTag = PSolidUtil::GetEntityTag (in);
    PK_ENTITY_t entityTag = SolidKernelUtil::GetEntityTag(in);

    if (PK_ERROR_no_errors == PK_TOPOL_find_box (entityTag, &box))
        {
        range.InitFrom (box.coord[0], box.coord[1], box.coord[2], box.coord[3], box.coord[4], box.coord[5]);
    
        double  rangeSize = range.low.Distance (range.high);

        if (0.0 != solidTolerance && rangeSize / solidTolerance > s_maxToleranceRatio)
            solidTolerance = rangeSize / s_maxToleranceRatio;

        options.control.curve_chord_tol = options.control.surface_plane_tol = solidTolerance;
        }

    if (0.0 != solidTolerance)
        {
        options.control.is_curve_chord_tol   = PK_LOGICAL_true;
        options.control.is_surface_plane_tol = PK_LOGICAL_true;
        }

    if (0.0 != facetOptions.GetMaxFacetWidth ())
        {
        DPoint3d maxWidthPoint;
        maxWidthPoint.Init (facetOptions.GetMaxFacetWidth (), 0.0, 0.0);
        outputToSolidTransform.MultiplyMatrixOnly (maxWidthPoint);
        options.control.is_max_facet_width  = PK_LOGICAL_true;
        options.control.max_facet_width     = maxWidthPoint.Magnitude ();
        }

    options.control.is_curve_chord_ang   = PK_LOGICAL_true;
    options.control.curve_chord_ang      = angleTol * s_facetAngleCurveFactor;

    options.control.is_surface_plane_ang = PK_LOGICAL_true;
    options.control.surface_plane_ang    = angleTol;

    options.control.match                = PK_facet_match_topol_c;
//    options.control.max_facet_sides      = PSolidUtil::HasCurvedFacesOrEdges (entityTag) ? facetOptions.GetCurvedSurfaceMaxPerFace () : facetOptions.GetMaxPerFace ();
    options.control.max_facet_sides      = facetOptions.GetCurvedSurfaceMaxPerFace ();
    options.control.shape                = facetOptions.GetConvexFacetsRequired () ? PK_facet_shape_convex_c : PK_facet_shape_cut_c;

    PK_TOPOL_facet_choice_2_o_m (options.choice);

    options.choice.data_point_idx   = PK_LOGICAL_true;
    options.choice.data_normal_idx  = facetOptions.GetNormalsRequired ();
    options.choice.data_param_idx   = facetOptions.GetParamsRequired ();
    options.choice.point_vec        = PK_LOGICAL_true;
    options.choice.normal_vec       = facetOptions.GetNormalsRequired ();
    options.choice.param_uv         = facetOptions.GetParamsRequired ();
    options.choice.facet_face       = PK_LOGICAL_true;
    options.choice.fin_data         = PK_LOGICAL_true;
    options.choice.facet_fin        = PK_LOGICAL_true;
    options.choice.fin_edge         = PK_LOGICAL_true;

    if (facetOptions.GetParamsRequired ())
        {
        options.choice.consistent_parms = PK_facet_consistent_parms_yes_c;
        options.choice.split_strips     = PK_facet_split_strip_yes_c; // TR# 269038
        options.choice.strip_boundary   = PK_LOGICAL_true;            // TR# 276122
        }

    memset (&m_table, 0, sizeof (m_table)); // Not initialized if error?!?

    if (SUCCESS != PK_TOPOL_facet_2 (1, &entityTag, NULL, &options, &m_table) || !_IsTableValid ())
        return;

    IFaceMaterialAttachmentsCP attachments = in.GetFaceMaterialAttachments();

//    CompleteTable (entityTag, attachments, PSolidUtil::HasHiddenEdge (entityTag), PSolidUtil::HasHiddenFace (entityTag));
    CompleteTable (entityTag, attachments, false, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
static PSolidFacetTopologyTable* CreateNewFacetTable (ISolidKernelEntityCR in, double pixelSize)
    {
    return new PSolidFacetTopologyTable (in, pixelSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
static PSolidFacetTopologyTable* CreateNewFacetTable (ISolidKernelEntityCR in, IFacetOptionsR options)
    {
    return new PSolidFacetTopologyTable (in, options);
    }

}; // PSolidFacetTopologyTable;

#define COMPARE_INT_VALUES(val0, val1) if (val0 < val1) return true; if (val0 > val1) return false;

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      05/2007
+===============+===============+===============+===============+===============+======*/
struct EdgeIndices 
{
    int32_t    m_index0;
    int32_t    m_index1;

    EdgeIndices () { }
    EdgeIndices (int32_t index0, int32_t index1) : m_index0 (index0), m_index1 (index1)  { }

    bool operator < (EdgeIndices const& rhs) const { return m_index0 == rhs.m_index0 ? m_index1 < rhs.m_index1 : m_index0 < rhs.m_index0; }
};


struct CompareParams { bool operator() (DPoint2dCR param1, DPoint2dCR param2) { return param1.x == param2.x ? (param1.y < param2.y) : (param1.x < param2.x); } };

typedef     bmap <DPoint2d, int32_t, CompareParams>         T_ParamIndexMap;        // Untoleranced parameter to index map used to reindex parameters within a single face.
typedef     bmap <int32_t, int32_t>                         T_IndexRemap;
typedef     bmap <int32_t, int32_t>                         T_FinToEdgeMap;
typedef     std::multimap <CurveTopologyId, EdgeIndices>    T_EdgeIdToIndicesMap;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void    initPolyface (PolyfaceHeaderR polyface, IFacetTopologyTable& ftt)
    {
    polyface.ClearTags (0, MESH_ELM_STYLE_INDEXED_FACE_LOOPS);
    polyface.ClearAllVectors ();

    polyface.PointIndex  ().SetStructsPerRow (1);
    polyface.NormalIndex ().SetStructsPerRow (1);
    polyface.ParamIndex  ().SetStructsPerRow (1);

    polyface.PointIndex  ().SetActive (true);
    polyface.NormalIndex ().SetActive (false);
    polyface.ParamIndex  ().SetActive (false);
    polyface.Point().SetActive (true);
    polyface.Normal ().SetActive (false);

    if (ftt._GetNormalCount () > 0)
        {
        polyface.Normal ().SetActive (true);
        polyface.NormalIndex ().SetActive (true);
        }

    if (ftt._GetParamUVCount () > 0)
        {
        polyface.Param ().SetActive (true);
        polyface.ParamIndex ().SetActive (true);
        polyface.FaceIndex().SetActive (true);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void     initFinToEdgeMap (T_FinToEdgeMap& finToEdgeMap, IFacetTopologyTable& ftt, bool doEdgeHiding)
    {
    Point2dCP                       finEdge  = ftt._GetFinEdge();

    for (int i=0, count = ftt._GetFinEdgeCount(); i<count; i++)
        if (!doEdgeHiding || !ftt._IsHiddenEdge (finEdge[i].y))
            finToEdgeMap.insert (bpair <int32_t, int32_t> (finEdge[i].x, finEdge[i].y));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void     addEdgeChains (BlockedVector<PolyfaceEdgeChain>& edgeChains, T_EdgeIdToIndicesMap& edgeIdToIndicesMap)
    {
    for (T_EdgeIdToIndicesMap::iterator curr = edgeIdToIndicesMap.begin(); curr != edgeIdToIndicesMap.end(); curr++)
        edgeChains.push_back (PolyfaceEdgeChain (curr->first, curr->second.m_index0, curr->second.m_index1));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IFacetTopologyTable::ConvertToPolyface (PolyfaceHeaderR polyface, IFacetTopologyTable& ftt, IFacetOptionsCR facetOptions)
    {
    bool                        edgeChainsRequired = facetOptions.GetEdgeChainsRequired();
    T_FinToEdgeMap              finToEdgeMap;
    bset<int32_t>               edgeSet;
    T_EdgeIdToIndicesMap        edgeIdToIndicesMap;
        
    initPolyface (polyface, ftt);
    initFinToEdgeMap (finToEdgeMap, ftt, facetOptions.GetEdgeHiding());

    DPoint3dOps::Copy (&polyface.Point (), ftt._GetPoint (), (size_t)ftt._GetPointCount ());

    if (ftt._GetNormalCount () > 0)
        DVec3dOps::Copy   (&polyface.Normal (), ftt._GetNormal (), (size_t)ftt._GetNormalCount ());

    if (ftt._GetParamUVCount () > 0)
        DPoint2dOps::Copy  (&polyface.Param (), ftt._GetParamUV(), (size_t)ftt._GetParamUVCount ());

    BlockedVectorIntR polyfacePointIndex  = polyface.PointIndex ();
    BlockedVectorIntR polyfaceNormalIndex = polyface.NormalIndex ();
    BlockedVectorIntR polyfaceParamIndex  = polyface.ParamIndex ();

    // The parasolid terminology is:
    // FIN is an end of an edge, on a a paricular side (like coedge, half edge, or vertex use in various other modelers)
    // Point, ParamUV, and Normal are NUMERIC coordinates
    // VERTEX is a collection of indices into the numeric tables (point, normal, paramUV).
    // several fins can share the same vertex data -- i.e. all the fins that leave a POINT within the same face.

    int         numFacetFin          = ftt._GetFacetFinCount ();
    int const*  ftt_finToVertex      = ftt._GetFinData ();
    int const*  ftt_vertexToPoint    = ftt._GetPointIndex ();
    int const*  ftt_vertexToNormal   = ftt._GetNormalIndex ();
    int const*  ftt_vertexToParam    = ftt._GetParamUVIndex ();
    int const*  ftt_facetToFace      = ftt._GetFacetFace ();                                                                                                                                    
    Point2dCP   ftt_facetFin         = ftt._GetFacetFin ();

    int     thisFace, currentFace  = -1;

    for (int facetIndex = 0, ffIndex0 = 0, ffIndex1; ffIndex0 < numFacetFin; facetIndex++, ffIndex0 = ffIndex1)
        {
        ffIndex1 = ffIndex0 + 1;
        while (ffIndex1 < numFacetFin && ftt_facetFin[ffIndex1].x == ftt_facetFin[ffIndex0].x)
            ffIndex1++;

        if (0 == (thisFace = ftt_facetToFace[facetIndex]) ||
            (facetOptions.GetEdgeHiding() && ftt._IsHiddenFace (thisFace)))
            continue;
        
        if (thisFace != currentFace)
            {
            polyface.SetNewFaceData (NULL);
            currentFace = thisFace;
            }

        for (int ffIndex = ffIndex0; ffIndex < ffIndex1; )
            {
            int32_t   vertexIndex = ftt_finToVertex[ffIndex];
            int32_t   xyzIndex    = ftt_vertexToPoint[vertexIndex];
            int32_t   nextFinIndex;

            if ((nextFinIndex = ++ffIndex) == ffIndex1)
                nextFinIndex = ffIndex0;

            T_FinToEdgeMap::iterator    found = finToEdgeMap.find (nextFinIndex);

            if (found == finToEdgeMap.end())
                {
                polyfacePointIndex.push_back (-xyzIndex - 1);
                }
            else
                {
                polyfacePointIndex.push_back (xyzIndex + 1);
                if (edgeChainsRequired)
                    {
                    CurveTopologyId      curveTopologyId;

                     if (edgeSet.find (found->second) == edgeSet.end() &&
                        ftt._GetEdgeCurveId (curveTopologyId, found->second, true))
                        {
                        edgeSet.insert (found->second);
                        edgeIdToIndicesMap.insert (std::pair <CurveTopologyId, EdgeIndices> (curveTopologyId, EdgeIndices (1 + xyzIndex, 1 + ftt_vertexToPoint[ftt_finToVertex[nextFinIndex]])));
                        }
                    }
                }

            if (NULL != ftt_vertexToNormal)
                polyfaceNormalIndex.push_back (ftt_vertexToNormal[vertexIndex] + 1);

            if (NULL != ftt_vertexToParam)
                polyfaceParamIndex.push_back (ftt_vertexToParam[vertexIndex] + 1);
            }
        polyface.TerminateAllActiveIndexVectors ();
        }

    polyface.SetNewFaceData (NULL);
    if (edgeChainsRequired)           // Edge chains requested...
        addEdgeChains (polyface.EdgeChain(), edgeIdToIndicesMap);

        
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t remapPointIndex (int32_t pointIndex, T_IndexRemap& pointIndexMap, BlockedVectorDPoint3dR polyfacePoints, DPoint3dCP fttPoints)
    {
    T_IndexRemap::iterator  found = pointIndexMap.find (pointIndex);
                
    if (found == pointIndexMap.end())
        {
        int32_t pointIndexRemapped = (int32_t) polyfacePoints.size();

        polyfacePoints.push_back (fttPoints [pointIndex]);
        pointIndexMap[pointIndex] = pointIndexRemapped;
        return  pointIndexRemapped;
        }
    else
        {
        return found->second;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt convertFaceFacetsToPolyface (PolyfaceHeaderR polyface, bmap<int, PolyfaceHeaderCP>& facePolyfaces, IFacetTopologyTable& ftt, T_FinToEdgeMap finToEdgeMap, IFacetOptionsCR facetOptions)
    {
    bool        edgeChainsRequired   = facetOptions.GetEdgeChainsRequired();
    int         numFacetFin          = ftt._GetFacetFinCount ();
    int const*  ftt_finToVertex      = ftt._GetFinData ();
    int const*  ftt_vertexToPoint    = ftt._GetPointIndex ();
    int const*  ftt_vertexToNormal   = ftt._GetNormalIndex ();
    int const*  ftt_vertexToParam    = ftt._GetParamUVIndex ();
    int const*  ftt_facetToFace      = ftt._GetFacetFace ();
    DPoint2dCP  ftt_paramUV          = ftt._GetParamUV ();
    DPoint3dCP  ftt_points           = ftt._GetPoint ();
    DVec3dCP    ftt_normals          = ftt._GetNormal(); 
    Point2dCP   ftt_facetFin         = ftt._GetFacetFin ();

    T_IndexRemap            pointIndexMap, normalIndexMap, paramIndexMap;
    T_EdgeIdToIndicesMap    edgeIdToIndicesMap;
    bset<int32_t>           edgeSet;

    int32_t thisFace, currentFace = -1;

    for (int facetIndex = 0, ffIndex0 = 0, ffIndex1; ffIndex0 < numFacetFin; facetIndex++, ffIndex0 = ffIndex1)
        {
        bmap <int, PolyfaceHeaderCP>::const_iterator foundPolyface;

        ffIndex1 = ffIndex0 + 1;
        while (ffIndex1 < numFacetFin && ftt_facetFin[ffIndex1].x == ftt_facetFin[ffIndex0].x)
            ffIndex1++;

        if (0 == (thisFace = ftt_facetToFace[facetIndex]) ||
            (foundPolyface = facePolyfaces.find (thisFace)) == facePolyfaces.end() ||
            foundPolyface->second != &polyface)
            continue;

        if (thisFace != currentFace)
            {
            polyface.SetNewFaceData (NULL);
            currentFace = thisFace;
            }

        for (int ffIndex = ffIndex0; ffIndex < ffIndex1; )
            {
            int     vertexIndex = ftt_finToVertex[ffIndex];
            int32_t pointIndex = ftt_vertexToPoint[vertexIndex], pointIndexRemapped;

            // POINT INDICES
            pointIndexRemapped = remapPointIndex (pointIndex, pointIndexMap, polyface.Point(), ftt_points);

            int     nextFinIndex;

            if ((nextFinIndex = ++ffIndex) == ffIndex1)
                nextFinIndex = ffIndex0;

            int32_t nextPointIndex = ftt_vertexToPoint[ftt_finToVertex[nextFinIndex]], nextPointIndexRemapped;

            nextPointIndexRemapped = remapPointIndex (nextPointIndex, pointIndexMap, polyface.Point(), ftt_points);

            T_FinToEdgeMap::iterator foundEdge = finToEdgeMap.find (nextFinIndex);

            if (foundEdge == finToEdgeMap.end())
                {
                polyface.PointIndex().push_back (-pointIndexRemapped - 1);
                }
            else
                {
                polyface.PointIndex().push_back (pointIndexRemapped + 1);

                if (edgeChainsRequired)
                    {
                    CurveTopologyId curveTopologyId;

                    if (edgeSet.find (foundEdge->second) == edgeSet.end() && ftt._GetEdgeCurveId (curveTopologyId, foundEdge->second, true))
                        {
                        edgeSet.insert (foundEdge->second);
                        edgeIdToIndicesMap.insert (std::pair <CurveTopologyId, EdgeIndices> (curveTopologyId, EdgeIndices (1 + pointIndexRemapped, 1 + nextPointIndexRemapped)));
                        }
                    }
                }

            // NORMAL INDICES
            if (NULL != ftt_vertexToNormal)
                {
                int32_t                 normalIndex = ftt_vertexToNormal[vertexIndex], normalIndexRemapped;
                T_IndexRemap::iterator  found = normalIndexMap.find (normalIndex);

                if (found == normalIndexMap.end())
                    {
                    normalIndexRemapped = (int32_t) polyface.Normal().size();
                    polyface.Normal().push_back (ftt_normals[normalIndex]);
                    normalIndexMap[normalIndex] = normalIndexRemapped;
                    }
                else
                    {
                    normalIndexRemapped = found->second;
                    }

                polyface.NormalIndex().push_back (normalIndexRemapped + 1);
                }

            if (NULL != ftt_vertexToParam)
                {
                int32_t                 paramIndex = ftt_vertexToParam[vertexIndex], paramIndexRemapped;
                T_IndexRemap::iterator  found = paramIndexMap.find (paramIndex);

                if (found == paramIndexMap.end())
                    {
                    paramIndexRemapped = (int32_t) polyface.Param().size();
                    polyface.Param().push_back (ftt_paramUV[paramIndex]);
                    paramIndexMap[paramIndex] = paramIndexRemapped;
                    }
                else
                    {
                    paramIndexRemapped = found->second;
                    }

                polyface.ParamIndex().push_back (paramIndexRemapped + 1);
                }
            }

        polyface.TerminateAllActiveIndexVectors ();
        }

    polyface.SetNewFaceData (NULL);

    if (edgeChainsRequired)
        addEdgeChains (polyface.EdgeChain(), edgeIdToIndicesMap);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IFacetTopologyTable::ConvertToPolyfaces
(
bvector<PolyfaceHeaderPtr>&     polyfaces,
bmap<int, PolyfaceHeaderCP>&    facePolyfaces,
IFacetTopologyTable&            ftt,
IFacetOptionsCR                 facetOptions
)
    {
    T_FinToEdgeMap  finToEdgeMap;
    bset<uint32_t>  idsWithSymbology;
    StatusInt       status;

    initFinToEdgeMap(finToEdgeMap, ftt, facetOptions.GetEdgeHiding());

    for (PolyfaceHeaderPtr& polyface: polyfaces)
        {
        initPolyface(*polyface, ftt);

        if (SUCCESS != (status = convertFaceFacetsToPolyface(*polyface, facePolyfaces, ftt, finToEdgeMap, facetOptions)))
            return status;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/98
+---------------+---------------+---------------+---------------+---------------+------*/
static int      pki_get_curve_of_edge
(
int             *curveTagOutP,
double          *startParamP,
double          *endParamP,
bool            *reversedP,
int             edgeTagIn
)
    {
    PK_FIN_t        fin;
    PK_INTERVAL_t   interval;
    PK_LOGICAL_t    sense = PK_LOGICAL_true;

    *curveTagOutP = NULTAG;

    if (SUCCESS == PK_EDGE_ask_oriented_curve (edgeTagIn, curveTagOutP, &sense) && *curveTagOutP != NULTAG)
        {
        if (startParamP || endParamP)
            {
            if (SUCCESS != PK_EDGE_find_interval (edgeTagIn, &interval))
                PK_CURVE_ask_interval (*curveTagOutP, &interval);
            }
        }
    else if (SUCCESS == PK_EDGE_ask_first_fin (edgeTagIn, &fin))
        {
        PK_FIN_ask_oriented_curve (fin, curveTagOutP, &sense);

        if ((startParamP || endParamP) && *curveTagOutP != NULTAG)
            PK_FIN_find_interval (fin, &interval);
        }

    if (startParamP)
        *startParamP = interval.value[sense == PK_LOGICAL_true ? 0 : 1];

    if (endParamP)
        *endParamP = interval.value[sense == PK_LOGICAL_true ? 1 : 0];

    if (reversedP)
        *reversedP = (sense == PK_LOGICAL_true ? false : true);

    return (*curveTagOutP != NULTAG ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool PSolidUtil_HasCurvedFacesOrEdges (PK_BODY_t entity)
    {
    if (!entity)
        return false;

    int         numFaces = 0;
    PK_FACE_t*  faces = NULL;

    PK_BODY_ask_faces (entity, &numFaces, &faces);

    for (int i=0; i < numFaces; i++)
        {
        PK_SURF_t       surfaceTag;
        PK_LOGICAL_t    orientation;
        bool            isPlanar = false;

        if (SUCCESS == PK_FACE_ask_oriented_surf (faces[i], &surfaceTag, &orientation))
            {
            PK_CLASS_t  entityClass;

            PK_ENTITY_ask_class (surfaceTag, &entityClass);

            switch (entityClass)
                {
                case PK_CLASS_plane:
                case PK_CLASS_circle:
                case PK_CLASS_ellipse:
                    {
                    isPlanar = true;
                    break;
                    }
                }
            }

        if (!isPlanar)
            {
            PK_MEMORY_free (faces);

            return true;
            }
        }

    if (faces)
        PK_MEMORY_free (faces);

    int         numEdges = 0;
    PK_EDGE_t*  edges = NULL;

    PK_BODY_ask_edges (entity, &numEdges, &edges);

    for (int i=0; i < numEdges; i++)
        {
        PK_CURVE_t      curveTag;
        bool            isStraight = false;

        if (SUCCESS == pki_get_curve_of_edge (&curveTag, NULL, NULL, NULL, edges[i]))
            {
            PK_CLASS_t  entityClass;

            PK_ENTITY_ask_class (curveTag, &entityClass);
            isStraight = (PK_CLASS_line == entityClass);
            }

        if (!isStraight)
            {
            PK_MEMORY_free (edges);

            return true;
            }
        }

    if (edges)
        PK_MEMORY_free (edges);

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool SolidKernelUtil::HasCurvedFaceOrEdge(ISolidKernelEntityCR entity)
    {
    return PSolidUtil_HasCurvedFacesOrEdges(GetEntityTag(entity));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t SolidKernelUtil::GetEntityTag(ISolidKernelEntityCR entity)
    {
    PSolidKernelEntity const* psEntity = dynamic_cast <PSolidKernelEntity const*> (&entity);

    return (nullptr != psEntity ? psEntity->GetEntityTag() : 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ISolidKernelEntityPtr SolidKernelUtil::CreateNewEntity(uint32_t entityTag, TransformCR entityTransform, bool owned)
    {
    return PSolidKernelEntity::CreateNewEntity(entityTag, entityTransform, owned);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr SolidKernelUtil::FacetEntity(ISolidKernelEntityCR entity, double pixelSize, DRange1dP pixelSizeRange)
    {
    if (nullptr != pixelSizeRange)
        pixelSizeRange->InitNull();

    IFacetOptionsPtr facetOptions = IFacetOptions::Create();

    facetOptions->SetNormalsRequired(true);
    facetOptions->SetParamsRequired(true);

    if (!HasCurvedFaceOrEdge(entity))
        {
        facetOptions->SetAngleTolerance(Angle::PiOver2()); // Shouldn't matter...use max angle tolerance...
        facetOptions->SetChordTolerance(1.0); // Shouldn't matter...don't bother getting AABB...
        }
    else
        {
        DRange3d range = entity.GetEntityRange();
        double   maxDimension = range.DiagonalDistance();

        if (0.0 >= pixelSize)
            {
            facetOptions->SetAngleTolerance(0.2); // ~11 degrees
            facetOptions->SetChordTolerance(0.1 * maxDimension);
            }
        else
            {
            static double sizeDependentRatio = 5.0;
            static double pixelToChordRatio = 0.5;
            static double minRangeRelTol = 1.0e-4;
            static double maxRangeRelTol = 1.5e-2;
            double minChordTol = minRangeRelTol * maxDimension;
            double maxChordTol = maxRangeRelTol * maxDimension;
            double chordTol = pixelToChordRatio * pixelSize;
            bool isMin = false, isMax = false;

            if (isMin = (chordTol < minChordTol))
                chordTol = minChordTol; // Don't allow chord to get too small relative to BRep size...
            else if (isMax = (chordTol > maxChordTol))
                chordTol = maxChordTol; // Don't keep creating coarser and coarser graphics as you zoom out, at a certain point it just wastes memory/time...

            facetOptions->SetChordTolerance(chordTol);
            facetOptions->SetAngleTolerance(Angle::PiOver2()); // Use max angle tolerance...mesh coarseness dictated by pixel size based chord...

            if (nullptr != pixelSizeRange)
                {
                if (isMin)
                    *pixelSizeRange = DRange1d::FromLowHigh(0.0, chordTol * sizeDependentRatio); // Finest tessellation, keep using this as we zoom in...
                else if (isMax)
                    *pixelSizeRange = DRange1d::FromLowHigh(chordTol / sizeDependentRatio, DBL_MAX); // Coarsest tessellation, keep using this as we zoom out...
                else
                    *pixelSizeRange = DRange1d::FromLowHigh(chordTol / sizeDependentRatio, chordTol * sizeDependentRatio);
                }
            }
        }

    IFacetTopologyTablePtr facetTopo = PSolidFacetTopologyTable::CreateNewFacetTable(entity, pixelSize); // NEEDSWORK: Account for pixelSizeRange and remove code above?

    if (!facetTopo->_IsTableValid())
        return nullptr;

    PolyfaceHeaderPtr mesh = PolyfaceHeader::New();

    if (SUCCESS != IFacetTopologyTable::ConvertToPolyface(*mesh, *facetTopo, *facetOptions))
        return nullptr;

    mesh->SetTwoSided(ISolidKernelEntity::EntityType::Solid != entity.GetEntityType());
    mesh->Transform(entity.GetEntityTransform());

    return mesh;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr SolidKernelUtil::FacetEntity(ISolidKernelEntityCR entity, IFacetOptionsR facetOptions)
    {
    IFacetTopologyTablePtr facetTopo = PSolidFacetTopologyTable::CreateNewFacetTable(entity, facetOptions);

    if (!facetTopo->_IsTableValid())
        return nullptr;

    PolyfaceHeaderPtr mesh = PolyfaceHeader::New();

    if (SUCCESS != IFacetTopologyTable::ConvertToPolyface(*mesh, *facetTopo, facetOptions))
        return nullptr;

    mesh->SetTwoSided(ISolidKernelEntity::EntityType::Solid != entity.GetEntityType());
    mesh->Transform(entity.GetEntityTransform());

    return mesh;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/10
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt pki_combine_memory_blocks (PK_MEMORY_block_t* pBuffer)
    {
    if (NULL == pBuffer->next)
        return SUCCESS;

    // Create block with single contiguous buffer...
    PK_MEMORY_block_t*  thisBufferP = pBuffer;
    PK_MEMORY_block_t   tmpBlock;

    memset (&tmpBlock, 0, sizeof (tmpBlock));

    do
        {
        tmpBlock.n_bytes += thisBufferP->n_bytes;

        } while (thisBufferP = thisBufferP->next);

    if (NULL == (tmpBlock.bytes = (char *) malloc (tmpBlock.n_bytes)))
        {
        PK_MEMORY_block_f (pBuffer);

        return ERROR;
        }

    size_t  dataOffset = 0;

    thisBufferP = pBuffer;

    do
        {
        memcpy ((void *) &tmpBlock.bytes[dataOffset], thisBufferP->bytes, thisBufferP->n_bytes);
        dataOffset += thisBufferP->n_bytes;

        } while (thisBufferP = thisBufferP->next);

    PK_MEMORY_block_f (pBuffer);
    *pBuffer = tmpBlock;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/96
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt pki_save_entity_to_memory
(
PK_MEMORY_block_t*  pBuffer,                // <= output buffer
int                 entityTagIn,            // => input entitytag to be saved
int                 transmitVersion         // => version of parasolid schema to be used
)
    {
    if (!pBuffer)
        return ERROR;

    memset (pBuffer, 0, sizeof (*pBuffer));

    if (PK_ENTITY_null == entityTagIn)
        return ERROR;

    PK_PART_transmit_o_t transmitOptions;

    PK_PART_transmit_o_m (transmitOptions);
    transmitOptions.transmit_format  = PK_transmit_format_binary_c;
    transmitOptions.transmit_version = transmitVersion;

    if (PK_ERROR_no_errors != PK_PART_transmit_b (1, &entityTagIn, &transmitOptions, pBuffer))
        return ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/96
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt pki_restore_entity_from_memory
(
int*                pEntityTagOut,          // <= output tag of restored entity
PK_MEMORY_block_t*  pBuffer                 // => input buffer
)
    {
    *pEntityTagOut = NULTAG;

    if (!pBuffer || 0 == pBuffer->n_bytes)
        return ERROR;

    PK_PART_receive_o_t receiveOptions;

    PK_PART_receive_o_m (receiveOptions);
    receiveOptions.transmit_format = PK_transmit_format_binary_c;

    int         nParts = 0;
    PK_PART_t*  parts = NULL;

    if (PK_ERROR_no_errors != PK_PART_receive_b (*pBuffer, &receiveOptions, &nParts, &parts) || !nParts)
        return ERROR;

    for (int iPart = 0; iPart < nParts; iPart++)
        {
        if (0 == iPart)
            *pEntityTagOut = parts[0];
        else
            PK_ENTITY_delete (1, &parts[iPart]); // Only expect/return 1st entity...assert?
        }

    *pEntityTagOut = parts[0];

    PK_MEMORY_free (parts);

    return SUCCESS;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SolidKernelUtil::SaveEntityToMemory
(
uint8_t**               ppBuffer,
size_t&                 bufferSize,
ISolidKernelEntityCR    entity
) const
    {
    PSolidKernelEntity const* psEntity = dynamic_cast <PSolidKernelEntity const*> (&entity);

    if (!psEntity)
        return ERROR; // Should never happen...

    PK_MEMORY_block_t*  block = psEntity->GetPrivateMemoryBlock();

    if (SUCCESS != pki_save_entity_to_memory(block, psEntity->GetEntityTag (), DataVersion_PSolid) ||
        SUCCESS != pki_combine_memory_blocks(block))
        return ERROR;

    *ppBuffer  = (uint8_t*) block->bytes;
    bufferSize = block->n_bytes;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SolidKernelUtil::RestoreEntityFromMemory
(
ISolidKernelEntityPtr&  entityOut,
uint8_t const*          buffer,
size_t                  bufferSize,
TransformCR             transform
) const
    {
    PSolidKernelManager::StartSession (); // Make sure frustrum is initialized...

    PK_ENTITY_t         entityTag;
    PK_MEMORY_block_t   block;

    memset (&block, 0, sizeof(block));

    block.n_bytes = bufferSize;
    block.bytes   = (const char *) buffer;

    if (SUCCESS != pki_restore_entity_from_memory(&entityTag, &block))
        return ERROR;

    entityOut = PSolidKernelEntity::CreateNewEntity(entityTag, transform);

    return SUCCESS;
    }

#elif defined (BENTLEYCONFIG_OPENCASCADE)    
/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  03/16
+===============+===============+===============+===============+===============+======*/
struct OpenCascadeEntity : RefCounted<ISolidKernelEntity>
{
private:

TopoDS_Shape m_shape;

protected:

virtual Transform _GetEntityTransform () const override {Transform transform = OCBRep::ToTransform(m_shape.Location()); return transform;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _SetEntityTransform (TransformCR transform) override
    {
    DPoint3d    origin;
    RotMatrix   rMatrix, rotation, skewFactor;
    Transform   shapeTrans, goopTrans; 

    transform.GetTranslation(origin);
    transform.GetMatrix(rMatrix);

    // NOTE: Don't allow scaled TopoDS_Shape::Location...too many bugs, also non-uniform scale isn't supported...
    if (rMatrix.RotateAndSkewFactors(rotation, skewFactor, 0, 1))
        {
        goopTrans.InitFrom(skewFactor);
        shapeTrans.InitFrom(rotation, origin);
        }
    else
        {
        goopTrans = transform;
        shapeTrans.InitIdentity();
        }

    try
        {
        if (!goopTrans.IsIdentity())
            {
            double  goopScale;

            goopTrans.GetMatrix(rMatrix);

            if (rMatrix.IsUniformScale(goopScale))
                {
                gp_Trsf goopTrsf = OCBRep::ToGpTrsf(goopTrans);

                m_shape.Location(TopLoc_Location()); // NOTE: Need to ignore shape location...
                BRepBuilderAPI_Transform transformer(m_shape, goopTrsf);
    
                if (!transformer.IsDone())
                    {
                    BeAssert(false);
                    return false;
                    }

                m_shape = transformer.ModifiedShape(m_shape);
                }
            else
                {
                gp_GTrsf goopTrsf = OCBRep::ToGpGTrsf(goopTrans);

                m_shape.Location(TopLoc_Location()); // NOTE: Need to ignore shape location...
                BRepBuilderAPI_GTransform transformer(m_shape, goopTrsf);
    
                if (!transformer.IsDone())
                    {
                    BeAssert(false);
                    return false;
                    }

                m_shape = transformer.ModifiedShape(m_shape);
                }

            BeAssert(m_shape.Location().IsIdentity());
            }
        }
    catch (Standard_Failure)
        {
        return false;
        }

    m_shape.Location(OCBRep::ToGpTrsf(shapeTrans));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/16
+---------------+---------------+---------------+---------------+---------------+------*/
EntityType ToEntityType(TopAbs_ShapeEnum shapeType) const
    {
    switch (shapeType)
        {
        case TopAbs_COMPOUND:
            return ISolidKernelEntity::EntityType::Compound;

        case TopAbs_COMPSOLID:
        case TopAbs_SOLID:
            return ISolidKernelEntity::EntityType::Solid;

        case TopAbs_SHELL:
        case TopAbs_FACE:
            return ISolidKernelEntity::EntityType::Sheet;

        case TopAbs_WIRE:
        case TopAbs_EDGE:
            return ISolidKernelEntity::EntityType::Wire;

        case TopAbs_VERTEX:
        default:
            return ISolidKernelEntity::EntityType::Minimal;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/16
+---------------+---------------+---------------+---------------+---------------+------*/
EntityType _GetEntityType() const {return ToEntityType(OCBRepUtil::GetShapeType(m_shape));}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d _GetEntityRange() const
    {
    Bnd_Box box;

    try
        {
        BRepBndLib::AddOptimal(m_shape, box, false); // Never use triangulation...
        }
    catch (Standard_Failure)
        {
        BRepBndLib::Add(m_shape, box); // Sloppy AABB implementation is apparently more robust...we NEED a valid range!
        }

    return OCBRep::ToDRange3d(box);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsEqual (ISolidKernelEntityCR entity) const override
    {
    if (this == &entity)
        return true;

    OpenCascadeEntity const* ocEntity;

    if (NULL == (ocEntity = dynamic_cast <OpenCascadeEntity const*>(&entity)))
        return false;

    return TO_BOOL(m_shape == ocEntity->GetShape());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual IFaceMaterialAttachmentsCP _GetFaceMaterialAttachments() const override {return nullptr;}
virtual bool _InitFaceMaterialAttachments (Render::GeometryParamsCP baseParams) override {return false;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ISolidKernelEntityPtr _Clone() const override
    {
    TopoDS_Shape clone(m_shape);

    return OpenCascadeEntity::CreateNewEntity(clone);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
OpenCascadeEntity(TopoDS_Shape const& shape) : m_shape(shape) {}

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TopoDS_Shape const& GetShape() const {return m_shape;}
TopoDS_Shape& GetShapeR() {return m_shape;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
static OpenCascadeEntity* CreateNewEntity(TopoDS_Shape const& shape)
    {
    if (OCBRepUtil::IsEmptyCompoundShape(shape))
        return nullptr; // Don't create OpenCascadeEntity from empty compound (ex. useless result from BRepAlgoAPI_Cut if target is completely inside tool)...

    return new OpenCascadeEntity(shape);
    }

}; // OpenCascadeEntity

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TopoDS_Shape const* SolidKernelUtil::GetShape(ISolidKernelEntityCR entity)
    {
    OpenCascadeEntity const* ocEntity = dynamic_cast <OpenCascadeEntity const*> (&entity);

    if (!ocEntity)
        return nullptr;

    return &ocEntity->GetShape();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool SolidKernelUtil::HasCurvedFaceOrEdge(ISolidKernelEntityCR entity)
    {
    TopoDS_Shape const* shape = GetShape(entity);
    BeAssert(nullptr != shape);
    return nullptr != shape ? OCBRep::HasCurvedFaceOrEdge(*shape) : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TopoDS_Shape* SolidKernelUtil::GetShapeP(ISolidKernelEntityR entity)
    {
    OpenCascadeEntity* ocEntity = dynamic_cast <OpenCascadeEntity*> (&entity);

    if (!ocEntity)
        return nullptr;

    return &ocEntity->GetShapeR();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ISolidKernelEntityPtr SolidKernelUtil::CreateNewEntity(TopoDS_Shape const& shape)
    {
    return OpenCascadeEntity::CreateNewEntity(shape);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr SolidKernelUtil::FacetEntity(ISolidKernelEntityCR entity, double pixelSize, DRange1dP pixelSizeRange)
    {
    TopoDS_Shape const* shape = SolidKernelUtil::GetShape(entity);

    if (nullptr == shape)
        return nullptr;

    if (nullptr != pixelSizeRange)
        pixelSizeRange->InitNull();

    IFacetOptionsPtr facetOptions = IFacetOptions::Create();

    facetOptions->SetNormalsRequired(true);
    facetOptions->SetParamsRequired(true);

    if (!OCBRep::HasCurvedFaceOrEdge(*shape))
        {
        facetOptions->SetAngleTolerance(Angle::PiOver2()); // Shouldn't matter...use max angle tolerance...
        facetOptions->SetChordTolerance(1.0); // Shouldn't matter...avoid expense of getting AABB...
        }
    else
        {
        Bnd_Box box;
        Standard_Real maxDimension = 0.0;

        BRepBndLib::Add(*shape, box);
        BRepMesh_ShapeTool::BoxMaxDimension(box, maxDimension);

        if (0.0 >= pixelSize)
            {
            facetOptions->SetAngleTolerance(0.2); // ~11 degrees
            facetOptions->SetChordTolerance(0.1 * maxDimension);
            }
        else
            {
            static double sizeDependentRatio = 5.0;
            static double pixelToChordRatio = 0.5;
            static double minRangeRelTol = 1.0e-4;
            static double maxRangeRelTol = 1.5e-2;
            double minChordTol = minRangeRelTol * maxDimension;
            double maxChordTol = maxRangeRelTol * maxDimension;
            double chordTol = pixelToChordRatio * pixelSize;
            bool isMin = false, isMax = false;

            if (isMin = (chordTol < minChordTol))
                chordTol = minChordTol; // Don't allow chord to get too small relative to shape size...
            else if (isMax = (chordTol > maxChordTol))
                chordTol = maxChordTol; // Don't keep creating coarser and coarser graphics as you zoom out, at a certain point it just wastes memory/time...

            facetOptions->SetChordTolerance(chordTol);
            facetOptions->SetAngleTolerance(Angle::PiOver2()); // Use max angle tolerance...mesh coarseness dictated by pixel size based chord...

            if (nullptr != pixelSizeRange)
                {
                if (isMin)
                    *pixelSizeRange = DRange1d::FromLowHigh(0.0, chordTol * sizeDependentRatio); // Finest tessellation, keep using this as we zoom in...
                else if (isMax)
                    *pixelSizeRange = DRange1d::FromLowHigh(chordTol / sizeDependentRatio, DBL_MAX); // Coarsest tessellation, keep using this as we zoom out...
                else
                    *pixelSizeRange = DRange1d::FromLowHigh(chordTol / sizeDependentRatio, chordTol * sizeDependentRatio);
                }
            }
        }

    return OCBRep::IncrementalMesh(*shape, *facetOptions);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr SolidKernelUtil::FacetEntity(ISolidKernelEntityCR, IFacetOptionsR) {return nullptr;}
#else
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr SolidKernelUtil::FacetEntity(ISolidKernelEntityCR, double pixelSize, DRange1dP pixelSizeRange) {return nullptr;}
PolyfaceHeaderPtr SolidKernelUtil::FacetEntity(ISolidKernelEntityCR, IFacetOptionsR) {return nullptr;}
bool SolidKernelUtil::HasCurvedFaceOrEdge(ISolidKernelEntityCR entity) {return true;}
#endif