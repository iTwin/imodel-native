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
