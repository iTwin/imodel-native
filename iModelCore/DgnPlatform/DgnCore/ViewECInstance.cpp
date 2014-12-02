/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ViewECInstance.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#include <ECObjects/ECExpressionNode.h>
#include <DgnPlatform/DgnCore/ViewDisplayRules.h>

/* 
  For each type require
  -- names
  -- property index
  -- ...ECInstance class, preferrably derived from  BaseECInstance with custom _GetValue
                and AddProperties methods, in output of generated
  -- schema, enabler, and class -- 
  --  ....SymbolContext derived from InstanceListExpressionContext
*/

#define USING_NAMESPACE_BENTLEY_ECOBJECT using namespace BentleyApi::ECN;
USING_NAMESPACE_BENTLEY_ECOBJECT

enum ViewPropIdx
    {
    ViewPropIdx_FIRST=1,
    ViewPropIdx_DrawPurpose = 1,
    ViewPropIdx_MAX=1
    };

enum DisplayParamsIdx
    {
    DisplayParamsIdx_FIRST=1,
    DisplayParamsIdx_ColorIsByCell = 1,
    DisplayParamsIdx_FillColorIsByCell = 2,
    DisplayParamsIdx_StyleIsByCell = 3,
    DisplayParamsIdx_WeightIsByCell = 4,
    DisplayParamsIdx_PriorityIsByCell = 5,
    DisplayParamsIdx_Color = 6,
    DisplayParamsIdx_FillColor = 7,
    DisplayParamsIdx_Style = 8,
    DisplayParamsIdx_Weight = 9,
    DisplayParamsIdx_Priority = 10,
    DisplayParamsIdx_MAX=10
    };

enum ModelParamsIdx
    {
    ModelParamsIdx_FIRST=1,
    ModelParamsIdx_Dummy = 1,
    ModelParamsIdx_MAX=1
    };

enum ElementPropIdx
    {
    ElementPropIdx_FIRST=1,
    ElementPropIdx_Dummy = 1,
    ElementPropIdx_MAX=1
    };

typedef RefCountedPtr<struct GenericECEnabler>                  GenericECEnablerPtr;
/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct GenericECEnabler : ECEnabler
    {
private:
    UInt32          m_minIndex;
    UInt32          m_maxIndex;
    ECClassR        m_ecClass;
    WString         m_name;
    GenericECEnabler (WCharCP name, ECSchemaR schema, ECClassR ecClass, UInt32 maxIndex);

    ECSchemaPtr             m_schema;
    bvector<WCharCP>        m_accessors;

    // ECEnabler
    virtual WCharCP         _GetName() const override { return m_name.c_str(); }
    virtual ECObjectsStatus _GetPropertyIndex (UInt32& propIdx, WCharCP accessor) const override
        {
        for (UInt32 i = m_minIndex; i < m_maxIndex; i++)
            if (0 == wcscmp (accessor, m_accessors[i]))
                {
                propIdx = i;
                return ECOBJECTS_STATUS_Success;
                }

        return ECOBJECTS_STATUS_PropertyNotFound;
        }

    virtual ECObjectsStatus _GetAccessString (WCharCP& accessor, UInt32 propIdx) const override
        {
        if (propIdx < m_maxIndex && propIdx > m_minIndex)
            {
            accessor = m_accessors[propIdx];
            return ECOBJECTS_STATUS_Success;
            }

        return ECOBJECTS_STATUS_PropertyNotFound;
        }

    virtual UInt32          _GetFirstPropertyIndex (UInt32 parentIndex) const override  { return m_minIndex; }
    virtual UInt32          _GetNextPropertyIndex (UInt32 parentIndex, UInt32 inputIndex) const override   { return 0 == parentIndex && (inputIndex+1) < m_maxIndex ? inputIndex+1 : 0; }
    virtual ECObjectsStatus _GetPropertyIndices (bvector<UInt32>& indices, UInt32 parentIndex) const override
        {
        if (0 != parentIndex)
            return ECOBJECTS_STATUS_Error;

        for (UInt32 i = m_minIndex; i < m_maxIndex; i++)
            indices.push_back (i);

        return ECOBJECTS_STATUS_Success;
        }

    virtual bool            _IsPropertyReadOnly (UInt32 propIdx) const override { return true; }
    virtual bool            _HasChildProperties (UInt32 parentIndex) const override { return false; }
public:
    ~GenericECEnabler() {}
    void                    AddProperty (WCharCP name, UInt32 propIdx, PrimitiveType type);
    void                    SetMaxPropertyIndex(UInt32 max);
    static GenericECEnablerPtr     Create(WCharCP enablerName, WCharCP schemaName, WCharCP className, Int32 maxIndex)
        {
        ECSchemaPtr schema;
        ECSchema::CreateSchema (schema, schemaName, 1, 0);
        ECClassP ecClass;
        schema->CreateClass (ecClass, className);

        BeAssert(maxIndex > 0);
        return new GenericECEnabler (enablerName, *schema, *ecClass, (UInt32)maxIndex);
        }
    };


