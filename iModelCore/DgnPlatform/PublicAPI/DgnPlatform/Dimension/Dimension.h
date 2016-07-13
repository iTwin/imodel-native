//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/Dimension/Dimension.h $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnElement.h>
#include <DgnPlatform/ElementHandler.h>
#include <DgnPlatform/ECSqlStatementIterator.h>
//#include <DgnPlatform/Annotations/AnnotationPropertyBag.h>

DGNPLATFORM_TYPEDEFS(DimensionStyle);
DGNPLATFORM_TYPEDEFS(LinearDimension);
DGNPLATFORM_TYPEDEFS(LinearDimension2d);
DGNPLATFORM_TYPEDEFS(LinearDimension3d);

DGNPLATFORM_REF_COUNTED_PTR(DimensionStyle);
DGNPLATFORM_REF_COUNTED_PTR(LinearDimension2d);
DGNPLATFORM_REF_COUNTED_PTR(LinearDimension3d);

#define DGN_CLASSNAME_DimensionStyle            "DimensionStyle"
#define DGN_CLASSNAME_LinearDimension2d         "LinearDimension2d"
#define DGN_CLASSNAME_LinearDimension3d         "LinearDimension3d"

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! @ingroup GROUP_Annotation
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DimensionStyle : DefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_DimensionStyle, DefinitionElement);

private:
    DgnElementId        m_textStyleId;

    DgnDbStatus                     BindParams(BeSQLite::EC::ECSqlStatement& stmt);

    static DgnCode CreateCodeFromName(Utf8CP name) { return ResourceAuthority::CreateResourceCode(name, DGN_CLASSNAME_AnnotationTextStyle); }

protected:
    DGNPLATFORM_EXPORT  virtual DgnDbStatus                 _ReadSelectParams(BeSQLite::EC::ECSqlStatement&, ECSqlClassParams const&) override;
    DGNPLATFORM_EXPORT  virtual DgnDbStatus                 _BindInsertParams(BeSQLite::EC::ECSqlStatement&) override;
    DGNPLATFORM_EXPORT  virtual DgnDbStatus                 _BindUpdateParams(BeSQLite::EC::ECSqlStatement&) override;
    DGNPLATFORM_EXPORT  virtual void                        _CopyFrom(DgnElementCR) override;

    virtual DgnDbStatus _OnDelete() const override { return DgnDbStatus::DeletionProhibited; /* Must be "purged" */ }
    virtual DgnCode _GenerateDefaultCode() const override { return DgnCode(); }
    virtual bool _SupportsCodeAuthority(DgnAuthorityCR auth) const override { return ResourceAuthority::IsResourceAuthority(auth); }
    DGNPLATFORM_EXPORT virtual void _RemapIds(DgnImportContext&) override;

public:
    static DgnClassId               QueryClassId(DgnDbR db) { return DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_DimensionStyle)); }
    
    explicit DimensionStyle(DgnDbR db) : T_Super(CreateParams(db, DgnModel::DictionaryId(), QueryClassId(db), DgnCode())) {}
    explicit DimensionStyle(CreateParams const& params) : T_Super(params) {}
    DimensionStylePtr CreateCopy() const { return MakeCopy<DimensionStyle>(); }

    Utf8String GetName() const { return GetCode().GetValue(); }
    void SetName(Utf8CP value) { T_Super::_SetCode(CreateCodeFromName(value)); /* Only SetName is allowed to SetCode. */ }

    static DgnElementId QueryId(DgnDbR db, Utf8CP name) { return db.Elements().QueryElementIdByCode(CreateCodeFromName(name)); }
    static DimensionStyleCPtr Get(DgnDbR db, Utf8CP name) { return Get(db, QueryId(db, name)); }
    static DimensionStyleCPtr Get(DgnDbR db, DgnElementId id) { return db.Elements().Get<DimensionStyle>(id); }
    static DimensionStylePtr GetForEdit(DgnDbR db, DgnElementId id) { return db.Elements().GetForEdit<DimensionStyle>(id); }
    static DimensionStylePtr GetForEdit(DgnDbR db, Utf8CP name) { return GetForEdit(db, QueryId(db, name)); }
    DimensionStyleCPtr Insert() { return GetDgnDb().Elements().Insert<DimensionStyle>(*this); }
    DimensionStyleCPtr Update() { return GetDgnDb().Elements().Update<DimensionStyle>(*this); }

    DGNPLATFORM_EXPORT          DgnElementId            GetTextStyleId () const;
    DGNPLATFORM_EXPORT          void                    SetTextStyleId (DgnElementId textStyleId);

    DGNPLATFORM_EXPORT  static  DimensionStylePtr       Create(DgnElementId textStyleId, DgnDbR db);
};

