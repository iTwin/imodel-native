/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
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
