/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatform.h>
#include <iModelBridge/iModelBridge.h>
#include <Dwg/DwgImporter.h>
#include <Dwg/DwgHelper.h>
#include <Dwg/DwgDb/DwgDbHost.h>

USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG

BEGIN_DWG_NAMESPACE

/*=================================================================================**//**
* A simple & separate DwgDbHost enough for the affinity calculation in this sample code.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct AffinityHost : public IDwgDbHost
{
public:
    AffinityHost () {}
    ~AffinityHost ();
    static AffinityHost&    GetHost ();
}; // AffinityHost


static AffinityHost*    s_affinityHostInstance = nullptr;


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AffinityHost::~AffinityHost ()
    {
    AffinityHost::TerminateToolkit ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AffinityHost&  AffinityHost::GetHost ()
    {
    if (nullptr == s_affinityHostInstance)
        {
        s_affinityHostInstance = new AffinityHost ();
        AffinityHost::InitializeToolkit (*s_affinityHostInstance);
        }
    return  *s_affinityHostInstance;
    }

END_DWG_NAMESPACE



extern "C"
{

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
EXPORT_ATTRIBUTE void iModelBridge_getAffinity (WCharP buffer, const size_t bufferSize, BentleyApi::Dgn::iModelBridgeAffinityLevel& affinityLevel, WCharCP affinityLibPath, WCharCP dwgdxfName)
    {
    /*-----------------------------------------------------------------------------------
    Supply an affinity to the iModelBridgeAssignment for the input file.

    We want our sample iModel Bridge to precede the base DwgBridge, i.e. set to a higher level
    for a DWG file we want to own.  Yet, we do not intend to own all DWG/DXF files.  We'd rather 
    the base DwgBridge to handle DWG files in which we are not interested.  In this sample, we 
    only want to own DWG files which may have customer XDATA.  We demonstrate two steps in 
    achieving this gaol:

    1) First check if the input file is of DWG or DXF format. This sanity check does not
        load the input DWG file and can filter out non-DWG files more efficiently than next step.

    2) The second step is to check RegApp table records, by loading the DWG file through a toolkit.
        We only want to own files which have at least one non-ACAD RegApp table entry.  Obviously,
        this is a far more expensive step, particularly the first time.  Therefore, if you can
        sniff binary content of a DWG file for your bridge, without loading the file via a toolkit, 
        you have a lot of performance to gain.

    If you will test this sample bridge in a full integration of ProjectWise & Bentley Automation 
    Services, you must add below registry on the server you are testing, assuming this sample's 
    output dll & exe along with all dependencies are under folder C:\programs\SampleBridge\:

    [HKEY_LOCAL_MACHINE\SOFTWARE\Bentley\iModelBridges\ImportXDataBridge]
        "AffinityLibraryPath"="C:\\programs\\SampleBridge\\SampleAffinity.dll"
        "BridgeLibraryPath"="C:\\programs\\SampleBridge\\ImportXData.exe"
        "iModelFrameWorkExe"="C:\\programs\\SampleBridge\\iModelBridgeFwk.exe"
        "BridgeAssetsDir"="C:\\programs\SampleBridge\\Assets"
        "IsPowerPlatformBased"="false"

    Note that the above registry sub key "ImportXDataBridge" must match to what this method returns 
    through the output argument "buffer".
    -----------------------------------------------------------------------------------*/
    affinityLevel = BentleyApi::Dgn::iModelBridge::Affinity::None;

    // Step-1: quickly filter out the input file based on file type:
    BeFileName  filename(dwgdxfName);
    if (!DwgHelper::SniffDwgFile(filename) && !DwgHelper::SniffDxfFile(filename))
        return;

    // Step-2: open the file via a toolkit, an expensive operation, particularly at the first time when many DLL's are loaded.
    auto dwg = AffinityHost::GetHost().ReadFile (dwgdxfName, false, false, FileShareMode::DenyNo);
    if (!dwg.IsValid())
        return;

    // Open the RegApp table:
    DwgDbRegAppTablePtr regappTable(dwg->GetRegAppTableId(), DwgDbOpenMode::ForRead);
    if (regappTable.OpenStatus() != DwgDbStatus::Success)
        return;

    // Create an iterator to loop through RegApp table entries:
    auto iter = regappTable->NewIterator ();
    if (!iter->IsValid())
        return;

    // Check each RegApp table entry:
    bool    hasNonAcadRegApp = false;
    for (iter->Start(); !iter->Done(); iter->Step())
        {
        //
        DwgDbRegAppTableRecordPtr   regapp(iter->GetRecordId(), DwgDbOpenMode::ForRead);
        if (regapp.OpenStatus() == DwgDbStatus::Success)
            {
            // A RegApp name starts with prefix ACAD, AEC, or AVE is an ACAD entry.
            auto name = regapp->GetName ();
            if (name.StartsWithI(L"ACAD") || name.StartsWithI(L"AEC") || name.StartsWithI(L"AVE"))
                continue;

            // Found one entry that is not of an ACAD RegApp
            hasNonAcadRegApp = true;
            break;
            }
        }

    if (hasNonAcadRegApp)
        {
        // Set to level High overtake the ownership from other bridges with lower levels.
        affinityLevel = BentleyApi::Dgn::iModelBridge::Affinity::High;
        // Return our registry sub key, HKLM\SOFTWARE\Bentley\iModelBridges\ImportXDataBridge:
        BeStringUtilities::Wcsncpy(buffer, bufferSize, L"ImportXDataBridge");
        }
    }

}   // extern "C"
