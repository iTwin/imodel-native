/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/iModelBridge/iModelBridgeFwkTypes.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <Bentley/RefCounted.h>
#include <Bentley/WString.h>
#include <Bentley/BeFileName.h>
#include <BeSQLite/BeSQLite.h>

#ifndef BEGIN_BENTLEY_DGN_NAMESPACE
  #define BEGIN_BENTLEY_DGN_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace Dgn {
  #define END_BENTLEY_DGN_NAMESPACE   } END_BENTLEY_NAMESPACE
    #define USING_NAMESPACE_BENTLEY_DGN         using namespace BentleyApi::Dgn;
#endif

BEGIN_BENTLEY_DGN_NAMESPACE

//! Properties that may be assigned to a document by its home document control system (DCS).
struct iModelBridgeDocumentProperties
    {
    Utf8String m_docGuid; //!< The GUID assigned to the document
    Utf8String m_webURN; //!< The URN to use when referring to this document over the Internet
    Utf8String m_desktopURN; //!< The URN to use when referring to this document from a desktop program
    Utf8String m_attributesJSON; //!< Document attributes, in JSON format
    Utf8String m_spatialRootTransformJSON; //!< Spatial data transform for root document, in JSON format
    Utf8String m_changeHistoryJSON;
    iModelBridgeDocumentProperties() {}
    iModelBridgeDocumentProperties(Utf8CP g, Utf8CP w, Utf8CP d, Utf8CP a, Utf8CP o) : m_docGuid(g), m_webURN(w), m_desktopURN(d), m_attributesJSON(o), m_spatialRootTransformJSON(o) {}
    };


struct IModelBridgeRegistry : IRefCounted
    {
    virtual bool _IsFileAssignedToBridge(BeFileNameCR fn, wchar_t const* bridgeRegSubKey) = 0;
    virtual void _QueryAllFilesAssignedToBridge(bvector<BeFileName>& fns, wchar_t const* bridgeRegSubKey) = 0;
    virtual BentleyStatus _GetDocumentProperties(iModelBridgeDocumentProperties&, BeFileNameCR fn) = 0;
    virtual BentleyStatus _GetDocumentPropertiesByGuid(iModelBridgeDocumentProperties& props, BeFileNameR localFilePath, BeSQLite::BeGuid const& docGuid) = 0;
    virtual BentleyStatus _AssignFileToBridge(BeFileNameCR fn, wchar_t const* bridgeRegSubKey) = 0;
    virtual void          _DiscoverInstalledBridges() = 0;
    virtual BentleyStatus _FindBridgeInRegistry(BeFileNameR bridgeLibraryPath, BeFileNameR bridgeAssetsDir, WStringCR bridgeName) = 0;
    virtual ~IModelBridgeRegistry() {}
    };

//! The bridge's affinity to some source file.
enum iModelBridgeAffinityLevel {None, Low, Medium, High, ExactMatch};

//! Identifies a bridge that has some affinity to a requested source file
struct iModelBridgeWithAffinity
    {
    WString m_bridgeRegSubKey;              //!< The @ref ANCHOR_BridgeRegistration "subkey" that identifies the bridge in the registry.
    iModelBridgeAffinityLevel m_affinity;   //!< The affinity of the bridge for the requested source file.
    iModelBridgeWithAffinity() : m_affinity(iModelBridgeAffinityLevel::None) {}
    };

END_BENTLEY_DGN_NAMESPACE

/*! \typedef typedef void T_iModelBridge_getAffinity (BentleyApi::Dgn::iModelBridge::BridgeAffinity& bridgeaffinity, BentleyApi::BeFileName const& bridgeLibraryPath, BentleyApi::BeFileName const& sourceFileName);
 *  \brief The signature of the <code>iModelBridge_getAffinity</code> function that a shared library must implement in order to @ref iModelBridge_getAffinity "report the affinity of a bridge for a source document to the framework".
 *  Note that the iModelBridge_getAffinity function must have extern "C" linkage and must be exported.
 *  \param[out] bridgeAffinity      Return the bridge, if any, that could convert this source document.
 *  \param[in] affinityLibraryPath  The full path to the affinity library that implements this function. This is a convenience, in case this function needs to locate assets relative to itself.
 *  \param[in] sourceFIleName       The name of the source file to check
 *  @note If set, the value in bridgeAffinity.m_bridgeRegSubKey must match the @ref ANCHOR_BridgeRegistration "subkey" of a bridge in the registry.
 *  @note This function will be called in a separate process from the bridge. The iModelBridge_getAffinity function can (and must) do its own initialization as required to compute affinity.
 * @ingroup GROUP_iModelBridge
 */
extern "C"
    {
    typedef void T_iModelBridge_getAffinity (BentleyApi::WCharP buffer,
                                             const size_t bufferSize,
                                             BentleyApi::Dgn::iModelBridgeAffinityLevel& affinityLevel,
                                             BentleyApi::WCharCP affinityLibraryPath,
                                             BentleyApi::WCharCP sourceFileName);
    };

#ifdef __IMODEL_BRIDGE_FWK_BUILD__
#define IMODEL_BRIDGE_FWK_EXPORT EXPORT_ATTRIBUTE
#else
#define IMODEL_BRIDGE_FWK_EXPORT IMPORT_ATTRIBUTE
#endif
