//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/h/HStlStuff.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include <Bentley/WString.h>
#include <functional>

BEGIN_IMAGEPP_NAMESPACE
/*
** --------------------------------------------------------------------------
**  FREEZE_STL_STRING
**
**  Use to disable reference counting on strings that will be shared among
**  many threads. RefCounting (Copy on Write) is implemented in Microsoft's
**  STL (currently version 6.0 SP5)
** --------------------------------------------------------------------------
*/

#if 1
// Copy on write is not legal in C++11
//http://stackoverflow.com/questions/12199710/legality-of-cow-stdstring-implementation-in-c11

#define FREEZE_STL_STRING(x)

#else
#   define STL_STRING_DOES_REFCOUNTING

// Call this macro to ensure that the specified string (STL::basic_string<>)
//  has its private copy of the string data.
#   define FREEZE_STL_STRING_A(x)  {const_cast<std::string&>(x).begin();}
#   define FREEZE_STL_STRING_W(x)  {const_cast<WString&>(x).begin();}
#   define FREEZE_STL_STRING_SEQUENCE_A(ItrBegin, ItrEnd)  { std::for_each(ItrBegin, ItrEnd, FreezeStlStringFunction_A); }
#   define FREEZE_STL_STRING_SEQUENCE_W(ItrBegin, ItrEnd)  { std::for_each(ItrBegin, ItrEnd, FreezeStlStringFunction_W); }

#       define FREEZE_STL_STRING(x) FREEZE_STL_STRING_W(x)
#       define FREEZE_STL_STRING_SEQUENCE(ItrBegin, ItrEnd)  { std::for_each(ItrBegin, ItrEnd, FreezeStlStringFunction_W); }

// To be used with STL for_each on a vector of string objects.
inline void FreezeStlStringFunction_A(std::string& pi_rObj)
    {
    FREEZE_STL_STRING_A(pi_rObj)
    }

inline void FreezeStlStringFunction_W(WString& pi_rObj)
    {
    FREEZE_STL_STRING_W(pi_rObj)
    }  
#endif

// TEMPLATE STRUCT less
struct lessDoubleEpsilon
        : public std::binary_function<double, double, bool>
    {   // functor for operator<
    bool operator()(const double& _Left, const double& _Right) const
        {   // apply operator< to operands
        return HDOUBLE_SMALLER_EPSILON(_Left, _Right);
        }
    };

/*
** --------------------------------------------------------------------------
**  Static cast functor
**
**  Example:
**  // Cast all v1's items to UShort and store the result in v2
**  transform(v1.begin(), v1.end(), back_inserter(v2), StaticCast<UShort>());
** --------------------------------------------------------------------------
*/
template<typename ToType>
struct StaticCast
    {
    template<typename FromType>
    ToType operator () (const FromType& pi_rFrom) const
        {
        return static_cast<ToType>(pi_rFrom);
        }
    };

/*
** --------------------------------------------------------------------------
**  Type trait used to remove reference from a specified type.
** --------------------------------------------------------------------------
*/
template <typename T> struct RemoveReference     {
    typedef T type;
    };
template <typename T> struct RemoveReference<T&> {
    typedef T type;
    };


/*
** --------------------------------------------------------------------------
**  Destroy and zero initialize ptr functor.
** e.g. vector<int32_t*> m_MyVector;
**      for_each(m_MyVector.begin(), m_MyVector.end(), DestroyReinitPtr());
** --------------------------------------------------------------------------
*/
struct DestroyReinitPtr
    {
    template <typename T>
    void operator () (T*& pi_rpDirInfo) const
        {
        if (0 == pi_rpDirInfo)
            return;

        delete pi_rpDirInfo;
        pi_rpDirInfo = 0;
        }
    };

/*
** --------------------------------------------------------------------------
**  Destroy only ptr functor. Pointers will not be zero initialized.
** e.g. vector<int32_t*> m_MyVector;
**      for_each(m_MyVector.begin(), m_MyVector.end(), DestroyPtr());
** --------------------------------------------------------------------------
*/
struct DestroyPtr
    {
    template <typename T>
    void operator () (T* pi_pDirInfo) const
        {
        // Delete is assumed to have no effect on null pointers
        delete pi_pDirInfo;
        }
    };


