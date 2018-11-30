/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/DimensionIterator.h $
|    $RCSfile: DimensionIterator.h,v $
|   $Revision: 1.2 $
|       $Date: 2011/06/01 14:02:45 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

// NTERAY: This is a private include file. When possible move it with cpp that use it in order to hide it from library users

#include <ImagePP/h/HTraits.h>
#include <ImagePP/h/HIterators.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
*    
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T, size_t IncrementToNext = sizeof(T)>
class DimensionIterator : public ImagePP::RandomAccessIteratorWithAutoReverseConst<DimensionIterator<T, IncrementToNext>,
                                                                                DimensionIterator<typename ImagePP::ReverseConstTrait<T>::type, IncrementToNext>,
                                                                                T,
                                                                                sizeof(T) == IncrementToNext>
    {
public:
    // Typedef a void type that match in constness with T
    typedef typename ImagePP::SameConstAsTrait<void, T>::type
                                        VoidType;
    // Typedef a void type that match in constness with T
    typedef typename ImagePP::SameConstAsTrait<byte, T>::type
                                        ByteType;

    typedef DimensionIterator<T, IncrementToNext> super_class;

    explicit                            DimensionIterator                  ()                                       
                                            :   m_pCurrent(0)                                                   {}


    explicit                            DimensionIterator                  (VoidType*                   pi_pBuffer)
                                            :   m_pCurrent(reinterpret_cast<T*>(pi_pBuffer))                    {}

    explicit                            DimensionIterator                  (VoidType*                   pi_pBuffer,
                                                                            ptrdiff_t                   pi_Offset)
                                            :   m_pCurrent(reinterpret_cast<T*>(reinterpret_cast<ByteType*>(pi_pBuffer) + pi_Offset)) 
                                                                                                                {}
    
    explicit                            DimensionIterator                  (ByteType*                   pi_pBuffer)
                                            :   m_pCurrent(reinterpret_cast<T*>(pi_pBuffer))                    {}

    explicit                            DimensionIterator                  (ByteType*                   pi_pBuffer,
                                                                            ptrdiff_t                   pi_Offset)
                                            :   m_pCurrent(reinterpret_cast<T*>(pi_pBuffer + pi_Offset)) 
                                                                                                                {}

    template <typename OtherType>
    DimensionIterator<typename ImagePP::SameConstAsTrait<OtherType, T>::type, IncrementToNext>
                                        CreateNextDimension                () const 
        {
        return DimensionIterator<typename SameConstAsTrait<OtherType, T>::value, IncrementToNext>(m_pCurrent, sizeof(T));
        }

    void                                NextDimension                      ()
        {
        Offset(sizeof(T));
        }

    void                                Offset                             (ptrdiff_t                   pi_Offset)
        {
        reinterpret_cast<ByteType*&>(m_pCurrent) += pi_Offset;
        }

    typename super_class::difference_type  GetMemoryDistanceFrom              (typename super_class::iterator_t                  pi_Iter)
        {
        return (ByteType*)m_pCurrent - (ByteType*)pi_Iter.m_pCurrent;
        }

    // NOTE: Public interface is STL random access iterators interface. 
    //       See ImagePP::RandomAccessIterator.

private:
//    friend                              super_class;
//    friend                              rconst_iterator_t;

    enum OptimizationLevel
        {
        OPTL_NONE,
        OPTL_T_SZ_MULT_OF_INCR,
        OPTL_T_SZ_EQ_INCR,
        };

    HSTATICASSERT(sizeof(T) <= IncrementToNext);
   /* static const OptimizationLevel      OPT_LEVEL = (sizeof(T) == IncrementToNext) ? OPTL_T_SZ_EQ_INCR
                                                        : (0 == (IncrementToNext % sizeof(T))) ? OPTL_T_SZ_MULT_OF_INCR 
                                                                : OPTL_NONE;*/

    static const size_t                 TypedIncrementToNext = IncrementToNext/sizeof(T);


    typename super_class::const_iterator_t ConvertToConst                     () const                                     {return const_iterator_t(m_pCurrent);}

    typename super_class::const_reference  Dereference                        () const                                     {return *m_pCurrent;}
    typename super_class::reference        Dereference                        ()                                           {return *m_pCurrent;}

    void                                Increment                          ()                                           {Increment<OPT_LEVEL>(m_pCurrent);}
    void                                Decrement                          ()                                           {Decrement<OPT_LEVEL>(m_pCurrent);}

    void                                AdvanceOf                          (typename super_class::difference_type             pi_Offset)      {Advance<OPT_LEVEL>(m_pCurrent, pi_Offset);}


    typename super_class::difference_type DistanceFrom                       (const typename super_class::iterator_t&           pi_rRight) const {return ComputeDistance<OPT_LEVEL>(m_pCurrent, pi_rRight.m_pCurrent);}
    typename super_class::difference_type DistanceFrom                       (const typename super_class::rconst_iterator_t&    pi_rRight) const {return ComputeDistance<OPT_LEVEL>(m_pCurrent, pi_rRight.m_pCurrent);}

    bool                                EqualTo                            (const typename super_class::iterator_t&           pi_rRight) const {return m_pCurrent == pi_rRight.m_pCurrent;}
    bool                                LessThan                           (const typename super_class::iterator_t&           pi_rRight) const {return m_pCurrent < pi_rRight.m_pCurrent;}
    bool                                EqualTo                            (const typename super_class::rconst_iterator_t&    pi_rRight) const {return m_pCurrent == pi_rRight.m_pCurrent;}
    bool                                LessThan                           (const typename super_class::rconst_iterator_t&    pi_rRight) const {return m_pCurrent < pi_rRight.m_pCurrent;}


    template <class U = T>
    static typename std::enable_if<(sizeof(U) != IncrementToNext), typename super_class::difference_type>::type              ComputeDistance                    (T*                          pi_pTo,
                                                                            T*                          pi_pFrom)
        {
        return (reinterpret_cast<ByteType*>(pi_pTo) - reinterpret_cast<ByteType*>(pi_pFrom))/IncrementToNext;
        }
    
    template <class U = T>
    static typename std::enable_if<(sizeof(U) == IncrementToNext), typename super_class::difference_type>::type  ComputeDistance (T*                          pi_pTo,
                                                                            T*                          pi_pFrom)
        {
        return pi_pTo - pi_pFrom;
        }


    template <class U = T>
    static typename std::enable_if<(sizeof(U) != IncrementToNext && (0 != (IncrementToNext % sizeof(U)))),void>::type                       Increment                          (T*&                         pi_rpCurrent)
        {
        reinterpret_cast<ByteType*&>(pi_rpCurrent) += IncrementToNext;
        }
    template <class U = T>
    static typename std::enable_if<(0 == (IncrementToNext % sizeof(U))),void>::type      Increment (T*&                         pi_rpCurrent)
        {
        pi_rpCurrent += TypedIncrementToNext;
        }
    template <class U = T>
    static typename std::enable_if<(sizeof(U) == IncrementToNext),void>::type               Increment       (T*&                         pi_rpCurrent)
        {
        ++pi_rpCurrent;
        }

    template <class U = T>
    static typename std::enable_if<(sizeof(U) != IncrementToNext && (0 != (IncrementToNext % sizeof(U)))),void>::type                        Decrement                          (T*&                         pi_rpCurrent)
        {
        reinterpret_cast<ByteType*&>(pi_rpCurrent) -= IncrementToNext;
        }
    template <class U = T>
    static typename std::enable_if<(0 == (IncrementToNext % sizeof(U))),void>::type  Decrement (T*&                         pi_rpCurrent)
        {
        pi_rpCurrent += TypedIncrementToNext;
        }
    template <class U = T>
    static typename std::enable_if<(sizeof(U) == IncrementToNext),void>::type           Decrement       (T*&                         pi_rpCurrent)
        {
        ++pi_rpCurrent;
        }


    template <class U = T>
    static typename std::enable_if<(sizeof(U) != IncrementToNext), typename super_class::difference_type>::type          Advance                            (T*&                         pi_rpCurrent,
                                                                            typename super_class::difference_type             pi_Offset)
        {
        reinterpret_cast<ByteType*&>(m_pCurrent) += (IncrementToNext*pi_Offset);
        }
    template <class U = T>
    static typename std::enable_if<(sizeof(U) == IncrementToNext), typename super_class::difference_type>::type            Advance         (T*&                         pi_rpCurrent,
                                                                            typename super_class::difference_type             pi_Offset)
        {
        m_pCurrent += pi_Offset;
        }

    T*                                  m_pCurrent;
    };


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
