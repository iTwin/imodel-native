/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/ImportAttribute.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_DGNDBSYNC_DWG


static Utf8CP   s_blankTagName = "BLANKTAG";


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
AttributeFactory::AttributeFactory (DwgImporter& importer, DgnElementR hostElement, DwgImporter::ElementImportResults& results, DwgImporter::ElementImportInputs& inputs) : 
    m_importer(importer), m_hostElement(hostElement), m_outputResults(results), m_inputContext(inputs)
    {
    m_propertyCount = 0;
    m_adhocCount = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            AttributeFactory::AddPropertyOrAdhocFromAttribute (DwgDbAttributeCR attrib)
    {
    // extrat tag name from attribute, replacing blank name with "BLANKTAG":
    Utf8String  tag (DwgHelper::ToUtf8CP(attrib.GetTag(), true));
    if (tag.empty())
        tag.assign (s_blankTagName);

    // build an internal EC property name from the tag name
    Utf8String  propName;
    ECNameValidation::EncodeToValidName (propName, tag.c_str());

    Utf8String  valueString;
    DwgString   text;
    // extract attribute value, single or multiline text:
    if (attrib.GetValueString(text))
        valueString.Assign (text.c_str());

    // if an attrdef exists, get its ECClass imported:
    uint32_t        propIndex = 0;
    ECObjectsStatus status = ECObjectsStatus::ClassNotFound;
    if (m_ecInstance.IsValid())
        status = m_ecInstance->GetEnabler().GetPropertyIndex (propIndex, propName.c_str());

    bool    propOrAdhoc = false;

    if (ECObjectsStatus::Success == status)
        {
        // has attrdef - set the property
        ECValue     value (valueString.c_str());
        status = m_ecInstance->SetValue (propIndex, value);

        propOrAdhoc = true;
        m_propertyCount++;
        }

    if (ECObjectsStatus::Success != status)
        {
        // missing attrdef - create an Adhoc property
        uint32_t    count = 0;
        while (!m_hostElement.GetUserProperties(tag.c_str()).isNull())
            {
            Utf8PrintfString    tryName("%s%d", tag.c_str(), ++count);
            tag.assign (tryName.c_str());
            }

        AdHocJsonValue  adhocProp;
        adhocProp.SetValueText (tag.c_str(), valueString.c_str());
        m_hostElement.SetUserProperties (tag.c_str(), adhocProp);

        propOrAdhoc = false;
        m_adhocCount++;
        }

    return  propOrAdhoc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AttributeFactory::AddConstantProperty (DwgDbAttributeDefinitionCR attrdef)
    {
    Utf8String  tag (attrdef.GetTag().c_str());
    Utf8String  valueString;
    DwgString   text;
    if (attrdef.GetValueString(text))
        valueString.Assign (text.c_str());

    uint32_t        propIndex = 0;
    ECObjectsStatus status = m_ecInstance->GetEnabler().GetPropertyIndex (propIndex, tag.c_str());

    if (ECObjectsStatus::Success == status)
        {
        // has attrdef - set the property
        ECValue     value (valueString.c_str());
        status = m_ecInstance->SetValue (propIndex, value);

        m_propertyCount++;
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AttributeFactory::CreateECInstance (DwgDbObjectIdCR blockId)
    {
    // create an ECInstance of an attribute definition class:
    if (!blockId.IsValid())
        return  BSIERROR;

    ECSchemaCP  attrdefSchema = m_importer.GetAttributeDefinitionSchema ();
    if (nullptr == attrdefSchema)
        return  BSIERROR;

    DwgDbBlockTableRecordPtr    block(blockId, DwgDbOpenMode::ForRead);
    if (block.IsNull())
        return  BSIERROR;
        
    // get the attrdef ECClass
    Utf8String  className = DwgHelper::GetAttrdefECClassNameFromBlockName (block->GetName().c_str());
    ECClassCP   attrdefClass = attrdefSchema->GetClassCP (className.c_str());
    if (nullptr == attrdefClass)
        return  BSIERROR;

    // instantiate the class
    m_ecInstance = attrdefClass->GetDefaultStandaloneEnabler()->CreateInstance ();

    return  m_ecInstance.IsValid() ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AttributeFactory::ProcessVariableAttributes (DwgDbObjectIterator& attrIter)
    {
    BentleyStatus   status = BSISUCCESS;

    DwgImporter::ElementImportInputs    childInputs(*m_hostElement.GetModel());
    childInputs.SetClassId (m_hostElement.GetElementClassId());
    childInputs.SetTransform (m_inputContext.GetTransform());
    childInputs.SetModelMapping (m_inputContext.GetModelMapping());
    childInputs.SetParentEntity (m_inputContext.GetEntityPtrR().get());

    // walk through variable attributes: create a visible text and an ElementAspect from each attribute
    for (attrIter.Start(); !attrIter.Done(); attrIter.Next())
        {
        DwgDbAttributeCP    attrib = nullptr;

        childInputs.m_entity.OpenObject (attrIter.GetObjectId(), DwgDbOpenMode::ForRead);

        if (DwgDbStatus::Success == childInputs.m_entity.OpenStatus() && nullptr != (attrib = DwgDbAttribute::Cast(childInputs.GetEntityP())))
            {
            // 1 - create a PhysicalElement displaying the visible attribute:
            DwgString       value;
            if (!attrib->IsInvisible() && attrib->GetValueString(value) && !value.IsEmpty())
                {
                childInputs.SetEntityId (attrib->GetObjectId());

                DwgImporter::ElementImportResults   childResults;
                status = m_importer.ImportEntity (childResults, childInputs);

                // add the new element as a child of the host element, i.e. the insert entity:
                if (BSISUCCESS == status)
                    m_outputResults.AddChildResults (childResults);
                }
            
            // 2 - create either a defined property or an Adhoc prop from the attribute, even if invisible or empty:
            this->AddPropertyOrAdhocFromAttribute (*attrib);
            }
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AttributeFactory::ProcessConstantAttributes (DwgDbObjectIdCR blockId, DwgDbObjectIdCR blockRefId, TransformCR toBlockRef)
    {
    BentleyStatus       status = BSISUCCESS;
    DwgDbObjectIdArray  idArray;
    if (!m_importer.GetConstantAttrdefIdsFor(idArray, blockId))
        return  status;
    
    DwgImporter::ElementImportInputs    childInputs(*m_hostElement.GetModel());
    childInputs.SetClassId (m_hostElement.GetElementClassId());
    childInputs.SetTransform (m_inputContext.GetTransform());
    childInputs.SetModelMapping (m_inputContext.GetModelMapping());
    childInputs.SetParentEntity (m_inputContext.GetEntityPtrR().get());

    // walk through variable attributes: create a visible text and an ElementAspect from each attribute
    for (auto& id : idArray)
        {
        DwgDbAttributeDefinitionPtr attrdef(id, DwgDbOpenMode::ForRead);
        if (DwgDbStatus::Success == attrdef.OpenStatus())
            {
            DwgDbAttributeP attrib = nullptr;
            DwgString   defaultValue;

            // 1 - create an PhysicalElement displaying a visible attribute:
            if (!attrdef->IsInvisible() && attrdef->GetValueString(defaultValue) && !defaultValue.IsEmpty() && nullptr != (attrib = DwgDbAttribute::StaticCreateObject()))
                {
                attrib->SetFrom (attrdef.get(), toBlockRef);

                DwgDbEntityP    entity = DwgDbEntity::Cast (attrib);
                childInputs.m_entity.AcquireObject (entity);

                // make an element code "attrdefID[blockRefId]":
                Utf8PrintfString                    elementCode("%llx[%llx]", attrdef->GetObjectId().ToUInt64(), blockRefId.ToUInt64());
                DwgImporter::ElementImportResults   childResults;

                status = m_importer.ImportNewEntity (childResults, childInputs, blockId, elementCode);

                // add the new element as a child of the host element, i.e. the insert entity:
                if (BSISUCCESS == status)
                    m_outputResults.AddChildResults (childResults);
                }

            // 2 - create a property from attrdef, even if invisible or empty:
            this->AddConstantProperty (*attrdef);
            }
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AttributeFactory::CreateElements (DwgDbBlockReferenceCR blockReference)
    {
    DwgDbObjectIterator     iter = blockReference.GetAttributeIterator ();
    if (!iter.IsValid())
        return  BSISUCCESS;

    DwgDbObjectId   blockId = blockReference.GetBlockTableRecordId ();
    bool            hasEcInstance = BSISUCCESS == this->CreateECInstance(blockId);

    // create DgnElements and collect ElementAspects from variable attributes
    BentleyStatus   status = this->ProcessVariableAttributes (iter);

    // if a block has no attrdef at all, which would have resulted in no ECInstance created, we are done:
    if (!hasEcInstance)
        return  status;

    Transform   blockRefMatrix;
    blockReference.GetBlockTransform (blockRefMatrix);
    
    // create DgnElement's and collect ElementAspects from constant attributes, if any
    this->ProcessConstantAttributes (blockId, blockReference.GetObjectId(), blockRefMatrix);

    // schedule a GenericMultiAspect to be inserted or updated along with the host element:
    status = BSIERROR;
    if (this->GetPropertyCount() > 0)
        {
        if (DgnElement::GenericMultiAspect::AddAspect(m_hostElement, *m_ecInstance.get()) == DgnDbStatus::Success)
            {
            m_importer.AddPresentationRuleContent (m_hostElement, m_ecInstance->GetClass().GetName());
            status = BSISUCCESS;
            }
        }

    return  status;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus DwgImporter::AddAttrdefECClassFromBlock (ECSchemaPtr& attrdefSchema, DwgDbBlockTableRecordCR block)
    {
    ECObjectsStatus status = ECObjectsStatus::Success;

    // create attredef schema if not already created:
    if (attrdefSchema.IsNull())
        {
        ECObjectsStatus status = ECSchema::CreateSchema (attrdefSchema, SCHEMAName_AttributeDefinitions, SCHEMAAlias_AttributeDefinitions, 1, 0, 0);
        if (ECObjectsStatus::Success == status)
            {
            // set label
            status = attrdefSchema->SetDisplayLabel (SCHEMALabel_AttributeDefinitions);
            // set description
            Utf8PrintfString    description (DataStrings::GetString(DataStrings::AttrdefsSchemaDescription()).c_str(), m_rootFileName.GetFileNameAndExtension().c_str());
            status = attrdefSchema->SetDescription (description.c_str());

            // Reference schema Generic
            ECSchemaCP  refSchema = this->GetDgnDb().Schemas().GetSchema (GENERIC_DOMAIN_NAME);
            if (nullptr != refSchema)
                status = attrdefSchema->AddReferencedSchema (const_cast<ECSchemaR>(*refSchema));
            else
                BeAssert (false && "Failed finding schema: Generic!");

            // Reference schema Dgn
            if (nullptr != (refSchema = this->GetDgnDb().Schemas().GetSchema(BIS_ECSCHEMA_NAME)))
                status = attrdefSchema->AddReferencedSchema (const_cast<ECSchemaR>(*refSchema));
            else
                BeAssert (false && "Failed finding schema: Dgn!");
            }
        }
    if (ECObjectsStatus::Success != status)
        return  status;

    // add an attrdef class for the block
    ECEntityClassP  attrdefClass = nullptr;
    Utf8String      className = DwgHelper::GetAttrdefECClassNameFromBlockName (block.GetName().c_str());

    status = attrdefSchema->CreateEntityClass (attrdefClass, className.c_str());

    if (ECObjectsStatus::Success == status)
        {
        // display the class as the block name:
        attrdefClass->SetDisplayLabel (DwgHelper::ToUtf8CP(block.GetName()));
        // description of the class
        Utf8PrintfString    description (DataStrings::GetString(DataStrings::BlockAttrdefDescription()).c_str(), block.GetName().c_str());
        attrdefClass->SetDescription (description);

        // add GenericMultiAspect as a base ECClass:
        ECClassCP   multiAspect = this->GetDgnDb().Schemas().GetClass (BIS_ECSCHEMA_NAME, BIS_CLASS_ElementMultiAspect);
        if (nullptr != multiAspect)
            {
            attrdefClass->AddBaseClass (*multiAspect);
            }
        else
            {
            BeAssert (false && "Failed adding ElementMultiAspect as the base class of attrdef class!");
            PrimitiveECPropertyP    idProp = nullptr;
            status = attrdefClass->CreatePrimitiveProperty (idProp, "Element", PRIMITIVETYPE_Long);
            }

        DwgDbBlockChildIterator     entityIter = block.GetBlockChildIterator ();
        if (!entityIter.IsValid())
            return  ECObjectsStatus::Error;

        ConstantBlockAttrdefs       constantAttrdefs(block.GetObjectId());

        // get all attrdefs in the block and create a string property for each and everyone of them:
        for (entityIter.Start(); !entityIter.Done(); entityIter.Step())
            {
            DwgDbAttributeDefinitionPtr attrdef(entityIter.GetEntityId(), DwgDbOpenMode::ForRead);
            if (attrdef.IsNull())
                continue;

            // replace blank tag name with "BLANKTAG"
            Utf8String  tagName (DwgHelper::ToUtf8CP(attrdef->GetTag(), true));
            if (tagName.empty())
                tagName.assign (s_blankTagName);

            PrimitiveECPropertyP    prop = nullptr;
            Utf8String              propName;

            // build an internal EC property name from attrdef tag
            ECNameValidation::EncodeToValidName (propName, tagName.c_str());

            // Property tag
            status = attrdefClass->CreatePrimitiveProperty (prop, propName.c_str(), PRIMITIVETYPE_String);
            if (ECObjectsStatus::Success == status)
                {
                prop->SetDisplayLabel (propName.c_str());

                if (attrdef->IsConstant())
                    {
                    // set read-only for a constant attrdef:
                    prop->SetIsReadOnly (true);
                    // save this attrdef entity for future use
                    constantAttrdefs.Add (attrdef->GetObjectId());
                    }
                }
            }

        // cache constant attrdefs for this block:
        if (constantAttrdefs.GetCount() > 0)
            m_constantBlockAttrdefList.push_back (constantAttrdefs);
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgImporter::GetConstantAttrdefIdsFor (DwgDbObjectIdArray& ids, DwgDbObjectIdCR blockId)
    {
    if (m_constantBlockAttrdefList.empty())
        return  false;
    
    struct FindAttrdefsPredicate
        {
        DwgDbObjectId   m_blockId;
        FindAttrdefsPredicate (DwgDbObjectIdCR blockId) : m_blockId (blockId) {}
        bool operator () (ConstantBlockAttrdefs const& ca) { return m_blockId == ca.GetBlockId(); }
        };

    FindAttrdefsPredicate   pred(blockId);

    auto found = std::find_if (m_constantBlockAttrdefList.begin(), m_constantBlockAttrdefList.end(), pred);
    if (found != m_constantBlockAttrdefList.end())
        {
        ids = found->GetAttrdefIdArrayR();
        return  !ids.empty();
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::AddPresentationRuleContent (DgnElementCR hostElement, Utf8StringCR attrdefName)
    {
    /*-----------------------------------------------------------------------------------
    Cache PresentationRules applied in the modelspace and/or paperspaces.
    We do this because the same attrdef may be used in the modelspace and/or a paperspace.
    A class applied in both models will end up with two entries, one Generic:PhysicalObject
    for the modelspace and another BisCore::DrawingGraphic for the paperspaces.
    -----------------------------------------------------------------------------------*/
    ECClassCP   elementClass = hostElement.GetElementClass ();
    if (nullptr == elementClass)
        return  BentleyStatus::SUCCESS;

    PresentationRuleContent content(attrdefName, elementClass->GetName(),  elementClass->GetSchema().GetName());

    auto found = std::find_if (m_presentationRuleContents.begin(), m_presentationRuleContents.end(), [&](PresentationRuleContent const& c){return c == content;});
    if (found == m_presentationRuleContents.end())
        m_presentationRuleContents.push_back (content);

    return  BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_EmbedPresentationRules ()
    {
    // PresentationRuleSet seems now required for ElementAspects.
    if (m_presentationRuleContents.empty())
        return  BentleyStatus::SUCCESS;

    // db file name
    Utf8String fileName(this->GetDgnDb().GetFileName().GetFileNameWithoutExtension().c_str());
    // supported schemas
    Utf8PrintfString supported("%s,Generic", SCHEMAName_AttributeDefinitions);
    // DwgAttributeDefinition schema specific
    Utf8PrintfString purpose("%s specific", SCHEMAName_AttributeDefinitions);
    
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance (fileName, 1, 0, true, purpose, supported, "", false);
    if (!ruleset.IsValid())
        return  BSIERROR;

    // BisCore:ElementOwnsMultiAspects
    Utf8PrintfString    multiAspect ("%s:%s", BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsMultiAspects);

    for (auto content : m_presentationRuleContents)
        {
        // elements may be in the modelspace (Generic:PhysicalObject), or in a paperspace (BsiCore::DrawingGraphic)
        ContentModifierP    modifier = new ContentModifier (content.GetHostElementSchema(), content.GetHostElementClass());
        if (nullptr == modifier)
            return  BSIERROR;

        ruleset->AddPresentationRule (*modifier);

        // relating property DwgAttributeDefinition:ECClass name from the attrdef tag:
        Utf8PrintfString    related("%s:%s", SCHEMAName_AttributeDefinitions, content.GetAttrdefClass());
        RelatedPropertiesSpecificationP prop = new RelatedPropertiesSpecification (RequiredRelationDirection_Forward, multiAspect, related, "", RelationshipMeaning::SameInstance);
        if (nullptr == prop)
            return  BSIERROR;

        modifier->AddRelatedProperty (*prop);
        }

    RuleSetEmbedder embedder(this->GetDgnDb());
    BeSQLite::DbResult  status = embedder.Embed (*ruleset);

    return  static_cast<BentleyStatus>(status);
    }

