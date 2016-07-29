from Writer import Writer


class BufferSourceWriter(Writer):
    def __init__(self, ecschemas, source_filename, api, status_codes):
        super(BufferSourceWriter, self).__init__(ecschemas, source_filename, api, status_codes)

    def write_source(self):
        self._write_header_comments()
        self._write_spacing()
        self.__write_includes()
        self._write_spacing()
        self.__write_wsg_functions()

    def __write_includes(self):
        self._file.write('#include "{0}Internal.h"\n'.format(self._api.get_upper_api_acronym()))

    def __write_wsg_functions(self):
        self.__write_wsg_buffer_free_function()
        self._write_spacing()
        self.__write_wsg_get_functions()

    def __write_wsg_buffer_free_function(self):
        self._file.write(self._COMMENT_BsiMethod)
        self._file.write("CallStatus WSG_DataBufferFree\n")
        self._file.write("(\n")
        self._file.write("LP{0} api,\nH{0}BUFFER buf\n".format(self._api.get_upper_api_acronym()))
        self._file.write(")\n")
        self._file.write("    {\n")
        self._file.write("    switch(buf->lSchemaType)\n")
        self._file.write("        {\n")
        for schema in self._ecschemas:
            self._file.write("        case SCHEMA_TYPE_{0}:\n".format(schema.get_upper_name()))
            self._file.write("            {\n")
            self._file.write("            return {0}_DataBufferFree(api, buf);\n".format(schema.get_name()))
            self._file.write("            }\n")
        self._file.write("        default:\n")
        self._file.write('            api->SetStatusMessage("{1}");\n            api->SetStatusDescription("{2}");\n'
                         '            return {0};\n        }}\n'.format("INVALID_PARAMETER", self._status_codes["SUCCESS"].message,
                                                                        "The dataBuffer passed into {0}_DataBufferFree is invalid."
                                                                        .format(self._api.get_api_name())))
        self._file.write("    }\n")

    def __write_wsg_get_functions(self):
        self.__write_wsg_accessors('string')
        self._write_spacing()
        self.__write_wsg_accessors('StringLength')
        self._write_spacing()
        self.__write_wsg_accessors('dateTime')
        self._write_spacing()
        self.__write_wsg_accessors('guid')
        self._write_spacing()
        self.__write_wsg_accessors('boolean')
        self._write_spacing()
        self.__write_wsg_accessors('int')
        self._write_spacing()
        self.__write_wsg_accessors('double')
        self._write_spacing()
        self.__write_wsg_accessors('long')
        self._write_spacing()

    def __write_wsg_accessors(self, property_type):
        if self.ecschemas_have_ecclass_with_property_type(property_type):
            self._file.write(self._COMMENT_BsiMethod)
            self._file.write(self._get_wsg_buffer_accessor_function_implementation(property_type))
