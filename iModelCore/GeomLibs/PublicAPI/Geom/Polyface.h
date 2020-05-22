/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/
//! @file Polyface.h  Data for face containing facets: FacetFaceData, DSegment3dOnFacets, BlockedVectorInt, FacetLocationDetail, FacetLocationDetailPair, PolyfaceEdgeChain, DirectionalVolumeData, PolyfaceQuery,    PolyfaceQuery::IClipToPlaneSetOutput, PolyfaceQueryCarrier, PolyfaceVectors, PolyfaceHeader, PolyfaceVisitor, PolyfaceCoordinateMap
#if defined (__cplusplus)
#include <Bentley/bmap.h>
#include <Bentley/bvector.h>
#include <Bentley/WString.h>
#include <Bentley/RefCounted.h>
#include <Bentley/NonCopyableClass.h>

#include <Geom/PolyfaceAuxData.h>

#include <limits.h>
#include <cstdlib> // for std::abs
/*__PUBLISH_SECTION_END__*/
#include <Geom/BinaryRangeHeap.h>
/*__PUBLISH_SECTION_START__*/
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

typedef RefCountedPtr<PolyfaceHeader>  PolyfaceHeaderPtr;               //!< @ingroup BentleyGeom_Mesh
typedef RefCountedPtr<PolyfaceVisitor> PolyfaceVisitorPtr;              //!< @ingroup BentleyGeom_Mesh
typedef RefCountedPtr<PolyfaceCoordinateMap> PolyfaceCoordinateMapPtr;  //!< @ingroup BentleyGeom_Mesh

typedef PolyfaceCoordinateMap *PolyfaceCoordinateMapP;
typedef PolyfaceCoordinateMap &PolyfaceCoordinateMapR;


//=======================================================================================
//!
//! Data for face containing facets.
//!  This is built up cooperatively by the IPolyfaceConstruction and its callers
//!  and stored as FaceData array in PolyfaceHeader.
//!
//=======================================================================================
struct FacetFaceData
{
DRange2d m_paramDistanceRange;
//! Range that parameters actually span in their stored form.
DRange2d m_paramRange;

//! construct with null ranges, 0 indices.
GEOMDLLIMPEXP FacetFaceData ();
//! restore to constructor state.
GEOMDLLIMPEXP void Init ();

//! convert parameter from stored value to distance-based parameter.
GEOMDLLIMPEXP void ConvertParamToDistance (DPoint2dR distanceParam, DPoint2dCR param) const;

//! convert parameter from stored value to normalized (0-1) parameter.
GEOMDLLIMPEXP void ConvertParamToNormalized (DPoint2dR normalizedParam, DPoint2dCR param) const;


//! Scale distance parameters
GEOMDLLIMPEXP void ScaleDistances (double distanceScale);
//! To be called just after one or more "one-based, zero terminated" facets have been added to the polyface.
//! The new facets are identified as a "face" and the face size data is recorded.
//! Face size is tied to parameter range by scale factors from simple triangulation of all facets.
GEOMDLLIMPEXP void SetParamDistanceRangeFromNewFaceData (PolyfaceHeaderR polyface, size_t endIndex = 0);
};

//! Struct to carry a DSegment3d with construction history (in facet merge)
struct DSegment3dOnFacets : DSegment3d
{
enum class HistoryBit
    {
    Null = 0,
    Transverse  = 1,
    EdgeOfA     = 2,
    EdgeOfB     = 4
    };
uint32_t m_history;

// Constructor from just a DSegement3d with no history . . .
DSegment3dOnFacets (DSegment3dCR segment)
    : DSegment3d(segment), m_history((uint32_t)HistoryBit::Null){}
DSegment3dOnFacets (DSegment3dCR segment, HistoryBit bit)
    : DSegment3d(segment), m_history((uint32_t)bit){}

DSegment3dOnFacets (DPoint3dCR xyzA, DPoint3dCR xyzB, HistoryBit bit)
    : m_history((uint32_t)bit)
    {
    point[0] = xyzA;
    point[1] = xyzB;
    }

void SetBit (HistoryBit bit){m_history |= (uint32_t)bit;}
bool HasBit (HistoryBit bit){return 0 != (m_history & (uint32_t)bit);}
};

typedef ValueSizeSize <DSegment3dOnFacets> DSegment3dSizeSize;





END_BENTLEY_GEOMETRY_NAMESPACE

#include <Geom/FacetOptions.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/**
@ingroup BentleyGeom_MeshSupport
   A BlockedVector<T> is an in-memory bvector<T> augmented with blocking and context information corresponding to a DGN file "MATRIX_ELM".

   <p>The following information in the BlockedVector directly matches what is stored in a DGN file MATRIX_HEADER_ELM:
   <TABLE BORDER="1">
   <TR><TD>NumPerStruct</TD> <TD>Number of machine primitives (int or double) in the template structure T.</TD>
   </TR>
   <TR><TD>StructsPerRow</TD> <TD>For coordinate-grid meshes, this is the number of points in a grid row.</TD>
   </TR>
   <TR><TD>Tag</TD> <TD>Tag to identify the role of this array.</TD>
   </TR>
   <TR><TD>IndexFamily</TD> <TD>Describes interpretation of the index data</TD>
   </TR>
   <TR><TD>IndexedBy</TD> <TD>Tag of the index array that references data in this array.</TD>
   </TR>
   </TD>
   </TR>
   </TABLE>
   </p>

   <p>In addition, the BlockedVector has a boolean Active.  In a runtime PolyfaceVectors structure, all possible arrays are present, and those not in use
   for a particular mesh instance have the Active property false.

   The NumPerStruct values for common arrayed type T are:
   <TABLE BORDER="1">
   <TR>
   <TH>typename T
   </TH>
   <TH>matrix primitive
   </TH>
   <TH>NumPerStruct
   </TH>
   </TR>
   <TR><TD>DPoint3d</TD> <TD>double</TD>   <TD>3</TD>
   </TR>
   <TR><TD>DPoint2d</TD> <TD>double</TD>   <TD>2</TD>
   </TR>
   <TR><TD>DVec3d</TD> <TD>double</TD>   <TD>3</TD>
   </TR>
   <TR><TD>RgbFactor</TD> <TD>double</TD>   <TD>3</TD>
   </TR>
   <TR><TD>int</TD> <TD>int</TD>   <TD>1</TD>
   </TR>
   <TR><TD>double</TD> <TD>double</TD>   <TD>1</TD>
   </TR>
   </TABLE>
*/
template <typename T>
struct BlockedVector : bvector <T>
{
protected:
    //! Number of primitive values (ints or doubles) in a single struct.
    uint32_t m_numPerStruct;
    //! Number of structs considered as a single row.
    uint32_t m_structsPerRow;
    //! Application tag.
    uint32_t m_tag;              // application tag
    uint32_t m_indexFamily;
    //! Tag of the blocked vector that indexes into this one.
    uint32_t m_indexedBy;
    //! Indicates if this blocked vector is to be filled.
    //! (A Polyface header declares all possible blocked vectors (e.g. multiple color representations) but does not have them all in use for all meshes.)
    bool m_active;
public:
    //! Constructor with optional tag values.
    GEOMDLLIMPEXP BlockedVector (uint32_t numPerStruct,
        uint32_t structsPerRow = 0, uint32_t tag = 0, uint32_t indexFamily = 0, uint32_t indexedBy = 0, bool active = false);
    //! Constructor, fills all tags with zero.
    GEOMDLLIMPEXP BlockedVector ();

public:
//! Query the number of machine primitves (ints or doubles, depending on the MATRIX_ELM type) that constitute a single struct of the BlockedVector.
    GEOMDLLIMPEXP uint32_t NumPerStruct () const;
//! Query the number of structs considered a "row".
    GEOMDLLIMPEXP uint32_t StructsPerRow () const;
//! Set the number of structs considered a "row".
    GEOMDLLIMPEXP void   SetStructsPerRow (uint32_t num);
//! Return the context tag.
    GEOMDLLIMPEXP uint32_t Tag () const;
//! Return the IndexFamily.
    GEOMDLLIMPEXP uint32_t IndexFamily () const;
//! Return the IndexedBy
    GEOMDLLIMPEXP uint32_t IndexedBy () const;
//! Set all blocking and tag data at once
    GEOMDLLIMPEXP void SetTags (uint32_t numPeStruct, uint32_t structsPerRow, uint32_t tag, uint32_t IndexFamily, uint32_t IndexedBy, bool active);
//! Query if the BlockedVector has been marked active.  (In memory mesh objects will have empty, inactive arrays.)
    bool Active () const {return this->m_active;}
//! Mark the BlockedVector active.
    void SetActive (bool active) {this->m_active=active;}
//! clear the bvector, then append (push_back) from source vector.  Leave blocking data alone.
    GEOMDLLIMPEXP void CopyVectorFrom (bvector<T>&source);

//! Clear this vector.   Append up to n values from source, with first source index i0.   Replicate leading numWrap values at end.
    GEOMDLLIMPEXP uint32_t ClearAndAppendBlock (BlockedVector<T> &source, uint32_t i0, uint32_t numWrap, uint32_t n);
//! Clear this vector.   Append up to n values from source, with first source index i0.   Replicate leading numWrap values at end.
    GEOMDLLIMPEXP uint32_t ClearAndAppendBlock (T const *source, size_t sourceSize, uint32_t i0, uint32_t numWrap, uint32_t n);

//! Clear this vector.   Append all data from source
    GEOMDLLIMPEXP void ClearAndAppend (bvector<T> const &source);

//! clear the bvector, then append (push_back) {numItem+numWrap} source values identified by contiguous indices in index array.
    //! Fill zeroBasedIndices vector to record source index
    //! Optionally record sign in oneBasedIndices vector.
    //! Return the number actually copied, whether delimited by end of array, 0 terminator, or invalid index.
    GEOMDLLIMPEXP uint32_t ClearAndAppendByOneBasedIndices
            (
            bvector<int> &zeroBasedIndices,
            bvector<BoolTypeForVector> *positive,
            bvector<T> &source,
            bvector<int> &oneBasedIndices,
            uint32_t i0,
            uint32_t numItem,
            uint32_t numWrap
            );

//! clear the bvector, then append (push_back) {numItem+numWrap} source values identified by contiguous indices in index array.
    //! Fill zeroBasedIndices vector to record source index
    //! Optionally record sign in oneBasedIndices vector.
    //! No actions if source pointers are null.
    //! Return the number actually copied, whether delimited by end of array, 0 terminator, or invalid index.
    GEOMDLLIMPEXP uint32_t ClearAndAppendByOneBasedIndices
            (
            bvector<int> &zeroBasedIndices,
            bvector<BoolTypeForVector> *positive,
            T const *pSource,
            size_t sourceCount,
            int const *oneBasedIndices,
            size_t oneBasedIndexCount,
            uint32_t i0,
            uint32_t numItem,
            uint32_t numWrap
            );

//! Simple append from array.  SetActive and return new size.
    GEOMDLLIMPEXP size_t Append
            (
            T const *pBuffer,
            size_t count
            );
//! Simple append from array.  SetActive and return new size.
    GEOMDLLIMPEXP size_t Append
            (
            BlockedVector<T> const &source
            );

//! Simple append one value.   SetActive and return new size.
    GEOMDLLIMPEXP size_t Append (T const &source);

//! Simple append one value.   SetActive and return index where the new item appears.
    GEOMDLLIMPEXP size_t AppendAndReturnIndex (T const &source);

//! Return the number of complete rows, based on StructsPerRow ad actual size.
    GEOMDLLIMPEXP size_t NumCompleteRows ();

//! Return pointer to the flat buffer.  Illicit in std, but ....
    GEOMDLLIMPEXP T* GetPtr ();

//! Return pointer to the flat buffer.  Illicit in std, but ....
    GEOMDLLIMPEXP T const * GetCP () const;

//! Reverse entries iFirst < i < iLast

    GEOMDLLIMPEXP void ReverseInRange (size_t iFirst, size_t iLast);

//! Copy (or quietly ignore range errors) from one place to another

    GEOMDLLIMPEXP void CopyData (size_t fromIndex, size_t toIndex);

//! Copy {count} values (or quietly ignore range errors) from index0 to 0.
//!        Trim all others.

    GEOMDLLIMPEXP void Trim (size_t index0, size_t count);

//! Checked access ..
    GEOMDLLIMPEXP bool TryGetAt (size_t index, T const &defaultValue, T &value) const;
};

//=======================================================================================
//! A BlockedVectorInt specialized BlockedVector<int> with services for manipulating the int values as mesh indices.
struct BlockedVectorInt : BlockedVector<int>
{
/*__PUBLISH_SECTION_END__*/
//! Constructor with all auxilliary data 0.
BlockedVectorInt () : BlockedVector<int> () {}

//! Constructor with optional tag valus.
BlockedVectorInt (uint32_t numPerStruct,
        uint32_t structsPerRow = 0, uint32_t tag = 0, uint32_t indexFamily = 0, uint32_t indexedBy = 0, bool active = false)
    : BlockedVector<int> (numPerStruct, structsPerRow, tag, indexFamily, indexedBy, active)
    {}
/*__PUBLISH_SECTION_START__*/
//!
//! If the current array has blocked structsPerRow, expand to variable length 0-terminated form. (ASSUMES all zeros in blocked form are placeholders.)
//!
GEOMDLLIMPEXP void ConvertBlockedToZeroTerminated ();

//!
//! Count zeros in the vector.
//!
GEOMDLLIMPEXP size_t CountZeros ();

//!
//! Add numRow blocks of numPerRow sequential values with terminator after each row.
//!
GEOMDLLIMPEXP void AddTerminatedSequentialBlocks
(
size_t numRow,
size_t numPerRow,
bool clearFirst = false,
int firstValue = 1,
int terminator = 0
);

//!
//! Add one row with wraparound, optional terminator.   NO ACTION if the array is not active !!!!
//!
GEOMDLLIMPEXP void AddSequentialBlock
(
int firstValue,
size_t numValue,
size_t numWrap,
size_t numTrailingZero = 0,
bool clearFirst = false
);

//!
//! Add one row with wraparound, optional terminator.   NO ACTION if the array is not active !!!!
//!
GEOMDLLIMPEXP void AddSteppedBlock
(
    int firstValue,
    int valueStep,
    size_t numValue,
    size_t numWrap,
    size_t numTrailingZero = 0,
    bool clearFirst = false
);

//!
//! Add one row with terminator or pad.
//!
GEOMDLLIMPEXP bool AddAndTerminate
(
int *pValues,
size_t numValues
);
//!
//! Create indices for a rectangular grid.
//!
GEOMDLLIMPEXP void AddTerminatedGridBlocks
(
size_t numRow,
size_t numPerRow,
size_t rowStep,
size_t colStep,
bool triangulated,
bool clearFirst,
int firstValue,
int terminator);

//! Negate a portion of the array.

GEOMDLLIMPEXP void NegateInRange (size_t iFirst, size_t iLast);
//! Shift signs from (cyclic) predecessors.

//! for each k in the inclusive range kFirst<=k<=kLast, set the sign to the prior value from its
GEOMDLLIMPEXP void ShiftSignsFromCyclicPredecessorsInRange (size_t kFirst, size_t kLast);
//! for each k in the inclusive range kFirst<=k<=kLast, set the entry to its absolute value.
GEOMDLLIMPEXP void AbsInRange (size_t iFirst, size_t iLast);
//! Set each entry to its absolute value
GEOMDLLIMPEXP void Abs ();
//! return true if all entries in the inclusive range kFirst<=k<=kLast are negative.
GEOMDLLIMPEXP bool AllNegativeInRange (size_t iFirst, size_t iLast);
//! for each k in the inclusive range kFirst<=k<=kLast, set the entry to the negative of its absolute value.
GEOMDLLIMPEXP void NegativeAbsInRange (size_t iFirst, size_t iLast);
//! From given start position, find final (inclusive) position and position for next start search.
//!  Initialize iFirst to zero before first call.
//!  Return false if no more faces.
GEOMDLLIMPEXP bool DelimitFace (int numPerFace, size_t iFirst, size_t &iLast, size_t &iNext);

//! Return min and max values in entire vector.
//! @param [out] minValue smallest value, INT_MAX if empty array.
//! @param [out] maxValue largest value, INT_MIN if empty array.
GEOMDLLIMPEXP bool MinMax (int &minValue, int &maxValue) const;

//! Append from source array, shifting each nonzero index by signed shift

GEOMDLLIMPEXP void AppendShifted (BlockedVectorInt const & source, int shift);

//! enumeration for sign change effects on indices after reversal by ReverseIndicesOneFace and ReverseIndicesAllFaces
enum IndexAction
    {
    None,
    ForcePositive,
    ForceNegative,
    Negate
    };
};


typedef BlockedVector<DPoint3d>&            BlockedVectorDPoint3dR;
typedef BlockedVector<DPoint3d> const &     BlockedVectorDPoint3dCR;
typedef BlockedVector<DPoint3d> const *     BlockedVectorDPoint3dCP;

typedef BlockedVector<DPoint2d>&            BlockedVectorDPoint2dR;
typedef BlockedVector<DPoint2d> const&      BlockedVectorDPoint2dCR;
typedef BlockedVector<DPoint2d>*            BlockedVectorDPoint2dP;
typedef BlockedVector<DPoint2d> const *     BlockedVectorDPoint2dCP;

typedef BlockedVector<DVec3d>&              BlockedVectorDVec3dR;
typedef BlockedVector<DVec3d> const&        BlockedVectorDVec3dCR;
typedef BlockedVector<DVec3d>*              BlockedVectorDVec3dP;
typedef BlockedVector<DVec3d> const*        BlockedVectorDVec3dCP;

typedef BlockedVectorInt&                   BlockedVectorIntR;
typedef BlockedVectorInt const&             BlockedVectorIntCR;
typedef BlockedVectorInt*                   BlockedVectorIntP;
typedef BlockedVectorInt const*             BlockedVectorIntCP;

typedef BlockedVector<uint32_t>&              BlockedVectorUInt32R;
typedef BlockedVector<uint32_t> const&        BlockedVectorUInt32CR;
typedef BlockedVector<uint32_t>*              BlockedVectorUInt32P;
typedef BlockedVector<uint32_t> const*        BlockedVectorUInt32CP;

typedef BlockedVector<double>&              BlockedVectorDoubledR;
typedef BlockedVector<double> const&        BlockedVectorDoubledCR;
typedef BlockedVector<double>   *           BlockedVectorDoubledP;
typedef BlockedVector<double> const*        BlockedVectorDoubledCP;

typedef BlockedVector<RgbFactor>&           BlockedVectorRgbFactorR;
typedef BlockedVector<RgbFactor> const &    BlockedVectorRgbFactorCR;
typedef BlockedVector<RgbFactor>*           BlockedVectorRgbFactorP;
typedef BlockedVector<RgbFactor> const *    BlockedVectorRgbFactorCP;

typedef BlockedVector<FloatRgb>&            BlockedVectorFloatRgbR;

typedef BlockedVector<CurveTopologyId>&     BlockedVectorCurveTopologyIdR;




//! Complete data for a point "within a facet"
struct FacetLocationDetail
{
bool isInteriorPoint;
size_t   activeMask;
size_t readIndex;

DPoint3d point; // interpolated
DPoint2d param; // interpolated
DVec3d   normal;    // interpolated
#define MAX_FACET_LOCATION_INDEX 4
size_t sourceIndex[MAX_FACET_LOCATION_INDEX];
double sourceFraction[MAX_FACET_LOCATION_INDEX];
uint32_t intColor[MAX_FACET_LOCATION_INDEX];
uint32_t colorIndex[MAX_FACET_LOCATION_INDEX];    // can't be interpolated?

int    numSourceIndex;

double a;
DVec3d dXdu;
DVec3d dXdv;
//! Zero all contents.
GEOMDLLIMPEXP void Zero ();
//! Default constructor -- zeros all contents.
GEOMDLLIMPEXP FacetLocationDetail ();
//! Constructor assigning read index and optional "a" value
GEOMDLLIMPEXP FacetLocationDetail (size_t readIndex, double a = 0.0);

//! accumulate a scaled multiple of all numeric data from source.
//! append index data and fraction arrays to arrays (if possible within space restrictions).
//!    Fractions from the source are scaled by the new fraction.
GEOMDLLIMPEXP void AccumulateScaledData (FacetLocationDetailCR source, double fraction);
//! Copy point data. Return false if not available.
GEOMDLLIMPEXP bool TryGetPoint (DPoint3dR data) const;
//! Copy parameter data. Return false if not available.
GEOMDLLIMPEXP bool TryGetParam (DPoint2dR data) const;
//! Copy normal data. Return false if not available.
GEOMDLLIMPEXP bool TryGetNormal (DVec3dR data) const;

//! Return the number of vertices that are weighted together for the computed
//! values.   This (along with the weight fractions and intColor)
//! can be used compute additional data accessed through the indices.
//! At most 4 weights can be stored.
//! The possible data is:
//! <ul>
//! <li>vertex index -- the index of the vertex (numbered within the facet)
//! <li>fraction -- (double) the weight used for data at this indexed vertex.
//! <li>intColor -- integer color data.
//! </ul>
GEOMDLLIMPEXP size_t GetNumWeights () const;
//! Access (for an index into the tables within the detail) a weight
//!    for contributing data.
GEOMDLLIMPEXP bool TryGetWeight (size_t index, double &weight) const;
//! Access (for an index into the tables within the detail) the vertex index
//!    (numbered within the facet)
GEOMDLLIMPEXP bool TryGetVertexIndex (size_t index, size_t &data) const;
//! Access (for an index into the tables within the detail) the int color
GEOMDLLIMPEXP bool TryGetIntColor (size_t index, uint32_t &data) const;
//! Set flag as interior point
GEOMDLLIMPEXP void SetIsInterior (bool value);
//! Get interior point flag
GEOMDLLIMPEXP bool GetIsInterior () const;
//! Get read index
GEOMDLLIMPEXP size_t GetReadIndex () const;
//! Set read index
GEOMDLLIMPEXP void SetReadIndex (size_t readIndex);
//! Get next read index
GEOMDLLIMPEXP size_t GetNextReadIndex () const;

//! Set the normal and record that it is valid.
GEOMDLLIMPEXP void SetNormal (DVec3dCR data);
GEOMDLLIMPEXP void SetParam (DPoint2dCR data);

//! lexical compare in u,v coordinates.
GEOMDLLIMPEXP bool CompareUV (FacetLocationDetail const &other) const;
//! Sort an array based by CompareUV ...
static GEOMDLLIMPEXP void SortUV (bvector<FacetLocationDetail> &data);

//! compare by "a" coordinate
GEOMDLLIMPEXP bool CompareA (FacetLocationDetail const &other) const;
//! Sort an array based by CompareA
static GEOMDLLIMPEXP void SortA (bvector<FacetLocationDetail> &data);

};

//! Pair of FacetLocationDetail, e.g. scan line hits before and after a pick point.
struct FacetLocationDetailPair
{
FacetLocationDetail detailA;
FacetLocationDetail detailB;
};

struct FacetFractionDetail
{
size_t readIndex;
double fraction;
};




//=======================================================================================
//!
//! Data for describing a polyface edge.
//! @ingroup BentleyGeom_MeshMarkup
//!

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     05/2017
//=======================================================================================
struct  PolyfaceEdge
    {
    int32_t                m_indices[2];

    PolyfaceEdge() { }
    GEOMDLLIMPEXP PolyfaceEdge(uint32_t index0, uint32_t index1);

    GEOMDLLIMPEXP bool operator < (PolyfaceEdge const& rhs) const;
    };


//=======================================================================================
//!
//! Data for describing a set of connected edges of a polyface and their source.
//! @ingroup BentleyGeom_MeshMarkup
//!
struct PolyfaceEdgeChain
{
private:
    CurveTopologyId                 m_id;
    bvector<int32_t>                m_vertexIndices;

    void Build(bvector<PolyfaceEdge>&& edges);
public:
//! Construct an empty chain with default CurveTopologyId.
GEOMDLLIMPEXP                   PolyfaceEdgeChain ();
//! Construct an empty chain with CurveTopologyId.
GEOMDLLIMPEXP                   PolyfaceEdgeChain (CurveTopologyIdCR id);
//! construct a chain with two initial indices.
GEOMDLLIMPEXP                   PolyfaceEdgeChain (CurveTopologyIdCR id, int32_t index0, int32_t index1);
//! constra a chain with indices;
GEOMDLLIMPEXP                   PolyfaceEdgeChain (CurveTopologyIdCR id, bvector<int32_t>&& indices);
//! construct a chain with potentially connected set of edges.;
GEOMDLLIMPEXP                   PolyfaceEdgeChain (CurveTopologyIdCR id, bvector<PolyfaceEdge>&& edges);
//! construct a chain containing all polyface edges (connected at random).
GEOMDLLIMPEXP                   PolyfaceEdgeChain (CurveTopologyIdCR id, PolyfaceQueryCR polyface);
//! add an index.
GEOMDLLIMPEXP void              AddIndex (int32_t index);
//! add an index.
GEOMDLLIMPEXP void              AddZeroBasedIndex (uint32_t index);
//! add indices
GEOMDLLIMPEXP void              AddZeroBasedIndices (bvector<size_t> const &indices);
//! query the CurveTopologyId
GEOMDLLIMPEXP CurveTopologyIdCR GetId () const;
//! Get pointer to the contiguous indices.
GEOMDLLIMPEXP int32_t const*    GetIndexCP() const;
//! Query the number of indices.
GEOMDLLIMPEXP size_t            GetIndexCount () const;
//! extract coordinates from points.
//! @return false if any indices out of bounds.
GEOMDLLIMPEXP bool GetXYZ (bvector<DPoint3d> &dest, bvector<DPoint3d> const &source) const;

void ReserveIndices(size_t count) { m_vertexIndices.reserve(count); }
};

typedef PolyfaceEdgeChain const*    PolyfaceEdgeChainCP;

//! integration data for directional volume calculation (and assessing validity from cancelation of moments)
struct DirectionalVolumeData
{
double volume;
DMatrix4d areaProducts;
};


//! base interface with virtuals for announcements to cutFill callers.
//!Typical implementation of this would be
//! <ul>
//! <li>For fast volume calculation
//!    <ul>
//!    <li>compute xyz centroid and xy area of each facet. (The two loops will have identical xy area)
//!    <li>volume is the product of the xy area and the z difference at the centroids.
//!    </ul>
//! <li>For saving meshes
//!    <ul>
//!    <li>Create polyface construction objects in the constructor for the handler.
//!    <li>Distribute facets as desired.
//!    <li>Be sure to observe facet orientation -- "bottom facing" facets will need to be reversed from the top-facing inputs
//!    </ul>
//! </ul>

struct FacetCutFillHandler
{
//! Test if the handler object is still interested in the calls.
//! This is not normally implemented
GEOMAPI_VIRTUAL bool ContinueSearch (){ return true;}

//! Announce coordinates in paired dtm and road loops.
//! <ul>
//! <li>both loops are oriented CCW.
//! <li>Hence when building closed meshes, the lower of the two must be reversed to get the downward directed surface properly oriented
//! <li>points are in corresponding order -- dtm[i] and road[i] have identical xy coordinates, different z
//! <li>isCut is true when the dtm is above the road
//! <li>isCut is false for road above dtm.
//! <li>isCut (or comparison of corresponding z coordinates) is consistent around the entire facet.
//! </ul>
GEOMAPI_VIRTUAL void ProcessCutFillFacet (
bvector<DPoint3d> &dtm,             //!< [in] points around the dtm loop
size_t dtmReadIndex,                //!< [in] read index for the dtm facet this came from.
bvector<DPoint3d> &road,            //!< [in] points around the road loop.
size_t roadReadIndex,               //!< [in] read index for the road facet this came freom.
bvector<BoolTypeForVector> &roadBoundaryFlag,    //!< [in] true if the succeeding edge is a boundary of the cutFill component.
bool isCut                          //!< [in] true if this is a "cut" -- road below dtm.  false if this is "fill" -- dtm below road
) {}

// Announce a vertical panel or a specific target mesh.
// <ul>
// <li>The loop is oriented CCW from outside in its destination
// <li>Default action is to do nothing.
// </ul>
//
GEOMAPI_VIRTUAL void ProcessSideFacet (bvector<DPoint3d> &points, bool isCut) {}

//! Called to reset the computation.
//! This is important:  The FastCutFill caller may decide to throw away partially completed results
//!    and restart.
GEOMAPI_VIRTUAL void Reset (){}

//! compute the volume between the two loops (in z direction)
//! This may be applied to the loops received by ProcessCutFillFacet
//! volume is {area * (centroidB.z - centroidA.z)}
//! loopA, loopB area assumed to have identical xy coordinates but different z.
static GEOMDLLIMPEXP ValidatedDouble ZVolumeBetweenFacets (bvector<DPoint3d> const &loopA, bvector<DPoint3d> const &loopB);
};




//=======================================================================================
//! "Read only" facet interface.
//!    These queries require "flat array" data layout, but do not specify how the
//!      flat memory is managed.
//! @ingroup BentleyGeom_Mesh
//!
struct PolyfaceQuery
{
GEOMAPI_VIRTUAL size_t                       _GetPointCount () const = 0;
GEOMAPI_VIRTUAL size_t                       _GetNormalCount () const = 0;
GEOMAPI_VIRTUAL size_t                       _GetParamCount () const = 0;
GEOMAPI_VIRTUAL size_t                       _GetColorCount () const = 0;
GEOMAPI_VIRTUAL size_t                       _GetFaceCount () const = 0;
GEOMAPI_VIRTUAL size_t                       _GetPointIndexCount () const = 0;
GEOMAPI_VIRTUAL size_t                       _GetFaceIndexCount () const = 0;
GEOMAPI_VIRTUAL size_t                       _GetEdgeChainCount () const = 0;
GEOMAPI_VIRTUAL DPoint3dCP                   _GetPointCP () const = 0;
GEOMAPI_VIRTUAL DVec3dCP                     _GetNormalCP () const = 0;
GEOMAPI_VIRTUAL DPoint2dCP                   _GetParamCP () const = 0;
GEOMAPI_VIRTUAL uint32_t const *             _GetIntColorCP () const = 0;
GEOMAPI_VIRTUAL FacetFaceDataCP              _GetFaceDataCP () const = 0;
GEOMAPI_VIRTUAL int32_t const*               _GetPointIndexCP () const = 0;
GEOMAPI_VIRTUAL int32_t const*               _GetColorIndexCP () const = 0;
GEOMAPI_VIRTUAL int32_t const*               _GetParamIndexCP () const = 0;
GEOMAPI_VIRTUAL int32_t const*               _GetNormalIndexCP () const = 0;
GEOMAPI_VIRTUAL int32_t const*               _GetFaceIndexCP () const = 0;
GEOMAPI_VIRTUAL bool                         _GetTwoSided () const = 0;
GEOMAPI_VIRTUAL uint32_t                     _GetNumPerFace () const = 0;
GEOMAPI_VIRTUAL uint32_t                     _GetNumPerRow () const = 0;
GEOMAPI_VIRTUAL uint32_t                     _GetMeshStyle () const = 0;
GEOMAPI_VIRTUAL PolyfaceEdgeChainCP          _GetEdgeChainCP () const = 0;
GEOMAPI_VIRTUAL PolyfaceAuxDataCPtr          _GetAuxDataCP() const;
GEOMAPI_VIRTUAL PolyfaceVectors *            _AsPolyfaceVectorsP() const;

                                            PolyfaceQuery () { }

public:

//! Test if this mesh is vaiable sized indexed.
GEOMDLLIMPEXP bool IsVariableSizeIndexed () const;

//! Return the number of points.
GEOMDLLIMPEXP size_t                        GetPointCount () const;
//! Return the number of normals.
GEOMDLLIMPEXP size_t                        GetNormalCount () const;
//! Return the number of parameters.
GEOMDLLIMPEXP size_t                        GetParamCount () const;
//! Return the number of colors.
GEOMDLLIMPEXP size_t                        GetColorCount () const;
//! Return the number of faces.  Note that this is not a "facet" count -- many facets can reference the same
//! containing face in the parent geometry.
GEOMDLLIMPEXP size_t                        GetFaceCount () const;
//! Return the number of point indices.
GEOMDLLIMPEXP size_t                        GetPointIndexCount () const;
//! Return the number of face data indices
GEOMDLLIMPEXP size_t                        GetFaceIndexCount () const;
//! Return the number of edge chains.
GEOMDLLIMPEXP size_t                        GetEdgeChainCount () const;
//! Return a pointer to contiguous point memory.
GEOMDLLIMPEXP DPoint3dCP                    GetPointCP () const;
//! Return a pointer to contiguous normal memory.
GEOMDLLIMPEXP DVec3dCP                      GetNormalCP () const;
//! Return a pointer to contiguous parameter memory.
GEOMDLLIMPEXP DPoint2dCP                    GetParamCP () const;
//! Return a pointer to contiguous int color structs.
GEOMDLLIMPEXP uint32_t const*               GetIntColorCP () const;
//! Return a pointer to contiguous face FacetFaceData structs.
GEOMDLLIMPEXP FacetFaceDataCP               GetFaceDataCP () const;
//! Return a pointer to contiguous edge chain structs
GEOMDLLIMPEXP PolyfaceEdgeChainCP           GetEdgeChainCP () const;
//! Return a pointer to Aux data
GEOMDLLIMPEXP PolyfaceAuxDataCPtr           GetAuxDataCP() const;


//! For Color, Param, and normal indices, resolveToDefaults allows caller to request using
//! PointIndex (or other default decision) if respective index is same as PointIndex.

//! Return pointer to contiguous point indices.
GEOMDLLIMPEXP int32_t const*                  GetPointIndexCP () const;
//! Return pointer to contiguous color indices.  Optionally use default indices (points)
GEOMDLLIMPEXP int32_t const*                  GetColorIndexCP  (bool resolveToDefaults = false) const;
//! Return pointer to contiguous param indices.  Optionally use default indices (points)
GEOMDLLIMPEXP int32_t const*                  GetParamIndexCP  (bool resolveToDefaults = false) const;
//! Return pointer to contiguous normal indices.  Optionally use default indices (points)
GEOMDLLIMPEXP int32_t const*                  GetNormalIndexCP (bool resolveToDefaults = false) const;
//! Return pointer to contiguous face indices.  Optionally use default indices (points)
GEOMDLLIMPEXP int32_t const*                  GetFaceIndexCP   (bool resolveToDefaults = false) const;

//! Query if facets are considered two sided. (If not, outward normal can be used to cull backfaces)
GEOMDLLIMPEXP bool                          GetTwoSided () const;
//! Query the nominal number of facets per face.  If this is 0 or 1, facets are variable size and separated by 0 as terminator.
//! If larger the indices are blocked (with 0 as pad if needed)
GEOMDLLIMPEXP uint32_t                      GetNumPerFace () const;
//! Query the row size for gridded mesh (quad or triangular)
GEOMDLLIMPEXP uint32_t                      GetNumPerRow () const;
//! Query the mesh style (MESH_ELM_STYLE_INDEXED_FACE_LOOPS etc)
GEOMDLLIMPEXP uint32_t                      GetMeshStyle () const;
//! Try to access point coordinates through a readIndex in the pointIndex array.
GEOMDLLIMPEXP bool                          TryGetPointAtReadIndex (size_t readIndex, DPoint3dR data) const;
//! Try to access normal coordinates through a readIndex in the normalIndex array.
GEOMDLLIMPEXP bool                          TryGetNormalAtReadIndex (size_t readIndex, DVec3dR data) const;
//! Try to access param coordinates through a readIndex in the paramIndex array.
GEOMDLLIMPEXP bool                          TryGetParamAtReadIndex (size_t readIndex, DPoint2dR data) const;
//! Try to access color coordinates through a readIndex in the colorIndex array.
GEOMDLLIMPEXP bool                          TryGetFacetFaceDataAtReadIndex (size_t readIndex, FacetFaceDataR data, size_t& zeroBasedIndex) const;

//! Return a PolyfaceHeader with the same contents.
GEOMDLLIMPEXP PolyfaceHeaderPtr Clone () const;

//! Return a PolyfaceHeader, with variable length faces.
GEOMDLLIMPEXP PolyfaceHeaderPtr CloneAsVariableSizeIndexed (PolyfaceQueryCR source) const;

//!
//! Collect information about faces in the mesh.
//! @param [out] numLoop number of faces.
//! @param [out] minPerLoop smallest point count on any loop.
//! @param [out] maxPerLoop largest point count on any loop.
//! @param [out] hasNonPlanarFaces true if any non-planar face is found
//! @param [out] hasNonConvexFaces true if any non-convex face is found
//!
GEOMDLLIMPEXP void InspectFaces
(
size_t &numLoop,
size_t &minPerLoop,
size_t &maxPerLoop,
bool   &hasNonPlanarFaces,
bool   &hasNonConvexFaces
) const;

//! Test if the mesh passes IsClosedByEdgePairing, and compute volume if so.
GEOMDLLIMPEXP ValidatedDouble ValidatedVolume () const;


//! Return the sum of tetrahedral volumes from the specified origin.
//!        If the mesh is closed and all facets are simply connected planar, this is the signed enclosed volume.
//!        (The facets and volume are not require to be convex.)
//!
GEOMDLLIMPEXP double SumTetrahedralVolumes (DPoint3dCR origin) const;

//! Return the sum of areas as viewed with given direction toward the eye.
//! @param [in] vectorToEye vector towards eye.
//! @param [out] numPositive number of facets that face the eye.
//! @param [out] numPerpendicular number of facets that are perpendicular to the eye.
//! @param [out] numNegative number of facets that face away from the eye.
//! @returns sum of (signed !!!) projected areas -- should be zero for closed mesh.
GEOMDLLIMPEXP double SumDirectedAreas (DVec3dCR vectorToEye, size_t &numPositive, size_t &numPerpendicular, size_t &numNegative) const;


//! Return the sum of areas as viewed with given direction toward the eye.
//! @param [in] vectorToEye vector towards eye.
//! @param [out] numPositive number of facets that face the eye.
//! @param [out] numPerpendicular number of facets that are perpendicular to the eye.
//! @param [out] numNegative number of facets that face away from the eye.
//! @param [out] forwardProjectedSum sum of forward facing areas (positive)
//! @param [out] reverseProjectedSum sum of rear facing areas (positive)
//! @param [out] forwardAbsoluteSum sum of absolute area of forward facing facets.
//! @param [out] reverseAbsoluteSum sum of absolute area of rear facing facets.
//! @param [out] perpendicularAbsoluteSum sum of side facing areas.
//! @returns difference of forward and reverse projected areas.  Should be zero for closed mesh.
GEOMDLLIMPEXP double SumDirectedAreas
(
DVec3dCR vectorToEye,
size_t &numPositive,
size_t &numPerpendicular,
size_t &numNegative,
double &forwardProjectedSum,
double &reverseProjectedSum,
double &forwardAbsoluteSum,
double &reverseAbsoluteSum,
double &perpendicularAbsoluteSum
) const;

//! Return the sum of areas as viewed with given direction toward the eye.
//! @param [in] vectorToEye vector towards eye.
//! @param [out] numPositive number of facets that face the eye.
//! @param [out] numPerpendicular number of facets that are perpendicular to the eye.
//! @param [out] numNegative number of facets that face away from the eye.
//! @param [out] forwardProjectedSum sum of forward facing areas (positive)
//! @param [out] reverseProjectedSum sum of rear facing areas (positive)
//! @param [out] forwardAbsoluteSum sum of absolute area of forward facing facets.
//! @param [out] reverseAbsoluteSum sum of absolute area of rear facing facets.
//! @param [out] perpendicularAbsoluteSum sum of side facing areas.
//! @param [out] volume  Use the first point as origin.   Form volumes swept to plane through this point and perpendicular to vectorToEye
//! @returns difference of forward and reverse projected areas.  Should be zero for closed mesh.
GEOMDLLIMPEXP double SumDirectedAreas
(
DVec3dCR vectorToEye,
size_t &numPositive,
size_t &numPerpendicular,
size_t &numNegative,
double &forwardProjectedSum,
double &reverseProjectedSum,
double &forwardAbsoluteSum,
double &reverseAbsoluteSum,
double &perpendicularAbsoluteSum,
double &volume
) const;


//! Return the sum of tetrahedral moments from the specified origin.
//!        If the mesh is closed and all facets are simply connected planar, this is the moment around the origin.
//!        (The facets and volume are not require to be convex.)
//! @param [in] origin origin for tetrahedra.
//! @param [out] moments sum of (x,y,z) dV
//! @return summed volume
GEOMDLLIMPEXP double SumTetrahedralFirstMoments (DPoint3dCR origin, DVec3dR moments) const;


//! Return the sum (integrated) products within tetrahedra from local origin to all facets.
//! @param [in] worldToLocal transform to apply to move polyface vertices to local system of product accumulation;
//! @param [out] volume summed volume.
//! @param [out] moment1 first moments
//! @param [out] products product integrals [xx xy xz; yx yy yz; zx xy zz].    Note that this is the products; inertial tensor can be computed from the diagonals.
//!
GEOMDLLIMPEXP void SumTetrahedralMomentProducts (TransformCR worldToLocal, double &volume, DVec3dR moment1, RotMatrixR products) const;

//! Return the volume, centroid, orientation, and principal moments
//! This is only valid if the mesh is closed and consistently oriented.  This method only checks for superficial verification of the closure conditions.
//! @return false if (a) no points in the mesh or (b) apparent volume swept from 000 and from centroid are different.
GEOMDLLIMPEXP bool ComputePrincipalMoments
(
double &volume,         //!< [out] mesh volume.
DPoint3dR centroid,     //!< [out] centroid of the mesh.
RotMatrixR axes,        //!< [out] columns of this matrix are the principal directions.
DVec3dR momentxyz,       //!< [out] moments (yy+zz,xx+zz,xx+yy) around the principal directions.
bool forcePositiveVolume = false    //!< [in] if true, the volume and moments are reversed if volume is negative.
) const;

//! Return the sum of facet areas. There is no check for planarity.
GEOMDLLIMPEXP double SumFacetAreas () const;

//! Return the sum of facet areas.  Return moment with respect to origin.
GEOMDLLIMPEXP double SumFacetFirstAreaMoments (DPoint3dCR origin, DVec3dR moment1) const;

//! Return the sum of facet areas.  Return moment products with respect to origin.
GEOMDLLIMPEXP double SumFacetSecondAreaMomentProducts (DPoint3d origin, DMatrix4dR products) const;

//! Return the volume, centroid, orientation, and principal moments of the facets understood as a thin surface (NOT a volume!!!!)
//! @param [out] area mesh area.
//! @param [out] centroid area centroid of the mesh.
//! @param [out] axes columns of this matrix are the principal directions.
//! @param [out] momentxyz moments (yy+zz,xx+zz,xx+yy) around the principal directions.
//! @return false if (a) no points in the mesh or (b) apparent volume swept from 000 and from centroid are different.
GEOMDLLIMPEXP bool ComputePrincipalAreaMoments (double &area, DPoint3dR centroid, RotMatrixR axes, DVec3dR momentxyz) const;

//! Return range of the points.
GEOMDLLIMPEXP DRange3d PointRange () const;

//! Return a tolerance appropriate for high accuracy calculations (12 or more digits relative)
GEOMDLLIMPEXP double GetTightTolerance () const;

//! Return a tolerance appropriate for medium accuracy calculations (8 digits relative)
GEOMDLLIMPEXP double GetMediumTolerance () const;

//! Return range of the parameters.
GEOMDLLIMPEXP DRange2d ParamRange () const;

//!
//! Reverse a single face loop in parallel index arrays.
//! @remarks A face loop is reversed after the 1st index: the 2nd/last indices are swapped, the 3rd/penultimate indices are swapped, etc.
//! @param [in] iFirst       0-based offset to the first index in the face loop
//! @param [in] iLast       0-based offset to the last index of the face loop.
//! @param [in] normalArrayIndexAction  selects action in normal array. This can be
//! <ul>
//! <li>IndexAction::None -- leave the index value unchanged
//! <li>IndexAction::ForcePositive -- change to positive
//! <li>IndexAction::ForceNegative -- change to negative
//! <li>IndexAction::Negate -- change to negative of its current sign
//! </ul>
//!
//!
GEOMDLLIMPEXP void ReverseIndicesOneFace
(
size_t        iFirst,
size_t        iLast,
BlockedVectorInt::IndexAction           normalArrayIndexAction = BlockedVectorInt::None
);

//!
//! Reverse index orientations in place, optionally restricting by sign of indices in NormalIndex()
//! @remarks The only coordinate data array that is changed by this function is the normal matrix, and then
//!       only if it is present and negateNormals is true.
//! @param [in] flipMarked  true to flip when marked by negative indices in NormalIndex()
//! @param [in] flipUnMarked  true to flip when not marked by negative indices in NormalIndex()
//! @param [in] normalIndexAction  selects final form of indices in NormalIndex()
//! @param [in] negateNormals      true to negate normal vectors.
//!
//! return SUCCESS if face loops are successfully reversed
//!
GEOMDLLIMPEXP bool ReverseIndicesAllFaces
(
bool           negateNormals = true,
bool           flipMarked = true,
bool           flipUnMarked = true,
BlockedVectorInt::IndexAction normalIndexAction = BlockedVectorInt::None
);





//!
//! Inspect edge-to-face incidence.
//! @param [out] numPolygonEdge Total number of sides on all polygons. (i.e. each interior edge is counted twice in the usual case of an edge shared by two polygons.)
//! @param [out] numMatedPair number of edges with exactly two incident faces, with the faces in the properly opposing orientation.
//! @param [out] num1 Number of edges with 1 incident face.
//! @param [out] num2 Number of edges with 2 incident faces.
//! @param [out] num3 Number of edges with 3 incident faces.
//! @param [out] num4 Number of edges with 4 incident faces.
//! @param [out] numMore Number of edges with 5 or more incident faces.
//! @param [out] numCollapsed Number of polygon sides collapsed to points.
//! @param [out] ignoreSliverFaces supresses counting edges on sliver faces
//! @param [out] numWith0Visible number of edge clusters with none visible
//! @param [out] numwith1OrMoreVisible number of edge clusters with 1 or more visible.
//!
GEOMDLLIMPEXP void CountSharedEdges (size_t &numPolygonEdge, size_t &numMatedPair,
    size_t &num1, size_t &num2, size_t &num3, size_t &num4, size_t &numMore, size_t &numCollapsed,
    bool ignoreSliverFaces,
    size_t &numWith0Visible,
    size_t &numwith1OrMoreVisible) const;

GEOMDLLIMPEXP void CountSharedEdges (size_t &numPolygonEdge, size_t &numMatedPair,
    size_t &num1, size_t &num2, size_t &num3, size_t &num4, size_t &numMore, size_t &numCollapsed,
    bool ignoreSliverFaces) const;


GEOMDLLIMPEXP void CountSharedEdges (size_t &numPolygonEdge, size_t &numMatedPair,
    size_t &num1, size_t &num2, size_t &num3, size_t &num4, size_t &numMore, size_t &numCollapsed) const;
//! @param [out] numVertex number of vertices
//! @param [out] numFacet number of facets
//! @param [out] numQuad number of 4-sided facets.
//! @param [out] numTriangle number of 3-sided facets
//! @param [out] numImplicitTriangle number of triangles if the mesh is triangulated.
//! @param [out] numVisibleEdges number of visible edges.
//! @param [out] numInvisibleEdges number of hidden edges.
GEOMDLLIMPEXP void CollectCounts
(
size_t &numVertex,
size_t &numFacet,
size_t &numQuad,
size_t &numTriangle,
size_t &numImplicitTriangle,
size_t &numVisibleEdges,
size_t &numInvisibleEdges
) const;


//! @param [out] minPerFace smallest count.
//! @param [out] maxPerFace largest count.
GEOMDLLIMPEXP void CollectPerFaceCounts (size_t &minPerFace, size_t &maxPerFace) const;

//! Collect individual segments for each distinct edge.
//! @param [out] segments array to receive segments.
//! @param [in] omitInvisibles true to hide segments that are not visible (due to negated indices)
GEOMDLLIMPEXP void CollectSegments (bvector<DSegment3d> &segments, bool omitInvisibles) const;

//! Cut with a plane. (Prototype)
//! Return as a curve vector. Optionally structure as area-bounding loops.
//! @param [in] sectionPlane plane to cut the mesh.
//! @param [in] formRegions true to look for closed loops and structure the return as a loop or parity CurveVector.
//! @param [in] markEdgeFractions true to attache FacetEdgeLocationDetailVector to the linestrings.
GEOMDLLIMPEXP CurveVectorPtr PlaneSlice (DPlane3dCR sectionPlane, bool formRegions, bool markEdgeFractions = false) const;

//! Project linestring in given direction to intersection with facets.
//! Return as a curve vector.
//! @param [in] spacePoints points to project onto the polyface
//! @param [in] direction direction to project.
GEOMDLLIMPEXP CurveVectorPtr DrapeLinestring (bvector<DPoint3d> &spacePoints, DVec3dCR direction) const;

//! Project linestring in given direction to intersection with facets.
//! Return as array of detailed intersection data.
//! @param [in] spacePoints
//! @param [in] direction
//! @param [out] drapeSegments
GEOMDLLIMPEXP void DrapeLinestring (bvector<DPoint3d> const &spacePoints, DVec3dCR direction, bvector<DrapeSegment> &drapeSegments) const;

//! Pick facets with points dropped in xy direction.  In returned array:
//!<ul>
//!<li>Each point is a hit point on a facet.
//!<li>GetTagA() is the facet read index.
//!<li>GetTagB() is the spacePointIndex.
//!</ul>
//! The points in the returned array, the points are sorted by GetTagB () (i.e. line index), then z coordinate of the facet point.
GEOMDLLIMPEXP void DrapePointsXY (bvector<DPoint3d> const &spacePoints, bvector<DPoint3dSizeSize> &drapePoints) const;

//! Test if vertex indices around faces indicate watertight closure.
GEOMDLLIMPEXP bool IsClosedByEdgePairing () const;

//! Test if any facets are defined (Specifically, true if the point index set is nonempty).
GEOMDLLIMPEXP bool HasFacets () const;

//! Test if all facets are 3 sided
GEOMDLLIMPEXP bool IsTriangulated () const;

//! Test if all facets are all planar with consistent normals (as in a facetted closed region).
GEOMDLLIMPEXP bool IsClosedPlanarRegion(DPlane3dR plane, double planeTolerance = 1.0E-6, double distanceTolerance = 1.0E-10) const;

//! Count facets
GEOMDLLIMPEXP size_t GetNumFacet () const;

//! Count facets
GEOMDLLIMPEXP size_t GetNumFacet (size_t &maxVertexPerFacet) const;

//! Check convexity
GEOMDLLIMPEXP bool HasConvexFacets () const;

//! Query largest absolute coordinate
GEOMDLLIMPEXP double LargestCoordinate () const;

//! Try to convert the index and edge fraction of a FacetEdgeLocationDetail to a point.
GEOMDLLIMPEXP bool TryEvaluateEdge (FacetEdgeLocationDetailCR position, DPoint3dR xyz) const;

//--------------------------------------------------------------------------------
//! @description Compute intersections (line strings) of this mesh with a swept linestring
//! @param [out] xyzOut array of points on the intersection linestrings.  DISCONNECTS separate multiple linestrings.
//! @param [out] linestringIndexOut for each xyzOut[i], the index of the input segment that it came from.
//! @param [out] meshIndexOut for each xyzOut[i], the mesh read index it came from.
//! @param [in] linestringPoints points to sweep.
//! @param [in] sweepDirection sweep direction
GEOMDLLIMPEXP void  SweepLinestringToMesh
(
bvector<DPoint3d> &xyzOut,
bvector<int> &linestringIndexOut,
bvector<int> &meshIndexOut,
bvector <DPoint3d> const &linestringPoints,
        DVec3dCR                sweepDirection
);

//!  Output processor for ClipToPlaneSetIntersection.
struct IClipToPlaneSetOutput
{
GEOMAPI_VIRTUAL StatusInt   _ProcessUnclippedPolyface (PolyfaceQueryCR polyfaceQuery) = 0;
GEOMAPI_VIRTUAL StatusInt   _ProcessClippedPolyface (PolyfaceHeaderR polyfaceHeader) = 0;
};

