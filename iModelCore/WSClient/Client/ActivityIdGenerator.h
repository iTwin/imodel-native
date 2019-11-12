/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/WString.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

typedef std::shared_ptr<struct IActivityIdGenerator> IActivityIdGeneratorPtr;
typedef const IActivityIdGenerator& IActivityIdGeneratorCR;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                 Mantas.Smicius        10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
//! IActivityIdGenerator generates activity id for http request headers
struct IActivityIdGenerator
    {
    public:
        virtual ~IActivityIdGenerator() {};
        virtual Utf8String GenerateNextId() const = 0;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                 Mantas.Smicius        10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct ActivityIdGenerator : public IActivityIdGenerator
    {
    public:
        WSCLIENT_EXPORT Utf8String GenerateNextId() const override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE