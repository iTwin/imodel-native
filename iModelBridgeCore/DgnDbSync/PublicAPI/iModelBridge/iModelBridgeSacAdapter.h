/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <iModelBridge/iModelBridge.h>
#include <DgnPlatform/DesktopTools/KnownDesktopLocationsAdmin.h>

#ifdef __IMODEL_BRIDGE_BUILD__
    #define IMODEL_BRIDGE_EXPORT EXPORT_ATTRIBUTE
#else
    #define IMODEL_BRIDGE_EXPORT IMPORT_ATTRIBUTE
#endif

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Adapter that helps a standalone converter (SAC) to run a bridge.
//! A stand-alone converter program may be needed to do one-off publishing.
//! Also, running a bridge from a stand-alone converter program is a convenient way to test the 
//! conversion logic of the bridge, independently of the iModelHub.
//! <p>
//! You can write a standalone executable for any bridge using the following code. 
//! @code
//! #include <stdio.h>
//! #include <iModelBridge/iModelBridgeSacAdapter.h>
//! 
//! USING_NAMESPACE_BENTLEY_DGN
//! 
//! int main(int argc, WCharCP argv[])
//!     {
//!     iModelBridgeSacAdapter::InitCrt(false);
//! 
//!     iModelBridge* bridge = iModelBridge_getInstance();
//! 
//!     iModelBridgeSacAdapter::Params saparams;
//!     if (BentleyStatus::SUCCESS != iModelBridgeSacAdapter::ParseCommandLine(*bridge, saparams, argc, argv))
//!         return BentleyStatus::ERROR;
//! 
//!     iModelBridgeSacAdapter::InitializeHost(*bridge);
//! 
//!     if (BSISUCCESS != bridge->_Initialize(argc, argv))
//!         {
//!         fprintf(stderr, "_Initialize failed\n");
//!         return BentleyStatus::ERROR;
//!         }
//! 
//!     return iModelBridgeSacAdapter::Execute(*bridge, saparams);
//!     }
//! @endcode
//! Compile this code into an executable and link it with the shared library that implements 
//! the bridge that you want to run. 
//! @ingroup GROUP_iModelBridge
// @bsiclass                                    BentleySystems 
//=======================================================================================
struct iModelBridgeSacAdapter
{
    //! @private
    static WCharCP std_CompressedDgnDbExt() {return L"imodel";}

    //! Parameters that are specific to standalone converters that use iModelBridges.
    struct Params : iMBridgeDocPropertiesAccessor
        {
      protected:
        bool m_tryUpdate = false;               
        bool m_quietAsserts = false;            
        bool m_createStandalone = false;            
        bool m_shouldCompress = false;
        bool m_isFileAssignedToBridge = true;
        bool m_detectDeletedFiles = false;
        bool m_mergeDefinitions = false;
        uint32_t m_compressChunkSize = 0;
        BeFileName m_loggingConfigFile;         
        BeFileName m_dupInputFileName;         
        Utf8String m_description;
        Utf8String m_attributesJSON;
        BeSQLite::BeGuid m_docGuid;
        DateTime m_expirationDate;
        BeFileName m_bridgeAssetsDir;

      public:
        //! Helper function to parse a command-line argument for a stand-alone converter.
        IMODEL_BRIDGE_EXPORT iModelBridge::CmdLineArgStatus ParseCommandLineArg(int iarg, int argc, WCharCP argv[]);

        IMODEL_BRIDGE_EXPORT static void PrintUsage();

        //! Initialize logging and assertion handling
        IMODEL_BRIDGE_EXPORT void Initialize() const;