namespace dgn_ElementHandler
{
    //=======================================================================================
    //! The handler for dimension styles
    //=======================================================================================
    struct DimensionStyleHandler : Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_DimensionStyle, DimensionStyle, DimensionStyleHandler, Element, DGNPLATFORM_EXPORT);

    protected:
        DGNPLATFORM_EXPORT virtual void _TEMPORARY_GetPropertyHandlingCustomAttributes(ECSqlClassParams::PropertyHandlingCustomAttributes&) override;  // *** WIP_AUTO_HANDLED_PROPERTIES
    };
}

//=======================================================================================
//! @ingroup GROUP_Annotation
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LinearDimension
{
private:
    DgnElementId        m_dimStyleId;
    bvector<DPoint2d>   m_points;

protected:
    void                            CopyFrom(LinearDimensionCR);
    DgnDbStatus                     BindParams(BeSQLite::EC::ECSqlStatement&);
    DgnDbStatus                     ReadSelectParams(BeSQLite::EC::ECSqlStatement&, ECSqlClassParamsCR);
    void                            SetPlacementOrigin(DPoint3dCR world);

    virtual GeometricElement const& _DimToElement() const = 0;

public:
                    GeometricElement const&         ToElement() const { return _DimToElement(); }
                    GeometricElement &              ToElementR() { return const_cast <GeometricElement&> (ToElement());}

                    DPoint2d                        GetPointLocal (uint32_t index) const;
                    BentleyStatus                   AppendPointLocal (DPoint2dCR point);
                    BentleyStatus                   InsertPointLocal (DPoint2dCR point, uint32_t index);
                    BentleyStatus                   SetPointLocal (DPoint2dCR point, uint32_t index);

DGNPLATFORM_EXPORT  DgnElementId                    GetDimensionStyleId () const;
DGNPLATFORM_EXPORT  void                            SetDimensionStyleId (DgnElementId);

DGNPLATFORM_EXPORT  uint32_t                        GetPointCount () const;
DGNPLATFORM_EXPORT  DPoint3d                        GetPoint (uint32_t index) const;
DGNPLATFORM_EXPORT  BentleyStatus                   AppendPoint (DPoint3dCR point);
DGNPLATFORM_EXPORT  BentleyStatus                   InsertPoint (DPoint3dCR point, uint32_t index);
DGNPLATFORM_EXPORT  BentleyStatus                   SetPoint (DPoint3dCR point, uint32_t index);

DGNPLATFORM_EXPORT  void                            UpdateGeometryRepresentation();
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct          LinearDimensionStroker
{
private:
    LinearDimensionCR       m_dim;
    DimensionStyleCPtr      m_dimStyle;
    DgnElementId            m_textStyleId;
    GeometryBuilderR        m_geomBuilder;
    bvector<DPoint2d>       m_dimPoints;
    double                  m_textHeight;
    double                  m_runningStackOffset;
    DPoint2d                m_textMargin;

    Utf8String              FormatDistanceString (double distance);
    double                  CalculateMeasureDistance (DPoint2dCR start, DPoint2dCR end);

    void                    AppendDimensionLine (DPoint2dCR start, DPoint2dCR end);
    void                    AppendWitness (DPoint2dCR start, DPoint2dCR end);
    void                    AppendTerminator (bvector<DPoint2d> const& points, bool filled);

    void                    GenerateText (DPoint2dCR textPoint, DVec2dCR textDir, Utf8CP string);
    void                    GenerateTerminator (DPoint2dCR termPoint, DVec2dCR termDir);
    void                    GenerateDimension (DPoint2dCR start, DPoint2dCR end);
    void                    GenerateSegment (uint32_t iSegment);

public:
    LinearDimensionStroker(LinearDimensionCR, GeometryBuilderR);

    LinearDimensionCR    GetDimension() const { return m_dim; }

    void            AppendDimensionGeometry();
};

//=======================================================================================
//! @ingroup GROUP_Annotation
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LinearDimension2d : AnnotationElement2d, LinearDimension
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_LinearDimension2d, AnnotationElement2d);

protected:
    DGNPLATFORM_EXPORT  virtual DgnDbStatus                 _OnInsert() override;
    DGNPLATFORM_EXPORT  virtual DgnDbStatus                 _OnUpdate(DgnElementCR original) override;

    DGNPLATFORM_EXPORT  virtual DgnDbStatus                 _ReadSelectParams(BeSQLite::EC::ECSqlStatement&, ECSqlClassParams const&) override;
    DGNPLATFORM_EXPORT  virtual DgnDbStatus                 _BindInsertParams(BeSQLite::EC::ECSqlStatement&) override;
    DGNPLATFORM_EXPORT  virtual DgnDbStatus                 _BindUpdateParams(BeSQLite::EC::ECSqlStatement&) override;

    DGNPLATFORM_EXPORT  virtual void                        _CopyFrom(DgnElementCR) override;

