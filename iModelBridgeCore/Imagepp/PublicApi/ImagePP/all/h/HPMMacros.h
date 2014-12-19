//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPMMacros.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Macros to use when defining a class of persistent objects //
//-----------------------------------------------------------------------------
#pragma once

// Normally included by hstdcpp.h and HDllSupport.h
#ifndef _HDLLNone
#define _HDLLNone
#endif

// Macro to call inside declaration of class
#define HPM_DECLARE_CLASS_DLL(HDLL, pi_ClassID) \
    public: \
        enum { CLASS_ID = pi_ClassID }; \
        virtual HCLASS_ID GetClassID() const { return CLASS_ID; } \
        HDLL virtual bool IsCompatibleWith(HCLASS_ID pi_AncClassID) const; \
        HDLL virtual size_t GetObjectSize() const; \
        virtual void AddHFCPtr() { _internal_NotifyAdditionOfASmartPointer(); } \
        virtual void RemoveHFCPtr() { _internal_NotifyRemovalOfASmartPointer(); }

#define HPM_DECLARE_CLASS(pi_ClassID) HPM_DECLARE_CLASS_DLL(_HDLLNone, pi_ClassID)


// Macro to call inside declaration of template class
// Too have a valid ID, we need to define a specialization template with this macro.

#define HPM_DECLARE_TEMPLATE_DLL_ID(HDLL, pi_ClassID) \
    public: \
        enum { CLASS_ID = pi_ClassID }; \
        virtual HCLASS_ID GetClassID() const { return CLASS_ID; } \
        HDLL virtual bool IsCompatibleWith(HCLASS_ID pi_AncClassID) const; \
        HDLL virtual size_t GetObjectSize() const; \
        virtual void AddHFCPtr() { _internal_NotifyAdditionOfASmartPointer(); } \
        virtual void RemoveHFCPtr() { _internal_NotifyRemovalOfASmartPointer(); }

#define HPM_DECLARE_TEMPLATE()      HPM_DECLARE_TEMPLATE_DLL_ID(_HDLLNone, 0)
#define HPM_DECLARE_TEMPLATE_DLL(HDLL)  HPM_DECLARE_TEMPLATE_DLL_ID(HDLL, 0)

// Macro to call in .cpp file of class if class is not instanciable, like having
// protected constructor or pure virtual methods

#define HPM_REGISTER_ABSTRACT_CLASS(pi_ClassName, pi_Ancestor) \
    bool pi_ClassName::IsCompatibleWith(HCLASS_ID pi_ClassID) const \
    { \
        return (CLASS_ID == pi_ClassID) ? true : pi_Ancestor::IsCompatibleWith(pi_ClassID); \
    } \
    size_t pi_ClassName::GetObjectSize() const \
    { \
        static size_t s_InstanceSize = sizeof(pi_ClassName); \
        return s_InstanceSize + GetAdditionalSize(); \
    }

// ^^^^^^^^^^^^^^^^^^^^^^^
// Not for support of smart pointers just above:  Objects pointed to by smart
// pointers inherits from two classes: HPMPersistentObject and HPMShareableObject,
// but the point where inheritance from HPMShareableObject is defined is unknown
// at generic level.  Thus the dictionary must store the offset between address of object
// of type T and its part coming from HPMShareableObject.  Notice that we use a dummy
// address to calculate it.  A dummy address cannot be zero because casting a null
// address always gives another null address.




// Macro to call in .cpp file of class if class is instanciable (if it does not
// have pure virtual methods)

#define HPM_REGISTER_CLASS(pi_ClassName, pi_Ancestor) \
    HPM_REGISTER_ABSTRACT_CLASS(pi_ClassName, pi_Ancestor)

// Macro to call in .cpp, once for each instantiation of a template class
// pi_ClassID should be the same value used in the macro HPM_DECLARE_TEMPLATE_DLL_ID

#define HPM_REGISTER_TEMPLATE(pi_ParameterizedName, pi_Ancestor, pi_ClassID) \
    bool pi_ParameterizedName::IsCompatibleWith(HCLASS_ID pi_AClassID) const \
    { \
        return (CLASS_ID == pi_AClassID) ? true : pi_Ancestor::IsCompatibleWith(pi_AClassID); \
    } \
    size_t pi_ParameterizedName::GetObjectSize() const \
    { \
        static size_t s_InstanceSize = sizeof(pi_ParameterizedName); \
        return s_InstanceSize + GetAdditionalSize(); \
    }
