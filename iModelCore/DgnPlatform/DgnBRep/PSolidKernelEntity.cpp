/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnBRep/PSolidKernelEntity.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
T_FaceToSubElemIdMap    m_faceToSubElemIdMap;

public:

FaceMaterialAttachments() {}

virtual T_FaceAttachmentsVec const& _GetFaceAttachmentsVec() const override {return m_faceAttachmentsVec;}
virtual T_FaceToSubElemIdMap const& _GetFaceToSubElemIdMap() const override {return m_faceToSubElemIdMap;}

virtual T_FaceAttachmentsVec& _GetFaceAttachmentsVecR() override {return m_faceAttachmentsVec;}
virtual T_FaceToSubElemIdMap& _GetFaceToSubElemIdMapR() override {return m_faceToSubElemIdMap;}

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
virtual bool _IsEqual(IBRepEntityCR entity) const override
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

    IFaceMaterialAttachmentsPtr attachments = PSolidUtil::CreateNewFaceAttachments(m_entityTag, *baseParams);

    if (!attachments.IsValid())
        return false;

    m_faceAttachments = attachments;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual IBRepEntityPtr _Clone() const override
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
    return PSolidKernelEntity::CreateNewEntity(PSolidUtil::GetEntityTag(entity), entity.GetEntityTransform(), false);
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

    PK_ENTITY_t entityTag;

    // *** IMPORTANT *** Extract fails for a non-owning/cached entity ptr. We must create a copy to make sure it's not freed!!!
    if (PK_ENTITY_null == (entityTag = psEntity->ExtractEntityTag()))
        PK_ENTITY_copy(psEntity->GetEntityTag(), &entityTag);

    return entityTag;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
IFaceMaterialAttachmentsPtr PSolidUtil::CreateNewFaceAttachments(PK_ENTITY_t entityTag, Render::GeometryParamsCR baseParams)
    {
    int         nFaces;
    PK_FACE_t*  faces = NULL;

    if (SUCCESS != PK_BODY_ask_faces(entityTag, &nFaces, &faces))
        return nullptr;

    FaceMaterialAttachments* attachments = new FaceMaterialAttachments();

    attachments->_GetFaceAttachmentsVecR().push_back(FaceAttachment(baseParams));

    for (int iFace = 0; iFace < nFaces; iFace++)
        attachments->_GetFaceToSubElemIdMapR()[faces[iFace]] = make_bpair(iFace + 1, 0);
    
    PK_MEMORY_free(faces);

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

    *ppBuffer  = (uint8_t*) block->bytes;
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

