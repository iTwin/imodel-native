/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/IECPresentationManager.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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

//=======================================================================================
//! An abstract presentation manager which drives presentation controls and unified selection.
//! @ingroup GROUP_Presentation
// @bsiclass                                    Grigas.Petraitis                03/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IECPresentationManager : public NonCopyableClass
{
//__PUBLISH_SECTION_END__
private:
    static IECPresentationManager* s_instance;
    static IECPresentationSerializer const* s_serializer;
    
private:
    folly::Future<NodesPathElement> FindNode(ECDbCR, NavNodeCP, ECInstanceKeyCR, JsonValueCR);

//__PUBLISH_SECTION_START__
private:
    IConnectionManagerR m_connections;
    ILocalizationProvider const* m_localizationProvider = nullptr;
    
protected:
/** @name Navigation  
 *  @{ */
    //! @see GetRootNodes
    virtual folly::Future<NavNodesContainer> _GetRootNodes(IConnectionCR, PageOptionsCR, JsonValueCR) = 0;

    //! @see GetRootNodesCount
    virtual folly::Future<size_t> _GetRootNodesCount(IConnectionCR, JsonValueCR) = 0;

    //! @see GetChildren
    virtual folly::Future<NavNodesContainer> _GetChildren(IConnectionCR, NavNodeCR, PageOptionsCR, JsonValueCR) = 0;

    //! @see GetChildrenCount
    virtual folly::Future<size_t> _GetChildrenCount(IConnectionCR, NavNodeCR, JsonValueCR) = 0;
    
    //! Checks if node has a child with specified ECInstanceKey.
    virtual folly::Future<bool> _HasChild(IConnectionCR, NavNodeCR, ECInstanceKeyCR, JsonValueCR) = 0;

    //! Retrieves the parent node of the specified node.
    //! @see GetParent
    virtual folly::Future<NavNodeCPtr> _GetParent(IConnectionCR, NavNodeCR, JsonValueCR) = 0;

    //! Retrieves a node by node key.
    //! @see GetNode
    virtual folly::Future<NavNodeCPtr> _GetNode(IConnectionCR, NavNodeKeyCR, JsonValueCR) = 0;

    //! Retrieves filtered Node paths.
    //! @see GetFilteredNodes
    virtual folly::Future<bvector<NavNodeCPtr>> _GetFilteredNodes(IConnectionCR, Utf8CP, JsonValueCR) = 0;

    //! @see NotifyNodeChecked
    virtual folly::Future<folly::Unit> _OnNodeChecked(IConnectionCR, NavNodeKeyCR, JsonValueCR) = 0;
    //! @see NotifyNodeUnchecked
    virtual folly::Future<folly::Unit> _OnNodeUnchecked(IConnectionCR, NavNodeKeyCR, JsonValueCR) = 0;

    //! @see NotifyNodeExpanded
    virtual folly::Future<folly::Unit> _OnNodeExpanded(IConnectionCR, NavNodeKeyCR, JsonValueCR) = 0;
    //! @see NotifyNodeCollapsed
    virtual folly::Future<folly::Unit> _OnNodeCollapsed(IConnectionCR, NavNodeKeyCR, JsonValueCR) = 0;
    //! @see NotifyAllNodesCollapsed
    virtual folly::Future<folly::Unit> _OnAllNodesCollapsed(IConnectionCR, JsonValueCR) = 0;
/** @} */

/** @name Content  
 *  @{ */
    //! Get content classes from the list of supplied input classes.
    //! @see GetContentClasses
    virtual folly::Future<bvector<SelectClassInfo>> _GetContentClasses(IConnectionCR, Utf8CP, bvector<ECClassCP> const&, JsonValueCR) = 0;

    //! Get the content descriptor based on the supplied parameters.
    //! @see GetContentDescriptor
    virtual folly::Future<ContentDescriptorCPtr> _GetContentDescriptor(IConnectionCR, Utf8CP, KeySetCR, SelectionInfo const*, JsonValueCR) = 0;

    //! Get the content.
    //! @see GetContent
    virtual folly::Future<ContentCPtr> _GetContent(ContentDescriptorCR, PageOptionsCR) = 0;

    //! Get the content set size. 
    //! @see GetContentSetSize
    virtual folly::Future<size_t> _GetContentSetSize(ContentDescriptorCR) = 0;
    
    //! Get display label
    //! @see GetDisplayLabel
    virtual folly::Future<Utf8String> _GetDisplayLabel(IConnectionCR, KeySetCR) = 0;
/** @} */
    
/** @name Updating
 *  @{ */
    //! Changes an ECInstance(s) value using the specified parameters.
    //! @see SaveValueChange
    virtual folly::Future<bvector<ECInstanceChangeResult>> _SaveValueChange(IConnectionCR, bvector<ChangedECInstanceInfo> const&, Utf8CP, ECValueCR, JsonValueCR) = 0;
/** @} */

    virtual void _OnLocalizationProviderChanged() {}

public:
/** @name General */
/** @{ */
    //! Registers the PresentationManager implementation that's used as the singleton
    ECPRESENTATION_EXPORT static void RegisterImplementation(IECPresentationManager*);

    //! Check whether a registered presentation manager exists.
    ECPRESENTATION_EXPORT static bool IsActive();
    
    //! Get the presentation manager.
    //! @warning @ref IsActive() can be used to verify if a manager is registered.
    ECPRESENTATION_EXPORT static IECPresentationManagerR GetManager();
    
    //! Constructor.
    IECPresentationManager(IConnectionManagerR connections) : m_connections(connections) {}

    //! Virtual destructor.
    virtual ~IECPresentationManager() { DELETE_AND_CLEAR(m_localizationProvider); }

    //! Get the connection manager used by this presentation manager.
    IConnectionManagerCR GetConnections() const {return m_connections;}
    IConnectionManagerR Connections() { return m_connections; }

    //! Set ECPresentation objects serializer
    ECPRESENTATION_EXPORT static void SetSerializer(IECPresentationSerializer const*);

    //! Get ECPresentation objects serializer
    static IECPresentationSerializer const& GetSerializer();
    
    //! Set ECPresentation objects serializer
    ECPRESENTATION_EXPORT void SetLocalizationProvider(ILocalizationProvider const* provider);

    //! Get ECPresentation objects serializer
    ILocalizationProvider const* GetLocalizationProvider();
/** @} */

/** @name Navigation  
 *  @{ */
    //! Retrieves the root nodes.
    //! @param[in] db The db to use for getting the nodes.
    //! @param[in] pageOptions Info about the requested page of data.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT folly::Future<NavNodesContainer> GetRootNodes(ECDbCR db, PageOptionsCR pageOptions, JsonValueCR extendedOptions = Json::Value());

    //! Retrieves the number of root nodes. This number may be higher than the size of data container returned by GetRootNodes in cases
    //! when the presentation manager returns nodes in a pageable manner.
    //! @param[in] db The db to use for getting the nodes count.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT folly::Future<size_t> GetRootNodesCount(ECDbCR db, JsonValueCR extendedOptions = Json::Value());

    //! Retrieves the child nodes.
    //! @param[in] db The db to use for getting the nodes.
    //! @param[in] parentNode The parent node to get the children for.
    //! @param[in] pageOptions Info about the requested page of data.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT folly::Future<NavNodesContainer> GetChildren(ECDbCR db, NavNodeCR parentNode, PageOptionsCR pageOptions, JsonValueCR extendedOptions = Json::Value());

    //! Retrieves the number of child nodes. This number may be higher than the size of data container returned by GetChildren in cases
    //! when the presentation manager returns nodes in a pageable manner.
    //! @param[in] db The db to use for getting the nodes count.
    //! @param[in] parentNode The parent node to get the children for.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT folly::Future<size_t> GetChildrenCount(ECDbCR db, NavNodeCR parentNode, JsonValueCR extendedOptions = Json::Value());
    
    //! Retrieves the parent node of the specified node.
    //! @param[in] db The db to use for getting the node.
    //! @param[in] childNode The child node to get the parent for.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT folly::Future<NavNodeCPtr> GetParent(ECDbCR db, NavNodeCR childNode, JsonValueCR extendedOptions = Json::Value());

    //! Retrieves the node with the specified node key.
    //! @param[in] db The db to use for getting the node.
    //! @param[in] nodeKey Key of the node to get. @ref NavNodeKey
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT folly::Future<NavNodeCPtr> GetNode(ECDbCR db, NavNodeKeyCR nodeKey, JsonValueCR extendedOptions = Json::Value());
    
    //! Provided a path of node keys, returns a path of nodes.
    //! @param[in] db The db to use for getting the nodes path.
    //! @param[in] keyPath ECInstanceKey path describing the path from the root node down to the target node.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT folly::Future<NodesPathElement> GetNodesPath(ECDbCR db, bvector<ECInstanceKey> const& keyPath, JsonValueCR extendedOptions = Json::Value());

    //! Returns node paths from the provided node key paths.
    //! @param[in] db The db to use for getting the nodes path.
    //! @param[in] keyPaths ECInstanceKey paths describing paths from the root node down to the target nodes.
    //! @param[in] markedIndex Index of the path which will be marked in the resulting path's list.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT folly::Future<bvector<NodesPathElement>> GetNodesPath(ECDbCR db, bvector<bvector<ECInstanceKey>> const& keyPaths, int64_t markedIndex, JsonValueCR extendedOptions = Json::Value());
    
    //! Returns filtered nodes paths
    //! @param[in] db The db to use for getting the nodes path.
    //! @param[in] filterText The Text to filter nodes by.
    //! @param[in] options Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT folly::Future<bvector<NodesPathElement>> GetFilteredNodesPaths(ECDbCR db, Utf8CP filterText, JsonValueCR options = Json::Value());
    
    //! Mark node with the specified node key as checked.
    ECPRESENTATION_EXPORT folly::Future<folly::Unit> NotifyNodeChecked(ECDbCR, NavNodeKeyCR nodeKey, JsonValueCR extendedOptions = Json::Value());
    //! Mark node with the specified node key as not checked.
    ECPRESENTATION_EXPORT folly::Future<folly::Unit> NotifyNodeUnchecked(ECDbCR, NavNodeKeyCR nodeKey, JsonValueCR extendedOptions = Json::Value());

    //! Mark node with the specified node key as expanded.
    ECPRESENTATION_EXPORT folly::Future<folly::Unit> NotifyNodeExpanded(ECDbCR, NavNodeKeyCR nodeKey, JsonValueCR extendedOptions = Json::Value());
    //! Mark node with the specified node key as collapsed.
    ECPRESENTATION_EXPORT folly::Future<folly::Unit> NotifyNodeCollapsed(ECDbCR, NavNodeKeyCR nodeKey, JsonValueCR extendedOptions = Json::Value());
    //! Collapse all expanded nodes
    ECPRESENTATION_EXPORT folly::Future<folly::Unit> NotifyAllNodesCollapsed(ECDbCR, JsonValueCR options = Json::Value());
/** @} */

/** @name Content  
 *  @{ */
    //! Get content classes from the list of supplied input classes.
    //! @param[in] db The db to use for getting the content.
    //! @param[in] preferredDisplayType The display type that the content will be displayed in. See @ref ContentDisplayType.
    //! @param[in] inputClasses Input classes to get content classes for.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT folly::Future<bvector<SelectClassInfo>> GetContentClasses(ECDbCR db, Utf8CP preferredDisplayType, bvector<ECClassCP> const& inputClasses, JsonValueCR extendedOptions = Json::Value());

    //! Get the content descriptor based on the supplied parameters.
    //! @param[in] db The db to use for getting the content.
    //! @param[in] preferredDisplayType The display type that the content will be displayed in. See @ref ContentDisplayType.
    //! @param[in] inputKeys The keys set to get content descriptor for.
    //! @param[in] selectionInfo Info about the selection.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT folly::Future<ContentDescriptorCPtr> GetContentDescriptor(ECDbCR db, Utf8CP preferredDisplayType, KeySetCR inputKeys, SelectionInfo const* selectionInfo, JsonValueCR extendedOptions = Json::Value());

    //! Get the content.
    //! @param[in] descriptor The content descriptor which describes what should be included in the content and how
    //!            it should be formatted. To get the default descriptor, use @ref GetContentDescriptor.
    //! @param[in] pageOptions Info about the requested page of data.
    ECPRESENTATION_EXPORT folly::Future<ContentCPtr> GetContent(ContentDescriptorCR descriptor, PageOptionsCR pageOptions);

    //! Get the content set size.
    //! @param[in] descriptor The content descriptor which describes what should be included in the content and how
    //!            it should be formatted. To get the default descriptor, use @ref GetContentDescriptor.
    ECPRESENTATION_EXPORT folly::Future<size_t> GetContentSetSize(ContentDescriptorCR descriptor);
    
    //! Get display label of specific ECInstance
    //! @param[in] db The db to use for getting the label.
    //! @param[in] key Key of ECInstance to get the label for.
    ECPRESENTATION_EXPORT folly::Future<Utf8String> GetDisplayLabel(ECDbCR db, ECInstanceKeyCR key);
    
    //! Get aggregated display label of multiple ECInstances
    //! @param[in] db The db to use for getting the label.
    //! @param[in] keys Set of ECInstance keys to get the label for.
    ECPRESENTATION_EXPORT folly::Future<Utf8String> GetDisplayLabel(ECDbCR db, KeySetCR keys);
/** @} */
    
/** @name Updating
 *  @{ */
    //! Changes an ECInstance(s) value using the specified parameters.
    //! @param[in] db The db to change the value in.
    //! @param[in] instanceInfo Info about changed instance.
    //! @param[in] propertyAccessor Changed ECProperty accessor.
    //! @param[in] value The value to change to.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT folly::Future<ECInstanceChangeResult> SaveValueChange(ECDbCR db, ChangedECInstanceInfo const& instanceInfo, Utf8CP propertyAccessor, ECValueCR value, JsonValueCR extendedOptions = Json::Value());

    //! Changes an ECInstance(s) value using the specified parameters.
    //! @param[in] db The db to change the value in.
    //! @param[in] instanceInfos Infos about changed instances.
    //! @param[in] propertyAccessor Changed ECProperty accessor.
    //! @param[in] value The value to change to.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT folly::Future<bvector<ECInstanceChangeResult>> SaveValueChange(ECDbCR db, bvector<ChangedECInstanceInfo> const& instanceInfos, Utf8CP propertyAccessor, ECValueCR value, JsonValueCR extendedOptions = Json::Value());
    
    //! Changes an ECInstance(s) value using the specified parameters.
    //! @param[in] db The db to change the value in.
    //! @param[in] instanceInfo Info about changed instance.
    //! @param[in] propertyAccessor Changed ECProperty accessor.
    //! @param[in] value The value to change to.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT folly::Future<ECInstanceChangeResult> SaveValueChange(ECDbCR db, ChangedECInstanceInfo const& instanceInfo, Utf8CP propertyAccessor, JsonValueCR value, JsonValueCR extendedOptions = Json::Value());

    //! Changes an ECInstance(s) value using the specified parameters.
    //! @param[in] db The db to change the value in.
    //! @param[in] instanceInfos Infos about changed instances.
    //! @param[in] propertyAccessor Changed ECProperty accessor.
    //! @param[in] value The value to change to.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT folly::Future<bvector<ECInstanceChangeResult>> SaveValueChange(ECDbCR db, bvector<ChangedECInstanceInfo> const& instanceInfos, Utf8CP propertyAccessor, JsonValueCR value, JsonValueCR extendedOptions = Json::Value());
/** @} */
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