public:
                    explicit                        LinearDimension2d(CreateParams const& params);
DGNPLATFORM_EXPORT  static LinearDimension2dPtr     Create(CreateParams const& params);

                    static DgnClassId               QueryClassId(DgnDbR db) { return DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_LinearDimension2d)); }

                    virtual GeometricElement const& _DimToElement() const override final { return *this; }

                    static LinearDimension2dCPtr    Get(DgnDbR db, DgnElementId id) { return db.Elements().Get<LinearDimension2d>(id); }
                    static LinearDimension2dPtr     GetForEdit(DgnDbR db, DgnElementId id) { return db.Elements().GetForEdit<LinearDimension2d>(id); }
                    LinearDimension2dCPtr           Insert() { return GetDgnDb().Elements().Insert<LinearDimension2d>(*this); }
                    LinearDimension2dCPtr           Update() { return GetDgnDb().Elements().Update<LinearDimension2d>(*this); }
                    DgnDbStatus                     Delete() const { return GetDgnDb().Elements().Delete(*this); }

DGNPLATFORM_EXPORT  static LinearDimension2dPtr     Create(CreateParams const& params, DgnElementId dimStyleId, DPoint3dCR endPoint);
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LinearDimension3d : GraphicalElement3d, LinearDimension
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_LinearDimension3d, GraphicalElement3d);

protected:
    DGNPLATFORM_EXPORT  virtual DgnDbStatus                 _OnInsert() override;
    DGNPLATFORM_EXPORT  virtual DgnDbStatus                 _OnUpdate(DgnElementCR original) override;

    DGNPLATFORM_EXPORT  virtual DgnDbStatus                 _ReadSelectParams(BeSQLite::EC::ECSqlStatement&, ECSqlClassParams const&) override;
    DGNPLATFORM_EXPORT  virtual DgnDbStatus                 _BindInsertParams(BeSQLite::EC::ECSqlStatement&) override;
    DGNPLATFORM_EXPORT  virtual DgnDbStatus                 _BindUpdateParams(BeSQLite::EC::ECSqlStatement&) override;

    DGNPLATFORM_EXPORT  virtual void                        _CopyFrom(DgnElementCR) override;

public:
                    explicit                        LinearDimension3d(CreateParams const& params);
DGNPLATFORM_EXPORT  static LinearDimension3dPtr     Create(CreateParams const& params);

                    static DgnClassId               QueryClassId(DgnDbR db) { return DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_LinearDimension3d)); }

                    virtual GeometricElement const& _DimToElement() const override final { return *this; }

                    static LinearDimension3dCPtr    Get(DgnDbR db, DgnElementId id) { return db.Elements().Get<LinearDimension3d>(id); }
                    static LinearDimension3dPtr     GetForEdit(DgnDbR db, DgnElementId id) { return db.Elements().GetForEdit<LinearDimension3d>(id); }
                    LinearDimension3dCPtr           Insert() { return GetDgnDb().Elements().Insert<LinearDimension3d>(*this); }
                    LinearDimension3dCPtr           Update() { return GetDgnDb().Elements().Update<LinearDimension3d>(*this); }
                    DgnDbStatus                     Delete() const { return GetDgnDb().Elements().Delete(*this); }

DGNPLATFORM_EXPORT  static LinearDimension3dPtr     Create(CreateParams const& params, DgnElementId dimStyleId, DPoint3dCR endPoint);
};

namespace dgn_ElementHandler
{
    //=======================================================================================
    //! The ElementHandler for LinearDimension2d
    //=======================================================================================
    struct LinearDimensionHandler2d : Annotation2d
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_LinearDimension2d, LinearDimension2d, LinearDimensionHandler2d, Annotation2d, DGNPLATFORM_EXPORT);
        virtual void _TEMPORARY_GetPropertyHandlingCustomAttributes(ECSqlClassParams::PropertyHandlingCustomAttributes& params) override;  // *** WIP_AUTO_HANDLED_PROPERTIES
    };

    //=======================================================================================
    //! The ElementHandler for LinearDimension3d
    //=======================================================================================
    struct LinearDimensionHandler3d : Geometric3d
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_LinearDimension3d, LinearDimension3d, LinearDimensionHandler3d, Geometric3d, DGNPLATFORM_EXPORT);
        virtual void _TEMPORARY_GetPropertyHandlingCustomAttributes(ECSqlClassParams::PropertyHandlingCustomAttributes& params) override;  // *** WIP_AUTO_HANDLED_PROPERTIES
    };
}

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
END_BENTLEY_DGN_NAMESPACE
