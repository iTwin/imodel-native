/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/Import/ImportPolicy.cpp $
|    $RCSfile: ImportPolicy.cpp,v $
|   $Revision: 1.3 $
|       $Date: 2011/08/19 13:50:36 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableTerrainModelPCH.h>

#include <ScalableTerrainModel/Import/ImportPolicy.h>
#include <ScalableTerrainModel/Memory/Allocation.h>
#include <ScalableTerrainModel/Import/DataType.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE


namespace {

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DefaultTypeSelectionPolicy : public TypeSelectionPolicy
    {
private:
    virtual TypeSelectionPolicy*            _Clone                                 () const
        {
        return new DefaultTypeSelectionPolicy(*this);
        }

    virtual const DataType*                 _Select                                (const DataTypeFamily&           typeFamily,
                                                                                    const DataType*                 typesBegin,
                                                                                    const DataType*                 typesEnd) const
        {   
        return std::find (typesBegin, typesEnd, typeFamily);
        }
    };

}


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TypeSelectionPolicy::TypeSelectionPolicy ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TypeSelectionPolicy::TypeSelectionPolicy (const TypeSelectionPolicy& rhs)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TypeSelectionPolicy::~TypeSelectionPolicy   ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TypeSelectionPolicy* TypeSelectionPolicy::Clone () const
    {
    return _Clone();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataType* TypeSelectionPolicy::Select   (const DataTypeFamily&   typeFamily,
                                            const DataType*         typesBegin,
                                            const DataType*         typesEnd) const
    {
    return _Select(typeFamily, typesBegin, typesEnd);
    }



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImportPolicy::Impl : public ShareableObjectTypeTrait<Impl>::type
    {
    std::auto_ptr<MemoryAllocationPolicy>   m_memoryAllocationPolicyP;
    std::auto_ptr<TypeSelectionPolicy>      m_sourceTypeSelectorP;
    std::auto_ptr<TypeSelectionPolicy>      m_targetTypeSelectorP;
    
    explicit                            Impl                                   ()
        :   m_memoryAllocationPolicyP(new DefaultMemoryAllocator),
            m_sourceTypeSelectorP(new DefaultTypeSelectionPolicy),
            m_targetTypeSelectorP(new DefaultTypeSelectionPolicy)
        {
        }


    explicit                            Impl                                   (const MemoryAllocationPolicy&   memoryAllocPolicy,
                                                                                const TypeSelectionPolicy&      sourceTypeSelector,
                                                                                const TypeSelectionPolicy&      targetTypeSelector)
        :   m_memoryAllocationPolicyP(memoryAllocPolicy.Clone()),
            m_sourceTypeSelectorP(sourceTypeSelector.Clone()),
            m_targetTypeSelectorP(targetTypeSelector.Clone())
        {
        }


    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const TypeSelectionPolicy& ImportPolicy::GetDefaultSourceTypeSelectionPolicy ()
    {
    static const DefaultTypeSelectionPolicy INSTANCE;
    return INSTANCE;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const TypeSelectionPolicy& ImportPolicy::GetDefaultTargetTypeSelectionPolicy ()
    {
    static const DefaultTypeSelectionPolicy INSTANCE;
    return INSTANCE;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const MemoryAllocationPolicy& ImportPolicy::GetDefaultMemoryAllocationPolicy ()
    {
    static const DefaultMemoryAllocator INSTANCE;
    return INSTANCE;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportPolicy::ImportPolicy ()
    :   m_implP(new Impl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportPolicy::ImportPolicy (const MemoryAllocationPolicy&   memoryAllocationPolicy)
    :   m_implP(new Impl(memoryAllocationPolicy, GetDefaultSourceTypeSelectionPolicy(), GetDefaultTargetTypeSelectionPolicy()))
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportPolicy::ImportPolicy (const TypeSelectionPolicy& sourceTypeSelectPolicy,
                            const TypeSelectionPolicy& targetTypeSelectPolicy)
    :   m_implP(new Impl(GetDefaultMemoryAllocationPolicy(), sourceTypeSelectPolicy, targetTypeSelectPolicy))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportPolicy::ImportPolicy (const MemoryAllocationPolicy&   memoryAllocationPolicy,
                            const TypeSelectionPolicy&      sourceTypeSelectPolicy,
                            const TypeSelectionPolicy&      targetTypeSelectPolicy)
    :   m_implP(new Impl(memoryAllocationPolicy, sourceTypeSelectPolicy, targetTypeSelectPolicy))
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportPolicy::~ImportPolicy ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportPolicy::ImportPolicy (const ImportPolicy& rhs)
    :   m_implP(rhs.m_implP)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ImportPolicy& ImportPolicy::operator= (const ImportPolicy& rhs)
    {
    m_implP = rhs.m_implP;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const MemoryAllocationPolicy& ImportPolicy::GetMemoryAllocationPolicy () const
    {
    return *m_implP->m_memoryAllocationPolicyP;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const TypeSelectionPolicy& ImportPolicy::GetSourceTypeSelection () const
    {
    return *m_implP->m_sourceTypeSelectorP;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const TypeSelectionPolicy& ImportPolicy::GetTargetTypeSelection () const
    {
    return *m_implP->m_targetTypeSelectorP;
    }






END_BENTLEY_MRDTM_IMPORT_NAMESPACE