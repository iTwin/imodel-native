/*--------------------------------------------------------------------------------------+ 
|
|     $Source: DgnHandlers/DgnEC/dgnecmodelprovider.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

#include    "GenericHostTypeIteration.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

const UInt16 PROVIDERID_ModelProvider =  0xEC01;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnModelBaseECInstance : DgnECInstance
    {

private:
    virtual FindInstancesScopePtr   _GetInstanceScope () const override;

    virtual DgnECHostType   _GetHostType () const override
        {
        return DgnECHostType_Model;
        }

    virtual DgnElementECInstancePtr     _GetAsElementInstance() const override
        {
        return NULL;
        }
    

public:
    virtual ECObjectsStatus     _SetValue (WCharCP managedPropertyAccessor, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex) override
        {
        BeAssert (false);
        return ECOBJECTS_STATUS_Success;
        }

protected:
    virtual ECObjectsStatus     _GetValue (ECValueR v, WCharCP managedPropertyAccessor, bool useArrayIndex, UInt32 arrayIndex) const override;

    virtual ECObjectsStatus     _SetValue (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex) override
        {
        BeAssert (false);
        return ECOBJECTS_STATUS_Success;
        }

    virtual ECObjectsStatus     _InsertArrayElements (WCharCP managedPropertyAccessor, UInt32 index, UInt32 size)
        {
        BeAssert (false);
        return ECOBJECTS_STATUS_ECInstanceImplementationNotSupported;
        }
    virtual ECObjectsStatus     _AddArrayElements (WCharCP managedPropertyAccessor, UInt32 size)
        {
        BeAssert (false);
        return ECOBJECTS_STATUS_ECInstanceImplementationNotSupported;
        }
    virtual ECObjectsStatus     _RemoveArrayElement (WCharCP managedPropertyAccessor, UInt32 index)
        {
        BeAssert (false);
        return ECOBJECTS_STATUS_ECInstanceImplementationNotSupported;
        }
    virtual ECObjectsStatus     _ClearArray (WCharCP managedPropertyAccessor)
        {
        BeAssert (false);
        return ECOBJECTS_STATUS_ECInstanceImplementationNotSupported;
        }

    virtual WString             _ToString (WCharCP indent) const override
        {
        BeAssert (false);
        return WString();
        }
    virtual size_t              _GetOffsetToIECInstance () const override
        {
        BeAssert (false);
        return 0;
        }
    public:
        virtual DgnModelR GetDgnModel () const = 0;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnModelECInstance : public DgnModelBaseECInstance
    {
    private:
        DgnModelR                       m_model;
        DgnECInstanceEnabler const&     m_enabler;

        virtual FindInstancesScopePtr   _GetRelatedInstanceScope () const override;
        virtual DgnModelP        _GetDgnModelP () const override 
            {
            return &m_model;
            }
        virtual DgnProjectP            _GetDgnFileP () const override 
            {
            return m_model.GetDgnProject();
            }
        virtual StatusInt           _ReplaceInModel () override
            { 
            BeAssert (false);
            return ERROR;
            }
        
        virtual WString             _GetInstanceId() const override;

        
        virtual ECObjectsStatus     _GetValue (ECValueR v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override;
    
    protected:
     
        virtual ECEnablerCR         _GetEnabler() const override
            {
            return m_enabler;
            }

        virtual DgnECInstanceEnablerCR  _GetDgnECInstanceEnabler() const override
            {
            return m_enabler;
            }

        virtual bool                _IsReadOnly() const 
            {
            return m_model.IsReadOnly();
            }

     DgnModelECInstance (DgnModelR model, DgnECInstanceEnablerCR enabler)
            :m_model(model), m_enabler(enabler)
            {}
    public:
        static const int MAX_PROPERTY_COUNT = 11;
        static const WCharCP PropertyNameList[MAX_PROPERTY_COUNT] ;

    public:
    
    static DgnModelECInstance* CreateInstance (DgnModelR model, DgnECInstanceEnablerCR enabler)
        {
        return new DgnModelECInstance (model, enabler);
        }

    virtual DgnModelR GetDgnModel () const override {return m_model;}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ModelBaseIntrinsicECInstanceEnabler: DgnECInstanceEnabler
    {
    protected:
    ModelBaseIntrinsicECInstanceEnabler (ECClassCR ecClass, IStandaloneEnablerLocaterP structStandaloneEnablerLocater)
            : DgnECInstanceEnabler (ecClass, structStandaloneEnablerLocater)
            {}
    virtual UInt16                  _GetProviderId() const override;
    virtual UInt32                  _GetFirstPropertyIndex (UInt32 parentIndex) const override
        {
        return 1;
        }
        
    virtual UInt32                  _GetNextPropertyIndex  (UInt32 parentIndex, UInt32 inputIndex) const override;
    virtual bool                    _HasChildProperties (UInt32 parentIndex) const override
        {
        return false;
        }
    virtual ECObjectsStatus         _GetPropertyIndices (bvector<UInt32>& indices, UInt32 parentIndex) const override;

    public:
        enum IntrinsicEnablerType
            {
            DgnModel_Enabler        = 1,
            DgnAttachment_Enabler   = 1<<1,
            AllModel_Enabler        = DgnModel_Enabler | DgnAttachment_Enabler
            };

        virtual void    GetInstances (DgnECInstanceVector& results, DgnModelP model) = 0;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ModelIntrinsicECInstanceEnabler: public ModelBaseIntrinsicECInstanceEnabler
    {
    private:
        
        virtual WCharCP                 _GetName() const override {return L"ModelIntrinsicECInstanceEnabler";}
        virtual ECObjectsStatus         _GetPropertyIndex (UInt32& propertyIndex, WCharCP propertyAccessString) const override;
            

        virtual ECObjectsStatus         _GetAccessString  (WCharCP& propertyAccessString, UInt32 propertyIndex) const override;
        
        virtual UInt32                  _GetPropertyCount () const override
            {
            return DgnModelECInstance::MAX_PROPERTY_COUNT;
            }

        ModelIntrinsicECInstanceEnabler (ECClassCR ecClass, IStandaloneEnablerLocaterP structStandaloneEnablerLocater)
            : ModelBaseIntrinsicECInstanceEnabler (ecClass, structStandaloneEnablerLocater)
            {}
    public:
    
        virtual void    GetInstances (DgnECInstanceVector& results, DgnModelP model) override;
        
    static ModelIntrinsicECInstanceEnabler* CreateEnabler ();
    };


/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Abeesh.Basheer                  11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnModelProvider;
struct ModelECInstanceFinder: public BaseHostInstanceFinder<DgnModelP>
    {
    private:
        friend struct DgnModelProvider;
        bvector<ModelBaseIntrinsicECInstanceEnabler*> m_enablers;
        virtual DgnECHostType           _GetHostType () const override {return DgnECHostType_Model;}
        virtual DgnECInstanceIterable   _GetRelatedInstances (DgnECInstanceCR instance, 
                    QueryRelatedClassSpecifier const& relationshipClassSpecifier) const override;

    protected:
        ModelECInstanceFinder (IECPropertyValueFilterPtr valueFilter, WhereCriterionPtr whereCriteria)
            :BaseHostInstanceFinder<DgnModelP> (valueFilter,  whereCriteria)
            {}

    public:

        virtual StatusInt    _FindInstances (DgnECInstanceVector& results, DgnModelP model) const override;
        static IDgnECInstanceFinderPtr Create (IECPropertyValueFilterPtr valueFilter, WhereCriterionPtr whereCriteria, 
                                    bvector<ModelBaseIntrinsicECInstanceEnabler*>const& enablers);
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE