/*----------------------------------------------------------------------+
|
|   $Source: PrivateApi/DgnPlatformInternal/DgnHandlers/RelativeOffsetAssociation.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct RelativeOffsetAssociation
    {
    private:
        static ConstElementLinkageIterator     GetConstRelativeAssociationVectorLinkage(ElementHandleCR element);
        static ElementLinkageIterator          GetRelativeAssociationVectorLinkage(EditElementHandleR element);
        static bool                            IsOfRelativeOffsetAssociationType(Handler& handler);
    public:
    static StatusInt        GetOffsetValue(ElementHandleCR element, DVec3dR offset);
    static BentleyStatus    RemoveOffsetAssociation (EditElementHandleR element);
    static BentleyStatus    AddOffsetAssociation (EditElementHandleR element, ElementHandleCR targetElement, AssocPoint const& assoc, DPoint3dCR origin);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct RelativeAssocPointRootsChangedExtension  : public IAssocPointRootsChangedExtension
    {
    protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Abeesh.Basheer                  04/2013
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual StatusInt   _GetOffsetReferenceLocation (DPoint3dR location, ElementHandleCR element) = 0;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Abeesh.Basheer                  04/2013
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual StatusInt _GetPoint (ElementHandleCR element, DPoint3dR point, int index) override;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Abeesh.Basheer                  04/2013
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual StatusInt _SetPoint (EditElementHandleR eeh, DPoint3dCR point, int index) override;
    };
