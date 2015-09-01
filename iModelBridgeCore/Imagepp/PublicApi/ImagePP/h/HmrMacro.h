//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/h/HmrMacro.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <Bentley/Bentley.h>
#include <Bentley/RefCounted.h>


#if !defined(MAX)
#   define MAX(a,b)                 ((a)>(b)?(a):(b))
#endif

#if !defined(MIN)
#   define MIN(a,b)                 ((a)<(b)?(a):(b))
#endif

#if !defined(BOUND)
#define BOUND(x,min,max)            ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))
#endif

#if !defined(LIMIT_RANGE)
#define LIMIT_RANGE(min,max,val)    {if ((val) < (min)) val = min; else if ((val) > (max)) val = max;}
#endif

#if !defined(IN_RANGE)
#define IN_RANGE(x,min,max)         (((x) >= (min)) && ((x) <= (max)))
#endif

#define BEGIN_IMAGEPP_NAMESPACE              BEGIN_BENTLEY_NAMESPACE namespace ImagePP {
#define END_IMAGEPP_NAMESPACE                } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_IMAGEPP              using namespace BentleyApi::ImagePP;

#define IMAGEPP_TYPEDEFS(_name_) \
    BEGIN_IMAGEPP_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_IMAGEPP_NAMESPACE 

#define IMAGEPP_REF_COUNTED_PTR(_sname_) \
    BEGIN_IMAGEPP_NAMESPACE struct _sname_; DEFINE_REF_COUNTED_PTR(_sname_) END_IMAGEPP_NAMESPACE

// Define __HMR_DEBUG if NDEBUG is NOT defined
#ifndef NDEBUG
#   ifndef __HMR_DEBUG
#       define __HMR_DEBUG
#   endif
#endif

#include <malloc.h>

#include <Bentley/BeAssert.h>
#include <stdlib.h>
#include <time.h>

// To activate dump in file :
#   if (1)
#       define HDUMP_USEFILE
#       define HDUMP_FILENAME "C:\\ImagePP.err"
#   endif


// DUMP macro setting valid only in C++
#if defined (__cplusplus)
#   ifdef HDUMP_USEFILE
#       ifndef HDUMP_FILENAME
#           error Dump file name not defined
#       endif
#       include <iostream>
#       include <fstream>
#   endif
#endif

/* If in denug mode than activate printstate */
#ifdef __HMR_DEBUG
#   define __HMR_PRINTSTATE
#endif

/*====================================================================
**  Definition de macro
**====================================================================*/

/*
** -----------------------------------------------------------------------
**  HPRINTSTATE Public name for __HMR_PRINTSTATE
**
** -----------------------------------------------------------------------
*/
#ifdef __HMR_PRINTSTATE
#   define HPRINTSTATE
#endif

/*
** -----------------------------------------------------------------------
**  HVERIFYCONTRACT - Automatiaclly defined in debug mode
**
** -----------------------------------------------------------------------
*/
#ifdef __HMR_DEBUG
#   define HVERIFYCONTRACT
#endif

// Validate that every classID is defined in ImagePPClassId.
//#if defined(__IMAGEPP_BUILD__) || defined(__IPPIMAGING_BUILD__)
    #define IPPCLASSIDVALIDATE(ClassID) HDEBUGCODE(ImagePPClassId t=ClassID;t;);
//#else
//    #define IPPCLASSIDVALIDATE(ClassID)
//#endif

/*
** Macro used to assign an ID and implement the method GetClassID().
** Used HDECLARE_BASECLASS_ID for the class without parent, and
**      HDECLARE_CLASS_ID for the descendent classes.
**      HDECLARE_SEALEDCLASS_ID for class with no parent and no descendent (GetClassID is not virtual).
**
** Normaly called at the begining of the declaration, if the object is
** not persistent, because the Persistence Macro makes the same definition.
*/
#define HDECLARE_BASECLASS_ID(ClassID) \
    public: \
        enum { CLASS_ID = ClassID }; \
        virtual HCLASS_ID GetClassID() const {IPPCLASSIDVALIDATE(ClassID) return CLASS_ID; } \
        virtual bool IsCompatibleWith(HCLASS_ID pi_ClassID) const \
            { return (CLASS_ID == pi_ClassID); }

#define HDECLARE_CLASS_ID(ClassID, pi_Ancestor) \
    public: \
        enum { CLASS_ID = ClassID }; \
        DEFINE_T_SUPER(pi_Ancestor)   \
        virtual HCLASS_ID GetClassID() const override {IPPCLASSIDVALIDATE(ClassID) return CLASS_ID; } \
        virtual bool IsCompatibleWith(HCLASS_ID pi_ClassID) const override \
            { return (CLASS_ID == pi_ClassID) ? true : pi_Ancestor::IsCompatibleWith(pi_ClassID); }

#define HDECLARE_SEALEDCLASS_ID(ClassID) \
    public: \
    enum { CLASS_ID = ClassID }; \
    HCLASS_ID GetClassID() const {IPPCLASSIDVALIDATE(ClassID) return CLASS_ID; } \
    bool IsCompatibleWith(HCLASS_ID pi_ClassID) const \
            { return (CLASS_ID == pi_ClassID); }


/*
** -----------------------------------------------------------------------
**  HSTATICASSERT(static_expr) - Validate a compile time condition.
**                               The compiler will fail compiling the line
**                               of the static assertion if "static_expr"
**                               is false. Could be placed at any scope
**                               (function/class/global).
**  e.g.: HSTATICASSERT(sizeof(m_Data) == sizeof(m_Data2))
**          -> Will not compile if members differ in size
**
** -----------------------------------------------------------------------
*/
#define HSTATICASSERTMSG(expr, exprMsg) static_assert((int)(expr), exprMsg)
#define HSTATICASSERT(expr) HSTATICASSERTMSG(expr, "Static Assertion.")


/*
** -----------------------------------------------------------------------
**  HASSERT(expr) - ...
**
** -----------------------------------------------------------------------
*/
#if defined(__HMR_DEBUG) || defined(HVERIFYCONTRACT)

#   if defined(__HMR_REDIRECT_ASSERTPATH)
inline void RedirectedAssert(bool pi_Success, WCharCP pi_pExpr, WCharCP pi_pFile, size_t pi_Line)
    {
    static const char* ASSERT_LOG_FILE_PATH = getenv("__HMR_REDIRECT_ASSERTPATH");
    if (!pi_Success)
        {
        FILE* Exist = fopen(ASSERT_LOG_FILE_PATH, "r");
        if (0 == Exist)
            {
            _wassert(pi_pExpr, pi_pFile, static_cast<unsigned int>(pi_Line));
            return;
            }
        fclose(Exist);

        FILE* LogFile = fopen(ASSERT_LOG_FILE_PATH, "a");
        if (0 != LogFile)
            {
            time_t rawtime;
            time ( &rawtime );
            fwprintf(LogFile, L"%s %s(%d): %s\n", _wctime(&rawtime), pi_pFile, pi_Line, pi_pExpr);
            fclose(LogFile);
            }
        }
    }

#       define REDIRECTED_ASSERT(expr) RedirectedAssert((int)(expr), _CRT_WIDE(#expr), _CRT_WIDE(__FILE__), __LINE__)
#       define  HASSERT(expr) REDIRECTED_ASSERT(expr)
#       define  HASSERT_X64(expr) REDIRECTED_ASSERT(expr)
#       define  HASSERT_DATA(expr) REDIRECTED_ASSERT(expr)

#   else
#       define HASSERT(expr) BeAssert(expr)
#       define HASSERT_X64(expr) BeAssert(expr)        // I use this one to identify special assert related to x64 only.
#       define HASSERT_DATA(expr) BeDataAssert(expr)

#   endif
#else
#   define HASSERT(expr)
#   define HASSERT_X64(expr)
#   define HASSERT_DATA(expr) BeDataAssert(expr)
#endif


/*
** -----------------------------------------------------------------------
**  HASSERTSUPERDEBUG(expr) - ...
**
** -----------------------------------------------------------------------
*/
#if defined(__HMR_SUPERDEBUG)
#   define HASSERTSUPERDEBUG(expr) HASSERT(expr)
#else
#   define HASSERTSUPERDEBUG(expr)
#endif



