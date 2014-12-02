//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/AnnotationPropertyBag.h $
//  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>

DGNPLATFORM_TYPEDEFS(AnnotationPropertyBag);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationPropertyBag : public RefCountedBase
{
    typedef Int32 T_Key;
    typedef Int64 T_Integer;
    typedef double T_Real;

//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(RefCountedBase)

protected:
    bmap<T_Key, T_Integer> m_integerProperties;
    bmap<T_Key, T_Real> m_realProperties;
    
private:
    void CopyFrom(AnnotationPropertyBagCR);

protected:
    virtual bool _IsIntegerProperty(T_Key) const = 0;
    virtual bool _IsRealProperty(T_Key) const = 0;
    
    AnnotationPropertyBag();
    AnnotationPropertyBag(AnnotationPropertyBagCR);
    AnnotationPropertyBagR operator=(AnnotationPropertyBagCR);

    bool HasProperty(T_Key) const;
    void ClearProperty(T_Key);
    T_Integer GetIntegerProperty(T_Key) const;
    void SetIntegerProperty(T_Key, T_Integer);
    T_Real GetRealProperty(T_Key) const;
    void SetRealProperty(T_Key, T_Real);

public:
//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    DGNPLATFORM_EXPORT void ClearAllProperties();
    DGNPLATFORM_EXPORT size_t ComputePropertyCount() const;
    DGNPLATFORM_EXPORT void MergeWith(AnnotationPropertyBagCR);

}; // AnnotationPropertyBag

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__
