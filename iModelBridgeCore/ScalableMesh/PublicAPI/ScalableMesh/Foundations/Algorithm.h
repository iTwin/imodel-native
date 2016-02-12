/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Foundations/Algorithm.h $
|    $RCSfile: Algorithm.h,v $
|   $Revision: 1.2 $
|       $Date: 2011/12/20 16:23:40 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Foundations/Definitions.h>
#include <ScalableMesh/Foundations/Traits.h>

#include <functional>

BEGIN_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE




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


#include <ScalableMesh/Foundations/Algorithm.hpp>

END_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE
