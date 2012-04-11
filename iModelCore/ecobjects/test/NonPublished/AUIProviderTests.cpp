/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/AUIProviderTests.cpp $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"

#include "StopWatch.h"
#include "TestFixture.h"

#include <EcPresentation/uipresentationmgr.h>
#include <EcPresentation/uienabler.h>
#include <EcPresentation/uiitem.h>
BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct  TestLoggerCommand: public UICommand
    {
    virtual BentleyStatus   _ExecuteCmd(IECInstanceCP instance) const
        {
        return SUCCESS;
        }

    virtual void            _Journal (IJournalItemR journalEntry) const
        {
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct  TestMenuItem: public IAUICommandItem
    {
    private:
        UIECEnablerCR       m_enabler;
        static TestLoggerCommand   m_cmd;
    public:
    // Virtual and returning WString because a subclass may want to calculate it on demand
    virtual WString             _GetInstanceId() const
        {
        return L"IAUICommandItem";
        }
    
    virtual ECObjectsStatus     _GetValue (ECValueR v, WCharCP managedPropertyAccessor, bool useArrayIndex, UInt32 arrayIndex) const
        {
        return ECOBJECTS_STATUS_OperationNotSupported;
        }

    virtual ECObjectsStatus     _GetValue (ECValueR v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const
        {
        return ECOBJECTS_STATUS_OperationNotSupported;
        }

    virtual ECObjectsStatus     _SetValue (WCharCP managedPropertyAccessor, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex)
        {
        return ECOBJECTS_STATUS_OperationNotSupported;
        }

    virtual ECObjectsStatus     _SetValue (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex)
        {
        return ECOBJECTS_STATUS_OperationNotSupported;
        }

    virtual ECObjectsStatus     _InsertArrayElements (WCharCP managedPropertyAccessor, UInt32 index, UInt32 size)
        {
        return ECOBJECTS_STATUS_OperationNotSupported;
        }

    virtual ECObjectsStatus     _AddArrayElements (WCharCP managedPropertyAccessor, UInt32 size)
        {
        return ECOBJECTS_STATUS_OperationNotSupported;
        }

    virtual ECObjectsStatus     _RemoveArrayElement (WCharCP managedPropertyAccessor, UInt32 index)
        {
        return ECOBJECTS_STATUS_OperationNotSupported;
        }

    virtual ECObjectsStatus     _ClearArray (WCharCP managedPropertyAccessor)
        {
        return ECOBJECTS_STATUS_OperationNotSupported;
        }

    virtual ECEnablerCR         _GetEnabler() const
        {
        return m_enabler;
        }

    virtual bool                _IsReadOnly() const
        {
        return false;
        }

    virtual WString             _ToString (WCharCP indent) const
        {
        return WString();
        }

    virtual size_t              _GetOffsetToIECInstance () const
        {
        return 0;
        }

    virtual UICommandCR         _GetCommand () const 
        {
        return m_cmd;
        }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
    TestMenuItem (UIECEnablerCR enabler)
        :m_enabler(enabler)
        {}

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct TestMenuEnabler: public PropertyIndexedEnabler <TestMenuEnabler, UIECEnabler>
    {
    public:
        static const UInt32 MAX_PROPERTY_COUNT = 4;
        static const WCharCP PropertyNameList[MAX_PROPERTY_COUNT] ;
    protected:
        TestMenuEnabler ()
            :PropertyIndexedEnabler <TestMenuEnabler, UIECEnabler>(UIECEnabler::GetUIClass(L"MenuCommandItem"), NULL)
            {
            }
    protected:
        virtual WCharCP                 _GetName() const override
            {
            return L"TestMenuEnabler";
            }
        virtual IAUIItemPtr     _GetUIItem(Bentley::EC::IECInstanceP) override;
    public:
    static UIECEnablerPtr Create ()
        {
        return new TestMenuEnabler();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct TestUIProvider : public IAUIProvider
    {
    virtual UInt16 _GetProviderId(void) const override
        {
        return 0xA1;
        }

    //!Get the provider name
    virtual WCharCP     _GetProviderName () const override
        {
        return L"TestUIProvider";
        }

    virtual UIECEnablerPtr  _GetUIEnabler (ECClassCR classInstance) override;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct AUITestFixture: public ECTestFixture
    {
    private:
        TestUIProvider          m_provider;
    
    protected:
        UIPresentationManagerP  m_mgr;

    public:
        virtual void SetUp() override;
        virtual void TearDown() override;
    };


END_BENTLEY_EC_NAMESPACE

USING_NAMESPACE_EC

TestLoggerCommand TestMenuItem::m_cmd;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            AUITestFixture::SetUp ()
    {
    m_mgr = &UIPresentationManager::GetManager();
    m_mgr->AddProvider (m_provider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            AUITestFixture::TearDown()
    {
    m_mgr->RemoveProvider (m_provider);
    m_mgr = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
const WCharCP   TestMenuEnabler::PropertyNameList [] = {
            L"Data",
            L"Icon",
            L"Label",
            L"Tooltip",
            };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
 IAUIItemPtr    TestMenuEnabler::_GetUIItem(Bentley::EC::IECInstanceP instanceData)
     {
     return new TestMenuItem(*this);
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
UIECEnablerPtr  TestUIProvider::_GetUIEnabler (ECClassCR classInstance)
    {
    WString name =  classInstance.GetName();
    if (0 != name.CompareToI(UIPresentationManager::MenuCommandItemClassName))
        return NULL;

    return TestMenuEnabler::Create();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AUITestFixture, GetMenuItem)
    {
    ECSchemaCR displaySchema = m_mgr->GetAUISchema();

    UIECClassP menuItemClass = static_cast<UIECClassP> (displaySchema.GetClassP(UIPresentationManager::MenuCommandItemClassName));
    EXPECT_TRUE (NULL != menuItemClass);

    IAUIItemPtr item = m_mgr->GetUIItem(*menuItemClass, NULL);
    EXPECT_TRUE (item.IsValid());


    }