/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/auicommand.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
//__PUBLISH_SECTION_START__

#pragma once

#include "ecimagekey.h"
#include "ecpresentationtypedefs.h"
#include "auiprovider.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

typedef RefCountedPtr<IUICommand> IUICommandPtr;

/*=================================================================================**//**
//! NEEDSWORK: Add comment here
* @bsiclass                                    Abeesh.Basheer                  04/2012
+===============+===============+===============+===============+===============+======*/
struct IUICommand : public RefCountedBase
    {
    public:
    /*=================================================================================**//**
    //! The possible "marks" that can be placed to the left of menu entries for Edit Actions.
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
    //! Recommended priority values for Edit Actions. Higher values appear higher in the popup menu.
    * @bsiinterface
    +===============+===============+===============+===============+===============+======*/
    enum EditActionPriority
        {
        EDITACTION_PRIORITY_Highest            = 1000,
        EDITACTION_PRIORITY_TopGroup1          = 950,
        EDITACTION_PRIORITY_TopGroup2          = 940,
        EDITACTION_PRIORITY_TopGroup3          = 930,
        EDITACTION_PRIORITY_TopGroup4          = 920,
        EDITACTION_PRIORITY_TopGroup5          = 900,

        EDITACTION_PRIORITY_UserCommon         = 700,
        EDITACTION_PRIORITY_CommonManipulation = 500,
        EDITACTION_PRIORITY_ElementSpecific    = 400,
        EDITACTION_PRIORITY_UserSelection      = 350,
        EDITACTION_PRIORITY_Selection          = 300,

        EDITACTION_PRIORITY_UserClipboard      = 250,
        EDITACTION_PRIORITY_Clipboard          = 200,
        EDITACTION_PRIORITY_UserDelete         = 150,
        EDITACTION_PRIORITY_Delete             = 100,

        EDITACTION_PRIORITY_UserProperties     = 50,
        EDITACTION_PRIORITY_Properties         = 0,

        EDITACTION_PRIORITY_BottomGroup1       = -2,
        EDITACTION_PRIORITY_BottomGroup2       = -4,
        EDITACTION_PRIORITY_BottomGroup3       = -6,
        EDITACTION_PRIORITY_BottomGroup4       = -8,
        EDITACTION_PRIORITY_BottomGroup5       = -10,
        EDITACTION_PRIORITY_Lowest             = -1000,

        #if defined (BEIJING_DGNPLATFORM_WIP_SWW)
        #endif // defined (BEIJING_DGNPLATFORM_WIP_SWW)
        EDITACTION_PRIORITY_MiscGroup1,
        EDITACTION_PRIORITY_MiscGroup2,
        EDITACTION_PRIORITY_MiscGroup3,
        EDITACTION_PRIORITY_MiscGroup4,
        EDITACTION_PRIORITY_MiscGroup5,
        EDITACTION_PRIORITY_MiscGroup6
        };

    /*=================================================================================**//**
    //! NEEDSWORK: Add comment here
    * @bsiinterface
    +===============+===============+===============+===============+===============+======*/
    enum ApplicationPurpose
        {
        Unknown              = 0,
        ViewWindowRightClick = 1,
        ToolBar              = 2,
        ExplorerMenu         = 3,
        };

//__PUBLISH_SECTION_END__

    protected:
        //! Execute the action that this command supports.
        virtual BentleyStatus       _ExecuteCmd(IAUIDataContextCP instance) = 0;

        //! Describe how you want to journal your command by calling methods on journal item entry.
        virtual void                _Journal (IJournalItemR journalEntry) = 0;

        virtual WCharCP             _GetCommandId () const = 0;
        
        virtual WString             _GetLabel () const = 0;
        virtual void                _SetLabel (WCharCP label) {}

        virtual WString             _GetDescription () const { return WString(); }
        virtual void                _SetDescription (WCharCP description) {}
        
        virtual WString             _GetApplicatonArg () const { return WString(); }
        virtual void                _SetApplicationArg (WCharCP arg) {}

        virtual ECImageKeyCP        _GetImageId () const = 0;
        virtual void                _SetImageId (ECImageKeyCR key) { }
        
        virtual bool                _IsSeparator () const {return false;}
        
        virtual EditActionMenuMark  _GetMenuMark() const {return MENUMARK_None;}
        virtual void                _SetMenuMark(EditActionMenuMark mark) {}

        virtual bool                _GetIsEnabled () const {return false;}
        virtual void                _SetIsEnabled (bool enabled) {;}

        virtual IUICommand*         _GetParent() const {return NULL;}
        
        virtual void                _GetChildren (bvector<IUICommandPtr>& children) const {}
        virtual EditActionPriority  _GetPriority () const {return EDITACTION_PRIORITY_UserCommon;}
        virtual void                _SetPriority (EditActionPriority priority) {}

        virtual void                _GetChildren(bvector<IUICommandPtr>& children) {}

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    public:
        //! Virtual destructor.
        virtual ~IUICommand () {}

        //! Execute the action associated with this command. If a journal provider is registered 
        //! the call results in the creation of the corresponding journal entry.
        //! @param[in] instance      NEEDSWORK: Add comment here
        ECOBJECTS_EXPORT BentleyStatus      ExecuteCmd (IAUIDataContextCP instance);

        //! Describe how you want to journal your command by calling methods on journal item entry.
        ECOBJECTS_EXPORT void               Journal (IJournalItemR journalEntry);

        //! NEEDSWORK: Add comment here
        ECOBJECTS_EXPORT WCharCP            GetCommandId () const;
        
        //! NEEDSWORK: Add comment here
        ECOBJECTS_EXPORT WString            GetLabel ()  const;
        //! NEEDSWORK: Add comment here
        ECOBJECTS_EXPORT void               SetLabel (WCharCP label);

        //! NEEDSWORK: Add comment here
        ECOBJECTS_EXPORT WString            GetDescription ()  const;
        //! NEEDSWORK: Add comment here
        ECOBJECTS_EXPORT void               SetDescription (WCharCP description);
        
        //! NEEDSWORK: Add comment here
        ECOBJECTS_EXPORT ECImageKeyCP       GetImageId ()  const;
        //! NEEDSWORK: Add comment here
        ECOBJECTS_EXPORT void               SetImageId (ECImageKeyCR key);
        
        //! NEEDSWORK: Add comment here
        ECOBJECTS_EXPORT bool               IsSeparator ()  const;
        
        //! NEEDSWORK: Add comment here
        ECOBJECTS_EXPORT WString            GetApplicatonArg ()  const;
        //! NEEDSWORK: Add comment here
        ECOBJECTS_EXPORT void               SetApplicationArg (WCharCP arg);

        //! NEEDSWORK: Add comment here
        ECOBJECTS_EXPORT EditActionMenuMark GetMenuMark() const;
        //! NEEDSWORK: Add comment here
        ECOBJECTS_EXPORT void               SetMenuMark(EditActionMenuMark mark);

        //! NEEDSWORK: Add comment here
        ECOBJECTS_EXPORT bool               GetIsEnabled () const;
        //! NEEDSWORK: Add comment here
        ECOBJECTS_EXPORT void               SetIsEnabled (bool enabled);

        //! NEEDSWORK: Add comment here
        ECOBJECTS_EXPORT IUICommand*        GetParent() const;
        //! NEEDSWORK: Add comment here
        ECOBJECTS_EXPORT void               GetChildren (bvector<IUICommandPtr>& children) const;
        //! Get the priority value for this Edit Action. Higher values are placed higher in the popup menu.
        //! @returns the priority for this Edit Action.
        ECOBJECTS_EXPORT EditActionPriority GetPriority () const;
        //! NEEDSWORK: Add comment here
        ECOBJECTS_EXPORT void               SetPriority (EditActionPriority priority);

        //! NEEDSWORK: Add comment here
        ECOBJECTS_EXPORT void               GetChildren (bvector<IUICommandPtr>& children);
    };

