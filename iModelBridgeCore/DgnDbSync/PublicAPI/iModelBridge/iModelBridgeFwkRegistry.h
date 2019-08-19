/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <Bentley/RefCounted.h>
#include <Bentley/WString.h>
#include <Bentley/BeFileName.h>

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

//! The bridge's affinity to some source file.
enum iModelBridgeAffinityLevel {None, Low, Medium, High, ExactMatch, Override};

//! Identifies a bridge that has some affinity to a requested source file
struct iModelBridgeWithAffinity
    {
    WString m_bridgeRegSubKey;              //!< The @ref ANCHOR_BridgeRegistration "subkey" that identifies the bridge in the registry.
    iModelBridgeAffinityLevel m_affinity;   //!< The affinity of the bridge for the requested source file.
    iModelBridgeWithAffinity() : m_affinity(iModelBridgeAffinityLevel::None) {}
    };


//=======================================================================================
// @bsiclass                                                    Sam.Wilson   02/15
//=======================================================================================
struct iModelBridgeRegistryUtils
    {
    static void InitCrt(bool quietAsserts);
    static void* GetBridgeFunction(BeFileNameCR bridgeDllName, Utf8CP funcName);
    static int ComputeAffinityMain(int argc, WCharCP argv[]);
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

/*! \typedef typedef bool T_iModelBridge_isMyFile(BentleyApi::WCharP buffer, const size_t bufferSize, BentleyApi::Dgn::iModelBridgeAffinityLevel& affinityLevel, void* dgnFileP);
 *  \brief The signature of the <code>iModelBridge_isMyFile</code> function that a shared library can implement.  The base DgnV8Bridge will call this method, if it exists, to ask the bridge if it can handle
 *  the given DgnFileP.  In this way, any bridge that is based on the DgnV8Bridge does not need to handle opening the DgnFile and confirming it is a valid V8 file.
 *  \param[out] buffer              Return the name of the bridge if it can handle this file
 *  \param[in] bufferSize           Size of the buffer for storing the bridge name
 *  \param[in] affinityLibraryPath  The full path to the affinity library that implements this function. This is a convenience, in case this function needs to locate assets relative to itself.
 *  \param[in] sourceFIleName       The name of the source file to check
 *  \param[in] dgnFileP             DgnFileP that points to an open and validated dgn file.
 * @ingroup GROUP_iModelBridge
*/
extern "C"
    {
    typedef void T_iModelBridge_getAffinity (BentleyApi::WCharP buffer,
                                             const size_t bufferSize,
                                             BentleyApi::Dgn::iModelBridgeAffinityLevel& affinityLevel,
                                             BentleyApi::WCharCP affinityLibraryPath,
                                             BentleyApi::WCharCP sourceFileName);

    typedef bool T_iModelBridge_isMyFile(BentleyApi::WCharP buffer,
                                         const size_t bufferSize,
                                         BentleyApi::Dgn::iModelBridgeAffinityLevel& affinityLevel,
                                         void* dgnFileP);
    };

#ifdef __IMODEL_BRIDGE_FWK_BUILD__
#define IMODEL_BRIDGE_FWK_EXPORT EXPORT_ATTRIBUTE
#else
#define IMODEL_BRIDGE_FWK_EXPORT IMPORT_ATTRIBUTE
#endif