#if 1
/*
** --------------------------------------------------------------------------
**  Setter output iterator:
**  Wrap an existing iterator (can also be a pointer) and when a value
**  is outputted to this iterator, this value is saved to the existing
**  object using the specified setter function.
**
**  Example:
**  // Set values to the object using its SetValue setter.
**  copy(m_Values.begin(), m_Values.end(),
**       SetterIter(m_Objects.Begin(), &MyObjectClass::SetValue));
** --------------------------------------------------------------------------
*/
template <typename WrappedIterator, typename SetterFunction>
class SetterIterator :  public std::iterator<std::output_iterator_tag,
                                             typename RemoveReference<typename SetterFunction::second_argument_type>::type>
    {
public:
    typedef SetterIterator<WrappedIterator, SetterFunction> iterator_type;
    typedef typename std::iterator<std::output_iterator_tag, typename RemoveReference<typename SetterFunction::second_argument_type>::type>::value_type value_type_SetterIterator;

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

    iterator_type&              operator*()                         {
        return (*this);
        }
    iterator_type&              operator++()                        {
        ++m_WrappedIterator;
        return (*this);
        }
    iterator_type               operator++(int32_t)                     {
        iterator_type Tmp(*this);
        ++(*this);
        return Tmp;
        }
private:
    WrappedIterator             m_WrappedIterator;
    SetterFunction              m_SetterFunction;
    };



template <typename SetterFunction, typename WrappedIterator>
inline SetterIterator<WrappedIterator, SetterFunction> SetterIter  (WrappedIterator         pi_rWrappedIterator,
                                                                    SetterFunction&         pi_rSetterFunction)
    {
    return SetterIterator<WrappedIterator,
           SetterFunction>(pi_rWrappedIterator, pi_rSetterFunction);
    }


template <typename Result, typename T, typename Argument, typename WrappedIterator>
inline SetterIterator<WrappedIterator, std::mem_fun1_ref_t<Result, T, Argument>>
                                                                              SetterIter (WrappedIterator         pi_rWrappedIterator,
                                                                                          Result                  (T::*pi_pfnSetter)(Argument))
    {
    std::mem_fun1_ref_t<Result, T, Argument> MemFunc(pi_pfnSetter);
    return SetterIterator<WrappedIterator,
           std::mem_fun1_ref_t<Result, T, Argument> >(pi_rWrappedIterator, MemFunc);
    }

#endif

/*
** --------------------------------------------------------------------------
**  char version
** --------------------------------------------------------------------------
*/

/*
** --------------------------------------------------------------------------
**  Case-insensitive string ordering predicate, to be used with ordered
**  containers like set and map.
**
**  Behaves as a less_than (<) operator, case insensitive. Builds a
**  map at construction to save toupper calls. Defaults to the "classic C"
**  locale if none is specified.
**
**  Taken from "Effective STL", Scott Meyers, Appendix A.
** --------------------------------------------------------------------------
*/
struct CaseInsensitiveStringCompareA : public std::binary_function<std::string, std::string, bool>
    {
    // Case insensitive character ordering predicate
    struct CaseInsensitiveCharCompare
        {
        const char* pCharMap;

        CaseInsensitiveCharCompare(const char* pi_CharMap)
            : pCharMap(pi_CharMap)
            {
            }

        bool operator()(char x, char y) const
            {
            return pCharMap[x - CHAR_MIN] < pCharMap[y - CHAR_MIN];
            }
        };

    char CharMap[CHAR_MAX - CHAR_MIN + 1];

    CaseInsensitiveStringCompareA(const std::locale& L = std::locale::classic())
        {
        const std::ctype<char>& ct = std::use_facet< std::ctype<char> >(L);

        for (int32_t i = CHAR_MIN ; i <= CHAR_MAX ; ++i)
            CharMap[i - CHAR_MIN] = (char) i;

        ct.toupper(CharMap, CharMap + (CHAR_MAX - CHAR_MIN + 1));
        }

    bool operator()(const std::string& x, const std::string& y) const
        {
        return std::lexicographical_compare(x.begin(), x.end(),
                                            y.begin(), y.end(),
                                            CaseInsensitiveCharCompare(CharMap));
        }
    };


