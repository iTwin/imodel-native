/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/auicommand.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/
#include "ecimagekey.h"
#include "ecpresentationtypedefs.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct IUICommand : public RefCountedBase
    {
    public:
    /*=================================================================================**//**
    * the possible "marks" that can be placed to the left of menu entries for Edit Actions.
    * @bsiinterface
    +===============+===============+===============+===============+===============+======*/
    enum EditActionMenuMark
        {
        MENUMARK_None       = 0,
        MENUMARK_ToggleOut  = 1,
        MENUMARK_ToggleIn   = 2,
        MENUMARK_RadioOut   = 3,
        MENUMARK_RadioIn    = 4,
        MENUMARK_RightArrow = 5,
        };

    /*=================================================================================**//**
    * recommended priority values for Edit Actions. Higher values appear higher in the popup menu.
    * @bsiinterface
    +===============+===============+===============+===============+===============+======*/
    enum EditActionPriority
        {
        EDITACTION_PRIORITY_Highest            = 10000,
        EDITACTION_PRIORITY_TopGroup1          = 9500,
        EDITACTION_PRIORITY_TopGroup2          = 9400,
        EDITACTION_PRIORITY_TopGroup3          = 9300,
        EDITACTION_PRIORITY_TopGroup4          = 9200,
        EDITACTION_PRIORITY_TopGroup5          = 9000,

        EDITACTION_PRIORITY_UserCommon         = 7000,
        EDITACTION_PRIORITY_CommonManipulation = 5000,
        EDITACTION_PRIORITY_ElementSpecific    = 4000,
        EDITACTION_PRIORITY_UserSelection      = 3500,
        EDITACTION_PRIORITY_Selection          = 3000,

        EDITACTION_PRIORITY_UserClipboard      = 2500,
        EDITACTION_PRIORITY_Clipboard          = 2000,
        EDITACTION_PRIORITY_UserDelete         = 1500,
        EDITACTION_PRIORITY_Delete             = 1000,

        EDITACTION_PRIORITY_UserProperties     = 500,
        EDITACTION_PRIORITY_Properties         = 0,

        EDITACTION_PRIORITY_BottomGroup1       = -20,
        EDITACTION_PRIORITY_BottomGroup2       = -40,
        EDITACTION_PRIORITY_BottomGroup3       = -60,
        EDITACTION_PRIORITY_BottomGroup4       = -80,
        EDITACTION_PRIORITY_BottomGroup5       = -100,
        EDITACTION_PRIORITY_Lowest             = -10000,

        #if defined (BEIJING_DGNPLATFORM_WIP_SWW)
        #endif // defined (BEIJING_DGNPLATFORM_WIP_SWW)
        EDITACTION_PRIORITY_MiscGroup1,
        EDITACTION_PRIORITY_MiscGroup2,
        EDITACTION_PRIORITY_MiscGroup3,
        EDITACTION_PRIORITY_MiscGroup4,
        EDITACTION_PRIORITY_MiscGroup5,
        EDITACTION_PRIORITY_MiscGroup6
        };

    protected:
        
        //!Execute the action that this command supports.
        virtual BentleyStatus       _ExecuteCmd(IAUIDataContextCP instance) = 0;

        //!Describe how you want to journal your command by calling methods on journal item
        //! entry.
        virtual void                _Journal (IJournalItemR journalEntry) = 0;

        virtual WCharCP             _GetCommandId () const = 0;
        
        virtual WString             _GetLabel () const = 0;
        virtual void                _SetLabel (WStringCR label) {}

        virtual WCharCP             _GetDescription () const { return NULL; }
        virtual void                _SetDescription (WStringCR description) {}
        
        virtual ECImageKeyCP        _GetImageId () const = 0;
        virtual void                _SetImageId (ECImageKeyCR key) { }
        
        virtual bool                _IsSeparator () const { return false; }
        
        virtual EditActionMenuMark  _GetMenuMark() const {return MENUMARK_None;}
        virtual void                _SetMenuMark(EditActionMenuMark mark) {}

        virtual bool                _GetIsEnabled () const {return false;}
        virtual void                _SetIsEnabled (bool enabled) {;}

        virtual UICommand*          _GetParent() const {return NULL;}
        
        virtual EditActionPriority  _GetPriority () const {return EDITACTION_PRIORITY_UserCommon;}
        virtual void                _SetPriority (EditActionPriority priority) {}

    public:
        //!virtual destructor.
        virtual ~IUICommand ()
            {}

        //! Execute the action associated with this command. If a journal provider is registered 
        //! the call results in the creation of the corresponding journal entry.
        ECOBJECTS_EXPORT BentleyStatus  ExecuteCmd (IAUIDataContextCP instance);

                //!Describe how you want to journal your command by calling methods on journal item
        //! entry.
        ECOBJECTS_EXPORT void               Journal (IJournalItemR journalEntry);

        ECOBJECTS_EXPORT WCharCP            GetCommandId () const;
        
        ECOBJECTS_EXPORT WString            GetLabel ()  const;
        ECOBJECTS_EXPORT void               SetLabel (WStringCR label);

        ECOBJECTS_EXPORT WCharCP            GetDescription ()  const;
        ECOBJECTS_EXPORT void               SetDescription (WStringCR description);
        
        ECOBJECTS_EXPORT ECImageKeyCP       GetImageId ()  const;
        ECOBJECTS_EXPORT void               SetImageId (ECImageKeyCR key);
        
        ECOBJECTS_EXPORT bool               IsSeparator ()  const;
        
        ECOBJECTS_EXPORT EditActionMenuMark GetMenuMark() const;
        ECOBJECTS_EXPORT void               SetMenuMark(EditActionMenuMark mark);

        ECOBJECTS_EXPORT bool               GetIsEnabled () const;
        ECOBJECTS_EXPORT void               SetIsEnabled (bool enabled);

        ECOBJECTS_EXPORT UICommand*         GetParent() const;
        
        /*---------------------------------------------------------------------------------**//**
        * Get the priority value for this Edit Action. Higher values are placed higher in the popup menu.
        * @return       the priority for this Edit Action.
        +---------------+---------------+---------------+---------------+---------------+------*/
        ECOBJECTS_EXPORT EditActionPriority GetPriority () const;
        ECOBJECTS_EXPORT void               SetPriority (EditActionPriority priority);
    };

