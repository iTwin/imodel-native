//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/IDTMFeatureArray.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : IDTMFeatureArray
//-----------------------------------------------------------------------------


#pragma once

#include "HPUCompositeArray.h"
#include "Stores/SMStoreUtils.h"
//#include "IDTMFileDirectories/FeatureHeaderTypes.h"

template <typename PointType, typename HeaderType> class IDTMFeatureArray;

/*---------------------------------------------------------------------------------**//**
* @description  A facade that provides consistent interface to a feature whose points
*               and header are separately stored in their respective array. This facade
*               access directly those array without any copying of the data.
*
* @see IDTMCompositeFacadeBaseWExtCopy
*
* @bsiclass                                                  Raymond.Gauthier   5/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType, typename HeaderType>
class IDTMFeatureFacade : public HPU::CompositeFacadeBaseWExtCopy<IDTMFeatureFacade<PointType, HeaderType>,
    HeaderType,
    IDTMFeatureArray<PointType, HeaderType>>
    {
public:
    typedef HeaderType                          header_type;
    typedef typename header_type::feature_type  feature_type;
    typedef typename header_type::group_id_type group_id_type;

    typedef PointType                       value_type;
    typedef value_type*                     iterator;
    typedef const value_type*               const_iterator;


    feature_type                            GetType                            () const;

    group_id_type                           GetGroupID                         () const;

    size_t                                  GetSize                            () const;

    const_iterator                          Begin                              () const;
    const_iterator                          End                                () const;

    bool                                    operator==                         (const IDTMFeatureFacade&    pi_rRight) const;

    template <typename InputIter>
    void                                    Insert                             (const_iterator          pi_Position,
                                                                                InputIter               pi_Begin,
                                                                                InputIter               pi_End);
    template <typename InputIter>
    void                                    Append                             (InputIter               pi_Begin,
                                                                                InputIter               pi_End);

    iterator                                Erase                              (const_iterator          pi_Begin,
                                                                                const_iterator          pi_End);

    iterator                                BeginEdit                          ();
    iterator                                EndEdit                            ();

    typedef HPU::Array<value_type>      PointArray;

    explicit                                IDTMFeatureFacade                  () {}

    void                                    OnArrayDataChanged                 ();

    static void                             InitHeader                         (header_type&                pio_rHeader,
                                                                                feature_type                pi_Type,
                                                                                size_t                      pi_Offset,
                                                                                size_t                      pi_Size = 0,
                                                                                group_id_type               pi_GroupID = header_type::GetNullID());


    const PointArray&                       GetPoints                          () const;
    PointArray&                             EditPoints                         ();

    bool  IsPartOfSameList (const typename HPU::CompositeFacadeBaseWExtCopy<IDTMFeatureFacade<PointType, HeaderType>,HeaderType,IDTMFeatureArray<PointType, HeaderType> >::facade_type&          pi_rRight) const;

    };


/*---------------------------------------------------------------------------------**//**
* @description  The array provide consistent interface to 2 underlying arrays which are:
*               feature headers array and feature points array. It actually refers to
*               virtual features which in fact, are facades to elements of the underlying
*               arrays. This enable us to avoid copying data to another form, keeping
*               locality of reference and providing secure access/edition of feature
*               data. This is a copy on write(COW) array, so that when you wrap on the data
*               of another array, this array only refers to the data of the wrapped
*               array (read only). When this array happens to write the data, all sub
*               arrays are reallocated/copied at another memory location so that this
*               array can edit data without corrupting wrapped array's data. After first
*               edit, array is now said to be owner of the data.
*
*               e.g:
*
*               IDTMFeatureArray<IDTM3dPt> MyFeatures(10, 50); -> Reserve space for at
*                                                                least 10 feature and
*                                                                50 points.
*
*               const IDTMFeatureType MyFeatureType1 = 5; -> See application documentation
*                                                            for valid list of feature type
*
*               const IDTMGroupID MyFeatureGroupID = 8 -> See application documentation
*                                                         for valid range of group id
*
*               IDTM3dPt MyFeaturePointCArray [10] {(...) -> Add some points}
*
*               // Append a new feature to the end of the list
*               MyFeatures.Append(MyFeatureType1, MyFeaturePointCArray, MyFeaturePointCArray + 10);
*
*               // Append a new feature linked to a group id to the end of the list
*               MyFeatures.Append(MyFeatureType1, MyFeaturePointCArray, MyFeaturePointCArray + 5, MyFeatureGroupID);
*
*               const IDTMFeatureType MyFeatureType2 = 2;
*               vector<IDTM3dPt> MyFeaturePointsVector;
*               (...) -> Add some points
*
*               // Insert my new feature to the beginning of the list
*               MyFeatures.Insert(MyFeatures.Begin(), MyFeatureType2)->Append(MyFeaturePointsVector.begin(),
*                                                                             MyFeaturePointsVector.end());
*
*               (...) -> Add a lot of feature here
*
*               // Do some punctual navigation/query through feature list
*               IDTMFeatureArray<IDTM3dPt>::const_iterator MyConstIt = MyFeatures.Begin() + 3;
*               MyConstIt += 2;
*               --MyConstIt;
*
*               MyConstIt->GetType();
*               MyConstIt->GetGroupId();
*
*               const IDTMFeatureArray<IDTM3dPt>::value_type& rMyFeatureFacade = *MyConstIt;
*               rMyFeatureFacade.GetSize();
*
*               // Do some punctual edition through feature list
*               IDTMFeatureArray<IDTM3dPt>::iterator MyIt = MyFeatures.EndEdit() - 4;
*               MyIt->Append(MyFeaturePointsVector.begin(), MyFeaturePointsVector.end());
*               for_each(MyIt, MyFeatures.EndEdit(), SomeEditionFunctor());
*
*
*               // Use some STL algorithm on the feature list
*               reverse(TestFeatureListCopy.BeginEdit(), TestFeatureListCopy.EndEdit());
*               partition(TestFeatureListCopy.BeginEdit(), TestFeatureListCopy.EndEdit(),
*                         IsSmallerFeature());
*               random_shuffle(TestFeatureListCopy.BeginEdit(), TestFeatureListCopy.EndEdit(),
*                              DefaultPtrShufflerGenerator());
*               reverse(TestFeatureListCopy.BeginEdit(), TestFeatureListCopy.EndEdit());
*
*               sort(TestFeatureListCopy.BeginEdit(), TestFeatureListCopy.EndEdit(), CompareFeature());
*
*               // Always realign points with headers after applying algorithms that
*                  modify feature sequence.
*               TestFeatureListCopy.AlignPointsWithHeaders();
*
*               // Use some STL search algorithms on the feature list
*               includes(TestFeatureListCopy.Begin(), TestFeatureListCopy.End(),
*                        TestFeatureList.Begin(), TestFeatureListCopy.End(), CompareFeature());
*               find_if(TestFeatureListCopy.Begin(), TestFeatureListCopy.End(), IsSmallerFeature());
*
*
* @see IDTMCompositeArrayBase
*
* @bsiclass                                                  Raymond.Gauthier   5/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType, typename HeaderType = ISMStore::FeatureHeader>
class IDTMFeatureArray : public HPU::CompositeArrayBase<IDTMFeatureFacade<PointType, HeaderType>,
                                                        HeaderType,
                                                        IDTMFeatureArray<PointType, HeaderType>>
    {
public:
    typedef HeaderType                          header_type;
    typedef typename header_type::feature_type  feature_type;
    typedef typename header_type::group_id_type group_id_type;

    typedef HPU::Array<HeaderType>      HeaderArray;
    typedef HPU::Array<PointType>       PointArray;

    typedef HPU::CompositeArrayBase<IDTMFeatureFacade<PointType, HeaderType>,HeaderType,IDTMFeatureArray<PointType, HeaderType> >
              IDTMFeatureArray_Type1;  

    explicit                                IDTMFeatureArray                   (size_t                      pi_Capacity = 0,
                                                                                size_t                      pi_TotalPointCapacity = 0);

    size_t                                  GetTotalPointQty                   () const;
    size_t                                  GetTotalPointCapacity              () const;

    void                                    Reserve                            (size_t                      pi_Capacity,
                                                                                size_t                      pi_TotalPointCapacity = 0);


    IDTMFeatureArray_Type1::iterator                                Insert                             (const_iterator   pi_Position,
                                                                                IDTMFeatureArray_Type1::const_reference  pi_rFeature);

    iterator                                Insert                             (const_iterator              pi_Position,
                                                                                feature_type                pi_FeatureType,
                                                                                group_id_type               pi_GroupId = header_type::GetNullID());

    void                                    Insert                             (const_iterator              pi_Position,
                                                                                const_iterator              pi_Begin,
                                                                                const_iterator              pi_End);

    void                                    Append                             (const_reference             pi_rFeature);

    iterator                                Append                             (feature_type                pi_FeatureType,
                                                                                group_id_type               pi_GroupId = header_type::GetNullID());

    template <typename InputIter>
    void                                    Append                             (feature_type                pi_FeatureType,
                                                                                InputIter                   pi_PtBegin,
                                                                                InputIter                   pi_PtEnd,
                                                                                group_id_type               pi_GroupId = header_type::GetNullID());

    void                                    Append                             (const_iterator              pi_Begin,
                                                                                const_iterator              pi_End);

    void                                    Clear                              ();

    iterator                                Erase                              (const_iterator              pi_Position);

    iterator                                Erase                              (const_iterator              pi_Begin,
                                                                                const_iterator              pi_End);

    /*---------------------------------------------------------------------------------**//**
    * This is an alias for Append. Was created in order to be able to reuse stl vector
    * existing tools such as back_inserter & inserter.
    +---------------+---------------+---------------+---------------+---------------+------*/
    void                                    push_back                          (const_reference             pi_rFeature);
    iterator                                insert                             (const_iterator              pi_Position,
                                                                                const_reference             pi_rFeature);
    /*---------------------------------------------------------------------------------**//**
    * If the user decided to manually sort or rearrange directly the underlying header array,
    * he must use this method before doing anything else so that points are aligned again
    * with their respective headers.
    +---------------+---------------+---------------+---------------+---------------+------*/
    void                                    AlignPointsWithHeaders             ();


    // Internal methods
    const HeaderArray&                      GetHeaders                         () const {
        return super_class::GetHeaders();
        }
    HeaderArray&                            EditHeaders                        () {
        return super_class::EditHeaders();
        }

    const PointArray&                       GetPoints                          () const {
        return m_Points;
        }
    PointArray&                             EditPoints                         () {
        return m_Points;
        }


    struct IncrementHeaderOffset : binary_function<header_type, size_t, void>
        {
        void        operator()         (header_type& pi_rHeader, size_t pi_OffsetIncrement) const
            {
            pi_rHeader.offset += static_cast<typename header_type::index_type>(pi_OffsetIncrement);
            }
        };

    struct DecrementHeaderOffset : binary_function<header_type, size_t, void>
        {
        void        operator()         (header_type& pi_rHeader, size_t pi_OffsetDecrement) const
            {
            pi_rHeader.offset -= static_cast<typename header_type::index_type>(pi_OffsetDecrement);
            }
        };

    struct RealignHeaderOffsets : public unary_function<header_type, void>
        {
        explicit    RealignHeaderOffsets   () : m_CurrentOffset(0) {}

        void        operator()             (header_type&     pi_rHeader)
            {
            pi_rHeader.offset = static_cast<typename header_type::index_type>(m_CurrentOffset);
            m_CurrentOffset += pi_rHeader.size;
            }

    private:
        size_t      m_CurrentOffset;
        };

    struct CopyFeaturePoints : public unary_function<value_type, void>
        {
        explicit    CopyFeaturePoints      (typename PointArray::iterator po_OutputPointIt) : m_OutputPointIt(po_OutputPointIt) {}

        void        operator()             (const facade_type&       pi_rFeature)
            {
            m_OutputPointIt = copy(pi_rFeature.Begin(), pi_rFeature.End(), m_OutputPointIt);
            }
    private:
        typename PointArray::iterator m_OutputPointIt;
        };

    void                                    IncrementPointOffsets              (typename HeaderArray::iterator  pi_From,
                                                                                size_t                          pi_OffsetIncrement);

    void                                    DecrementPointOffsets              (typename HeaderArray::iterator  pi_From,
                                                                                size_t                          pi_OffsetDecrement);

    void                                    RecomputePointOffsets              ();


    PointArray                              m_Points;
    bool                                    m_PointsAlignedWithHeader;
    };

