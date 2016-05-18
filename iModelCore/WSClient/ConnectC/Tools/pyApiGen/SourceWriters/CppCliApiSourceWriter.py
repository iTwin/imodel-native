from Helpers.CppCliStruct import CppCliStruct
from SourceWriters.SourceWriter import SourceWriter


class CppCliApiSourceWriter(SourceWriter):
    def __init__(self, ecclasses, source_filename, api, status_codes, excluded_classes):
        super(CppCliApiSourceWriter, self).__init__(ecclasses, source_filename, api, status_codes, excluded_classes)
        self.__cpp_cli_structs = []
        for ecclass in self._ecclasses:
            if ecclass.attributes["typeName"].value in excluded_classes and \
                    excluded_classes[ecclass.attributes["typeName"].value].should_exclude_entire_class():
                continue
            self.__cpp_cli_structs.append(CppCliStruct(ecclass, api, status_codes))

    def write_source(self):
        self._write_header_comments()
        self._write_spacing()
        self.__write_includes()
        self._write_spacing()
        self._file.write('namespace {0}Sharp\n'.format(self._api.get_api_name()))
        self._file.write('    {\n')
        self.__write_api_ctor_dtor()
        self._write_spacing()
        self.__write_api_gws_functions()
        self._file.write('    }\n')
        self._close_file()

    def __write_includes(self):
        self._file.write('#include "stdafx.h"\n')

    def __write_api_ctor_dtor(self):
        self._file.write(self._COMMENT_BsiMethod)
        self._file.write('    {0}::{0}(String^ username, String^ password, int id)\n'.format(self._api.get_api_name()))
        self._file.write('        {\n')
        self._file.write('        pin_ptr<const wchar_t> usernamePtr = PtrToStringChars(username);\n')
        self._file.write('        pin_ptr<const wchar_t> passwordPtr = PtrToStringChars(password);\n')
        self._file.write('        m_api = {0}_InitializeApiWithCredentials(usernamePtr, passwordPtr, id);\n'.format(self._api.get_api_name()))
        self._file.write('        }\n\n')

        self._file.write(self._COMMENT_BsiMethod)
        self._file.write('    {0}::{0}(String^ tokenStr, int id)\n'.format(self._api.get_api_name()))
        self._file.write('        {\n')
        self._file.write('        pin_ptr<const wchar_t> tokenStrPtr = PtrToStringChars(tokenStr);\n')
        self._file.write('        m_api = {0}_InitializeApiWithToken(tokenStrPtr, id);\n'.format(self._api.get_api_name()))
        self._file.write('        }\n\n')

        self._file.write(self._COMMENT_BsiMethod)
        self._file.write('    {0}::~{0}()\n'.format(self._api.get_api_name()))
        self._file.write('        {\n')
        self._file.write('        {0}_FreeApi(m_api);\n'.format(self._api.get_api_name()))
        self._file.write('        }')

    def __write_api_gws_functions(self):
        for cpp_cli_struct in self.__cpp_cli_structs:
            self.__write_api_gws_read_list_implementation(cpp_cli_struct)
            self.__write_api_gws_create_implementation(cpp_cli_struct)
            self.__write_api_gws_read_implementation(cpp_cli_struct)
            self.__write_api_gws_update_implementation(cpp_cli_struct)
            self.__write_api_gws_delete_implementation(cpp_cli_struct)

    def __write_api_gws_read_list_implementation(self, cpp_cli_struct):
        self._write_spacing()
        self._file.write(self._COMMENT_BsiMethod)
        self._file.write(cpp_cli_struct.get_api_gws_read_list_implementation())

    def __write_api_gws_create_implementation(self, cpp_cli_struct):
        self._write_spacing()
        self._file.write(self._COMMENT_BsiMethod)
        self._file.write(cpp_cli_struct.get_api_gws_create_implementation())

    def __write_api_gws_read_implementation(self, cpp_cli_struct):
        self._write_spacing()
        self._file.write(self._COMMENT_BsiMethod)
        self._file.write(cpp_cli_struct.get_api_gws_read_implementation())

    def __write_api_gws_update_implementation(self, cpp_cli_struct):
        self._write_spacing()
        self._file.write(self._COMMENT_BsiMethod)
        self._file.write(cpp_cli_struct.get_api_gws_update_implementation())

    def __write_api_gws_delete_implementation(self, cpp_cli_struct):
        self._write_spacing()
        self._file.write(self._COMMENT_BsiMethod)
        self._file.write(cpp_cli_struct.get_api_gws_delete_implementation())
