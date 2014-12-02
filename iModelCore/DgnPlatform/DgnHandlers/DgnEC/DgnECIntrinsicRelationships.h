/*--------------------------------------------------------------------------------------+ 
|
|     $Source: DgnHandlers/DgnEC/DgnECIntrinsicRelationships.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/
USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   11/11
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnECIntrinsicRelationshipEnabler : ECEnabler
    {
private:
    DgnECIntrinsicRelationshipEnabler (ECRelationshipClassCR relClass) : ECEnabler (relClass, NULL) { }

protected:
    virtual WCharCP                 _GetName() const override { return L"OnSameElementEnabler"; }
    virtual ECObjectsStatus         _GetPropertyIndex (UInt32& propertyIndex, WCharCP propertyAccessString) const override { return ECOBJECTS_STATUS_OperationNotSupported; }
    virtual ECObjectsStatus         _GetAccessString  (WCharCP& propertyAccessString, UInt32 propertyIndex) const override { return ECOBJECTS_STATUS_OperationNotSupported; }
    virtual UInt32                  _GetFirstPropertyIndex (UInt32 parentIndex) const override { return 0; }
    virtual UInt32                  _GetNextPropertyIndex  (UInt32 parentIndex, UInt32 inputIndex) const override { return 0; }
    virtual UInt32                  _GetPropertyCount () const override { return 0; }
    virtual ECObjectsStatus         _GetPropertyIndices (bvector<UInt32>& indices, UInt32 parentIndex) const override { return ECOBJECTS_STATUS_OperationNotSupported; }
    virtual bool                    _HasChildProperties(UInt32) const {return false;}
public:
    static DgnECIntrinsicRelationshipEnabler* CreateEnabler (ECRelationshipClassCR relClass) { return new DgnECIntrinsicRelationshipEnabler (relClass); } 
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   11/11
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnECIntrinsicRelationshipInstance : IDgnECRelationshipInstance
    {
private:
    RefCountedPtr<DgnECIntrinsicRelationshipEnabler> m_enabler;
    DgnECInstancePtr                            m_source;
    DgnECInstancePtr                            m_target;
    
    DgnECIntrinsicRelationshipInstance (DgnECIntrinsicRelationshipEnabler& enabler, DgnECInstanceR source, DgnECInstanceR target)
        : m_enabler(&enabler), m_source(&source), m_target(&target) { }
protected:
    // IECInstance
    virtual WString             _GetInstanceId() const override;
#if WIP_DEAD_DGNEC_CODE
    virtual ECObjectsStatus     _GetValue (ECValueR v, WCharCP accessor, bool useArrayIndex, UInt32 arrayIndex) const override { return ECOBJECTS_STATUS_OperationNotSupported; }
#endif
    virtual ECObjectsStatus     _GetValue (ECValueR v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override { return ECOBJECTS_STATUS_OperationNotSupported; }
#if WIP_DEAD_DGNEC_CODE
    virtual ECObjectsStatus     _SetValue (WCharCP accessor, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex) override { return ECOBJECTS_STATUS_OperationNotSupported; }
#endif
    virtual ECObjectsStatus     _SetValue (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex) override { return ECOBJECTS_STATUS_OperationNotSupported; }
    virtual ECObjectsStatus     _InsertArrayElements (WCharCP accessor, UInt32 index, UInt32 size) override { return ECOBJECTS_STATUS_OperationNotSupported; }
    virtual ECObjectsStatus     _AddArrayElements (WCharCP accessor, UInt32 size) override { return ECOBJECTS_STATUS_OperationNotSupported; }
    virtual ECObjectsStatus     _RemoveArrayElement (WCharCP accessor, UInt32 index) override { return ECOBJECTS_STATUS_OperationNotSupported; }
    virtual ECObjectsStatus     _ClearArray (WCharCP accessor) override { return ECOBJECTS_STATUS_OperationNotSupported; }
    virtual ECEnablerCR         _GetEnabler() const override { return *m_enabler; }
    virtual bool                _IsReadOnly() const override { return true; }
    virtual WString             _ToString (WCharCP indent) const override { return L"DgnECIntrinsicRelationshipInstance::_ToString() ***NOT IMPLEMENTED***"; }
    virtual size_t              _GetOffsetToIECInstance() const override
        {
        ECN::IECInstanceP iecInstanceP   = (ECN::IECInstanceP)this;
        return size_t ((byte const*)iecInstanceP - (byte const*)this);
        }

    virtual ECN::ECObjectsStatus _GetIsPropertyNull(bool &isNull,UInt32,bool,UInt32) const override 
        {
        isNull=false;
        return ECN::ECOBJECTS_STATUS_Success;
        }
        
    // IECRelationshipInstance
    virtual void            _SetSource (IECInstanceP) override { /* nonsensical */ }
    virtual IECInstancePtr  _GetSource () const override { return m_source.get(); }
    virtual ECObjectsStatus _GetSourceOrderId (Int64&) const override { return ECOBJECTS_STATUS_OperationNotSupported; }
    virtual void            _SetTarget (IECInstanceP) override { /* nonsensical */ }
    virtual IECInstancePtr  _GetTarget () const override { return m_target.get(); }
    virtual ECObjectsStatus _GetTargetOrderId (Int64&) const override { return ECOBJECTS_STATUS_OperationNotSupported; }
    
    // IDgnECRelationshipInstance
    virtual BentleyStatus   _DeleteRelationship() override { /* nonsensical */ return ERROR; }
public:
    static DgnECIntrinsicRelationshipInstance* Create (DgnECIntrinsicRelationshipEnabler& enabler, DgnECInstanceR source, DgnECInstanceR target)
        {
        return new DgnECIntrinsicRelationshipInstance (enabler, source, target);
        }
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE