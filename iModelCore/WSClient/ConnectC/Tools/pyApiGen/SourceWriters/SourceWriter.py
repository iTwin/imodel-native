from Writer import Writer
import datetime


class SourceWriter(Writer):
    _COMMENT_BsiMethod = "/*---------------------------------------------------------------------------------**//**\n" \
                         "* @bsimethod                                                                    {0}\n"       \
                         "+---------------+---------------+---------------+---------------+---------------+------*/\n" \
        .format(datetime.date.today().strftime('%m/%Y'))

    def __init__(self, ecclasses, filename, api, status_codes, excluded_classes):
        super(SourceWriter, self).__init__(ecclasses, filename, api, status_codes, excluded_classes)

    def _write_string_to_wstring_function(self):
        self._file.write(self._COMMENT_BsiMethod)
        self._file.write('WString stringToWString(const Utf8String &str)\n')
        self._file.write('    {\n')
        self._file.write('    Utf16Buffer _16buf;\n')
        self._file.write('    BeStringUtilities::Utf8ToUtf16(_16buf, str.c_str());\n')
        self._file.write('    WString outStr;\n')
        self._file.write('    BeStringUtilities::Utf16ToWChar(outStr, _16buf.data());\n')
        self._file.write('    return outStr;\n')
        self._file.write('    }\n')

    def _write_resolve_wserror_function(self):
        self._file.write(self._COMMENT_BsiMethod)
        self._file.write("CallStatus wsresultTo{1}Status(LPCWSCC api, WSError::Id errorId, Utf8StringCR errorMessage, "
                         "Utf8StringCR errorDescription)\n"
                         .format(self._api.get_upper_api_name(), self._api.get_api_name()))
        self._file.write("    {\n")
        self._file.write('    api->SetStatusMessage(errorMessage.c_str());\n')
        self._file.write('    api->SetStatusDescription(errorDescription.c_str());\n')
        self._file.write("    switch(errorId)\n")
        self._file.write('        {\n')
        self._file.write("        case WSError::Id::Unknown:\n")
        self._file.write('            return ERROR500;\n')
        self._file.write("        case WSError::Id::LoginFailed:\n")
        self._file.write('            return LOGIN_FAILED;\n')
        self._file.write("        case WSError::Id::SslRequired:\n")
        self._file.write('            return SSL_REQUIRED;\n')
        self._file.write("        case WSError::Id::NotEnoughRights:\n")
        self._file.write('            return NOT_ENOUGH_RIGHTS;\n')
        self._file.write("        case WSError::Id::RepositoryNotFound:\n")
        self._file.write('            return REPOSITORY_NOT_FOUND;\n')
        self._file.write("        case WSError::Id::SchemaNotFound:\n")
        self._file.write('            return SCHEMA_NOT_FOUND;\n')
        self._file.write("        case WSError::Id::ClassNotFound:\n")
        self._file.write('            return CLASS_NOT_FOUND;\n')
        self._file.write("        case WSError::Id::PropertyNotFound:\n")
        self._file.write('            return PROPERTY_NOT_FOUND;\n')
        self._file.write("        case WSError::Id::InstanceNotFound:\n")
        self._file.write('            return INSTANCE_NOT_FOUND;\n')
        self._file.write("        case WSError::Id::FileNotFound:\n")
        self._file.write('            return FILE_NOT_FOUND;\n')
        self._file.write("        case WSError::Id::NotSupported:\n")
        self._file.write('            return NOT_SUPPORTED;\n')
        self._file.write("        case WSError::Id::NoServerLicense:\n")
        self._file.write('            return NO_SERVER_LICENSE;\n')
        self._file.write("        case WSError::Id::NoClientLicense:\n")
        self._file.write('            return NO_CLIENT_LICENSE;\n')
        self._file.write("        case WSError::Id::TooManyBadLoginAttempts:\n")
        self._file.write('            return TO_MANY_BAD_LOGIN_ATTEMPTS;\n')
        self._file.write("        case WSError::Id::ServerError:\n")
        self._file.write('            return ERROR500;\n')
        self._file.write("        case WSError::Id::BadRequest:\n")
        self._file.write('            return ERROR400;\n')
        self._file.write("        case WSError::Id::Conflict:\n")
        self._file.write('            return ERROR409;\n')
        self._file.write("        default:\n")
        self._file.write('            return ERROR500;\n')
        self._file.write("        }\n")
        self._file.write("    }\n")
