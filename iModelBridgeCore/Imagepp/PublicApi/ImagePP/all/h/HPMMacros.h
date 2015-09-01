//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPMMacros.h $
//:>
//:>  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Macros to use when defining a class of persistent objects //
//-----------------------------------------------------------------------------
#pragma once

// Macro to call inside declaration of class
#define HPM_DECLARE_CLASS_DLL(HDLL, pi_ClassID) \
    public: \
        enum { CLASS_ID = pi_ClassID }; \
        virtual HCLASS_ID GetClassID() const { return CLASS_ID; } \
        HDLL virtual bool IsCompatibleWith(HCLASS_ID pi_AncClassID) const; \
        HDLL virtual size_t GetObjectSize() const; \
        virtual void AddHFCPtr() { _internal_NotifyAdditionOfASmartPointer(); } \
        virtual void RemoveHFCPtr() { _internal_NotifyRemovalOfASmartPointer(); }

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


