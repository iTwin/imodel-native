/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#if !defined(JSON_IS_AMALGAMATION)
# include <json/value.h>
# include <json/writer.h>

#endif // if !defined(JSON_IS_AMALGAMATION)
#include <utility>
#include <stdexcept>
#include <cstring>
#include <cstddef>    // size_t
#include <Bentley/BeAssert.h>
#include <math.h>

#define JSON_ASSERT_UNREACHABLE BeAssert(false)
#define JSON_ASSERT(condition) BeAssert(condition);  // @todo <= change this into an exception throw
#define JSON_FAIL_MESSAGE(message) {BeAssert(false && message);}
#define JSON_ASSERT_MESSAGE(condition, message) if (!(condition)) JSON_FAIL_MESSAGE(message)

BEGIN_BENTLEY_NAMESPACE

#if !defined(JSON_IS_AMALGAMATION)
# include "json_valueiterator.inl"
#endif // if !defined(JSON_IS_AMALGAMATION)

namespace Json {

JsonValueCR Value::GetNull() {static Value s_null; return s_null;}

#if defined (__APPLE__)
#if !defined (__LP64__)
Value::Value(UInt32 value) : type_(uintValue)
{
    // see MacTypes.h for conflicting definition of UInt32
   value_.uint_ = (UInt) value;
}
#else
Value::Value(long value) : type_(intValue)
{
    // see MacTypes.h for conflicting definition of UInt32
   value_.uint_ = value;
}
Value::Value(unsigned long value) : type_(uintValue)
{
    // see MacTypes.h for conflicting definition of UInt32
   value_.uint_ = value;
}
#endif
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool Value::operator <(JsonValueCR other) const
{
   int typeDelta = type_ - other.type_;
   if (typeDelta)
      return typeDelta < 0 ? true : false;
   switch (type_)
   {
   case nullValue:
      return false;
   case intValue:
      return value_.int_ < other.value_.int_;
   case uintValue:
      return value_.uint_ < other.value_.uint_;
   case realValue:
      return value_.real_ < other.value_.real_;
   case booleanValue:
      return value_.bool_ < other.value_.bool_;
   case stringValue:
      return (value_.string_ == 0  &&  other.value_.string_)
             || (other.value_.string_  
                  &&  value_.string_  
                  && strcmp(value_.string_, other.value_.string_) < 0);
   case arrayValue:
   case objectValue:
      {
         int delta = int(value_.map_->size() - other.value_.map_->size());
         if (delta)
            return delta < 0;
         return (*value_.map_) < (*other.value_.map_);
      }
   default:
      JSON_ASSERT_UNREACHABLE;
   }
   return false;  // unreachable
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool Value::operator ==(JsonValueCR other) const
{
   if (type_ != other.type_)
      return false;

   switch (type_)
   {
   case nullValue:
      return true;
   case intValue:
      return value_.int_ == other.value_.int_;
   case uintValue:
      return value_.uint_ == other.value_.uint_;
   case realValue:
      return value_.real_ == other.value_.real_;
   case booleanValue:
      return value_.bool_ == other.value_.bool_;
   case stringValue:
      return (value_.string_ == other.value_.string_)
             || (other.value_.string_  
                  &&  value_.string_  
                  && strcmp(value_.string_, other.value_.string_) == 0);
   case arrayValue:
   case objectValue:
      return (*value_.map_) == (*other.value_.map_);
   default:
      JSON_ASSERT_UNREACHABLE;
   }
   return false;  // unreachable
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Value::Int Value::asInt(Int defaultVal) const
{
   switch (type_)
   {
   case nullValue:
      return defaultVal;

   case intValue:
      JSON_ASSERT_MESSAGE(value_.int_ >= minInt()  &&  value_.int_ <= maxInt(), "unsigned integer out of signed int range");
      return Int(value_.int_);

   case uintValue:
      JSON_ASSERT_MESSAGE(value_.uint_ <= UInt(maxInt()), "unsigned integer out of signed int range");
      return Int(value_.uint_);

   case realValue:
      JSON_ASSERT_MESSAGE(value_.real_ >= minInt()  &&  value_.real_ <= maxInt(), "Real out of signed integer range");
      return Int(value_.real_);

   case booleanValue:
      return value_.bool_ ? 1 : 0;

   case stringValue:
        {
        Int val = defaultVal;
        int stat = sscanf(value_.string_, "%" PRId32, &val);
        BeAssert(stat == 1);
        UNUSED_VARIABLE(stat);;
        return val;
        }
   }
    JSON_FAIL_MESSAGE("Type is not convertible to int");
    return defaultVal;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Value::UInt Value::asUInt(UInt defaultVal) const
{
   switch (type_)
   {
   case nullValue:
      return defaultVal;

   case intValue:
      JSON_ASSERT_MESSAGE(value_.int_ >= 0, "Negative integer can not be converted to unsigned integer");
      JSON_ASSERT_MESSAGE(value_.int_ <= maxUInt(), "signed integer out of UInt range");
      return UInt(value_.int_);

   case uintValue:
      JSON_ASSERT_MESSAGE(value_.uint_ <= maxUInt(), "unsigned integer out of UInt range");
      return UInt(value_.uint_);

   case realValue:
      JSON_ASSERT_MESSAGE(value_.real_ >= 0  &&  value_.real_ <= maxUInt(),  "Real out of unsigned integer range");
      return UInt(value_.real_);

   case booleanValue:
      return value_.bool_ ? 1 : 0;

   case stringValue:
        {
        UInt val = defaultVal;
        int stat = sscanf(value_.string_, "%" PRIu32, &val);
        BeAssert(stat == 1);
        UNUSED_VARIABLE(stat);;
        return val;
        }
   }
    JSON_FAIL_MESSAGE("Type is not convertible to uint");
    return defaultVal;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Value::Int64 Value::asInt64(Int64 defaultVal) const
{
   switch (type_)
   {
   case nullValue:
      return defaultVal;

   case intValue:
      return value_.int_;

   case uintValue:
      // Bentley change: allow Int64 / UInt64 conversion
      // JSON_ASSERT_MESSAGE(value_.uint_ <= UInt64(maxInt64), "unsigned integer out of Int64 range");
      return value_.uint_;

   case realValue:
      JSON_ASSERT_MESSAGE(value_.real_ >= minInt64()  &&  value_.real_ <= maxInt64(), "Real out of Int64 range");
      return Int(value_.real_);

   case booleanValue:
      return value_.bool_ ? 1 : 0;

   case stringValue:
        {
        Int64 val = defaultVal;
        int stat = sscanf(value_.string_, "%" PRId64, &val);
        BeAssert(stat == 1);
        UNUSED_VARIABLE(stat);;
        return val;
        }
   }

    JSON_FAIL_MESSAGE("Type is not convertible to Int64");
    return defaultVal;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Value::UInt64 Value::asUInt64(UInt64 defaultVal) const
{
   switch (type_)
   {
   case nullValue:
      return defaultVal;

   case intValue:
      // Bentley change: allow Int64 / UInt64 conversion
      // JSON_ASSERT_MESSAGE(value_.int_ >= 0, "Negative integer can not be converted to UInt64");
      return value_.int_;

   case uintValue:
      return value_.uint_;

   case realValue:
      JSON_ASSERT_MESSAGE(value_.real_ >= 0  &&  value_.real_ <= maxUInt64(),  "Real out of UInt64 range");
      return UInt(value_.real_);

   case booleanValue:
      return value_.bool_ ? 1 : 0;

   case stringValue:
        {
        UInt64 val = defaultVal;
        auto fmt = (value_.string_[0] == '0' && (value_.string_[1] == 'X' || value_.string_[1] == 'x')) ? "%" SCNx64 : "%" SCNu64;
        sscanf(value_.string_, fmt, &val);
        return val;
        }
   }

    JSON_FAIL_MESSAGE("Type is not convertible to UInt64");
    return defaultVal;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double Value::asDouble(double defaultVal) const
{
   switch (type_)
   {
   case nullValue:
      return defaultVal;

   case intValue:
      return static_cast<double>(value_.int_);

   case uintValue:
#if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
      return static_cast<double>(value_.uint_);
#else // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
      return static_cast<double>(Int(value_.uint_/2)) * 2 + Int(value_.uint_ & 1);
#endif // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)

   case realValue:
      return value_.real_;

   case booleanValue:
      return value_.bool_ ? 1.0 : 0.0;

   case stringValue:
        {
        double val = defaultVal;
        int stat = sscanf(value_.string_, "%lf", &val);
        BeAssert(stat == 1);
        UNUSED_VARIABLE(stat);;
        return val;
        }
   }
    JSON_FAIL_MESSAGE("Type is not convertible to double");
    return defaultVal;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
float Value::asFloat(float defaultVal) const
{
   switch (type_)
   {
   case nullValue:
      return defaultVal;
   case intValue:
      return static_cast<float>(value_.int_);
   case uintValue:
#if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
      return static_cast<float>(value_.uint_);
#else // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
      return static_cast<float>(Int(value_.uint_/2)) * 2 + Int(value_.uint_ & 1);
#endif // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
   case realValue:
      return static_cast<float>(value_.real_);
   case booleanValue:
      return value_.bool_ ? 1.0f : 0.0f;
   case stringValue:
        {
        float val = defaultVal;
        int stat = sscanf(value_.string_, "%f", &val);
        BeAssert(stat == 1);
        UNUSED_VARIABLE(stat);;
        return val;
        }
   }
    JSON_FAIL_MESSAGE("Type is not convertible to float");
    return defaultVal; // unreachable;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool Value::asBool(bool defaultVal) const
{
   switch (type_)
   {
   case nullValue:
      return defaultVal;
   case intValue:
   case uintValue:
      return value_.int_ != 0;
   case realValue:
      return value_.real_ != 0.0;
   case booleanValue:
      return value_.bool_;
   case stringValue:
      return value_.string_  &&  value_.string_[0] != 0;
   case arrayValue:
   case objectValue:
      return value_.map_->size() != 0;
   default:
      JSON_ASSERT_UNREACHABLE;
   }
   return defaultVal; // unreachable;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool Value::isConvertibleTo(ValueType other) const
{
   switch (type_)
   {
   case nullValue:
      return true;
   case intValue:
      return (other == nullValue  &&  value_.int_ == 0)
             || other == intValue
             || (other == uintValue  && value_.int_ >= 0)
             || other == realValue
             || other == stringValue
             || other == booleanValue;
   case uintValue:
      return (other == nullValue  &&  value_.uint_ == 0)
             || (other == intValue  && value_.uint_ <= (unsigned)maxInt())
             || other == uintValue
             || other == realValue
             || other == stringValue
             || other == booleanValue;
   case realValue:
      return (other == nullValue  &&  value_.real_ == 0.0)
             || (other == intValue  &&  value_.real_ >= minInt()  &&  value_.real_ <= maxInt())
             || (other == uintValue  &&  value_.real_ >= 0  &&  value_.real_ <= maxUInt())
             || other == realValue
             || other == stringValue
             || other == booleanValue;
   case booleanValue:
      return (other == nullValue  &&  value_.bool_ == false)
             || other == intValue
             || other == uintValue
             || other == realValue
             || other == stringValue
             || other == booleanValue;
   case stringValue:
      return other == stringValue || (other == nullValue && (!value_.string_ || value_.string_[0] == 0));

   case arrayValue:
      return other == arrayValue || (other == nullValue && value_.map_->size() == 0);

   case objectValue:
      return other == objectValue || (other == nullValue && value_.map_->size() == 0);

   default:
      JSON_ASSERT_UNREACHABLE;
   }
   return false; // unreachable;
}

/*---------------------------------------------------------------------------------**//**
/// Number of values in array or object
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayIndex Value::size() const
{
   switch (type_)
   {
   case nullValue:
   case intValue:
   case uintValue:
   case realValue:
   case booleanValue:
   case stringValue:
      return 0;
   case arrayValue:  // size of the array is highest index + 1
      if (!value_.map_->empty())
      {
         ObjectValues::const_iterator itLast = value_.map_->end();
         --itLast;
         return (*itLast).first.index()+1;
      }
      return 0;
   case objectValue:
      return ArrayIndex(value_.map_->size());
   default:
      JSON_ASSERT_UNREACHABLE;
   }
   return 0; // unreachable;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Value::clear()
{
   JSON_ASSERT(type_ == nullValue  ||  type_ == arrayValue  || type_ == objectValue);

   switch (type_)
   {
   case arrayValue:
   case objectValue:
      value_.map_->clear();
      break;
   default:
      break;
   }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Value::resize(ArrayIndex newSize)
{
   JSON_ASSERT(type_ == nullValue  ||  type_ == arrayValue);
   if (type_ == nullValue)
      *this = Value(arrayValue);
   ArrayIndex oldSize = size();
   if (newSize == 0)
      clear();
   else if (newSize > oldSize)
      (*this)[ newSize - 1 ];
   else
   {
      for (ArrayIndex index = newSize; index < oldSize; ++index)
      {
         value_.map_->erase(index);
      }
      BeAssert(size() == newSize);
   }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
JsonValueR Value::operator[](ArrayIndex index)
{
   JSON_ASSERT(type_ == nullValue  ||  type_ == arrayValue);
   if (type_ == nullValue)
      *this = Value(arrayValue);
   CZString key(index);
   ObjectValues::iterator it = value_.map_->lower_bound(key);
   if (it != value_.map_->end()  &&  (*it).first == key)
      return (*it).second;

   ObjectValues::value_type defaultValue(key, GetNull());
   it = value_.map_->insert(it, defaultValue);
   return (*it).second;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
JsonValueCR Value::operator[](ArrayIndex index) const
{
   JSON_ASSERT(type_ == nullValue  ||  type_ == arrayValue);
   if (type_ == nullValue)
      return GetNull();
   CZString key(index);
   ObjectValues::const_iterator it = value_.map_->find(key);
   if (it == value_.map_->end())
      return GetNull();
   return (*it).second;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Value Value::removeMember(const char* key)
{
   JSON_ASSERT(type_ == nullValue  ||  type_ == objectValue);
   if (type_ == nullValue)
      return GetNull();
   CZString actualKey(key, CZString::noDuplication);
   ObjectValues::iterator it = value_.map_->find(actualKey);
   if (it == value_.map_->end())
      return GetNull();
   Value old(it->second);
   value_.map_->erase(it);
   return old;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool Value::removeIndex(ArrayIndex index, Value* removed) {
  if (type_ != arrayValue) {
    return false;
  }
  CZString key(index);
  ObjectValues::iterator it = value_.map_->find(key);
  if (it == value_.map_->end()) {
    return false;
  }
  *removed = it->second;
  ArrayIndex oldSize = size();
  // shift left all items left, into the place of the "removed"
  for (ArrayIndex i = index; i < (oldSize - 1); ++i){
    CZString key(i);
    (*value_.map_)[key] = (*this)[i + 1];
  }
  // erase the last one ("leftover")
  CZString keyLast(oldSize - 1);
  ObjectValues::iterator itLast = value_.map_->find(keyLast);
  value_.map_->erase(itLast);
  return true;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Value::Members Value::getMemberNames() const
{
   JSON_ASSERT(type_ == nullValue  ||  type_ == objectValue);
   if (type_ == nullValue)
       return Value::Members();
   Members members;
   members.reserve(value_.map_->size());
   ObjectValues::const_iterator it = value_.map_->begin();
   ObjectValues::const_iterator itEnd = value_.map_->end();
   for (; it != itEnd; ++it)
      members.push_back(Utf8String((*it).first.c_str()));
   return members;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Value::toStyledString() const
    {
   StyledWriter writer;
   return writer.write(*this);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Value::const_iterator Value::begin() const
{
   switch (type_)
   {
   case arrayValue:
   case objectValue:
      if (value_.map_)
         return const_iterator(value_.map_->begin());
      break;
   default:
      break;
   }
   return const_iterator();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Value::const_iterator Value::end() const
{
   switch (type_)
   {
   case arrayValue:
   case objectValue:
      if (value_.map_)
         return const_iterator(value_.map_->end());
      break;
   default:
      break;
   }
   return const_iterator();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Value::iterator Value::begin()
{
   switch (type_)
   {
   case arrayValue:
   case objectValue:
      if (value_.map_)
         return iterator(value_.map_->begin());
      break;
   default:
      break;
   }
   return iterator();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Value::iterator Value::end()
{
   switch (type_)
   {
   case arrayValue:
   case objectValue:
      if (value_.map_)
         return iterator(value_.map_->end());
      break;
   default:
      break;
   }
   return iterator();
}

} // namespace Json
END_BENTLEY_NAMESPACE