typedef RefCountedPtr<UICommand> UICommandPtr;

/*---------------------------------------------------------------------------------**//**
//! A UIcommand represents an action that can be applied on a data context. (Usually an ecinstnace)
* @bsiclass                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct UICommand : public IUICommand
    {
    private:
        WString             m_commandId;
        WString             m_label;
        WString             m_description;
        ECImageKey          m_imageKey;
        bool                m_separator;
        EditActionMenuMark  m_menuMark;
        bool                m_enabled;
        EditActionPriority  m_priority;
    
    protected:

        virtual UICommand*  _GetParent() const override {return NULL;}

        UICommand (WStringCR id, WStringCR label, WStringCR description, ECImageKeyCR imgKey)
            :m_commandId(id), m_label(label),m_description(description), m_imageKey(imgKey),
            m_separator(false), m_enabled(true), m_priority(EDITACTION_PRIORITY_Lowest)
            {}

        UICommand (WStringCR id)
            :m_separator (true), m_enabled(true), m_priority(EDITACTION_PRIORITY_Lowest),
            m_imageKey(L"", ECImageKey::Icon)
            {}

        //! Return properties of the UICommand
        virtual WCharCP             _GetCommandId () const override { return m_commandId.c_str(); }
        
        virtual WString             _GetLabel ()  const override { return m_label; }
        virtual void                _SetLabel (WStringCR label) override {m_label = label; }

        virtual WCharCP             _GetDescription ()  const override { return m_description.c_str(); }
        virtual void                _SetDescription (WStringCR description) override {m_description = description;}
        
        virtual ECImageKeyCP        _GetImageId ()  const override { return &m_imageKey; }
        virtual void                _SetImageId (ECImageKeyCR key) override {m_imageKey = key; }
        
        virtual bool                _IsSeparator ()  const override { return m_separator; }
        
        virtual EditActionMenuMark  _GetMenuMark() const override {return m_menuMark;}
        virtual void                _SetMenuMark(EditActionMenuMark mark) override {m_menuMark = mark;}

        virtual bool                _GetIsEnabled () const override {return m_enabled;}
        virtual void                _SetIsEnabled (bool enabled) override {m_enabled = enabled;}

        virtual EditActionPriority  _GetPriority () const override {return m_priority;}
        virtual void                _SetPriority (EditActionPriority priority) override {m_priority = priority;}

    public:
    

    };

/*---------------------------------------------------------------------------------**//**
//! A provider that registers with the presentation manager to provide actions for specific 
//! data contexts. The presentation manager providers a union.
* @bsiclass                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECPresentationCommandProvider
    {
    protected:
        virtual bvector<UICommandPtr> _GetCommand (IAUIDataContextCR instance, int purpose) const = 0;
    
    public:

        virtual ~ECPresentationCommandProvider() {}

        ECOBJECTS_EXPORT bvector<UICommandPtr> GetCommand (IAUIDataContextCR instance, int purpose) const;
    };


END_BENTLEY_ECOBJECT_NAMESPACE