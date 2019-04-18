/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
declare type CxxCodeTemplate = string;

interface CxxType<JsType, CxxName, JsName, ConversionToJsOperation, ConversionToCxxOperation> {}

interface BeJsProjection_RefCounted {}

interface BeJsProjection_SuppressConstructor {}

interface BeJsProjection_SuppressBeProjectedCasts {}

interface BeJsProjection_IsInterface {}

interface BeJsProjection_SuppressDisposeDelete {}

interface BeJsProjection_ValueType {}

interface IDisposable
    {
    Dispose() : void;
    OnDispose() : void;
    }

declare type cxx_int8_t = CxxType<number,
    CxxCodeTemplate /*** int8_t ***/,
    CxxCodeTemplate /*** Number ***/,
    CxxCodeTemplate /*** BentleyApi::BeJsNumber __name__ (__context__, static_cast<int8_t>(__object__)); ***/,
    CxxCodeTemplate /*** int8_t __name__ = static_cast<int8_t>(__object__.GetIntegerValue()); ***/>;

declare type cxx_int16_t = CxxType<number,
    CxxCodeTemplate /*** int16_t ***/,
    CxxCodeTemplate /*** Number ***/,
    CxxCodeTemplate /*** BentleyApi::BeJsNumber __name__ (__context__, static_cast<int16_t>(__object__)); ***/,
    CxxCodeTemplate /*** int16_t __name__ = static_cast<int16_t>(__object__.GetIntegerValue()); ***/>;

declare type cxx_int32_t = CxxType<number,
    CxxCodeTemplate /*** int32_t ***/,
    CxxCodeTemplate /*** Number ***/,
    CxxCodeTemplate /*** BentleyApi::BeJsNumber __name__ (__context__, static_cast<int32_t>(__object__)); ***/,
    CxxCodeTemplate /*** int32_t __name__ = __object__.GetIntegerValue(); ***/>;

declare type cxx_size_t = CxxType<number,
    CxxCodeTemplate /*** size_t ***/,
    CxxCodeTemplate /*** Number ***/,
    CxxCodeTemplate /*** BentleyApi::BeJsNumber __name__ (__context__, static_cast<uint32_t>(__object__)); ***/,
    CxxCodeTemplate /*** size_t __name__ = __object__.GetUnsignedIntegerValue(); ***/>;

declare type cxx_uint8_t = CxxType<number,
    CxxCodeTemplate /*** uint8_t ***/,
    CxxCodeTemplate /*** Number ***/,
    CxxCodeTemplate /*** BentleyApi::BeJsNumber __name__ (__context__, static_cast<uint8_t>(__object__)); ***/,
    CxxCodeTemplate /*** uint8_t __name__ = static_cast<uint8_t>(__object__.GetUnsignedIntegerValue()); ***/>;

declare type cxx_uint16_t = CxxType<number,
    CxxCodeTemplate /*** uint16_t ***/,
    CxxCodeTemplate /*** Number ***/,
    CxxCodeTemplate /*** BentleyApi::BeJsNumber __name__ (__context__, static_cast<uint16_t>(__object__)); ***/,
    CxxCodeTemplate /*** uint16_t __name__ = static_cast<uint16_t>(__object__.GetUnsignedIntegerValue()); ***/>;

declare type cxx_uint32_t = CxxType<number,
    CxxCodeTemplate /*** uint32_t ***/,
    CxxCodeTemplate /*** Number ***/,
    CxxCodeTemplate /*** BentleyApi::BeJsNumber __name__ (__context__, static_cast<uint32_t>(__object__)); ***/,
    CxxCodeTemplate /*** uint32_t __name__ = __object__.GetUnsignedIntegerValue(); ***/>;

declare type cxx_int64_t = CxxType<string,
    CxxCodeTemplate /*** int64_t ***/,
    CxxCodeTemplate /*** String ***/,
    CxxCodeTemplate /*** BentleyApi::BeJsString __name__ = BentleyApi::BeJsString::CreateFromInt64 (__context__, static_cast<int64_t>(__object__)); ***/,
    CxxCodeTemplate /*** int64_t __name__ = __object__.GetValueAsInt64(); ***/>;

