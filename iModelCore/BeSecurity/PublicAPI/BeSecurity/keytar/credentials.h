/*---------------------------------------------------------------------------------------------
 * Adapted from https://github.com/atom/node-keytar
 * See LICENSE.md in the libsrc/keytar folder for license information. 
 *--------------------------------------------------------------------------------------------*/
#pragma once
#include <BeSecurity/BeSecurity.h>
#include <string>
#include <utility>

BEGIN_KEYTAR_NAMESPACE

typedef std::pair<std::string, std::string> Credentials;

END_KEYTAR_NAMESPACE
