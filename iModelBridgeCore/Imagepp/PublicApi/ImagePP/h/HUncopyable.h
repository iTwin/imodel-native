//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/h/HUncopyable.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class: HUncopyable
// ----------------------------------------------------------------------------
#pragma once


BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// HUncopyable
//
// Utility class used to facilitate deactivation of copy constructor and
// assignment operator. Classes inheriting from HUncopyable become themselves
// non-copyable by thwarting any compiler's attempt to generate default
// version of copy constructor / assignment operator.
//
// Example:
// class HTest : private HUncopyable
// {
//  ...
// };
//
// NOTE: Private inheritance is preferable because we do not want that users
//       manipulate derived objects using via polymorphism.
//-----------------------------------------------------------------------------
class HUncopyable
    {
protected:
    // Allow construction and destruction of derived classes
    HUncopyable () {};
    ~HUncopyable() {};

private:
    // Prevent copying derived classes
    HUncopyable(const HUncopyable&);
    HUncopyable& operator=(const HUncopyable&);
    };

END_IMAGEPP_NAMESPACE