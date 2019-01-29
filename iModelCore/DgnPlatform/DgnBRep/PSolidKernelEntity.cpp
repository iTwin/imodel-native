/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnBRep/PSolidKernelEntity.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnBRep/PSolidUtil.h>

enum SolidDataVersion
    {
    DataVersion_PSolid = 120, //!< Transmit schema version used to persist Parasolid data in a dgn file.
    };

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  12/12
+===============+===============+===============+===============+===============+======*/
struct FaceMaterialAttachments : RefCounted <IFaceMaterialAttachments>
{
private:

T_FaceAttachmentsVec    m_faceAttachmentsVec;

public:

FaceMaterialAttachments() {}

T_FaceAttachmentsVec const& _GetFaceAttachmentsVec() const override {return m_faceAttachmentsVec;}
T_FaceAttachmentsVec& _GetFaceAttachmentsVecR() override {return m_faceAttachmentsVec;}

}; // FaceMaterialAttachments

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  04/09
+===============+===============+===============+===============+===============+======*/
struct PSolidKernelEntity : public RefCounted <IBRepEntity>
{
private:

PK_ENTITY_t                 m_entityTag;
Transform                   m_transform;
bool                        m_owned;
mutable PK_MEMORY_block_t   m_block;
IFaceMaterialAttachmentsPtr m_faceAttachments;

protected:

Transform _GetEntityTransform() const override {return m_transform;}
bool _SetEntityTransform(TransformCR transform) override {m_transform = transform; return true;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
EntityType _GetEntityType() const override
    {
    PK_BODY_type_t  bodyType = PK_BODY_type_unspecified_c;

    PK_BODY_ask_type(m_entityTag, &bodyType);

    switch (bodyType)
        {
        case PK_BODY_type_solid_c:
            return IBRepEntity::EntityType::Solid;

        case PK_BODY_type_sheet_c:
            return IBRepEntity::EntityType::Sheet;

        case PK_BODY_type_wire_c:
            return IBRepEntity::EntityType::Wire;

        default:
            return IBRepEntity::EntityType::Invalid;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d _GetEntityRange() const  override
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
bool _IsEqual(IBRepEntityCR entity) const override
    {
    PSolidKernelEntity const* psEntity;

    if (nullptr == (psEntity = dynamic_cast <PSolidKernelEntity const*> (&entity)))
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
IFaceMaterialAttachmentsCP _GetFaceMaterialAttachments() const override
    {
    return m_faceAttachments.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
IBRepEntityPtr _Clone() const override
    {
    PK_ENTITY_t entityTag = PK_ENTITY_null;

    PK_ENTITY_copy(m_entityTag, &entityTag);

    if (PK_ENTITY_null == entityTag)
        return nullptr;

    IBRepEntityPtr instance = PSolidKernelEntity::CreateNewEntity(entityTag, m_transform);

    if (instance.IsValid() && m_faceAttachments.IsValid())
        {
        FaceMaterialAttachments* cloneAttachments = new FaceMaterialAttachments();

        cloneAttachments->_GetFaceAttachmentsVecR() = m_faceAttachments->_GetFaceAttachmentsVec();
        PSolidUtil::SetFaceAttachments(*instance, cloneAttachments);
        }

    return instance;
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
* @bsimethod                                                    BrienBastings   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
void SetEntityTag(PK_ENTITY_t entityTag)
    {
    PK_MEMORY_block_f(&m_block);

    if (m_owned)
        PK_ENTITY_delete(1, &m_entityTag);

    m_entityTag = entityTag;
    m_owned = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void SetFaceMaterialAttachments(IFaceMaterialAttachmentsP attachments)
    {
    m_faceAttachments = attachments;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
static PSolidKernelEntity* CreateNewEntity(PK_ENTITY_t entityTag, TransformCR transform, bool owned = true)
    {
    if (PK_ENTITY_null == entityTag)
        return nullptr;

    PK_BODY_type_t bodyType = PK_BODY_type_unspecified_c;

    PK_BODY_ask_type(entityTag, &bodyType);

    switch (bodyType)
        {
        case PK_BODY_type_solid_c:
        case PK_BODY_type_sheet_c:
        case PK_BODY_type_wire_c:
            break; // Don't allow empty, minimal, acorn, general, or compound bodies...has unknown ramifications for down-stream operations...

        default:
            return nullptr;
        }

    return new PSolidKernelEntity(entityTag, transform, owned);
    }

}; // PSolidKernelEntity

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IBRepEntityPtr PSolidUtil::CreateNewEntity(PK_ENTITY_t entityTag, TransformCR entityTransform, bool owned)
    {
    return PSolidKernelEntity::CreateNewEntity(entityTag, entityTransform, owned);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IBRepEntityPtr PSolidUtil::InstanceEntity(IBRepEntityCR entity)
    {
    IBRepEntityPtr instance = PSolidKernelEntity::CreateNewEntity(PSolidUtil::GetEntityTag(entity), entity.GetEntityTransform(), false);

    if (instance.IsValid() && nullptr != entity.GetFaceMaterialAttachments())
        PSolidUtil::SetFaceAttachments(*instance, const_cast<IFaceMaterialAttachmentsP> (entity.GetFaceMaterialAttachments()));

    return instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ENTITY_t PSolidUtil::GetEntityTag(IBRepEntityCR entity, bool* isOwned)
    {
    if (isOwned)
        *isOwned = false;

    PSolidKernelEntity const* psEntity = dynamic_cast <PSolidKernelEntity const*> (&entity);

    if (nullptr == psEntity)
        return PK_ENTITY_null;
    
    if (isOwned)
        *isOwned = psEntity->IsOwnedEntity();

    return psEntity->GetEntityTag();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/11
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ENTITY_t PSolidUtil::ExtractEntityTag(IBRepEntityR entity)
    {
    PSolidKernelEntity* psEntity;

    if (nullptr == (psEntity = dynamic_cast <PSolidKernelEntity*> (&entity)))
        return PK_ENTITY_null;

    return psEntity->ExtractEntityTag();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/11
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ENTITY_t PSolidUtil::GetEntityTagForModify(IBRepEntityR entity)
    {
    PSolidKernelEntity* psEntity;

    if (nullptr == (psEntity = dynamic_cast <PSolidKernelEntity*> (&entity)))
        return PK_ENTITY_null;

    if (!psEntity->IsOwnedEntity())
        return PK_ENTITY_null;

    return psEntity->GetEntityTag();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool PSolidUtil::SetEntityTag(IBRepEntityR entity, PK_ENTITY_t entityTag)
    {
    PSolidKernelEntity* psEntity;

    if (nullptr == (psEntity = dynamic_cast <PSolidKernelEntity*> (&entity)))
        return false;

    psEntity->SetEntityTag(entityTag);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
IFaceMaterialAttachmentsPtr PSolidUtil::CreateNewFaceAttachments(PK_ENTITY_t entityTag, Render::GeometryParamsCR baseParams)
    {
    int nFaces = 0;

    if (SUCCESS != PK_BODY_ask_faces(entityTag, &nFaces, nullptr) || 0 == nFaces)
        return nullptr;

    FaceMaterialAttachments* attachments = new FaceMaterialAttachments();

    attachments->_GetFaceAttachmentsVecR().push_back(FaceAttachment(baseParams));

    return attachments;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void PSolidUtil::SetFaceAttachments(IBRepEntityR entity, IFaceMaterialAttachmentsP attachments)
    {
    PSolidKernelEntity* psEntity = dynamic_cast <PSolidKernelEntity*> (&entity);

    if (nullptr == psEntity)
        return;
    
    psEntity->SetFaceMaterialAttachments(attachments);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
PK_FACE_t PSolidUtil::GetPreferredFaceAttachmentFaceForEdge(PK_EDGE_t edgeTag)
    {
    int         nFaces = 0;
    PK_FACE_t*  faceTags = nullptr;

    if (SUCCESS != PK_EDGE_ask_faces(edgeTag, &nFaces, &faceTags) || 0 == nFaces)
        return PK_ENTITY_null;

    int     symbIndex = 0;
    FaceId  entityId;

    // Prefer a face with the same nodeId as the edge...
    if (SUCCESS == PSolidTopoId::IdFromEntity(entityId, edgeTag, true))
        {
        for (int iFace=0; iFace < nFaces; iFace++)
            {
            FaceId  faceId;

            if (SUCCESS != PSolidTopoId::IdFromEntity(faceId, faceTags[iFace], true) || entityId.nodeId != faceId.nodeId)
                continue;

            symbIndex = iFace;
            break;
            }
        }

    PK_FACE_t faceTag = faceTags[symbIndex];

    PK_MEMORY_free(faceTags);

    return faceTag;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
PK_FACE_t PSolidUtil::GetPreferredFaceAttachmentFaceForVertex(PK_VERTEX_t vertexTag)
    {
    int         nFaces = 0;
    PK_FACE_t*  faceTags = nullptr;

    if (SUCCESS != PK_VERTEX_ask_faces(vertexTag, &nFaces, &faceTags) || 0 == nFaces)
        return PK_ENTITY_null;

    PK_FACE_t faceTag = faceTags[0]; // Might need to change to use face with highest node id...

    PK_MEMORY_free(faceTags);

    return faceTag;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/10
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt pki_combine_memory_blocks (PK_MEMORY_block_t* pBuffer)
    {
    if (nullptr == pBuffer->next)
        return SUCCESS;

    // Create block with single contiguous buffer...
    PK_MEMORY_block_t*  thisBufferP = pBuffer;
    PK_MEMORY_block_t   tmpBlock;

    memset (&tmpBlock, 0, sizeof (tmpBlock));

    do
        {
        tmpBlock.n_bytes += thisBufferP->n_bytes;

        } while (thisBufferP = thisBufferP->next);

    if (nullptr == (tmpBlock.bytes = (char *) malloc (tmpBlock.n_bytes)))
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
    PK_ERROR_code_t result = PK_PART_transmit_b(1, &entityTagIn, &transmitOptions, pBuffer);
    if (PK_ERROR_no_errors != result)
        {
        LOG.errorv("pki_save_entity_to_memory failed with status %d", result);
        return ERROR;
        }

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
    PK_PART_t*  parts = nullptr;

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
BentleyStatus PSolidUtil::SaveEntityToMemory
(
uint8_t**       ppBuffer,
size_t&         bufferSize,
IBRepEntityCR   entity
)
    {
    PSolidKernelEntity const* psEntity = dynamic_cast <PSolidKernelEntity const*> (&entity);

    if (!psEntity)
        return ERROR; // Should never happen...

    PK_MEMORY_block_t*  block = psEntity->GetPrivateMemoryBlock();

    if (SUCCESS != pki_save_entity_to_memory(block, psEntity->GetEntityTag (), DataVersion_PSolid) ||
        SUCCESS != pki_combine_memory_blocks(block))
        return ERROR;

    *ppBuffer  = const_cast<uint8_t*>((uint8_t const*)block->bytes);
    bufferSize = block->n_bytes;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidUtil::RestoreEntityFromMemory
(
IBRepEntityPtr& entityOut,
uint8_t const*  buffer,
size_t          bufferSize,
TransformCR     transform
)
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

//=======================================================================================
// @bsiclass 
//=======================================================================================
struct PSolidTopoSubEntity : public RefCounted<ISubEntity>
{
protected:

PK_ENTITY_t                         m_entityTag;
GeometricPrimitiveCPtr              m_parentGeom; // Needs to be non-owning IBRepEntity to avoid freeing body out from under ElementGeometryTool cache...
mutable GeometricPrimitiveCPtr      m_entityGeom; // Create on demand...
mutable DRange3d                    m_range = DRange3d::NullRange(); // Calculate on demand...
mutable Render::GraphicPtr          m_graphic; // Create on demand...
DPoint2d                            m_param = DPoint2d::FromZero(); // Locate param(s) for face/edge...convenient for tools...
DPoint3d                            m_point = DPoint3d::FromZero(); // Locate point for face/edge/vertex...convenient for tools...
bool                                m_haveLocation = false; // Whether this sub-entity was created from a locate and point/param are valid...
bool                                m_displayTangentEdges = false; // Whether to add tangent blend edges to graphic...

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
PSolidTopoSubEntity(GeometricPrimitiveCR parentGeom, PK_ENTITY_t entityTag) : m_parentGeom(&parentGeom), m_entityTag(entityTag) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
GeometricPrimitiveCPtr _GetParentGeometry() const override {return m_parentGeom;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
SubEntityType _GetSubEntityType() const override
    {
    PK_CLASS_t  entityClass;

    PK_ENTITY_ask_class(m_entityTag, &entityClass);

    switch (entityClass)
        {
        case PK_CLASS_face:
            return ISubEntity::SubEntityType::Face;

        case PK_CLASS_edge:
            return ISubEntity::SubEntityType::Edge;

        case PK_CLASS_vertex:
        default:
            return ISubEntity::SubEntityType::Vertex;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool _IsEqual(ISubEntityCR subEntity) const override
    {
    if (this == &subEntity)
        return true;

    PSolidTopoSubEntity const* topoSubEntity;

    if (nullptr == (topoSubEntity = dynamic_cast <PSolidTopoSubEntity const*> (&subEntity)))
        return false;

    if (m_entityTag == topoSubEntity->m_entityTag)
        return true;

    if (!m_displayTangentEdges || !topoSubEntity->m_displayTangentEdges)
        return false;

    bvector<PK_EDGE_t> smoothEdges;

    if (SUCCESS != PSolidTopo::GetTangentBlendEdges(smoothEdges, m_entityTag))
        return false;

    if (smoothEdges.end() == std::find(smoothEdges.begin(), smoothEdges.end(), topoSubEntity->m_entityTag))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsParentEqual(GeometricPrimitiveCR parent) const override
    {
    if (GeometricPrimitive::GeometryType::BRepEntity != parent.GetGeometryType())
        return false;

    return (PSolidUtil::GetBodyForEntity(m_entityTag) == PSolidUtil::GetEntityTag(*parent.GetAsIBRepEntity()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
GeometricPrimitiveCPtr _GetGeometry() const override
    {
    if (!m_entityGeom.IsValid())
        {
        PK_CLASS_t entityClass;

        PK_ENTITY_ask_class(m_entityTag, &entityClass);

        switch (entityClass)
            {
            case PK_CLASS_face:
                {
                PK_BODY_t sheetTag = PK_ENTITY_null;

                if (SUCCESS != PK_FACE_make_sheet_body(1, &m_entityTag, &sheetTag))
                    break;

                m_entityGeom = GeometricPrimitive::Create(PSolidKernelEntity::CreateNewEntity(sheetTag, m_parentGeom->GetAsIBRepEntity()->GetEntityTransform()));
                break;
                }

            case PK_CLASS_edge:
                {
                ICurvePrimitivePtr curve;

                if (SUCCESS != PSolidGeom::EdgeToCurvePrimitive(curve, m_entityTag))
                    break;

                curve->TransformInPlace(m_parentGeom->GetAsIBRepEntity()->GetEntityTransform());
                m_entityGeom = GeometricPrimitive::Create(curve);
                break;
                }

            case PK_CLASS_vertex:
                {
                DPoint3d point;

                if (SUCCESS != PSolidUtil::GetVertex(point, m_entityTag))
                    break;

                m_parentGeom->GetAsIBRepEntity()->GetEntityTransform().Multiply(point);
                m_entityGeom = GeometricPrimitive::Create(ICurvePrimitive::CreatePointString(&point, 1));
                }
            }
        }

    return m_entityGeom;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr _GetGraphic(ViewContextR context) const override
    {
    if (!m_graphic.IsValid())
        {
        auto builder = context.CreateSceneGraphic(); // Don't supply viewport, graphic has to be independent of view render mode, etc.
        builder->SetSymbology(ColorDef::White(), ColorDef::White(), 1); // Expect caller to draw using overrides to control color, weight, and style.

        GeometricPrimitiveCPtr geom = _GetGeometry();

        if (geom.IsValid())
            {
            if (m_displayTangentEdges)
                {
                bvector<PK_EDGE_t> smoothEdges;

                if (SUCCESS == PSolidTopo::GetTangentBlendEdges(smoothEdges, m_entityTag))
                    {
                    for (PK_EDGE_t thisEdgeTag : smoothEdges)
                        {
                        if (thisEdgeTag == m_entityTag)
                            continue;

                        ICurvePrimitivePtr curve;

                        if (SUCCESS != PSolidGeom::EdgeToCurvePrimitive(curve, thisEdgeTag))
                            continue;

                        curve->TransformInPlace(m_parentGeom->GetAsIBRepEntity()->GetEntityTransform());
                        builder->AddCurveVectorR(*CurveVector::Create(CurveVector::BOUNDARY_TYPE_None, curve), false);
                        }
                    }
                }

            geom->AddToGraphic(*builder);
            m_graphic = builder->Finish();
            }
        }

    return m_graphic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d _GetSubEntityRange() const override
    {
    if (m_range.IsNull())
        {
        PK_BOX_t box;
        DPoint3d point;

        if (PK_ERROR_no_errors == PK_TOPOL_find_box(m_entityTag, &box)) // <- takes care of face and edge...not vertex...
            m_range.InitFrom(box.coord[0], box.coord[1], box.coord[2], box.coord[3], box.coord[4], box.coord[5]);
        else if (SUCCESS == PSolidUtil::GetVertex(point, m_entityTag))
            m_range.InitFrom(point);
        else
            m_range.InitFrom(DPoint3d::FromZero()); // Don't keep calling _GetGeometry, should maybe set to an "empty" range?

        m_parentGeom->GetAsIBRepEntity()->GetEntityTransform().Multiply(m_range, m_range);
        }

    return m_range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool _GetFaceLocation(DPoint3dR point, DPoint2dR param) const override
    {
    if (!m_haveLocation)
        return false;

    PK_CLASS_t entityClass;

    PK_ENTITY_ask_class(m_entityTag, &entityClass);

    if (PK_CLASS_face != entityClass)
        return false;

    point = m_point;
    param = m_param;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool _GetEdgeLocation(DPoint3dR point, double& param) const override
    {
    if (!m_haveLocation)
        return false;

    PK_CLASS_t entityClass;

    PK_ENTITY_ask_class(m_entityTag, &entityClass);

    if (PK_CLASS_edge != entityClass)
        return false;

    point = m_point;
    param = m_param.x;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool _GetVertexLocation(DPoint3dR point) const override
    {
    PK_CLASS_t entityClass;

    PK_ENTITY_ask_class(m_entityTag, &entityClass);

    if (PK_CLASS_vertex != entityClass)
        return false;

    if (!m_haveLocation)
        {
        if (SUCCESS != PSolidUtil::GetVertex(point, m_entityTag))
            return false;

        m_parentGeom->GetAsIBRepEntity()->GetEntityTransform().Multiply(point);

        return true;
        }

    point = m_point;

    return true;
    }

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsValidEntityType(PK_ENTITY_t entityTag)
    {
    PK_CLASS_t  entityClass;

    PK_ENTITY_ask_class(entityTag, &entityClass);

    switch (entityClass)
        {
        case PK_CLASS_face:
        case PK_CLASS_edge:
        case PK_CLASS_vertex:
            return true;

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
static PK_ENTITY_t GetEntityTag(ISubEntityCR subEntity)
    {
    PSolidTopoSubEntity const* topoSubEntity;

    if (nullptr == (topoSubEntity = dynamic_cast <PSolidTopoSubEntity const*> (&subEntity)))
        return PK_ENTITY_null;

    return topoSubEntity->m_entityTag;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
static Transform GetEntityTransform(ISubEntityCR subEntity)
    {
    PSolidTopoSubEntity const* topoSubEntity;

    if (nullptr == (topoSubEntity = dynamic_cast <PSolidTopoSubEntity const*> (&subEntity)))
        return Transform::FromIdentity();

    return topoSubEntity->m_parentGeom->GetAsIBRepEntity()->GetEntityTransform();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
static void SetLocation(ISubEntityR subEntity, DPoint3dCR point, DPoint2dCR param)
    {
    PSolidTopoSubEntity* topoSubEntity;

    if (nullptr == (topoSubEntity = dynamic_cast <PSolidTopoSubEntity*> (&subEntity)))
        return;

    topoSubEntity->m_point = point;
    topoSubEntity->m_param = param;
    topoSubEntity->m_haveLocation = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
static void SetDisplayTangentEdges(ISubEntityR subEntity, bool display)
    {
    PSolidTopoSubEntity* topoSubEntity;

    if (nullptr == (topoSubEntity = dynamic_cast <PSolidTopoSubEntity*> (&subEntity)))
        return;

    PK_CLASS_t entityClass;

    PK_ENTITY_ask_class(topoSubEntity->m_entityTag, &entityClass);

    if (PK_CLASS_edge != entityClass)
        return;

    topoSubEntity->m_displayTangentEdges = display;
    topoSubEntity->m_graphic = nullptr; // Invalidate graphic...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdateCache(ISubEntityCR donorEntity)
    {
    PSolidTopoSubEntity const* donor;

    if (nullptr == (donor = dynamic_cast<PSolidTopoSubEntity const*>(&donorEntity)))
        return;

    // Use donor geometry/range only when entity tag matches; don't want to update if this is just a tangent edge when m_displayTangentEdges is set...
    if (!m_entityGeom.IsValid() && m_entityTag == donor->m_entityTag)
        {
        m_entityGeom = donor->m_entityGeom;
        m_range = donor->m_range;
        }

    // Use donor graphic only when m_displayTangentEdges matches...
    if (m_displayTangentEdges == donor->m_displayTangentEdges)
        m_graphic = donor->m_graphic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
static ISubEntityPtr CreateSubEntityPtr(PK_ENTITY_t entityTag, GeometricPrimitiveCR parent)
    {
    return new PSolidTopoSubEntity(parent, entityTag);
    }

}; // PSolidTopoSubEntity

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
ISubEntityPtr PSolidSubEntity::CreateSubEntity(PK_ENTITY_t entityTag, TransformCR transform)
    {
    if (!PSolidTopoSubEntity::IsValidEntityType(entityTag))
        return nullptr;

    // NOTE: Create non-owning instance, LocateSubEntityTool cache needs to control life-span of parent IBRepEntity...
    return PSolidTopoSubEntity::CreateSubEntityPtr(entityTag, *GeometricPrimitive::Create(PSolidUtil::CreateNewEntity(PSolidUtil::GetBodyForEntity(entityTag), transform, false)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
ISubEntityPtr PSolidSubEntity::CreateSubEntity(PK_ENTITY_t entityTag, IBRepEntityCR parent)
    {
    if (!PSolidTopoSubEntity::IsValidEntityType(entityTag))
        return nullptr;

    // NOTE: Create non-owning instance, LocateSubEntityTool cache needs to control life-span of parent IBRepEntity...
    return PSolidTopoSubEntity::CreateSubEntityPtr(entityTag, *GeometricPrimitive::Create(PSolidUtil::CreateNewEntity(PSolidUtil::GetEntityTag(parent), parent.GetEntityTransform(), false)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/12
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ENTITY_t PSolidSubEntity::GetSubEntityTag(ISubEntityCR subEntity)
    {
    return PSolidTopoSubEntity::GetEntityTag(subEntity);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
Transform PSolidSubEntity::GetSubEntityTransform(ISubEntityCR subEntity)
    {
    return PSolidTopoSubEntity::GetEntityTransform(subEntity);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
void PSolidSubEntity::SetLocation(ISubEntityR subEntity, DPoint3dCR point, DPoint2dCR param)
    {
    PSolidTopoSubEntity::SetLocation(subEntity, point, param);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void PSolidSubEntity::SetDisplayTangentEdges(ISubEntityR subEntity, bool display)
    {
    PSolidTopoSubEntity::SetDisplayTangentEdges(subEntity, display);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void PSolidSubEntity::UpdateCache(ISubEntityR subEntity, ISubEntityCR donorEntity)
    {
    PSolidTopoSubEntity* topoSubEntity;

    if (nullptr == (topoSubEntity = dynamic_cast<PSolidTopoSubEntity*>(&subEntity)))
        return;

    topoSubEntity->UpdateCache(donorEntity);
    }