/*---------------------------------------------------------------------------------**//**
* Collect as much boilerplate as possible into one base class...
* @bsistruct                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T> struct BaseECInstance  : IECInstance
    {
protected:
    GenericECEnablerPtr         m_enabler;
    T*                          m_data;

    BaseECInstance (GenericECEnabler& enabler) : m_enabler(&enabler), m_data(nullptr) { }
    ~BaseECInstance() {}

    virtual bool                _IsReadOnly() const { return true; }
    virtual WString             _GetInstanceId() const { return L""; }
    virtual WString             _ToString (WCharCP indent) const { return L""; }
    virtual size_t              _GetOffsetToIECInstance () const { return 0; }
    virtual ECObjectsStatus     _SetValue (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex) { return ECOBJECTS_STATUS_OperationNotSupported; }
    virtual ECObjectsStatus     _InsertArrayElements (UInt32 propertyIndex, UInt32 index, UInt32 size) { return ECOBJECTS_STATUS_OperationNotSupported; }
    virtual ECObjectsStatus     _AddArrayElements (UInt32 propertyIndex, UInt32 size) { return ECOBJECTS_STATUS_OperationNotSupported; }
    virtual ECObjectsStatus     _RemoveArrayElement (UInt32 propertyIndex, UInt32 index) { return ECOBJECTS_STATUS_OperationNotSupported; }
    virtual ECObjectsStatus     _ClearArray (UInt32 propertyIndex) { return ECOBJECTS_STATUS_OperationNotSupported; }

    virtual ECEnablerCR         _GetEnabler() const { return *m_enabler; }
    virtual ECObjectsStatus     _GetIsPropertyNull (bool& v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const
        {
        ECValue ecValue;
        ECObjectsStatus status = GetValue (ecValue, propertyIndex);
        if (ECOBJECTS_STATUS_Success == status)
            v = ecValue.IsNull();
        return status;
        }
public:
    T *                 GetData() const { return m_data; }
    T*                  SetData(T*data) { T*old = m_data; m_data = data; return old;}
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    John.Gooding    10/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct ElementECInstance : BaseECInstance<ElementHandle const>
    {
private:
    ElementECInstance (GenericECEnabler& enabler) : BaseECInstance (enabler) { }

    virtual ECObjectsStatus     _GetValue (ECValueR v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override;
    void AddProperties();

public:
    static RefCountedPtr<ElementECInstance> Create (GenericECEnabler& enabler) { return new ElementECInstance (enabler); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    John.Gooding    10/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct DisplayParamsECInstance : BaseECInstance<ElemDisplayParams>
    {
private:
    DisplayParamsECInstance (GenericECEnabler& enabler) : BaseECInstance (enabler) { }

    virtual ECObjectsStatus     _GetValue (ECValueR v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override;
    void AddProperties();

public:
    static RefCountedPtr<DisplayParamsECInstance> Create (GenericECEnabler& enabler) { return new DisplayParamsECInstance (enabler); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    John.Gooding    10/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct ModelECInstance : BaseECInstance<DgnModel const>
    {
private:
    ModelECInstance (GenericECEnabler& enabler) : BaseECInstance (enabler) { }

    virtual ECObjectsStatus     _GetValue (ECValueR v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override;
    void AddProperties();

public:
    static RefCountedPtr<ModelECInstance> Create (GenericECEnabler& enabler) { return new ModelECInstance (enabler); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct ViewECInstance : BaseECInstance<ViewContext>
    {
private:
    void AddProperties();
    ViewECInstance (GenericECEnabler& enabler) : BaseECInstance (enabler) { AddProperties(); }

    virtual ECObjectsStatus     _GetValue (ECValueR v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override;

public:
    static RefCountedPtr<ViewECInstance> Create (GenericECEnabler& enabler) { return new ViewECInstance (enabler); }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
void GenericECEnabler::SetMaxPropertyIndex(UInt32 max)
    {
    m_accessors.resize(max+1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
GenericECEnabler::GenericECEnabler (WCharCP name, ECSchemaR schema, ECClassR ecClass, UInt32 maxIndex) : ECEnabler (ecClass, NULL), m_schema (&schema), m_name(name), m_maxIndex(maxIndex), m_minIndex(1), m_ecClass(ecClass)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void GenericECEnabler::AddProperty (WCharCP name, UInt32 propIdx, PrimitiveType type)
    {
    PrimitiveECPropertyP prop;
    m_ecClass.CreatePrimitiveProperty (prop, name, type);
    m_accessors[propIdx] = name;
    }

//  Generated code
//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
void ViewECInstance::AddProperties()
    {
    m_enabler->SetMaxPropertyIndex(ViewPropIdx_MAX);
    m_enabler->AddProperty (L"DrawPurpose", ViewPropIdx_DrawPurpose, PRIMITIVETYPE_Integer);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
ECObjectsStatus ViewECInstance::_GetValue (ECValueR v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const
    {
    switch(propertyIndex)
        {
        case ViewPropIdx_DrawPurpose:
            v.SetInteger((Int32)m_data->GetDrawPurpose());
            break;

        default:
            return ECOBJECTS_STATUS_OperationNotSupported;
        }

    return ECOBJECTS_STATUS_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
void DisplayParamsECInstance::AddProperties()
    {
    m_enabler->SetMaxPropertyIndex(DisplayParamsIdx_MAX);
    m_enabler->AddProperty (L"ColorIsByCell", DisplayParamsIdx_ColorIsByCell, PRIMITIVETYPE_Integer);
    m_enabler->AddProperty (L"FillColorIsByCell", DisplayParamsIdx_FillColorIsByCell, PRIMITIVETYPE_Integer);
    m_enabler->AddProperty (L"StyleIsByCell", DisplayParamsIdx_StyleIsByCell, PRIMITIVETYPE_Integer);
    m_enabler->AddProperty (L"WeightIsByCell", DisplayParamsIdx_WeightIsByCell, PRIMITIVETYPE_Integer);
    m_enabler->AddProperty (L"PriorityIsByCell", DisplayParamsIdx_PriorityIsByCell, PRIMITIVETYPE_Integer);
    m_enabler->AddProperty (L"Color", DisplayParamsIdx_Color, PRIMITIVETYPE_Integer);
    m_enabler->AddProperty (L"FillColor", DisplayParamsIdx_FillColor, PRIMITIVETYPE_Integer);
    m_enabler->AddProperty (L"Style", DisplayParamsIdx_Style, PRIMITIVETYPE_Integer);
    m_enabler->AddProperty (L"Weight", DisplayParamsIdx_Weight, PRIMITIVETYPE_Integer);
    m_enabler->AddProperty (L"Priority", DisplayParamsIdx_Priority, PRIMITIVETYPE_Integer);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
ECObjectsStatus DisplayParamsECInstance::_GetValue (ECValueR v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const
    {
    switch(propertyIndex)
        {
        case DisplayParamsIdx_ColorIsByCell:
            v.SetInteger(COLOR_BYCELL == m_data->GetLineColorTBGR());
            break;

        case DisplayParamsIdx_FillColorIsByCell:
            v.SetInteger(COLOR_BYCELL == m_data->GetFillColorTBGR());
            break;

        case DisplayParamsIdx_StyleIsByCell:
            v.SetInteger(STYLE_BYCELL == m_data->GetLineStyle());
            break;

        case DisplayParamsIdx_WeightIsByCell:
            v.SetInteger(WEIGHT_BYCELL == m_data->GetWeight());
            break;

        case DisplayParamsIdx_PriorityIsByCell:
            v.SetInteger(DISPLAYPRIORITY_BYCELL == m_data->GetElementDisplayPriority());
            break;

        case DisplayParamsIdx_Color:
            v.SetInteger(m_data->GetLineColorTBGR());
            break;

        case DisplayParamsIdx_FillColor:
            v.SetInteger(m_data->GetFillColorTBGR());
            break;

        case DisplayParamsIdx_Style:
            v.SetInteger(m_data->GetLineStyle());
            break;

        case DisplayParamsIdx_Weight:
            v.SetInteger(m_data->GetWeight());
            break;

        case DisplayParamsIdx_Priority:
            v.SetInteger(m_data->GetElementDisplayPriority());
            break;

        default:
            return ECOBJECTS_STATUS_OperationNotSupported;
        }

    return ECOBJECTS_STATUS_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
void ModelECInstance::AddProperties()
    {
    m_enabler->AddProperty (L"Dummy", ModelParamsIdx_Dummy, PRIMITIVETYPE_Integer);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
ECObjectsStatus ModelECInstance::_GetValue (ECValueR v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const
    {
    switch(propertyIndex)
        {
        case ModelParamsIdx_Dummy:
            v.SetInteger(1);
            break;

        default:
            return ECOBJECTS_STATUS_OperationNotSupported;
        }

    return ECOBJECTS_STATUS_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
void ElementECInstance::AddProperties()
    {
    m_enabler->AddProperty (L"Dummy", ElementPropIdx_Dummy, PRIMITIVETYPE_Integer);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
ECObjectsStatus ElementECInstance::_GetValue (ECValueR v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const
    {
    switch(propertyIndex)
        {
        case ElementPropIdx_Dummy:
            v.SetInteger(1);
            break;

        default:
            return ECOBJECTS_STATUS_OperationNotSupported;
        }

    return ECOBJECTS_STATUS_Success;
    }