template <typename PointType, typename HeaderType>
inline void                                 swap                               (IDTMFeatureFacade<PointType, HeaderType>& pi_rLeft,
                                                                                IDTMFeatureFacade<PointType, HeaderType>& pi_rRight)
    {
    pi_rLeft.Swap(pi_rRight);
    }


#include "IDTMFeatureArray.hpp"

// TDORAY: Dirty Microsoft fix/hack for their bad use of swap in std algorithms. In VC2010 the problem is fixed so remove.
//#include "IDTMFileDirectories/PointTypes.h"
//#include "IDTMFileDirectories/FeatureHeaderTypes.h"
/*namespace std
{

inline void                             swap                               (IDTMFeatureFacade<ISMStore::Point3d64f, ISMStore::FeatureHeader>& pi_rLeft,
                                                                            IDTMFeatureFacade<ISMStore::Point3d64f, ISMStore::FeatureHeader>& pi_rRight)
    {
    pi_rLeft.Swap(pi_rRight);
    }

inline void                             iter_swap                          (IDTMFeatureArray<ISMStore::Point3d64f, ISMStore::FeatureHeader>::iterator pi_rLeft,
                                                                            IDTMFeatureArray<ISMStore::Point3d64f, ISMStore::FeatureHeader>::iterator pi_rRight)
    {
    (*pi_rLeft).Swap(*pi_rRight);
    }
};*/