/*
** --------------------------------------------------------------------------
**  Case-insensitive string tools.
**  The tools default to the "classic C" locale if none is specified.
**
**  Adapted from "Effective STL", Scott Meyers, Appendix A.
** --------------------------------------------------------------------------
*/
struct CaseInsensitiveStringToolsA
    {
    // Character equality test predicate
    struct CaseInsensitiveCharEqualityTester
        {
        const std::ctype<char>& ct;
        CaseInsensitiveCharEqualityTester(const std::ctype<char>& c)
            : ct(c)
            {
            }

        bool operator()(char x, char y) const
            {
            return ct.toupper(x) == ct.toupper(y);
            }
        };

    // The following methods should normally have been in an embedded traits class.
    // However, there vas a compilation problem.
    bool traitseq(const char& pi_rChar1, const char& pi_rChar2) const
        {
        return ct.toupper(pi_rChar1) == ct.toupper(pi_rChar2);
        }
    bool traitslt(const char& pi_rChar1, const char& pi_rChar2) const
        {
        return ct.toupper(pi_rChar1) < ct.toupper(pi_rChar2);
        }
    int32_t traitscompare(const char* pi_pStr1, const char* pi_pStr2, size_t pi_n) const
        {
        while ((pi_n > 0) && (traitseq(*pi_pStr1, *pi_pStr2)))
            {
            --pi_n;
            ++pi_pStr1;
            ++pi_pStr2;
            }

        if (pi_n > 0)
            return traitslt(*pi_pStr1, *pi_pStr2) ? -1 : 1;
        else
            return 0;
        }
    const char* traitsfind(const char* pi_pStr, size_t pi_n, const char& pi_rChar) const
        {
        while ((pi_n > 0) && (! traitseq(*pi_pStr, pi_rChar)))
            {
            --pi_n;
            ++pi_pStr;
            }

        return (pi_n == 0) ? NULL : pi_pStr;
        }

    std::locale loc;
    const std::ctype<char>& ct;

    CaseInsensitiveStringToolsA(const std::locale& L = std::locale::classic())
        : loc(L), ct(std::use_facet< std::ctype<char> >(L))
        {
        }

    // string equality test operation
    bool AreEqual(const std::string& x, const std::string& y) const
        {
        // Must be equal length AND equivalent characters.
        return x.length() == y.length() &&
               std::equal(x.begin(), x.end(),
                          y.begin(),
                          CaseInsensitiveCharEqualityTester(ct));
        }

    // find methods, code adapted from microsoft's xstring header.
    std::string::size_type Find(const std::string& Object, const std::string& X, std::string::size_type P = 0) const
        {
        return (Find(Object, X.c_str(), P, X.size()));
        }
    std::string::size_type Find(const std::string& Object, const char* S, std::string::size_type P, std::string::size_type N) const
        {
        std::string::size_type _Len = Object.size();
        const char* _Ptr = Object.c_str();
        if (N == 0 && P <= _Len)
            return (P);
        std::string::size_type _Nm;
        if (P < _Len && N <= (_Nm = _Len - P))
            {
            const char* U, *V;
            for (_Nm -= N - 1, V = _Ptr + P;
                 (U = traitsfind(V, _Nm, *S)) != 0;
                 _Nm -= U - V + 1, V = U + 1)
                if (traitscompare(U, S, N) == 0)
                    return (U - _Ptr);
            }
        return (std::string::npos);
        }
    std::string::size_type Find(const std::string& Object, const char* S, std::string::size_type P = 0) const
        {
        return (Find(Object, S, P, std::char_traits<char>::length(S)));
        }
    std::string::size_type Find(const std::string& Object, char C, std::string::size_type P = 0) const
        {
        return (Find(Object, (const char*)&C, P, 1));
        }

    void ToLower(std::string& Object) const
        {
        // TR 128069 the _strlwr failed with an empty string when Regional Options
        // is set to something else than English(xxxx).  We don't know the real cause
        // of this crash because we can't debug the function.
        if(!Object.empty())
            BeStringUtilities::Strlwr(&(Object[0]));
        }

    void ToUpper(std::string& Object) const
        {
        // TR 128069 the _strupr failed with an empty string when Regional Options
        // is set to something else than English(xxxx).  We don't know the real cause
        // of this crash because we can't debug the function.
        if(!Object.empty())
            BeStringUtilities::Strupr(&(Object[0]));
        }
    };


/*
** --------------------------------------------------------------------------
**  wchar version
** --------------------------------------------------------------------------
*/

/*
** --------------------------------------------------------------------------
**  Case-insensitive string ordering predicate, to be used with ordered
**  containers like set and map.
**
**  Behaves as a less_than (<) operator, case insensitive. Builds a
**  map at construction to save toupper calls. Defaults to the "classic C"
**  locale if none is specified.
**
**  Taken from "Effective STL", Scott Meyers, Appendix A.
** --------------------------------------------------------------------------
*/
struct CaseInsensitiveStringCompareW : public std::binary_function<WString, WString, bool>
    {
    bool operator()(const WString& x, const WString& y) const
        {
        return BeStringUtilities::Wcsicmp(x.c_str(), y.c_str()) < 0;
        }
    };

