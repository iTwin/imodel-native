/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/uicommand.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <EcPresentation/uiitem.h>
#include <EcPresentation/journal.h>

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct IUICommand
    {
    protected:

        virtual BentleyStatus   _ExecuteCmd(IECInstanceCP instance) const = 0;

        virtual void            _Journal (IJournalItemR journalEntry) const = 0;

    };

typedef RefCountedPtr<UICommand> UICommandPtr;

/*---------------------------------------------------------------------------------**//**
//! A UIcommand represents 
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct UICommand : public IUICommand, public RefCountedBase
    {
    private:
        
    public:
        //! Execute the action associated with this command. If a journal provider is registered 
        //! the call results in the creation of the corresponding journal entry.
        ECOBJECTS_EXPORT BentleyStatus  Execute (IECInstanceCP instance) const;
    };

END_BENTLEY_EC_NAMESPACE