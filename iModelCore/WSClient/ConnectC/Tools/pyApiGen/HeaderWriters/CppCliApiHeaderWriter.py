from Helpers.CppCliStruct import CppCliStruct
from HeaderWriters.HeaderWriter import HeaderWriter


class CppCliApiHeaderWriter(HeaderWriter):
    def __init__(self, ecclasses, header_filename, api, status_codes, excluded_classes):
        super(CppCliApiHeaderWriter, self).__init__(ecclasses, header_filename, api, status_codes, excluded_classes)
        self.__cpp_cli_structs = []
        for ecclass in self._ecclasses:
            if ecclass.attributes["typeName"].value in excluded_classes and \
                    excluded_classes[ecclass.attributes["typeName"].value].should_exclude_entire_class():
                continue
            self.__cpp_cli_structs.append(CppCliStruct(ecclass, api, self._status_codes))

    def write_header(self):
        self._write_header_comments(True)
        self._write_spacing()
        self.__write_includes()
        self._write_spacing()
        self.__write_using_declarations()
        self._write_spacing()
        self._file.write('namespace {0}Sharp\n'.format(self._api.get_api_name()))
        self._file.write('    {\n')
        self.__write_api_class_declaration()
        self._close_file()

    def __write_includes(self):
        self._file.write('#include <WebServices/ConnectC/{0}Public.h>\n'.format(self._api.get_api_acronym()))

    def __write_using_declarations(self):
        self._file.write('using namespace System;\n')
        self._file.write('using namespace System::Collections::Generic;\n')

    def __write_api_class_declaration(self):
        self._file.write('    public ref class {0}\n'.format(self._api.get_api_name()))
        self._file.write('        {\n')
        self._file.write('        private:\n')
        self._file.write('            {0}HANDLE m_api;\n\n'.format(self._api.get_upper_api_acronym()))
        self._file.write('        public:\n')
        self._file.write('            {0}(String^ username, String^ password, int id);\n'.format(self._api.get_api_name()))
        self._file.write('            {0}(String^ tokenStr, int id);\n'.format(self._api.get_api_name()))
        self._file.write('            ~{0}();\n'.format(self._api.get_api_name()))
        self._write_spacing()
        for cpp_cli_struct in self.__cpp_cli_structs:
            self.__write_api_function_definitions(cpp_cli_struct)
        self._file.write('        };\n')
        self._file.write('    }\n')

    def __write_api_function_definitions(self, cpp_cli_struct):
        self._file.write(self._COMMENT_GroupStart
                         .format("{0} CRUD functions".format(cpp_cli_struct.get_name()),
                                 "The CRUD functions for the {0} class".format(cpp_cli_struct.get_name())))
        self.__write_api_read_list_definition(cpp_cli_struct)
        self.__write_api_create_definition(cpp_cli_struct)
        self.__write_api_read_definition(cpp_cli_struct)
        self.__write_api_update_definition(cpp_cli_struct)
        self.__write_api_delete_definition(cpp_cli_struct)
        self._file.write(self._COMMENT_GroupEnd)
        self._write_spacing()

    def __write_api_read_list_definition(self, cpp_cli_struct):
        self._file.write(self._COMMENT_GroupBriefLong.format("{0} read list function".format(cpp_cli_struct.get_name()),
                                                             "\param[out] {1}s List of {0}s".format(cpp_cli_struct.get_name(),
                                                                                                    cpp_cli_struct.get_lower_name())))
        self._file.write("            " + cpp_cli_struct.get_api_gws_read_list_definition())

    def __write_api_create_definition(self, cpp_cli_struct):
        self._file.write(self._COMMENT_GroupBriefLong.format("{0} create function".format(cpp_cli_struct.get_name()),
                                                             "\param[in] {1} The {0} to create".format(cpp_cli_struct.get_name(),
                                                                                                       cpp_cli_struct.get_lower_name())))
        self._file.write("            " + cpp_cli_struct.get_api_gws_create_definition())

    def __write_api_read_definition(self, cpp_cli_struct):
        self._file.write(self._COMMENT_GroupBriefLong.format("{0} read function".format(cpp_cli_struct.get_name()),
                                                             "\param[in] {1}Id The {0} id\n* \param[out] {1} The retreived {0}"
                                                             .format(cpp_cli_struct.get_name(),
                                                                     cpp_cli_struct.get_lower_name())))
        self._file.write("            " + cpp_cli_struct.get_api_gws_read_definition())

    def __write_api_update_definition(self, cpp_cli_struct):
        self._file.write(self._COMMENT_GroupBriefLong.format("{0} update function".format(cpp_cli_struct.get_name()),
                                                             "\param[in] The {0} id\n* \param[in] {1} The {0} to update"
                                                             .format(cpp_cli_struct.get_name(),
                                                                     cpp_cli_struct.get_lower_name())))
        self._file.write("            " + cpp_cli_struct.get_api_gws_update_definition())

    def __write_api_delete_definition(self, cpp_cli_struct):
        self._file.write(self._COMMENT_GroupBriefLong.format("{0} delete function".format(cpp_cli_struct.get_name()),
                                                             "\param[in] {1}Id The {0} id to delete"
                                                             .format(cpp_cli_struct.get_name(),
                                                                     cpp_cli_struct.get_lower_name())))
        self._file.write("            " + cpp_cli_struct.get_api_gws_delete_definition())






