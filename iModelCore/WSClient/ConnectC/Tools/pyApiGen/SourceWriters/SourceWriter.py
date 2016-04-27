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
        self._file.write("CALLSTATUS wsresultTo{1}Status(WSError::Id errorId, Utf8StringCR errorMessage, Utf8StringCR errorDescription)\n"
                         .format(self._api.get_upper_api_name(), self._api.get_api_name()))
        self._file.write("    {\n")
        self._file.write("    switch(errorId)\n")
        self._file.write('        {\n')
        self._file.write("        case WSError::Id::Unknown:\n")
        self._file.write('            return CALLSTATUS {ERROR500, errorMessage.c_str(), errorDescription.c_str()};\n')
        self._file.write("        case WSError::Id::RepositoryNotFound:\n")
        self._file.write('            return CALLSTATUS {ERROR400, errorMessage.c_str(), errorDescription.c_str()};\n')
        self._file.write("        case WSError::Id::SchemaNotFound:\n")
        self._file.write('            return CALLSTATUS {ERROR400, errorMessage.c_str(), errorDescription.c_str()};\n')
        self._file.write("        case WSError::Id::ClassNotFound:\n")
        self._file.write('            return CALLSTATUS {ERROR400, errorMessage.c_str(), errorDescription.c_str()};\n')
        self._file.write("        case WSError::Id::PropertyNotFound:\n")
        self._file.write('            return CALLSTATUS {ERROR404, errorMessage.c_str(), errorDescription.c_str()};\n')
        self._file.write("        case WSError::Id::InstanceNotFound:\n")
        self._file.write('            return CALLSTATUS {ERROR404, errorMessage.c_str(), errorDescription.c_str()};\n')
        self._file.write("        case WSError::Id::NotSupported:\n")
        self._file.write('            return CALLSTATUS {ERROR400, errorMessage.c_str(), errorDescription.c_str()};\n')
        self._file.write("        case WSError::Id::ServerError:\n")
        self._file.write('            return CALLSTATUS {ERROR500, errorMessage.c_str(), errorDescription.c_str()};\n')
        self._file.write("        case WSError::Id::BadRequest:\n")
        self._file.write('            return CALLSTATUS {ERROR400, errorMessage.c_str(), errorDescription.c_str()};\n')
        self._file.write("        case WSError::Id::Conflict:\n")
        self._file.write('            return CALLSTATUS {ERROR409, errorMessage.c_str(), errorDescription.c_str()};\n')
        self._file.write("        default:\n")
        self._file.write('            return CALLSTATUS {ERROR500, errorMessage.c_str(), errorDescription.c_str()};\n')
        self._file.write("        }\n")
        self._file.write("    }\n")
