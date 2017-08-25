/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/IECPresentationManager.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <ECPresentation/ECPresentationTypes.h>
#include <ECPresentation/DataSource.h>
#include <ECPresentation/NavNode.h>
#include <ECPresentation/Content.h>
#include <ECPresentation/Connection.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

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
    
private:
    NodesPathElement FindNode(BeSQLite::EC::ECDbR, NavNodeCP, NavNodeKeyCR, JsonValueCR);

//__PUBLISH_SECTION_START__
private:
    ConnectionManager m_connections;
    
protected:
    virtual ~IECPresentationManager() {}

protected:    
/** @name Navigation  
 *  @{ */
    //! Retrieves the root nodes.
    //! @see GetRootNodes
    virtual DataContainer<NavNodeCPtr> _GetRootNodes(BeSQLite::EC::ECDbR, PageOptionsCR, JsonValueCR) = 0;

    //! Retrieves the number of root nodes.
    //! @see GetRootNodesCount
    virtual size_t _GetRootNodesCount(BeSQLite::EC::ECDbR, JsonValueCR) = 0;

    //! Retrieves the child nodes.
    //! @see GetChildren
    virtual DataContainer<NavNodeCPtr> _GetChildren(BeSQLite::EC::ECDbR, NavNodeCR, PageOptionsCR, JsonValueCR) = 0;

    //! Retrieves the number of child nodes.
    //! @see GetChildrenCount
    virtual size_t _GetChildrenCount(BeSQLite::EC::ECDbR, NavNodeCR, JsonValueCR) = 0;
    
    //! @copydoc HasChild
    virtual bool _HasChild(BeSQLite::EC::ECDbR connection, NavNodeCR parentNode, NavNodeKeyCR childNodeKey, JsonValueCR extendedOptions) = 0;

    //! Retrieves the parent node of the specified node.
    //! @see GetParent
    virtual NavNodeCPtr _GetParent(BeSQLite::EC::ECDbR, NavNodeCR, JsonValueCR) = 0;

    //! Retrieves a node by ID.
    //! @see GetNode
    virtual NavNodeCPtr _GetNode(BeSQLite::EC::ECDbR, uint64_t) = 0;

    //! @copydoc NotifyNodeChecked
    virtual void _OnNodeChecked(BeSQLite::EC::ECDbR, uint64_t nodeId) = 0;
    //! @copydoc NotifyNodeUnchecked
    virtual void _OnNodeUnchecked(BeSQLite::EC::ECDbR, uint64_t nodeId) = 0;

    //! @copydoc NotifyNodeExpanded
    virtual void _OnNodeExpanded(BeSQLite::EC::ECDbR, uint64_t nodeId) = 0;
    //! @copydoc NotifyNodeCollapsed
    virtual void _OnNodeCollapsed(BeSQLite::EC::ECDbR, uint64_t nodeId) = 0;
/** @} */

/** @name Content  
 *  @{ */
    //! Get the content descriptor based on the supplied parameters.
    //! @see GetContentDescriptor
    virtual ContentDescriptorCPtr _GetContentDescriptor(BeSQLite::EC::ECDbR, Utf8CP, SelectionInfo const&, JsonValueCR) = 0;

    //! Get the content.
    //! @see GetContent
    virtual ContentCPtr _GetContent(BeSQLite::EC::ECDbR, ContentDescriptorCR, SelectionInfo const&, PageOptionsCR, JsonValueCR) = 0;

    //! Get the content set size. 
    //! @see GetContentSetSize
    virtual size_t _GetContentSetSize(BeSQLite::EC::ECDbR, ContentDescriptorCR, SelectionInfo const&, JsonValueCR) = 0;
/** @} */
    
/** @name Updating
 *  @{ */
    //! Changes an ECInstance(s) value using the specified parameters.
    //! @see SaveValueChange
    virtual bvector<ECInstanceChangeResult> _SaveValueChange(BeSQLite::EC::ECDbR, bvector<ChangedECInstanceInfo> const&, Utf8CP, ECN::ECValueCR, JsonValueCR) = 0;
/** @} */

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

    //! Get the connection manager used by this presentation manager.
    ConnectionManager const& GetConnections() const {return m_connections;}
    ConnectionManager& GetConnections() {return m_connections;}
/** @} */

