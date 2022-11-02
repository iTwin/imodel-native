/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BRepCore/SolidKernel.h>
#include <BRepCore/PSolidUtil.h>
#include <Bentley/Logging.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN

#define LOG (NativeLogging::CategoryLogger("BRepCore"))

enum SolidDataVersion
    {
    DataVersion_PSolid = 120, //!< Transmit schema version used to persist Parasolid data in a dgn file.
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct FaceMaterialAttachments : RefCounted <IFaceMaterialAttachments>
{
private:

T_FaceAttachmentsVec    m_faceAttachmentsVec;

public:

FaceMaterialAttachments() {}
explicit FaceMaterialAttachments(FaceAttachment face) : m_faceAttachmentsVec(1, face) { }

T_FaceAttachmentsVec const& _GetFaceAttachmentsVec() const override {return m_faceAttachmentsVec;}
T_FaceAttachmentsVec& _GetFaceAttachmentsVecR() override {return m_faceAttachmentsVec;}

}; // FaceMaterialAttachments

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static IBRepEntity::EntityType entityTypeFromBodyType(PK_BODY_type_t bodyType)
    {
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

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PSolidKernelEntity : public RefCounted <IBRepEntity>
{
private:

PK_ENTITY_t                 m_entityTag;
Transform                   m_transform;
bool                        m_owned;
mutable PK_MEMORY_block_t   m_block;
IFaceMaterialAttachmentsPtr m_faceAttachments;

mutable DRange3d            m_localRange;
mutable bool                m_isLocalRangeCached;
mutable EntityType          m_entityType;
mutable bool                m_isEntityTypeCached;
mutable bool                m_hasCurvedFaceOrEdge;
mutable bool                m_isHasCurvedFaceOrEdgeCached;

protected:

Transform _GetEntityTransform() const override {return m_transform;}
bool _SetEntityTransform(TransformCR transform) override {m_transform = transform; return true;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
EntityType _GetEntityType() const override
    {
    if (!m_isEntityTypeCached)
        {
        PK_BODY_type_t bodyType = PK_BODY_type_unspecified_c;
        PK_BODY_ask_type(m_entityTag, &bodyType);
        m_entityType = entityTypeFromBodyType(bodyType);
        m_isEntityTypeCached = true;
        }

    return m_entityType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool _HasCurvedFaceOrEdge() const override
    {
    if (!m_isHasCurvedFaceOrEdgeCached)
        {
        m_hasCurvedFaceOrEdge = PSolidUtil::HasCurvedFaceOrEdge(m_entityTag);
        m_isHasCurvedFaceOrEdgeCached = true;
        }

    return m_hasCurvedFaceOrEdge;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d _GetLocalEntityRange() const override
    {
    if (!m_isLocalRangeCached)
        {
        PK_BOX_t box;
        if (PK_ERROR_no_errors != PK_TOPOL_find_box(m_entityTag, &box))
            m_localRange = DRange3d::NullRange();
        else
            m_localRange = DRange3d::From(box.coord[0], box.coord[1], box.coord[2], box.coord[3], box.coord[4], box.coord[5]);

        m_isLocalRangeCached = true;
        }

    return m_localRange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d _GetEntityRange() const override
    {
    DRange3d worldRange = _GetLocalEntityRange();
    if (!worldRange.IsNull())
        m_transform.Multiply(worldRange, worldRange);
    return worldRange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IFaceMaterialAttachmentsCP _GetFaceMaterialAttachments() const override
    {
    return m_faceAttachments.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void _SetFaceMaterialAttachments(T_FaceAttachmentsVec const& attachments, T_FaceIndexToAttachmentIndexVec const* faceIndexToAttachIndex) override
    {
    for (FaceAttachment attachment : attachments)
        {
        if (!m_faceAttachments.IsValid())
            {
            IFaceMaterialAttachmentsPtr attachments = PSolidUtil::CreateNewFaceAttachments(m_entityTag, attachment);

            if (!attachments.IsValid())
                return;

            m_faceAttachments = attachments.get();
            }
        else
            {
            m_faceAttachments->_GetFaceAttachmentsVecR().push_back(attachment);
            }
        }

    // Support for older BRep that didn't have face attachment index attrib and add the attributes now...
    if (nullptr == faceIndexToAttachIndex)
        return;

    int         nFaces;
    PK_FACE_t*  faces = nullptr;

    if (SUCCESS != PK_BODY_ask_faces(m_entityTag, &nFaces, &faces))
        return;

    bmap<int32_t, uint32_t> subElemIdToFaceMap;

    for (int iFace = 0; iFace < nFaces; iFace++)
        subElemIdToFaceMap[iFace + 1] = faces[iFace]; // subElemId is 1 based face index...

    PK_MEMORY_free(faces);

    for (FaceIndexToAttachmentIndex faceAttIndex : *faceIndexToAttachIndex)
        {
        bmap<int32_t, uint32_t>::const_iterator foundIndex = subElemIdToFaceMap.find(faceAttIndex.m_faceIndex);

        if (foundIndex == subElemIdToFaceMap.end())
            continue;

        PSolidAttrib::SetFaceMaterialIndexAttribute(foundIndex->second, faceAttIndex.m_symbIndex); // NOTE: Call with 0 will remove an existing attrib...
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IBRepEntityPtr _CreateInstance(bool owned) const override
    {
    return PSolidUtil::InstanceEntity(*this, owned);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool _IsAlive() const override
    {
    PK_LOGICAL_t isEntity = PK_LOGICAL_false;
    return (PK_ERROR_no_errors == PK_ENTITY_is(m_entityTag, &isEntity) && isEntity);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void _ChangePartition() const override
    {
    PK_BODY_change_partition(m_entityTag, PSolidThreadUtil::GetThreadPartition());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool _IsSameStructureAndGeometry(IBRepEntityCR entity, double tolerance) const override
    {
    // Can't ignore body to uor transform...post instancing in V8 converter only compares local range for a match which doesn't account for body transform differences...
    if (!m_transform.IsEqual(entity.GetEntityTransform(), tolerance, tolerance))
        return false;

    double      solidTolerance = tolerance;
    PK_BODY_t   bodyTag1 = m_entityTag;
    PK_BODY_t   bodyTag2 = PSolidUtil::GetEntityTag(entity);

    if (0.0 != solidTolerance)
        {
        Transform uorToSolid;

        uorToSolid.InverseOf(m_transform);
        uorToSolid.ScaleDoubleArrayByXColumnMagnitude(&solidTolerance, 1);
        }

    return PSolidUtil::AreBodiesEqual(bodyTag1, bodyTag2, solidTolerance, nullptr);
    }

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PSolidKernelEntity(PK_ENTITY_t entityTag, PK_BODY_type_t bodyType, TransformCR transform, bool owned)
    {
    m_entityTag = entityTag;
    m_transform = transform;
    m_owned     = owned;

    InvalidateCachedProperties();
    m_entityType = entityTypeFromBodyType(bodyType);
    m_isEntityTypeCached = true;

    memset (&m_block, 0, sizeof(m_block));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
~PSolidKernelEntity()
    {
    PK_MEMORY_block_f(&m_block);

    if (!m_owned)
        return;

    PK_ENTITY_delete(1, &m_entityTag);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PK_MEMORY_block_t* GetPrivateMemoryBlock() const
    {
    PK_MEMORY_block_f(&m_block); // Avoid leak if _SaveEntityToMemory called twice on same entity...

    return &m_block;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsOwnedEntity() const
    {
    return m_owned;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ENTITY_t GetEntityTag() const
    {
    return m_entityTag;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ENTITY_t ExtractEntityTag()
    {
    if (!m_owned)
        return PK_ENTITY_null;

    PK_ENTITY_t entityTag = m_entityTag;
    m_entityTag = PK_ENTITY_null;
    InvalidateCachedProperties();

    return entityTag;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SetEntityTag(PK_ENTITY_t entityTag)
    {
    InvalidateCachedProperties();

    PK_MEMORY_block_f(&m_block);

    if (m_owned)
        PK_ENTITY_delete(1, &m_entityTag);

    m_entityTag = entityTag;
    m_owned = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SetFaceMaterialAttachments(IFaceMaterialAttachmentsP attachments)
    {
    m_faceAttachments = attachments;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void InvalidateCachedProperties()
    {
    m_isEntityTypeCached = false;
    m_isLocalRangeCached = false;
    m_isHasCurvedFaceOrEdgeCached = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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

    return new PSolidKernelEntity(entityTag, bodyType, transform, owned);
    }

}; // PSolidKernelEntity

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IBRepEntityPtr PSolidUtil::CreateNewEntity(PK_ENTITY_t entityTag, TransformCR entityTransform, bool owned)
    {
    return PSolidKernelEntity::CreateNewEntity(entityTag, entityTransform, owned);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IBRepEntityPtr PSolidUtil::InstanceEntity(IBRepEntityCR entity, bool owned)
    {
    IBRepEntityPtr instance = PSolidKernelEntity::CreateNewEntity(PSolidUtil::GetEntityTag(entity), entity.GetEntityTransform(), owned);

    if (instance.IsValid() && nullptr != entity.GetFaceMaterialAttachments())
        PSolidUtil::SetFaceAttachments(*instance, const_cast<IFaceMaterialAttachmentsP> (entity.GetFaceMaterialAttachments()));

    return instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ENTITY_t PSolidUtil::ExtractEntityTag(IBRepEntityR entity)
    {
    PSolidKernelEntity* psEntity;

    if (nullptr == (psEntity = dynamic_cast <PSolidKernelEntity*> (&entity)))
        return PK_ENTITY_null;

    return psEntity->ExtractEntityTag();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ENTITY_t PSolidUtil::GetEntityTagForModify(IBRepEntityR entity)
    {
    PSolidKernelEntity* psEntity;

    if (nullptr == (psEntity = dynamic_cast <PSolidKernelEntity*> (&entity)))
        return PK_ENTITY_null;

    if (!psEntity->IsOwnedEntity())
        return PK_ENTITY_null;

    psEntity->InvalidateCachedProperties();
    return psEntity->GetEntityTag();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IFaceMaterialAttachmentsPtr PSolidUtil::CreateNewFaceAttachments(PK_ENTITY_t entityTag, FaceAttachment baseAttachment)
    {
    int nFaces = 0;

    if (SUCCESS != PK_BODY_ask_faces(entityTag, &nFaces, nullptr) || 0 == nFaces)
        return nullptr;

    return new FaceMaterialAttachments(baseAttachment);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PSolidUtil::SetFaceAttachments(IBRepEntityR entity, IFaceMaterialAttachmentsP attachments)
    {
    PSolidKernelEntity* psEntity = dynamic_cast <PSolidKernelEntity*> (&entity);

    if (nullptr == psEntity)
        return;

    psEntity->SetFaceMaterialAttachments(attachments);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
IBRepEntityCPtr                     m_parentGeom; // Needs to be non-owning IBRepEntity to avoid freeing body out from under ElementGeometryTool cache...
mutable TopologyPrimitiveCPtr       m_entityGeom; // Create on demand...
mutable DRange3d                    m_range = DRange3d::NullRange(); // Calculate on demand...
DPoint2d                            m_param = DPoint2d::FromZero(); // Locate param(s) for face/edge...convenient for tools...
DPoint3d                            m_point = DPoint3d::FromZero(); // Locate point for face/edge/vertex...convenient for tools...
bool                                m_haveLocation = false; // Whether this sub-entity was created from a locate and point/param are valid...

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PSolidTopoSubEntity(IBRepEntityCR parentGeom, PK_ENTITY_t entityTag) : m_parentGeom(&parentGeom), m_entityTag(entityTag) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IBRepEntityCPtr _GetParentGeometry() const override {return m_parentGeom;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TopologyPrimitiveCPtr _GetGeometry() const override
    {
    if (!m_entityGeom.IsValid())
        {
        PK_CLASS_t entityClass;

        PK_ENTITY_ask_class(m_entityTag, &entityClass);

        switch (entityClass)
            {
            case PK_CLASS_face:
                {
                CurveVectorPtr region = PSolidGeom::PlanarFaceToCurveVector(m_entityTag);

                if (region.IsValid())
                    {
                    region->TransformInPlace(m_parentGeom->GetEntityTransform());
                    m_entityGeom = TopologyPrimitive::Create(region);
                    break;
                    }

                ISolidPrimitivePtr surface = PSolidGeom::FaceToSolidPrimitive(m_entityTag);

                if (surface.IsValid())
                    {
                    surface->TransformInPlace(m_parentGeom->GetEntityTransform());
                    m_entityGeom = TopologyPrimitive::Create(surface);
                    break;
                    }

                PK_BODY_t sheetTag = PK_ENTITY_null;

                if (SUCCESS != PK_FACE_make_sheet_body(1, &m_entityTag, &sheetTag))
                    break;

                m_entityGeom = TopologyPrimitive::Create(PSolidKernelEntity::CreateNewEntity(sheetTag, m_parentGeom->GetEntityTransform()));
                break;
                }

            case PK_CLASS_edge:
                {
                ICurvePrimitivePtr curve;

                if (SUCCESS != PSolidGeom::EdgeToCurvePrimitive(curve, m_entityTag))
                    break;

                curve->TransformInPlace(m_parentGeom->GetEntityTransform());
                m_entityGeom = TopologyPrimitive::Create(curve);
                break;
                }

            case PK_CLASS_vertex:
                {
                DPoint3d point;

                if (SUCCESS != PSolidUtil::GetVertex(point, m_entityTag))
                    break;

                m_parentGeom->GetEntityTransform().Multiply(point);
                m_entityGeom = TopologyPrimitive::Create(ICurvePrimitive::CreatePointString(&point, 1));
                }
            }
        }

    return m_entityGeom;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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

        m_parentGeom->GetEntityTransform().Multiply(m_range, m_range);
        }

    return m_range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool _SetFaceLocation(DPoint3dCR point, DPoint2dCR uvParam) override
    {
    PK_CLASS_t entityClass;

    PK_ENTITY_ask_class(m_entityTag, &entityClass);

    if (PK_CLASS_face != entityClass)
        return false;

    m_point = point;
    m_param = uvParam;
    m_haveLocation = true;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool _SetEdgeLocation(DPoint3dCR point, double uParam) override
    {
    PK_CLASS_t entityClass;

    PK_ENTITY_ask_class(m_entityTag, &entityClass);

    if (PK_CLASS_edge != entityClass)
        return false;

    m_point = point;
    m_param.x = uParam;
    m_param.y = 0.0;
    m_haveLocation = true;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool _GetVertexLocation(DPoint3dR point) const override
    {
    PK_CLASS_t entityClass;

    PK_ENTITY_ask_class(m_entityTag, &entityClass);

    if (PK_CLASS_vertex != entityClass)
        return false;

    if (SUCCESS != PSolidUtil::GetVertex(point, m_entityTag))
        return false;

    m_parentGeom->GetEntityTransform().Multiply(point);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool _IsHidden() const override
    {
    bool isHiddenEntity = false;

    return (SUCCESS == PSolidAttrib::GetHiddenAttribute(isHiddenEntity, m_entityTag) && isHiddenEntity);
    }

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static PK_ENTITY_t GetEntityTag(ISubEntityCR subEntity)
    {
    PSolidTopoSubEntity const* topoSubEntity;

    if (nullptr == (topoSubEntity = dynamic_cast <PSolidTopoSubEntity const*> (&subEntity)))
        return PK_ENTITY_null;

    return topoSubEntity->m_entityTag;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Transform GetEntityTransform(ISubEntityCR subEntity)
    {
    PSolidTopoSubEntity const* topoSubEntity;

    if (nullptr == (topoSubEntity = dynamic_cast <PSolidTopoSubEntity const*> (&subEntity)))
        return Transform::FromIdentity();

    return topoSubEntity->m_parentGeom->GetEntityTransform();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ISubEntityPtr CreateSubEntityPtr(PK_ENTITY_t entityTag, IBRepEntityCR parent)
    {
    return new PSolidTopoSubEntity(parent, entityTag);
    }

}; // PSolidTopoSubEntity

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ISubEntityPtr PSolidSubEntity::CreateSubEntity(PK_ENTITY_t entityTag, TransformCR transform)
    {
    if (!PSolidTopoSubEntity::IsValidEntityType(entityTag))
        return nullptr;

    // NOTE: Create non-owning instance, LocateSubEntityTool cache needs to control life-span of parent IBRepEntity...
    return PSolidTopoSubEntity::CreateSubEntityPtr(entityTag, *PSolidUtil::CreateNewEntity(PSolidUtil::GetBodyForEntity(entityTag), transform, false));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ISubEntityPtr PSolidSubEntity::CreateSubEntity(PK_ENTITY_t entityTag, IBRepEntityCR parent)
    {
    if (!PSolidTopoSubEntity::IsValidEntityType(entityTag))
        return nullptr;

    // NOTE: Create non-owning instance, LocateSubEntityTool cache needs to control life-span of parent IBRepEntity...
    return PSolidTopoSubEntity::CreateSubEntityPtr(entityTag, *PSolidUtil::CreateNewEntity(PSolidUtil::GetEntityTag(parent), parent.GetEntityTransform(), false));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PK_ENTITY_t PSolidSubEntity::GetSubEntityTag(ISubEntityCR subEntity)
    {
    return PSolidTopoSubEntity::GetEntityTag(subEntity);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Transform PSolidSubEntity::GetSubEntityTransform(ISubEntityCR subEntity)
    {
    return PSolidTopoSubEntity::GetEntityTransform(subEntity);
    }






