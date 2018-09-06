/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/Samples/XData/ImportXData.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "ImportXData.h"

//---------------------------------------------------------------------------------------
//  This is a helper macro that adds a unique Adhoc property in AdhocPropertiesBuilder(below):
//
//  1. Create an Adhoc property name, "XDxx", where xx is a 2-digit count of total entries
//  2. Get AdHocJsonPropertyValue by name, from the input DgnElement
//  3. Set the Adhoc value by type
//  4. Increase the property counter by 1 for next property
//
//---------------------------------------------------------------------------------------
#define ADDADHOCPROPERTY(_Type_, _Value_)                                               \
    Utf8PrintfString    uniqueName("XD%02d", m_count++);                                \
    AdHocJsonValue      adhocProp;                                                      \
    adhocProp.SetValue##_Type_ (#_Type_, _Value_);                                      \
    m_element.SetUserProperties (uniqueName.c_str(), adhocProp);


BEGIN_DGNDBSYNC_DWG_NAMESPACE

//=======================================================================================
// A helper class that adds an Adhoc property at a time to an input DgnElement. It tracks
// number of the properties added to ensure unique property names being added on the same 
// element.
//
// @bsiclass
//=======================================================================================
struct  AdhocPropertiesBuilder
    {
private:
    DgnElementR     m_element;
    uint32_t        m_count;

public:
    // Constructor
    AdhocPropertiesBuilder (DgnElementR el) : m_element(el) { m_count = 0; }

    // Add properties by type
    void AddInt (int32_t i)         { ADDADHOCPROPERTY(Int, i) }
    void AddInt64 (int64_t i)       { ADDADHOCPROPERTY(Int64, i) }
    void AddDouble (double d)       { ADDADHOCPROPERTY(Double, d) }
    void AddText (Utf8CP t)         { ADDADHOCPROPERTY(Text, t) }
    void AddPoint3d (DPoint3dCR p)  { ADDADHOCPROPERTY(Point3d, p) }
    };  // AdhocPropertiesBuilder

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus   ImportXData::ConvertXData (DgnElementR element, DwgDbEntityCR entity)
    {
    // Get xdata from the entity - an empty regapp name extracts all xdata
    DwgString           regAppName (WString(m_sampleOptions.GetRegAppName().c_str(), true).c_str());
    DwgResBufIterator   xdata = entity.GetXData (regAppName);

    BentleyStatus       status = BSISUCCESS;

    if (xdata.IsValid())
        {
        LOG.tracev ("Entity %ls has XDATA, adding Adhoc properties to DgnElement...", entity.GetDxfName().c_str());

        AdhocPropertiesBuilder  builder(element);

        // Iterate through all entries, print them and add them to DgnElement as Adhoc properties:
        for (DwgResBufP curr = xdata->Start(); curr != xdata->End(); curr = curr->Next())
            {
            switch (curr->GetDataType())
                {
                case DwgResBuf::DataType::Integer8:
                    LOG.tracev ("XDATA Int8= %d", curr->GetInteger8());
                    builder.AddInt (curr->GetInteger8());
                    break;
                case DwgResBuf::DataType::Integer16:
                    LOG.tracev ("XDATA Int16= %d", curr->GetInteger16());
                    builder.AddInt (curr->GetInteger16());
                    break;
                case DwgResBuf::DataType::Integer32:
                    LOG.tracev ("XDATA Int32= %d", curr->GetInteger32());
                    builder.AddInt (curr->GetInteger32());
                    break;
                case DwgResBuf::DataType::Integer64:
                    LOG.tracev ("XDATA Int64= %I64d", curr->GetInteger64());
                    builder.AddInt64 (curr->GetInteger64());
                    break;
                case DwgResBuf::DataType::Double:
                    LOG.tracev ("XDATA Double= %g", curr->GetDouble());
                    builder.AddDouble (curr->GetDouble());
                    break;
                case DwgResBuf::DataType::Text:
                    LOG.tracev ("XDATA String= %ls", curr->GetString().c_str());
                    builder.AddText (DwgHelper::ToUtf8CP(curr->GetString()));
                    break;
                case DwgResBuf::DataType::BinaryChunk:
                    {
                    // Binary is not currently supported as an Adhoc property - just show size in a text
                    DwgBinaryData   data;
                    if (DwgDbStatus::Success == curr->GetBinaryData(data))
                        {
                        LOG.tracev ("XDATA Binary data size = %lld", data.GetSize());
                        builder.AddText (Utf8PrintfString("binary data in %lld bytes", data.GetSize()).c_str());
                        }
                    else
                        {
                        BeDataAssert(false && "failed extracting binary xdata!");
                        }
                    break;
                    }
                case DwgResBuf::DataType::Handle:
                    LOG.tracev ("XDATA Handle= %ls", curr->GetHandle().AsAscii().c_str());
                    builder.AddText (Utf8PrintfString("Handle=%ls", curr->GetHandle().AsAscii().c_str()).c_str());
                    break;
                case DwgResBuf::DataType::HardOwnershipId:
                case DwgResBuf::DataType::SoftOwnershipId:
                case DwgResBuf::DataType::HardPointerId:
                case DwgResBuf::DataType::SoftPointerId:
                    LOG.tracev ("XDATA ObjectId= %ls", curr->GetObjectId().ToAscii().c_str());
                    builder.AddText (Utf8PrintfString("ObjectID=%ls", curr->GetObjectId().ToAscii().c_str()).c_str());
                    break;
                case DwgResBuf::DataType::Point3d:
                    {
                    DPoint3d    point;
                    if (DwgDbStatus::Success == curr->GetPoint3d(point))
                        {
                        LOG.tracev ("XDATA Point3d= %g, %g, %g", point.x, point.y, point.z);
                        builder.AddPoint3d (point);
                        }
                    else
                        {
                        BeDataAssert (false && "failed extracting Point3d xdata!");
                        }
                    break;
                    }
                case DwgResBuf::DataType::None:
                case DwgResBuf::DataType::NotRecognized:
                default:
                    LOG.warning ("Unexpected XDATA type!!!");
                }
            }
        }

    return  status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus   ImportXData::_ImportEntity (ElementImportResults& results, ElementImportInputs& inputs)
    {
    // Create a DgnElement from DWG entity by default, and get the results back:
    DgnElementP     dgnElement = nullptr;
    BentleyStatus   status = T_Super::_ImportEntity (results, inputs);
    if (BSISUCCESS != status || nullptr == (dgnElement = results.GetImportedElement()))
        {
        LOG.errorv ("Failed creating DgnElement from DWG entity %lld", inputs.GetEntity().GetObjectId().ToUInt64());
        return  status;
        }

    LOG.tracev ("DgnElement %s(ID=%lld) has been created, checking XDATA...", dgnElement->GetDisplayLabel().c_str(), dgnElement->GetElementId());

    // Convert XDATA as Adhoc properties and add them to the new element:
    status = ConvertXData (*dgnElement, inputs.GetEntity());

    if (BSISUCCESS != status)
        LOG.error ("Failed adding xdata to DgnElement as Adhoc properties!");

    return  status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnElementPtr   ImportXData::_ImportGroup (DwgDbGroupCR group)
    {
    // This method creates a GenericGroup in the BisCore's generic group model:
    LOG.tracev ("Importing group %ls in file %ls", group.GetName().c_str(), group.GetDatabase()->GetFileName().c_str());

    GenericGroupPtr     genericGroup;
    DwgDbObjectIdArray  objectIds;
    if (0 == group.GetAllEntityIds(objectIds))
        return  genericGroup;

    // A generic group must be added to a GenericGroupModel:
    auto& db = T_Super::GetDgnDb ();
    auto groupModel = db.Models().Get<GenericGroupModel>(T_Super::GetGroupModelId());
    if (!groupModel.IsValid())
        return  genericGroup;

    // Create a new generic group element and insert it to the db.
    // Currently a GenericGroup requires a valid source, i.e. the group element, ID at the time its memebers are added.
    genericGroup = GenericGroup::Create (*groupModel);
    if (!genericGroup.IsValid() || !genericGroup->Insert().IsValid())
        return  genericGroup;

    // Process each and every member entity found in the DWG group:
    for (auto objectId : objectIds)
        {
        // Find all elements that have been mapped from this object:
        auto memberIds = FindElementsMappedFrom (objectId);

        // Add them all in the same GenericGroup:
        for (auto memberId : memberIds)
            {
            auto element = db.Elements().GetElement (memberId);
            if (element.IsValid())
                genericGroup->AddMember (*element);
            }
        }
    
    auto groupId = genericGroup->GetElementId ();
    auto count = genericGroup->QueryMembers().size ();

    LOG.tracev ("%d group memebers have been added to GenericGroup ID=%d!", count, groupId.GetValue());

    if (0 == count)
        {
        // Delete the empty group from DgnDb:
        T_Super::GetDgnDb().Elements().Delete (groupId);
        genericGroup = nullptr;
        }
    
    return  genericGroup;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ImportXData::_UpdateGroup (DgnElementR dgnGroup, DwgDbGroupCR dwgGroup)
    {
    // This method gets called only in an update job run that creates changeset.
    // When a change has been detected via _OnUpdateGroup, this method is called; otherwise _ImportGroup is called.
    LOG.tracev ("Updating group %ls in file %ls", dwgGroup.GetName().c_str(), dwgGroup.GetDatabase()->GetFileName().c_str());

    // This sample code only creates GenericGroup's:
    auto genericGroup = dynamic_cast<GenericGroupP> (&dgnGroup);
    if (nullptr == genericGroup)
        {
        T_Super::ReportError (IssueCategory::UnexpectedData(), Issue::GroupError(), Utf8PrintfString("existing element is not a GenericGroup for %lls!", dwgGroup.GetName().c_str()).c_str());
        return  BentleyStatus::BSIERROR;
        }

    // Build existing member list from the GenericGroup so we know which members to be removed at the end:
    auto oldMembers = ElementGroupsMembers::QueryMembers (*genericGroup);

    DwgDbObjectIdArray  objectIds;
    if (0 == dwgGroup.GetAllEntityIds(objectIds))
        return  BentleyStatus::BSISUCCESS;

    auto& db = T_Super::GetDgnDb ();

    // Step-1: add new members which are not seen from the sync info, and remove them from the existing list:
    for (auto objectId : objectIds)
        {
        // Find all elements that have been mapped from this object:
        auto newMembers = FindElementsMappedFrom (objectId);

        // Add new or record existing members in GenericGroup:
        for (auto newMember : newMembers)
            {
            // If a member is present in both groups, remove it from the existing list:
            if (oldMembers.find(newMember) != oldMembers.end())
                {
                oldMembers.erase (newMember);
                continue;
                }
            // Otherwise this is a new member which needs to be added to the DgnDb group:
            auto element = db.Elements().GetElement (newMember);
            if (element.IsValid())
                genericGroup->AddMember (*element);
            }
        }

    // Now we are ready to remove members from the DgnDb group: the remaining entries in existing list are the ones to be removed:
    for (auto oldMember : oldMembers)
        {
        auto element = db.Elements().GetElement (oldMember);
        if (element.IsValid())
            genericGroup->RemoveMember (*element);
        }

    return  BentleyStatus::BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet ImportXData::FindElementsMappedFrom (DwgDbObjectIdCR objectId)
    {
    DgnElementIdSet ids;

    // If the member is an xRef insert, find the target DgnModel and get all elements in the model:
    DwgDbBlockReferencePtr  blockInsert(objectId, DwgDbOpenMode::ForRead);
    if (blockInsert.OpenStatus() == DwgDbStatus::Success && blockInsert->IsXAttachment())
        {
        auto modelMap = T_Super::FindModel (objectId, DwgSyncInfo::ModelSourceType::XRefAttachment);

        DgnModelP model = nullptr;
        if (modelMap.IsValid() && nullptr != (model = modelMap.GetModel()))
            ids = model->MakeIterator().BuildIdSet<DgnElementId> ();

        return  ids;
        }

    // Consult the SyncInfo and Find the elements that have been mapped from this object, in the same model:
    if (!T_Super::GetSyncInfo().FindElements(ids, objectId))
        T_Super::ReportError (IssueCategory::UnexpectedData(), Issue::GroupError(), Utf8PrintfString("No DgnElement found for object ID=%llx", objectId.ToUInt64()).c_str());
        
    return  ids;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ImportXData::DumpXrecord (DwgDbXrecordCR xRecord)
    {
    DwgResBufIterator   rbIter = xRecord.GetRbChain ();
    if (!rbIter.IsValid())
        return  BentleyStatus::ERROR;

    // Dump xRecord data into the importer's issue file as an Unsupported category:
    _ReportIssue (IssueSeverity::Info, IssueCategory::Unsupported(), "Dictionary Entry Xrecord", m_xRecordName.c_str());
    for (DwgResBufP curr = rbIter->Start(); curr != rbIter->End(); curr = curr->Next())
        {
        switch (curr->GetDataType())
            {
            case DwgResBuf::DataType::Integer8:
                _ReportIssue (IssueSeverity::Info, IssueCategory::Unsupported(), "Xrecord", Utf8PrintfString("\tInt8= %d", curr->GetInteger8()).c_str());
                break;
            case DwgResBuf::DataType::Integer16:
                _ReportIssue (IssueSeverity::Info, IssueCategory::Unsupported(), "Xrecord", Utf8PrintfString("\tInt16= %d", curr->GetInteger16()).c_str());
                break;
            case DwgResBuf::DataType::Integer32:
                _ReportIssue (IssueSeverity::Info, IssueCategory::Unsupported(), "Xrecord", Utf8PrintfString("\tInt32= %d", curr->GetInteger32()).c_str());
                break;
            case DwgResBuf::DataType::Integer64:
                _ReportIssue (IssueSeverity::Info, IssueCategory::Unsupported(), "Xrecord", Utf8PrintfString("\tInt64= %I64d", curr->GetInteger64()).c_str());
                break;
            case DwgResBuf::DataType::Double:
                _ReportIssue (IssueSeverity::Info, IssueCategory::Unsupported(), "Xrecord", Utf8PrintfString("\tDouble= %g", curr->GetDouble()).c_str());
                break;
            case DwgResBuf::DataType::Text:
                _ReportIssue (IssueSeverity::Info, IssueCategory::Unsupported(), "Xrecord", Utf8PrintfString("\tString= %ls", curr->GetString().c_str()).c_str());
                break;
            case DwgResBuf::DataType::BinaryChunk:
                {
                DwgBinaryData   data;
                if (DwgDbStatus::Success == curr->GetBinaryData(data))
                    _ReportIssue (IssueSeverity::Info, IssueCategory::Unsupported(), "Xrecord", Utf8PrintfString("\tBinary data size = %lld", data.GetSize()).c_str());
                else
                    BeDataAssert(false && "failed extracting binary xdata!");
                break;
                }
            case DwgResBuf::DataType::Handle:
                _ReportIssue (IssueSeverity::Info, IssueCategory::Unsupported(), "Xrecord", Utf8PrintfString("\tEntity Handle= %ls", curr->GetHandle().AsAscii().c_str()).c_str());
                break;
            case DwgResBuf::DataType::HardOwnershipId:
            case DwgResBuf::DataType::SoftOwnershipId:
            case DwgResBuf::DataType::HardPointerId:
            case DwgResBuf::DataType::SoftPointerId:
                _ReportIssue (IssueSeverity::Info, IssueCategory::Unsupported(), "Xrecord", Utf8PrintfString("\tObjectId= %ls", curr->GetObjectId().ToAscii().c_str()).c_str());
                break;
            case DwgResBuf::DataType::Point3d:
                {
                DPoint3d    point;
                if (DwgDbStatus::Success == curr->GetPoint3d(point))
                    _ReportIssue (IssueSeverity::Info, IssueCategory::Unsupported(), "Xrecord", Utf8PrintfString("\tPoint3d= %g, %g, %g", point.x, point.y, point.z).c_str());
                else
                    BeDataAssert (false && "failed extracting Point3d xdata!");
                break;
                }
            case DwgResBuf::DataType::None:
            case DwgResBuf::DataType::NotRecognized:
            default:
                ReportIssue (IssueSeverity::Warning, IssueCategory::UnexpectedData(), Issue::Message(), "\tUnexpected XDATA type!!!");
            }
        }

    m_dumpCount++;

    return  BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ImportXData::DumpDictionaryXrecords (DwgDbDictionaryCR dictionary)
    {
    DwgDbDictionaryIteratorPtr iter = dictionary.GetIterator ();
    if (!iter.IsValid())
        return  BentleyStatus::ERROR;

    // Check all entries of the dictionary:
    for (; !iter->Done(); iter->Next())
        {
        DwgDbDictionaryP    child = nullptr;
        DwgDbXrecordP       xRecord = nullptr;
        DwgDbObjectPtr      entry(iter->GetObjectId(), DwgDbOpenMode::ForRead);

        // Only care about either a child dictionary or an xRecord object:
        if (!entry.IsNull() && (nullptr != (child = DwgDbDictionary::Cast(entry.get())) || nullptr != (xRecord = DwgDbXrecord::Cast(entry.get()))))
            {
            // Push current dictionary name into the xRecord name:
            Utf8String  previousName = m_xRecordName;
            size_t      previousSize = previousName.size ();
            DwgString   entryName = iter->GetName ();
            if (!entryName.IsEmpty())
                {
                if (m_xRecordName.empty())
                    m_xRecordName = Utf8String(entryName);
                else
                    m_xRecordName = m_xRecordName + ":" + Utf8String(entryName);
                }

            if (nullptr != xRecord)
                DumpXrecord (*xRecord);
            else if (nullptr != child)
                DumpDictionaryXrecords (*child);
            else
                BeAssert (false);

            // Pop current dictinary name out from the xRecord name:
            if (!entryName.IsEmpty())
                m_xRecordName = m_xRecordName.substr (0, previousSize);
            }
        }
    return  BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ImportXData::_BeginImport ()
    {
    // Let parent process begin
    T_Super::_BeginImport ();

    m_xRecordName.clear ();
    m_dumpCount = 0;

    // Read Regapp table and validate the desired regapp name
    Utf8String  desiredRegappName = m_sampleOptions.GetRegAppName ();
    if (!desiredRegappName.empty())
        {
        bool    isValid = false;
        DwgDbRegAppTablePtr regappTable(GetDwgDb().GetRegAppTableId(), DwgDbOpenMode::ForRead);
        if (!regappTable.IsNull())
            {
            DwgDbSymbolTableIteratorPtr iter = regappTable->NewIterator ();
            if (iter.IsValid())
                {
                for (iter->Start(); !iter->Done(); iter->Step())
                    {
                    DwgDbRegAppTableRecordPtr   regapp(iter->GetRecordId(), DwgDbOpenMode::ForRead);
                    if (!regapp.IsNull())
                        {
                        Utf8String  utf8Name (regapp->GetName().c_str());
                        if (utf8Name.CompareToI(desiredRegappName.c_str()) == 0)
                            {
                            // The input desired regapp name exists in Regapp table.
                            isValid = true;
                            break;
                            }
                        }
                    }
                }
            }

        if (!isValid)
            {
            m_sampleOptions.SetRegAppName ("");
            ReportIssue (IssueSeverity::Warning, IssueCategory::UnexpectedData(), Issue::Message(), Utf8PrintfString("Desired Regapp %s is not found in DWG, will dump XDATA for all Regapp's.", desiredRegappName.c_str()).c_str());
            }
        }

    // If our bridge option wants to dump dictionary xRecords, do it now
    if (m_sampleOptions.ShouldDumpXrecords())
        {
        DwgDbDictionaryPtr  mainDictionary (GetDwgDb().GetNamedObjectsDictionaryId(), DwgDbOpenMode::ForRead);
        if (mainDictionary.IsNull())
            ReportError (IssueCategory::CorruptData(), Issue::CantOpenObject(), "the named/main dictionary of the DWG file!");
        else
            DumpDictionaryXrecords (*mainDictionary.get());
        
        _ReportIssue (IssueSeverity::Info, IssueCategory::Unsupported(), "Total dictionary xRecords dumped", Utf8PrintfString("\t%d", m_dumpCount).c_str());
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DwgImporter* ImportXDataSample::_CreateDwgImporter ()
    {
    // Create our sample importer
    return  new ImportXData (GetImportOptions(), m_sampleOptions);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
iModelBridge::CmdLineArgStatus ImportXDataSample::_ParseCommandLineArg (int iArg, int argc, WCharCP argv[])
    {
    if (0 == wcsncmp(argv[iArg], L"--regappname=", 13))
        {
        // Will only dump XDATA of the specified regApp.
        WString cmdlineArg(argv[iArg]);
        cmdlineArg = cmdlineArg.substr (cmdlineArg.find_first_of(L'=', 0) + 1);
        m_sampleOptions.SetRegAppName (Utf8String(cmdlineArg.c_str()));
        return CmdLineArgStatus::Success;
        }
    else if (0 == wcscmp(argv[iArg], L"--dumpxrecords"))
        {
        // Will dump dictionary xRecords.
        m_sampleOptions.SetShouldDumpXrecords (true);
        return CmdLineArgStatus::Success;
        }
    // Not our sample option.
    return CmdLineArgStatus::NotRecognized;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ImportXDataSample::_PrintUsage ()
    {
    // Print out our sample options, along with all inherited iModelBridge options:
    fwprintf (stderr,
L"\
--regappname=\t\t(optional) A desired regApp to dump entity XData (default dumps all).\n\
--dumpxrecords\t\t(optional; logging only) An option to read and print out dictionary xRecords.\n\
");
    }

END_DGNDBSYNC_DWG_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridge* iModelBridge_getInstance(wchar_t const* bridgeRegSubKey)
    {
    // Supply our sample Bridge to the iModelBridge Framework.
    return  new ImportXDataSample();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int wmain (int argc, wchar_t const* argv[])
    {
    /*-----------------------------------------------------------------------------------
    Minimal command line arguments:
    ImportXData -i=<input DWG full file name> -o=<output DgnDb folder name>

    No arguments will print usage.
    -----------------------------------------------------------------------------------*/
    ImportXDataSample     sampleBridge;

    // Register this sample as an iModelBridge:
    sampleBridge.GetImportOptions().SetBridgeRegSubKey (L"XDataSampleBridge");

    // Begin importing DWG file into DgnDb
    BentleyStatus   status = sampleBridge.RunAsStandaloneExe (argc, argv);

    return (int)status;
    }
