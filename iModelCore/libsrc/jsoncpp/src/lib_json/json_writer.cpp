// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#if !defined(JSON_IS_AMALGAMATION)
# include <json/writer.h>
# include "json_tool.h"
#endif // if !defined(JSON_IS_AMALGAMATION)
#include <utility>
#include <stdio.h>
#include <string.h>
#include <iomanip>
#include <Bentley/BeAssert.h>

#if _MSC_VER >= 1400 // VC++ 8.0
#pragma warning( disable : 4996 )   // disable warning about strdup being deprecated.
#endif

BEGIN_BENTLEY_NAMESPACE
namespace Json {

static bool containsControlCharacter( const char* str )
{
   while ( *str ) 
   {
      if ( isControlCharacter( *(str++) ) )
         return true;
   }
   return false;
}


Utf8String valueToString( LargestInt value )
{
   UIntToStringBuffer buffer;
   char *current = buffer + sizeof(buffer);
   bool isNegative = value < 0;
   if ( isNegative )
      value = -value;
   uintToString( LargestUInt(value), current );
   if ( isNegative )
      *--current = '-';
   BeAssert( current >= buffer );
   return current;
}


Utf8String valueToString( LargestUInt value )
{
   UIntToStringBuffer buffer;
   char *current = buffer + sizeof(buffer);
   uintToString( value, current );
   BeAssert( current >= buffer );
   return current;
}

#if defined(JSON_HAS_INT64)

Utf8String valueToString( Int value )
{
   return valueToString( LargestInt(value) );
}


Utf8String valueToString( UInt value )
{
   return valueToString( LargestUInt(value) );
}

#endif // # if defined(JSON_HAS_INT64)


Utf8String valueToString( double value )
{
if (std::isnan(value) || std::isinf(value))
   return "null";

#if !defined (ANDROID)
   // Make sure that no one is attempting to use the json library with their locale set to anything other than the standard "C" locale. 
   // The sprintf below relies on the decimal point format being ".", period (no pun intended). Don't remove this BeAssert - change your program.
   // It is a mistake to change the locale anyway; don't do that (at least, don't change the decimal point specification in the locale) - KAB
   BeAssert (*(localeconv()->decimal_point) == '.');
#endif

   char buffer[40];
   // BENTLEY CHANGE: increased precision from %#.16g to %#.17g. This makes it so we never lose anything round-tripping IEEE floating point values.
#if defined(_MSC_VER) && defined(__STDC_SECURE_LIB__) // Use secure version with visual studio 2005 to avoid warning. 
   sprintf_s(buffer, sizeof(buffer), "%#.17g", value); 
#else	
   sprintf(buffer, "%#.17g", value); 
#endif

   char* ch = buffer + strlen(buffer) - 1;

   // BENTLEY_CHANGE: The `#` specifier above ensures decimal point always printed even if no digits follow. This can result in a string
   // in which the last character is '.'. That is not valid. We could remove the trailing decimal point, but that changes the value's type from
   // double to integer. Much of our code assumes if we put a double in, after round-tripping through a string back to JSON we'll get a double back out.
   // So append the trailing zero instead.
   if (*ch == '.'){
     *(ch + 1) = '0';
     *(ch + 2) = 0;
     return buffer;
   }

   if (*ch != '0') return buffer; // nothing to truncate, so save time
   while(ch > buffer && *ch == '0'){
     --ch;
   }
   char* last_nonzero = ch;
   while(ch >= buffer){
     switch(*ch){
     case '0':
     case '1':
     case '2':
     case '3':
     case '4':
     case '5':
     case '6':
     case '7':
     case '8':
     case '9':
       --ch;
       continue;
     case '.':
       // Truncate zeroes to save bytes in output, but keep one.
       *(last_nonzero+2) = '\0';
       return buffer;
     default:
       return buffer;
     }
   }
   return buffer;
}


Utf8String valueToString( bool value )
{
   return value ? "true" : "false";
}

Utf8String valueToQuotedString( const char *value )
{
   // Not sure how to handle unicode...
   if (strpbrk(value, "\"\\\b\f\n\r\t") == NULL && !containsControlCharacter( value ))
      return Utf8String("\"") + value + "\"";
   // We have to walk value and escape any special characters.
   // Appending to std::string is not efficient, but this should be rare.
   // (Note: forward slashes are *not* rare, but I am not escaping them.)
   Utf8String::size_type maxsize = strlen(value)*2 + 3; // allescaped+quotes+NULL
   Utf8String result;
   result.reserve(maxsize); // to avoid lots of mallocs
   result += "\"";
   for (const char* c=value; *c != 0; ++c)
   {
      switch(*c)
      {
         case '\"':
            result += "\\\"";
            break;
         case '\\':
            result += "\\\\";
            break;
         case '\b':
            result += "\\b";
            break;
         case '\f':
            result += "\\f";
            break;
         case '\n':
            result += "\\n";
            break;
         case '\r':
            result += "\\r";
            break;
         case '\t':
            result += "\\t";
            break;
         //case '/':
            // Even though \/ is considered a legal escape in JSON, a bare
            // slash is also legal, so I see no reason to escape it.
            // (I hope I am not misunderstanding something.
            // blep notes: actually escaping \/ may be useful in javascript to avoid </ 
            // sequence.
            // Should add a flag to allow this compatibility mode and prevent this 
            // sequence from occurring.
         default:
            if ( isControlCharacter( *c ) )
            {
                // \uXXXX is how to encode an arbitrary Unicode code point in JSON.
                // We know we're processing one char at a time, so the maximum is 0xff.
                // Other known control characters are processed above; if we got here, it's probably garbage, but properly encode it to avoid worse issues down the line.
                static char const HEX_DIGITS[] = "0123456789ABCDEF";
                char buf[7] = "\\u00";
                buf[4] = HEX_DIGITS[static_cast<Byte>(*c) >> 4];
                buf[5] = HEX_DIGITS[static_cast<Byte>(*c) & 0xF];
                buf[6] = 0;
                result += buf;
            }
            else
            {
               result += *c;
            }
            break;
      }
   }
   result += "\"";
   return result;
}

Utf8String FastWriter::write(const Value &root)
{
   document_.clear();
   writeValue( root );
   document_ += "\n";
   return document_;
}

void FastWriter::writeValue(const Value &value)
{
   switch ( value.type() )
   {
   case nullValue:
      document_ += "null";
      break;
   case intValue:
      document_ += valueToString( value.asLargestInt() );
      break;
   case uintValue:
      document_ += valueToString( value.asLargestUInt() );
      break;
   case realValue:
      document_ += valueToString( value.asDouble() );
      break;
   case stringValue:
      document_ += valueToQuotedString( value.asCString() );
      break;
   case booleanValue:
      document_ += valueToString( value.asBool() );
      break;
   case arrayValue:
      {
         document_ += "[";
         int size = value.size();
         for ( int index =0; index < size; ++index )
         {
            if ( index > 0 )
               document_ += ",";
            writeValue( value[index] );
         }
         document_ += "]";
      }
      break;
   case objectValue:
      {
         Value::Members members( value.getMemberNames() );
         document_ += "{";
         for ( Value::Members::iterator it = members.begin(); 
               it != members.end(); 
               ++it )
         {
            const Utf8String &name = *it;
            if ( it != members.begin() )
               document_ += ",";
            document_ += valueToQuotedString( name.c_str() );
            document_ += ":";
            writeValue( value[name] );
         }
         document_ += "}";
      }
      break;
   }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String FastWriter::ToString(JsonValueCR root)
    {
    FastWriter writer;
    if (nullValue != root.type())
        writer.writeValue(root);

    return writer.document_;
    }

// Class StyledWriter
// //////////////////////////////////////////////////////////////////

StyledWriter::StyledWriter()
   : rightMargin_( 74 )
   , indentSize_( 3 )
{
}


Utf8String 
StyledWriter::write( const Value &root )
{
   document_ = "";
   addChildValues_ = false;
   indentString_ = "";
   writeValue( root );
   document_ += "\n";
   return document_;
}


void 
StyledWriter::writeValue( const Value &value )
{
   switch ( value.type() )
   {
   case nullValue:
      pushValue( "null" );
      break;
   case intValue:
      pushValue( valueToString( value.asLargestInt() ) );
      break;
   case uintValue:
      pushValue( valueToString( value.asLargestUInt() ) );
      break;
   case realValue:
      pushValue( valueToString( value.asDouble() ) );
      break;
   case stringValue:
      pushValue( valueToQuotedString( value.asCString() ) );
      break;
   case booleanValue:
      pushValue( valueToString( value.asBool() ) );
      break;
   case arrayValue:
      writeArrayValue( value);
      break;
   case objectValue:
      {
         Value::Members members( value.getMemberNames() );
         if ( members.empty() )
            pushValue( "{}" );
         else
         {
            writeWithIndent( "{" );
            indent();
            Value::Members::iterator it = members.begin();
            for (;;)
            {
               const Utf8String &name = *it;
               const Value &childValue = value[name];
               writeWithIndent( valueToQuotedString( name.c_str() ) );
               document_ += " : ";
               writeValue( childValue );
               if ( ++it == members.end() )
               {
                  break;
               }
               document_ += ",";
            }
            unindent();
            writeWithIndent( "}" );
         }
      }
      break;
   }
}


void 
StyledWriter::writeArrayValue( const Value &value )
{
   unsigned size = value.size();
   if ( size == 0 )
      pushValue( "[]" );
   else
   {
      bool isArrayMultiLine = isMultineArray( value );
      if ( isArrayMultiLine )
      {
         writeWithIndent( "[" );
         indent();
         bool hasChildValue = !childValues_.empty();
         unsigned index =0;
         for (;;)
         {
            const Value &childValue = value[index];
            if ( hasChildValue )
               writeWithIndent( childValues_[index] );
            else
            {
               writeIndent();
               writeValue( childValue );
            }
            if ( ++index == size )
            {
               break;
            }
            document_ += ",";
         }
         unindent();
         writeWithIndent( "]" );
      }
      else // output on a single line
      {
         BeAssert( childValues_.size() == size );
         document_ += "[ ";
         for ( unsigned index =0; index < size; ++index )
         {
            if ( index > 0 )
               document_ += ", ";
            document_ += childValues_[index];
         }
         document_ += " ]";
      }
   }
}


bool 
StyledWriter::isMultineArray( const Value &value )
{
   int size = value.size();
   bool isMultiLine = size*3 >= rightMargin_ ;
   childValues_.clear();
   for ( int index =0; index < size  &&  !isMultiLine; ++index )
   {
      const Value &childValue = value[index];
      isMultiLine = isMultiLine  ||
                     ( (childValue.isArray()  ||  childValue.isObject())  &&  
                        childValue.size() > 0 );
   }
   if ( !isMultiLine ) // check if line length > max line length
   {
      childValues_.reserve( size );
      addChildValues_ = true;
      int lineLength = 4 + (size-1)*2; // '[ ' + ', '*n + ' ]'
      for ( int index =0; index < size  &&  !isMultiLine; ++index )
      {
         writeValue( value[index] );
         lineLength += int( childValues_[index].length() );
      }
      addChildValues_ = false;
      isMultiLine = isMultiLine  ||  lineLength >= rightMargin_;
   }
   return isMultiLine;
}


void 
StyledWriter::pushValue( const Utf8String &value )
{
   if ( addChildValues_ )
      childValues_.push_back( value );
   else
      document_ += value;
}


void 
StyledWriter::writeIndent()
{
   if ( !document_.empty() )
   {
      char last = document_[document_.length()-1];
      if ( last == ' ' )     // already indented
         return;
      if ( last != '\n' )    // Comments may add new-line
         document_ += '\n';
   }
   document_ += indentString_;
}


void 
StyledWriter::writeWithIndent( const Utf8String &value )
{
   writeIndent();
   document_ += value;
}


void 
StyledWriter::indent()
{
   indentString_ += Utf8String( indentSize_, ' ' );
}


void 
StyledWriter::unindent()
{
   BeAssert( int(indentString_.size()) >= indentSize_ );
   indentString_.resize( indentString_.size() - indentSize_ );
}

Utf8String 
StyledWriter::normalizeEOL( const Utf8String &text )
{
   Utf8String normalized;
   normalized.reserve( text.length() );
   const char *begin = text.c_str();
   const char *end = begin + text.length();
   const char *current = begin;
   while ( current != end )
   {
      char c = *current++;
      if ( c == '\r' ) // mac or dos EOL
      {
         if ( *current == '\n' ) // convert dos EOL
            ++current;
         normalized += '\n';
      }
      else // handle unix EOL & other char
         normalized += c;
   }
   return normalized;
}


} // namespace Json
END_BENTLEY_NAMESPACE
