/*---------------------------------------------------------------------------------------------
 * Adapted from https://github.com/atom/node-keytar
 * See LICENSE.md in the libsrc/keytar folder for license information. 
 *--------------------------------------------------------------------------------------------*/
#pragma once
#include <BeSecurity/BeSecurity.h>
#include "credentials.h"
#include <string>
#include <vector>

BEGIN_KEYTAR_NAMESPACE

enum KEYTAR_OP_RESULT
{
  SUCCESS,
  FAIL_ERROR,
  FAIL_NONFATAL
};

BESECURITY_EXPORT KEYTAR_OP_RESULT SetPassword(const std::string &service,
                             const std::string &account,
                             const std::string &password,
                             std::string *error);

BESECURITY_EXPORT KEYTAR_OP_RESULT GetPassword(const std::string &service,
                             const std::string &account,
                             std::string *password,
                             std::string *error);

BESECURITY_EXPORT KEYTAR_OP_RESULT DeletePassword(const std::string &service,
                                const std::string &account,
                                std::string *error);

BESECURITY_EXPORT KEYTAR_OP_RESULT FindPassword(const std::string &service,
                              std::string *password,
                              std::string *error);

BESECURITY_EXPORT KEYTAR_OP_RESULT FindCredentials(const std::string &service,
                                 std::vector<Credentials> *,
                                 std::string *error);

END_KEYTAR_NAMESPACE