/*
** -----------------------------------------------------------------------
**  HPRECONDITION(expr) - Assertion on entry conditions
**
**                        See HASSERT(expr)
**
** -----------------------------------------------------------------------
*/
#if !defined(HVERIFYCONTRACT)
#   define HPRECONDITION(expr)
#   define HPRECONDITION_T(...)
#else
#   define HPRECONDITION(expr) HASSERT((expr))
#   define HPRECONDITION_T(...) HASSERT((__VA_ARGS__))
#endif

/*
** -----------------------------------------------------------------------
**  HPRECONDITIONSHAREABLE() - Assertion if the ref count was 0
**
**                        See HASSERT(expr)
**
** -----------------------------------------------------------------------
*/
#if !defined(HVERIFYCONTRACT)
#   define HPRECONDITIONSHAREABLE()
#else
#   define HPRECONDITIONSHAREABLE() HASSERT(GetRefCount() > 0)
#endif

/*
** -----------------------------------------------------------------------
**  HCONTRACTCODE(expr) - contract verification
**
**                         See HASSERT(expr)
**
** -----------------------------------------------------------------------
*/
#if !defined(HVERIFYCONTRACT)
#   define HCONTRACTCODE(expr)
#else
#   define HCONTRACTCODE(expr) expr
#endif


/*
** -----------------------------------------------------------------------
**  HPOSTCONDITION(expr) - Assertion on exit conditions
**
**                         See HASSERT(expr)
**
** -----------------------------------------------------------------------
*/
#if !defined(HVERIFYCONTRACT)
#   define HPOSTCONDITION(expr)
#else
#   define HPOSTCONDITION(expr) HASSERT(expr)
#endif


/*
** -----------------------------------------------------------------------
**  HINVARIANTS - Assertion on consistant object state
**
**                See HASSERT(expr)
**
** -----------------------------------------------------------------------
*/
#if !defined(HVERIFYCONTRACT)
#   define HINVARIANTS
#else
#   define HINVARIANTS  ValidateInvariants()
#endif


/*
** -----------------------------------------------------------------------
**  HVERIFY(expr) - Allows to enter a line of code that will have its return value
**                  compared to true in debug only. In release the line is
**                  executed but the return value not checked.
**
**  Example:  HVERIFY(testBool;);
** -----------------------------------------------------------------------
*/
#if !defined(__HMR_DEBUG)
#   define HVERIFY(expr) expr
#else
#   define HVERIFY(expr) HASSERT(expr)
#endif


/*
** -----------------------------------------------------------------------
**  HVERIFYRETURNVALUE(expr, expectedValue) - Allows to enter a line of 
**                  code that will have its return value
**                  compared to the specified expected return value in debug only. 
**                  In release the line is
**                  executed but the return value not checked.
**
**  Example:  HVERIFYRETURNVALUE(strcmp("a", "b"), 0);
** -----------------------------------------------------------------------
*/
#if !defined(__HMR_DEBUG)
#   define HVERIFYRETURNVALUE(expr, expectedValue) expr
#else
#   define HVERIFYRETURNVALUE(expr, expectedValue) HASSERT(expr == expectedValue)
#endif


/*
** -----------------------------------------------------------------------
**  HDEBUGCODE(expr) - Allows to enter a line of code that compiles only
**                     in debug mode
**
**  Example:  HDEBUGCODE(int i=0;);
**            HDEBUGCODE(printf("I: %d\n",i););
** -----------------------------------------------------------------------
*/
#if !defined(__HMR_DEBUG)
#   define HDEBUGCODE(expr)
#else
#   define HDEBUGCODE(expr) expr
#endif



/*
** -----------------------------------------------------------------------
**  HDUMP0(text)
**
** -----------------------------------------------------------------------
*/
#if !defined(__HMR_DEBUG) || !defined(__HMR_PRINTSTATE) || !defined(HDUMP_USEFILE)
#   define HDUMP0(text)
#else
#   define HDUMP0(text)    { \
                               filebuf HMRRESERVEDDUMPFile; \
                               HMRRESERVEDDUMPFile.open(HDUMP_FILENAME, ios::out | ios::app); \
                               ostream HMRRESERVEDOstream(&HMRRESERVEDDUMPFile); \
                               HMRRESERVEDOstream << text << endl; \
                               HMRRESERVEDDUMPFile.close(); \
                           }
#endif

#define HDUMPDEVICE cerr