/** @name Navigation  
 *  @{ */
    //! Retrieves the root nodes.
    //! @param[in] connection The connection to use for getting the nodes.
    //! @param[in] pageOptions Info about the requested page of data.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT DataContainer<NavNodeCPtr> GetRootNodes(BeSQLite::EC::ECDbR connection, PageOptionsCR pageOptions, JsonValueCR extendedOptions = Json::Value());

    //! Retrieves the number of root nodes. This number may be higher than the size of data container returned by GetRootNodes in cases
    //! when the presentation manager returns nodes in a pageable manner.
    //! @param[in] connection The connection to use for getting the nodes count.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT size_t GetRootNodesCount(BeSQLite::EC::ECDbR connection, JsonValueCR extendedOptions = Json::Value());

    //! Retrieves the child nodes.
    //! @param[in] connection The connection to use for getting the nodes.
    //! @param[in] parentNode The parent node to get the children for.
    //! @param[in] pageOptions Info about the requested page of data.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT DataContainer<NavNodeCPtr> GetChildren(BeSQLite::EC::ECDbR connection, NavNodeCR parentNode, PageOptionsCR pageOptions, JsonValueCR extendedOptions = Json::Value());

    //! Retrieves the number of child nodes. This number may be higher than the size of data container returned by GetChildren in cases
    //! when the presentation manager returns nodes in a pageable manner.
    //! @param[in] connection The connection to use for getting the nodes count.
    //! @param[in] parentNode The parent node to get the children for.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT size_t GetChildrenCount(BeSQLite::EC::ECDbR connection, NavNodeCR parentNode, JsonValueCR extendedOptions = Json::Value());
    
    //! Checks whether the specified parent node has the specified child node as its children.
    //! @param[in] connection The connection to use for checking.
    //! @param[in] parentNode The parent node whose children should be checked.
    //! @param[in] childNodeKey Key of the child node to look for.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT bool HasChild(BeSQLite::EC::ECDbR connection, NavNodeCR parentNode, NavNodeKeyCR childNodeKey, JsonValueCR extendedOptions = Json::Value());

    //! Retrieves the parent node of the specified node.
    //! @param[in] connection The connection to use for getting the node.
    //! @param[in] childNode The child node to get the parent for.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT NavNodeCPtr GetParent(BeSQLite::EC::ECDbR connection, NavNodeCR childNode, JsonValueCR extendedOptions = Json::Value());

    //! Retrieves the node with the specified node ID.
    //! @param[in] connection The connection to use for getting the node.
    //! @param[in] nodeId ID of the node to get. See @ref NavNode::GetNodeId()
    ECPRESENTATION_EXPORT NavNodeCPtr GetNode(BeSQLite::EC::ECDbR connection, uint64_t nodeId);
    
    //! Provided a path of node keys, returns a path of nodes.
    //! @param[in] connection The connection to use for getting the nodes path.
    //! @param[in] keyPath Node keys path describing the path from the root node down to the target node.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT NodesPathElement GetNodesPath(BeSQLite::EC::ECDbR connection, NavNodeKeyPath const& keyPath, JsonValueCR extendedOptions = Json::Value());

    //! Returns node paths from the provided node key paths.
    //! @param[in] connection The connection to use for getting the nodes path.
    //! @param[in] keyPaths Node key paths describing the path from the root node down to the target nodes.
    //! @param[in] markedIndex Index of the path which will be marked in the resulting path's list.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT bvector<NodesPathElement> GetNodesPath(BeSQLite::EC::ECDbR connection, bvector<NavNodeKeyPath> const& keyPaths, int64_t markedIndex, JsonValueCR extendedOptions = Json::Value());
    
    //! Mark node with the specified ID as checked.
    ECPRESENTATION_EXPORT void NotifyNodeChecked(BeSQLite::EC::ECDbR, uint64_t nodeId);
    //! Mark node with the specified ID as not checked.
    ECPRESENTATION_EXPORT void NotifyNodeUnchecked(BeSQLite::EC::ECDbR, uint64_t nodeId);

    //! Mark node with the specified ID as expanded.
    ECPRESENTATION_EXPORT void NotifyNodeExpanded(BeSQLite::EC::ECDbR, uint64_t nodeId);
    //! Mark node with the specified ID as collapsed.
    ECPRESENTATION_EXPORT void NotifyNodeCollapsed(BeSQLite::EC::ECDbR, uint64_t nodeId);
