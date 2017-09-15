/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/iModelBridge/iModelBridgeFwkTypes.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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

struct IModelBridgeRegistry : IRefCounted
    {
    virtual bool _IsFileAssignedToBridge(BeFileNameCR fn, wchar_t const* bridgeRegSubKey) = 0;
    virtual BentleyStatus _FindBridgeInRegistry(BeFileNameR bridgeLibraryPath, BeFileNameR bridgeAssetsDir, WStringCR bridgeName) = 0;
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
    typedef void T_iModelBridge_getAffinity (BentleyApi::Dgn::iModelBridgeWithAffinity& bridgeAffinity,
                                             BentleyApi::BeFileName const& affinityLibraryPath,
                                             BentleyApi::BeFileName const& sourceFileName);
    };