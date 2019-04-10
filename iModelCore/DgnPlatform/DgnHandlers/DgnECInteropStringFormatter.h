/*--------------------------------------------------------------------------------------+ 
|
|     $Source: DgnHandlers/DgnECInteropStringFormatter.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* In some cases ECObjects persists .NET-style format strings, e.g. in calculated
* EC properties, or as formatting instructions for ECFields pointing to DateTime properties.
* At some point we will probably need to address how we will apply this formatting in a
* purely native context (e.g. for mobile apps). For now, the managed layer can supply
* an IECInteropStringFormatter to perform the formatting; a barebones IECInteropStringFormatter
* is provided if no managed formatter has been supplied.
* InteropStringFormatter also provides the reverse operation of parsing a string into a
* primitive ECValue.
* @bsistruct                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct IECInteropStringFormatter
    {
    struct IECValueList
        {
        virtual uint32_t        GetCount() const = 0;
        virtual ECN::ECValueCP   operator[] (uint32_t index) const = 0;
        };

    // Apply the format string to the list of values.
    virtual bool                Format (Utf8StringR formatted, Utf8CP formatString, IECValueList const& valueList) const = 0;
    // Format a value using the format string. Native implementation supports DateTime and Double values, with limited formatting
    virtual bool                FormatValue (Utf8StringR formatted, Utf8CP formatString, ECN::ECValueCR v) const = 0;
    virtual bool                Parse (ECN::ECValueR parsedValue, Utf8CP valueAsString, ECN::PrimitiveType parseAsType) const = 0;
    virtual void                Release() const = 0;
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct InteropStringFormatter : IECInteropStringFormatter
{
public:
    bool                Format (Utf8StringR formatted, Utf8CP formatString, IECValueList const& values) const override;
    bool                FormatValue (Utf8StringR formatted, Utf8CP formatString, ECValueCR v) const override;
    bool                Parse (ECN::ECValueR parsedValue, Utf8CP valueAsString, ECN::PrimitiveType parseAsType) const override;
    void                Release() const override { }

    static InteropStringFormatter const&    GetInstance()
        {
        // Note we can have a static instance because this is a stateless object...
        static InteropStringFormatter s_instance;
        return s_instance;
        }
};


END_BENTLEY_DGNPLATFORM_NAMESPACE