 //! @description Clip polyface to intersection of an array of plane sets.
GEOMDLLIMPEXP StatusInt   ClipToPlaneSetIntersection (T_ClipPlaneSets const& planeSets, IClipToPlaneSetOutput& output, bool triangulateOutput) const;


//!  @description Fas clustered vertex decimator - used during tile generation.
GEOMDLLIMPEXP PolyfaceHeaderPtr ClusteredVertexDecimate (double tolerance, double minCompressionRatio = .5);


 //! @description Clip polyface to range.
GEOMDLLIMPEXP StatusInt   ClipToRange (DRange3dCR clipRange, PolyfaceQuery::IClipToPlaneSetOutput& output, bool triangulateOutput) const;

//! @description add all facets to a TaggedPolygonVectorR.
//! indexB is set to the read index in the PolyfaceQuery
//! @param [out] polygons growing polygon collection.
//! @param [in] indexA value for indexA of all polygons.
//! @param [in] numWrap number of wraparound points to have on each polygon.
//! @param [in] selectRange optional range to restrict facets.
GEOMDLLIMPEXP void AddToTaggedPolygons
(
TaggedPolygonVectorR polygons,
ptrdiff_t indexA,
size_t numWrap,
DRange3dCP selectRange = NULL
) const;

//! @description add all facets to a TaggedPolygonVectorR.
//! indexB is set to the read index in the PolyfaceQuery
//! @param [out] polygons growing polygon collection.
//! @param [in] indexA value for indexA of all polygons.
//! @param [in] numWrap number of wraparound points to have on each polygon.
//! @param [in] filter object
GEOMDLLIMPEXP void AddToTaggedPolygons
(
TaggedPolygonVectorR polygons,
ptrdiff_t indexA,
size_t numWrap,
IPolyfaceVisitorFilter *filter
) const;

//! @description  (DEPRECATED) Compute volumes "between" primary and barrier facets
//! @param [in] polyfaceA first facet set (e.g. road surface)
//! @param [in] polyfaceB second facet set (e.g. ground dtm)
//! @param [out] resultAaboveB volume where polyfaceA is above polyfaceB
//! @param [out] resultBaboveA volume where polyfaceB is above polyfaceA
static GEOMDLLIMPEXP void ComputeCutAndFill
(
PolyfaceHeaderCR polyfaceA,
PolyfaceHeaderCR polyfaceB,
bvector<PolyfaceHeaderPtr> &resultAaboveB,
bvector<PolyfaceHeaderPtr> &resultBaboveA
);
//! @description Compute volumes where polyfaceB undercuts polyfaceA
static GEOMDLLIMPEXP void ComputeUndercut
(
PolyfaceHeaderCR polyfaceA,             //!< [in] upper surface of lower geometry(for instance, upward facing facets of road, all shifted upwards by clearance)
IPolyfaceVisitorFilter *filterA,        //!< [in] optional filter object
PolyfaceHeaderCR polyfaceB,             //!< [in] lower surface of upper geometry (for instance, downward facing facets of bridge)
IPolyfaceVisitorFilter *filterB,        //!< [in] optional filter object
PolyfaceHeaderPtr &undercutPolyface     //!< [in] undercut results
);
static GEOMDLLIMPEXP void ComputeOverAndUnderXY
(
PolyfaceHeaderCR polyfaceA,             //!< [in] upper surface of lower geometry(for instance, upward facing facets of road, all shifted upwards by clearance)
IPolyfaceVisitorFilter *filterA,        //!< [in] optional filter object
PolyfaceHeaderCR polyfaceB,             //!< [in] lower surface of upper geometry (for instance, downward facing facets of bridge)
IPolyfaceVisitorFilter *filterB,        //!< [in] optional filter object
PolyfaceHeaderPtr &polyfaceAOverB,      //!< [out] parts of polyfaceA that are over polyfaceB
PolyfaceHeaderPtr &polyfaceBUnderA,      //!< [out] parts of polyfaceB that are over polyfaceA
bool computeAndApplyTransform = true     //!< [in] if true, compute a transform to move data to the origin.
);
template <typename T>
struct AnnotatedMesh
{
PolyfaceHeaderPtr mesh;
T data;
};


//! @description Compute volumes "between" primary and barrier facets
//! @param [in] polyfaceA first facet set (e.g. road surface)
//! @param [in] polyfaceB second facet set (e.g. ground dtm)
//! @param [out] resultAaboveB volume where polyfaceA is above polyfaceB
//! @param [out] resultBaboveA volume where polyfaceB is above polyfaceA
static GEOMDLLIMPEXP void ComputeCutAndFill
(
bvector<PolyfaceHeaderPtr> polyfaceA,
bvector<PolyfaceHeaderPtr> polyfaceB,
bvector<PolyfaceHeaderPtr> &resultAaboveB,
bvector<PolyfaceHeaderPtr> &resultBaboveA
);



//! @description "Punch" through target polygons.
//! @param [in] punch punch polygons
//! @param [in] target target polygons
//! @param [in] keepInside true to return the target mesh parts that are inside the punch, false to return outside parts.
//! @param [out] result punched mesh
static GEOMDLLIMPEXP void ComputePunch
(
PolyfaceQueryCR punch,
PolyfaceQueryCR target,
bool keepInside,
bvector<PolyfaceHeaderPtr> &result
);

// sweep a punching mesh in the xy direction to clip a target mesh.
static GEOMDLLIMPEXP void ComputePunchXYByPlaneSets
(
PolyfaceQueryCR punch,  //!< [in] each facet of this is used as a "punch"
PolyfaceQueryCR target, //!< [in] facets to be split by the punch.
PolyfaceHeaderPtr *inside,  //!< [out] (target intersect punch)
PolyfaceHeaderPtr *outside,  //!< [out] (target outsideOf punch)
PolyfaceHeaderPtr *debugMesh = nullptr,  //!< [inout] debug output
bool computeAndApplyTransform = true     //!< [in] if true, compute a transform to move data to the origin.
);


//! Attempt to heal vertical gaps in a mesh.
//! @param [in] polyface original polyface
//! @param [in] tryVerticalPanels true to seek pure vertical panels
//! @param [in] trySpaceTriangulation true to seek triangulation of any missing faces, as viewed from any direction found useful.
//! @param [out] healedPolyface modified polyface.  This is NOT constructed if no panels can be added.
//! @param [in] simplifySlivers true to look for sliver faces
//! @returns number of facets added
static GEOMDLLIMPEXP size_t HealVerticalPanels
(
PolyfaceQueryCR polyface,
bool tryVerticalPanels,
bool trySpaceTriangulation,
PolyfaceHeaderPtr &healedPolyface,
bool simplifySlivers = false
);

//!<ul>
//!<li> Create a plane for each facet.
//!<li> Assemble the planes as a single clip plane set.
//!<li> If the facets are closed by edge pairing, use the volume to control plane orientation.
//!<li> If the facets are not closed, the facet orientation determines plane orientation.
//!<li> The implication of this is that if the facets are a convex volume the clip plane set is convex.
//!</ul>
//! @return the computed volume (if closed), 0 if not closed.
GEOMDLLIMPEXP double BuildConvexClipPlaneSet (ConvexClipPlaneSetR planes);

//! @description Compute (many) integrals of volume properties, using directional
//!    formulas that will give correct results (and confidence indicators) when "some" facets are missing
//! @param [in] polyface facets for integration
//! @param [out] pData array (allocated by caller) of various integrals:
//! <ul>
//!    <li>pData[0], pData[1], pData[2] = view along respective axes.   Use signed area, so result should be zero
//!              if all facets are present to cancel.
//!    <li>pData[0], pData[1], pData[2] = view along respective axes.   Use absolute area, so result should be useful for
//!             setting tolerances.
//!    <li>pData[6] = full 3d area.
//! </ul>
//! @param [out] directionalProducts array of products integrals wrt origin.  Allocated by caller.
//! @param [out] origin origin used for directonal integrals.    (Directional integrals are "from the principal" planes through this origin.)
GEOMDLLIMPEXP void DirectionalVolumeIntegrals
(
PolyfaceQueryCR       polyface,
DirectionalVolumeData pData[7],
DMatrix4d             directionalProducts[3],
DPoint3dR             origin
) const;

//! Return the volume, centroid, orientation, and principal moments, using an algorithm that tolerates missing
//!  "side"facets.
GEOMDLLIMPEXP bool ComputePrincipalMomentsAllowMissingSideFacets
(
double &volume,         //!< [out] mesh volume.
DPoint3dR centroid,     //!< [out] centroid of the mesh.
RotMatrixR axes,        //!< [out] columns of this matrix are the principal directions.
DVec3dR momentxyz,       //!< [out] moments (yy+zz,xx+zz,xx+yy) around the principal directions.
bool forcePositiveVolume , //!< [in] if true, the volume and moments are reversed if volume is negative.
double relativeTolerance = 1.0e-8  //!< [in] relative tolerance for assessing the checksums.  (Suggested value: 1e-8 or tighter)
) const;

//! Compute areas, centroids, volumes of projections onto principal planes.
GEOMDLLIMPEXP void DirectionalAreaAndVolume
(
DPoint3dCR origin,  //!<  [in] common point of principal planes.   When adding volumes from multiple meshes, use the same origing for all meshes.
DVec3dR areaXYZ,    //!<  [out] signed area of projection in the respective directions
DVec3dR volumeXYZ, //!<  [out] signed volume of the projections
DVec3dR centroidX, //!<  [out] vector from origin to x projection centroid.
DVec3dR centroidY, //!<  [out] vector from origin to y projection centroid.
DVec3dR centroidZ //! <  [out] vector from origin to z projection centroid.
) const;


//! Search for facets that are touched by a stroke.
//! returns true if valid point data and one or more facets selected.
GEOMDLLIMPEXP bool PickFacetsByStroke
(
DPoint4dCR eyePoint,    //!< [in] eyepoint for stroke (e.g. weight 1 for camera point, weight 0 for flat view vector.
DPoint3dCR point0,      //!< [in] start point of stroke
DPoint3dCR point1,      //!< [in] end point of stroke
bvector<FacetLocationDetail> &pickDetail, //!< [out] pairs of (readIndex, u,v, xyz) for the crossings.  u is along the stroke edge.  v is positive towards the eye. xyz are on the facet plane.

bool exitAfterFirstPick     //!< [in] true to quit as soon as a single pick is found.
);

//! Search for facets that are touched by a box
//! returns true if valid point data and one or more facets selected.
GEOMDLLIMPEXP bool PickFacetsByRectangle
(
DPoint4dCR eyePoint,    //!< [in] eyepoint for stroke (e.g. weight 1 for camera point, weight 0 for flat view vector.
DPoint3dCR point0,      //!< [in] corner of rectangle
DPoint3dCR point1,      //!< [in] opposite corner of rectangle
bvector<FacetLocationDetail> &pickDetail, //!< [out] pairs of (readIndex, u,v, xyz) for the crossings.  u is along the stroke edge.  v is positive towards the eye. xyz are on the facet plane.

bool exitAfterFirstPick     //!< [in] true to quit as soon as a single pick is found.
);



//! Search for intersection edges, tagged with readIndex of affected facets.
static GEOMDLLIMPEXP bool SearchIntersectionSegments (
PolyfaceQueryR polyfaceA,           //!< first polyface
PolyfaceQueryR polyfaceB,           //!< second polyface
struct PolyfaceIndexedHeapRangeTree &treeA,    //!< optional range tree for polyfaceA
struct PolyfaceIndexedHeapRangeTree &treeB,     //!< optional range tree for polyfaceB
bvector<DSegment3dSizeSize> &segments           //!< intersection segments
);

//! Search for intersection edges, tagged with readIndex of affected facets.
static GEOMDLLIMPEXP bool SearchIntersectionSegments (
PolyfaceQueryR polyfaceA,               //!< first polyface
PolyfaceQueryR polyfaceB,               //!< second polyface
bvector<DSegment3dSizeSize> &segments   //!< intersection segments
);

//! Copy facets from the source to builder, splitting those facets for which there
//! are segments in the segments array.
static GEOMDLLIMPEXP bool CopyFacetsWithSegmentSplitImprint
(
IPolyfaceConstructionR builder, //!< facets are added here.
PolyfaceQueryR source,          //!< facets being split
bvector<DSegment3dSizeSize> &segments,  //!< Segment coordinates, with tag A or B as readIndex into source
bool byTagA     //!< true to use tagA as read index, false for tagB.
);



//! Return the outer volumes bounded by facets in two facet sets.
//! (If each is a simple volume, this is a union.  If they are unstructured but collectively enclose one or more volumes,
//!   this is the enclosed volume or volumes.)
static GEOMDLLIMPEXP void MergeAndCollectVolumes
(
PolyfaceQueryR meshA,
PolyfaceQueryR meshB,
bvector<PolyfaceHeaderPtr> &enclosedVolumes
);

//! Return volumes bounded by facets in multiple facet sets.
//! Each set is assumed to be well formed (no intersections among facets within a set, other than their edges)
//! (If each is a simple volume, this is a union.  If they are unstructured but collectively enclose one or more volumes,
//!   this is the enclosed volume or volumes.)
static GEOMDLLIMPEXP void MergeAndCollectVolumes
(
bvector<PolyfaceHeaderPtr> &inputMesh,
bvector<PolyfaceHeaderPtr> &enclosedVolumes
);

//! Compute total absolute volume as reference.
//! Classify each volume as negative, zero, or positive.
//! collect the pointers in the respective outputarrays
//! Remark there is no test to confirm closure.  Volume computed for a ragged mesh is unpredictable.
static GEOMDLLIMPEXP void SelectMeshesByVolumeSign
(
bvector<PolyfaceHeaderPtr> &inputVolumes,
bvector<PolyfaceHeaderPtr> *negativeVolumeMeshes,
bvector<PolyfaceHeaderPtr> *zeroVolumeMeshes,
bvector<PolyfaceHeaderPtr> *positiveVolumeMeshes
);

//! Search for closest approach between two meshes.
static GEOMDLLIMPEXP bool SearchClosestApproach (
PolyfaceQueryR polyfaceA,           //!< first polyface
PolyfaceQueryR polyfaceB,           //!< second polyface
double maxDistance,                 //!< ignore larger distances.  Send 0 to consider all pairs.
DSegment3dR segment                 //!< shortest segment.
);

//! Search for closest approach between two meshes.
static GEOMDLLIMPEXP bool SearchClosestApproach (
PolyfaceQueryR polyfaceA,           //!< first polyface
PolyfaceQueryR polyfaceB,           //!< second polyface
double maxDistance,                 //!< ignore larger distances.  Send 0 to consider all pairs.
DSegment3dR segment,                 //!< shortest segment.
struct PolyfaceIndexedHeapRangeTree *treeA,    //!< optional range tree for polyfaceA
struct PolyfaceIndexedHeapRangeTree *treeB     //!< optional range tree for polyfaceB
);

//! Search for smallest distance between any two facets that have no shared vertices.
//! The filter normalTestRadians is used as follows to filter out facets that are close but "off to the side" instead of "above".
//!<ul>
//!<li> Compute the vector between two close points.
//!<li> Compute the angles between that vector and the normals.
//!<li> Subtract 90 degrees from each angle -- i.e. adjust the angle so it is the angle between the vector and the facet plane.
//!<li> If this deviation angle is small, don't consider this proximity.
//!</ul>
static GEOMDLLIMPEXP bool SearchClosestApproach (
PolyfaceQueryR polyfaceA,           //!< polyface
double maxDistance,                 //!< ignore larger distances.  Send 0 to consider all pairs.
DSegment3dR segment,                //!< shortest segment
double normalTestRadians            //!< filter angle. 0.0 means do not do this test.
);

//! Calls long form with normalTestRadians = 0
static GEOMDLLIMPEXP bool SearchClosestApproach (
PolyfaceQueryR polyfaceA,           //!< first polyface
double maxDistance,                 //!< ignore larger distances.  Send 0 to consider all pairs.
DSegment3dR segment                 //!< shortest segment
);


static GEOMDLLIMPEXP bool SearchClosestApproachToLinestring (
PolyfaceQueryR polyfaceA,           //!< polyface
bvector<DPoint3d> const &points,    //!< linestring
DSegment3dR segment                 //!< shortest segment
);

//! Test for AlmostEqual () conditions.
GEOMDLLIMPEXP bool IsSameStructureAndGeometry (PolyfaceQueryCR other, double tolerance) const;
//! Apply various checks for indexing structure.
//! @return true if any errors were found.
GEOMDLLIMPEXP bool HasIndexErrors
(
MeshAnnotationVector &description    //!< array to receive error descriptions.
);
//! Apply various tests for indexing structure (but with no returned descriptions.)
GEOMDLLIMPEXP bool HasIndexErrors ();

//! Return blocks of read indices for grouping components with vertex connectivity
//! @param [out] blockedReadIndexArray read indices for individual faces, separated by (-1).
//! @param [in] connectivityType 0 for vertex connectivity, 1 for edge connectivity, 2 for connectity across non-drawn edges
GEOMDLLIMPEXP bool PartitionByConnectivity (int connectivityType, bvector<ptrdiff_t> &blockedReadIndexArray) const;

//! Spread data from this mesh to many new meshes according to partition.  This method is to be used following
//!    PartitionMaintainFaceOrder or PartitionByRange
//! @param [in] blockedReadIndex Array of read indices with -1 as terminator between blocks that are to go to the same destination mesh.
//! @param [out] submeshArray This is initially cleared, then filled with as many (smartpointers to) new arrays as needed
//!     for the blocking.  Each new array receives data from a block.
GEOMDLLIMPEXP bool CopyPartitions
(
bvector<ptrdiff_t> const& blockedReadIndex,
bvector<PolyfaceHeaderPtr> &submeshArray
) const;

//! Spread data from this mesh to many new meshes according to partition.  This method is to be used following
//!    PartitionMaintainFaceOrder or PartitionByRange
//! @param [in] blockedReadIndex Array arrays of read indices.
//! @param [out] submeshArray This is initially cleared, then filled with as many (smartpointers to) new arrays as needed
//!     for the blocking.  Each new array receives data from a block.
GEOMDLLIMPEXP bool CopyPartitions
(
bvector<bvector<ptrdiff_t>> &blockedReadIndex,
bvector<PolyfaceHeaderPtr> &submeshArray
) const;


//! Return connected meshes.
//! @param [in] connectivityType 0 for vertex connectivity, 1 for invisible edge connectivity (any shared visible edge is a barrier), 2 for connectivity across any edge (no visibility test)
//! @param [out] submeshArray This is initially cleared, then filled with as many (smartpointers to) new arrays as needed
//!     for the blocking.  Each new array receives data from a block.
GEOMDLLIMPEXP bool PartitionByConnectivity (int connectivityType, bvector<PolyfaceHeaderPtr> &submeshArray) const;

//! Return indices of subsets with consistent forward and reverse visibility for given vector.
//! <ul>
//! <li> facets with normal in the direction of the vector are in readIndices[0]
//! <li> facets with normals opposing the drection of the vector are in readIndices[1]
//! <li> facets wtihin tolerance of perpendicular to the vector are in readIndices[2]
//! </ul>
//! @param [in] vector viewing direction vector
//! @param [out] readIndices read indices for forward, reverse, and perpendicular facets
//! @param [in] sideFaceRadiansTolerance for declaring a facet's normal is perpenducular to the vector.
//! @return false if vector is 000.
GEOMDLLIMPEXP bool PartitionReadIndicesByNormal(
DVec3dCR vector,
bvector<bvector<ptrdiff_t>> &readIndices,
double sideFaceRadiansTolerance = Angle::SmallAngle ()
);
};


