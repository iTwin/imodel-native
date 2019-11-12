/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableTerrainModel/Import/Command/Base.h>
#include <ScalableTerrainModel/Import/ImportSequenceVisitor.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

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


END_BENTLEY_MRDTM_IMPORT_NAMESPACE