declare type cxx_uint64_t = CxxType<string,
    CxxCodeTemplate /*** uint64_t ***/,
    CxxCodeTemplate /*** String ***/,
    CxxCodeTemplate /*** BentleyApi::BeJsString __name__ = BentleyApi::BeJsString::CreateFromUnsignedInt64 (__context__, static_cast<uint64_t>(__object__)); ***/,
    CxxCodeTemplate /*** uint64_t __name__ = __object__.GetValueAsUnsignedInt64(); ***/>;

declare type cxx_uintptr_t = CxxType<string,
    CxxCodeTemplate /*** uintptr_t ***/,
    CxxCodeTemplate /*** String ***/,
    CxxCodeTemplate /*** BentleyApi::BeJsString __name__ = BentleyApi::BeJsString::CreateFromUnsignedInt64 (__context__, static_cast<uint64_t>(__object__)); ***/,
    CxxCodeTemplate /*** uintptr_t __name__ = static_cast<uintptr_t>(__object__.GetValueAsUnsignedInt64()); ***/>;

declare type cxx_double = CxxType<number,
    CxxCodeTemplate /*** double ***/,
    CxxCodeTemplate /*** Number ***/,
    CxxCodeTemplate /*** BentleyApi::BeJsNumber __name__ (__context__, static_cast<double>(__object__)); ***/,    
    CxxCodeTemplate /*** double __name__ = __object__.GetValue(); ***/>;

declare type cxx_bool = CxxType<boolean,
    CxxCodeTemplate /*** bool ***/,
    CxxCodeTemplate /*** Boolean ***/,
    CxxCodeTemplate /*** BentleyApi::BeJsBoolean __name__ (__context__, static_cast<bool>(__object__)); ***/,
    CxxCodeTemplate /*** bool __name__ = __object__.GetValue(); ***/>;

declare class cxx_pointer<T> implements CxxType<T,
    CxxCodeTemplate /*** __T__* ***/,
    CxxCodeTemplate /*** NativePointer ***/,
    CxxCodeTemplate /*** BentleyApi::BeJsNativePointer __name__ = __context__.ObtainProjectedClassInstancePointer (__object__, false, __customPrototype__); ***/,
    CxxCodeTemplate /*** __T__* __name__ = __object__.GetValueTyped<__T__>(); ***/> {};

declare class cxx_owner_pointer<T> implements CxxType<T,
    CxxCodeTemplate /*** __T__* ***/,
    CxxCodeTemplate /*** NativePointer ***/,
    CxxCodeTemplate /*** BentleyApi::BeJsNativePointer __name__ = __context__.ObtainProjectedClassInstancePointer (__object__, true, __customPrototype__); ***/,
    CxxCodeTemplate /*** __T__* __name__ = __object__.GetValueTyped<__T__>(); ***/> {};

declare class cxx_reference<T> implements CxxType<T,
    CxxCodeTemplate /*** __T__& ***/,
    CxxCodeTemplate /*** NativePointer ***/,
    CxxCodeTemplate /*** BentleyApi::BeJsNativePointer __name__ = __context__.ObtainProjectedClassInstancePointer (&__object__, false, __customPrototype__); ***/,
    CxxCodeTemplate /*** __T__& __name__ = __object__.GetValueReference<__T__>(); ***/> {};

declare class cxx_owner_reference<T> implements CxxType<T,
    CxxCodeTemplate /*** __T__& ***/,
    CxxCodeTemplate /*** NativePointer ***/,
    CxxCodeTemplate /*** BentleyApi::BeJsNativePointer __name__ = __context__.ObtainProjectedClassInstancePointer (&__object__, true, __customPrototype__)); ***/,
    CxxCodeTemplate /*** __T__& __name__ = __object__.GetValueReference<__T__>(); ***/> {};

