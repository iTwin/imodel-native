/*--------------------------------------------------------------------------------------+
|
|     $Source: include/json/value.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#include "forwards.h"
#include <Bentley/WString.h>
#include <Bentley/bvector.h>
#include <Bentley/bmap.h>
#include <deque>
#include <stack>
#include <cmath>

#define BE_JSON_NAME(__val__) static constexpr Json::StaticString json_##__val__() {return Json::StaticString(#__val__);}

BEGIN_BENTLEY_NAMESPACE

typedef Json::Value& JsonValueR;
typedef Json::Value const& JsonValueCR;
typedef Json::Value const* JsonValueCP;

/** \brief JsonCpp JSON (JavaScript Object Notation) library.
 */
namespace Json {

class Features
{
   public:
      /** \brief A configuration that allows all features and assumes all strings are UTF-8.
       * - C & C++ comments are allowed
       * - Root object can be any JSON value
       */
      static Features all() {return Features();}

      /** \brief A configuration that is strictly compatible with the JSON specification.
       * - Comments are forbidden.
       * - Root object must be either an array or an object value.
       * - Assumes Value strings are encoded in UTF-8
       */
      static Features strictMode() {Features features; features.allowComments_ = false; features.strictRoot_ = true; return features;}

      /** \brief Initialize the configuration like JsonConfig::allFeatures;
       */
      Features() {}

      /// \c true if comments are allowed. Default: \c true.
      bool allowComments_ = true;

      /// \c true if root must be either an array or an object value. Default: \c false.
      bool strictRoot_ = false;
};

/** \brief Type of the value held by a Value object.
*/
enum ValueType : unsigned char
{
  nullValue = 0, ///< 'null' value
  intValue,      ///< signed integer value
  uintValue,     ///< unsigned integer value
  realValue,     ///< double value
  stringValue,   ///< Utf8 string value
  booleanValue,  ///< bool value
  arrayValue,    ///< array value (ordered list)
  objectValue    ///< object value (collection of name/value pairs).
};

/** \brief Lightweight wrapper to tag static string.
*
* Value constructor and objectValue member assignement takes advantage of the
* StaticString and avoid the cost of string duplication when storing the
* string or the member name.
*
* Example of usage:
* \code
* Json::Value aValue(StaticString("some text"));
* Json::Value object;
* static const StaticString code("code");
* object[code] = 1234;
* \endcode
*/
class StaticString
{
private:
  Utf8CP m_str;

public:
  constexpr explicit StaticString(Utf8CP str) : m_str(str) {}
  operator Utf8CP() const {return m_str;}
  Utf8CP c_str() const {return m_str;}
};

/** \brief Outputs a Value in <a HREF="http://www.json.org">JSON</a> format without formatting (not human friendly).
    *
    * The JSON document is written in a single line. It is not intended for 'human' consumption,
    * but may be usefull to support feature such as RPC where bandwith is limited.
    * \sa Reader, Value
    */
class FastWriter 
{
private:
    Utf8String document_;
    void writeValue(JsonValueCR);

public:
    JSON_API Utf8String write(JsonValueCR root);
    JSON_API static Utf8String ToString(JsonValueCR root);
};

/** Unserialize a <a HREF="http://www.json.org">JSON</a> document into a Value. */
class Reader
{
   public:
      typedef char Char;
      typedef const Char *Location;

      /** \brief Constructs a Reader allowing all features
       * for parsing.
       */
      Reader() : features_(Features::all()) {}

      /** \brief Constructs a Reader allowing the specified feature set
       * for parsing.
       */
      Reader(Features const& features) : features_(features){}

