/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//#include "ConverterInternal.h"
#include <DgnDbSync/DgnV8/DgnV8.h>
#define VANCOUVER_API
#include <VersionedDgnV8Api/ScalableMesh/ScalableMeshLib.h>

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
#ifdef FROMELEMREF

struct  SMHostv8 : Bentley::ScalableMesh::ScalableMeshLib::Host
    {
    SMHostv8()
        {
        }

    Bentley::ScalableMesh::ScalableMeshAdmin& _SupplyScalableMeshAdmin()
        {
        struct CsScalableMeshAdmin : public Bentley::ScalableMesh::ScalableMeshAdmin
            {
            //virtual IScalableMeshTextureGeneratorPtr _GetTextureGenerator() override
            //    {
            //    IScalableMeshTextureGeneratorPtr generator;
            //    return generator;
            //    }

            virtual bool _CanImportPODfile() const override
                {
                return false;
                }

            virtual Utf8String _GetProjectID() const override
                {             
                Utf8String projectGUID("95b8160c-8df9-437b-a9bf-22ad01fecc6b");

                Bentley::WString projectGUIDw;

                if (BSISUCCESS == DgnV8Api::ConfigurationManager::GetVariable(projectGUIDw, L"SM_PROJECT_GUID"))
                    {
                    projectGUID.Assign(projectGUIDw.c_str());
                    }
                return projectGUID;
                }

            virtual uint64_t _GetProductId() const override
                {
                return 1; //Default product Id for internal product
                }            

            virtual Bentley::ScalableMesh::ScalableMeshAdmin::ProductInfo _GetProductInfo() const override
                {
                Bentley::ScalableMesh::ScalableMeshAdmin::ProductInfo productInfo;
                productInfo.m_productName = L"DgnV8Converter";
                return productInfo;
                }
            };
        return *new CsScalableMeshAdmin;
        };

    };

BentleyStatus  ConvertDTMElementRefTo3SM(DgnV8EhCR v8el, WCharCP smFile)
    {
    StatusInt status;
    Bentley::ScalableMesh::ScalableMeshLib::Initialize(*new SMHostv8());
    auto scalableMeshCreatorPtr = Bentley::ScalableMesh::IScalableMeshSourceCreator::GetFor(smFile, status);

    auto sourceP = Bentley::ScalableMesh::IDTMDgnTerrainModelSource::Create(v8el.GetElementRef());
    scalableMeshCreatorPtr->EditSources().Add(sourceP);
    scalableMeshCreatorPtr->Create();
    scalableMeshCreatorPtr->SaveToFile();
    scalableMeshCreatorPtr = nullptr;
    return SUCCESS;
    }
#endif