/** @} */

/** @name Content  
 *  @{ */
    //! Get the content descriptor based on the supplied parameters.
    //! @param[in] connection The connection to use for getting the content.
    //! @param[in] preferredDisplayType The display type that the content will be displayed in. See @ref ContentDisplayType.
    //! @param[in] selectionInfo Info about the selection.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT ContentDescriptorCPtr GetContentDescriptor(BeSQLite::EC::ECDbR connection, Utf8CP preferredDisplayType, SelectionInfo const& selectionInfo, JsonValueCR extendedOptions = Json::Value());

    //! Get the content.
    //! @param[in] connection The connection to use for getting the content.
    //! @param[in] descriptor The content descriptor which describes what should be included in the content and how
    //!            it should be formatted. To get the default descriptor, use @ref GetContentDescriptor.
    //! @param[in] selectionInfo Info about the selection.
    //! @param[in] pageOptions Info about the requested page of data.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT ContentCPtr GetContent(BeSQLite::EC::ECDbR connection, ContentDescriptorCR descriptor, SelectionInfo const& selectionInfo, PageOptionsCR pageOptions, JsonValueCR extendedOptions = Json::Value());

    //! Get the content set size.
    //! @param[in] connection The connection to use for getting the content.
    //! @param[in] descriptor The content descriptor which describes what should be included in the content and how
    //!            it should be formatted. To get the default descriptor, use @ref GetContentDescriptor.
    //! @param[in] selectionInfo Info about the selection.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT size_t GetContentSetSize(BeSQLite::EC::ECDbR connection, ContentDescriptorCR descriptor, SelectionInfo const& selectionInfo, JsonValueCR extendedOptions = Json::Value());
/** @} */
    
/** @name Updating
 *  @{ */
    //! Changes an ECInstance(s) value using the specified parameters.
    //! @param[in] connection The connection to change the value in.
    //! @param[in] instanceInfo Info about changed instance.
    //! @param[in] propertyAccessor Changed ECProperty accessor.
    //! @param[in] value The value to change to.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT ECInstanceChangeResult SaveValueChange(BeSQLite::EC::ECDbR connection, ChangedECInstanceInfo const& instanceInfo, Utf8CP propertyAccessor, ECN::ECValueCR value, JsonValueCR extendedOptions = Json::Value());

    //! Changes an ECInstance(s) value using the specified parameters.
    //! @param[in] connection The connection to change the value in.
    //! @param[in] instanceInfos Infos about changed instances.
    //! @param[in] propertyAccessor Changed ECProperty accessor.
    //! @param[in] value The value to change to.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT bvector<ECInstanceChangeResult> SaveValueChange(BeSQLite::EC::ECDbR connection, bvector<ChangedECInstanceInfo> const& instanceInfos, Utf8CP propertyAccessor, ECN::ECValueCR value, JsonValueCR extendedOptions = Json::Value());
    
    //! Changes an ECInstance(s) value using the specified parameters.
    //! @param[in] connection The connection to change the value in.
    //! @param[in] instanceInfo Info about changed instance.
    //! @param[in] propertyAccessor Changed ECProperty accessor.
    //! @param[in] value The value to change to.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT ECInstanceChangeResult SaveValueChange(BeSQLite::EC::ECDbR connection, ChangedECInstanceInfo const& instanceInfo, Utf8CP propertyAccessor, JsonValueCR value, JsonValueCR extendedOptions = Json::Value());

    //! Changes an ECInstance(s) value using the specified parameters.
    //! @param[in] connection The connection to change the value in.
    //! @param[in] instanceInfos Infos about changed instances.
    //! @param[in] propertyAccessor Changed ECProperty accessor.
    //! @param[in] value The value to change to.
    //! @param[in] extendedOptions Additional options which depend on the implementation of @ref IECPresentationManager.
    ECPRESENTATION_EXPORT bvector<ECInstanceChangeResult> SaveValueChange(BeSQLite::EC::ECDbR connection, bvector<ChangedECInstanceInfo> const& instanceInfos, Utf8CP propertyAccessor, JsonValueCR value, JsonValueCR extendedOptions = Json::Value());
/** @} */
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