//=======================================================================================
//! Implement PolyfaceQuery with directly-stored pointers.
//! Intended for use by callers that have data strewn in memory rather than as an object and just
//! need to package arguments for a call.
//!
//! @DotNetClassExclude
struct PolyfaceQueryCarrier : PolyfaceQuery
{
private:
    DPoint3dCP          m_pointPtr;
    DVec3dCP            m_normalPtr;
    DPoint2dCP          m_paramPtr;
    uint32_t const*     m_intColorPtr;
    int32_t const*      m_pointIndexPtr;
    int32_t const*      m_normalIndexPtr;
    int32_t const*      m_paramIndexPtr;
    int32_t const*      m_colorIndexPtr;
    int32_t const*      m_faceIndexPtr;
    FacetFaceDataCP     m_faceDataPtr;
    PolyfaceEdgeChainCP m_edgeChainsPtr;
    PolyfaceAuxDataCPtr m_auxDataPtr;

    //bool              m_twoSided;
    size_t              m_pointCount;
    size_t              m_paramCount;
    size_t              m_normalCount;
    size_t              m_colorCount;
    size_t              m_indexCount;
    size_t              m_faceCount;
    size_t              m_edgeChainCount;
    uint32_t            m_numPerFace;
    bool                m_twoSided;
    uint32_t            m_meshStyle;
    uint32_t            m_numPerRow;
public:

//! Constructor with parameters for all the data storable in a mesh element

GEOMDLLIMPEXP PolyfaceQueryCarrier (
    uint32_t numPerFace,
    bool   twoSided,
    size_t indexCount,
    size_t pointCount,      DPoint3dCP pPoint, int32_t const* pPointIndex,
    size_t normalCount = 0, DVec3dCP  pNormal = NULL, int32_t const* pNormalIndex = NULL,
    size_t paramCount = 0,  DPoint2dCP pParam = NULL, int32_t const* pParamIndex = NULL,
    size_t colorCount = 0,  int32_t const* pColorIndex = NULL, uint32_t const* pIntColor = NULL,
    void const* unused = nullptr,                // Was illuminated name.
    uint32_t                meshStyle = 1,       // MESH_ELM_STYLE_INDEXED_FACE_LOOPS
    uint32_t                numPerRow = 0        // only needed for QUAD_GRID and TRIANGLE_GRID
    );


//! set FacetFaceData array and count.
GEOMDLLIMPEXP void SetFacetFaceData (FacetFaceDataCP facetFaceData, size_t n);
//! set face index pointer in the carrier.  Note that the number of entries must match all other index arrays (point, normal, param, color)
GEOMDLLIMPEXP void SetFaceIndex (int32_t const *indexArray);
//! set Aux data in the carrier.  Note that the number of entries must match all other index arrays (point, normal, param, color)
void SetAuxData (PolyfaceAuxDataCPtr& auxData) { m_auxDataPtr = auxData; }

public:


size_t              _GetPointCount       ()     const override { return m_pointCount;}
size_t              _GetNormalCount      ()     const override { return m_normalCount;}
size_t              _GetParamCount       ()     const override { return m_paramCount;}
size_t              _GetFaceCount        ()     const override { return m_faceCount; }
size_t              _GetColorCount       ()     const override { return m_colorCount;}
size_t              _GetPointIndexCount  ()     const override { return m_indexCount;}
size_t              _GetFaceIndexCount   ()     const override { return m_indexCount;}
size_t              _GetEdgeChainCount   ()     const override { return m_edgeChainCount;}

DPoint3dCP          _GetPointCP      ()         const override { return m_pointPtr;}
DVec3dCP            _GetNormalCP     ()         const override { return m_normalPtr;}
DPoint2dCP          _GetParamCP      ()         const override { return m_paramPtr;}

uint32_t const*     _GetIntColorCP   ()         const override { return m_intColorPtr;}
FacetFaceDataCP     _GetFaceDataCP()            const override { return m_faceDataPtr; }
PolyfaceEdgeChainCP _GetEdgeChainCP()           const override { return m_edgeChainsPtr;}
PolyfaceAuxDataCPtr _GetAuxDataCP()             const override { return m_auxDataPtr;}


//! For Color, Param, and normal indices, resolveToDefaults allows caller to request using
//! PointIndex (or other default decision) if respective index is same as PointIndex.

int32_t const*      _GetPointIndexCP ()         const override { return m_pointIndexPtr;}
int32_t const*      _GetColorIndexCP ()         const override { return m_colorIndexPtr;}
int32_t const*      _GetParamIndexCP ()         const override { return m_paramIndexPtr;}
int32_t const*      _GetNormalIndexCP ()        const override { return m_normalIndexPtr;}
int32_t const*      _GetFaceIndexCP ()          const override { return m_faceIndexPtr;}
bool                _GetTwoSided ()             const override { return m_twoSided; }
uint32_t            _GetNumPerFace ()           const override { return m_numPerFace; }
uint32_t            _GetNumPerRow ()            const override { return m_numPerRow; }
uint32_t            _GetMeshStyle()             const override { return m_meshStyle; }


};

// Facet Factoids:
// Facets have 9 potential data vectors:
//      Point,Normal,Param,RgbFactor,IntColor
//      PointIndex, NormalIndex, ParamIndex, ColorIndex
// There are two interpretations of intColor:
//     GetIntColorStyle () == 0:  byte style RGBx (Red,Green,Blue,unused)
//     GetIntColorStyle () == 1:  table inex


//! In-memory form of polyface mesh data.
//! The query methods are supported by two levels of implementation:
//! <ul>
//! <li> PolyfaceHeader -- complete collection of facets.  Counts and indices are in the "global" sense
//! <li> PolyfaceVisitor -- individual face.  Counts and indices are for "current face only".
//! </ul>
//! The methods in PolyfaceVectors return references to the various arrays of a polyface mesh.
//! These methods do not implement policy for filling and traversing the arrays.
//! @ingroup BentleyGeom_Mesh
//!
//=======================================================================================
//! @bsiclass                                                     Bentley Systems
//=======================================================================================

struct PolyfaceVectors : PolyfaceQuery
{
friend PolyfaceQuery;
///@cond
#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunused-private-field"
#endif // __clang__

private:    bool                m_dummy;    // published base class must have at least one member

#ifdef __clang__
    #pragma clang diagnostic pop
#endif // __clang__
///@endcond
protected:
    BlockedVector<DPoint3d>             m_point;
    BlockedVector<DPoint2d>             m_param;
    BlockedVector<DVec3d>               m_normal;
    BlockedVector<uint32_t>             m_intColor;
    BlockedVector<FacetFaceData>        m_faceData;
    BlockedVector<PolyfaceEdgeChain>    m_edgeChain;
    BlockedVectorInt                    m_pointIndex;
    BlockedVectorInt                    m_paramIndex;
    BlockedVectorInt                    m_normalIndex;
    BlockedVectorInt                    m_colorIndex;
    BlockedVectorInt                    m_faceIndex;
    uint32_t                            m_numPerFace;
    bool                                m_twoSided;
    uint32_t                            m_meshStyle;
    uint32_t                            m_numPerRow;
    PolyfaceAuxDataPtr                  m_auxData;

protected:
size_t                  _GetPointCount ()       const override;
size_t                  _GetNormalCount ()      const override;
size_t                  _GetParamCount ()       const override;
size_t                  _GetColorCount ()       const override;
size_t                  _GetFaceCount ()        const override;
size_t                  _GetPointIndexCount ()  const override;
size_t                  _GetFaceIndexCount ()   const override;
size_t                  _GetEdgeChainCount ()   const override;
DPoint3dCP              _GetPointCP ()          const override;
DVec3dCP                _GetNormalCP ()         const override;
DPoint2dCP              _GetParamCP ()          const override;
uint32_t const*         _GetIntColorCP ()       const override;
FacetFaceDataCP         _GetFaceDataCP ()       const override;
PolyfaceEdgeChainCP     _GetEdgeChainCP ()      const override;
PolyfaceAuxDataCPtr     _GetAuxDataCP ()        const override;


//! These virtuals are called by plain methods with additional argument for default control.
//!    The defualt logic is implemented by the plain methods.  The virtuals are simple queries,
//!    return null if no specific array provided.
//!
int32_t const*          _GetPointIndexCP  ()     const override;
int32_t const*          _GetColorIndexCP  ()     const override;
int32_t const*          _GetParamIndexCP  ()     const override;
int32_t const*          _GetNormalIndexCP ()     const override;
int32_t const*          _GetFaceIndexCP   ()     const override;
bool                    _GetTwoSided ()          const override;
uint32_t                _GetNumPerFace ()        const override;
uint32_t                _GetMeshStyle ()         const override;
uint32_t                _GetNumPerRow ()         const override;
PolyfaceVectors *       _AsPolyfaceVectorsP() const override; // fast upcast

    PolyfaceVectors ();
    GEOMAPI_VIRTUAL ~PolyfaceVectors () { }  // Virtual destructor needed when there are virtual methods
    //! Reset all BlockedVector tags to constructor defaults.


public:
    //! Copy active flags from the various arrays of the source.
    GEOMDLLIMPEXP void CopyAllActiveFlagsFrom (PolyfaceVectors const& source);
    //! clear all arrays (but flags stay unchanged)
    GEOMDLLIMPEXP void ClearAllArrays ();
    //! In PolyfaceQuery, determine active status from pointers.
    //! This is only valid if the PolyfaceQuery has already been filled !!!
    GEOMDLLIMPEXP void CopyAllActiveFlagsFromQuery (PolyfaceQueryCR source);
    //! Set the flag for twosided facets
    GEOMDLLIMPEXP void  SetTwoSided (bool twoSided);
    //! Set the index blocking count
    GEOMDLLIMPEXP void  SetNumPerFace (uint32_t numPerFace);
    //! Set the row count for gridded facets.
    GEOMDLLIMPEXP void  SetNumPerRow (uint32_t numPerRow);
    //! Set the facet data style.
    GEOMDLLIMPEXP void  SetMeshStyle (uint32_t meshStyle);
    //! Set the auxilliary data.
    GEOMDLLIMPEXP void SetAuxData(PolyfaceAuxDataPtr& auxData);


//! Given signed, one-based indices to a (large) data array.
//! Copy out the data entries that are actually used, and revise indices in place.
//! Return false if data array is empty.  In this case indices are left alone.
template<typename T>
static bool ReindexOneBasedData
(
bvector<T> const &data,                 //!< [in] source data
int invalidIndexReplacement,                   //!< [in] index to replace invalid index
bvector<int> &indices,                  //!< [inout] indices
bvector<T> &newData,                    //!< [out] new data
bvector<size_t> &oldDataToNewData,        //!< [out] array mapping old 0-based data index to new 0-based data index
size_t &numRangeError                   //!< [out] number of indices out of range (replaced by invalidIndexReplacement)
)
    {
    newData.clear ();
    size_t numData = data.size ();
    oldDataToNewData.clear ();
    if (numData == 0)
        {
        numRangeError = indices.size ();
        return false;
        }
    for (size_t i = 0; i < numData; i++)
        oldDataToNewData.push_back (SIZE_MAX);
    numRangeError = 0;
    for (int &index1 : indices)  // ONE BASED
        {
        if (index1 != 0)
            {
            size_t index0 = std::abs (index1) - 1;  // 0 BASED
            if (index0 >= numData)
                {
                index1 = invalidIndexReplacement;     // PANIC
                numRangeError++;
                }
            else
                {
                size_t newIndex0 = oldDataToNewData[index0];
                if (newIndex0 == SIZE_MAX)
                    {
                    newIndex0 = oldDataToNewData[index0] = newData.size ();
                    newData.push_back (data[index0]);
                    }
                int newIndex1 = (int)newIndex0+ 1;
                index1 = index1 > 0 ? (int)newIndex1 : -(int)newIndex1;
                }
            }
        }
    return true;
    }
};
// Mapping from 3 points (indexed within their parent polygon) to barycentric.
// Methods to map arbitrary xyz to uv, normal, and color data from correspondingly organized source.
struct IndexedParameterMap
    {
    // indices of 3 non-colinera points in preclip polygon
    size_t index0;
    size_t index1;
    size_t index2;
    // Transform from world xyz to u,v in triangle.  z coordinate of transform out is off-plane, ignored.
    Transform worldToLocal;
    Transform localToWorld;

    //! Find any 3 non-colinear points and construct world-to-barycentric map.
    //! Return false (with identity transforms) if no independent triple found.   
    bool ConstructMapping(bvector<DPoint3d> const &points);
    // map xyz to barycentric.  Apply these in the data
    DPoint2d MapPoint2d(DPoint3dCR xyz, bvector<DPoint2d> const &params) const;
    // map xyz to barycentric.  Apply these in the normals
    DVec3d MapDVec3d(DPoint3dCR xyz, bvector<DVec3d> const &normals) const;
    };


