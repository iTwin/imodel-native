/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <DgnPlatform/ECSqlClassParams.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// Implements ECDBuffer on the EC property data buffer held by a DgnElement.
//
//    *** ONLY FOR FOR AUTO-HANDLED PROPERTY I/O AND SOME INTERNAL OPERATIONS ***
//
//  !! Misuse of this class could potentially bypass validation and violate the API guarantees!
//  !! In particular, this interface does NOT call m_element._Get/SetPropertyValue. 
//  !! It reads and writes the ECDBuffer memory directly. That is a valid and necessary
//  !! optimization for proeprty load/store and for internal, low-level property access.
//
// ***TRICKY*** ElementAutoHandledPropertiesECInstanceAdapter objects are ALWAYs put on the stack 
//              They are never ref-counted. However, since this class inherits from IECInstance
//              it appears to be ref-counted, and some EC functions will add a reference to instances
//              of this class as a result. We defeat that by always adding an initial reference
//              to an instance in the constructor.
//
// @See ElementECInstanceAdapter
// @bsiclass                                                     Sam.Wilson        10/16
//=======================================================================================
struct ElementAutoHandledPropertiesECInstanceAdapter : ECN::ECDBuffer, ECN::IECInstance
{
    friend struct ElementECPropertyAccessor;

private:
    DgnElement& m_element;
    ECClassCP m_eclass;
    ECN::ClassLayoutCP m_layout;

    void Init(bool loadProperties, size_t initialAllocation);
    void AllocateBuffer(size_t);
    uint32_t GetBytesUsed() const;

    void SetDirty();
    BentleyStatus LoadProperties();

    IECInstanceP        _GetAsIECInstance() const;

    // ECDBuffer:
    ECN::StructArrayEntry const* GetAddressOfStructArrayEntry(StructValueIdentifier key) const;
    ECN::StructValueIdentifier GetMaxStructValueIdentifier() const;
    ECN::ECObjectsStatus RemoveStructArrayEntry(ECN::StructValueIdentifier structValueId);

    ECN::PrimitiveType _GetStructArrayPrimitiveType() const override {return ECN::PrimitiveType::PRIMITIVETYPE_Integer;}
    ECN::ECObjectsStatus _SetStructArrayValueToMemory(ECN::ECValueCR v, ECN::PropertyLayoutCR propertyLayout, uint32_t index) override;
    ECN::ECObjectsStatus _GetStructArrayValueFromMemory(ECN::ECValueR v, ECN::PropertyLayoutCR propertyLayout, uint32_t index) const override;
    ECN::ECObjectsStatus _RemoveStructArrayElementsFromMemory(ECN::PropertyLayoutCR propertyLayout, uint32_t removeIndex, uint32_t removeCount) override;
    bool _IsStructValidForArray(ECN::IECInstanceCR structInstance, ECN::PropertyLayoutCR propLayout) const override;
    void _SetPerPropertyFlag(ECN::PropertyLayoutCR propertyLayout, bool, uint32_t, int flagIndex, bool enable) override { BeDataAssert(false); }
    ECN::ECObjectsStatus _EvaluateCalculatedProperty(ECN::ECValueR evaluatedValue, ECN::ECValueCR existingValue, ECN::PropertyLayoutCR propLayout) const override { BeDataAssert(false); return ECN::ECObjectsStatus::Error; }
    ECN::ECObjectsStatus _UpdateCalculatedPropertyDependents(ECN::ECValueCR calculatedValue, ECN::PropertyLayoutCR propLayout) override;

    bool _AcquireData(bool) const override 
        {
        BeAssert((DgnElement::PropState::Unknown != m_element.m_flags.m_propState) && (DgnElement::PropState::NotFound != m_element.m_flags.m_propState));
        return true;
        }
    bool _ReleaseData() const override { return true; }
    bool _IsMemoryInitialized() const override { return (nullptr != m_element.m_ecPropertyData); }
    Byte const * _GetData() const override { return m_element.m_ecPropertyData; }
    bool _AllowWritingDirectlyToInstanceMemory() const override { return false; } // I need to call SetDirty in _MoveData and _ModifyData
    bool _AreAllPropertiesCalculated() const override { return true; }
    void _SetAllPropertiesCalculated(bool) override {}
    uint32_t _GetBytesAllocated() const override { return m_element.m_ecPropertyDataSize; }
    ECN::ECObjectsStatus _ModifyData(uint32_t offset, void const * newData, uint32_t dataLength) override;
    ECN::ECObjectsStatus _MoveData(uint32_t toOffset, uint32_t fromOffset, uint32_t dataLength) override;
    ECN::ECObjectsStatus _GrowAllocation(uint32_t additionalBytesNeeded) override;
    ECN::ECObjectsStatus _ShrinkAllocation() override;
    void _FreeAllocation() override;
    void _ClearValues() override;
    ECN::ECObjectsStatus _CopyFromBuffer(ECN::ECDBufferCR source) override;
    ECN::ClassLayoutCR _GetClassLayout() const override {return *m_layout;}

