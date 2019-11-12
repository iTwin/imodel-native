/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include    "ListProperty.h"


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool    ListProperty::ValidatePropertyFormat (WStringR newString, WStringR currString, WStringR previousString) const
    {
    // An additional string that may need to be inserted before currString
    newString.clear ();

    if (WString::npos == currString.find(L':'))
        {
        // Current string has no property separator ":" - don't guess, just use previous string as is:
        if (!previousString.empty())
            newString = previousString;

        // move on to next string
        previousString = currString;
        return  false;
        }
    else if (currString.at(0) == L':')
        {
        // Current string starts by ":", combine it with previous string.
        if (!previousString.empty())
            {
            previousString += currString;
            return  false;
            }
        }
    else
        {
        // Current string appears like a well formated string by itself - safe to use previousString
        if (!previousString.empty())
            {
            newString = previousString;
            previousString.clear ();
            return  false;
            }
        }

    // Always empty the string we are tracking with if we have a valid string to add:
    previousString.clear ();

    // Go ahead and attach validated string to the element
    return  true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus   ListProperty::ConvertMessage (T_Utf8StringVectorR utf8List, WStringR currMessage, WStringR prevMessage) const
    {
    currMessage.Trim ();
    if (currMessage.empty())
        return  BSIERROR;

    // Make format of "property name :/= <property value>".
    WString     messageToInsert;
    bool        saveCurrMessage = ValidatePropertyFormat (messageToInsert, currMessage, prevMessage);

    // Insert a new string before currString per above validation.
    Utf8String  propString;
    if (!messageToInsert.empty())
        {
        propString.Assign (messageToInsert.c_str());
        utf8List.push_back (propString);
        }

    // Add current string per above validation.
    if (saveCurrMessage)
        {
        propString.Assign (currMessage.c_str());
        utf8List.push_back (propString);
        }

    return  BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus   ListProperty::ConvertMessageCollection (T_Utf8StringVectorR utf8List, T_WStringVectorR wcharList) const
    {
    /*-----------------------------------------------------------------------------------
    The strings have harvested from the DWG toolkit host's callback are just a collection
    of text strings.  These are the text string you'd see in AutoCAD when you use the LIST
    command on the same entity.  They are not real properties.
    
    To help make our Adhoc string properties look readable, here we try to parse the string 
    dump into a collection of strings each of which is in a format of:

        "property name : property value"

    Note that this by no means a foolproof string processor and exceptions printed out by
    the toolkit can happen, but is enough to serve for the purpose of a sample code.
    -----------------------------------------------------------------------------------*/
    WString     lastMessage;

    utf8List.clear ();

    for (auto& message : wcharList)
        {
        if (message.empty())
            continue;

        // Break down the text body into multiple lines of texts that are separately by a LINEFEED:
        size_t  lineFeedAt = WString::npos;
        while (WString::npos != (lineFeedAt = message.find(0x0A)))
            {
            if (lineFeedAt+1 == message.length())
                {
                lineFeedAt = WString::npos;
                break;
                }

            WString     subString(message.c_str(), lineFeedAt);
            ConvertMessage (utf8List, subString, lastMessage);
            message.erase (0, lineFeedAt + 1);
            }

        // Add last line of text:
        if (WString::npos == lineFeedAt)
            ConvertMessage (utf8List, message, lastMessage);
        }

    // Add last string that happens to be "property name : <no property value>"
    if (!lastMessage.empty())
        {
        WString     emptyMessage;
        ConvertMessage (utf8List, lastMessage, emptyMessage);
        }

    return  utf8List.empty() ? BSIERROR : BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus   ListProperty::ListEntityProperties (DgnElementR element, DwgDbEntityCR entity)
    {
    BentleyStatus       status = BSISUCCESS;
    T_WStringVector     messageList;

    // Request DwgImporter host to collect message dump sent from the toolkit(RealDWG):
    T_Super::GetMessageCenter().StartListMessageCollection (&messageList);

    try
        {
        // Start ACAD command LIST on this entity and let the DwgImporter host fill up our messageList:
        entity.List ();
        }
    catch (...)
        {
        status = BSIERROR;
        LOG.errorv ("Failed LIST command on entity %ls[ID=%lld]", entity.GetDxfName().c_str(), entity.GetObjectId().ToUInt64());
        }

    // Done message dump
    T_Super::GetMessageCenter().StopListMessageCollection ();
    if (BSISUCCESS != status)
        return  status;

    // Convert the raw AutoCAD messages into something presentable as string properies:
    T_Utf8StringVector  stringProperties;
    if (BSISUCCESS != ConvertMessageCollection(stringProperties, messageList))
        return  BSIERROR;

    // Add each and every string as an Adhoc text property of the DgnElement
    size_t  digits = static_cast<size_t> (floor(log10(stringProperties.size())) + 1);
    size_t  count = 1;
    for (auto& string : stringProperties)
        {
        // Use property name "LPxx"
        Utf8PrintfString    uniqueName("LP%0*d", digits, count++);
        // Set the text value to the element's Adhoc property
        AdHocJsonValue      adhocProp;
        adhocProp.SetValueText ("Text", string.c_str());
        element.SetUserProperties (uniqueName.c_str(), adhocProp);
        }

    return  status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus   ListProperty::_ImportEntity (ElementImportResults& results, ElementImportInputs& inputs)
    {
    // Create a DgnElement from DWG entity by default, and get the results back:
    DgnElementP     dgnElement = nullptr;
    BentleyStatus   status = T_Super::_ImportEntity (results, inputs);
    if (BSISUCCESS != status || nullptr == (dgnElement = results.GetImportedElement()))
        {
        LOG.errorv ("Failed creating DgnElement from DWG entity %lld", inputs.GetEntity().GetObjectId().ToUInt64());
        return  status;
        }

    // Don't want to LIST standard ACAD entities:
    DwgString   className = inputs.GetEntity().GetDwgClassName ();
    if (className.StartsWithI(L"AcDb"))
        return  status;

    LOG.tracev ("DgnElement %s(ID=%lld) has been created from a custom object, trying LIST command...", dgnElement->GetDisplayLabel().c_str(), dgnElement->GetElementId().GetValue());

    // Create Adhoc properties from the LIST "properties" of a custom object:
    status = ListEntityProperties (*dgnElement, inputs.GetEntity());

    if (BSISUCCESS != status)
        LOG.error ("Failed adding LIST properties to DgnElement as Adhoc properties!");

    return  status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String  ListProperty::_ComputeImportJobName (DwgDbBlockTableRecordCR modelspaceBlock) const
    {
    /*-----------------------------------------------------------------------------------
    This method is called from DwgImporter to build a import job name from a root DWG file's 
    modelspace block. The default implementation builds the job name based on input block name
    plus below iModelBridge params:

    DwgImporter::Options::GetBridgeJobName
    DwgImporter::Options::GetBridgeRegSubKeyUtf8
    DwgImporter::Options::GetBridgeRegSubKey

    This example sets the job name to a simple constant name.
    -----------------------------------------------------------------------------------*/
    return  "Sample iModelBridge ListProperty";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
SubjectPtr ListProperty::ConvertDictionary(SubjectCR parentSubject, DwgDbObjectCR sourceObject, Utf8StringCR name)
    {
    // Calculate a new object provenance from the input dictionary object
    DwgSourceAspects::ObjectProvenance  prov(sourceObject, *this);

    // Set the new provenance as current provenance in DetectionResults for the change detector.
    IDwgChangeDetector::DetectionResults    detection;
    detection.SetCurrentProvenance (prov);

    // Get active change detector
    auto& changeDetector = _GetChangeDetector();

    // Will create a subject element as child of the input parent subject
    SubjectPtr  subject;
    auto targetModel = parentSubject.GetModel();
    auto sourceHandle = sourceObject.GetObjectId().GetHandle();

    // Detect changes
    Json::Value data(Json::nullValue);
    if (changeDetector._IsElementChanged(detection, *this, sourceHandle, *targetModel))
        {
        // A change has been detected since last job run, convert source data as necessary - this sample only saves class name as a json property.
        data["DwgClass"] = DwgHelper::ToUtf8CP(sourceObject.GetDwgClassName());
        }

    // Act based on change detection result
    switch (detection.GetChangeType())
        {
        case IDwgChangeDetector::ChangeType::None:
            {
            // No change - just tell change detector that we have seen this element
            changeDetector._OnElementSeen(*this, detection.GetExistingElementId());

            // In a normal workflow, probaly nothing else needs to be done, but this sample needs a subject element as next parent subject, so take the extra step here.
            auto aspect = T_Super::GetSourceAspects().FindObjectAspect(sourceHandle, *targetModel);
            BeAssert (aspect.IsValid());
            subject = T_Super::GetDgnDb().Elements().GetForEdit<Subject>(aspect.GetElementId());
            break;
            }
        case IDwgChangeDetector::ChangeType::Insert:
            {
            // New dictionary entry - create & insert a new subject element representing it
            subject = Subject::Create(parentSubject, name);
            subject->SetSubjectJsonProperties("DwgDictionary", data);

            // Create an ExternalSourceAspect with pre-caculated provenance
            T_Super::GetSourceAspects().AddOrUpdateObjectAspect(*subject, sourceHandle, detection.GetCurrentProvenance());

            DgnDbStatus status;
            auto newElement = subject->Insert(&status);
            if (!newElement.IsValid() || status!=DgnDbStatus::Success)
                break;  // error

            changeDetector._OnElementSeen(*this, newElement->GetElementId());
            break;
            }
        case IDwgChangeDetector::ChangeType::Update:
            {
            // Existing element - find it from ExternalSourceAspect then update it from the input dictionary
            auto aspect = T_Super::GetSourceAspects().FindObjectAspect(sourceHandle, *targetModel);
            BeAssert (aspect.IsValid());
            subject = T_Super::GetDgnDb().Elements().GetForEdit<Subject>(aspect.GetElementId());
            if (!subject.IsValid())
                break;  // error

            // Update set data and update the element
            subject->SetSubjectJsonProperties("DwgDictionary", data);
            subject->Update();
            
            changeDetector._OnElementSeen(*this, detection.GetExistingElementId());
            break;
            }
        }
    return  subject;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ListProperty::DrillInDictionary (SubjectCR parentSubject, DwgDbDictionaryP dictionary, Utf8StringCR name, Utf8StringCR prefix)
    {
    auto sourceObject = DwgDbObject::Cast(dictionary);
    if (sourceObject == nullptr)
        return  BentleyStatus::BSIERROR;

    // Create/update a subject element from input source dictionary
    auto nextSubject = ConvertDictionary(parentSubject, *sourceObject, name);
    if (!nextSubject.IsValid())
        return  BentleyStatus::BSIERROR;

    auto iter = dictionary->GetIterator();
    if (!iter.IsValid() || !iter->IsValid())
        return  BentleyStatus::BSIERROR;

    for (; !iter->Done(); iter->Next())
        {
        // Filter out entries by entry name prefix
        Utf8String  childName(iter->GetName().c_str());
        if (!childName.StartsWithI(prefix.c_str()))
            continue;
        DwgDbObjectPtr  child(iter->GetObjectId(), DwgDbOpenMode::ForRead);
        if (child.OpenStatus() != DwgDbStatus::Success)
            continue;

        // Drill into the dictionary
        auto nextDictionary = DwgDbDictionary::Cast(child.get());
        if (nextDictionary != nullptr)
            DrillInDictionary(*nextSubject, nextDictionary, childName, prefix);
        else
            ConvertDictionary(*nextSubject, *child, name);
        }
    return  BentleyStatus::BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ListProperty::_MakeDefinitionChanges(SubjectCR jobSubject)
    {
    /*-----------------------------------------------------------------------------------
    This method is called from iModelBridgeFwk which holds dictionary locks for a bridge.
    The default implementation updates dictionary model from DWG tables(layers, materials, etc).
    After the tables have been processed, this sample code loops through DWG main dictionary
    table and creates or updates Subject elements from selected dictionaries.  These subject
    elements are inserted under the job subject, retaining the same hierarchy as they are
    in the source dictionary.

    Notice that the change detector is used in such a generic way that any source object
    can apply.
    -----------------------------------------------------------------------------------*/
    T_Super::_MakeDefinitionChanges(jobSubject);

    DwgDbDictionaryPtr acadDictionary(GetDwgDb().GetNamedObjectsDictionaryId(), DwgDbOpenMode::ForRead);
    if (acadDictionary.OpenStatus() != DwgDbStatus::Success)
        return  BentleyStatus::BSIERROR;

    // Create the top subject as AcadDictionary and filter in "AVEVA_" dictionary entries
    Utf8String  subjectName("AcadDictionary");
    Utf8String  entryPrefix("AVEVA_");

    return  DrillInDictionary(jobSubject, acadDictionary.get(), subjectName, entryPrefix);
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DwgImporter* ListPropertySample::_CreateDwgImporter ()
    {
    // Create our sample importer, using DwgBridge options:
    DwgImporter::Options* opts = static_cast<DwgImporter::Options*> (&_GetParams());
    if (nullptr == opts)
        {
        BeAssert (false && "This sample is not a sub-class of DwgBridge!!");
        return  nullptr;
        }
    return new ListProperty (*opts);
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int wmain (int argc, wchar_t const* argv[])
    {
    /*-----------------------------------------------------------------------------------
    Expected command line arguments:

    ListProperty <input DWG full file name> <output DgnDb folder name>
    -----------------------------------------------------------------------------------*/
    if (argc < 3)
        return  1;

    // Begin importing DWG file into DgnDb using DwgBridge
    ListPropertySample  sampleApp;
    if (BentleyStatus::SUCCESS != sampleApp.RunAsStandaloneExe(argc, argv))
        return 2;

    return 0;
    }
