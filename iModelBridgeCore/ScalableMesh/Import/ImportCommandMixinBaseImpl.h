/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/ImportCommandMixinBaseImpl.h $
|    $RCSfile: ImportCommandMixinBaseImpl.h,v $
|   $Revision: 1.4 $
|       $Date: 2011/10/21 17:32:25 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableMesh/Import/Command/Base.h>
#include <ScalableMesh/Import/ImportSequenceVisitor.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ComponentT>
ImportCommandBase::ClassID ImportCommandMixinBase<ComponentT>::_GetClassID () const
    {
    return s_GetClassID();
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ComponentT>
ImportCommandBase* ImportCommandMixinBase<ComponentT>::_Clone () const
    {
    return new ComponentT(static_cast<const ComponentT&>(*this));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ComponentT>
void ImportCommandMixinBase<ComponentT>::_Accept (IImportSequenceVisitor& visitor) const
    {
    visitor._Visit(static_cast<const ComponentT&>(*this));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ComponentT>
ImportCommandMixinBase<ComponentT>::ImportCommandMixinBase ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ComponentT>
ImportCommandMixinBase<ComponentT>::~ImportCommandMixinBase ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ComponentT>
ImportCommandMixinBase<ComponentT>::ImportCommandMixinBase (const ImportCommandMixinBase& rhs)
    :   ImportCommandBase(rhs) 
    {
    }


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
