/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/PrivateAPI/base/bcGmcNorm.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if !defined (mdl) && !defined (resource)
#pragma warning(disable: 4102) 
#endif

#define BC_START() \
         int __bc__status; \
         int __bc__status_return; \
         __bc__status = SUCCESS; \
         __bc__status_return = SUCCESS

#define BC_RETURN() \
        goto __label_exit

#define BC_RETURN_ARG(arg) \
        { \
            arg_return = arg; \
            goto __label_exit; \
        }

#define BC_RETURN_ERRSTATUS(errStatus) \
        { \
            __bc__status_return = errStatus; \
            goto __label_exit; \
        }

#define BC_RETURN_ERRSTATUS_CLEANUP(errStatus) \
        { \
            __bc__status_return = errStatus; \
            goto __label_exit_error; \
        }

#define BC_END \
        __label_exit

#define BC_END_ERRORCLEANUP \
        goto __label_return; \
        __label_exit_error

#define BC_END_RETURNSTATUS() \
         { \
            __label_return: \
            return (DTMStatusInt)(__bc__status_return); \
         }

#define BC_END_RETURNARG \
         { \
            __label_return: \
            return (arg_return); \
         }

#define BC_TRY(call) \
   { \
      __bc__status = (call); \
      if (__bc__status > SUCCESS) BC_RETURN_ERRSTATUS(__bc__status); \
   }

#define BC_TRY_CLEANUP(call) \
   { \
      __bc__status = (call); \
      if (__bc__status > SUCCESS) BC_RETURN_ERRSTATUS_CLEANUP(__bc__status); \
   }

#define BC_STATUSFLAG __bc__status_return

#define BC_ERRORFLAG __bc__status

#define  BC_CLASSID(version, objNum) \
   RTYPE(version,((objNum)/(16*16))%16,((objNum)/16)%16,(objNum)%16)

#define  BC_NONERSCID  0xffff