        void SetShouldTryUpdate(bool b) {m_tryUpdate=b;} //!< Tell the converter if it should try to update an existing dgndb, if possible.
        bool ShouldTryUpdate() const {return m_tryUpdate;} //!< Attempt to update existing dgndb, if possible.
        void SetQuietAsserts(bool b) {m_quietAsserts=b;}
        bool GetQuietAsserts() const {return m_quietAsserts;} //!< Write assertion failures to stderr and do not interrupt the converter?
        void SetDetectDeletedFiles(bool b) {m_detectDeletedFiles=b;}
        bool GetDetectDeletedFiles() const {return m_detectDeletedFiles;} //!< Invoke the bridge's logic to detect deleted files?
        void SetLoggingConfigFile(BeFileNameCR f) {m_loggingConfigFile=f;} //!< Location of the logging configuration xml file
        BeFileNameCR GetLoggingConfigFile() const {return m_loggingConfigFile;} //!< Location of the logging configuration xml file
        void SetDupInputFileName(BeFileNameCR f) {m_dupInputFileName=f;}
        BeFileNameCR GetDupInputFileName() const {return m_dupInputFileName;}
        void SetCreateStandalone(bool standalone) {m_createStandalone = standalone;}
        bool GetCreateStandalone() const {return m_createStandalone;}
        void SetDescription(BentleyApi::Utf8CP descr) {m_description=descr;}
        Utf8String GetDescription() const {return m_description;}
        void SetDocAttributesJson(BentleyApi::Utf8CP descr) {m_attributesJSON=descr;}
        Utf8String GetDocAttributesJson() const {return m_attributesJSON;}
        void SetExpirationDate(DateTime const& d) {m_expirationDate=d;}
        DateTime GetExpirationDate() const {return m_expirationDate;}
        bool ShouldCompress() const {return m_shouldCompress;}
        uint32_t GetCompressChunkSize() const {return m_compressChunkSize;}
        void SetDocumentGuid(BeSQLite::BeGuid const& g) {m_docGuid=g;}
        BeSQLite::BeGuid GetDocumentGuid() const {return m_docGuid;}
        BeFileNameCR GetBridgeAssetsDir() const {return m_bridgeAssetsDir;}

        IMODEL_BRIDGE_EXPORT void GetDocumentProperties(iModelBridgeDocumentProperties&);
        IMODEL_BRIDGE_EXPORT bool _IsFileAssignedToBridge(BeFileNameCR fn, wchar_t const* bridgeRegSubKey) override;
        IMODEL_BRIDGE_EXPORT void _QueryAllFilesAssignedToBridge(bvector<BeFileName>& fns, wchar_t const* bridgeRegSubKey) override;
        IMODEL_BRIDGE_EXPORT BentleyStatus _GetDocumentProperties(iModelBridgeDocumentProperties& props, BeFileNameCR fn) override;
        IMODEL_BRIDGE_EXPORT BentleyStatus _GetDocumentPropertiesByGuid(iModelBridgeDocumentProperties& props, BeFileNameR localFilePath, BeSQLite::BeGuid const& docGuid) override;
        virtual BentleyStatus _AssignFileToBridge(BeFileNameCR fn, wchar_t const* bridgeRegSubKey, BeSQLite::BeGuidCP guid) override { return BSIERROR;}
        };

    //! @private
    //! Helper function to initialize the common bridge Params with values that can be inferred for a stand-alone converter
    //! @param[out] bparams common bridge params that are encountered in @a argv are stored here
    //! @param[in] argc The number of arguments in @a argv
    //! @param[in] argv The arguments to the program
    IMODEL_BRIDGE_EXPORT static void Init(iModelBridge::Params& bparams, int argc, WCharCP argv[]);

    //! @private
    //! Helper function to initialize the common bridge Params with values that can be inferred for a unit test
    //! @param[out] bparams common bridge params that are encountered in @a argv are stored here
    IMODEL_BRIDGE_EXPORT static void InitForBeTest(iModelBridge::Params& bparams);

    //! @private
    //! Helper function to parse a command-line argument for common bridge parameters for a stand-alone converter.
    IMODEL_BRIDGE_EXPORT static iModelBridge::CmdLineArgStatus ParseCommandLineArg(iModelBridge::Params& bparams, int iarg, int argc, WCharCP argv[]);

    //! @private
    //! Helper function to parse command-line arguments for a stand-alone converter.
    //! @param[out] unrecognized all arguments that are not bridge Params or StandaloneConverterParams
    //! @param[out] bparams common bridge params that are encountered in @a argv are stored here
    //! @param[out] saparams standalone-converter-specific params that are encountered in @a argv are stored here
    //! @param[in] argc The number of arguments in @a argv
    //! @param[in] argv The arguments to the program
    //! @return non-zero error status if an @em invalid argument is encountered. Unrecognized arguments do not cause an error. They are returned in @a unrecognized.
    IMODEL_BRIDGE_EXPORT static BentleyStatus ParseCommandLine(bvector<WString>& unrecognized, iModelBridge::Params& bparams, Params& saparams, int argc, WCharCP argv[]);

    //! @private
    IMODEL_BRIDGE_EXPORT static BentleyStatus CreateIModel(BeFileNameCR imodelName, BeFileNameCR bimName, Params const& saparams);
    //! @private
    IMODEL_BRIDGE_EXPORT static BentleyStatus ExtractFromIModel(BeFileName& outFile, BeFileNameCR imodelFile);
    //! @private
    IMODEL_BRIDGE_EXPORT static BentleyStatus CreateOrUpdateBim(iModelBridge& bridge, Params const& saparams);

public:
    //! Initialize the DgnPlatform host
    //! @param[in] bridge           The bridge that will be run.
    //! @param[in] productName      Name of the product.  This will be used for the LastEditor field in the bim file's be_prop table.
    IMODEL_BRIDGE_EXPORT static void InitializeHost(iModelBridge& bridge, Utf8CP productName="");

    //! Tell the specified bridge to update an existing BIM or to create a new one. This is called by stand-alone converters only.
    //! @param[in] bridge           The bridge to run
    //! @param[in] saparams         Standalone-converter-specific command parameters
    //! @return non-zero error status if the db could not be created or updated. 
    IMODEL_BRIDGE_EXPORT static BentleyStatus Execute(iModelBridge& bridge, Params const& saparams);

    private:
    //! @private
    //! Make sure bparams.m_inputFileName is a fully qualified file name.
    BentleyStatus static FixInputFileName(iModelBridge::Params& bparams);

    //! @private
    //! Make sure bparams.m_briefcaseName is a fully qualified file name. If the input was a directory name,
    //! then the input file's basename is used. The file extension is defaulted to iModelBridge::str_BriefcaseExt().
    BentleyStatus static FixBriefcaseName(iModelBridge::Params& bparams);

    public:
    //! Helper function to parse command-line arguments for a stand-alone converter. 
    //! This function first tries the arguments as either iModelBridge::Params or a iModelBridgeSacAdapter::Params.
    //! All arguments that are not accepted by either of them are passed to _ParseCommandLine/_ParseCommandLineArg.
    //! @param[inout] bridge the bridge whose parameters should be defined
    //! @param[out] saparams standalone-converter-specific params that are encountered in @a argv are stored here
    //! @param[in] argc The number of arguments in @a argv
    //! @param[in] argv The arguments to the program
    //! @return non-zero error status if an invalid or unrecognized argument is encountered.
    IMODEL_BRIDGE_EXPORT static BentleyStatus ParseCommandLine(iModelBridge& bridge, Params& saparams, int argc, WCharCP argv[]);

    //! @private
    //! Helper function to print a help message describing command-line arguments for a stand-alone converter.
    //! @param[in] bridge the bridge, which may its own print custom usage message.
    //! @param[in] argc The number of arguments in @a argv
    //! @param[in] argv The arguments to the program
    IMODEL_BRIDGE_EXPORT static void PrintCommandLineUsage(iModelBridge& bridge, int argc, WCharCP argv[]);

    //! Helper function to initialize the CRT for a stand-alone converter, including assertion failure handling and the clocale.
    IMODEL_BRIDGE_EXPORT static void InitCrt(bool quietAsserts);

    //! @private
    IMODEL_BRIDGE_EXPORT static void ParseCommandLineForBeTest(iModelBridge& bridge, bvector<bpair<WString,WString>> const& argPairs);

    //! @}
    };

END_BENTLEY_DGN_NAMESPACE
