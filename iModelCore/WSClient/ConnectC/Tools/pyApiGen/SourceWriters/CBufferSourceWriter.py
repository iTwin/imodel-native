from Helpers.CBufferStruct import CBufferStruct
from SourceWriters.SourceWriter import SourceWriter
from PropertyTypeError import PropertyTypeError


class CBufferSourceWriter(SourceWriter):
    def __init__(self, ecclasses, source_filename, api, status_codes, excluded_classes=None):
        super(CBufferSourceWriter, self).__init__(ecclasses, source_filename, api, status_codes, excluded_classes)
        self.__buffer_structs = []
        for ecclass in self._ecclasses:
            if ecclass.attributes["typeName"].value not in self._excluded_classes:
                self.__buffer_structs.append(CBufferStruct(ecclass, api, status_codes))

    def write_source(self):
        self._write_header_comments()
        self._write_spacing()
        self.__write_includes()
        self._write_spacing()
        self.__write_utility_functions()
        self.__write_api_funtions()
        self._write_spacing()
        self.__write_buffer_accessor_funtions()
        self._write_spacing()
        self.__write_buffer_stuffer_functions()
        self._close_file()

    def __write_includes(self):
        self._file.write('#include "{0}Internal.h"\n'.format(self._api.get_upper_api_acronym()))

    def __write_utility_functions(self):
        self._write_string_to_wstring_function()

    def __write_buffer_stuffer_functions(self):
        self._write_spacing()
        for buffer_struct in self.__buffer_structs:
            self._file.write(buffer_struct.get_stuffer_implementation())
            self._write_spacing()

    def __write_api_funtions(self):
        self.__write_api_buffer_free_function()
        self._write_spacing()
        self.__write_api_buffer_count_function()
        self._write_spacing()
        self.__write_api_get_functions()

    def __write_buffer_accessor_funtions(self):
        if self.__write_buffer_accessors("string") > 0:
            self._write_spacing()
            if self.__write_buffer_accessors("StringLength") > 0:
                self._write_spacing()
        if self.__write_buffer_accessors("guid") > 0:
            self._write_spacing()
        if self.__write_buffer_accessors("boolean") > 0:
            self._write_spacing()
        if self.__write_buffer_accessors("int") > 0:
            self._write_spacing()
        if self.__write_buffer_accessors("double") > 0:
            self._write_spacing()
        if self.__write_buffer_accessors("long") > 0:
            self._write_spacing()

    def __write_api_buffer_free_function(self):
        self._file.write(self._COMMENT_BsiMethod)
        self._file.write("CallStatus {0}_DataBufferFree\n".format(self._api.get_api_name()))
        self._file.write("(\n")
        self._file.write("{0}HANDLE apiHandle,\n{0}DATABUFHANDLE dataBuffer\n".format(self._api.get_upper_api_acronym()))
        self._file.write(")\n")
        self._file.write("    {\n")
        self._file.write("    VERIFY_API\n")
        self._file.write("    if (nullptr == dataBuffer)\n")
        self._file.write('        {\n')
        self._file.write('        api->SetStatusMessage("{1}");\n        api->SetStatusDescription("{2}");\n'
                         '        return {0};\n        }}\n\n'.format("INVALID_PARAMETER", self._status_codes["INVALID_PARAMETER"].message,
                                                                      "The dataBuffer passed into {0}_DataBufferFree is invalid."
                                                                      .format(self._api.get_api_name())))
        self._file.write("    H{0}BUFFER buf = (H{0}BUFFER)dataBuffer;\n".format(self._api.get_api_acronym()))
        self._file.write("    for (int index = 0; index < buf->lItems.size(); index++)\n")
        self._file.write("        {\n")
        self._file.write("        if (buf->lItems[index] != nullptr)\n")
        self._file.write("            {\n")
        self._file.write("            switch(buf->lType)\n")
        self._file.write("                {\n")
        for buffer_struct in self.__buffer_structs:
            self._file.write("                case BUFF_TYPE_{0}:\n".format(buffer_struct.get_upper_name()))
            self._file.write("                    {\n")
            self._file.write("                    LP{0}{1}BUFFER {2}Buf = (LP{0}{1}BUFFER) buf->lItems[index];\n"
                             .format(self._api.get_upper_api_acronym(), buffer_struct.get_upper_name(), buffer_struct.get_lower_name()))
            self._file.write("                    delete {0}Buf;\n".format(buffer_struct.get_lower_name()))
            self._file.write("                    }\n")
            self._file.write("                    break;\n")
        self._file.write("                default:\n")
        self._file.write("                    continue;\n")
        self._file.write("                }\n")
        self._file.write("            }\n")
        self._file.write("        }\n")
        self._file.write("    free(buf);\n")
        self._file.write('    api->SetStatusMessage("{1}");\n    api->SetStatusDescription("{2}");\n'
                         '    return {0};\n    }}\n'.format("SUCCESS", self._status_codes["SUCCESS"].message,
                                                            "The {0}_DataBufferFree function successfully completed."
                                                            .format(self._api.get_api_name())))

    def __write_api_buffer_count_function(self):
        self._file.write(self._COMMENT_BsiMethod)
        self._file.write("uint64_t {0}_DataBufferGetCount\n".format(self._api.get_api_name()))
        self._file.write("(\n")
        self._file.write("{0}DATABUFHANDLE dataBuffer\n".format(self._api.get_upper_api_acronym()))
        self._file.write(")\n")
        self._file.write("    {\n")
        self._file.write("    if (nullptr == dataBuffer)\n")
        self._file.write('        return 0;\n\n')
        self._file.write("    H{0}BUFFER buf = (H{0}BUFFER)dataBuffer;\n".format(self._api.get_api_acronym()))
        self._file.write("    return buf->lCount;\n")
        self._file.write("    }\n")

    def __write_api_get_functions(self):
        self.__write_api_string_accessors()
        self._write_spacing()
        self.__write_api_guid_accessor()
        self._write_spacing()
        self.__write_api_bool_accessor()
        self._write_spacing()
        self.__write_api_integer_accessor()
        self._write_spacing()
        self.__write_api_double_accessor()
        self._write_spacing()
        self.__write_api_long_accessor()
        self._write_spacing()

    def __write_api_string_accessors(self):
        ecclass_contains_a_string_property = False
        for buffer_struct in self.__buffer_structs:
            if buffer_struct.does_contain_string():
                ecclass_contains_a_string_property = True
                break
        if ecclass_contains_a_string_property:
            self._file.write(self._COMMENT_BsiMethod)
            self._file.write(self.__get_api_accessor("string"))
            self._write_spacing()
            self._file.write(self._COMMENT_BsiMethod)
            self._file.write(self.__get_api_accessor("StringLength"))

    def __write_api_guid_accessor(self):
        ecclass_contains_a_guid_property = False
        for buffer_struct in self.__buffer_structs:
            if buffer_struct.does_contain_guid():
                ecclass_contains_a_guid_property = True
                break
        if ecclass_contains_a_guid_property:
            self._file.write(self._COMMENT_BsiMethod)
            self._file.write(self.__get_api_accessor("guid"))

    def __write_api_bool_accessor(self):
        ecclass_contains_a_boolean_property = False
        for buffer_struct in self.__buffer_structs:
            if buffer_struct.does_contain_boolean():
                ecclass_contains_a_boolean_property = True
                break
        if ecclass_contains_a_boolean_property:
            self._file.write(self._COMMENT_BsiMethod)
            self._file.write(self.__get_api_accessor("boolean"))

    def __write_api_integer_accessor(self):
        ecclass_contains_a_integer_property = False
        for buffer_struct in self.__buffer_structs:
            if buffer_struct.does_contain_int():
                ecclass_contains_a_integer_property = True
                break
        if ecclass_contains_a_integer_property:
            self._file.write(self._COMMENT_BsiMethod)
            self._file.write(self.__get_api_accessor("int"))

    def __write_api_double_accessor(self):
        ecclass_contains_a_double_property = False
        for buffer_struct in self.__buffer_structs:
            if buffer_struct.does_contain_int():
                ecclass_contains_a_double_property = True
                break
        if ecclass_contains_a_double_property:
            self._file.write(self._COMMENT_BsiMethod)
            self._file.write(self.__get_api_accessor("double"))

    def __write_api_long_accessor(self):
        ecclass_contains_a_long_property = False
        for buffer_struct in self.__buffer_structs:
            if buffer_struct.does_contain_long():
                ecclass_contains_a_long_property = True
                break
        if ecclass_contains_a_long_property:
            self._file.write(self._COMMENT_BsiMethod)
            self._file.write(self.__get_api_accessor("long"))

    def __get_api_accessor(self, property_type):
        if property_type is 'StringLength':
            accessor_str = "CallStatus {0}_DataBufferGetStringLength\n".format(self._api.get_api_name())
        else:
            accessor_str = "CallStatus {0}_DataBufferGet{1}Property\n".format(self._api.get_api_name(), property_type.title())
        accessor_str += "(\n"
        accessor_str += "{0}HANDLE apiHandle,\n{0}DATABUFHANDLE dataBuffer,\n".format(self._api.get_upper_api_acronym())
        accessor_str += "int16_t bufferProperty,\n"
        accessor_str += "uint32_t index,\n"
        if property_type == "string":
            accessor_str += "uint32_t strLength,\n"
            accessor_str += "WCharP str\n"
        elif property_type == "StringLength":
            accessor_str += "size_t* outStringSize\n"
        elif property_type == "guid":
            accessor_str += "uint32_t strLength,\n"
            accessor_str += "WCharP guid\n"
        elif property_type == "boolean":
            accessor_str += "bool* boolean\n"
        elif property_type == "int":
            accessor_str += "int32_t* integer\n"
        elif property_type == "double":
            accessor_str += "double* pDouble\n"
        elif property_type == "long":
            accessor_str += "int64_t* pLong\n"
        else:
            raise PropertyTypeError("Property type {0} not accepted".format(property_type))
        accessor_str += ")\n"
        accessor_str += "    {\n"
        accessor_str += "    VERIFY_API\n"
        accessor_str += "    if(nullptr == dataBuffer)\n"
        accessor_str += '        {\n'
        accessor_str += '        api->SetStatusMessage("{1}");\n        api->SetStatusDescription("{2}");\n' \
                        '        return {0};\n        }}\n\n'.format("INVALID_PARAMETER", self._status_codes["INVALID_PARAMETER"],
                                                                     "The dataBuffer passed into {0} data access function is invalid."
                                                                     .format(self._api.get_api_name()))
        accessor_str += "    H{0}BUFFER buf = (H{0}BUFFER) dataBuffer;\n\n".format(self._api.get_api_acronym())
        accessor_str += "    switch (buf->lType)\n"
        accessor_str += "        {\n"
        for buffer_struct in self.__buffer_structs:
            if buffer_struct.does_contain_property_type(property_type):
                accessor_str += "        case BUFF_TYPE_{0}:\n".format(buffer_struct.get_upper_name())
                accessor_str += "            "
                if property_type is 'StringLength':
                    accessor_str += "return {0}GetStringLength".format(buffer_struct.get_lower_name())
                else:
                    accessor_str += "return {0}Get{1}Property".format(buffer_struct.get_lower_name(), property_type.title())
                if property_type == "string":
                    accessor_str += "(api, buf, bufferProperty, index, strLength, str);\n"
                elif property_type == "StringLength":
                    accessor_str += "(api, buf, bufferProperty, index, outStringSize);\n"
                elif property_type == "guid":
                    accessor_str += "(api, buf, bufferProperty, index, strLength, guid);\n"
                elif property_type == "boolean":
                    accessor_str += "(api, buf, bufferProperty, index, boolean);\n"
                elif property_type == "int":
                    accessor_str += "(api, buf, bufferProperty, index, integer);\n"
                elif property_type == "double":
                    accessor_str += "(api, buf, bufferProperty, index, pDouble);\n"
                elif property_type == "long":
                    accessor_str += "(api, buf, bufferProperty, index, pLong);\n"
                else:
                    raise PropertyTypeError("Property type {0} not accepted".format(property_type))
        accessor_str += "        default:\n"
        accessor_str += '        api->SetStatusMessage("{1}");\n        api->SetStatusDescription("{2}");\n' \
                        '        return {0};\n'.format("INVALID_PARAMETER", self._status_codes["INVALID_PARAMETER"],
                                                       "The buffer type passed in is invalid.")
        accessor_str += "        }\n"
        accessor_str += "    }\n"
        return accessor_str

    def __write_buffer_accessors(self, property_type):
        num_written = 0
        for buffer_struct in self.__buffer_structs:
            if buffer_struct.does_contain_property_type(property_type):
                self._file.write(self._COMMENT_BsiMethod)
                self._file.write(buffer_struct.get_accessor_function_implementation(property_type))
                self._write_spacing()
                num_written += 1
        return num_written
