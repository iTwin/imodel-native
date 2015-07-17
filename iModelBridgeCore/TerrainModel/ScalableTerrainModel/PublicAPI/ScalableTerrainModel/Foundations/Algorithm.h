/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Foundations/Algorithm.h $
|    $RCSfile: Algorithm.h,v $
|   $Revision: 1.2 $
|       $Date: 2011/12/20 16:23:40 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Foundations/Definitions.h>
#include <ScalableTerrainModel/Foundations/Traits.h>

#include <functional>

BEGIN_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE



/*---------------------------------------------------------------------------------**//**
* @description  Integral linear numbers generator. 
*               
*               e.g: std::generate(range.begin(), range.end(), 
*                                  LinearIntegerGenerator<int>(1000, -12));
*
* @bsiclass                                                  Raymond.Gauthier   12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T, const int INCREMENT = 1>
struct LinearIntegerGenerator
    {
private:
    T                               m_current;
    T                               m_increment;

public:
    explicit                        LinearIntegerGenerator                         (T           start,
                                                                                    T           increment) 
        :   m_current(start - increment),
            m_increment(increment) {}
    T                               operator()                                     ()                          
        { return m_current += m_increment; }

    };

/*---------------------------------------------------------------------------------**//**
* @description  Integral linear numbers generator. Use only when increment is known
*               at compile time.
*               
*               e.g: std::generate(range.begin(), range.end(), 
*                                  StaticLinearIntegerGenerator<int, -12>(1000));
*
* NOTE: Specialized version for -1/1 increments
*
* @bsiclass                                                  Raymond.Gauthier   12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T, const int INCREMENT = 1>
struct StaticLinearIntegerGenerator
    {
private:
    T                               m_current;

public:
    explicit                        StaticLinearIntegerGenerator                   (T           start) 
        :   m_current(start - INCREMENT) {}
    T                               operator()                                     ()                          
        { return m_current += INCREMENT; }

    };


/*---------------------------------------------------------------------------------**//**
* @description  Random integral number generator which compensate for standard random 
*               number generator limitations in precision (RAND_MAX).
*               
*               This generator expect that user has appropriately initialized his standard
*               random seed (via srand).
*
*               e.g: std::generate(range.begin(), range.end(), 
*                                  RandomIntegerGenerator<int>(-1000, 100000));
*
* TDORAY: See if a replacement exist in C++0x standard.
*
* @bsiclass                                                  Raymond.Gauthier   12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
struct RandomIntegerGenerator
    {
private:
    static_assert(is_integral<T>::value, "") // Only works with integral types

    T                               m_min;
    T                               m_range;

public:
    explicit                        RandomIntegerGenerator                     (const T&            min,
                                                                                const T&            max);

    T                               operator()                                 () const;
    };


/*---------------------------------------------------------------------------------**//**
* @description  Optimized random integral number generator which compensate for standard
*               random number generator limitations in precision (RAND_MAX). Use only
*               when range is known at compile time.
*               
*               This generator expect that user has appropriately initialized his standard
*               random seed (via srand).
*
*               e.g: std::generate(range.begin(), range.end(), 
*                                  StaticRandomIntegerGenerator<int, -1000, 100000>());
*
* TDORAY: See if a replacement exist in C++0x standard.
*
* @bsiclass                                                  Raymond.Gauthier   12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T, T T_MIN, T T_MAX>
struct StaticRandomIntegerGenerator
{
private:

    static_assert(T_MAX > T_MIN, "") // Validate that T_MAX is greater than T_MIN
    static_assert(is_integral<T>::value, "") // Only works with integral types

    static const T                  RANGE = T_MAX - T_MIN + 1;

public:
    T                               operator()                                 () const;
};


/*---------------------------------------------------------------------------------**//**
* @description  Setter output iterator:
*               Wrap an existing iterator (can also be a pointer) and when a value
*               is outputted to this iterator, this value is saved to the existing
*               object using the specified setter function.
*               
*  Example:
*  // Set values to the object using its SetValue setter.
*  copy(m_Values.begin(), m_Values.end(),
*       SetterIter(m_Objects.Begin(), &MyObjectClass::SetValue));
*
* TDORAY:   Taken from <ImagePP/h/HStlStuff.h>. Find a way to place this tool in 
*           a place that both ImagePP and Memory can depend on (e.g.: Bentley headers).
* @bsiclass                                                  Raymond.Gauthier   06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename WrappedIterator, typename SetterFunction>
class SetterIterator :  public std::iterator<std::output_iterator_tag,
                                             typename RemoveReference<typename SetterFunction::second_argument_type>::type>
    {
public:
    typedef SetterIterator<WrappedIterator, SetterFunction> iterator_type;
    typedef typename std::iterator<std::output_iterator_tag, typename RemoveReference<typename SetterFunction::second_argument_type>::type>::value_type 
                                value_type_SetterIterator;

    explicit                    SetterIterator                     (WrappedIterator         pi_rWrappedIterator,
                                                                    SetterFunction&         pi_rSetterFunction)
        :   m_WrappedIterator(pi_rWrappedIterator),
            m_SetterFunction(pi_rSetterFunction)

        {
        }

    iterator_type&              operator=                          (const value_type_SetterIterator&       pi_rInputValue)
        {
        m_SetterFunction(*m_WrappedIterator, pi_rInputValue);
        return (*this);
        }

    iterator_type&              operator*()                         
        {
        return (*this);
        }
    iterator_type&              operator++()                        
        {
        ++m_WrappedIterator;
        return (*this);
        }
    iterator_type               operator++(int)                     
        {
        iterator_type Tmp(*this);
        ++(*this);
        return Tmp;
        }
private:
    WrappedIterator             m_WrappedIterator;
    SetterFunction              m_SetterFunction;
    };


template <typename SetterFunction, typename WrappedIterator>
inline SetterIterator<WrappedIterator, SetterFunction> setterIter  (WrappedIterator         pi_rWrappedIterator,
                                                                    SetterFunction&         pi_rSetterFunction)
    {
    return SetterIterator<WrappedIterator,
           SetterFunction>(pi_rWrappedIterator, pi_rSetterFunction);
    }


template <typename Result, typename T, typename Argument, typename WrappedIterator>
inline SetterIterator<WrappedIterator, std::mem_fun1_ref_t<Result, T, Argument>>
                                                        setterIter (WrappedIterator         pi_rWrappedIterator,
                                                                    Result                  (T::*pi_pfnSetter)(Argument))
    {
    std::mem_fun1_ref_t<Result, T, Argument> MemFunc(pi_pfnSetter);
    return SetterIterator<WrappedIterator,
           std::mem_fun1_ref_t<Result, T, Argument> >(pi_rWrappedIterator, MemFunc);
    }


#include <ScalableTerrainModel/Foundations/Algorithm.hpp>

END_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE
