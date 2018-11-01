/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbSync/SyncManager/SyncManager.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatform.r.h>
#include <DgnDbSync/DgnDbSync.h>
#include <Bentley/BeFileName.h>

DGNDBSYNC_TYPEDEFS(IProgressListener)
DGNDBSYNC_TYPEDEFS(IConverter)
DGNDBSYNC_TYPEDEFS(SyncManager)

DGNDBSYNC_REFCOUNTED_TYPEDEFS(IConverter)

BEGIN_DGNDBSYNC_NAMESPACE

//=======================================================================================
//! Error states of the synchronization
//=======================================================================================
enum class SyncStatus
    {
    Completed = 0,
    CompletedWithWarnings = 1,
    InvalidInput = 2,
    ConverterNotFound = 3,
    Aborted = 4,
    Error = 5
    };

//=======================================================================================
//! Progress listener interface to track and abort conversion to a .bim
//=======================================================================================
struct IProgressListener
{
//! Flag to indicate if detection needs to be aborted
enum class Abort
    {
    No,
    Yes
    };

protected:
    //! Implement to show progress updates, and abort the conversion
    //! @return Return Abort::Yes to abort the processing. Return Abort::Yes to abort processing 
    DGNDBSYNC_EXPORT virtual Abort _UpdateProgress() = 0;

public:
    //! Method called by converter to update progress
    Abort UpdateProgress() { return _UpdateProgress(); }
};

//=======================================================================================
//! Base class for an out of process converter
//=======================================================================================
struct IConverter : RefCountedBase
{
private:
    BeFileName m_exePathname;
    mutable Json::Value m_options; // Command line options

    static Utf8String FormatOptionValue(Utf8CP optionValue);
    
    // Init options before generating the command line
    void InitOptions(BeFileNameCR inputPathname, BeFileNameCR outputDirectory, WCharCP outputExtension, bool isUpdate) const;

    // Clean output before running a non-update process
    void CleanOutput(BeFileNameCR outputDirectory, WCharCP outputFileNameNoExt);

    //! Build the command line to launch the converter out of process
    Utf8String BuildCommandLine(BeFileNameCR inputPathname, BeFileNameCR outputDirectory, WCharCP outputExtension, bool isUpdate) const;

    static BeFileName BuildSemaphorePathname(BeFileNameCR outputPathname);
    static void MarkConversionComplete(BeFileNameCR inputPathname, BeFileNameCR outputPathname);
    static bool CheckConversionComplete(BeFileNameCR inputPathname, BeFileNameCR outputPathname);

    static BentleyStatus WriteJsonToFile(BeFileNameCR pathname, JsonValueCR jsonValue);
    static BentleyStatus ReadJsonFromFile(JsonValueR jsonValue, BeFileNameCR pathname);

    static SyncStatus LaunchProcess(Utf8StringCR cmdLine, IProgressListenerP progressListene);
    static Utf8CP SyncStatusToString(SyncStatus status);

protected:
    bvector<WString> m_inExtensions;
    bvector<WString> m_outExtensions;
    WString m_logExtension;
    bvector<WString> m_interimExtensions;
    BeFileName m_rspPathname;

protected:
    IConverter(BeFileNameCR exePathname) : m_exePathname(exePathname), m_options(Json::nullValue) {}

    DGNDBSYNC_EXPORT virtual void _InitOptions(BeFileNameCR inputPathname, BeFileNameCR outputDirectory, WCharCP outputExtension, bool isUpdate) const = 0;
    DGNDBSYNC_EXPORT virtual bool _SupportsUpdates() const = 0;

    DGNDBSYNC_EXPORT void AddOption(Utf8CP prefix, Utf8CP name, Utf8CP separator, Utf8CP value) const;

public:
    //! Set the path name to the executable
    void SetExePathName(BeFileNameCR exePathname) { m_exePathname = exePathname; }

    //! Get the path name to the executable
    BeFileNameCR GetExePathName() const {return m_exePathname;}

    //! Returns true if the converter can handle the supplied input extension
    DGNDBSYNC_EXPORT bool SupportsInputExtension(WCharCP inputExtension) const;
        
    //! Returns true if the converter can handle the supplied output extension
    DGNDBSYNC_EXPORT bool SupportsOutputExtension(WCharCP outputExtension) const;

    //! Returns true if the converter supports updates
    bool SupportsUpdates() { return _SupportsUpdates(); }

    //! Set a response file with command line options that is to be passed into the converter
    //! @remarks These command line options are appended to the other command line options passed by the converter
    void SetResponseFile(BeFileNameCR rspPathname) { m_rspPathname = rspPathname; }

    // Performs a validation check on the converter
    DGNDBSYNC_EXPORT bool Validate() const;

    //! Build the pathname of the log file
    BeFileName BuildLogPathname(BeFileNameCR inputPathname, BeFileNameCR outputDirectory) const;

