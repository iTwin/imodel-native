/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/ScalableMeshV8Utilities.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//#include "ConverterInternal.h"
//
#include <DgnDbSync/DgnV8/DgnV8.h>
#include <VersionedDgnV8Api/ScalableMeshElement/ScalableMeshAttachment.h>
#include <VersionedDgnV8Api/ScalableMesh/ScalableMeshDefs.h>

#define XATTRIBUTE_MINOR_ID_COVERAGE 6
#define XATTRIBUTE_MINOR_ID_CLIPDEFINITIONS 7
static XAttributeHandlerId  s_xAttributeCoverageId(XATTRIBUTEID_ScalableMesh, (int)XATTRIBUTE_MINOR_ID_COVERAGE);
static XAttributeHandlerId  s_xAttributeClipDefinitionsId(XATTRIBUTEID_ScalableMesh, (int)XATTRIBUTE_MINOR_ID_CLIPDEFINITIONS);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                 01/18
+---------------+---------------+---------------+---------------+---------------+------*/

BentleyStatus ExtractClipDefinitionsInfo(bmap <uint64_t, bpair<DGNV8_BENTLEY_NAMESPACE_NAME::ScalableMesh::SMNonDestructiveClipType, bvector<DPoint3d>>> * clipDefs, DgnV8EhCR eh)
    {
    if (NULL == eh.GetModelRef() || NULL == eh.GetModelRef()->GetDgnModelP())
        {
        BeAssert(false);
        return ERROR;
        }

    ElementHandle::XAttributeIter xai(eh, s_xAttributeClipDefinitionsId, 0);

    if (!xai.IsValid())
        return ERROR;

    DataInternalizer source((byte*)xai.PeekData(), xai.GetSize());

    UInt32 nOfClipDefs = 0;
    if (!source.AtOrBeyondEOS())
        source.get(&nOfClipDefs);
    if (nOfClipDefs > 0)
        {
        for (UInt32 i = 0; i < nOfClipDefs; ++i)
            {
            UInt64 clipId = 0;
            if (!source.AtOrBeyondEOS())
                source.get(&clipId);
            UInt32 type = 0;
            if (!source.AtOrBeyondEOS())
                source.get(&type);

            Int8 isActive;
            if (!source.AtOrBeyondEOS())
                source.get(&isActive);

            UInt32 geomType;
            if (!source.AtOrBeyondEOS())
                source.get(&geomType);

            UInt64 clipSize = 0;
            if (!source.AtOrBeyondEOS())
                source.get(&clipSize);

            bvector<DPoint3d> clipDef(clipSize);

            for (uint64_t j = 0; j < clipSize; ++j)
                {
                if (!source.AtOrBeyondEOS())
                    source.get(&clipDef[j].x);
                if (!source.AtOrBeyondEOS())
                    source.get(&clipDef[j].y);
                if (!source.AtOrBeyondEOS())
                    source.get(&clipDef[j].z);
                }
            (*clipDefs).insert(make_bpair(clipId, make_bpair((ScalableMesh::SMNonDestructiveClipType)type, clipDef)));
            }
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                 01/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ExtractTerrainLinkInfo(bvector<bpair<uint64_t, uint64_t>>* linksToModels, DgnV8EhCR eh)
    {
    if (NULL == eh.GetModelRef() || NULL == eh.GetModelRef()->GetDgnModelP())
        {
        BeAssert(false);
        return ERROR;
        }

    ElementHandle::XAttributeIter xai(eh, s_xAttributeCoverageId, 0);

    if (!xai.IsValid())
        return ERROR;

    DataInternalizer source((byte*)xai.PeekData(), xai.GetSize());

    UInt32 nOfLinkedGroundEntries = 0;
    if (!source.AtOrBeyondEOS())
        source.get(&nOfLinkedGroundEntries);
    if (nOfLinkedGroundEntries > 0)
        {
        for (UInt32 i = 0; i < nOfLinkedGroundEntries; ++i)
            {
            UInt64 terrainModelId = 0, clipId = 0;
            if (!source.AtOrBeyondEOS())
                source.get(&terrainModelId);
            if (!source.AtOrBeyondEOS())
                source.get(&clipId);
            (*linksToModels).push_back(make_bpair(terrainModelId, clipId));
            }
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                 07/17
+---------------+---------------+---------------+---------------+---------------+------*/
//ConvertToDgnDbElementExtension::Result ConvertScalableMeshAttachment::_PreConvertElement(DgnV8EhCR v8el, Converter& converter, ResolvedModelMapping const& v8mm)
void Convert(DgnV8EhCR v8el)
    {
/*
    bmap<uint64_t, bpair<ScalableMesh::SMNonDestructiveClipType, bvector<DPoint3d>>> clipDefs;

    BentleyStatus clipInfoStatus(ScalableMeshAttachment::ExtractClipDefinitionsInfo(&clipDefs, v8el));

    assert(clipInfoStatus == SUCCESS);
*/
/*
    bmap <uint64_t, SMModelClipInfo> clipInfoMap;

    for (auto& clipDefIter : clipDefs)
        { 
        SMModelClipInfo clipInfo;

        clipInfo.m_shape.resize(clipDefIter->second->second.size());
        memcpy(&clipInfo.m_shape[0], &clipDefIter->second->second[0], clipInfo.m_shape.size() * sizeof(DPoint3d));
        clipInfo.m_type = clipDefIter->second->first;
       
        clipInfoMap.insert(std::pair<uint64_t, SMModelClipInfo>(clipDefIter->first, clipInfo));
        }
    
    ((ScalableMeshModel*)spatialModel)->SetScalableClips(clipInfoMap);

    return Result::SkipElement;
*/
    }
