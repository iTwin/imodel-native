/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/Import/DimensionIterator.h $
|    $RCSfile: DimensionIterator.h,v $
|   $Revision: 1.2 $
|       $Date: 2011/06/01 14:02:45 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

// NTERAY: This is a private include file. When possible move it with cpp that use it in order to hide it from library users

#include <ImagePP/h/HTraits.h>
#include <ImagePP/h/HIterators.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

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

    difference_type                     GetMemoryDistanceFrom              (iterator_t                  pi_Iter)
        {
        return (ByteType*)m_pCurrent - (ByteType*)pi_Iter.m_pCurrent;
        }

    // NOTE: Public interface is STL random access iterators interface. 
    //       See ImagePP::RandomAccessIterator.

private:
//    friend                              super_class;
    friend                              rconst_iterator_t;

    enum OptimizationLevel
        {
        OPTL_NONE,
        OPTL_T_SZ_MULT_OF_INCR,
        OPTL_T_SZ_EQ_INCR,
        };

    HSTATICASSERT(sizeof(T) <= IncrementToNext);
    static const OptimizationLevel      OPT_LEVEL = (sizeof(T) == IncrementToNext) ? OPTL_T_SZ_EQ_INCR
                                                        : (0 == (IncrementToNext % sizeof(T))) ? OPTL_T_SZ_MULT_OF_INCR 
                                                                : OPTL_NONE;

    static const size_t                 TypedIncrementToNext = IncrementToNext/sizeof(T);


    const_iterator_t                    ConvertToConst                     () const                                     {return const_iterator_t(m_pCurrent);}

    const_reference                     Dereference                        () const                                     {return *m_pCurrent;}
    reference                           Dereference                        ()                                           {return *m_pCurrent;}

    void                                Increment                          ()                                           {Increment<OPT_LEVEL>(m_pCurrent);}
    void                                Decrement                          ()                                           {Decrement<OPT_LEVEL>(m_pCurrent);}

    void                                AdvanceOf                          (difference_type             pi_Offset)      {Advance<OPT_LEVEL>(m_pCurrent, pi_Offset);}


    difference_type                     DistanceFrom                       (const iterator_t&           pi_rRight) const {return ComputeDistance<OPT_LEVEL>(m_pCurrent, pi_rRight.m_pCurrent);}
    difference_type                     DistanceFrom                       (const rconst_iterator_t&    pi_rRight) const {return ComputeDistance<OPT_LEVEL>(m_pCurrent, pi_rRight.m_pCurrent);}

    bool                                EqualTo                            (const iterator_t&           pi_rRight) const {return m_pCurrent == pi_rRight.m_pCurrent;}
    bool                                LessThan                           (const iterator_t&           pi_rRight) const {return m_pCurrent < pi_rRight.m_pCurrent;}
    bool                                EqualTo                            (const rconst_iterator_t&    pi_rRight) const {return m_pCurrent == pi_rRight.m_pCurrent;}
    bool                                LessThan                           (const rconst_iterator_t&    pi_rRight) const {return m_pCurrent < pi_rRight.m_pCurrent;}


    template <OptimizationLevel OptLevel>
    static difference_type              ComputeDistance                    (T*                          pi_pTo, 
                                                                            T*                          pi_pFrom)
        {
        return (reinterpret_cast<ByteType*>(pi_pTo) - reinterpret_cast<ByteType*>(pi_pFrom))/IncrementToNext;
        }
    template <> static difference_type  ComputeDistance<OPTL_T_SZ_EQ_INCR> (T*                          pi_pTo, 
                                                                            T*                          pi_pFrom)
        {
        return pi_pTo - pi_pFrom;
        }


    template <OptimizationLevel OptLevel>
    static void                         Increment                          (T*&                         pi_rpCurrent)
        {
        reinterpret_cast<ByteType*&>(pi_rpCurrent) += IncrementToNext;
        }
    template <> static void             Increment<OPTL_T_SZ_MULT_OF_INCR>  (T*&                         pi_rpCurrent)
        {
        pi_rpCurrent += TypedIncrementToNext;
        }
    template <> static void             Increment<OPTL_T_SZ_EQ_INCR>       (T*&                         pi_rpCurrent)
        {
        ++pi_rpCurrent;
        }

    template <OptimizationLevel OptLevel>
    static void                         Decrement                          (T*&                         pi_rpCurrent)
        {
        reinterpret_cast<ByteType*&>(pi_rpCurrent) -= IncrementToNext;
        }
    template <> static void             Decrement<OPTL_T_SZ_MULT_OF_INCR>  (T*&                         pi_rpCurrent)
        {
        pi_rpCurrent += TypedIncrementToNext;
        }
    template <> static void             Decrement<OPTL_T_SZ_EQ_INCR>       (T*&                         pi_rpCurrent)
        {
        ++pi_rpCurrent;
        }


    template <OptimizationLevel OptLevel>
    static void                         Advance                            (T*&                         pi_rpCurrent,
                                                                            difference_type             pi_Offset)
        {
        reinterpret_cast<ByteType*&>(m_pCurrent) += (IncrementToNext*pi_Offset);
        }
    template <> static void             Advance<OPTL_T_SZ_EQ_INCR>         (T*&                         pi_rpCurrent,
                                                                            difference_type             pi_Offset)
        {
        m_pCurrent += pi_Offset;
        }

    T*                                  m_pCurrent;
    };


END_BENTLEY_MRDTM_IMPORT_NAMESPACE