    // IECInstance 
    ECDBuffer*      _GetECDBuffer() const override {return const_cast<ElementAutoHandledPropertiesECInstanceAdapter*>(this);}
    Utf8String      _GetInstanceId() const override {return m_element.GetElementId().ToString();}
    ECObjectsStatus _GetIsPropertyNull (bool& isNull, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const override
        {return GetIsNullValueFromMemory (isNull, propertyIndex, useArrayIndex, arrayIndex);}
    ECObjectsStatus _GetValue (ECValueR v, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const override 
        {return GetValueFromMemory(v, propertyIndex, useArrayIndex, arrayIndex);}
    ECObjectsStatus _SetValue (uint32_t propertyIndex, ECValueCR v, bool useArrayIndex, uint32_t arrayIndex) override
        {return SetValueToMemory(propertyIndex, v, useArrayIndex, arrayIndex);}
    ECObjectsStatus _InsertArrayElements (uint32_t propertyIndex, uint32_t index, uint32_t size) override
        {SetDirty(); return InsertNullArrayElementsAt(propertyIndex, index, size);}
    ECObjectsStatus _AddArrayElements (uint32_t propertyIndex, uint32_t size) override
        {SetDirty(); return AddNullArrayElementsAt(propertyIndex, size);}
    ECObjectsStatus _RemoveArrayElement (uint32_t propertyIndex, uint32_t index) override
        {SetDirty(); return RemoveArrayElementsAt(propertyIndex, index, 1);}
    ECObjectsStatus _ClearArray (uint32_t propIdx) override;
    ECObjectsStatus _GetShouldSerializeProperty(bool& serialize, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const override
        {return GetShouldSerializeProperty(serialize, propertyIndex, useArrayIndex, arrayIndex);}

    ECEnablerCR     _GetEnabler() const override {return *(m_eclass->GetDefaultStandaloneEnabler());}
    bool            _IsReadOnly() const override {return false;}
    Utf8String      _ToString (Utf8CP indent) const override {return "";}
    size_t          _GetOffsetToIECInstance () const override {return 0;} // WIP_AUTO_HANDLED_PROPERTIES -- what is this??

public:
    ElementAutoHandledPropertiesECInstanceAdapter(DgnElement const&, bool loadProperties, size_t initialAllocation = 0);
    ElementAutoHandledPropertiesECInstanceAdapter(DgnElement const&, ECClassCR, ECN::ClassLayoutCR, bool loadProperties, size_t initialAllocation = 0);
    
    bool IsValid() const {return (nullptr != m_eclass) && (DgnElement::PropState::NotFound != m_element.m_flags.m_propState);}

    BeSQLite::EC::ECInstanceUpdater* GetUpdater();
    DgnDbStatus UpdateProperties();

    ECObjectsStatus InsertArrayElements (uint32_t propertyIndex, uint32_t index, uint32_t size)
        {return _InsertArrayElements(propertyIndex, index, size);}
    ECObjectsStatus AddArrayElements (uint32_t propertyIndex, uint32_t size)
        {return _AddArrayElements(propertyIndex, size);}
    ECObjectsStatus RemoveArrayElement (uint32_t propertyIndex, uint32_t index)
        {return _RemoveArrayElement(propertyIndex, index);}
    ECObjectsStatus ClearArray (uint32_t propIdx)
        {return _ClearArray(propIdx);}
};

//=======================================================================================
//! Makes an element look and act like an ECInstance. Provides access to all of an element's
//! properties, including both auto- and custom-handled properties. Calls _GetProperyValue
//! and _SetPropertyValue, so that the element class can impose validation rules and other
//! custom access logic.
//! @bsiclass                                                     Sam.Wilson        10/16
//=======================================================================================
struct ElementECInstanceAdapter : ECN::IECInstance
{
private:
    DgnElementR m_element;
    ECClassCP   m_eclass;
    bool        m_readOnly;

public:
    ElementECInstanceAdapter(DgnElementR);
    ElementECInstanceAdapter(DgnElementCR el);

    ECN::ClassLayoutCR GetClassLayout() const {return m_eclass->GetDefaultStandaloneEnabler()->GetClassLayout();}
    bool ArePropertiesEqualTo(ECValuesCollectionCR expected);
    DgnDbStatus CopyPropertiesFrom(ECValuesCollectionCR source, DgnElement::SetPropertyFilter const& filter);

    ECDBuffer*      _GetECDBuffer() const override {return nullptr;} // Don't use any short cuts!
    Utf8String      _GetInstanceId() const override {return m_element.GetElementId().ToString();}
    ECObjectsStatus _GetValue (ECValueR v, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const override;
    ECObjectsStatus _SetValue (uint32_t propertyIndex, ECValueCR v, bool useArrayIndex, uint32_t arrayIndex) override;
    ECObjectsStatus _InsertArrayElements (uint32_t propertyIndex, uint32_t index, uint32_t size) override;
    ECObjectsStatus _AddArrayElements (uint32_t propertyIndex, uint32_t size) override;
    ECObjectsStatus _RemoveArrayElement (uint32_t propertyIndex, uint32_t index) override;
    ECObjectsStatus _ClearArray (uint32_t propIdx) override;
    ECEnablerCR     _GetEnabler() const override {return *(m_eclass->GetDefaultStandaloneEnabler());}
    bool            _IsReadOnly() const override {return m_readOnly;}
    Utf8String      _ToString (Utf8CP indent) const override {return "";}
    size_t          _GetOffsetToIECInstance () const override {return 0;} // WIP_AUTO_HANDLED_PROPERTIES -- what is this??
};

END_BENTLEY_DGN_NAMESPACE