//=======================================================================================
//! return Access queries for header of polyface mesh.
//!    The combination of PolyfaceVectors base class with summary information.
//! @ingroup BentleyGeom_Mesh
//!
struct PolyfaceHeader : public RefCountedBase, PolyfaceVectors
{
protected:
    PolyfaceHeader ();


public:
//! Compress duplicate coordinate data
GEOMDLLIMPEXP void Compress ();

//! Initial setup for tag data in blocked vectors.
//! Points are active.
//! Point indices are active if style is MESH_ELM_STYLE_INDEXED_FACE_LOOPS
//! All other coordinate and index arrays are NOT active.
//! TwoSided is true.
GEOMDLLIMPEXP void ClearTags (uint32_t numPerFace, uint32_t meshStyle);

//! Add data to index arrays.
GEOMDLLIMPEXP bool AddIndexedFacet
    (
    size_t  numIndex,
    int     *pointIndices,
    int     *normalIndices = NULL,
    int     *paramIndices = NULL,
    int     *colorIndices = NULL
    );

//! Add data to index arrays.
GEOMDLLIMPEXP bool AddIndexedFacet
    (
    bvector<int> &pointIndices,
    bvector<int> *normalIndices = NULL,
    bvector<int> *paramIndices = NULL,
    bvector<int> *colorIndices = NULL
    );

public:
//! Copy all contents to destination vectors
//GEOMDLLIMPEXP void CopyTo (PolyfaceVectors& dest);
//! Return a reference to the point (vertex) coordinate vector.
GEOMDLLIMPEXP BlockedVectorDPoint3dR Point ();
//! Return a reference to the param (texture space) coordinate vector.
GEOMDLLIMPEXP BlockedVectorDPoint2dR Param ();
//! Return a reference to the normal vector.
GEOMDLLIMPEXP BlockedVectorDVec3dR   Normal();
//! Return a reference to the array of color as integers.
GEOMDLLIMPEXP BlockedVectorUInt32R   IntColor ();
//! Return a reference to the point (vertex) index vector.
GEOMDLLIMPEXP BlockedVectorIntR   PointIndex  ();
//! Return a reference to the param index vector.
GEOMDLLIMPEXP BlockedVectorIntR   ParamIndex  ();
//! Return a reference to the normal index vector.
GEOMDLLIMPEXP BlockedVectorIntR   NormalIndex ();
//! Return a reference to the color index vector.   Completion of color dereference depends on additional data in IntColor.
GEOMDLLIMPEXP BlockedVectorIntR   ColorIndex  ();
//! Return a reference to the face index vector.
GEOMDLLIMPEXP BlockedVectorIntR   FaceIndex ();
//! Return a reference to the face information.
GEOMDLLIMPEXP BlockedVector<FacetFaceData> & FaceData ();
//! Return a reference to the edge chain index vector.
GEOMDLLIMPEXP BlockedVector<PolyfaceEdgeChain>& EdgeChain();
//! Return a the aux data pointer.
GEOMDLLIMPEXP PolyfaceAuxDataPtr& AuxData();
//! Clear all index vectors.
GEOMDLLIMPEXP void ClearAllIndexVectors ();
//! Clear all facets.
GEOMDLLIMPEXP void ClearAllVectors ();

//! Add terminator to all active index vectors.
GEOMDLLIMPEXP void TerminateAllActiveIndexVectors ();

//! Set active flags so this polyface carries data and indices for all the data in source.
GEOMDLLIMPEXP void ActivateVectorsForIndexing (PolyfaceQueryR source);
//! Set active flags so this polyface carries data and indices for all the data in source.
//! (Same as ActivateVectorsForIndexing, but CR param)
GEOMDLLIMPEXP void ActivateVectorsForIndexingCR(PolyfaceQueryCR source);

//! Set active flags so this polyface carries data and indices for polylines compatible with source.
GEOMDLLIMPEXP void ActivateVectorsForPolylineIndexing (PolyfaceQueryR source);

//! Make each active flag true/false according to available data.
//! Point, Normal, Param, IntColor, FaceIndex, FaceData
//!     look only at their own data.
//! Indices look at their own size and that of respective indexed arrays.
GEOMDLLIMPEXP void SetActiveFlagsByAvailableData ();

//! Convert the mesh indexing to signed, one-based, variable size face loops.
GEOMDLLIMPEXP bool ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();

//! Find and remove faces with 2 or fewer edges.  This operates only on VariableSizeOneBasedMeshes -- caller responsible for converting to that if needed.
GEOMDLLIMPEXP void RemoveTwoEdgeFacesFromVariableSizeOneBasedMesh ();

//! Create a (smart pointer to a) new (empty) PolyfaceHeader, with variable length faces.
//! This is the most common mesh style.
GEOMDLLIMPEXP static PolyfaceHeaderPtr CreateVariableSizeIndexed ();

//! Create a (smart pointer to a) new (empty) PolyfaceHeader, with fixed number of indices per face
//! @returns invalid if numPerBlock is less than 3.
GEOMDLLIMPEXP static PolyfaceHeaderPtr CreateFixedBlockIndexed (int numPerBlock);

//! Create a (smart pointer to a) new (empty) PolyfaceHeader, with quadrilaterals defined by points in a grid.
//! @returns invalid if numPerRow < 2.
GEOMDLLIMPEXP static PolyfaceHeaderPtr CreateQuadGrid (int numPerRow);

//! Create a (smart pointer to a) new (empty) PolyfaceHeader, with quadrilaterals defined by points in a grid, and then each quad is split to triangle.
//! @returns invalid if numPerRow < 2.
GEOMDLLIMPEXP static PolyfaceHeaderPtr CreateTriangleGrid (int numPerRow);

//! Create a triangulation of points as viewed in xy.  Add the triangles to the polyface.
//! @param [in] points candidate points
//! @param [in] fringeExpansionFactor fractional factor (usually 0.10 to 0.20) for defining a surrounding rectangle.  The z of this triangle is
//!     at the low z of all the points.
//! @param [in] retainFringeTriangles true to keep the fringe triangles.  If false, any edge that reaches the outer rectangle is deleted.
//! @param [in] convexHull if true, force the convex hull to be part of the triangulation.
GEOMDLLIMPEXP static PolyfaceHeaderPtr CreateXYTriangulation (bvector <DPoint3d> const &points, double fringeExpansionFactor = 0.10, bool retainFringeTriangles = false, bool convexHull = true);

//! Create a new polyface with vertical panels between corresponding line segments.
GEOMDLLIMPEXP static PolyfaceHeaderPtr CreateVerticalPanelsBetweenSegments
(
bvector<FacetEdgeDetail> const &segments
);

//! Input an array of meshes expected to have boundary segments are separated by "missing side panels" as viewed in a certain direction.
//! return a (separate, new) mesh with only the side panels.
GEOMDLLIMPEXP static PolyfaceHeaderPtr CreateSidePanelsForViewDirection
(
bvector<PolyfaceHeaderPtr> const &meshes,       //!< [in] array of input meshes
DVec3dCR viewDirection                     //! view direction, e.g. (0,0,1) for usual "missing sides in xy view"
);

//! <ul>
//! <li> Input an array of meshes expected to have boundary segments are separated by "missing side panels" as viewed in a certain direction.
//! <li> return a (separate, new) mesh with the side panels added.   Additional midEdge vertices are inserted into the original facets if T vertices are present.
//! <li> CreateSidePanelsForViewDirection creaates the panels
//! <li> CloneWithTVertexFixup does touchup for extra vertices.
//! </ul>
GEOMDLLIMPEXP static PolyfaceHeaderPtr CloneWithSidePanelsInserted
(
bvector<PolyfaceHeaderPtr> const &meshes,       //!< [in] array of input meshes
DVec3dCR viewDirection                     //! view direction, e.g. (0,0,1) for usual "missing sides in xy view"
);


//! Create a Delauney triangulation of points as viewed in xy.  Return the triangulation and its Voronoi dual as separate polyfaces.
//! @return true if meshes created.
GEOMDLLIMPEXP static bool CreateDelauneyTriangulationAndVoronoiRegionsXY
(
bvector <DPoint3d> const &points,   //!< [in] points to triangulate
PolyfaceHeaderPtr &delauney,        //!< [out] delauney triangulation of the points.
PolyfaceHeaderPtr &voronoi          //!< [out] voronoi regions around the points.
);

//! Create a Delauney triangulation of points as viewed in xy.  Return the triangulation and its Voronoi dual as separate polyfaces, using optional non-euclidean metric for distance in the voronoi.
//! The voronoiMetric selects the assignment of "bisectors"
//!<ul>
//!<li>0 is simple bisector
//!<li>1 is split the distance between circles of specified radii.
//!<li>2 is ratio of radii.
//!<li>3 is the power method (https://en.wikipedia.org/wiki/Power_diagram).  This produces the best intersection points !!!
//!</ul>
//!
//!Detailed cellData contains (for each cell)
//!<ul>
//!<li>siteIndex = original point and radius index
//!<li>auxIndex = readIndex of facet
//!<li>For each Neighbor:
//!<ul>
//!<li>siteIndex = original point and radius index
//!<li>neighborIndex = index of the neighbor in the cellData bvector.  (And cellData[neighborIndex].GetSiteIndex () == siteIndex)
//!</ul>
//!</ul>
//! @return true if meshes created.
GEOMDLLIMPEXP static bool CreateDelauneyTriangulationAndVoronoiRegionsXY
(
bvector<DPoint3d> const &points, //!< [in] points to triangulate
bvector<double> const &radii,    //!< [in] point radii, for use in metric function
int voronoiMetric,               //!< [in] 0 for euclidean distance, 1 for effectiveDistance = euclideanDistance - radius.
PolyfaceHeaderPtr &delauney,    //!< [out] delauney triangulation of the points.
PolyfaceHeaderPtr &voronoi,      //!< [out] voronoi regions around the points.
bvector<NeighborIndices> *cellData = nullptr  //!< [out] optional array giving detailed neighbor data
);
//! Create a triangulation of regions as viewed in xy
GEOMDLLIMPEXP static PolyfaceHeaderPtr CreateConstrainedTriangulation
(
bvector<bvector <DPoint3d>> const &parityLoops,     //!< [in] inputs treated as loops with parity rules for inside/outisde
bvector<bvector <DPoint3d>> const *paths,            //!< [in] additional constraint lines.
bvector<DPoint3d> const *isolatedPoints             //!< [in] isolated points to insert
);

//! Create a triangulation of regions as viewed in xy
GEOMDLLIMPEXP static PolyfaceHeaderPtr CreateConstrainedTriangulation
(
CurveVectorCR loops,           //!< [in] inputs treated as loops with parity rules for inside/outisde
CurveVectorCP paths,            //!< [in] optional additional constraint lines.
bvector<DPoint3d> const *isolatedPoints,   //!< [in] optional additional isolated points to add to triangulation
IFacetOptionsP strokeOptions = nullptr //!< [in] options for stroking curves
);

//! Create a (closed, volumetric) mesh from "bore segment data"
//!<ul>
//!<li> Each bore segment is a pair of top and bottom points of the volume.
//!<li> Ideally the bore data is pure vertical.
//!<li> "somewhat non vertical" bores are ok.
//!<li>Severely non vertical bores will cause folded lower and side surfaces.
//!</ul>
GEOMDLLIMPEXP static PolyfaceHeaderPtr VolumeFromBoreData
(
bvector<DSegment3d> &segments,       //!< [inout] segment bottom and top.  During processing, start and end points are swapped as needed to point[0] is the lower point!!!
bool &foldedSurfaces,                //!< [out] true if bottom or side surfaces are folded.
bvector<ptrdiff_t> *topFacetReadIndex,     //!< [out] optional array of read indices of upper surface facets
bvector<ptrdiff_t> *bottomFacetReadIndex,     //!< [out] optional array of read indices of lower surface facets
bvector<ptrdiff_t> *sideFacetReadIndex     //!< [out] optional array of read indices of side facets
);
//! Create a (smart pointer to a) new (empty) PolyfaceHeader, with each facet defined
//! by 3 or 4 unindexed points as indicated by the arg.
//! @returns invalid if numPerFace is other than 3 or 4.
GEOMDLLIMPEXP static PolyfaceHeaderPtr CreateFixedBlockCoordinates (int numPerFace);

//! Create a mesh with (just) point and index data.
GEOMDLLIMPEXP static PolyfaceHeaderPtr CreateIndexedMesh (int numPerFace, bvector<DPoint3d> const &points, bvector<int> const &indexData);


//! Create a (indexed) polyface containing all polygons from a TaggedPolygonVector
GEOMDLLIMPEXP PolyfaceHeaderPtr static CreateFromTaggedPolygons
(
TaggedPolygonVectorCR polygons  //!< [in] tagged polygons to place in polyface
);

//! Create a polyface from (a subset of) polygons.
GEOMDLLIMPEXP PolyfaceHeaderPtr static CreateFromTaggedPolygons
(
TaggedPolygonVectorCR polygons,     //!< [in] source polygons
Acceptor<TaggedPolygon> &acceptor,   //!< [in] object with Accept () method to be called to test each polygon in the TaggedPolygonsVector
bvector<SizeSize> *destToSource = nullptr    //!< [inout] optional array.  In each entry, dataA is a readIndex in the output facets, and dataB is the index of its soruce in polygons.
);

//! Create a polyface containing all visible parts for a flat view of among multiple meshes.
GEOMDLLIMPEXP void static VisibleParts
(
bvector<PolyfaceHeaderPtr> &source, //!< [in] multiple meshes for viewing
DVec3dCR vectorToEye,               //!< [in] vector towards the eye
PolyfaceHeaderPtr &dest,            //!< [out] new mesh, containing only the visible portions of the inputs
TransformR localToWorld,            //!< [out] axes whose xy plane is the xy plane for viewing along local z axis.
TransformR worldToLocal             //!< [out] transform used to put the polygons in xy viewing position.
);

//! Find visible parts from facets in multiple input polygons.
//!
GEOMDLLIMPEXP void static VisibleParts
(
bvector<PolyfaceHeaderPtr> &source, //!< [in] multiple meshes for viewing
DVec3dCR vectorToEye,               //!< [in] vector towards the eye
bvector<PolyfaceHeaderPtr> &dest,            //!< [out] array of new mesh, containing only the visible portions of the inputs.  If a particular mesh source[i] has no visible parts, its corresponding dest[i] is a null pointer.
bvector<bvector<SizeSize>> *destReadIndexToSourceReadIndex,  //!< [out] array connecting destMesh readIndex to its corresponsding readIndex in the corresponding source mesh
TransformR localToWorld,            //!< [out] axes whose xy plane is the xy plane for viewing along local z axis.
TransformR worldToLocal             //!< [out] transform used to put the polygons in xy viewing position.
);

//! Compute what parts of meshB are hidden by meshA (the hider).
//! NOTE: If there is no hiding, both returned meshes are null. (hidable is NOT copied)
GEOMDLLIMPEXP void static MeshHidesMeshXYByPlaneSets (
PolyfaceHeaderPtr &hider,   //!< [in] mesh that might hid part of hidable.
PolyfaceHeaderPtr &hidable, //!< [in] mesh that might be partially hidden
PolyfaceHeaderPtr &meshBVisible,    //!< [out] visible parts of the hidable mesh
PolyfaceHeaderPtr &meshBHidden,      //!< [out] hidden parts of the hidable mesh
bool computeAndApplyTransform = true //!< [in] if true, compute a transform to move data to the origin.
);

//< Compute pairwise hidden-visible splits, and replace each input mesh by its visible parts.
//< Note that meshes that become fully hidden become nullptr in the allMesh array.
//< Each mesh is individually assumed "upward facing"
GEOMDLLIMPEXP void static MultiMeshVisiblePartsXYByPlaneSets
(
bvector<PolyfaceHeaderPtr> &allMesh,        //!< [in] multiple meshes
bvector<PolyfaceHeaderPtr> &visibleParts    //!< [out] the corresponding visible parts.   Some entries may be null meshes!!
);

//! DEPRECATED -- Use PolyfaceHeader::CreateVariableSizeIndexed ();
GEOMDLLIMPEXP static PolyfaceHeaderPtr New ();

/*__PUBLISH_SECTION_END__*/
GEOMDLLIMPEXP bool HasAuxiliaryColors (bool &colorsAreByVertex, bool &colorsAreByFace, bool &colorsAreBySector);
/*__PUBLISH_SECTION_START__*/

//! Copy all data to another header.
GEOMDLLIMPEXP void CopyTo (PolyfaceHeader& dest) const;

//!  Set face data for all facets added since last call to SetNewFaceData. (endIndex = 0 for all facets).
GEOMDLLIMPEXP void SetNewFaceData (FacetFaceData* faceData, size_t endIndex = 0);

//! Triangulate faces.
//! return SUCCESS if all faces triangulated.
//! @remark this should return bool.
GEOMDLLIMPEXP BentleyStatus Triangulate ();

//! Copy all data to a new mesh, reorganizing so that all data arrays have the same index structure.
//! This is a memory-efficient structure ONLY for smooth surfaces such as bspline, cylinder, sphere.
//! This is a highly inefficent structure for any mesh with interior edges.
//! Unfortunately it is a common mesh structure in exchange formats.
GEOMDLLIMPEXP static PolyfaceHeaderPtr CreateUnifiedIndexMesh (PolyfaceQueryCR source);

//! Inspect and correct the direction of "at vertex" normals relative to the ordering of vertices around facets.
//! Returns true if any changes were made.
//! The summary vector will contain entries indicating:
//!<ul>
//!<li>Early exit if fails assertion "Mesh should (but does not) have both NormalIndex and Normal () data"
//!<li>Quiet warning, not a change trigger: "Ignoring facet whose facet normal cannot be computed from vertex coordinates"
//!<li>Quiet warning, not a change trigger: "Ignoring out of range normal index"
//!<li>Quiet warning of change trigger: "vertex normal has both positive and negative incident facets.  A new negated normal is introduced"
//!<li>Quiet warning of change trigger: "All incident facets normals are reverse of vertex normal -- normal is negated"
//!<li>Quiet warning, not a change trigger: "unused normal coordinates"
//!</ul>
GEOMDLLIMPEXP bool FixupVertexNormalDirectionToFaceOrientation
(
MeshAnnotationVector &summary,  //!< [out] array of status messages
bool makeChanges                //!< [in] if false, just build up the status messages.   If true also make changes.
);
//! Find subsets of facets, considered connected if two adjacent facets share an edge with opposing orientation
GEOMDLLIMPEXP bool OrientAndCollectManifoldComponents
(
bvector<bvector<size_t>> &componentReadIndices, //!< [out] arrays of read indices gathered per component
MeshAnnotationVector &messages  //!< [out] array of status messages
);

//! Determine a facet order such that the LAST facets are the first to be removed when applying the logic
//! "Remove the longest exterior edge first"
//!<ul>
//!<li> If this is applied to facets of a triangulation (whose outer boundary is convex), the successive outer boundaries
//! are polygons that contain short edges and have inlets where there are long edges on the outside.
//! The readIndexSequence contains sequences of readIndices for the shuffled facets.
//!<li>  Suppose a facet
//!    <ul>
//!    <li>initially has vertices, params, normals, and colors indicated at (consecutive) readIndices [a b c]
//!    <li> the edges from b to c is chosen for removal
//!    </ul>
//!</li>
//!<li>That facet will appear as [b c a SIZE_MAX].
//!</ul>
GEOMDLLIMPEXP bool ConstructOrderingForLongEdgeRemoval
(
bvector<size_t> &readIndexSequence,    //!< [out] read indices in order described above.
double maxEdgeLength = 0.0  //!< [in] exclude facets whose edge length is larger than this.  Use 0.0 to include all facets.
);

//!
//! <ul>
//! <li> Find facets with boundary edges longer than maxEdgeLength.
//! <li> Remove the facets.
//! <li> Continue searching for long edges in the newly exposed facets.
//! <li> If the initial facets are an xy triangulation of points (with the convex hull outer boundary),
//!      the first removals creates a non-convex  outer boundary.  Later removals can create islands of facets.
//! </ul>
GEOMDLLIMPEXP bool ExcavateFacetsWithLongBoundaryEdges (double maxEdgeLength = 0.0);
//!
//! <ul>
//! <li> Find the maximum edge length of each facet.
//! <li> Split into two sets of facets with that criteria.
//! <li> Note that removal can happen anywhere in the mesh.
//! <li> use ExcavateFacetsWithBoundaryEdges to remove only edges reachable by crossing long edges 
//!          from a long starting edge on the boundary.
//! </ul>
GEOMDLLIMPEXP bool SplitByMaxEdgeLength(double splitLength, bvector<PolyfaceHeaderPtr> &splits);
//! Add Edge Chains
GEOMDLLIMPEXP BentleyStatus AddEdgeChains (size_t drawMethodIndex);

//! Triangulate faces that are nonplanar or have too many edges.
//! return true if all triangulated or within restrictions.
GEOMDLLIMPEXP bool Triangulate (size_t maxEdge);
//! Triangulate selected facets
GEOMDLLIMPEXP bool Triangulate
(
size_t maxEdge,                //!< [in] target edges per facet.
bool hideNewEdges,              //!< [in] true to mark triangle edges invisible.
IPolyfaceVisitorFilter *filter  //!< [in] optional object to ask if current facet of visitor is to be triangulated.
);
//! Revise index structure to minimize array lengths.
//! @returns true if any changes were made.
GEOMDLLIMPEXP bool CompactIndexArrays ();


//! Apply a transform to all coordinates. Optionally reverse index order (to maintain cross product relationships)

GEOMDLLIMPEXP void Transform
(
TransformCR transform,
bool        reverseIndicesIfMirrored = true
);

//! Apply a transform to all coordinates of an array of meshes. Optionally reverse index order (to maintain cross product relationships)

static GEOMDLLIMPEXP void Transform
(
bvector<PolyfaceHeaderPtr> &data,
TransformCR transform,
bool        reverseIndicesIfMirrored = true
);


//!
//! Reverse (negate) all stored normals.  Note that this does NOT change index order.
GEOMDLLIMPEXP void ReverseNormals ();

//!
//! Normalize parameters.
GEOMDLLIMPEXP void  NormalizeParameters ();

//!
//! Collect indices of (1) adjacent facets and (2) points within the active facets.
GEOMDLLIMPEXP void CollectAdjacentFacetAndPointIndices
(
bvector<size_t> &activeReadIndex,     //!< [in,out] readIndices of active facets.  This is sorted in place.
bvector<size_t> &fringeReadIndex, //!< [out] readIndices of facets that have a least one vertex indicent to the activeFacets.
bvector<size_t> &activePointIndex        //!< [out] indices of points incident to any active facet.
);

enum FacetTranslationMode
{
JustTranslatePoints,                        //!< just translate points. In a non-triangulated mesh, this may create nonplanar facets.
TranslatePointsAndAddSweepFaces,            //!< translate points and create new facets to fill the space.
TranslatePointsAndTriangulateFringeFaces    //!< triangulate adjacent facets before translating the active points.
};
//! Apply a translation to a subset of facets.
GEOMDLLIMPEXP void TranslateSelectedFacets
(
bvector<size_t> &activeReadIndex,     //!< [in,out] readIndices of active facets.  This is sorted in place.
DVec3dCR vector,            //!< [in] translation vector
FacetTranslationMode mode   //!< [in] selector for deforming or adding facets
);

GEOMDLLIMPEXP PolyfaceHeaderPtr CloneWithTranslatedFacets (bvector<size_t> &activeReadIndex, DVec3dCR vector, PolyfaceHeader::FacetTranslationMode mode);

//! Search the mesh for facets that identical sets of point indices.
//! Return a clone with only one copy of each.
GEOMDLLIMPEXP PolyfaceHeaderPtr CloneWithIndexedDuplicatesRemoved () const;
//! Search for adjacent, coplanar facets.
//! Merge to get maximual planar facets.
//! Optionally remove vertices that have only two incident and colinear edges.
GEOMDLLIMPEXP PolyfaceHeaderPtr CloneWithMaximalPlanarFacets
(
bool mergeCoplanarFacets,   //!< [in] true to merge coplanar facets
bool mergeColinearEdges     //!< [in] true to eliminate vertices splitting simple line edges.
);

//! Clone the meshes as a single mesh, inserting vertices along edges where vertices from other facets create T-Vertex topology
GEOMDLLIMPEXP static PolyfaceHeaderPtr CloneWithTVertexFixup
(
bvector<PolyfaceHeaderPtr> const &meshes,       //!< [in] array of input meshes
IFacetOptions *options = nullptr,             //!< [in] optional facet options.   If null, output is triangulated.
double onEdgeTolerance = 0.0        //!< [in] tolerance for identifying T vertex.  defaults to DoubleOps::SmallMetricDistance ()
);

//! Compute meshes "between" road and dtm.
//! Return as closed volume meshes.
//!<ul>
//!<li> dtm and road must be strictly single-valued Z
//!<li> both must be convex facets.
//!</ul>
static GEOMDLLIMPEXP void ComputeSingleSheetCutFill
(
PolyfaceHeaderCR dtm,                //!< [in] dtm mesh
PolyfaceHeaderCR road,               //!< [in] "road" mesh.
DVec3dCR viewVector,                 //!< [in]  viewDirection
PolyfaceHeaderPtr &cutMesh,         //!< [out] facets for cut volumes (road below dtm)
PolyfaceHeaderPtr &fillMesh        //!< [out] facets for fill volumes (road above dtm)
);

//! Decimate (in place) by simple rules that aggressively collapse short edges.
//! This is the fastest decimation, but it does not protect boundary points.
//! @return number of collapses.
GEOMDLLIMPEXP size_t DecimateByEdgeCollapse
(
double abstol,  //!< [in] absolute tolerance for collapsing vertices
double rangeFractionTol //!< [in] tolerance as fraction of range of the mesh points
);
//!  @description Fast normal generation - used during tile generation.
//! @param [in] creaseTolerance dihedral angle considered "smooth" for a single edge.
//! @param[in] sharedEdgeSizeTolerance if facet is smaller than this tolerance then expenseive shared normal calculation is omitted.
GEOMDLLIMPEXP void BuildNormalsFast(double creaseTolerance, double sharedEdgeSizeTolerance);

//! Examine vertex indices within each facet.
//! If consecutive indices are identical, that is a short edge.
//! if
size_t RemoveCollapsedFacetsByPointIndexComparison ();
#ifdef CompileDecimateByEdgeCollapseWithBoundaryControl
//! Decimate (in place) by edge collapse, protecting boundary points when possible.//! @return number of collapses.
GEOMDLLIMPEXP size_t DecimateByEdgeCollapseWithBoundaryControl
(
double abstol,  //!< [in] absolute tolerance for collapsing vertices
double rangeFractionTol //!< [in] tolerance as fraction of range of the mesh points
);
#endif
//! Search the mesh for facets that have identical sets of point indices.
//! Return read indices separated by counts.
//! Suppose
//! <ul>
//! <li>facets at read index 0,5 appear only once.
//! <li>facets at read index 10,15 are identical
//! <li>facets at read index 20,25 are identical
//! <li>facets at read index 30,35,40,45 are identical
//! </ul>
//! Then the contents of the various arrays are:
//! <ul>
//! <li>nonduplicatedFacetReadIndex: 0,5
//! <li>duplicatedFacetFirstReadIndex: 10,20,30
//! <li>duplicatedFacetAdditionalReadIndex: 15,25,35,40,45
//! <li>baseIndexForAdditionalReadIndex: 10,20,30,30,30
//! </ul>
//! Note that
//! <ul>
//! <li>The duplicatedFacetAdditionalReadIndex and baseIndexForAdditionalReadIndex vectors are the same length.
//! <li>All vectors are passed by pointer.
//! <li>Passing nullptr indicates the caller is no interested in that vector's contents.
//! <li>A vector address may be passed in two positions to combine those categories.
//! <li>A common usage is to pass call IdentifyDuplicates (&readIndices, &readIndices, nullptr, nullptr), i.e. get the nonduplicated facets and one representative from
//!      each set of duplicates.  Then pass that one vector to copyPartitions to get a cloned facet set with duplicates removed.
//! </ul>
GEOMDLLIMPEXP void IdentifyDuplicates
(
bvector<ptrdiff_t> *nonduplicatedFacetReadIndex,       //!< [out] (optional) vector of read indices of facets that appear only once
bvector<ptrdiff_t> *duplicatedFacetFirstReadIndex,     //!< [out] (optional) vector with one read index representing each set of duplicated facets.
bvector<ptrdiff_t> *duplicatedFacetAdditionalReadIndex,    //!< [out] (optional) vector with read indices of duplications whose first representative is already in duplicatedFacetFirstIndex
bvector<ptrdiff_t> *baseIndexForAdditionalReadIndex    //!< [out] (optional) vector where baseIndexForAdditionalReadIndex[i] entries tell the first representative of duplicatedFacetAdditionalReadIndex[i]
) const;


//! Add a polygon directly to the arrays. Indices created as needed.
GEOMDLLIMPEXP bool AddPolygon (DPoint3dCP xyz, size_t n, DVec3dCP normal = NULL, DPoint2dCP param = NULL);
//! Add a polygon directly to the arrays.  Indices created as needed.
GEOMDLLIMPEXP bool AddPolygon (bvector<DPoint3d> const &xyz, bvector<DVec3d> const *normal = NULL, bvector<DPoint2d> const *param = NULL);
//! Add a polygon directly to the arrays.  Indices created as needed.
//! Interpolate (if active) params, normals, and colors with barycentric mapping from visitor.
GEOMDLLIMPEXP bool AddPolygon(bvector<DPoint3d> const &xyz, PolyfaceVisitorR visitor, IndexedParameterMap const &mapping);
//! Add a polygon with linear mapping to parameter space
//! If compressNormal is true, the normal is compared to the most recent normal
//!     and that index is reused when identical normal index.
GEOMDLLIMPEXP bool AddPolygon(bvector<DPoint3d> const &xyz, TransformCR worldToParameterSpace, DVec3dCR normal, bool compressNormal, bool reverseXYZ);

//! Sweep the existing mesh promote to a solid
//! @returns false if the input mesh has inconsistent visibility -- i.e. side or mixture of forward and back facing facets.
GEOMDLLIMPEXP bool SweepToSolid (DVec3dCR sweepVector, bool triangulateSides);

//! Clear current data, append data from (readonly) source
GEOMDLLIMPEXP void CopyFrom (PolyfaceQueryCR source);

//! Add all content of source to this polyface.
//! This does NOT attempt to recognize duplicate coordinate data.
//! @returns false if mismatched data -- e.g. arrays present on one but not the other.
GEOMDLLIMPEXP bool AddIfMatchedLayout (PolyfaceQueryCR source);


//! Compute average normals by averaging local facet normals.
//! @param [in] maxSingleEdgeAngle dihedral angle considered "smooth" for a single edge.
//! @param [in] maxAccumulatedAngle max accumualted dihedral angle change over multiple edges.
//! @param [in] markAllTransitionsVisible if true, edges are marked visible if adjoining facet sectors to each side do not share normals.
//! @return true if normals computed.
GEOMDLLIMPEXP bool BuildApproximateNormals (double maxSingleEdgeAngle = 0.2, double maxAccumulatedAngle = 0.3, bool markAllTransitionsVisible = true);

//! options structure for polyface offset.
//! Default constructor sets typical values.
//! Fields are then accessible for modification.
struct OffsetOptions
{
Angle m_maxSingleEdgeAngle;     // max angle for sharing normal across a single edge
Angle m_maxAccumulatedAngle;    // max accumulated angle for sharing normal
Angle m_maxChamferAngle;        // max angle for simple chamfer. Recommended value between Angle::FromDegrees (91.0) to Angel::FromDegrees (120.0)
bool m_useStoredNormals;        // NOT USED

//! Default options
OffsetOptions () :
    m_maxSingleEdgeAngle (Angle::FromDegrees (20.0)),
    m_maxAccumulatedAngle (Angle::FromDegrees (60.0)),
    m_maxChamferAngle (Angle::FromDegrees (90.0)),
    m_useStoredNormals (false)
    {}
};

//! Compute offset(s) of a mesh surface.
//! Optionally combine two offsets to form a closed volume.
//!<ul>
//!<li>Example: to Thicken by 1.0 towards the "outside" use distances (1.0, 0.0)
//!<li>Example: to thicken by 1.0 towards the "inside" use distances (0.0, 1.0)
//!<li>Example: to thicken by 2.0 towards the "outside" and 1.0 towards the "inside" use distances (2.0, 1.0)
//!<li>All orientations and connectivity are taken from the input mesh.  This is not a fixup operation.
//!<ul>
GEOMDLLIMPEXP PolyfaceHeaderPtr ComputeOffset
(
OffsetOptions const &options, //!< [in] offset polyface offset options
double distance1,            //!< [in] offset distance for a positively oriented offset
double distance2,            //!< [in] offset distance for a negatively oriented offset
bool outputOffset1 = true,         //!< [in] true to output the (positive oriented) offset at distance1
bool outputOffset2 = true,         //!< [in] true to output the (negatively oriented) offset at distance2
bool outputSideFacets = true        //!< [in] true to output side facets where boundary edges are swept
);

//! Compute local coordinates within each facet.
//! @return true if parameters computed.
GEOMDLLIMPEXP bool BuildPerFaceParameters (LocalCoordinateSelect selector);
//! Compute parameters from xy parts of point coordinates.
//! Transformations are computed by PolygonOps::CoordinateFrame for the bottom rectangle of the world bounding box
GEOMDLLIMPEXP bool BuildXYParameters
(
LocalCoordinateSelect selector, //!< [in] selects range of params
TransformR localToWorld,        //!< [out] transform from params to world.
TransformR worldToLocal         //!< [out] transform from world to params
);

//! Compute parameters from caller-supplied transform from world to parameter space
GEOMDLLIMPEXP bool BuildParametersFromTransformedPoints
(
TransformCR worldToLocal         //!< [in] transform from world to params
);
//!    slightly twisted quads that must be triangulated for calculations.
//!    If triangulated, both the transverse edges and diagonals would get hidden
//!    by usual dihedral angle rules.
//!    This hides the diagonals but leaves the simple transverse edges visible.
GEOMDLLIMPEXP bool MarkDiagonalEdgesInvisible
(
double smoothAngle,
double edgeLengthFactor = 1.001,
uint32_t maxEdgesInFacetForDiagonalRules = 3);

//! Clear current normal data.
//! @param [in] active active state (true/false) to be applied after clearing.
GEOMDLLIMPEXP void ClearNormals (bool active);

//! Clear current param data.
//! @param [in] active active state (true/false) to be applied after clearing.
GEOMDLLIMPEXP void ClearParameters (bool active);

//! Compute a normal vector for each faceet.   Install indices.
GEOMDLLIMPEXP bool BuildPerFaceNormals ();

//! Compute face data for each facet.
GEOMDLLIMPEXP bool BuildPerFaceFaceData ();

//! Mark edges invisible (negative index) if dihedral angle between normals is small.
//! If normals are present they are used.  If not present, per-face normals are computed and used (but then removed)
GEOMDLLIMPEXP bool MarkInvisibleEdges
(
double smoothAngle,                 //!< [in] angle (in radians) for smooth angle markup.
DVec3dCP silhouetteVector           //!< [in] optional vector for silhouette markup.
);

//! Mark edges invisible (negative index) if dihedral angle between normals is small.
//! If normals are present they are used.  If not present, per-face normals are computed and used (but then removed)
GEOMDLLIMPEXP bool MarkInvisibleEdges (double smoothAngle) { return MarkInvisibleEdges (smoothAngle, nullptr);}


//! Expose topological boundaries
//! @param [in] preserveOtherVisibility if false, all others are marked in invisible.  If true, others remain as currently marked.
//! @return true if any edges were changed.
GEOMDLLIMPEXP bool MarkTopologicalBoundariesVisible (bool preserveOtherVisibility);
//! Mark all edges visible.
GEOMDLLIMPEXP void MarkAllEdgesVisible ();



//! Partition this mesh with target face counds and component count, using facet xy range to (roughly) maintain
//!   geometric proximity within components.
//! @param [in] targetFaceCount target number of faces per mesh.
//! @param [in] targetMeshCount target number of meshes.
//! @param [out] submeshArray This is initially cleared, then filled with as many (smartpointers to) new arrays as needed
//!     for the blocking.  Each new array receives data from a block.
GEOMDLLIMPEXP bool PartitionByXYRange
(
size_t targetFaceCount,
size_t targetMeshCount,
bvector<PolyfaceHeaderPtr> &submeshArray
);

//! Partition this mesh with target face counds and component count, maintaining current order of facets.
//! @param [in] targetFaceCount target number of faces per mesh.
//! @param [in] targetMeshCount target number of meshes.
//! @param [out] submeshArray This is initially cleared, then filled with as many (smartpointers to) new arrays as needed
//!     for the blocking.  Each new array receives data from a block.
GEOMDLLIMPEXP bool PartitionMaintainFaceOrder
(
size_t targetFaceCount,
size_t targetMeshCount,
bvector<PolyfaceHeaderPtr> &submeshArray
);


//! Return blocks of read indices for grouping with targets for number of faces and partitions.
//! @param [in] targetFaceCount target number of faces per mesh.
//! @param [in] targetMeshCount target number of meshes.
//! @param [out] blockedReadIndexArray read indices for individual faces, separated by (-1).
GEOMDLLIMPEXP bool PartitionByXYRange
(
size_t targetFaceCount,
size_t targetMeshCount,
bvector<ptrdiff_t> &blockedReadIndexArray
);

//! Return blocks of read indices for grouping with targets for number of faces and partitions.
//! @param [in] targetFaceCount target number of faces per mesh.
//! @param [in] targetMeshCount target number of meshes.
//! @param [out] blockedReadIndexArray read indices for individual faces, separated by (-1).
GEOMDLLIMPEXP bool PartitionMaintainFaceOrder
(
size_t targetFaceCount,
size_t targetMeshCount,
bvector<ptrdiff_t> &blockedReadIndexArray
);



//! Copy selected blocks of read indices to a new blocked index array.
//! @param [in] blockedReadIndex Array of read indices with -1 as terminator between blocks that are to go to the same destination mesh.
//! @param [in] selectedReadIndex Array of read indices for choosing blocks.
//! @param [in] keepIfSelected indicates what to do with a block when it contains an index that is in the selectedReadIndexArray.
//!        <ul>
//!        <li>true means when a block contains a selected read index it is copied.
//!        <li>false means when a block contains a selected read indexc it is ignored.
//!        </ul>
//! @param [out] blockedReadIndexOut array containing only the accepted blocks
static GEOMDLLIMPEXP void SelectBlockedIndices
(
bvector<ptrdiff_t> const &blockedReadIndex,
bvector<ptrdiff_t> const &selectedReadIndex,
bool keepIfSelected,
bvector<ptrdiff_t> &blockedReadIndexOut
);



//! Compute the lengths of the longest u and v direction size of any single facet, looking only at the stored param.
//! @param [out] uvLength sizes in u, v directions.
//! @return false if the facets do not have params
GEOMDLLIMPEXP bool TryGetMaxSingleFacetParamLength (DVec2dR uvLength);

//! Compute the lengths of the longest horizontal and vertical direction size of any single facet, using the
//! local coordinate system along the first edge of the facet for directions.
//! @param [out] xySize sizes in u, v directions.
GEOMDLLIMPEXP bool TryGetMaxSingleFacetLocalXYLength (DVec2dR xySize);

//! Create linestrings containing only the visible edges of this polyface.
//! @remark The extraction is based on edge visiblity, not boundary analysis.
//! @param [out] numOpen number of open faces.
//! @param [out] numClosed number of closed faces.
GEOMDLLIMPEXP CurveVectorPtr ExtractBoundaryStrings (size_t &numOpen, size_t &numClosed);

//! Create segments containing edges of this polyface.
//! The returned array indicates
//! <ul>
//! <li>segment coordinates
//! <li> a readIndex.  Based on the returnSingleEdgeReadIndex value, his can be either the base readIndex for the whole facet, or the detail read index for the individual edge.
//! <li> clusterIndex.  Shared edges will return the same cluster index.
//! <li> number of edges in the cluster.   If collecting only unmatched edges, this normally be 1, but can be
//     3 or more for nonmanifold edges, and 2 if the two edges do not have the required direction reversals.
//! </ul>
//!
//! @param [out] segments array of segment data.
//! @param [in] includeMatched true to include interior segemnts that have mates.
//! @param [in] returnSingleEdgeReadIndex if true, return read index to the base of the individual edge.  If false,
//!      return readIndex for the entire facet.  (The entire facet index is prefered for calling visitor->MoveToReadIndex ())
GEOMDLLIMPEXP void CollectEdgeMateData (bvector<FacetEdgeDetail> &segments,
        bool includeMatched = false,
        bool returnSingleEdgeReadIndex = false);

//! If (1) param, normal, or color indices are missing and (2) their respective data arrays have size match with points,
//!    fill up the index array as duplicate of the pointIndex
GEOMDLLIMPEXP void  ReplicateMissingIndexArrays ();
};