      /** \brief Read a Value from a <a HREF="http://www.json.org">JSON</a> document.
       * \param document UTF-8 encoded string containing the document to read.
       * \param root [out] Contains the root value of the document if it was
       *             successfully parsed.
       * \param collectComments \c true to collect comment and allow writing them back during
       *                        serialization, \c false to discard comments.
       *                        This parameter is ignored if Features::allowComments_
       *                        is \c false.
       * \return \c true if the document was successfully parsed, \c false if an error occurred.
       */
      bool parse(Utf8StringCR document, Value& root, bool collectComments = true) 
        {
        document_ = document; 
        Utf8CP begin = document_.c_str(); Utf8CP end = begin + document_.length();
        return parse(begin, end, root, collectComments);
        }

      static bool Parse(Utf8StringCR document, Value& root, bool comments=true) {Reader reader; return reader.parse(document, root, comments);}

      static Value DoParse(Utf8StringCR in, bool comments=true);

      /** \brief Read a Value from a <a HREF="http://www.json.org">JSON</a> document.
       * \param beginDoc Pointer on the beginning of the UTF-8 encoded string of the document to read.
       * \param endDoc Pointer on the end of the UTF-8 encoded string of the document to read. 
       \               Must be >= beginDoc.
       * \param root [out] Contains the root value of the document if it was
       *             successfully parsed.
       * \param collectComments \c true to collect comment and allow writing them back during
       *                        serialization, \c false to discard comments.
       *                        This parameter is ignored if Features::allowComments_
       *                        is \c false.
       * \return \c true if the document was successfully parsed, \c false if an error occurred.
       */
      JSON_API bool parse(Utf8CP beginDoc, Utf8CP endDoc, Value& root, bool collectComments=true);

      /** \brief Returns a user friendly string that list errors in the parsed document.
       * \return Formatted error message with the list of errors with their location in 
       *         the parsed document. An empty string is returned if no error occurred
       *         during parsing.
       */
      JSON_API Utf8String getFormattedErrorMessages() const;

   private:
      enum TokenType
      {
         tokenEndOfStream = 0,
         tokenObjectBegin,
         tokenObjectEnd,
         tokenArrayBegin,
         tokenArrayEnd,
         tokenString,
         tokenNumber,
         tokenTrue,
         tokenFalse,
         tokenNull,
         tokenArraySeparator,
         tokenMemberSeparator,
         tokenComment,
         tokenError
      };

      class Token
      {
      public:
         TokenType type_;
         Location start_;
         Location end_;
      };

      class ErrorInfo
      {
      public:
         Token token_;
         Utf8String message_;
         Location extra_;
      };

      typedef std::deque<ErrorInfo> Errors;

      bool expectToken( TokenType type, Token &token, const char *message );
      bool readToken( Token &token );
      void skipSpaces();
      bool match( Location pattern, 
                  int patternLength );
      bool readComment();
      bool readCStyleComment();
      bool readCppStyleComment();
      bool readString();
      void readNumber();
      bool readValue();
      bool readObject(Token &token);
      bool readArray(Token &token);
      bool decodeNumber(Token &token);
      bool decodeString(Token &token);
      bool decodeString(Token &token, Utf8String &decoded);
      bool decodeDouble(Token &token);
      bool decodeUnicodeCodePoint(Token &token, Location &current, Location end, unsigned int &unicode);
      bool decodeUnicodeEscapeSequence(Token &token, Location &current, Location end, unsigned int &unicode);
      bool addError(const Utf8String &message, Token &token, Location extra = 0);
      bool recoverFromError(TokenType skipUntilToken);
      bool addErrorAndRecover(const Utf8String &message, Token &token, TokenType skipUntilToken);
      void skipUntilSpace();
      Value &currentValue();
      Char getNextChar();
      void getLocationLineAndColumn(Location location, int &line, int &column) const;
      Utf8String getLocationLineAndColumn(Location location) const;
      void skipCommentTokens(Token &token);

      typedef std::stack<Value*> Nodes;
      Nodes nodes_;
      Errors errors_;
      Utf8String document_;
      Location begin_;
      Location end_;
      Location current_;
      Features features_;
};

   /** \brief Represents a <a HREF="http://www.json.org">JSON</a> value.
    *
    * This class is a discriminated union wrapper that can represents a:
    * - signed integer [range: Value::minInt - Value::maxInt]
    * - unsigned integer (range: 0 - Value::maxUInt)
    * - double
    * - UTF-8 string
    * - boolean
    * - 'null'
    * - an ordered list of Value
    * - collection of name/value pairs (javascript object)
    *
    * The type of the held value is represented by a #ValueType and 
    * can be obtained using type().
    *
    * values of an #objectValue or #arrayValue can be accessed using operator[]() methods. 
    * Non const methods will automatically create the a #nullValue element 
    * if it does not exist. 
    * The sequence of an #arrayValue will be automatically resize and initialized 
    * with #nullValue. resize() can be used to enlarge or truncate an #arrayValue.
    *
    * The get() methods can be used to obtain default value in the case the required element
    * does not exist.
    *
    * It is possible to iterate over the list of a #objectValue values using 
    * the getMemberNames() method.
    */
class JSON_API Value 
{
      friend class ValueIteratorBase;
   public:
      typedef bvector<Utf8String> Members;
      typedef ValueIterator iterator;
      typedef ValueConstIterator const_iterator;
      typedef Json::UInt UInt;
      typedef Json::Int Int;
      typedef Json::UInt32 UInt32;
      typedef Json::UInt64 UInt64;
      typedef Json::Int64 Int64;
      typedef Json::LargestInt LargestInt;
      typedef Json::LargestUInt LargestUInt;
      typedef Json::ArrayIndex ArrayIndex;

      //! Get a const reference to a NULL Json::Value
      //! NOTE: this used to be a public static variable called "null". That's a bad idea. If you're looking at old code that
      //! doesn't compile because it has Json::Value::null, simply change it to Json::Value::GetNull().
      static JsonValueCR GetNull();

      //! Minimum signed integer value that can be stored in a Json::Value.
      static constexpr LargestInt minLargestInt(){return LargestInt(~(LargestUInt(-1)/2));}

      //! Maximum signed integer value that can be stored in a Json::Value.
      static constexpr LargestInt maxLargestInt() {return LargestInt(LargestUInt(-1)/2);}

      //! Maximum unsigned integer value that can be stored in a Json::Value.
      static constexpr LargestUInt maxLargestUInt() {return LargestUInt(-1);}

      //! Minimum signed int value that can be stored in a Json::Value.
      static constexpr Int minInt() {return Int(~(UInt(-1)/2));}

      //! Maximum signed int value that can be stored in a Json::Value.
      static constexpr Int maxInt() {return Int(UInt(-1)/2);}

      //! Maximum unsigned int value that can be stored in a Json::Value.
      static constexpr UInt maxUInt() {return UInt(-1);}

      //! Minimum signed 64 bits int value that can be stored in a Json::Value.
      static constexpr Int64 minInt64() {return Int64(~(UInt64(-1)/2));}

      //! Maximum signed 64 bits int value that can be stored in a Json::Value.
      static constexpr Int64 maxInt64() {return Int64(UInt64(-1)/2);}

      //! Maximum unsigned 64 bits int value that can be stored in a Json::Value.
      static constexpr UInt64 maxUInt64() {return UInt64(-1);}

   private:
      class CZString 
      {
      public:
         static Utf8P Malloc(unsigned int size) {return (Utf8P ) bentleyAllocator_malloc(size);}
         static void Free(Utf8P str) {bentleyAllocator_free((void*) str);}
        
         static Utf8P Duplicate(Utf8CP value, unsigned int length=0)
          {
          if (length == 0) length = (unsigned int)strlen(value);
          char *newString = Malloc(length+1);
          memcpy(newString, value, length);
          newString[length] = 0;
          return newString;
          }
        
         enum DuplicationPolicy {noDuplication = 0, duplicate, duplicateOnCopy};
         CZString() : cstr_(nullptr), index_(0) {}
         CZString(ArrayIndex index) : cstr_(nullptr), index_(index) {}
         CZString(Utf8CP cstr, DuplicationPolicy policy) : cstr_(policy==duplicate ? Duplicate(cstr) : cstr), index_(policy){}
         CZString(CZString const& other) : cstr_(other.index_ != noDuplication && other.cstr_ != nullptr ?  Duplicate(other.cstr_) : other.cstr_), 
                                           index_(other.cstr_ ? (other.index_ == noDuplication ? noDuplication : duplicate) : other.index_){}
         ~CZString() {if (cstr_ && index_ == duplicate) Free(const_cast<Utf8P>(cstr_));}
         CZString& operator=(CZString const& other) {CZString temp(other); swap(temp); return *this;}
         bool operator<(CZString const& other) const {if (cstr_) return strcmp(cstr_, other.cstr_) < 0; return index_ < other.index_;}
         bool operator==(CZString const& other) const {if (cstr_) return strcmp(cstr_, other.cstr_) == 0; return index_ == other.index_;}
         ArrayIndex index() const {return index_;}
         Utf8CP c_str() const {return cstr_;}
         bool isStaticString() const {return index_ == noDuplication;}
      private:
         void swap(CZString& other) {std::swap(cstr_, other.cstr_); std::swap(index_, other.index_);}
         Utf8CP cstr_;
         ArrayIndex index_;
      };

   public:
      typedef bmap<CZString, Value> ObjectValues;

   public:
      /** \brief Create a default Value of the given type.

        This is a very useful constructor.
        To create an empty array, pass arrayValue.
        To create an empty object, pass objectValue.
        Another Value can then be set to this one by assignment.
    This is useful since clear() and resize() will not alter types.

        Examples:
    \code
    Json::Value null_value; // null
    Json::Value arr_value(Json::arrayValue); // []
    Json::Value obj_value(Json::objectValue); // {}
    \endcode
      */
      Value(ValueType type=ValueType::nullValue) : type_(type), allocated_(0)
        {
        switch (type)
           {
           case nullValue:
              break;
           case intValue:
           case uintValue:
              value_.int_ = 0;
              break;
           case realValue:
              value_.real_ = 0.0;
              break;
           case stringValue:
              value_.string_ = 0;
              break;
           case arrayValue:
           case objectValue:
              value_.map_ = new ObjectValues();
              break;
           case booleanValue:
              value_.bool_ = false;
              break;
           }
        }

      Value(Int value) : type_(intValue) {value_.int_ = value;}
      Value(UInt value) : type_(uintValue) {value_.uint_ = value;}

#if defined (__APPLE__)
#if !defined (__LP64__)
      Value(UInt32 value); // BeJsonCpp
#else
      Value(long value);
      Value(unsigned long value);
#endif
#endif
      // NB: The constructors accepting 64-bit integers are marked explicit to ensure that callers do not unintentionally
      // assign 64-bit integers to JSON values.
      // Javascript does not support 64-bit precision for integer values - if the JSON representation of values outside of
      // the supported range are transmitted to a Javascript context, errors may result.
      // If your 64-bit integer represents an ID (such as a BeInt64Id), use BeInt64Id::ToHexStr() instead.
      // If the value of your 64-bit integer is known not to exceed 32 bits, cast it to a uint32_t instead.
      // If you really want to store a 64-bit integer, invoke the constructor explicitly.
      explicit Value(Int64 value) : type_(intValue) {value_.int_ = value;}
      explicit Value(UInt64 value) : type_(uintValue) {value_.uint_ = value;}

      Value(double value) : type_(realValue) {if (std::isnan(value)) {BeAssert(false); type_ = nullValue; return;} value_.real_ = value;}
      Value(Utf8CP value) : type_(stringValue), allocated_(true) {value_.string_ = CZString::Duplicate(value ? value : "");}
      Value(Utf8CP beginValue, Utf8CP endValue) : type_(stringValue), allocated_(true) {value_.string_ = CZString::Duplicate(beginValue, (unsigned int)(endValue - beginValue));}

      /** \brief Constructs a value from a static string.

       * Like other value string constructor but do not duplicate the string for
       * internal storage. The given string must remain alive after the call to this
       * constructor.
       * Example of usage:
       * \code
       * Json::Value aValue(StaticString("some text"));
       * \endcode
       */
      Value(StaticString const& value) : type_(stringValue), allocated_(false) {value_.string_ = const_cast<char *>(value.c_str());}
      Value(Utf8StringCR value) : type_(stringValue), allocated_(true) {value_.string_ = CZString::Duplicate(value.c_str(), (unsigned int)value.length());}
      Value(bool value) : type_(booleanValue) {value_.bool_ = value;}
      Value(JsonValueCR other) : type_(other.type_)
        {
        switch (type_)
           {
           case nullValue:
           case intValue:
           case uintValue:
           case realValue:
           case booleanValue:
              value_ = other.value_;
              break;
           case stringValue:
              if (other.value_.string_)
              {
                 value_.string_ = CZString::Duplicate(other.value_.string_);
                 allocated_ = true;
              }
              else
                 value_.string_ = nullptr;
              break;
           case arrayValue:
           case objectValue:
              value_.map_ = new ObjectValues(*other.value_.map_);
              break;
           }
        }

      Value(Value&& other) : type_(ValueType::nullValue), allocated_(false) {swap(other);}
      ~Value()
        {
        switch (type_)
           {
           case nullValue:
           case intValue:
           case uintValue:
           case realValue:
           case booleanValue:
              break;
           case stringValue:
              if (allocated_) CZString::Free(value_.string_);
              break;
           case arrayValue:
           case objectValue:
              delete value_.map_;
              break;
           }
        }

      static Value From(Utf8StringCR document) {Value val; Reader reader; return reader.parse(document, val) ? val : GetNull();}
      static Value From(Utf8CP beginDoc, Utf8CP endDoc) {Value val; Reader reader; return reader.parse(beginDoc, endDoc, val) ? val : GetNull();}

      JsonValueR operator= (JsonValueCR other) {Value temp(other); swap(temp); return *this;}
      JsonValueR operator= (Value&& other) {swap(other); return *this;}

      /// Swap values.
      void swap(JsonValueR other)
        {
        std::swap(type_, other.type_);
        std::swap(value_, other.value_);
        std::swap(allocated_, other.allocated_);
        }

      ValueType type() const {return type_;}

      bool operator <(const Value &other) const;
      bool operator <=(const Value &other) const {return !(other < *this);}
      bool operator >=(const Value &other) const {return !(*this < other);}
      bool operator >(const Value &other) const {return other < *this;}
      bool operator ==(const Value &other) const;
      bool operator !=(const Value &other) const {return !(*this == other);}

      int compare(const Value &other) const {if (*this < other) return -1; if (*this > other) return 1; return 0;}

      Utf8CP asCString(Utf8CP defaultVal="") const
        {
        switch (type_)
           {
           case stringValue:
              return value_.string_;
           case booleanValue:
              return value_.bool_ ? "true" : "false";
           case intValue:
           case uintValue:
           case realValue:
           case arrayValue:
           case objectValue:
              BeAssert(false);
           default:
                break;
           }
        return defaultVal;
        }

      Utf8String asString(Utf8CP defaultVal="") const {return Utf8String(asCString(defaultVal));}
      Int asInt(Int defaultVal=0) const;
      UInt asUInt(UInt defaultVal=0) const;
      Int64 asInt64(Int64 defaultVal=0) const;
      UInt64 asUInt64(UInt64 defaultVal=0) const;
      LargestInt asLargestInt(LargestInt defaultVal=0) const {return asInt64(defaultVal);}
      LargestUInt asLargestUInt(LargestUInt defaultVal=0) const {return asUInt64(defaultVal);}
      float asFloat(float defaultVal=0.0f) const;
      double asDouble(double defaultVal=0.0) const;
      bool asBool(bool defaultVal=false) const;

      bool isNull() const {return type_ == nullValue;}
      bool isBool() const {return type_ == booleanValue;}
      bool isInt() const {return type_ == intValue;}
      bool isUInt() const {return type_ == uintValue;}
      bool isIntegral() const {return type_ == intValue || type_ == uintValue || type_ == booleanValue;}
      bool isDouble() const {return type_ == realValue;}
      bool isNumeric() const {return isIntegral() || isDouble();}
      bool isString() const {return type_ == stringValue;}
      bool isArray() const {return type_ == nullValue || type_ == arrayValue;}
      bool isObject() const {return type_ == nullValue || type_ == objectValue;}
      bool isConvertibleTo(ValueType other) const;

      /// Number of values in array or object
      ArrayIndex size() const;

      /// \brief Return true if empty array, empty object, or null;
      /// otherwise, false.
      bool empty() const {if (isArray() || isObject()) return size() == 0u; return false;}

      /// Return isNull()
      bool operator!() const {return isNull();}

      /// Remove all object members and array elements.
      /// \pre type() is arrayValue, objectValue, or nullValue
      /// \post type() is unchanged
      void clear();

      /// Resize the array to size elements. 
      /// New elements are initialized to null.
      /// May only be called on nullValue or arrayValue.
      /// \pre type() is arrayValue or nullValue
      /// \post type() is arrayValue
      void resize(ArrayIndex size);

      /// Access an array element (zero based index).
      /// If the array contains less than index element, then null value are inserted
      /// in the array so that its size is index+1.
      /// (You may need to say 'value[0u]' to get your compiler to distinguish
      ///  this from the operator[] which takes a string.)
      JsonValueR operator[](ArrayIndex index);

      /// Access an array element (zero based index).
      /// If the array contains less than index element, then null value are inserted
      /// in the array so that its size is index+1.
      /// (You may need to say 'value[0u]' to get your compiler to distinguish
      ///  this from the operator[] which takes a string.)
      JsonValueR operator[](int index) {BeAssert(index >= 0); return (*this)[ArrayIndex(index)];}

      /// Access an array element (zero based index)
      /// (You may need to say 'value[0u]' to get your compiler to distinguish
      ///  this from the operator[] which takes a string.)
      JsonValueCR operator[](ArrayIndex index) const;

      /// Access an array element (zero based index)
      /// (You may need to say 'value[0u]' to get your compiler to distinguish
      ///  this from the operator[] which takes a string.)
      JsonValueCR operator[](int index) const  {BeAssert(index >= 0); return (*this)[ArrayIndex(index)];}

      /// If the array contains at least index+1 elements, returns the element value, 
      /// otherwise returns defaultValue.
      Value get(ArrayIndex index, JsonValueCR defaultValue) const {Value const* value = &((*this)[index]); return value == &GetNull() ? defaultValue : *value;}

      /// Return true if index < size().
      bool isValidIndex(ArrayIndex index) const {return index < size();}

      /// \brief Append value to array at the end.
      ///
      /// Equivalent to jsonvalue[jsonvalue.size()] = value;
      JsonValueR append(JsonValueCR value) {return (*this)[size()] = value;}

      /// Access an object value by name, create a null member if it does not exist.
      JsonValueR operator[](Utf8CP key) {return resolveReference(key, false);}

      /// Access an object value by name, returns null if there is no member with that name.
      JsonValueCR operator[](Utf8CP key) const
        {
        if (isNull() || type_ != objectValue)
            return GetNull(); // don't assert that this is a problem, it's not.

        auto it = value_.map_->find(CZString(key, CZString::noDuplication));
        return (it == value_.map_->end()) ? GetNull() : (*it).second;
        }

      /// Access an object value by name, create a null member if it does not exist.
      JsonValueR operator[](Utf8StringCR key) {return (*this)[key.c_str()];}

      /// Access an object value by name, returns null if there is no member with that name.
      JsonValueCR operator[](Utf8StringCR key) const {return (*this)[key.c_str()];}

      /** \brief Access an object value by name, create a null member if it does not exist.

       * If the object as no entry for that name, then the member name used to store
       * the new entry is not duplicated.
       * Example of use:
       * \code
       * Json::Value object;
       * static const StaticString code("code");
       * object[code] = 1234;
       * \endcode
       */
      JsonValueR operator[](StaticString const& key) {return resolveReference(key, true);}

      /// Return the member named key if it exists, defaultValue otherwise.
      Value get(Utf8CP key, JsonValueCR defaultValue) const {Value const* value = &((*this)[key]); return value == &GetNull() ? defaultValue : *value;}

      /// Return the member named key if it exist, defaultValue otherwise.
      Value get(Utf8StringCR key, JsonValueCR defaultValue) const {return get(key.c_str(), defaultValue);}

      /// \brief Remove and return the named member.  
      ///
      /// Do nothing if it did not exist.
      /// \return the removed Value, or null.
      /// \pre type() is objectValue or nullValue
      /// \post type() is unchanged
      Value removeMember(Utf8CP key);

      /// Same as removeMember(Utf8CP )
      Value removeMember(Utf8StringCR key) {return removeMember(key.c_str());}

      void SetOrRemoveInt(Utf8CP key, Int val, Int defaultVal) {if (val==defaultVal) removeMember(key); else (*this)[key] = val;}
      void SetOrRemoveUInt(Utf8CP key, UInt val, UInt defaultVal) {if (val==defaultVal) removeMember(key); else (*this)[key] = val;}
      void SetOrRemoveInt64(Utf8CP key, Int64 val, Int64 defaultVal) {if (val==defaultVal) removeMember(key); else (*this)[key] = Value(val);}
      void SetOrRemoveUInt64(Utf8CP key, UInt64 val, UInt64 defaultVal) {if (val==defaultVal) removeMember(key); else (*this)[key] = Value(val);}
      void SetOrRemoveDouble(Utf8CP key, double val, double defaultVal) {if (val==defaultVal) removeMember(key); else (*this)[key] = val;}
      void SetOrRemoveBool(Utf8CP key, bool val, bool defaultVal) {if (val==defaultVal) removeMember(key); else (*this)[key] = val;}

      /** \brief Remove the indexed array element.

          O(n) expensive operations.
          Update 'removed' iff removed.
          \return true iff removed (no exceptions)
          */
      bool removeIndex(ArrayIndex i, Value* removed);
      bool removeIndex(ArrayIndex i) {Value removed; return removeIndex(i, &removed);}

      /// Return true if the object has a member named key.
      bool isMember(Utf8CP key) const {JsonValueCP value = &((*this)[key]); return value != &GetNull();}

      /// Return true if the object has a member named key.
      bool isMember(Utf8StringCR key) const {return isMember(key.c_str());}

      /// \brief Return a list of the member names.
      ///
      /// If null, return an empty list.
      /// \pre type() is objectValue or nullValue
      /// \post if type() was nullValue, it remains nullValue
      Members getMemberNames() const;

      Utf8String ToString() const {return FastWriter::ToString(*this);}

      Utf8String toStyledString() const;

      const_iterator begin() const;
      const_iterator end() const;

      iterator begin();
      iterator end();

   private:
      JsonValueR resolveReference(Utf8CP key, bool isStatic)
        {
        if (type_ != objectValue)
            {
            BeAssert(isNull()); // we're about to lose current value
            BeAssert(this != &GetNull()); // somebody did a const_cast of GetNull()? That's a disaster
            *this = Value(objectValue);
            }

        CZString actualKey(key, isStatic ? CZString::noDuplication : CZString::duplicateOnCopy);
        auto it = value_.map_->lower_bound(actualKey);
        if (it != value_.map_->end() && (*it).first == actualKey)
            return (*it).second;

        ObjectValues::value_type defaultValue(actualKey, GetNull());
        it = value_.map_->insert(it, defaultValue);
        return (*it).second;
        }

