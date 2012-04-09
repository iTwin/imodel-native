/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/uipresentationmgr.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <EcPresentation/uicommand.h>

EC_TYPEDEFS (UIPresentationManager)
EC_TYPEDEFS (IUICommandProvider)
EC_TYPEDEFS (IUIDisplayProvider)
EC_TYPEDEFS (IJournalProvider)
EC_TYPEDEFS (UIECEnabler);
BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct IUICommandProvider
    {
    virtual UICommandPtr GetCommand (IECInstanceCR instance) const = 0;
    virtual ~IUICommandProvider() {}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct IUIDisplayProvider
    {
    virtual IUIItemPtr   GetUIItem (UIECClassCR itemType, IECInstanceP instanceData) const = 0;
    virtual ~IUIDisplayProvider() {}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct  IJournalProvider
    {
    virtual void    JournalCmd (IUICommandCR cmd, IECInstanceCP instanceData) = 0;
    virtual ~IJournalProvider() {}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct  UIPresentationManager: public NonCopyableClass
    {
    private:
        typedef bset<IUICommandProviderCP> T_CmdProviderSet;
        typedef bset<IUIDisplayProviderCP> T_DisplayProviderSet;
        typedef bset<IJournalProviderP>    T_JournalProviderSet;

        T_CmdProviderSet        m_cmdProviders;
        T_DisplayProviderSet    m_displayProviders;
        T_JournalProviderSet    m_journalProviders;
        ECSchemaPtr             m_schema;

        UIPresentationManager ();
        bool                    InitSchema();
    public:
                     void                           JournalCmd (IUICommandCR cmd, IECInstanceCP instanceData);

    ECOBJECTS_EXPORT void                           AddProvider (IJournalProviderR provider);
    ECOBJECTS_EXPORT void                           RemoveProvider (IJournalProviderR provider);

    ECOBJECTS_EXPORT void                           AddProvider (IUICommandProviderCR provider);
    ECOBJECTS_EXPORT void                           RemoveProvider (IUICommandProviderCR provider);
    
    ECOBJECTS_EXPORT void                           AddProvider (IUIDisplayProviderCR provider);
    ECOBJECTS_EXPORT void                           RemoveProvider (IUIDisplayProviderCR provider);

    ECOBJECTS_EXPORT IUIItemPtr                     GetUIItem (UIECClassCR itemType, IECInstanceP instanceData) const;

    ECOBJECTS_EXPORT UICommandPtr                   GetCommand (IECInstanceCR instance) const;

    ECOBJECTS_EXPORT static UIPresentationManagerR  GetManager();
    };

END_BENTLEY_EC_NAMESPACE