/*
** -----------------------------------------------------------------------
**  HDUMP1(format, obj)
**
** -----------------------------------------------------------------------
*/
#if !defined(__HMR_DEBUG) || !defined(__HMR_PRINTSTATE) || !defined(HDUMP_USEFILE)
#   define HDUMP1(format, obj)
#else
#   define HDUMP1(format, obj){ \
                                   char HMRRESERVEDDebugText[512]; \
                                   sprintf(HMRRESERVEDDebugText, format, obj); \
                                   filebuf HMRRESERVEDDUMPFile; \
                                   HMRRESERVEDDUMPFile.open(HDUMP_FILENAME, ios::out | ios::app); \
                                   ostream HMRRESERVEDOstream(&HMRRESERVEDDUMPFile); \
                                   HMRRESERVEDOstream << HMRRESERVEDDebugText << endl; \
                                   HMRRESERVEDDUMPFile.close(); \
                               }
#endif

/*
** -----------------------------------------------------------------------
**  HDUMP2(format, obj1, obj2)
**
** -----------------------------------------------------------------------
*/
#if !defined(__HMR_DEBUG) || !defined(__HMR_PRINTSTATE) || !defined(HDUMP_USEFILE)
#   define HDUMP2(format, obj1, obj2)
#else
#   define HDUMP2(format, obj1, obj2){ \
                                         char HMRRESERVEDDebugText[512]; \
                                         sprintf(HMRRESERVEDDebugText, format, obj1, obj2); \
                                         filebuf HMRRESERVEDDUMPFile; \
                                         HMRRESERVEDDUMPFile.open(HDUMP_FILENAME, ios::out | ios::app); \
                                         ostream HMRRESERVEDOstream(&HMRRESERVEDDUMPFile); \
                                         HMRRESERVEDOstream << HMRRESERVEDDebugText << endl; \
                                         HMRRESERVEDDUMPFile.close(); \
                                     }
#endif

/*
** -----------------------------------------------------------------------
**  HDUMP3(format, obj1, obj2, obj3)
**
** -----------------------------------------------------------------------
*/
#if !defined(__HMR_DEBUG) || !defined(__HMR_PRINTSTATE) || !defined(HDUMP_USEFILE)
#   define HDUMP3(format, obj1, obj2, obj3)
#else
#   define HDUMP3(format, obj1, obj2, obj3)    { \
                                           char HMRRESERVEDDebugText[512]; \
                                           sprintf(HMRRESERVEDDebugText, format, obj1, obj2, obj3); \
                                           filebuf HMRRESERVEDDUMPFile; \
                                           HMRRESERVEDDUMPFile.open(HDUMP_FILENAME, ios::out | ios::app); \
                                           ostream HMRRESERVEDOstream(&HMRRESERVEDDUMPFile); \
                                           HMRRESERVEDOstream << HMRRESERVEDDebugText << endl; \
                                           HMRRESERVEDDUMPFile.close(); \
                                       }
#endif


/*
** -----------------------------------------------------------------------
**  HASSERTDUMP(expr, filename, obj) - ...
**
** -----------------------------------------------------------------------
*/
#if !defined(__HMR_DEBUG) && !defined(HVERIFYCONTRACT)
#   define HASSERTDUMP(expr, obj)
#else
#    ifdef __HMR_PRINTSTATE
#       define HASSERTDUMP(expr,obj)  if (!(expr)) \
                                      { \
                                         time_t HMRRESERVEDDegugTime_tStruct; \
                                         time(&HMRRESERVEDDegugTime_tStruct); \
                                         HDUMP1("Assertion Dump %s\n" ,ctime(&HMRRESERVEDDegugTime_tStruct)); \
                                         HDUMP2("For file %s at line%d\n" ,__FILE__, __LINE__); \
                                         (obj).PrintState(HDUMPDEVICE); \
                                         assert(expr); \
                                      }
#    else
#       define HASSERTDUMP(expr,obj)  assert(expr)
#    endif
#endif

