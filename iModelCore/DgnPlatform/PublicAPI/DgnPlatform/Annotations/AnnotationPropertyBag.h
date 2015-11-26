//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/Annotations/AnnotationPropertyBag.h $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>

DGNPLATFORM_TYPEDEFS(AnnotationPropertyBag);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

//=======================================================================================
//! This base class is a specialized collection that maps an integer key to either an "integer" (Int64) or a "real" (double) value. It it meant to be used by the Annotation style and seed systems by providing a re-usable way to store style properties, which can be of varying types. It also provides an easy way to store either underlying property values, or overrides to a style with the same kind of collection. When created, this collection has no values in it; they are assumed to be default. In other words, this only stores deltas from defaults.
//! @note While you can technically map a key to both value types, this should not be done in practice.
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AnnotationPropertyBag : RefCountedBase
{
    typedef int32_t T_Key;
    typedef int64_t T_Integer;
    typedef double T_Real;

private:
    DEFINE_T_SUPER(RefCountedBase)

protected:
    bmap<T_Key, T_Integer> m_integerProperties;
    bmap<T_Key, T_Real> m_realProperties;
    
private:
    DGNPLATFORM_EXPORT void CopyFrom(AnnotationPropertyBagCR);

protected:
    virtual bool _IsIntegerProperty(T_Key) const = 0;
    virtual bool _IsRealProperty(T_Key) const = 0;
    
    DGNPLATFORM_EXPORT AnnotationPropertyBag();
    AnnotationPropertyBag(AnnotationPropertyBagCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
    AnnotationPropertyBagR operator=(AnnotationPropertyBagCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}

    DGNPLATFORM_EXPORT bool HasProperty(T_Key) const;
    DGNPLATFORM_EXPORT void ClearProperty(T_Key);
    DGNPLATFORM_EXPORT T_Integer GetIntegerProperty(T_Key) const;
    DGNPLATFORM_EXPORT void SetIntegerProperty(T_Key, T_Integer);
    DGNPLATFORM_EXPORT T_Real GetRealProperty(T_Key) const;
    DGNPLATFORM_EXPORT void SetRealProperty(T_Key, T_Real);

public:
    DGNPLATFORM_EXPORT void ClearAllProperties();
    DGNPLATFORM_EXPORT size_t ComputePropertyCount() const;
    DGNPLATFORM_EXPORT void MergeWith(AnnotationPropertyBagCR);
    DGNPLATFORM_EXPORT uint32_t GetMemSize() const { return (uint32_t)(m_integerProperties.bytes_used() + m_realProperties.bytes_used()); }
};

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
