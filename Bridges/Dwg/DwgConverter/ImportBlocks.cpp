/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG

DWG_PROTOCOLEXT_DEFINE_MEMBERS(DwgBlockReferenceExt)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgImporter::SharedPartKey::SharedPartKey (DwgDbBlockReferenceCR insert, double scale) 
    {
    // build a SharedPartKey value by hashing blockId, layerId, ByBlock symbology and basePartScale
    MD5 hasher;
    auto v = insert.GetBlockTableRecordId().ToUInt64 ();
    hasher.Add (&v, sizeof(v));
    v = insert.GetLayerId().ToUInt64 ();
    hasher.Add (&v, sizeof(v));
    v = insert.GetLinetypeId().ToUInt64 ();
    hasher.Add (&v, sizeof(v));
    v = insert.GetEntityColor().GetMRGB ();
    hasher.Add (&v, sizeof(v));
    v = static_cast<uint64_t>(insert.GetLineweight());
    hasher.Add (&v, sizeof(v));
    hasher.Add (&scale, sizeof(scale));

    m_keyValue = hasher.GetHashVal ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgImporter::SharedPartKey::SharedPartKey ()
    {
    ::memset (m_keyValue.m_buffer, 0, MD5::HashBytes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool DwgImporter::SharedPartKey::IsValid () const
    {
    for (int i = 0; i < MD5::HashBytes; i++)
        {
        if (m_keyValue.m_buffer[i] != 0)
            return  true;
        }
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool DwgImporter::SharedPartKey::operator < (SharedPartKey const& rho) const
    {
    BeAssert (this->IsValid() && rho.IsValid() && "Uninitialized SharePartKey!");
    auto res = ::memcmp(m_keyValue.m_buffer, rho.GetKeyValue().m_buffer, MD5::HashBytes);
    return res < 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportBlockReference (ElementImportResults& results, ElementImportInputs& inputs)
    {
    BentleyStatus   status = BSIERROR;
    DwgDbBlockReferenceCP   insert = DwgDbBlockReference::Cast(inputs.GetEntityP());
    if (nullptr == insert)
        {
        BeAssert (false && "Not a DwgDbBlockReference!");
        return  status;
        }

    /*-----------------------------------------------------------------------------------
    We attempt to use shared parts for a block, but we first have to check:
    a) if the block transform is valid for shared parts, and
    b) if the block has been been previously imported as shared parts.
    -----------------------------------------------------------------------------------*/
    auto found = m_blockPartsMap.end ();

    // get the transform of the insert entity:
    auto blockTrans = Transform::FromIdentity ();
    insert->GetBlockTransform (blockTrans);

    // attempt to create shared parts from a block only if that is what the user wants:
    double  partScale = 0.0;
    bool    canShareParts = this->GetOptions().IsBlockAsSharedParts ();
    if (canShareParts)
        {
        auto blockToModel = Transform::FromProduct (inputs.GetTransform(), blockTrans);
        canShareParts = DwgHelper::GetTransformForSharedParts (nullptr, &partScale, blockToModel);

        // in a 2D model, a mirrored block needs a separate code value, VSTS 84042.
        if (canShareParts && inputs.GetTargetModel().Is2dModel())
            DwgHelper::NegateScaleForSharedParts (partScale, blockTrans);
        }

    if (canShareParts)
        {
        // user wants shared parts, and block transform is valid for shared parts, now search the parts cache:
        SharedPartKey key(*insert, partScale);
        found = m_blockPartsMap.find (key);
        }

    if (found == m_blockPartsMap.end())
        {
        // either can't share parts or parts cache not found - import the new insert entity from cratch:
        status = this->_ImportEntity (results, inputs);
        }
    else
        {
        // found parts from the cache - create elements from them:
        ElementCreateParams  params(inputs.GetTargetModelR());
        status = this->_GetElementCreateParams (params, inputs.GetTransform(), inputs.GetEntity());
        if (BSISUCCESS != status)
            return  status;

        ElementFactory  elemFactory(results, inputs, params, *this);
        elemFactory.SetCreateSharedParts (true);
        elemFactory.SetBaseTransform (blockTrans);

        // we should have avoided a transform not supported by shared parts
        BeAssert (elemFactory.CanCreateSharedParts() && "Shared parts cannot be created for this insert!!");

        // create elements from the block-parts cache
        status = elemFactory.CreatePartElements (found->second);
        }
    if (status != BSISUCCESS)
        return  status;

    // get imported DgnElement
    DgnElementP     hostElement = nullptr;
    if (BSISUCCESS == status)
        hostElement = results.GetImportedElement ();

    if (nullptr == hostElement)
        return  BSIERROR;

    // create elements from attributes attached to the block reference:
    AttributeFactory        attribFactory(*this, *hostElement, results, inputs);
    attribFactory.CreateElements (*insert);

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgBlockReferenceExt::_ConvertToBim (ProtocolExtensionContext& context, DwgImporter& importer)
    {
    DwgDbBlockReferenceP    insert = DwgDbBlockReference::Cast (context.GetEntityPtrR().get());
    if (nullptr == insert)
        return  BentleyStatus::BSIERROR;

    ElementInputsR  inputs = context.GetElementInputsR ();
    ElementResultsR results = context.GetElementResultsR ();

    return insert->IsXAttachment() ? importer._ImportXReference(results, inputs) : importer._ImportBlockReference(results, inputs);
    }