typedef RefCountedPtr<UICommand> UICommandPtr;
/*=================================================================================**//**
//! NEEDSWORK: Add comment here
* @bsimethod                                    Abeesh.Basheer                  12/2012
+===============+===============+===============+===============+===============+======*/
struct ActionPriorityComparer
    {
    bool operator () (IUICommandPtr const& lhs, IUICommandPtr const& rhs)
        {
        return lhs->GetPriority() > rhs->GetPriority();
        }
    };

/*=================================================================================**//**
//! A UICommand represents an action that can be applied on a data context. (Usually an ECInstance)
* @bsiclass                                     Abeesh.Basheer                  04/2012
+===============+===============+===============+===============+===============+======*/
struct UICommand : public IUICommand
    {
//__PUBLISH_SECTION_END__
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
        virtual void                _SetLabel (WCharCP label) override {m_label = label; }

        virtual WString             _GetDescription ()  const override { return m_description; }
        virtual void                _SetDescription (WCharCP description) override {m_description = description;}
        
        virtual ECImageKeyCP        _GetImageId ()  const override { return &m_imageKey; }
        virtual void                _SetImageId (ECImageKeyCR key) override {m_imageKey = key; }
        
        virtual bool                _IsSeparator ()  const override { return m_separator; }
        
        virtual EditActionMenuMark  _GetMenuMark() const override {return m_menuMark;}
        virtual void                _SetMenuMark(EditActionMenuMark mark) override {m_menuMark = mark;}

        virtual bool                _GetIsEnabled () const override {return m_enabled;}
        virtual void                _SetIsEnabled (bool enabled) override {m_enabled = enabled;}

        virtual EditActionPriority  _GetPriority () const override {return m_priority;}
        virtual void                _SetPriority (EditActionPriority priority) override {m_priority = priority;}

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    };

/*=================================================================================**//**
//! A provider that registers with the presentation manager to provide actions for specific 
//! data contexts. The presentation manager providers a union.
* @bsiclass                                     Abeesh.Basheer                  04/2012
+===============+===============+===============+===============+===============+======*/
struct ECPresentationCommandProvider : public IECPresentationProvider
    {

//__PUBLISH_SECTION_END__
    protected:
        virtual void _GetCommand (bvector<IUICommandPtr>&commands, IAUIDataContextCR instance, int purpose) = 0;
        virtual ProviderType    _GetProviderType (void) const {return CommandService;}
//__PUBLISH_SECTION_START__
    public:
        //! Virtual destructor.
        virtual ~ECPresentationCommandProvider() {}

        //! NEEDSWORK: Add comment here
        //! @param[out] commands     NEEDSWORK: Add comment here
        //! @param[in]  instance     NEEDSWORK: Add comment here
        //! @param[in]  purpose      NEEDSWORK: Add comment here
        ECOBJECTS_EXPORT void GetCommand (bvector<IUICommandPtr>&commands, IAUIDataContextCR instance, int purpose);
    };


END_BENTLEY_ECOBJECT_NAMESPACE

//__PUBLISH_SECTION_END__
