/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/Import/PluginRegistryHelper.hpp $
|    $RCSfile: PluginRegistryHelper.hpp,v $
|   $Revision: 1.4 $
|       $Date: 2012/02/16 00:36:57 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PluginCreatorT>
typename const PluginRegistry<PluginCreatorT>::CreatorList& PluginRegistry<PluginCreatorT>::GetCreators () const 
    {
    if (HasPostponedUnregister())
        RunPostponedUnregister();
    return m_creators;
    } 

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PluginCreatorT>
void PluginRegistry<PluginCreatorT>::RunPostponedUnregister () const
    {
    assert(HasPostponedUnregister());

    UnregisterCreators(m_unregisteredCreators, m_creators, CompareLessCreatorsWithID());
    m_unregisteredCreators.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PluginCreatorT>
typename PluginRegistry<PluginCreatorT>::CreatorPluginID PluginRegistry<PluginCreatorT>::Register (const CreatorPlugin& creator)
    {
    if (HasPostponedUnregister())
        RunPostponedUnregister();

    m_creators.push_back(creator);
    m_creatorsSorted = false;

    return creator.GetID();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PluginCreatorT>
void PluginRegistry<PluginCreatorT>::Unregister (CreatorPluginID id)
    {
    assert(0 != id);

    m_unregisteredCreators.push_back(id);
    }


