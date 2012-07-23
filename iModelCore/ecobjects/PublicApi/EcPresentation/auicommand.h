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
    protected:
        WString m_commandId;
        WString m_label;
        WString m_description;
        WString m_imageId;
        bool    m_separator;
        
    public:
        //! Execute the action associated with this command. If a journal provider is registered 
        //! the call results in the creation of the corresponding journal entry.
        ECOBJECTS_EXPORT BentleyStatus  Execute (IAUIDataContextCP instance) const;

        //! Return properties of the UICommand
        virtual WString const GetCommandId () { return m_commandId; };
        virtual WString const GetLabel () { return m_label; };
        virtual WString const GetDescription () { return m_description; };
        virtual WString const GetImageId () { return m_imageId; };
        virtual bool    const IsSeparator () { return m_separator; };
    };

/*---------------------------------------------------------------------------------**//**
//! A provider that registers with the presentation manager to provide actions for specific 
//! data contexts. The presentation manager providers a union.
* @bsiclass                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECPresentationCommandProvider
    {
    protected:
        virtual bvector<UICommandPtr> _GetCommand (IAUIDataContextCR instance) const = 0;
    
    public:

        virtual ~ECPresentationCommandProvider() {}

        ECOBJECTS_EXPORT bvector<UICommandPtr> GetCommand (IAUIDataContextCR instance) const;
    };


END_BENTLEY_EC_NAMESPACE