struct CaseInsensitiveStringCompareUtf8 : public std::binary_function<Utf8String, Utf8String, bool>
    {
    bool operator()(const Utf8String& x, const Utf8String& y) const
        {
        return x.CompareToI(y) < 0;
        }
    };


/*
** --------------------------------------------------------------------------
**  Case-insensitive string tools.
**  The tools default to the "classic C" locale if none is specified.
**
**  Adapted from "Effective STL", Scott Meyers, Appendix A.
** --------------------------------------------------------------------------
*/
struct CaseInsensitiveStringToolsW
    {
    // The following methods should normally have been in an embedded traits class.
    // However, there vas a compilation problem.
    bool traitseq(const WChar& pi_rChar1, const WChar& pi_rChar2) const
        {
        return ct.toupper(pi_rChar1) == ct.toupper(pi_rChar2);
        }
    bool traitslt(const WChar& pi_rChar1, const WChar& pi_rChar2) const
        {
        return ct.toupper(pi_rChar1) < ct.toupper(pi_rChar2);
        }
    int32_t traitscompare(const WChar* pi_pStr1, const WChar* pi_pStr2, size_t pi_n) const
        {
        while ((pi_n > 0) && (traitseq(*pi_pStr1, *pi_pStr2)))
            {
            --pi_n;
            ++pi_pStr1;
            ++pi_pStr2;
            }

        if (pi_n > 0)
            return traitslt(*pi_pStr1, *pi_pStr2) ? -1 : 1;
        else
            return 0;
        }
    const WChar* traitsfind(const WChar* pi_pStr, size_t pi_n, const WChar& pi_rChar) const
        {
        while ((pi_n > 0) && (! traitseq(*pi_pStr, pi_rChar)))
            {
            --pi_n;
            ++pi_pStr;
            }

        return (pi_n == 0) ? NULL : pi_pStr;
        }

    std::locale loc;
    const std::ctype<WChar>& ct;

    CaseInsensitiveStringToolsW(const std::locale& L = std::locale::classic())
        : loc(L), ct(std::use_facet< std::ctype<WChar> >(L))
        {
        }

    // string equality test operation
    bool AreEqual(const WString& x, const WString& y) const
        {
        return BeStringUtilities::Wcsicmp(x.c_str(), y.c_str()) == 0;
        }

    // find methods, code adapted from microsoft's xstring header.
    WString::size_type Find(const WString& Object, const WString& X, WString::size_type P = 0) const
        {
        return (Find(Object, X.c_str(), P, X.size()));
        }
    WString::size_type Find(const WString& Object, const WChar* S, WString::size_type P, WString::size_type N) const
        {
        WString::size_type _Len = Object.size();
        const WChar* _Ptr = Object.c_str();
        if (N == 0 && P <= _Len)
            return (P);
        WString::size_type _Nm;
        if (P < _Len && N <= (_Nm = _Len - P))
            {
            const WChar* U, *V;
            for (_Nm -= N - 1, V = _Ptr + P;
                 (U = traitsfind(V, _Nm, *S)) != 0;
                 _Nm -= U - V + 1, V = U + 1)
                if (traitscompare(U, S, N) == 0)
                    return (U - _Ptr);
            }
        return (WString::npos);
        }
    WString::size_type Find(const WString& Object, const WChar* S, WString::size_type P = 0) const
        {
        return (Find(Object, S, P, std::char_traits<WChar>::length(S)));
        }
    WString::size_type Find(const WString& Object, WChar C, WString::size_type P = 0) const
        {
        return (Find(Object, (const WChar*)&C, P, 1));
        }

    void ToLower(WString& Object) const
        {
        // TR 128069 the BeStringUtilities::Wcslwr failed with an empty string when Regional Options
        // is set to something else than English(xxxx).  We don't know the real cause
        // of this crash because we can't debug the function.
        if(!Object.empty())
            BeStringUtilities::Wcslwr(&(Object[0]));
        }

    void ToUpper(WString& Object) const
        {
        // TR 128069 the BeStringUtilities::Wcsupr failed with an empty string when Regional Options
        // is set to something else than English(xxxx).  We don't know the real cause
        // of this crash because we can't debug the function.
        if(!Object.empty())
            BeStringUtilities::Wcsupr(&(Object[0]));
        }
    };

END_IMAGEPP_NAMESPACE