from Helpers.CApiStruct import CApiStruct
from HeaderWriters.HeaderWriter import HeaderWriter


class CallStatus:
    def __init__(self, code, message):
        self.code = code
        self.message = message


class CPublicApiHeaderWriter(HeaderWriter):
    def __init__(self, schema_name, ecclasses, header_filename, api, status_codes, excluded_classes=None):
        super(CPublicApiHeaderWriter, self).__init__(ecclasses, header_filename, api, status_codes, excluded_classes)
        self.__schema_name = schema_name
        self.__api_structs = []
        for ecclass in self._ecclasses:
            if ecclass.attributes["typeName"].value not in self._excluded_classes:
                self.__api_structs.append(CApiStruct(self.__schema_name, ecclass, api, self._status_codes))

    def write_header(self):
        self.__write_header_comment()
        self._write_spacing()
        self.__write_defines()
        self._write_spacing()
        self.__write_status_codes()
        self._write_spacing()
        self.__write_api_handle()
        self._write_spacing()
        self.__write_api_function_definitions()
        self._close_file()

    def __write_header_comment(self):
        self._write_header_comments(True, True)

    def __write_defines(self):
        self._file.write('// define __{0}_DLL_BUILD__ when consuming header.\n'.format(self._api.get_upper_api_acronym()))
        self._file.write('#ifdef __{0}_DLL_BUILD__\n'.format(self._api.get_upper_api_acronym()))
        self._file.write('#define {0}_EXPORT EXPORT_ATTRIBUTE\n'.format(self._api.get_upper_api_acronym()))
        self._file.write('#else\n')
        self._file.write('#define {0}_EXPORT IMPORT_ATTRIBUTE\n'.format(self._api.get_upper_api_acronym()))
        self._file.write('#endif\n')

    def __write_status_codes(self):
        if len(self._status_codes) == 0:
            return
        self._file.write(self._COMMENT_GroupStart.format("{0}StatusCodes".format(self._api.get_api_name()),
                                                         "{0} Status Codes".format(self._api.get_api_name())))
        self._write_spacing()
        sorted_codes = sorted(self._status_codes.iteritems(), key=lambda x: x[1].code, reverse=True)
        for key, value in sorted_codes:
            self._file.write(self._COMMENT_GroupDef.format(key, value.message))
            self._write_spacing()

        status_code_str = ""
        for key, value in sorted_codes:
            status_code_str += '#define {0:40} {1}\n'.format(key, value.code)
        self._file.write(status_code_str)
        self._write_spacing()
        self._file.write(self._COMMENT_GroupBriefShort
                         .format("Call status code. See \\ref {0}StatusCodes\n"
                                 .format(self._api.get_api_name())))
        self._file.write("typedef int16_t CallStatus;\n")
        self._file.write(self._COMMENT_GroupEnd)

    def __write_api_handle(self):
        self._file.write(self._COMMENT_GroupStart.format("PointerTypes", "{0} Pointer types".format(self._api.get_api_name())))
        self._file.write(self._COMMENT_GroupBriefShort.format("API handle"))
        self._file.write("typedef void* {0}HANDLE;\n\n".format(self._api.get_upper_api_acronym()))
        self._file.write("typedef void* {0}DATABUFHANDLE;\n".format(self._api.get_upper_api_acronym()))
        self._file.write(self._COMMENT_GroupEnd)

    def __write_api_function_definitions(self):
        self._file.write(self._COMMENT_GroupStart.format("{0}APIFunctions".format(self._api.get_api_name()),
                                                         "{0} API Function Declarations".format(self._api.get_api_name())))
        self.__write_api_handle_free_declaration()
        self._write_spacing()
        for api_struct in self.__api_structs:
            self.__write_api_read_list_definition(api_struct)
            self.__write_api_create_definition(api_struct)
            self.__write_api_read_definition(api_struct)
            self.__write_api_update_definition(api_struct)
            self.__write_api_delete_definition(api_struct)
        self._file.write(self._COMMENT_GroupEnd)

    def __write_api_handle_free_declaration(self):
        self._file.write(self._COMMENT_GroupBriefShort.format("API handle free function"))
        self._file.write('{0}_EXPORT CallStatus {1}_FreeApi({0}HANDLE apiHandle);\n'.format(self._api.get_upper_api_acronym(),
                                                                                            self._api.get_api_name()))

    def __write_api_read_list_definition(self, api_struct):
        self._file.write(self._COMMENT_GroupBriefLong.format("Query WSG to get list of {0}s".format(api_struct.get_lower_name()),
                                                             "\param[in] apiHandle API object\n"
                                                             "* \param[out] {0}Buffer Buffer of {1} data\n"
                                                             "* \\return Success or error code. See \\ref {2}StatusCodes"
                                                             .format(api_struct.get_lower_name(), api_struct.get_name(),
                                                                     self._api.get_api_name())))
        self._file.write(api_struct.get_api_gws_read_list_definition())
        self._write_spacing()

    def __write_api_create_definition(self, api_struct):
        param_str = "\param[in] apiHandle API object\n"
        for ecproperty in api_struct.get_properties():
            if ecproperty.hasAttribute("readOnly") and ecproperty.attributes["readOnly"].value:
                continue
            param_str += "* \param[in] {0}\n".format(ecproperty.attributes["propertyName"].value)
        param_str += "* \\return Success or error code. See \\ref {0}StatusCodes".format(self._api.get_api_name())
        self._file.write(self._COMMENT_GroupBriefLong.format("Create a new {0}".format(api_struct.get_lower_name()), param_str))
        self._file.write(api_struct.get_api_gws_create_definition())
        self._write_spacing()

    def __write_api_read_definition(self, api_struct):
        self._file.write(self._COMMENT_GroupBriefLong.format("Get {0} information buffer".format(api_struct.get_lower_name()),
                                                             "\param[in] apiHandle API object\n"
                                                             "* \param[in] {0}Id {1} ID to select\n"
                                                             "* \param[out] {0}Buffer {1} data buffer\n"
                                                             "* \\return Success or error code. See \\ref {2}StatusCodes"
                                                             .format(api_struct.get_lower_name(), api_struct.get_name(),
                                                                     self._api.get_api_name())))
        self._file.write(api_struct.get_api_gws_read_definition())
        self._write_spacing()

    def __write_api_update_definition(self, api_struct):
        param_str = "\param[in] apiHandle API object\n"
        param_str += "* \param[in] {0}Id {1} ID to update\n".format(api_struct.get_lower_name(), api_struct.get_name())
        for ecproperty in api_struct.get_properties():
            if ecproperty.hasAttribute("readOnly") and ecproperty.attributes["readOnly"].value:
                continue
            param_str += "* \param[in] {0}\n".format(ecproperty.attributes["propertyName"].value)
        param_str += "* \\return Success or error code. See \\ref {0}StatusCodes".format(self._api.get_api_name())
        self._file.write(self._COMMENT_GroupBriefLong.format("Update an existing {0}".format(api_struct.get_lower_name()), param_str))
        self._file.write(api_struct.get_api_gws_update_definition())
        self._write_spacing()

    def __write_api_delete_definition(self, api_struct):
        self._file.write(self._COMMENT_GroupBriefLong.format("Delete a {0}".format(api_struct.get_lower_name()),
                                                             "\param[in] apiHandle API object\n"
                                                             "* \param[in] {0}Id {1} ID to remove\n"
                                                             "* \\return Success or error code. See \\ref {2}StatusCodes"
                                                             .format(api_struct.get_lower_name(), api_struct.get_name(),
                                                                     self._api.get_api_name())))
        self._file.write(api_struct.get_api_gws_delete_definition())
        self._write_spacing()
