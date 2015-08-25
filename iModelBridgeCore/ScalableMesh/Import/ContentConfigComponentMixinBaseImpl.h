/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/ContentConfigComponentMixinBaseImpl.h $
|    $RCSfile: ContentConfigComponentMixinBaseImpl.h,v $
|   $Revision: 1.5 $
|       $Date: 2011/10/21 17:32:30 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableMesh/Import/Config/Content/Base.h>
#include <ScalableMesh/Import/ContentConfigVisitor.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ComponentT>
ContentConfigComponentBase::ClassID ContentConfigComponentMixinBase<ComponentT>::_GetClassID () const
    {
    return s_GetClassID();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ComponentT>
ContentConfigComponentBase* ContentConfigComponentMixinBase<ComponentT>::_Clone () const
    {
    return new ComponentT(static_cast<const ComponentT&>(*this));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ComponentT>
void ContentConfigComponentMixinBase<ComponentT>::_Accept (IContentConfigVisitor& visitor) const
    {
    visitor._Visit(static_cast<const ComponentT&>(*this));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ComponentT>
void ContentConfigComponentMixinBase<ComponentT>::_Accept (ILayerConfigVisitor& visitor) const
    {
    visitor._Visit(static_cast<const ComponentT&>(*this));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <>
inline void ContentConfigComponentMixinBase<LayerConfig>::_Accept (ILayerConfigVisitor& visitor) const
    {
    assert(!"Not supported!"); // NTERAY: Throw instead?
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ComponentT>
ContentConfigComponentMixinBase<ComponentT>::ContentConfigComponentMixinBase ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ComponentT>
ContentConfigComponentMixinBase<ComponentT>::~ContentConfigComponentMixinBase ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ComponentT>
ContentConfigComponentMixinBase<ComponentT>::ContentConfigComponentMixinBase (const ContentConfigComponentMixinBase& rhs)
    :   ContentConfigComponentBase(rhs) 
    {
    }


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
