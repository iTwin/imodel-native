/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/MeshHeaderHandler.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <DgnPlatform/DgnCore/DisplayHandler.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
#if defined (NEEDS_WORK_DGNITEM)

/** @addtogroup 3DElements */
/** @beginGroup */
/*=================================================================================**//**
* The default type handler for the MESH_HEADER_ELM type that corresponds to the 
* Mesh_header structure. A mesh is a complex element whose components hold the various
* data arrays that define the type of mesh.
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE MeshHeaderHandler : DisplayHandler,
                                    IMeshEdit
{
    DEFINE_T_SUPER(DisplayHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (MeshHeaderHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
protected:

// Handler
DGNPLATFORM_EXPORT virtual void        _GetTypeName (WStringR string, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual StatusInt   _OnTransform (EditElementHandleR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual StatusInt   _FenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR eh, FenceParamsP fp, FenceClipFlags options) override;
DGNPLATFORM_EXPORT virtual StatusInt   _OnFenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR, FenceParamsP, FenceClipFlags) override;
DGNPLATFORM_EXPORT virtual StatusInt   _OnFenceStretch (EditElementHandleR, TransformInfoCR, FenceParamsP, FenceStretchFlags) override;
DGNPLATFORM_EXPORT virtual void        _OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir) override;

// DisplayHandler
virtual bool                           _IsRenderable (ElementHandleCR) override {return true;}
DGNPLATFORM_EXPORT virtual void        _Draw (ElementHandleCR, ViewContextR) override;
DGNPLATFORM_EXPORT virtual void        _GetOrientation (ElementHandleCR, RotMatrixR) override;
DGNPLATFORM_EXPORT virtual SnapStatus  _OnSnap (SnapContextP, int snapPathIndex) override;
DGNPLATFORM_EXPORT virtual StatusInt   _OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry) override;

// IMeshEdit
DGNPLATFORM_EXPORT virtual BentleyStatus   _SetMeshData (EditElementHandleR eeh, PolyfaceQueryR) override;
DGNPLATFORM_EXPORT virtual BentleyStatus   _GetMeshData (ElementHandleCR source, PolyfaceHeaderPtr&) override;

public:

DGNPLATFORM_EXPORT static BentleyStatus   PolyfaceFromElement (PolyfaceHeaderPtr&, ElementHandleCR eh);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus CreateMeshAssoc (ElementHandleCR, AssocPoint&, SnapPathCR, int modifierMask, bool createFarPathElems, DgnModelP parentModel);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static StatusInt EvaluateMeshAssoc (ElementHandleCR, AssocPoint&, DPoint3dR point);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

//! Create a MESH_HEADER_ELM element from the supplied polyface data.
//! @param[out] eeh         The new element.
//! @param[in]  templateEh  Template element to use for symbology; if NULL defaults are used.
//! @param[in]  meshData    The polyface array data.
//! @param[in]  is3d        Initialize the 2d or 3d element structure, typically modelRef->Is3d ().
//! @param[in]  modelRef    Model to associate this element with. Required to compute range.
//! @return SUCCESS if a valid element is created and range was sucessfully calculated.
DGNPLATFORM_EXPORT static BentleyStatus CreateMeshElement (EditElementHandleR eeh, ElementHandleCP templateEh, PolyfaceQueryCR meshData, bool is3d, DgnModelR modelRef);

}; // MeshHeaderHandler
#endif

//__PUBLISH_SECTION_END__
#if defined (NEEDS_WORK_DGNITEM)

/*=================================================================================**//**
* NOTE: Provided only for legacy element_transform/shared cell drop!
*
* @bsiclass                                                     Brien.Bastings  06/05
+===============+===============+===============+===============+===============+======*/
struct          MatrixDoubleDataHandler : Handler
{
    DEFINE_T_SUPER(Handler)
    ELEMENTHANDLER_DECLARE_MEMBERS (MatrixDoubleDataHandler, DGNPLATFORM_EXPORT)
protected:

// Handler
DGNPLATFORM_EXPORT virtual void        _GetTypeName (WStringR string, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual StatusInt   _OnTransform (EditElementHandleR, TransformInfoCR) override;

}; // MatrixDoubleDataHandler
#endif

/*=================================================================================**//**
* Strongly typed readers for various matrix types.
* @bsiclass                                                     Earlin.Lutz 04/09
+===============+===============+===============+===============+===============+======*/
struct          MatrixHeaderUtils
{
public:

//! Read from a MATRIX_HEADER_ELM into a bvector of int.
DGNPLATFORM_EXPORT static bool Read (ElementHandleCR eh, bvector <int> &data);
//! Read from a MATRIX_HEADER_ELM into a bvector of double.
DGNPLATFORM_EXPORT static bool Read (ElementHandleCR eh, bvector <double> &data);
//! Read from a MATRIX_HEADER_ELM, grouping doubles into DPoint3d structs
DGNPLATFORM_EXPORT static bool Read (ElementHandleCR eh, bvector <DPoint3d> &data);
//! Read from a MATRIX_HEADER_ELM, grouping doubles into DPoint2d structs
DGNPLATFORM_EXPORT static bool Read (ElementHandleCR eh, bvector <DPoint2d> &data);
//! Read from a MATRIX_HEADER_ELM, grouping doubles into DVec3d structs
DGNPLATFORM_EXPORT static bool Read (ElementHandleCR eh, bvector <DVec3d> &data);
//! Read from a MATRIX_HEADER_ELM, grouping doubles into RgbFactor structs
DGNPLATFORM_EXPORT static bool Read (ElementHandleCR eh, bvector <RgbFactor> &data);
//! Read from a MATRIX_HEADER_ELM into a bvector of UInt32.
DGNPLATFORM_EXPORT static bool Read (ElementHandleCR eh, bvector <UInt32> &data);
//! Read count data from a MATRIX_HEADER_ELM.
DGNPLATFORM_EXPORT static bool Read (ElementHandleCR headerHandle, UInt32 &numPerStruct,
                                    UInt32 &numPerRow, UInt32 &tag, UInt32 &indexFamily, UInt32 &indexedBy);

/**
Creates a single matrix element.  The matrix element must reside under a matrix header.
*/
DGNPLATFORM_EXPORT static void   CreateMatrixHeader
(
DgnElementR      out,
ElementHandleCP    templateEh,
UInt32          numPerStruct,
UInt32           numPerRow,
UInt32          tag,
UInt32          indexFamily,
UInt32          indexedBy,
DgnModelR    modelRef
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      08/00
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static size_t MatrixDataElementSize
(
size_t structSize,
size_t maxValue
);

/*---------------------------------------------------------------------------------**//**
* @remarks Ignore attributes on template; this is a non-graphics element.
* @bsimethod                                                    EarlinLutz      08/00
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static void CreateMatrixData
(
DgnElementR      out,
ElementHandleCP    pIn,
size_t          structSize,
size_t          maxValue,
size_t          numValue,
int             transformType,
const void*     pData,
UInt16          elementType,
DgnModelR    modelRef
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      08/00
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static void CreateMatrixIntData
(
DgnElementR      out,
ElementHandleCP    pIn,
size_t          maxValue,
size_t          numValue,
int             transformType,
const int*      pData,
DgnModelR    modelRef
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      08/00
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static void CreateMatrixDoubleData (DgnElementR out, ElementHandleCP pIn, size_t maxValue, size_t numValue, int transformType, const double* pData, DgnModelR modelRef);

DGNPLATFORM_EXPORT static MSElementDescrPtr CreateIntDataChain (ElementHandleCP templateEh, BlockedVectorIntR data, int transformType, DgnModelR modelRef);
DGNPLATFORM_EXPORT static MSElementDescrPtr CreateIntDataChain (ElementHandleCP templateEh, BlockedVectorUInt32R data, int transformType, DgnModelR modelRef);
DGNPLATFORM_EXPORT static MSElementDescrPtr CreateIntDataChain (ElementHandleCP templateEh, int const *data, size_t numStructs, UInt32 numPerStruct, UInt32 structsPerRow, UInt32 tag, UInt32 indexFamily, UInt32 indexedBy, int transformType, DgnModelR modelRef);
DGNPLATFORM_EXPORT static MSElementDescrPtr CreateDoubleDataChain (ElementHandleCP templateEh, double const *data, size_t numStructs, UInt32 numPerStruct, UInt32 structsPerRow, UInt32 tag, UInt32 indexFamily, UInt32 indexedBy, int transformType, DgnModelR modelRef);
DGNPLATFORM_EXPORT static MSElementDescrPtr CreateDoubleDataChain (ElementHandleCP templateEh, BlockedVectorDPoint3dR data, int transformType, DgnModelR modelRef);
DGNPLATFORM_EXPORT static MSElementDescrPtr CreateDoubleDataChain (ElementHandleCP templateEh, BlockedVectorDVec3dR data, int transformType, DgnModelR modelRef);
DGNPLATFORM_EXPORT static MSElementDescrPtr CreateDoubleDataChain (ElementHandleCP templateEh, BlockedVectorDPoint2dR data, int transformType, DgnModelR modelRef);
DGNPLATFORM_EXPORT static MSElementDescrPtr CreateDoubleDataChain (ElementHandleCP templateEh, BlockedVectorRgbFactorR data, int transformType, DgnModelR modelRef);

/*---------------------------------------------------------------------------------**//**
* Transform the doubles in this data element according to the transform type.
*
* @remarks The dimension of the structs in this matrix data element is encoded in its
*       transform type.  Dimension affects the transformation of a struct by the transform
*       <I>T</I> = [<I>M t</I>] as follows:
* <UL>
* <LI>1D: the <I>y/z</I>-rows/columns of M and the <I>y/z</I>-coordinates of <I>t</I> are irrelevant.  Note
*              that this means that we scale by the value of <I>M</I><SUB>0,0</SUB>, <EM>not</EM> by the
*              magnitude of the first column of <I>M</I>.
* <LI>2D: the <I>z</I>-row/column of <I>M</I> and the <I>z</I>-coordinate of <I>t</I> are irrelevant.
* <LI>3D: all of <I>T</I> is relevant.
* <LI>4D: data are not transformed by this function.
* </UL>
* @remarks The coordinate type of the structs in this matrix data element is encoded in
*       its transform type.  Coordinate type affects the transformation of a struct <I>p</I> by the
*       transform <I>T</I> = [<I>M t</I>] as follows:
* <UL>
* <LI>Point: [<I>M t</I>] <I>p</I> = <I>M p</I> + <I>t</I>
* <LI>Vector: [<I>M t</I>] <I>p</I> = <I>M p</I>
* <LI>Covector: [<I>M t</I>] <I>p</I> = <I>M</I><SUP>-^</SUP> <I>p</I>, e.g., multiplication by the
*               inverse transpose (this preserves orthogonality of the covectors)
* </UL>
* @remarks The normalization of the structs in this matrix data element is encoded in its
*       transform type.  Transformation of a normalized struct proceeds as above, and ends with
*       re-normalization to ensure that each struct has unit length.
* @param pMatrixDoubleDataElement   IN OUT  element to transform
* @param pTransform                 IN      transform
* @return SUCCESS if the transform was successful, or if it was not required
* @group    "Matrix Elements"
* @see mdlMatrixDoubleData_decodeTransformType
* @bsimethod                                    DavidAssaf      10/00
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static StatusInt TransformDoubleDataElementInPlace
(
        DgnElementP pMatrixDoubleDataElement,
TransformCP pTransform
);

/*---------------------------------------------------------------------------------**//**
* Validate the transform type for a matrix double data element.
*
* @param transformType  IN      code to validate
* @return SUCCESS if valid transform type code
* @group    "Matrix Elements"
* @bsimethod                                    DavidAssaf      12/00
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus   ValidateDoubleTransformType
(
int transformType
);

/*---------------------------------------------------------------------------------**//**
* Construct the transform type for a matrix double data element.
* <P>
* Pass MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_NONE for coordType if data is not to be
*       transformed (in this case, the other inputs are ignored).
* @param pTransformType OUT     code to encipher
* @param coordType      IN      MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_*
* @param dimension      IN      (1, 2, 3 or 4) dimension of data
* @param bNormalized    IN      true if unit length is preserved under transformation
* @return SUCCESS if valid transform type code
* @group    "Matrix Elements"
* @bsimethod                                    DavidAssaf      12/00
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus EncodeTransformType
(
int         *pTransformType,
int         coordType,
int         dimension,
bool        bNormalized
);

/*---------------------------------------------------------------------------------**//**
* Return the information encoded in the transform type of a matrix double
*       data element.  NULL may be supplied for any combination of output parameters.
* <P>
* If pCoordType returns (or would return, if it were not NULL) with the value
*       MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_NONE, then the other outputs are unfilled.
*       This indicates the matrix data is not transformable.
* @param transformType  IN      code to decipher
* @param pCoordType     OUT     MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_*
* @param pDimension     OUT     (1, 2, 3 or 4) dimension of data
* @param pbNormalized   OUT     true if unit length is preserved under transformation
* @return SUCCESS if valid transform type code
* @group    "Matrix Elements"
* @bsimethod                                    DavidAssaf      12/00
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus DecodeTransformType
(
int         transformType,
int         *pCoordType,
int         *pDimension,
bool        *pbNormalized
);

/*---------------------------------------------------------------------------------**//**
* Transform the given array of (scalar) doubles according to the transform type code.
*
* @param pBuf           IN OUT  array to transform
* @param count          IN      # points in array
* @param pTransform     IN      transform
* @param transformType  IN      indicates how structs are transformed
* @return SUCCESS if the transform was successful, or if it was not required
* @group    "Matrix Elements"
* @see mdlMatrixDoubleDataElement_transform
* @bsimethod                                    DavidAssaf      02/04
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus TransformScalarBuffer
(
        double*     pBuf,
        int         count,
TransformCP   pTransform,
        int         transformType
);

/*---------------------------------------------------------------------------------**//**
* Transform the given array of DPoint2d according to the transform type code.
*
* @param pBuf           IN OUT  array to transform
* @param count          IN      # points in array
* @param pTransform     IN      transform
* @param transformType  IN      indicates how structs are transformed
* @return SUCCESS if the transform was successful, or if it was not required
* @group    "Matrix Elements"
* @see mdlMatrixDoubleDataElement_transform
* @bsimethod                                    DavidAssaf      11/00
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus TransformDPoint2dBuffer
(
        DPoint2d    *pBuf,
        int         count,
TransformCP pTransform,
        int         transformType
);

/*---------------------------------------------------------------------------------**//**
* Transform the given array of DPoint3d according to the transform type code.
*
* @param pBuf           IN OUT  array to transform
* @param count          IN      # points in array
* @param pTransform     IN      transform
* @param transformType  IN      indicates how structs are transformed
* @return SUCCESS if the transform was successful, or if it was not required
* @group    "Matrix Elements"
* @see mdlMatrixDoubleDataElement_transform
* @bsimethod                                    DavidAssaf      11/00
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus TransformDPoint3dBuffer
(
        DPoint3dP pBuf,
        int         count,
TransformCP pTransform,
        int         transformType
);
/*---------------------------------------------------------------------------------**//**
* Transform the given array of doubles according to the transform type code.
*
* @param pBuf           IN OUT  array to transform
* @param count          IN      # doubles in array
* @param pTransform     IN      transform
* @param transformType  IN      indicates how structs are transformed
* @return SUCCESS if the transform was successful, or if it was not required
* @group    "Matrix Elements"
* @see mdlMatrixDoubleDataElement_transform
* @bsimethod                                    DavidAssaf      12/00
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus TransformBuffer
(
        double      *pBuf,
        int         count,
TransformCP pTransform,
        int         transformType
);
/*---------------------------------------------------------------------------------**//**
* Validate the index type (aka transform type) for a matrix int data element.
*
* @param transformType  IN      code to validate
* @return SUCCESS if valid transform type code
* @group    "Matrix Elements"
* @bsimethod                                    DavidAssaf      12/00
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus   ValidateIntIndexType
(
int transformType
);

/*---------------------------------------------------------------------------------**//**
* Return the information encoded in the index type (aka transform type)
*       of a matrix int data element.  NULL may be supplied for any combination of
*       output parameters.
* <P>
* If pBlocking returns (or would return, if it were not NULL) with the value
*       MATRIX_DATA_ELM_INDEX_TYPE_BLOCK_NONE, then the other outputs are unfilled.
*       This indicates the matrix data are not to be interpreted as indices.
* @param transformType  IN      code to decipher
* @param pBlocking      OUT     MATRIX_DATA_ELM_INDEX_TYPE_BLOCK_*
* @param pBase          OUT     (0 or 1) n-based data
* @param pbPadded       OUT     true if blocks are padded/terminated
* @param pPad           OUT     (-1 or 0) pad/terminator for fixed/variable blocks; returned only if pbPadded is true
* @param pbSigned       OUT     true if indices can be signed
* @return SUCCESS if valid transform type code
* @group    "Matrix Elements"
* @bsimethod                                    DavidAssaf      12/00
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus   DecodeIndexType
(
int         transformType,
int         *pBlocking,
int         *pBase,
bool        *pbPadded,
int         *pPad,
bool        *pbSigned
);

/*---------------------------------------------------------------------------------**//**
* Construct the index type (aka transform type) for a matrix int data
*       element.  The pad/terminator of a fixed/variable blocked matrix is automatically
*       set according to the given base: 0 base => -1 pad; 1 base => 0 pad.
* <P>
* Pass MATRIX_DATA_ELM_INDEX_TYPE_BLOCK_NONE for blocking if ints should not be
*       interpreted as indices (in this case, the other inputs are ignored).
* @param pTransformType OUT     code to encipher
* @param blocking       IN      MATRIX_DATA_ELM_INDEX_TYPE_BLOCK_*
* @param base           IN      0 or 1 for 0-based or 1-based indices
* @param bPadded        IN      true to set block pad/terminator according to base; false for no padding
* @param bSigned        IN      true if indices can be signed
* @return SUCESS if valid transform type code
* @group    "Matrix Elements"
* @bsimethod                                    DavidAssaf      12/00
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus   EncodeIndexType
(
int         *pTransformType,
int         blocking,
int         base,
bool        bPadded,
bool        bSigned
);


}; // MatrixHeaderUtils

/*=================================================================================**//**
* @bsiclass                                                     Earlin.Lutz 04/09
+===============+===============+===============+===============+===============+======*/
struct          MeshHeaderUtils
{
public:

/**

*/
DGNPLATFORM_EXPORT static void CreateMeshHeader
(
DgnElementR      out,
ElementHandleCP    templateEh,
int             meshStyle,
bool            bIs3d,
DgnModelR    modelRef
);

// Strongly typed access to meshStyle out of matrix header ... 
DGNPLATFORM_EXPORT static bool Read (ElementHandleCR headerHandle, UInt32 &meshStyle);
}; // MatrixHeaderUtils

//__PUBLISH_SECTION_START__
/** @endGroup */
END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