/*
** -----------------------------------------------------------------------
**  HASSERTDUMP2(expr, filename, obj1, obj2) - ...
**
** -----------------------------------------------------------------------
*/
#if !defined(__HMR_DEBUG) && !defined(HVERIFYCONTRACT)
#   define HASSERTDUMP2(expr, obj1, obj2)
#else
#    ifdef __HMR_PRINTSTATE
#       define HASSERTDUMP2(expr, obj1, obj2) if (!(expr)) \
                                              { \
                                                  time_t HMRRESERVEDDegugTime_tStruct; \
                                                  time(&HMRRESERVEDDegugTime_tStruct); \
                                                  HDUMP1("Assertion Dump %s\n" ,ctime(&HMRRESERVEDDegugTime_tStruct)); \
                                                  HDUMP2("For file %s at line%d\n" ,__FILE__, __LINE__); \
                                                  (obj1).PrintState(HDUMPDEVICE); \
                                                  (obj2).PrintState(HDUMPDEVICE); \
                                                  assert(expr); \
                                              }
#    else
#       define HASSERTDUMP2(expr, obj1, obj2) assert(expr)
#    endif
#endif

/*
** -----------------------------------------------------------------------
**  HASSERTDUMP3(expr, filename, obj1, obj2, obj3) - ...
**
** -----------------------------------------------------------------------
*/
#if !defined(__HMR_DEBUG) && !defined(HVERIFYCONTRACT)
#   define HASSERTDUMP3(expr, obj1, obj2, obj3)
#else
#    ifdef __HMR_PRINTSTATE
#       define HASSERTDUMP3(expr, obj1, obj2, obj3) if (!(expr)) \
                                                    { \
                                                        time_t HMRRESERVEDDegugTime_tStruct; \
                                                        time(&HMRRESERVEDDegugTime_tStruct); \
                                                        HDUMP1("Assertion Dump %ld\n" ,ctime(&HMRRESERVEDDegugTime_tStruct)); \
                                                        HDUMP2("For file %s at line%d\n" ,__FILE__, __LINE__); \
                                                        (obj1).PrintState(HDUMPDEVICE); \
                                                        (obj2).PrintState(HDUMPDEVICE); \
                                                        (obj3).PrintState(HDUMPDEVICE); \
                                                        assert(expr); \
                                                    }
#    else
#       define HASSERTDUMP3(expr, obj1, obj2, obj3) assert(expr)
#    endif
#endif

/*
** -----------------------------------------------------------------------
**  HASSERTDUMP4(expr, filename, obj1, obj2, obj3, obj4) - ...
**
** -----------------------------------------------------------------------
*/
#if !defined(__HMR_DEBUG) && !defined(HVERIFYCONTRACT)
#   define HASSERTDUMP4(expr, obj1, obj2, obj3, obj4)
#else
#    ifdef __HMR_PRINTSTATE
#       define HASSERTDUMP4(expr, obj1, obj2, obj3, obj4) if (!(expr)) \
                                                          { \
                                                              time_t HMRRESERVEDDegugTime_tStruct; \
                                                              time(&HMRRESERVEDDegugTime_tStruct); \
                                                              HDUMP1("Assertion Dump %ld\n" ,ctime(&HMRRESERVEDDegugTime_tStruct)); \
                                                              HDUMP2("For file %s at line%d\n" ,__FILE__, __LINE__); \
                                                              (obj1).PrintState(HDUMPDEVICE); \
                                                              (obj2).PrintState(HDUMPDEVICE); \
                                                              (obj3).PrintState(HDUMPDEVICE); \
                                                              (obj4).PrintState(HDUMPDEVICE); \
                                                              assert(expr); \
                                                          }

#    else
#       define HASSERTDUMP4(expr, obj1, obj2, obj3, obj4) assert(expr)
#    endif
#endif

/*
** -----------------------------------------------------------------------
**  HASSERTDUMP5(expr, filename, obj1, obj2, obj3, obj4, obj5) - ...
**
** -----------------------------------------------------------------------
*/
#if !defined(__HMR_DEBUG) && !defined(HVERIFYCONTRACT)
#   define HASSERTDUMP5(expr, obj1, obj2, obj3, obj4, obj5)
#else
#    ifdef __HMR_PRINTSTATE
#       define HASSERTDUMP5(expr, obj1, obj2, obj3, obj4, obj5) if (!(expr)) \
                                                                { \
                                                                   time_t HMRRESERVEDDegugTime_tStruct; \
                                                                   time(&HMRRESERVEDDegugTime_tStruct); \
                                                                   HDUMP1("Assertion Dump %ld\n" ,ctime(&HMRRESERVEDDegugTime_tStruct)); \
                                                                   HDUMP2("For file %s at line%d\n" ,__FILE__, __LINE__); \
                                                                   (obj1).PrintState(HDUMPDEVICE); \
                                                                   (obj2).PrintState(HDUMPDEVICE); \
                                                                   (obj3).PrintState(HDUMPDEVICE); \
                                                                   (obj4).PrintState(HDUMPDEVICE); \
                                                                   (obj5).PrintState(HDUMPDEVICE); \
                                                                   assert(expr); \
                                                                }
