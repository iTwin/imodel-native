/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/auicommand.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct IUICommand
    {
    protected:
        
        //!Execute the action that this command supports.
        virtual BentleyStatus   _ExecuteCmd(IAUIDataContextCP instance) const = 0;

        //!Describe how you want to journal your command by calling methods on journal item
        //! entry.
        virtual void            _Journal (IJournalItemR journalEntry) const = 0;

    public:
        //!virtual destructor.
        virtual ~IUICommand ()
            {}

    };

typedef RefCountedPtr<UICommand> UICommandPtr;

/*---------------------------------------------------------------------------------**//**
//! A UIcommand represents an action that can be applied on a data context. (Usually an ecinstnace)
* @bsiclass                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct UICommand : public IUICommand, public RefCountedBase
    {
    private:
        
    public:
        //! Execute the action associated with this command. If a journal provider is registered 
        //! the call results in the creation of the corresponding journal entry.
        ECOBJECTS_EXPORT BentleyStatus  Execute (IAUIDataContextCP instance) const;
    };

/*---------------------------------------------------------------------------------**//**
//! A provider that registers with the presentation manager to provide actions for specific 
//! data contexts. The presentation manager providers a union.
* @bsiclass                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct IUICommandProvider
    {
    protected:
        virtual bvector<UICommandPtr> _GetCommand (IAUIDataContextCR instance) const = 0;
    
    public:

        virtual ~IUICommandProvider() {}

        ECOBJECTS_EXPORT bvector<UICommandPtr> GetCommand (IAUIDataContextCR instance) const;
    };


END_BENTLEY_EC_NAMESPACE