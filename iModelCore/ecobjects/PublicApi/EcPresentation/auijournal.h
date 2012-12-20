/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/auijournal.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/


BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//IJournal Item can be modeled as an IECInstance where you configure rules to specify how a journal item can be obtained.
//However in general use case creating a rule processor for a specific scripting language will be very involved and complex.
//So for ease of debug and practicality, its implemented as a set of callbacks.
struct IJournalItem
    {

    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct  IJournalProvider :  public IECPresentationProvider
    {
    protected:
        virtual void    _JournalCmd (IUICommandCR cmd, IAUIDataContextCP instanceData) = 0;
    
    public:
        
        ECOBJECTS_EXPORT void    JournalCmd (IUICommandCR cmd, IAUIDataContextCP instanceData);
        
        virtual ~IJournalProvider() {}


    };

END_BENTLEY_ECOBJECT_NAMESPACE