    //! Build the pathname of the output file
    static BeFileName BuildOutputPathname(BeFileNameCR inputPathname, BeFileNameCR outputDirectory, WCharCP outputExtension);

    //! Launch converter to synchronize input and output
    SyncStatus Synchronize(BeFileNameCR inputPathname, BeFileNameCR outputDirectory, WCharCP outputExtension, IProgressListenerP progressListener);
};

//=======================================================================================
//! Base class for converters from Dgn/Dwgn format to IModel/ibim format
//=======================================================================================
struct DgnDwgConverterBase : IConverter
{
protected:
    DGNDBSYNC_EXPORT virtual void _InitOptions(BeFileNameCR inputPathname, BeFileNameCR outputDirectory, WCharCP outputExtension, bool isUpdate) const override;
    DGNDBSYNC_EXPORT virtual bool _SupportsUpdates() const override { return true; }

    DGNDBSYNC_EXPORT DgnDwgConverterBase(BeFileNameCR exePathname, WCharCP inputExtension);
};

//=======================================================================================
//! Converter from dgn format to dgndb/imodel format
//=======================================================================================
struct DgnConverter : DgnDwgConverterBase
{
protected:
    DgnConverter(BeFileNameCR exePathname) : DgnDwgConverterBase(exePathname, L"dgn") {}
public:
    DGNDBSYNC_EXPORT static IConverterPtr Create(BeFileNameCR exePathname);
};

//=======================================================================================
//! Converter from dgn format to dgndb/imodel format
//=======================================================================================
struct DwgConverter : DgnDwgConverterBase
{
protected:
    DwgConverter(BeFileNameCR exePathname) : DgnDwgConverterBase(exePathname, L"dwg") 
        {
        m_inExtensions.push_back(L"dxf");
        }
public:
    DGNDBSYNC_EXPORT static IConverterPtr Create(BeFileNameCR exePathname);
};

//=======================================================================================
//! Converter from rvm or vue formats to the dgn format
//=======================================================================================
struct RvmVueConverterBase : IConverter
{
protected:
    virtual void _InitOptions(BeFileNameCR inputPathname, BeFileNameCR outputDirectory, WCharCP outputExtension, bool isUpdate) const override;
    virtual bool _SupportsUpdates() const override { return false; }

    RvmVueConverterBase(BeFileNameCR exePathname, WCharCP inputExtension);
};

//=======================================================================================
//! Converter from rvm format to the dgn format
//=======================================================================================
struct RvmConverter : RvmVueConverterBase
{
protected:
    RvmConverter(BeFileNameCR exePathname) : RvmVueConverterBase(exePathname, L"rvm") {}
public:
    DGNDBSYNC_EXPORT static IConverterPtr Create(BeFileNameCR exePathname);
};

//=======================================================================================
//! Converter from vue format to the dgn format
//=======================================================================================
struct VueConverter : RvmVueConverterBase
{
protected:
    VueConverter(BeFileNameCR exePathname) : RvmVueConverterBase(exePathname, L"vue") {}
public:
    DGNDBSYNC_EXPORT static IConverterPtr Create(BeFileNameCR exePathname);
};

//=======================================================================================
//! Utility to orchestrate synchronization of various files with the DgnDb format
//=======================================================================================
struct SyncManager : NonCopyableClass
{
private:
    bvector<IConverterPtr> m_converters;
    IProgressListenerP m_progressListener = nullptr;

    IConverterP GetRegisteredConverter(WCharCP inExtension, WCharCP outExtension);
    SyncStatus LaunchConverter(IConverterR converter, BeFileNameCR inputPathname, BeFileNameCR outputDirectory, WCharCP outputExtension);

    static BeFileName BuildPathname(BeFileNameCR outputDirectory, WCharCP fileNameNoExt, WCharCP extension);
public:
    //! Create the sync manager
    SyncManager() {}

    //! Destructor
    ~SyncManager() {}

    //! Register a progress listener
    //! @param listener[in] Implementation of progress listener to register. Replaces any existing listener. Pass null to unregister. 
    //! @remarks There can only be one registered listener, and it's the caller's responsibility
    //! to keep any registered listener in memory for the lifetime of the sync manager. 
    DGNDBSYNC_EXPORT void RegisterProgressListener(IProgressListenerP listener) { m_progressListener = listener; }

    //! Register a converter
    DGNDBSYNC_EXPORT BentleyStatus RegisterConverter(IConverterR converter);

    //! Synchronize (convert or update) a source file with a dgndb
    DGNDBSYNC_EXPORT SyncStatus Synchronize(bvector<Utf8String>& errors, BeFileNameCR inputPathname, BeFileNameCR outputDirectory, WCharCP outputExtension);
};

END_DGNDBSYNC_NAMESPACE