#    else
#       define HASSERTDUMP5(expr, obj1, obj2, obj3, obj4, obj5)     assert(expr)
#    endif
#endif


/*
** --------------------------------------------------------------------------
**  HWARNING(text) -
**
**     Dump the 'text' to the visual studio debug output window.
** --------------------------------------------------------------------------
*/

#if defined(__HMR_DEBUG) && defined(_WIN32)
#define HDEBUGTEXTW(text)  OutputDebugStringW(text);
#define HDEBUGTEXTA(text)  OutputDebugStringA(text);
#else
#define HDEBUGTEXTW(text)
#define HDEBUGTEXTA(text)
#endif

#define HDEBUGTEXT(text) HDEBUGTEXTW(text)

/*
** --------------------------------------------------------------------------
**  HWARNING(condition, text) -
**
**     If the condtion is not respected, dump the 'text' to the visual studio
**     debug output window.
** --------------------------------------------------------------------------
*/

#if defined(__HMR_DEBUG) && defined(_WIN32)
#define HWARNING(condition, text)  if (!(int)(condition))     \
                                       {                          \
                                           HDEBUGTEXT(text)       \
                                       }
#else
#define HWARNING(condition, text)
#endif


/*
** --------------------------------------------------------------------------
**  Fast Floating-Point-to-Integer Conversion
**
**  The routine adds the "magic number" 2E52 + 2E51 to the source operand, then
**  store the double precision result to memory and retrives the lower
**  doubleword of the stored result. Adding the magic number shifts the original
**  argument to the right inside the double precision mantissa, placing the
**  binary point of the sum immediately to the right of the least significant
**  mantissa bit.  Extracting the lower double word of the sum then delivers the
**  integral portion of the original argument.
**
**  N.B. This code uses the current rounding when performing the conversion
**       which is always round to nearest.  You should used these routines only
**       when rounding precision is not a factor.
** --------------------------------------------------------------------------
*/
//#define OPTI_DOUBLE2INT_CONVERSION      	// comment this line to disable DOUBLE2INT optimization.
											// Make sure DOUBLE2INT routines are working correctly in a non WIN32 environment !!!

#if defined(OPTI_DOUBLE2INT_CONVERSION)

// Same result as round(d) except when the value equal .5. In this case the result will
// differ by 1.  ex. DOUBLE2INT_Fast(2.5) = 2, round(2.5) = 3. If you don't care about
// that use DOUBLE2INT_Fast instead of DOUBLE2INT_Round.
#define DOUBLE2INT_Fast(i, d) \
        { \
        HASSERT(sizeof(d) == sizeof(double)); HASSERT(sizeof(i) == sizeof(int)); \
        double double2IntTempVar(((d)+6755399441055744.0)); \
        i = *((int *)(&double2IntTempVar)); \
        }

// This routine tries to match the result of round(d) but in a more efficient way.
// The result may differ by 1 when the value is very close to .5(epsilon 1E-300).
#define DOUBLE2INT_Round(i, d) \
        { \
        HASSERT(sizeof(d) == sizeof(double)); HASSERT(sizeof(i) == sizeof(int)); \
        double double2IntTempVar; \
        if((d) > 0.0) \
            double2IntTempVar = (((d)+(1E-300))+6755399441055744.0); \
        else \
            double2IntTempVar = (((d)-(1E-300))+6755399441055744.0); \
        HASSERT(*((int *)(&double2IntTempVar)) == round(d)); \
        i = *((int *)(&double2IntTempVar)); \
        }