declare type cxx_const_char_pointer = CxxType<string,
    CxxCodeTemplate /*** char const* ***/,
    CxxCodeTemplate /*** String ***/,
    CxxCodeTemplate /*** BentleyApi::BeJsString __name__ (__context__, static_cast<char const*>(__object__)); ***/,

    CxxCodeTemplate /***
Utf8String __name__String = __object__.GetValue();
char const* __name__ = __name__String.c_str();
    ***/>;

declare class cxx_enum_class_uint8_t<T> implements CxxType<T,
    CxxCodeTemplate /*** __T__ ***/,
    CxxCodeTemplate /*** Number ***/,
    CxxCodeTemplate /*** BentleyApi::BeJsNumber __name__ (__context__, static_cast<uint8_t>(__object__)); ***/,
    CxxCodeTemplate /*** __T__ __name__ = static_cast<__T__>(__object__.GetUnsignedIntegerValue()); ***/> {};

declare class cxx_enum_class_uint16_t<T> implements CxxType<T,
    CxxCodeTemplate /*** __T__ ***/,
    CxxCodeTemplate /*** Number ***/,
    CxxCodeTemplate /*** BentleyApi::BeJsNumber __name__ (__context__, static_cast<uint16_t>(__object__)); ***/,
    CxxCodeTemplate /*** __T__ __name__ = static_cast<__T__>(__object__.GetUnsignedIntegerValue()); ***/> {};

declare class cxx_enum_class_uint32_t<T> implements CxxType<T,
    CxxCodeTemplate /*** __T__ ***/,
    CxxCodeTemplate /*** Number ***/,
    CxxCodeTemplate /*** BentleyApi::BeJsNumber __name__ (__context__, static_cast<uint32_t>(__object__)); ***/,
    CxxCodeTemplate /*** __T__ __name__ = static_cast<__T__>(__object__.GetUnsignedIntegerValue()); ***/> {};

declare type Bentley_Utf8CP = cxx_const_char_pointer;

declare type Bentley_Utf8String = CxxType<string,
    CxxCodeTemplate /*** Utf8String ***/,
    CxxCodeTemplate /*** String ***/,
    CxxCodeTemplate /*** BeJsString __name__ (__context__, __object__); ***/,
    CxxCodeTemplate /*** Utf8String __name__ = __object__.GetValue(); ***/>;

declare class Bentley_RefCountedPtr<T> implements CxxType<T,
    CxxCodeTemplate /*** RefCountedPtr<__T__> ***/,
    CxxCodeTemplate /*** NativePointer ***/,

    CxxCodeTemplate /***
BentleyApi::BeJsNativePointer __name__ = __context__.ObtainProjectedClassInstancePointer (__object__.get(), true, __customPrototype__);
    ***/,

    CxxCodeTemplate /*** BentleyApi::RefCountedPtr<__T__> __name__ = __object__.GetValueTyped<__T__>(); ***/> {};

declare class Bentley_BeJsObject<T> implements CxxType<T,
    CxxCodeTemplate /*** BentleyApi::BeJsObject ***/,
    CxxCodeTemplate /*** Object ***/,
    CxxCodeTemplate /*** BentleyApi::BeJsObject& __name__ = __object__; ***/,
    CxxCodeTemplate /*** BentleyApi::BeJsObject __name__ = __object__.GetValue(); ***/> {};

declare type Bentley_BeJsFunction = CxxType<() => void,
    CxxCodeTemplate /*** BentleyApi::BeJsFunction ***/,
    CxxCodeTemplate /*** Function ***/,
    CxxCodeTemplate /*** BentleyApi::BeJsFunction& __name__ = __object__; ***/,
    CxxCodeTemplate /*** BentleyApi::BeJsFunction __name__ = __object__.GetValue(); ***/>;

declare class Bentley_BeJsArray<T> implements CxxType<T,
    CxxCodeTemplate /*** BentleyApi::BeJsArray ***/,
    CxxCodeTemplate /*** Array ***/,
    CxxCodeTemplate /*** BentleyApi::BeJsArray& __name__ = __object__; ***/,
    CxxCodeTemplate /*** BentleyApi::BeJsArray __name__ = __object__.GetValue(); ***/> {};
