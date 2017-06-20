/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/ImportAttribute.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DGNDBSYNC_DWG


static Utf8CP   s_blankTagName = "BLANKTAG";


//=======================================================================================
// @bsiclass                                                    Keith.Bentley   05/15
//=======================================================================================
struct AspectInserter : DgnElement::AppData
    {
    friend struct DwgImporter;

private:
    static Key      s_key;
    DwgImporter&    m_importer;
    bvector<ECN::IECInstancePtr> m_aspectInstances;

    DropMe _OnInserted (DgnElementCR el) override;
    DropMe _OnUpdated (DgnElementCR modified, DgnElementCR original, bool isOriginal) override;

    void Insert (DgnElementCR, ECN::IECInstanceR aspectInstance);
    void UpdateOrInsert (DgnElementCR, ECN::IECInstanceR aspectInstance);
    BentleyStatus TryUpdate (DgnElementCR, ECN::IECInstanceCR aspectInstance);

public:
    explicit AspectInserter(DwgImporter& in) : m_importer(in) {}

    void AddAspectInstance(ECN::IECInstancePtr& aspectInstance) { m_aspectInstances.push_back(aspectInstance); }

    static Key& GetKey() { return s_key; }
    };  // AspectInserter

// static value
AspectInserter::Key     AspectInserter::s_key;

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     05/2015
//---------------------------------------------------------------------------------------
AspectInserter::DropMe AspectInserter::_OnInserted (DgnElementCR el)
    {
    for (IECInstancePtr& aspectInstance : m_aspectInstances)
        {
        Insert(el, *aspectInstance);
        }

    return DropMe::Yes;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     05/2015
//---------------------------------------------------------------------------------------
AspectInserter::DropMe AspectInserter::_OnUpdated (DgnElementCR modified, DgnElementCR original, bool isOriginal)
    {
    for (IECInstancePtr& aspectInstance : m_aspectInstances)
        UpdateOrInsert(modified, *aspectInstance);

    return DropMe::Yes;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Sam.Wilson     03/2015
//---------------------------------------------------------------------------------------
void AspectInserter::Insert (DgnElementCR el, IECInstanceR aspectInstance)
    {
    ECInstanceInserter const&   inserter = m_importer.GetECSqlCache().GetInserter(aspectInstance.GetClass());
    if (!inserter.IsValid())
        return;

    DgnElementId elementId = el.GetElementId();
    BeAssert(elementId.IsValid());
    ECInstanceId aspectId(elementId.GetValue());
    bool autogenerateECInstanceId = false;
    ECInstanceId* id = &aspectId;

    ECPropertyCP elementIdProp = aspectInstance.GetClass().GetPropertyP("Element");
    BeAssert(elementIdProp != nullptr && elementIdProp->GetIsNavigation());
    //WIP_NAV_PROP Enhance this to pass the correct subclass of the ElementOwnsMultiAspect relationship
    ECValue elementIdVal(aspectId, elementIdProp->GetAsNavigationProperty()->GetRelationshipClass());
    ECObjectsStatus ecstat = aspectInstance.SetValue("Element", elementIdVal);
    if (ECObjectsStatus::Success != ecstat)
        {
        Utf8String error;
        error.Sprintf("Could not set Element ECProperty in ElementMultiAspect ECInstance (ECClass: %s). ECObjectsStatus code: %d",
                      aspectInstance.GetClass().GetFullName(), (int) ecstat);
        m_importer.ReportIssue(DwgImporter::IssueSeverity::Fatal, DwgImporter::IssueCategory::Sync(), DwgImporter::Issue::Error(), error.c_str());
        BeAssert(false && "Could not set Element ECProperty in ElementMultiAspect ECInstance");
        return;
        }

    //if aspect is an item, use the id of the element. Otherwise let ECDb generate a new one.
    autogenerateECInstanceId = true;
    id = nullptr;

    ECInstanceKey aspectKey;
    if (BE_SQLITE_OK == inserter.Insert(aspectKey, aspectInstance, autogenerateECInstanceId, id))
        {
        //Set instance id on the aspect instance in case caller needs the information
        Utf8Char idStrBuffer[BeInt64Id::ID_STRINGBUFFER_LENGTH];
        aspectKey.GetInstanceId().ToString(idStrBuffer);
        aspectInstance.SetInstanceId(idStrBuffer);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Sam.Wilson     03/2015
//---------------------------------------------------------------------------------------
void AspectInserter::UpdateOrInsert (DgnElementCR el, IECInstanceR aspectInstance)
    {
    if (SUCCESS != TryUpdate(el, aspectInstance))
        Insert(el, aspectInstance);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Sam.Wilson     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus AspectInserter::TryUpdate (DgnElementCR el, IECInstanceCR aspectInstance)
    {
    if (aspectInstance.GetInstanceId().empty())
        return BSIERROR;

    ECInstanceUpdater const&    updater = m_importer.GetECSqlCache().GetUpdater(aspectInstance.GetClass());
    if (!updater.IsValid())
        return BSIERROR;

    return BE_SQLITE_OK == updater.Update(aspectInstance) ? BSISUCCESS : BSIERROR;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   02/2015
//---------------------------------------------------------------------------------------
DwgImporter::ECSqlCache const&  DwgImporter::GetECSqlCache () const
    {
    return ECSqlCache::GetCache(*const_cast<DwgImporter*>(this));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   02/2015
//---------------------------------------------------------------------------------------
DwgImporter::ECSqlCache::~ECSqlCache ()
    {
    //we cannot use unique_ptr as the cache is public API, so we need to free the inserters/updaters manually
    for (auto const& kvPair : m_inserterCache)
        delete kvPair.second;

    for (auto const& kvPair : m_updaterCache)
        delete kvPair.second;

    m_inserterCache.empty();
    m_updaterCache.empty();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
DwgImporter::ECSqlCache const&  DwgImporter::ECSqlCache::GetCache (DwgImporter& importer)
    {
    static const BeSQLite::Db::AppData::Key s_appDataKey;

    DgnDbR dgndb = importer.GetDgnDb();
    Db::AppData* appData = dgndb.FindAppData(s_appDataKey);
    if (nullptr == appData)
        {
        auto cache = new DwgImporter::ECSqlCache(importer);
        dgndb.AddAppData(s_appDataKey, cache);
        return *cache;
        }

    BeAssert(dynamic_cast<DwgImporter::ECSqlCache*>(appData) != nullptr);
    return *static_cast<DwgImporter::ECSqlCache*>(appData);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
ECInstanceInserter const&   DwgImporter::ECSqlCache::GetInserter (ECClassCR ecClass) const
    {
    auto it = m_inserterCache.find(&ecClass);
    if (it == m_inserterCache.end())
        {
        auto inserter = new ECInstanceInserter(m_importer.GetDgnDb(), ecClass, nullptr);

        //just log error, we will still return the invalid inserter, and the caller will check for validity again
        if (!inserter->IsValid())
            {
            Utf8String error;
            error.Sprintf("Could not create ECInstanceInserter for ECClass '%s'. No instances of that class will be imported into the DgnDb file. Please see ECDb entries in log file for details.",
                          Utf8String(ecClass.GetFullName()).c_str());
            m_importer.ReportError(DwgImporter::IssueCategory::Sync(), DwgImporter::Issue::Error(), error.c_str());
            }

        m_inserterCache[&ecClass] = inserter;
        return *inserter;
        }

    return *it->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
ECInstanceUpdater const&    DwgImporter::ECSqlCache::GetUpdater (ECClassCR ecClass) const
    {
    auto it = m_updaterCache.find(&ecClass);
    if (it == m_updaterCache.end())
        {
        bvector<ECPropertyCP> propertiesToBind;
        for (ECPropertyCP ecProperty : ecClass.GetProperties(true))
            {
            // Don't bind any of the dgn derived properties
            if (ecProperty->GetClass().GetSchema().GetName().Equals("dgn"))
                continue;
            if (ecProperty->IsCalculated())
                continue;
            propertiesToBind.push_back(ecProperty);
            }

        auto updater = new BeSQLite::EC::ECInstanceUpdater(m_importer.GetDgnDb(), ecClass, nullptr, propertiesToBind, "ReadonlyPropertiesAreUpdatable");

        //just log error, we will still return the invalid inserter, and the caller will check for validity again
        if (!updater->IsValid() && propertiesToBind.size() > 0)
            {
            Utf8String error;
            error.Sprintf("Could not create ECInstanceUpdater for ECClass '%s'. No instances of that class will be updated in the DgnDb file. Please see ECDb entries in log file for details.",
                          Utf8String(ecClass.GetFullName()).c_str());
            m_importer.ReportError(DwgImporter::IssueCategory::Sync(), DwgImporter::Issue::Error(), error.c_str());
            }

        m_updaterCache[&ecClass] = updater;
        return *updater;
        }

    return *it->second;
    }

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

    bool        propOrAdhoc;

    if (ECObjectsStatus::Success == status)
        {
        // has attrdef - set the property
        ECValue     value (valueString.c_str());
        status = m_ecInstance->SetValue (propIndex, value);

        propOrAdhoc = true;
        m_propertyCount++;
        }
    else
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
            // 1 - create an PhysicalElement displaying a visible attribute:
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

    // create DgnElements from variable attributes
    BentleyStatus   status = this->ProcessVariableAttributes (iter);

    // if a block has no attrdef at all, which would have resulted in no ECInstance created, we are done:
    if (!hasEcInstance)
        return  status;

    Transform   blockRefMatrix;
    blockReference.GetBlockTransform (blockRefMatrix);
    
    // create DgnElement's from constant attributes, if any
    this->ProcessConstantAttributes (blockId, blockReference.GetObjectId(), blockRefMatrix);

    // create an inserter such that the ElementAspect's created at above step will be post inserted along with the host element:
    if (this->GetPropertyCount() > 0)
        {
        AspectInserter* inserter = new AspectInserter (m_importer);

        m_hostElement.AddAppData (AspectInserter::GetKey(), inserter);
        inserter->AddAspectInstance (m_ecInstance);
        }

    return  this->GetTotalCount() > 0 ? BSISUCCESS : BSIERROR;
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
        ECClassCP   multiAspect = this->GetDgnDb().Schemas().GetClass (GENERIC_DOMAIN_NAME, GENERIC_CLASS_MultiAspect);
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