// Same thing as i = (int)d, but faster.
// The result may differ by 1 when the value is very close to 1.0. ex. 4.99998542 will give 5.0 instead of 4.0
#define DOUBLE2INT_Trunc(i, d) \
        { \
        HASSERT(sizeof(d) == sizeof(double)); HASSERT(sizeof(i) == sizeof(int)); \
        double double2IntTempVar; \
        if((d) > 0.0) \
            double2IntTempVar = (((d)-0.49999)+6755399441055744.0); \
        else \
            double2IntTempVar = (((d)+0.49999)+6755399441055744.0); \
        HASSERT(*((int *)(&double2IntTempVar)) == (int)d); \
        i = *((int *)(&double2IntTempVar)); \
        }
#else
#define DOUBLE2INT_Fast(i, d) i=(int)d

#define DOUBLE2INT_Round(i, d) i=round(d)

#define DOUBLE2INT_Trunc(i, d) i=(int)d;
#endif

/*
** --------------------------------------------------------------------------
**  HDOUBLE_EQUAL(v1, v2, precision) -
**
**      if (HDOUBLE_EQUAL (v1, v2, 0.000001))
** --------------------------------------------------------------------------
*/
#define HGLOBAL_EPSILON (0.0000001)
#define HEPSILON_MULTIPLICATOR (1E-13)
#define HMAX_EPSILON           (2E+294)


#define HDOUBLE_EQUAL_EPSILON(v1,v2)  ((v1 <= (v2+HGLOBAL_EPSILON)) && (v1 >= (v2-HGLOBAL_EPSILON)))
#define HDOUBLE_EQUAL(v1,v2,precision)  ((v1 <= (v2+precision)) && (v1 >= (v2-precision)))

#define HDOUBLE_GREATER_OR_EQUAL_EPSILON(v1, v2) (v1 >= (v2-HGLOBAL_EPSILON))
#define HDOUBLE_GREATER_OR_EQUAL(v1, v2, precision) (v1 >= (v2-precision))

#define HDOUBLE_SMALLER_OR_EQUAL_EPSILON(v1, v2) (v1 <= (v2+HGLOBAL_EPSILON))
#define HDOUBLE_SMALLER_OR_EQUAL(v1, v2, precision) (v1 <= (v2+precision))

#define HDOUBLE_GREATER_EPSILON(v1, v2) (v1 > (v2+HGLOBAL_EPSILON))
#define HDOUBLE_GREATER(v1, v2, precision) (v1 > (v2+precision))

#define HDOUBLE_SMALLER_EPSILON(v1, v2) (v1 < (v2-HGLOBAL_EPSILON))
#define HDOUBLE_SMALLER(v1, v2, precision) (v1 < (v2-precision))


#if defined(__HMR_DEBUG)

#define CHECK_HUINT64_TO_HDOUBLE_CONV(pi_IntVal) \
{ \
    double DbVal = (double)pi_IntVal; \
    uint64_t ConvIntVal = (uint64_t)DbVal; \
    HASSERT(ConvIntVal == pi_IntVal); \
}

#define CHECK_HSINT64_TO_HDOUBLE_CONV(pi_IntVal) \
{ \
    double DbVal = (double)pi_IntVal; \
    int64_t ConvIntVal = (int64_t)DbVal; \
    HASSERT(ConvIntVal == pi_IntVal); \
}

#else

#define CHECK_HUINT64_TO_HDOUBLE_CONV(pi_IntVal)
#define CHECK_HSINT64_TO_HDOUBLE_CONV(pi_IntVal)

#endif

#if !defined(hmin)
#    define hmin(a,b) ((a)<(b)?(a):(b))
#endif

#if !defined(hmax)
#   define hmax(a,b) ((a)>(b)?(a):(b))
#endif

#if defined (ANDROID) || defined (__APPLE__)

#elif defined (_WIN32)
#   if !defined(round)
#       define round(a) ((long)((a)<0.0?(a)-0.5:(a)+0.5))
#   endif
#else
#   error Unknown compiler.
#endif

/*
** --------------------------------------------------------------------------
**  HNOVTABLEINIT
**
**  No vtable pointer initialization. Use on classes that will never be
**  instantiated directly (virtual base classes).
** --------------------------------------------------------------------------
*/
#ifdef _WIN32
#    define HNOVTABLEINIT   __declspec(novtable)
#else
#    define HNOVTABLEINIT
#endif
