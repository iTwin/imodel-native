/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <ECPresentation/ECPresentationTypes.h>
#include <ECPresentation/DataSource.h>
#include <ECPresentation/NavNode.h>
#include <ECPresentation/Content.h>
#include <ECPresentation/KeySet.h>
#include <ECPresentation/Connection.h>
#include <ECPresentation/IECPresentationSerializer.h>
#include <ECPresentation/Localization.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC

//! A container of refcounted NavNode objects.
typedef DataContainer<NavNodeCPtr> NavNodesContainer;

//===================================================================================
// @bsiclass                                    Grigas.Petraitis            09/2019
//===================================================================================
struct PresentationRequestContext
{
private:
    std::function<void()> m_callbackOnTaskStart;
public:
    PresentationRequestContext(std::function<void()> onTaskStart = nullptr)
        : m_callbackOnTaskStart(onTaskStart)
        {}
    void OnTaskStart() const {if (m_callbackOnTaskStart) {m_callbackOnTaskStart();}}
};
DEFINE_POINTER_SUFFIX_TYPEDEFS(PresentationRequestContext)

//=======================================================================================
//! An abstract presentation manager which drives presentation components.
//! @ingroup GROUP_Presentation
// @bsiclass                                    Grigas.Petraitis                03/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IECPresentationManager : public NonCopyableClass
{
//__PUBLISH_SECTION_END__
private:
    static IECPresentationSerializer const* s_serializer;

//__PUBLISH_SECTION_START__
protected:
/** @name General */
/** @{ */
    //! Constructor.
    IECPresentationManager() {}

    virtual IConnectionManagerR _GetConnections() = 0;
/** @} */

/** @name Navigation
 *  @{ */
    //! @see GetRootNodes
    virtual folly::Future<NavNodesContainer> _GetRootNodes(ECDbCR, PageOptionsCR, JsonValueCR, PresentationRequestContextCR) = 0;

    //! @see GetRootNodesCount
    virtual folly::Future<size_t> _GetRootNodesCount(ECDbCR, JsonValueCR, PresentationRequestContextCR) = 0;

    //! @see GetChildren
    virtual folly::Future<NavNodesContainer> _GetChildren(ECDbCR, NavNodeCR, PageOptionsCR, JsonValueCR, PresentationRequestContextCR) = 0;

    //! @see GetChildrenCount
    virtual folly::Future<size_t> _GetChildrenCount(ECDbCR, NavNodeCR, JsonValueCR, PresentationRequestContextCR) = 0;

    //! Retrieves the parent node of the specified node.
    //! @see GetParent
    virtual folly::Future<NavNodeCPtr> _GetParent(ECDbCR, NavNodeCR, JsonValueCR, PresentationRequestContextCR) = 0;

    //! Retrieves a node by node key.
    //! @see GetNode
    virtual folly::Future<NavNodeCPtr> _GetNode(ECDbCR, NavNodeKeyCR, JsonValueCR, PresentationRequestContextCR) = 0;

    //! Retrieves filtered nodes.
    //! @see GetFilteredNodes
    virtual folly::Future<bvector<NavNodeCPtr>> _GetFilteredNodes(ECDbCR, Utf8CP, JsonValueCR, PresentationRequestContextCR) = 0;

    //! Retrieves filtered nodes.
    //! @see GetFilteredNodes
    virtual folly::Future<bvector<NodesPathElement>> _GetFilteredNodePaths(ECDbCR, Utf8CP, JsonValueCR, PresentationRequestContextCR) = 0;

    //! Retrieves node paths.
    //! @see GetNodePaths
    virtual folly::Future<bvector<NodesPathElement>> _GetNodePaths(ECDbCR, bvector<bvector<ECInstanceKey>> const&, int64_t, JsonValueCR, PresentationRequestContextCR) = 0;

    //! Retrieves a single node path.
    //! @see GetNodePath
    virtual folly::Future<NodesPathElement> _GetNodePath(ECDbCR, bvector<ECInstanceKey> const&, JsonValueCR, PresentationRequestContextCR) = 0;

/** @name Content
 *  @{ */
    //! Get content classes from the list of supplied input classes.
    //! @see GetContentClasses
    virtual folly::Future<bvector<SelectClassInfo>> _GetContentClasses(ECDbCR, Utf8CP, int, bvector<ECClassCP> const&, JsonValueCR, PresentationRequestContextCR) = 0;

    //! Get the content descriptor based on the supplied parameters.
    //! @see GetContentDescriptor
    virtual folly::Future<ContentDescriptorCPtr> _GetContentDescriptor(ECDbCR, Utf8CP, int, KeySetCR, SelectionInfo const*, JsonValueCR, PresentationRequestContextCR) = 0;

    //! Get the content.
    //! @see GetContent
    virtual folly::Future<ContentCPtr> _GetContent(ContentDescriptorCR, PageOptionsCR, PresentationRequestContextCR) = 0;

    //! Get the content set size.
    //! @see GetContentSetSize
    virtual folly::Future<size_t> _GetContentSetSize(ContentDescriptorCR, PresentationRequestContextCR) = 0;

    //! Get display label
    //! @see GetDisplayLabel
    virtual folly::Future<Utf8String> _GetDisplayLabel(ECDbCR, KeySetCR, JsonValueCR, PresentationRequestContextCR) = 0;
/** @} */

public:
/** @name General */
/** @{ */
    //! Virtual destructor.
    virtual ~IECPresentationManager() { }

    //! Set ECPresentation objects serializer
    ECPRESENTATION_EXPORT static void SetSerializer(IECPresentationSerializer const*);

    //! Get ECPresentation objects serializer
    static IECPresentationSerializer const& GetSerializer();

    IConnectionManagerR GetConnections() {return _GetConnections();}
/** @} */

/** @name Navigation
 *  @{ */
    //! Retrieves the root nodes.
    //! @param[in] db The db to use for getting the nodes.
    //! @param[in] pageOptions Info about the requested page of data.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT folly::Future<NavNodesContainer> GetRootNodes(ECDbCR db, PageOptionsCR pageOptions, JsonValueCR extendedOptions = Json::Value(), PresentationRequestContextCR = PresentationRequestContext());

    //! Retrieves the number of root nodes. This number may be higher than the size of data container returned by GetRootNodes in cases
    //! when the presentation manager returns nodes in a pageable manner.
    //! @param[in] db The db to use for getting the nodes count.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT folly::Future<size_t> GetRootNodesCount(ECDbCR db, JsonValueCR extendedOptions = Json::Value(), PresentationRequestContextCR = PresentationRequestContext());

    //! Retrieves the child nodes.
    //! @param[in] db The db to use for getting the nodes.
    //! @param[in] parentNode The parent node to get the children for.
    //! @param[in] pageOptions Info about the requested page of data.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT folly::Future<NavNodesContainer> GetChildren(ECDbCR db, NavNodeCR parentNode, PageOptionsCR pageOptions, JsonValueCR extendedOptions = Json::Value(), PresentationRequestContextCR = PresentationRequestContext());

    //! Retrieves the number of child nodes. This number may be higher than the size of data container returned by GetChildren in cases
    //! when the presentation manager returns nodes in a pageable manner.
    //! @param[in] db The db to use for getting the nodes count.
    //! @param[in] parentNode The parent node to get the children for.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT folly::Future<size_t> GetChildrenCount(ECDbCR db, NavNodeCR parentNode, JsonValueCR extendedOptions = Json::Value(), PresentationRequestContextCR = PresentationRequestContext());

    //! Retrieves the parent node of the specified node.
    //! @param[in] db The db to use for getting the node.
    //! @param[in] childNode The child node to get the parent for.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT folly::Future<NavNodeCPtr> GetParent(ECDbCR db, NavNodeCR childNode, JsonValueCR extendedOptions = Json::Value(), PresentationRequestContextCR = PresentationRequestContext());

    //! Retrieves the node with the specified node key.
    //! @param[in] db The db to use for getting the node.
    //! @param[in] nodeKey Key of the node to get. @ref NavNodeKey
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT folly::Future<NavNodeCPtr> GetNode(ECDbCR db, NavNodeKeyCR nodeKey, JsonValueCR extendedOptions = Json::Value(), PresentationRequestContextCR = PresentationRequestContext());

    //! Provided a path of node keys, returns a path of nodes.
    //! @param[in] db The db to use for getting the nodes path.
    //! @param[in] keyPath ECInstanceKey path describing the path from the root node down to the target node.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT folly::Future<NodesPathElement> GetNodePath(ECDbCR db, bvector<ECInstanceKey> const& keyPath, JsonValueCR extendedOptions = Json::Value(), PresentationRequestContextCR = PresentationRequestContext());

    //! Returns node paths from the provided node key paths.
    //! @param[in] db The db to use for getting the nodes path.
    //! @param[in] keyPaths ECInstanceKey paths describing paths from the root node down to the target nodes.
    //! @param[in] markedIndex Index of the path which will be marked in the resulting path's list.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT folly::Future<bvector<NodesPathElement>> GetNodePaths(ECDbCR db, bvector<bvector<ECInstanceKey>> const& keyPaths, int64_t markedIndex, JsonValueCR extendedOptions = Json::Value(), PresentationRequestContextCR = PresentationRequestContext());

    //! Returns filtered node paths
    //! @param[in] db The db to use for getting the nodes path.
    //! @param[in] filterText The Text to filter nodes by.
    //! @param[in] options Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT folly::Future<bvector<NodesPathElement>> GetFilteredNodePaths(ECDbCR db, Utf8CP filterText, JsonValueCR options = Json::Value(), PresentationRequestContextCR = PresentationRequestContext());

    //! Returns filtered nodes
    //! @param[in] db The db to use for getting the nodes path.
    //! @param[in] filterText The Text to filter nodes by.
    //! @param[in] options Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT folly::Future<bvector<NavNodeCPtr>> GetFilteredNodes(ECDbCR db, Utf8CP filterText, JsonValueCR options = Json::Value(), PresentationRequestContextCR = PresentationRequestContext());
/** @} */

/** @name Content
 *  @{ */
    //! Get content classes from the list of supplied input classes.
    //! @param[in] db The db to use for getting the content.
    //! @param[in] preferredDisplayType The display type that the content will be displayed in. See @ref ContentDisplayType.
    //! @param[in] contentFlags Content flags to use when generating content. If `0` is supplied, default content flags based on `preferredDisplayType` are used.
    //! @param[in] inputClasses Input classes to get content classes for.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT folly::Future<bvector<SelectClassInfo>> GetContentClasses(ECDbCR db, Utf8CP preferredDisplayType, int contentFlags, bvector<ECClassCP> const& inputClasses, JsonValueCR extendedOptions = Json::Value(), PresentationRequestContextCR = PresentationRequestContext());

    //! Get the content descriptor based on the supplied parameters.
    //! @param[in] db The db to use for getting the content.
    //! @param[in] preferredDisplayType The display type that the content will be displayed in. See @ref ContentDisplayType.
    //! @param[in] contentFlags Content flags to use when generating content. If `0` is supplied, default content flags based on `preferredDisplayType` are used.
    //! @param[in] inputKeys The keys set to get content descriptor for.
    //! @param[in] selectionInfo Info about the selection.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT folly::Future<ContentDescriptorCPtr> GetContentDescriptor(ECDbCR db, Utf8CP preferredDisplayType, int contentFlags, KeySetCR inputKeys, SelectionInfo const* selectionInfo, JsonValueCR extendedOptions = Json::Value(), PresentationRequestContextCR = PresentationRequestContext());

    //! Get the content.
    //! @param[in] descriptor The content descriptor which describes what should be included in the content and how
    //!            it should be formatted. To get the default descriptor, use @ref GetContentDescriptor.
    //! @param[in] pageOptions Info about the requested page of data.
    ECPRESENTATION_EXPORT folly::Future<ContentCPtr> GetContent(ContentDescriptorCR descriptor, PageOptionsCR pageOptions, PresentationRequestContextCR = PresentationRequestContext());

    //! Get the content set size.
    //! @param[in] descriptor The content descriptor which describes what should be included in the content and how
    //!            it should be formatted. To get the default descriptor, use @ref GetContentDescriptor.
    ECPRESENTATION_EXPORT folly::Future<size_t> GetContentSetSize(ContentDescriptorCR descriptor, PresentationRequestContextCR = PresentationRequestContext());

    //! Get display label of specific ECInstance
    //! @param[in] db The db to use for getting the label.
    //! @param[in] key Key of ECInstance to get the label for.
    ECPRESENTATION_EXPORT folly::Future<Utf8String> GetDisplayLabel(ECDbCR db, ECInstanceKeyCR key, JsonValueCR extendedOptions = Json::Value(), PresentationRequestContextCR = PresentationRequestContext());

    //! Get aggregated display label of multiple ECInstances
    //! @param[in] db The db to use for getting the label.
    //! @param[in] keys Set of ECInstance keys to get the label for.
    ECPRESENTATION_EXPORT folly::Future<Utf8String> GetDisplayLabel(ECDbCR db, KeySetCR keys, JsonValueCR extendedOptions = Json::Value(), PresentationRequestContextCR = PresentationRequestContext());
/** @} */
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