/*__PUBLISH_SECTION_END__*/
struct PolyfaceIndexedHeapRangeTree;
typedef RefCountedPtr<PolyfaceIndexedHeapRangeTree>  PolyfaceIndexedHeapRangeTreePtr;

// Range-based search with indexed range heap.
// This is wraps a binaryRangeHeap for either polyface or TaggedPolygonVector.
// It's hierarchical search tree has indices back to the indices of the source.
struct PolyfaceIndexedHeapRangeTree : public RefCountedBase
{
private:
BENTLEY_GEOMETRY_INTERNAL_NAMESPACE_NAME::IndexedRangeHeap m_heap;
bvector<size_t>  m_heapIndexToReadIndex;
PolyfaceIndexedHeapRangeTree ();
~PolyfaceIndexedHeapRangeTree ();

size_t LoadPolyface (PolyfaceQueryCR source, bool sortX, bool sortY, bool sortZ);
size_t LoadPolygons (TaggedPolygonVectorCR source, bool sortX, bool sortY, bool sortZ);

public:
// Create a searcher for facets in their existing order.
GEOMDLLIMPEXP static PolyfaceIndexedHeapRangeTreePtr CreateForPolyface (PolyfaceQueryCR source);
// Create a searcher for the facets using an xy sort
GEOMDLLIMPEXP static PolyfaceIndexedHeapRangeTreePtr CreateForPolyfaceXYSort (PolyfaceQueryCR source);
// Create a searcher for the facets using a combination of x,y,z sorts.
GEOMDLLIMPEXP static PolyfaceIndexedHeapRangeTreePtr CreateForPolyface
    (PolyfaceQueryCR source, bool sortX, bool sortY, bool sortZ);
GEOMDLLIMPEXP static PolyfaceIndexedHeapRangeTreePtr CreateForPolygons
    (TaggedPolygonVectorCR source, bool sortX, bool sortY, bool sortZ);

GEOMDLLIMPEXP bool TryGetReadIndex (size_t facetIndex, size_t &readIndex) const;
// Find all facets in specified search range (with optional expansion)
GEOMDLLIMPEXP void CollectInRange (bvector<size_t> &hits, DRange3dCR range, double expansion = 0);
// Return a reference to the heap itself.
GEOMDLLIMPEXP BENTLEY_GEOMETRY_INTERNAL_NAMESPACE_NAME::IndexedRangeHeap &GetHeapR();

// Collect read indices of each node at specified depth.
GEOMDLLIMPEXP bool CollectReadIndicesByTreeDepth (bvector<bvector<size_t>> &readIndices, int depth) const;
// return the number of ranges.
GEOMDLLIMPEXP size_t GetNumRanges () const;
// get a range by index.
GEOMDLLIMPEXP bool TryGetRange (size_t index, DRange3dR range) const;
};


/*=================================================================================**//**
* Filter class for searching facets for edge hits.
+===============+===============+===============+===============+===============+======*/
struct PolyfaceEdgeSearcher
{
private:
PolyfaceVisitorPtr m_visitor;
DMatrix4d m_worldToLocal;
double    m_localAperture;
/* unused - DPoint3d  m_worldPickPoint;*/
/* unused - DPoint3d  m_localPickPoint;*/
bool      m_ignoreInvisibleEdges;

/* unused - bool m_searchComplete;*/

public:
//! itemization of cause for return from searches.
enum class SearchState
    {
    Complete,
    SuspendByMaxTest,
    SuspendByMaxNewHit
    };

//! Initialize for search
GEOMDLLIMPEXP PolyfaceEdgeSearcher (PolyfaceQueryCR polyface, DMatrix4dCR worldToLocal, double localAperture, bool ignoreInvisibleEdges);
//! Reset for further search.
GEOMDLLIMPEXP void Reset ();
//! continue search.
//! @param [in] hits array of hits.  In order to allow interrupted search, this is NOT cleared.
//! @param [in] maxTest maximum number of facets to test
//! @param [in] maxNewHit maximum number of new hits to collect
//! @return indicator of reason for end of search.
//! @remarks
//! <ul>
//! <li>Caller can determine the number of new hits by comparing before-and-after size of the hits array.
//! <li>FacetLocationDetail contents are:
//!   <ul>
//!   <li>>readIndex = readIndex for entire facet.
//!   <li>point = closest point on edge
//!   <li>param.x = fractional coordinate on edge
//!   <li>dXdU = edge start point
//!   <li>dXdV = edge end point
//!   <li>sourceFraction[0] = fractional coordinate on edge
//!   <li>sourceIndex[0] = edge index within the facet
//!   <li>a = xy distance from space point (after transform)
//!   </ul>
//! </ul>
GEOMDLLIMPEXP SearchState AppendHits
(
DPoint3dCR localPick,               //!< [in] local (most worldToLocal) pick coordinates for edge hits
bvector<FacetLocationDetail> *edgeHits, //!< [inout] growing array of hits.
bvector<FacetLocationDetail> *facetHits, //!< [inout] growing array of hits.
size_t maxTest = SIZE_MAX,          //!< [in] maximum number of new facets to test
size_t maxNewHit = SIZE_MAX         //!< [in] maximum number of new hits to record
);
};

/*=================================================================================**//**
* Filter class for selecting facets by a polygon in view.
+===============+===============+===============+===============+===============+======*/
struct PolyfacePolygonPicker
{
//! Results class for pick-by-stroke
struct StrokePick
{
double u0;
double u1;
double z0;
double z1;
size_t index;
bool isVisible;
bool isHidden;


//! Interpolate (possibly multivalued) Z at U
//!<ul>
//!<li> if u is a simple interior coordinate in the pick, interpolate to its z as a valid result
//!<li> if u exactly matches both u0 and u1, return the z0..z1 range sorted as a valid result
//!<li> if u is outside and u0==u1, return the z0..z1 range sorted but not valid
//!<li> if u is outside and u0 differs from u1 return the nearer z but not valid
//!<li> i.e. IsValid means "strictly in the interval"
//!</ul>
ValidatedDSegment1d InteriorZAtU (double u) const;
};

// Determine u and z range for given pick data (presumably coming from stroke pick). Save in strokePick array.
void AppendStrokePick
(
bvector<StrokePick> &data,
uint32_t axisSelect,
bvector<DPoint3d> const &points,
size_t readIndex
);

// analyze overlaps among picks to set visibility flags isHidden and isVisible.
GEOMDLLIMPEXP void SetVisibilityBits (bvector<StrokePick> &picks);

private:
PolyfaceVisitorPtr m_visitor;
DMatrix4d m_worldToLocal;
double m_shiftFactor;



public:
//! itemization of cause for return from searches.
enum class SearchState
    {
    Complete,
    SuspendByMaxTest,
    SuspendByMaxNewHit
    };

//! Initialize for search
GEOMDLLIMPEXP PolyfacePolygonPicker (PolyfaceQueryCR polyface, DMatrix4dCR worldToLocal);
//! Reset for further search.
GEOMDLLIMPEXP void Reset ();
GEOMDLLIMPEXP SearchState AppendHitsByStroke
(
DPoint3dCR strokeStart,             //!< [in] local coordinates of stroke start
DPoint3dCR strokeEnd,               //< [in] local coordinates of stroke end
bvector<size_t> *facets,            //!< [inout] growing array of readIndices of facets touched by the edge.
bvector<StrokePick> *strokePicks = nullptr,   // [inout] growing array of readIndices and sort data for facets touched by the edge
size_t maxTest = SIZE_MAX,          //!< [in] maximum number of new facets to test
size_t maxNewHit = SIZE_MAX         //!< [in] maximum number of new hits to record
);

GEOMDLLIMPEXP SearchState AppendHitsByBox
(
DRange2dCR pickBox,                //!< [in] local coordinates of pick box
bvector<size_t> *allIn,            //!< [inout] (optional) growing array of readIndices of facets completely inside
bvector<size_t> *allOut,            //!< [inout] (optional) growing array of readIndices of facets completely outside
bvector<size_t> *crossing,            //!< [inout] (optional) growing array of readIndices of facets that have both inside and outside portions.
size_t maxTest = SIZE_MAX,          //!< [in] maximum number of new facets to test
size_t maxNewHit = SIZE_MAX         //!< [in] maximum number of new hits to record
);

//! Among the candidateReadIndex, select those that are visible
GEOMDLLIMPEXP void SelectVisibleFacets
(
bvector<size_t> *candidateReadIndex,  //!< [inout] (optional) read indices of candidates.  This will be sorted !!!
bvector<size_t> &visibleReadIndex     //!< [out] read indices of visible candidates.
);

};





struct TaggedPolygonVectorIndexedRangeHeap
{
BENTLEY_GEOMETRY_INTERNAL_NAMESPACE_NAME::IndexedRangeHeap m_heap;
GEOMDLLIMPEXP TaggedPolygonVectorIndexedRangeHeap (TaggedPolygonVectorCR polygons, bool sortX, bool sortY, bool sortZ);
GEOMDLLIMPEXP void Search (DRange3dCR range, bvector<size_t> &indices);

};

/// context for multiple polyface serach operations (drape and facet search)
/// The context maintains a range tree for fast selection of candidate facets.
///
struct PolyfaceSearchContext
{
private:

PolyfaceHeaderPtr m_mesh;
PolyfaceIndexedHeapRangeTreePtr m_rangeTree;
PolyfaceVisitorPtr m_visitor;

void DoDrapeXY_recurse
(
CurveVectorCR curves,
size_t &segmentCounter,
bvector<DrapeSegment> &workDrapeSegments,
bvector<DSegment3dSizeSize> &imprintSegments
);

void DoDrapeXY_go
(
DSegment3dCR segment,
size_t tagA,
bvector<DrapeSegment> &workDrapeSegments,
bvector<DSegment3dSizeSize> &imprintSegments
);

public:
/// Capture a reference to a mesh.
GEOMDLLIMPEXP PolyfaceSearchContext (PolyfaceHeaderPtr &mesh, bool sortX, bool sortY, bool sortZ);

/// compute drape for a single line segment.
/// the drape results are deposited in the m_segments vector.  Repeated calls will reuse the memory allocations.
GEOMDLLIMPEXP void DoDrapeXY (DSegment3dCR segment, bvector<DrapeSegment> &drapeSegments);

/// compute drape for a linestring. Return the results as DSegment3dSizeSize pairs with (tagA, tagB) = (linesegment index, facet index)
GEOMDLLIMPEXP void DoDrapeXY (bvector<DPoint3d> const &linestring, bvector<DSegment3dSizeSize> &drapeSegments);

/// compute drape for a multiple segments (e.g. from prior merges or facet extractions). Return the results as DSegment3dSizeSize pairs with (tagA, tagB) = (source index, facet index)
GEOMDLLIMPEXP void DoDrapeXY (bvector<DSegment3dSizeSize> const &sourceSegment, bvector<DSegment3dSizeSize> &drapeSegments);

/// compute drape for all linestrings in the curve vector. All other curves are ignored.  Return the results as DSegment3dSizeSize pairs with (tagA, tagB) = (linestringIndex, facet index)
/// (The segment index is counted sequentially within the curve vector.)
GEOMDLLIMPEXP void DoDrapeXY (CurveVectorCR curves, bvector<DSegment3dSizeSize> &drapeSegments);


/// find facets under a space point.
/// the drape results are deposited in the m_facetPoints .  (Repeated calls will reuse the memory allocations)
GEOMDLLIMPEXP void SelectFacetsXY (DPoint3dCR spacePoint, size_t pointIndex, bvector<DPoint3dSizeSize> &pickPoints);

};

/*__PUBLISH_SECTION_START__*/

//! @description Helper object to assist visiting each face of a polyface mesh.
//!  Access queries for header of polyface mesh.
//! A PolyfaceVisitor is a PolyfaceHeader which at any time contains (only) one face of a client Polyface.
//!  The various indexed, flat array, and bvector members are the the "current" face of the traversal.
//!  These are "like" the face as found in the full header except for:
//! <ul>
//! <li>The various vertex data (point, normal, parameter, colors) for the n vertices of the face
//!        are found starting at index 0 in the respective arrays.
//! <li> There is an additional array (Visible) indicating the visibility of outgoing edge
//!        at the vertex.
//! <li> Data is optionally "wrapped" with copies of leading points
//! <li> Index arrays are ZERO BASED indices into the arrays of the parent PolyfaceHeader
//! </ul>
//! @ingroup BentleyGeom_Traversal
//!
struct PolyfaceVisitor : public RefCountedBase, protected PolyfaceVectors
{
friend struct PolyfaceHeader;
friend struct PolyfaceQuery;
protected:

friend struct PolyfaceCoordinateAverageContext;
GEOMAPI_VIRTUAL void             _Reset () = 0;
GEOMAPI_VIRTUAL bool             _AdvanceToNextFace () = 0;
GEOMAPI_VIRTUAL bool             _MoveToFacetByReadIndex (size_t readIndex) = 0;
GEOMAPI_VIRTUAL size_t           _GetReadIndex () const = 0;
GEOMAPI_VIRTUAL PolyfaceQueryCR  _GetClientPolyfaceQueryCR () const = 0;
GEOMAPI_VIRTUAL bool _AddVertexByReadIndex (size_t readIndex);

// To be established by each implementation during AdvanceToNextFace ....
    uint32_t            m_numEdgesThisFace;
    uint32_t            m_numWrap;
    bvector<BoolTypeForVector>       m_visible;
    bvector<size_t>     m_indexPosition;
    bool                m_allData;

protected:
PolyfaceVisitor (uint32_t numWrap = 0);
public:
//! Get pointer to contiguous points.
GEOMDLLIMPEXP DPoint3dCP                        GetPointCP () const;
//! Get pointer to contiguous normals.
GEOMDLLIMPEXP DVec3dCP                          GetNormalCP () const;
//! Get pointer to contiguous params
GEOMDLLIMPEXP DPoint2dCP                        GetParamCP () const;
//! Get pointer to contiguous integer colors
GEOMDLLIMPEXP uint32_t const*                   GetIntColorCP () const;
//! Get pointer to contiguous FaceData structs
GEOMDLLIMPEXP FacetFaceDataCP                   GetFaceDataCP () const;
//! Query two-sided flag
GEOMDLLIMPEXP bool                              GetTwoSided () const;
//! Query number of indices per face block.  0 or 1 means 0-terminated variable size blocks.
GEOMDLLIMPEXP uint32_t                          GetNumPerFace () const;
//! Query the mesh style
GEOMDLLIMPEXP uint32_t                          GetMeshStyle () const;
//! Get reference to the Point array with blocking data.
GEOMDLLIMPEXP BlockedVectorDPoint3dR            Point ();
//! Get reference to the param array with blocking data.
GEOMDLLIMPEXP BlockedVectorDPoint2dR            Param ();
//! Get reference to the normal array with blocking data.
GEOMDLLIMPEXP BlockedVectorDVec3dR              Normal();
//! Get reference to the integer color array with blocking data.
GEOMDLLIMPEXP BlockedVectorUInt32R              IntColor ();
// Get auxiliary data.
GEOMDLLIMPEXP PolyfaceAuxDataCPtr               GetAuxDataCP() const;

//! The client indices are zero-based indices into the client mesh data.
//! Get reference to the blocked array of zero-based indices into client mesh points.
GEOMDLLIMPEXP BlockedVectorIntR                 ClientPointIndex  ();
//! Get reference to the blocked array of zero-based indices into client mesh params.
GEOMDLLIMPEXP BlockedVectorIntR                 ClientParamIndex  ();
//! Get reference to the blocked array of zero-based indices into client mesh normals.
GEOMDLLIMPEXP BlockedVectorIntR                 ClientNormalIndex ();
//! Get reference to the blocked array of zero-based indices into client mesh colors
GEOMDLLIMPEXP BlockedVectorIntR                 ClientColorIndex  ();
//! Get reference to the blocked array of zero-based indices into client mesh faces.
GEOMDLLIMPEXP BlockedVectorIntR                 ClientFaceIndex  ();

//! Get reference to the contiguous array of zero-based indices into client mesh points.
GEOMDLLIMPEXP int32_t const*                    GetClientPointIndexCP () const;
//! Get reference to the contiguous array of zero-based indices into client mesh colors.
GEOMDLLIMPEXP int32_t const*                    GetClientColorIndexCP() const;
//! Get reference to the contiguous array of zero-based indices into client mesh params.
GEOMDLLIMPEXP int32_t const*                    GetClientParamIndexCP() const;
//! Get reference to the contiguous array of zero-based indices into client mesh normals.
GEOMDLLIMPEXP int32_t const*                    GetClientNormalIndexCP() const;
//! Get reference to the contiguous array of zero-based indices into client mesh faces.
GEOMDLLIMPEXP int32_t const*                    GetClientFaceIndexCP() const;
//! Get reference to the contiguous array of zero-based indices into auxilliaryData.
GEOMDLLIMPEXP int32_t const*                    GetClientAuxIndexCP() const;

//! access zero-based point index and visibility flag for an vertex within the current face.
GEOMDLLIMPEXP bool TryGetClientZeroBasedPointIndex (int zeroBasedVisitorIndex, int &zeroBasedIndex, bool &visible);

//! access zero-based normal index for an vertex within the curent face.
GEOMDLLIMPEXP bool TryGetClientZeroBasedNormalIndex (int zeroBasedVisitorIndex, int &zeroBasedIndex);

//! access zero-based param index for an vertex within the curent face.
GEOMDLLIMPEXP bool TryGetClientZeroBasedParamIndex (int zeroBasedVisitorIndex, int &zeroBasedIndex);

//! access zero-based color index for an vertex within the curent face.
GEOMDLLIMPEXP bool TryGetClientZeroBasedColorIndex (int zeroBasedVisitorIndex, int &zeroBasedIndex);


//! Reset to beginning of attached mesh, i.e. to read facets again.
GEOMDLLIMPEXP void      Reset ();
//! Read the next face from the attached mesh.
//! return false if all faces have been visited.
GEOMDLLIMPEXP bool      AdvanceToNextFace ();

//! Call AdvanceToNextFace repeatedly, returning when a face is found within tolerance of search point.
//! @param [in] xyz search point
//! @param [in] tolerance proximity tolerance.
//! @param [out] facetPoint nearest point on facet
//!
//! return false if all faces have been visited.
GEOMDLLIMPEXP bool AdvanceToFacetBySearchPoint (DPoint3dCR xyz, double tolerance, DPoint3dR facetPoint);

//! Call AdvanceToNextFace repeatedly, returning when a face is found within tolerance of search point.
//! @param [in] xyz search point
//! @param [in] tolerance proximity tolerance.
//! @param [out] facetPoint nearest point on facet
//! @param [out] edgeIndex if facetPoint is a true interior point, edgeIndex is -1.
//!      If facetPoint is a vertex, edgeFraction is 0.0 and edgeIndex is the index of the vertex in the visitor arrays
//!      If facetPoint is along an edge, edgeFraction is the fractional position and edgeIndex is the index of the base vertex in the visitor arrays.
//! @param [out] edgeFraction see edgeIndex.
//!
//! return false if all faces have been visited.
GEOMDLLIMPEXP bool AdvanceToFacetBySearchPoint (DPoint3dCR xyz, double tolerance, DPoint3dR facetPoint, ptrdiff_t &edgeIndex, double &edgeFraction);


//! Call AdvanceToNextFace repeatedly, returning when a face is found within tolerance of a pick ray.
//! @param [in] ray ray
//! @param [in] tolerance proximity tolerance.
//! @param [out] facetPoint pierce point on facet.
//! @param [out] rayFraction parameter along ray.
//! return false if all faces have been visited.
GEOMDLLIMPEXP bool AdvanceToFacetBySearchRay (DRay3dCR ray, double tolerance, DPoint3dR facetPoint, double &rayFraction);

//! Call AdvanceToNextFace repeatedly, returning when a face is found within tolerance of search point.
//! @param [in] ray ray
//! @param [in] tolerance proximity tolerance.
//! @param [out] facetPoint pierce point on facet.
//! @param [out] rayFraction fraction along search ray.
//! @param [out] edgeIndex if facetPoint is a true interior point, edgeIndex is -1.
//!      If facetPoint is a vertex, edgeFraction is 0.0 and edgeIndex is the index of the vertex in the visitor arrays
//!      If facetPoint is along an edge, edgeFraction is the fractional position and edgeIndex is the index of the base vertex in the visitor arrays.
//! @param [out] edgePoint closest point on edge
//! @param [out] edgeFraction see edgeIndex.
//! @param [out] edgeDistance distance from ray to closest edge.
//! return false if all faces have been visited.
GEOMDLLIMPEXP bool AdvanceToFacetBySearchRay (DRay3d ray, double tolerance, DPoint3dR facetPoint, double &rayFraction, ptrdiff_t &edgeIndex, double &edgeFraction, DPoint3dR edgePoint, double &edgeDistance);

//! Call AdvanceToNextFace repeatedly, returning when a face is found within tolerance of search point.
//! @param [in] ray ray
//! @param [out] detail facet hit detail.
GEOMDLLIMPEXP bool AdvanceToFacetBySearchRay (DRay3dCR ray, FacetLocationDetail& detail);
//! Try to locate facet edges before and after a specified param in a facet.
//! returns false if the facet does not have params or if there are not edges on both sides along the scan lines.
//! @param [in] uvParam pick parameter.
//! @param [in] horizontalScanBracket edge crossing data to left and right of uv.
//! @param [in] verticalScanBracket edge crossign data below and above.
GEOMDLLIMPEXP bool TryParamToScanBrackets
    (
    DPoint2dCR uvParam,
    FacetLocationDetailPairP horizontalScanBracket,
    FacetLocationDetailPairP verticalScanBracket
    );

//! Clear all arrays in the visitor.
GEOMDLLIMPEXP void ClearAllArrays ();
//! Copy all data from a particular vertex (indexed within the visitor)
//! into a facet location detail.
//! @param [out] detail destination for copied data.
//! @param [in] index index within data arrays for the visitor.
GEOMDLLIMPEXP bool LoadVertexData (FacetLocationDetailR detail, size_t index);
GEOMDLLIMPEXP bool LoadCyclicVertexData(FacetLocationDetailR detail, size_t index);
//! add coordinate data from a vertex described by a facet location detail.
GEOMDLLIMPEXP bool AddCoordinatesFromFacetLocationDetail(FacetLocationDetailCR detail);

//! add coordinate data from a vertex at an index in another visitor.
//! Only point, param, normal, colorTable, and color data are copied if available
//! (visible and indices are not copied)
//! Return false if index is out of range for the source visitor.  Array sizes may be irregular.
GEOMDLLIMPEXP bool AddCoordinatesFromVisitor(PolyfaceVisitorCR source, size_t sourceIndex);
//! Find a uv location within the facet.  Compute all available data there.
//! returns false if the facet does not have params or if there are not edges on both sides along the scan lines.
//! @param [in] uvParam pick parameter.
//! @param [in] detail all coordinate data at this parametric coordinate.
GEOMDLLIMPEXP bool TryParamToFacetLocationDetail (DPoint2d uvParam, FacetLocationDetailR detail);

// Compute point and normal at uvParam wrt the triangle on (locally numbered) vertices 0,i1,(i1+1),
// Return an invalid ValidatedDRay3d if i0,i1,i2 not a valid triangle with non-colinear points
GEOMDLLIMPEXP ValidatedDRay3d TryTriangleParamToPerpendicularRay
(
DPoint2d uvParam,   //!< [in] fractional coordinates along vector from [0] to [i1] and [0] to [i1+1].
size_t i1           //!< [in] index of first target point.
) const;
//! Find a uv location within the facet.  Compute all available data there.
//! returns false if the facet does not have params or if there are not edges on both sides along the scan lines.
//! @param [in] ray ray to intersect with facet.
//! @param [in] detail all coordinate data at this parametric coordinate.
GEOMDLLIMPEXP bool TryDRay3dIntersectionToFacetLocationDetail (DRay3dCR ray, FacetLocationDetailR detail);

//! interpolate all possible data along an edge of the current facet.
GEOMDLLIMPEXP bool IntepolateDataOnEdge (FacetLocationDetailR detail, size_t vertexIndex, double edgeFraction = 0.0, double a = 0.0);

//! accumualted a (multiple of) all numeric data to a detail.  Copy integer color and table number unchanged.
GEOMDLLIMPEXP bool AccumulateScaledData (FacetLocationDetailR detail, size_t vertexIndex, double fraction);

//! Interogate the xy coordinates (NOT THE STORED Param() ARRAY !!!) to determine
//! a local coordinate frame for the current facet.
//! This is the same logic used for CurveVector::CloneInLocalCoordinates and
//!   PolygonOps::CoordinateFrame.  That is:
//!<ul>
//!<li>The prefered x axis direction is parallel to the first edge.
//!<li>The prefered z direction is the outward normal of the xyz loop with CCW direction.
//!<li>The selector parameter chooses among 4 options:
//!   <ul>
//!   <li>LOCAL_COORDINATE_SCALE_UnitAxesAtStart -- origin at first point (even if not lower left!!), local axes have unit length, so
//!        local coordinates are real distances.
//!   <li>LOCAL_COORDINATE_SCALE_UnitAxesAtLowerLeft -- origin at lower left.  All xy local coordinates are 0 or positive, and local coordinates
//!         are real distances along the local directions
//!   <li>LOCAL_COORDINATE_SCALE_01RangeBothAxes -- Shift to lower left and scale to both directions go 0 to 1
//!   <li>LOCAL_COORDINATE_SCALE_01RangeBothAxes -- Shift to lower left and scale so one direction goes 0 to 1, the other direction has same scale and
//!         its largest coordinate  is positive and 1 or less.
//!   </ul>
//!</ul>
//! Prefered x axis is parallel to the first edge.
//! @param [out] localToWorld transform from local to world
//! @param [out] worldToLocal transform from world to local
//! @param [in] selector indicates preference for origin and scaling.
GEOMDLLIMPEXP bool TryGetLocalFrame
(
TransformR localToWorld,
TransformR worldToLocal,
LocalCoordinateSelect selector = LOCAL_COORDINATE_SCALE_01RangeBothAxes
);

//! Return the readIndex (current position within client facets).  This value can be used to return
//!    here via MoveToFacetByReadIndex
//! @return readIndex in facet.
GEOMDLLIMPEXP size_t GetReadIndex () const;
//! Focus on facet with specified readIndex.
//! @param [in] readIndex readIndex in facet.
GEOMDLLIMPEXP bool MoveToFacetByReadIndex (size_t readIndex);


//! return the number of edges on the current face.
GEOMDLLIMPEXP uint32_t  NumEdgesThisFace() const;
//! return the number or wraparound vertices that are added to the arrays.
GEOMDLLIMPEXP uint32_t  GetNumWrap () const;
//! Set the number of wraparound vertices to be added to faces when they are read.
GEOMDLLIMPEXP void      SetNumWrap (uint32_t numWrap);
//! return the (reference to) the array of per-edge visibility flags.
GEOMDLLIMPEXP bvector<BoolTypeForVector> &Visible ();
//!  return the (reference to) the array indicating where vertex indices were read from the attached mesh.
GEOMDLLIMPEXP bvector<size_t>& IndexPosition ();
//!  Save data for traversing the given parentMesh
//! If allData is false, only load coordinate (Point()) array.
//! Note that the default numWrap is zero.
GEOMDLLIMPEXP static PolyfaceVisitorPtr Attach (PolyfaceQueryCR  parentMesh, bool allData = true);
//!  As with Attach, create a visitor to iterate facets in parentMesh.
//!  Immediately do SetNumWrap(numWrap)
GEOMDLLIMPEXP static PolyfaceVisitorPtr AttachWithWrap(PolyfaceQueryCR parentMesh, bool allData, uint32_t numWrap);
//!  return reference to the attached mesh as a PolyfaceQuery
GEOMDLLIMPEXP PolyfaceQueryCR GetClientPolyfaceQueryCR () const;

//! Copy all data from one index to another.
GEOMDLLIMPEXP void CopyData (size_t fromIndex, size_t toIndex);
//! Trim all arrays to newSize.
GEOMDLLIMPEXP void TrimData (size_t newSize);

//! Compress adjacent points (including last/first) within tolerance.
GEOMDLLIMPEXP void      CompressClosePoints (double tolerance);

// The following methods are used for manipulating the visitor face "pseudo in place", usually with the source and instance as the same object.
//     Manipulations copy and interpolate points "to the end" and then pack back to the beginning.
//! Copy face data (point, normal, param, color, visible) from specified index of source.
//! Active state is checked in receiver. Index is not checked.
//!
//! return the index of new point.
GEOMDLLIMPEXP size_t PushFaceData (PolyfaceVisitor &source, size_t i0);
//! In source, copy index data (point, normal, param, color) from specified index of source.
//! Active state is checked in receiver. Index is not checked.
GEOMDLLIMPEXP void PushIndexData (PolyfaceVisitor &source, size_t i0);

//! In source, interpolate between specified indices of face data.
//! Active state is checked in receiver. Index is not checked.
//!
//! return the index of new point.
GEOMDLLIMPEXP size_t PushInterpolatedFaceData (PolyfaceVisitor &source, size_t i0, double fraction, size_t i1, bool suppressVisibility = false);


//! Trim all face data arrays, retaining {count} values starting at {index0}
GEOMDLLIMPEXP void TrimFaceData (size_t index0, size_t count);

//! Test if ray pierces facet or come close to an edge or vertex.
//! @param [in] ray test ray.
//! @param [in] tolerance tolerance for edge/vertex passby
//! @param [out] facetPoint point on facet.
//! @param [out] rayFraction parameter along ray
//!
//! return true if a hit was found.
GEOMDLLIMPEXP bool TryFindFacetRayIntersection (DRay3dCR ray, double tolerance, DPoint3dR facetPoint, double &rayFraction) const;

//! Test if a point is close to a facet.
//! @param [in] spacePoint test point.
//! @param [in] tolerance tolerance for identifying a hit.
//! @param [out] facetPoint point on facet.
//!
//! return true if a hit was found.
GEOMDLLIMPEXP bool TryFindCloseFacetPoint (DPoint3dCR spacePoint, double tolerance, DPoint3dR facetPoint) const;

//! return true if centroid, area, and normal can be calculated for the current facet.
GEOMDLLIMPEXP bool TryGetFacetCentroidNormalAndArea (DPoint3dR centroid, DVec3dR normal, double &area) const;
//! return true if products of inertia for an AREA integral over the facet can be computed.
GEOMDLLIMPEXP bool TryGetFacetAreaMomentProducts (DPoint3dCR origin, DMatrix4dR products) const;
//! interpolate a point on an edge.  Edge index is interpretted cyclically (within the current face)
GEOMDLLIMPEXP bool TryGetEdgePoint (size_t edgeIndex, double f, DPoint3dR xyz) const;
//! get a normalized (0-1) parameter at an index within the current facet
GEOMDLLIMPEXP bool TryGetNormalizedParameter (size_t index, DPoint2dR normalizedParam)  const;
//! get a distance based parameter at an index within the current facet
GEOMDLLIMPEXP bool TryGetDistanceParameter (size_t index, DPoint2dR distanceParam) const;
//! gather all read indices.
GEOMDLLIMPEXP void CollectReadIndices (bvector<size_t> &indices);

//! Clear all arrays in the visitor.   This is used before AddVertexByReadIndex.
GEOMDLLIMPEXP void ClearFacet ();
//! Go to the source mesh at specified readIndex.  Bring all it's data into a new vertex in the visitor.
//! @return false if not a valid readIndex for the client array, or if the client array has a zero (terminator) there, or if the client is not fully indexed.
GEOMDLLIMPEXP bool TryAddVertexByReadIndex (size_t readIndex);
//! Recompute the (coordinate) normal data based on the point coordinates.
GEOMDLLIMPEXP bool TryRecomputeNormals ();

};

/*=================================================================================**//**
* Filter class for selecting facets presented by a visitor.
+===============+===============+===============+===============+===============+======*/
//! Interface with callbacks to test PolyfaceVisitor contents.
struct IPolyfaceVisitorFilter
{
// Function to test a face.
GEOMAPI_VIRTUAL bool TestFacet (PolyfaceVisitorCR visitor) {return true;}
};
//! Object to accept/reject the visitor facet based on orientation of the facet normal.
struct PolyfaceVisitorNormalOrientationFilter : IPolyfaceVisitorFilter
{
double m_normalTolerance;
bool m_acceptNegative;
bool m_acceptSide;
bool m_acceptPositive;
DVec3d m_positiveDirectionVector;
//! Tester constructor.
//! default positive direction vector is (001)
GEOMDLLIMPEXP PolyfaceVisitorNormalOrientationFilter
(
bool acceptPositive = true,     //!< [in] true to accept facets with positive normal direction
bool acceptNegative = false,    //!< [in] true to accept faces with negative normal direction.
bool acceptSide = false,        //!< [in] true to accept facets with side normal direction.
double normalTolerance = 1.0e-8 //!< [in] tolerance for normal test.
);
//! Set the positive direction vector.
//! @return false if vector is 000
GEOMDLLIMPEXP bool SetPositiveDirectionVector (DVec3dCR vector);

//! Test the face.  Accept/Reject according to face normal dotted with the direction vector.
GEOMDLLIMPEXP bool TestFacet (PolyfaceVisitorCR visitor) override;
};

/*=================================================================================**//**
* Toleranced comparison class for map searches...
+===============+===============+===============+===============+===============+======*/
struct DPoint3dZYXTolerancedSortComparison
{
double m_absTol;
double m_relTol;
GEOMDLLIMPEXP DPoint3dZYXTolerancedSortComparison(double absTol, double relTol);
GEOMDLLIMPEXP bool operator() (const DPoint3d& pointA, const DPoint3d &pointB) const;
};

struct DVec3dZYXTolerancedSortComparison
{
double m_absTol;
double m_relTol;
GEOMDLLIMPEXP DVec3dZYXTolerancedSortComparison(double absTol, double relTol);
GEOMDLLIMPEXP bool operator() (const DVec3d& pointA, const DVec3d &pointB) const;
};

/*--------------------------------------------------------------------------------**//**
+--------------------------------------------------------------------------------------*/
struct PolyfaceZYXMap : bmap <DPoint3d, size_t, DPoint3dZYXTolerancedSortComparison>
{
PolyfaceZYXMap (DPoint3dZYXTolerancedSortComparison const &compare);
};
struct PolyfaceZYXDVec3dMap : bmap <DVec3d, size_t, DVec3dZYXTolerancedSortComparison>
{
PolyfaceZYXDVec3dMap (DVec3dZYXTolerancedSortComparison const &compare);
};


struct DPoint3dZYXSortComparison
    : public std::binary_function <DPoint3d, DPoint3d, bool>
{
bool operator () (const DPoint3d& pointA, const DPoint3d &pointB) const;
};

struct DVec3dZYXSortComparison
    : public std::binary_function <DVec3d, DVec3d, bool>
{
bool operator () (const DVec3d& vecA, const DVec3d &vecB) const;
};

struct DPoint2dYXSortComparison
    : public std::binary_function <DPoint2d, DPoint2d, bool>
{
bool operator () (const DPoint2d& pointA, const DPoint2d &pointB) const;
};

struct UInt32SortComparison
    : public std::binary_function <uint32_t, uint32_t, bool>
{
bool operator () (const uint32_t & pointA, const uint32_t &pointB) const;
};




//! Support class for constructing meshes.
//! Maintains lookup structures to assist indexing as points, parameters, normals are added to a mesh.
//! The client mesh is owned by the caller.
//! @remarks
//! <ul>
//! <li>Parameters in the PolyfaceCoordinateMap are xyz, with the z coordinate used as a face index.
//! <li>The coordinate map has a modal variable for the face index.
//! <li>Coordianates transfered to the PolyfaceHeader are "just" xy
//! <li>As params are added to the PolyfaceCoordinateMap, a current face range is updated.
//! </ul>
//! @ingroup BentleyGeom_Construction
//! @DotNetClassExclude
struct PolyfaceCoordinateMap: public RefCountedBase, NonCopyableClass
{
protected:
    PolyfaceHeader &m_polyface;
    struct PolyfaceZYXMap *m_pointMap;
    struct PolyfaceZYXDVec3dMap *m_normalMap;
    struct PolyfaceZYXMap * m_paramMap;
    bmap <uint32_t, size_t, UInt32SortComparison> m_intColorMap;

