from py.Writer import Writer
import datetime


class SourceWriter(Writer):
    _COMMENT_BsiMethod = "/*---------------------------------------------------------------------------------**//**\n" \
                         "* @bsimethod                                                                    {0}\n"       \
                         "+---------------+---------------+---------------+---------------+---------------+------*/\n" \
        .format(datetime.date.today().strftime('%m/%Y'))

    def __init__(self, ecclasses, filename, api, status_codes, excluded_classes):
        super(SourceWriter, self).__init__(ecclasses, filename, api, status_codes, excluded_classes)

    def _write_guid_from_string_function(self):
        self._file.write(self._COMMENT_BsiMethod)
        self._file.write('GUID guidFromString(const std::wstring& guidStr)\n')
        self._file.write('    {\n')
        self._file.write('    //FROM "Essentials of COM" on PluralSight\n')
        self._file.write('    GUID parsed;\n')
        self._file.write('    HRESULT result = CLSIDFromString(guidStr.c_str(), &parsed);\n')
        self._file.write('    if (result != NOERROR)\n')
        self._file.write('        return GUID();\n')
        self._file.write('    return parsed;\n')
        self._file.write('    }\n')

    def _write_guid_to_wstring_function(self):
        self._file.write(self._COMMENT_BsiMethod)
        self._file.write('static std::wstring guidToString(LPCGUID guid)\n')
        self._file.write('    {\n')
        self._file.write('    WCHAR stringBuf[4096];\n')
        self._file.write('    if (0 == StringFromGUID2(*guid, stringBuf, _countof(stringBuf)))\n')
        self._file.write('        return std::wstring();\n')
        self._file.write('    return std::wstring(stringBuf, _countof(stringBuf) - 1);\n')
        self._file.write('    }\n')

    def _write_string_to_wstring_function(self):
        self._file.write(self._COMMENT_BsiMethod)
        self._file.write('std::wstring stringToWString(const std::string &str)\n')
        self._file.write('    {\n')
        self._file.write('    if (str.empty()) return std::wstring();\n')
        self._file.write('    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int) str.size(), NULL, 0);\n')
        self._file.write('    std::wstring wstrTo(size_needed, 0);\n')
        self._file.write('    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int) str.size(), &wstrTo[0], size_needed);\n')
        self._file.write('    return wstrTo;\n')
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
