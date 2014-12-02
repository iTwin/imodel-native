/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DgnEC/DgnECMemoryBasedInstance.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

DGNPLATFORM_TYPEDEFS (DgnECMemoryBasedEnabler);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnECMemoryBasedEnabler : ClassLayoutHolder, DgnECInstanceEnabler
    {
private:
    mutable ECN::StandaloneECEnablerPtr  m_sharedWipEnabler;
    mutable ECN::StandaloneECInstancePtr m_sharedWipInstance;
    
protected:
    DgnECMemoryBasedEnabler (ECClassCR ecClass, ClassLayoutCR classLayout, IStandaloneEnablerLocaterP structLocator)
        : DgnECInstanceEnabler (ecClass, structLocator), ClassLayoutHolder (classLayout),
          m_sharedWipEnabler (StandaloneECEnabler::CreateEnabler (GetClass(), GetClassLayout(), structLocator, false)),
          m_sharedWipInstance (m_sharedWipEnabler->CreateInstance())
        {
        //
        }

    virtual ECObjectsStatus         _GetPropertyIndex (UInt32& propertyIndex, WCharCP accessString) const override
        {
        return GetClassLayout().GetPropertyIndex (propertyIndex, accessString);
        }
    virtual ECObjectsStatus         _GetAccessString (WCharCP& accessString, UInt32 propertyIndex) const override
        {
        return GetClassLayout().GetAccessStringByIndex (accessString, propertyIndex);
        }
    virtual UInt32                  _GetFirstPropertyIndex (UInt32 parentIndex) const override
        {
        return GetClassLayout().GetFirstChildPropertyIndex (parentIndex);
        }
    virtual UInt32                  _GetNextPropertyIndex (UInt32 parentIndex, UInt32 inputIndex) const override
        {
        return GetClassLayout().GetNextChildPropertyIndex (parentIndex, inputIndex);
        }
    virtual UInt32                  _GetPropertyCount() const override
        {
        return GetClassLayout().GetPropertyCount();
        }
    virtual bool                    _HasChildProperties (UInt32 parentIndex) const override
        {
        return GetClassLayout().HasChildProperties (parentIndex);
        }
    virtual ECObjectsStatus         _GetPropertyIndices (bvector<UInt32>& indices, UInt32 parentIndex) const override
        {
        return GetClassLayout().GetPropertyIndices (indices, parentIndex);
        }
    virtual ECN::StandaloneECInstanceP   _GetSharedWipInstance() const override
        {
        return m_sharedWipInstance.get();
        }
    virtual ECN::StandaloneECInstancePtr _GetPrivateWipInstance() const override
        {
        return m_sharedWipEnabler->CreateInstance();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnECMemoryBasedInstance : MemoryECInstanceBase, DgnECInstance
    {
private:
    DgnECMemoryBasedEnablerCR   m_enabler;
protected:
    DgnECMemoryBasedInstance (DgnECMemoryBasedEnablerCR enabler) 
        : MemoryECInstanceBase (enabler.GetClassLayout(), 0, true, NULL),  m_enabler (enabler)
        {
        //
        }

#if WIP_DEAD_DGNEC_CODE
    virtual ECObjectsStatus                 _GetValue (ECValueR v, WCharCP accessString, bool useArrayIndex, UInt32 arrayIndex) const override
        {
        return GetValueFromMemory (GetClassLayout(), v, accessString, useArrayIndex, arrayIndex);
        }
#endif
    virtual ECObjectsStatus                 _GetValue (ECValueR v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override
        {
        return GetValueFromMemory (v, GetClassLayout(), propertyIndex, useArrayIndex, arrayIndex);
        }
#if WIP_DEAD_DGNEC_CODE
    virtual ECObjectsStatus                 _SetValue (WCharCP accessString, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex) override
        {
        UInt32 propertyIndex = 0;
        return (ECOBJECTS_STATUS_Success == GetEnabler().GetPropertyIndex (propertyIndex, accessString)) 
            ? _SetValue (propertyIndex, v, useArrayIndex, arrayIndex) : ECOBJECTS_STATUS_PropertyNotFound;
        }
#endif
    virtual ECObjectsStatus                 _SetValue (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex) override
        {
        SetPerPropertyBit ((UInt8)PROPERTYFLAGINDEX_IsLoaded, propertyIndex, true);
        SetPerPropertyBit ((UInt8)PROPERTYFLAGINDEX_IsDirty,  propertyIndex, true);
        return SetValueToMemory (GetClassLayout(), propertyIndex, v, useArrayIndex, arrayIndex);
        }
    virtual ECObjectsStatus                 _InsertArrayElements (WCharCP accessString, UInt32 index, UInt32 size) override
        {
        return InsertNullArrayElementsAt (GetClassLayout(), accessString, index, size);
        }
    virtual ECObjectsStatus                 _AddArrayElements (WCharCP accessString, UInt32 size) override
        {
        return AddNullArrayElementsAt (GetClassLayout(), accessString, size);
        }
    virtual ECObjectsStatus                 _RemoveArrayElement (WCharCP accessString, UInt32 index) override
        {
        return RemoveArrayElementsAt (GetClassLayout(), accessString, index, 1);
        }
    virtual ECObjectsStatus                 _ClearArray (WCharCP accessString) override
        {
        return ECOBJECTS_STATUS_OperationNotSupported;
        }
    virtual ECEnablerCR                     _GetEnabler() const override
        {
        return m_enabler;
        }
    virtual DgnECInstanceEnablerCR          _GetDgnECInstanceEnabler() const override
        {
        return m_enabler;
        }
    virtual WString                         _ToString (WCharCP indent) const override
        {
        return InstanceDataToString (indent, GetClassLayout());
        }
    virtual size_t                          _GetOffsetToIECInstance() const override
        {
        IECInstanceP iecInstanceP = (IECInstanceP)this;
        byte const* baseIEC = (byte const*)iecInstanceP;
        byte const* baseThis = (byte const*)this;
        return (size_t)(baseIEC - baseThis);
        }
    virtual ClassLayoutCR                   _GetClassLayout() const override
        {
        return m_enabler.GetClassLayout();
        }
    virtual IECInstancePtr                  _GetAsIECInstance() const override
        {
        return const_cast<DgnECMemoryBasedInstance*> (this);
        }
    virtual MemoryECInstanceBase*           _GetAsMemoryECInstance () const override
        {
        return const_cast<DgnECMemoryBasedInstance*> (this);
        }
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE