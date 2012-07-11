/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/ecviewdefinition.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Abeesh.Basheer                  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct IECPresentationViewTransform
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct IECPresentationUIItemInfo: public IAUIItemInfo
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct IECPresentationUIItem: public IAUIItem
    {
    protected:
        virtual IAUIItemInfoCR              _GetUIItemInfo () const {return _GetViewItemInfo();}
        virtual IECPresentationUIItemInfoCR _GetViewItemInfo () const = 0;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct  ECPresentationMenuItemInfo : public IECPresentationUIItemInfo
    {
    private:
        WString m_label;
        WString m_tooltip;
    protected:
        virtual ItemType    _GetItemType() const override {return Menu;}
        virtual bool        _IsAggregatable () const override {return true;}
        virtual bool        _IsSeperator () const {return false;}
    public:
        typedef ECPresentationMenuItem  T_ItemType;
        
        //! Get the label
        ECOBJECTS_EXPORT    WCharCP GetLabel() const;
        ECOBJECTS_EXPORT    void    SetLabel(WCharCP label);
        
        //! Get the tooltip
        ECOBJECTS_EXPORT    WCharCP GetToolTip() const;
        ECOBJECTS_EXPORT    void    SetToolTip();
        
        ECOBJECTS_EXPORT    bool    IsSeperator() const;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct  ECPresentationMenuItem : public IECPresentationUIItem
    {
    private:
        ECPresentationMenuItemInfo m_itemInfo;

    protected:
        
        virtual IECPresentationUIItemInfoCR _GetViewItemInfo () const {return GetItemInfo();}

    public:
        ECOBJECTS_EXPORT ECPresentationMenuItemInfoCR GetItemInfo () const;


        static ECPresentationMenuItem*  CreateSeperator();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct  IECPresentationViewDefinition : public RefCountedBase
    {
    typedef bvector <IECPresentationViewDefinitionPtr>  ChildDefinitions;
    
    protected:
        virtual IECPresentationUIItemCR         _GetUIItem () = 0;
        virtual ChildDefinitions                _GetChildDefinitions () = 0;
        virtual IECPresentationViewTransformCR  _GetViewTransform () = 0;

    public:
        //! The UI information associated with this view. It describes the control that needs to 
        //! to be instantiated to represent the data.
        ECOBJECTS_EXPORT IECPresentationUIItemCR GetUIItem();

        //! Get the child view definitions if there any. Its used by composite controls.
        ECOBJECTS_EXPORT ChildDefinitions GetChildDefinitions();
        
        ECOBJECTS_EXPORT IECPresentationViewTransformCR GetViewTransform ();

        static IECPresentationViewDefinitionPtr CreateCompositeViewDef (bvector<IECPresentationViewDefinitionPtr> const& viewDefs);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct  IECPresentationViewProvider : public IECPresentationProvider
    {
    protected:
        virtual IECPresentationViewDefinitionPtr    _GetViewDefinition (IAUIItemInfoCR itemInfo, IAUIDataContextCR dataContext) = 0;
        virtual ProviderType                        _GetProviderType() const override {return ViewService;}

    public:
        //!Get the view definition associated with a particular data context.
        //!@param[in] itemInfo      A hint to provide the context in which the view definition will be used. eg. MenuItem
        //!@param[in] dataContext   The data context for which the view definition is requested.
        ECOBJECTS_EXPORT    IECPresentationViewDefinitionPtr GetViewDefinition (IAUIItemInfoCR itemInfo, IAUIDataContextCR dataContext);

    };


END_BENTLEY_EC_NAMESPACE