    double m_currentParamZ;
public:
//! Construct a new coordinate map, with caller supplied client mesh.
PolyfaceCoordinateMap (PolyfaceHeader &referencedPolyface);

//! Construct with specific tolerances for duplicate points and params.
//! @param [in] referencedPolyface client to receive facets.
//! @param [in] xyzAbsTol absolute tolerance for point comparisons.
//! @param [in] xyzRelTol relative tolerance for point comparisons.
//! @param [in] paramAbsTol absolute tolerance for param comparisons.
//! @param [in] paramRelTol relative tolerance for param comparisons.
//! @param [in] normalAbsTol absolute tolerance for normal comparisons.
//! @param [in] normalRelTol relative tolerance for normal comparisons.
PolyfaceCoordinateMap (PolyfaceHeader &referencedPolyface,
    double xyzAbsTol, double xyzRelTol,
    double paramAbsTol, double paramRelTol,
    double normalAbsTol = 1.0E-14, double normalRelTol = 1.0E-12

    );

~PolyfaceCoordinateMap ();
public:
//! Query the client mesh.
    GEOMDLLIMPEXP PolyfaceHeader& GetPolyfaceHeaderR ();

//! Clear both the client mesh and the coordinate indexing.
    GEOMDLLIMPEXP void ClearData ();
    //! Assign a z value that is attached parameter values entered into the map.
    //! Setting a different Z for facets from different faces ensures that parameters "with a face" get unique entries from numerically
    //!   identical parameters of other faces.
    GEOMDLLIMPEXP void SetCurrentParamZ (double data);
    //! Query the current z assigned to parameters in the mapping.
    GEOMDLLIMPEXP double GetCurrentParamZ () const;

    //! set range data to null range.
    GEOMDLLIMPEXP void ResetCurrentFaceData ();
//! (Find or) Add a point.  Return its (0 based) index.
    //! @param [in] point point coordinates.
    GEOMDLLIMPEXP size_t AddPoint  (DPoint3dCR point);
//! Find a point by coordinates.
    //! return true if this point is already present.
    //! @param [in] point coordinates to look up.
    //! @param [out] index 0-based index within the client mesh.
    GEOMDLLIMPEXP bool   FindPoint (DPoint3dCR point, size_t &index);

//! (Find or) Add a normal.  Return its (0 based) index.
    //! @param [in] normal normal coordinates.
    GEOMDLLIMPEXP size_t AddNormal  (DVec3dCR normal);
    //! Find a normal by coordinates.
    //! return true if this normal is already present.
    //! @param [in] normal coordinates to look up.
    //! @param [out] index 0-based index within the client mesh.
    GEOMDLLIMPEXP bool   FindNormal (DVec3dCR normal, size_t &index);

//! (Find or) Add a param in the current face.  Return its (0 based) index.
    //! @param [in] param normal coordinates.
    GEOMDLLIMPEXP size_t AddParam  (DPoint2dCR param);
//! Find a param by coordinates, assuming current face.
    //! return true if this param is already present.
    //! @param [in] param coordinates to look up.
    //! @param [out] index 0-based index within the client mesh.
    GEOMDLLIMPEXP bool   FindParam (DPoint2dCR param, size_t &index);

//! (Find or) Add a color.  Return its (0 based) index.
    //! @param [in] color normal coordinates.
    GEOMDLLIMPEXP size_t AddIntColor  (uint32_t color);
//! Find a color by coordinates.
    //! return true if this color is already present.
    //! @param [in] color coordinates to look up.
    //! @param [out] index 0-based index within the client mesh.
    GEOMDLLIMPEXP bool   FindIntColor (uint32_t color, size_t &index);

//! Try to dereference a point by index.
    GEOMDLLIMPEXP bool TryGetPoint (size_t index, DPoint3dR xyz) const;

//! DEPRECATED -- use Create(polyface)
    static GEOMDLLIMPEXP PolyfaceCoordinateMapPtr New (PolyfaceHeaderR polyface);
//! Return a (ref counted !!) PolyfaceCoordinateMapPtr
    //! bound to the referenced PolyfaceVectors
    static GEOMDLLIMPEXP PolyfaceCoordinateMapPtr Create (PolyfaceHeaderR polyface);

//! Add a polygon, generating indices for each datum.
    //! If normals, params, and colors are supplied, their inidces are
    //!    added to the respective index array.
    void GEOMDLLIMPEXP AddPolygon (int n,
        DPoint3dCP      points,
        DVec3dCP        normals = NULL,
        DPoint2dCP      params = NULL,
        int *           intColor = NULL
        );
//! Add a polygon, generating indices for each datum.
    //! If normals, params, and colors are supplied, their inidces are
    //!    added to the respective index array.
    void GEOMDLLIMPEXP AddPolygon (bvector<DPoint3d> const &points,
        bvector<DVec3d> const *normals = nullptr,
        bvector<DPoint2d> const *params = nullptr,
        bvector<int> const *intColor = nullptr
        );



//! Add a polygon, using a (subset of !!!) data from visitor.
    //! @param[in]  source  source data
    //! @param[in]  i0      start index of face in visitor.
    //! @param[in]  n       vertex count in visitor.
    GEOMDLLIMPEXP void AddVisitorPartialFace (PolyfaceVisitor &source, size_t i0, size_t n);

//! Add a polygon, using data from a visitor.
    //! @param[in]  source  source data
    GEOMDLLIMPEXP void AddVisitorFace (PolyfaceVisitor &source);


//! Visit each face of source. Clip to chain and capture the clipped residual.
//! @param [in] source input mesh
//! @param [out] insideDest inside mesh.  (REQUIRED)
//! @param [out] outsideDest outside mesh (OPTIONAL)
//! @param [out] resultHasIncompleteCutPlanefaces indicates that edges on one or more cut planes could not be assembed into loops.
//! @param [in] clipPlanes chain of convex volumes
//! @param [in] formNewFacesOnClipPlanes true to attempt reassembling faces on clip planes.
//! @param [in] filter for mesh
//! @param [out] cutLoops optional coordinate data for cut loops
//! @param [out] cutChains optional coordinate data for cut chains
    GEOMDLLIMPEXP static void AddClippedPolyface (PolyfaceQueryR source,
        PolyfaceCoordinateMapP insideDest,
        PolyfaceCoordinateMapP outsideDest,
        bool &resultHasIncompleteCutPlanefaces,
        ClipPlaneSetP clipPlanes,
        bool formNewFacesOnClipPlanes = false,
        IPolyfaceVisitorFilter *filter = nullptr,
        bvector<bvector<DPoint3d>> *cutLoops = nullptr,
        bvector<bvector<DPoint3d>> *cutChains = nullptr
        );
};
/*__PUBLISH_SECTION_END__*/

typedef RefCountedPtr<struct PolyfaceCoordinateAverageContext>  PolyfaceCoordinateAverageContextPtr;

//! A polyfaceCoordinateAverager supports arrays of points and other data, with counters to allow averaging of the associated data.
struct PolyfaceCoordinateAverageContext: public RefCountedBase, NonCopyableClass
{
private:
PolyfaceZYXMap m_map;     // point lookup index.
size_t m_numPoints;       // total number of points (matches m_map.size ())
bvector<int> m_numIncident;         // m_numIncident[i] = number of times m_averages.Point ()[i] has been seen.
bvector<DPoint2d> m_param;
bvector<DVec3d> m_normal;
bvector<DPoint4d> m_doubleColor;    // xyzw are the 4 bytes of int colors.
bvector<int>   m_intColor;      // IntColor cannot be averaged -- " Last one wins"

PolyfaceCoordinateAverageContext ();

public:
//! Create a new context.
GEOMDLLIMPEXP static PolyfaceCoordinateAverageContextPtr Create ();

//! Announce a facet whose vertices are to (all) appear in the averages.
//! The world to local transform is only applied to the xyz in the points.
GEOMDLLIMPEXP void AnnounceFacet (PolyfaceVisitorR visitor, TransformCP worldToLocal = nullptr);
//! look up a point.
GEOMDLLIMPEXP ValidatedSize FindPoint (DPoint3dCR xyz, TransformCP worldToLocal = nullptr);
//! return the averaged parameter.
GEOMDLLIMPEXP DPoint2d GetAverageParam (size_t index) const;
//! return the averaged parameter.
GEOMDLLIMPEXP DVec3d GetAverageNormal (size_t index) const;
//! return the (last) intColor.
GEOMDLLIMPEXP int GetIntColor (size_t index) const;
//! return the average int color (byte-by-byte summed and averaged independently)
GEOMDLLIMPEXP int GetAverageIntColor (size_t index) const;
//! Load the xyz, uv, normal and color arrays of a visitor.
//! Return false if any input point lookup fails.
//! All other vistor content is left unchanged. (Caveat programmat)
GEOMDLLIMPEXP bool LoadVisitor
(
PolyfaceVisitorR visitor,   //!< Visitor whose Active() flags match the source and destination data.
bvector<DPoint3d> const &xyz
);
};

struct IFacetProcessor
{
GEOMAPI_VIRTUAL void Process (PolyfaceVectors &) = 0;
};

typedef RefCountedPtr<struct LightweightPolyfaceBuilder>  LightweightPolyfaceBuilderPtr;

struct LightweightPolyfaceBuilder : RefCountedBase
{
    struct QPoint3d
        {
        int64_t     m_x, m_y, m_z;

        QPoint3d() { }
        QPoint3d(DPoint3dCR point, double tol);
        QPoint3d(DVec3dCR vector, double tol);
        bool operator < (struct QPoint3d const& rhs) const;
        };
    struct QPoint2d
        {
        int64_t     m_x, m_y, m_z;

        QPoint2d() { }
        QPoint2d(DPoint2dCR point, double tol);
        bool operator < (struct QPoint2d const& rhs) const;
        };

private:
    bmap<QPoint3d, size_t>  m_pointMap;
    bmap<QPoint3d, size_t>  m_normalMap;
    bmap<QPoint2d, size_t>  m_paramMap;
    PolyfaceHeaderPtr       m_polyface;
    double                  m_pointTolerance;
    double                  m_normalTolerance;
    double                  m_paramTolerance;
    FacetFaceData           m_currentFaceData;


    LightweightPolyfaceBuilder(PolyfaceHeaderR polyface, double pointTolerance, double normalTolerance, double paramTolerance);

public:
    GEOMDLLIMPEXP static LightweightPolyfaceBuilderPtr Create(PolyfaceHeaderR polyface, double pointTolerance = 1.0E-8, double normalTolerance = 1.0E-10, double paramTolerance = 1.0E-10);

    //! Find or add a point.  Return the (0-based) index.
    GEOMDLLIMPEXP size_t FindOrAddPoint (DPoint3dCR point);

    //! Find or add a normal.  Return the (0-based) index.
    GEOMDLLIMPEXP size_t FindOrAddNormal (DVec3dCR normal);

    //! Find or add a param.  Return the (0-based) index.
    GEOMDLLIMPEXP size_t FindOrAddParam (DPoint2dCR param);


    //! Add a point index, adjusted to 1-based indexing with visibility in sign.
    GEOMDLLIMPEXP void AddPointIndex (size_t zeroBasedIndex, bool visible);

    //! Add a normal index, adjusted to 1-based indexing.
    GEOMDLLIMPEXP void AddNormalIndex (size_t zeroBasedIndex);

    //! Add a param index, adjusted to 1-based indexing
    GEOMDLLIMPEXP void AddParamIndex (size_t zeroBasedIndex);

    //! Add terminators.
    GEOMDLLIMPEXP void AddIndexTerminators();

    //! Add a terminator to the point index table.
    GEOMDLLIMPEXP void AddPointIndexTerminator ( );

    //! Add a terminator to the normal index table.
    GEOMDLLIMPEXP void AddNormalIndexTerminator ( );

    //! Add a terminator to the param index table.
    GEOMDLLIMPEXP void AddParamIndexTerminator ( );

    //! Return polyface
    PolyfaceHeaderPtr GetPolyface() { return m_polyface; }

    //! Set the current face data.
    GEOMDLLIMPEXP void SetFaceData (FacetFaceDataCR data);

    //! Finalize data for the current face.
    GEOMDLLIMPEXP void EndFace ();

    //! Add aux channel data at index.
    GEOMDLLIMPEXP void AddAuxDataByIndex (PolyfaceAuxData::ChannelsCR channels, size_t index);

};  //  LightweightPolyfaceBuilder


/*__PUBLISH_SECTION_START__*/
END_BENTLEY_GEOMETRY_NAMESPACE

#endif

/*----------------------------------------------------------------------+
|                                                                       |
|   Mesh Styles                                                         |
|                                                                       |
|   A mesh is stored as a number of matrix elements.  Particulars of    |
|   these arrays depend on the known structure of the mesh, as          |
|   signified by the (positive) mesh style:                             |
|                                                                       |
|   MESH_ELM_STYLE_INDEXED_FACE_LOOPS                                   |
|       The mesh consists of an array of vertex coordinates and an      |
|       array of indices into the vertex coordinate array.  Each face   |
|       appears as a row of indices in the index matrix.                |
|   MESH_ELM_STYLE_POINT_CLOUD                                          |
|       The mesh consists of a single array of (unblocked) coordinates. |
|       (No connectivity stored.)                                       |
|   MESH_ELM_STYLE_COORDINATE_TRIANGLES                                 |
|       The mesh consists of a single array of coordinates.  Each block |
|       of 3 points is a triangle.   (No connectivity stored.)          |
|   MESH_ELM_STYLE_COORDINATE_QUADS                                     |
|       The mesh consists of a single array of coordinates.  Each block |
|       of 4 points is a quad.  (No connectivity stored.)               |
|   MESH_ELM_STYLE_TRIANGLE_GRID                                        |
|       The mesh consists of a simple grid of points, to be             |
|       triangulated.  The triangulation convention is that within each |
|       quad of the grid, an edge is added from the lowest numbered     |
|       vertex to the highest, i.e., in a quad with indices i,i+1,j,j+1 |
|       (where j=i+numPerRow), an edge is added from i to j+1.          |
|   MESH_ELM_STYLE_QUAD_GRID                                            |
|       The mesh consists of a simple grid of points, to be formed into |
|       quads.                                                          |
|                                                                       |
+----------------------------------------------------------------------*/
#define MESH_ELM_STYLE_INDEXED_FACE_LOOPS       1
#define MESH_ELM_STYLE_POINT_CLOUD              2
#define MESH_ELM_STYLE_COORDINATE_TRIANGLES     3
#define MESH_ELM_STYLE_COORDINATE_QUADS         4
#define MESH_ELM_STYLE_TRIANGLE_GRID            5
#define MESH_ELM_STYLE_QUAD_GRID                6
#define MESH_ELM_STYLE_LARGE_MESH               7

/*----------------------------------------------------------------------+
|                                                                       |
|   Matrix Tags                                                         |
|                                                                       |
|   Each matrix element stores an identifier to distinguish its         |
|   contents.  These tags are application-specific (cf. matrix tags     |
|   for the mesh element API).  In those cases where a matrix tag       |
|   reference is irrelevant (e.g., in the indexedBy field of a data     |
|   matrix) use this constant.                                          |
|                                                                       |
+----------------------------------------------------------------------*/
#define MATRIX_ELM_TAG_NONE             0

/*----------------------------------------------------------------------+
|                                                                       |
|   Matrix Tags (Mesh parents)                                          |
|                                                                       |
|   A matrix element whose parent is a mesh element has one of these    |
|   (nonnegative) tags, to signify what the matrix data represents.     |
|   Matrices with MESH_ELM_TAG_*_INDICES tags are index matrices into   |
|   the associated data array.                                          |
|                                                                       |
+----------------------------------------------------------------------*/
#define MESH_ELM_TAG_NONE                               MATRIX_ELM_TAG_NONE

/* main data/index array */
#define MESH_ELM_TAG_FACE_LOOP_TO_VERTEX_INDICES        1
#define MESH_ELM_TAG_VERTEX_COORDINATES                 2

/* auxiliary data arrays */
#define MESH_ELM_TAG_NORMAL_COORDINATES                 4
#define MESH_ELM_TAG_UV_PARAMETERS                      6
/* each double color entry is an RGB color as 3 doubles in [0,1] */
//#define MESH_ELM_TAG_DOUBLE_COLOR                       8
/* each double color entry is an RGB color as 3 floats.
   Since there are no FLOAT dgn elements, this tag can only exist in
   in-memory copies of DOUBLE dgn elements.*/
//#define MESH_ELM_TAG_FLOAT_COLOR                        9
/* each int color entry is an RGB color as 1 int packed with 3 bytes in [0,255] */
#define MESH_ELM_TAG_INT_COLOR                          10
/* each table color entry is an index into a color table */
//#define MESH_ELM_TAG_TABLE_COLOR                        12
#define MESH_ELM_TAG_DENSITY                            14
#define MESH_ELM_TAG_PRESSURE                           16
#define MESH_ELM_TAG_TEMPERATURE                        18
#define MESH_ELM_TAG_DISPLACEMENT                       20
#define MESH_ELM_TAG_VELOCITY                           22

/* auxiliary index arrays: "by face loop" */
#define MESH_ELM_TAG_FACE_LOOP_TO_NORMAL_INDICES        3
#define MESH_ELM_TAG_FACE_LOOP_TO_UV_PARAMETER_INDICES  5
//#define MESH_ELM_TAG_FACE_LOOP_TO_DOUBLE_COLOR_INDICES  7
#define MESH_ELM_TAG_FACE_LOOP_TO_INT_COLOR_INDICES     9
//#define MESH_ELM_TAG_FACE_LOOP_TO_TABLE_COLOR_INDICES   11
#define MESH_ELM_TAG_FACE_LOOP_TO_DENSITY_INDICES       13
#define MESH_ELM_TAG_FACE_LOOP_TO_PRESSURE_INDICES      15
#define MESH_ELM_TAG_FACE_LOOP_TO_TEMPERATURE_INDICES   17
#define MESH_ELM_TAG_FACE_LOOP_TO_DISPLACEMENT_INDICES  19
#define MESH_ELM_TAG_FACE_LOOP_TO_VELOCITY_INDICES      21
#define MESH_ELM_TAG_FACE_LOOP_TO_FACE_INDICES          23

/* auxiliary index arrays: "by face" */
#define MESH_ELM_TAG_FACE_TO_NORMAL_INDICES             23
#define MESH_ELM_TAG_FACE_TO_UV_PARAMETER_INDICES       24
//#define MESH_ELM_TAG_FACE_TO_DOUBLE_COLOR_INDICES       25
#define MESH_ELM_TAG_FACE_TO_INT_COLOR_INDICES          26
//#define MESH_ELM_TAG_FACE_TO_TABLE_COLOR_INDICES        27
#define MESH_ELM_TAG_FACE_TO_DENSITY_INDICES            28
#define MESH_ELM_TAG_FACE_TO_PRESSURE_INDICES           29
#define MESH_ELM_TAG_FACE_TO_TEMPERATURE_INDICES        30
#define MESH_ELM_TAG_FACE_TO_DISPLACEMENT_INDICES       31
#define MESH_ELM_TAG_FACE_TO_VELOCITY_INDICES           32

/* auxiliary index arrays: "by vertex" */
#define MESH_ELM_TAG_VERTEX_TO_NORMAL_INDICES           33
#define MESH_ELM_TAG_VERTEX_TO_UV_PARAMETER_INDICES     34
//#define MESH_ELM_TAG_VERTEX_TO_DOUBLE_COLOR_INDICES     35
#define MESH_ELM_TAG_VERTEX_TO_INT_COLOR_INDICES        36
//#define MESH_ELM_TAG_VERTEX_TO_TABLE_COLOR_INDICES      37
#define MESH_ELM_TAG_VERTEX_TO_DENSITY_INDICES          38
#define MESH_ELM_TAG_VERTEX_TO_PRESSURE_INDICES         39
#define MESH_ELM_TAG_VERTEX_TO_TEMPERATURE_INDICES      40
#define MESH_ELM_TAG_VERTEX_TO_DISPLACEMENT_INDICES     41
#define MESH_ELM_TAG_VERTEX_TO_VELOCITY_INDICES         42

/*----------------------------------------------------------------------+
|                                                                       |
|   Matrix Index Families                                               |
|                                                                       |
|   The index family of a matrix element is used to distinguish         |
|   different kinds of index matrices.  These tags are application-     |
|   specific (cf. matrix index families in the mesh element API).  In   |
|   those cases where an index family reference is irrelevant (e.g., in |
|   the indexFamily field of a data matrix), use this constant.         |
|                                                                       |
+----------------------------------------------------------------------*/
#define MATRIX_ELM_INDEX_FAMILY_NONE    0

/*----------------------------------------------------------------------+
|                                                                       |
|   Matrix Index Families (Mesh parents)                                |
|                                                                       |
|   A matrix element whose parent is a mesh element has one of these    |
|   (nonpositive) index family tags to indicate how each row of the     |
|   matrix is to be interpreted.                                        |
|                                                                       |
|   MESH_ELM_INDEX_FAMILY_NONE                                          |
|       No row interpretation (as indices); default on data matrices.   |
|   MESH_ELM_INDEX_FAMILY_BY_FACE_LOOP                                  |
|       Row i consists of an ordered list of indices, one per vertex    |
|       (sector) of face i; default on index matrices.                  |
|   MESH_ELM_INDEX_FAMILY_BY_FACE                                       |
|       Row i consists of a single index for face i.                    |
|   MESH_ELM_INDEX_FAMILY_BY_VERTEX                                     |
|       Row i consists of a single index for vertex i.                  |
|                                                                       |
+----------------------------------------------------------------------*/
#define MESH_ELM_INDEX_FAMILY_NONE           MATRIX_ELM_INDEX_FAMILY_NONE
#define MESH_ELM_INDEX_FAMILY_BY_FACE_LOOP  -1
#define MESH_ELM_INDEX_FAMILY_BY_FACE       -2
#define MESH_ELM_INDEX_FAMILY_BY_VERTEX     -3

/*----------------------------------------------------------------------+
|                                                                       |
|   Matrix Int Data Transform Types                                     |
|                                                                       |
|   The following constants are used in de/enciphering the index type   |
|   code (also called transform type) in matrix int data elements:      |
|                                                                       |
|   MATRIX_DATA_ELM_INDEX_TYPE_BLOCK_NONE                               |
|       Data are not to be interpreted as indices.                      |
|   MATRIX_DATA_ELM_INDEX_TYPE_BLOCK_FIXED                              |
|       Fixed-length rows of indices, possibly right-padded with a      |
|       special value.                                                  |
|   MATRIX_DATA_ELM_INDEX_TYPE_BLOCK_VARIABLE                           |
|       Variable-length rows of indices, each terminated with a special |
|       value.                                                          |
|                                                                       |
+----------------------------------------------------------------------*/
#define MATRIX_DATA_ELM_INDEX_TYPE_BLOCK_NONE           0
#define MATRIX_DATA_ELM_INDEX_TYPE_BLOCK_FIXED          1
#define MATRIX_DATA_ELM_INDEX_TYPE_BLOCK_VARIABLE       2

/*----------------------------------------------------------------------+
|                                                                       |
|   Matrix Double Data Transform Types                                  |
|                                                                       |
|   The following constants are used in de/enciphering the transform    |
|   type code in matrix double data elements:                           |
|                                                                       |
|       Key: transform = [M t] = [Matrix translation]                   |
|            M-^ = inverse transpose                                    |
|            x = structure to transform                                 |
|                                                                       |
|   MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_NONE                           |
|       Data are not to be transformed.                                 |
|   MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_POINT                          |
|       Point data.  [M t]x = Mx + t.                                   |
|   MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_VECTOR                         |
|       Vector data.  [M t]x = Mx.                                      |
|   MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_COVECTOR                       |
|       Covector data (e.g., normals).  [M t]x = M-^x (this preserves   |
|       orthogonality of the data).                                     |
|                                                                       |
+----------------------------------------------------------------------*/
#define MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_NONE       0
#define MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_POINT      1
#define MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_VECTOR     2
#define MATRIX_DATA_ELM_TRANSFORM_TYPE_COORD_COVECTOR   3