      union ValueHolder
      {
         LargestInt int_;
         LargestUInt uint_;
         double real_;
         bool bool_;
         char *string_;
         ObjectValues *map_;
      } value_;
      ValueType type_;
      bool allocated_;
};


   /** \brief base class for Value iterators.
    *
    */
   class ValueIteratorBase
   {
   public:
      typedef unsigned int size_t;
      typedef int difference_type;
      typedef ValueIteratorBase SelfType;

      ValueIteratorBase();
      explicit ValueIteratorBase(const Value::ObjectValues::iterator &current);

      bool operator ==(const SelfType &other) const {return isEqual(other);}
      bool operator !=(const SelfType &other) const {return !isEqual(other);}

      difference_type operator -(const SelfType &other) const {return computeDistance(other);}

      /// Return either the index or the member name of the referenced value as a Value.
      Value key() const;

      /// Return the index of the referenced Value. -1 if it is not an arrayValue.
      UInt index() const;

      /// Return the member name of the referenced Value. "" if it is not an objectValue.
      Utf8CP memberName() const;

   protected:
      JsonValueR deref() const;

      void increment();

      void decrement();

      difference_type computeDistance(const SelfType &other) const;

      bool isEqual(const SelfType &other) const;

      void copy(const SelfType &other);

   private:
      Value::ObjectValues::iterator current_;
      // Indicates that iterator is for a null value.
      bool isNull_;
   };

   /** \brief const iterator for object and array value.
    *
    */
   class ValueConstIterator : public ValueIteratorBase
   {
      friend class Value;
   public:
      typedef unsigned int size_t;
      typedef int difference_type;
      typedef JsonValueCR reference;
      typedef JsonValueCP pointer;
      typedef ValueConstIterator SelfType;

      ValueConstIterator();
   private:
      /*! \internal Use by Value to create an iterator.
       */
      explicit ValueConstIterator(const Value::ObjectValues::iterator &current);
   public:
      SelfType &operator =(const ValueIteratorBase &other);

      SelfType operator++(int)
      {
         SelfType temp(*this);
         ++*this;
         return temp;
      }

      SelfType operator--(int)
      {
         SelfType temp(*this);
         --*this;
         return temp;
      }

      SelfType &operator--()
      {
         decrement();
         return *this;
      }

      SelfType &operator++()
      {
         increment();
         return *this;
      }

      reference operator *() const
      {
         return deref();
      }
   };

   /** \brief Iterator for object and array value.
    */
   class ValueIterator : public ValueIteratorBase
   {
      friend class Value;
   public:
      typedef unsigned int size_t;
      typedef int difference_type;
      typedef JsonValueR reference;
      typedef Value *pointer;
      typedef ValueIterator SelfType;

      ValueIterator();
      ValueIterator(const ValueConstIterator &other);
      ValueIterator(const ValueIterator &other);
   private:
      /*! \internal Use by Value to create an iterator.
       */
      explicit ValueIterator(const Value::ObjectValues::iterator &current);
   public:

      SelfType &operator =(const SelfType &other);

      SelfType operator++(int)
      {
         SelfType temp(*this);
         ++*this;
         return temp;
      }

      SelfType operator--(int)
      {
         SelfType temp(*this);
         --*this;
         return temp;
      }

      SelfType &operator--()
      {
         decrement();
         return *this;
      }

      SelfType &operator++()
      {
         increment();
         return *this;
      }

      reference operator *() const
      {
         return deref();
      }
   };

    inline Value Reader::DoParse(Utf8StringCR in, bool comments) {Value val; return Parse(in, val, comments) ? val : Value::GetNull();}
} // namespace Json

END_BENTLEY_NAMESPACE

