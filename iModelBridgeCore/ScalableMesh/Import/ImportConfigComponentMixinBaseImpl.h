/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/ImportConfigComponentMixinBaseImpl.h $
|    $RCSfile: ImportConfigComponentMixinBaseImpl.h,v $
|   $Revision: 1.4 $
|       $Date: 2011/10/21 17:32:22 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableMesh/Import/Config/Base.h>
#include <ScalableMesh/Import/ImportConfigVisitor.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ComponentT>
ImportConfigComponentBase::ClassID ImportConfigComponentMixinBase<ComponentT>::_GetClassID () const
    {
    return s_GetClassID();
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ComponentT>
ImportConfigComponentBase* ImportConfigComponentMixinBase<ComponentT>::_Clone () const
    {
    return new ComponentT(static_cast<const ComponentT&>(*this));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ComponentT>
void ImportConfigComponentMixinBase<ComponentT>::_Accept (IImportConfigVisitor& visitor) const
    {
    visitor._Visit(static_cast<const ComponentT&>(*this));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ComponentT>
ImportConfigComponentMixinBase<ComponentT>::ImportConfigComponentMixinBase ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ComponentT>
ImportConfigComponentMixinBase<ComponentT>::~ImportConfigComponentMixinBase ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ComponentT>
ImportConfigComponentMixinBase<ComponentT>::ImportConfigComponentMixinBase (const ImportConfigComponentMixinBase& rhs)
    :   ImportConfigComponentBase(rhs) 
    {
    }


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
