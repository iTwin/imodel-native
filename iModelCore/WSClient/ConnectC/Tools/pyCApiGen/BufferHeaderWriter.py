from Writer import Writer


class BufferHeaderWriter(Writer):
    def __init__(self, ecschemas, header_filename, api, status_codes):
        super(BufferHeaderWriter, self).__init__(ecschemas, header_filename, api, status_codes)

    def write_header(self):
        self._write_header_comments(True)
        self._write_spacing()
        self.__write_includes()
        self._write_spacing()
        self.__write_defines()
        self._write_spacing()
        self.__write_wsg_function_definitions()

    def __write_includes(self):
        self._file.write('#include <Bentley/bvector.h>\n')
        self._file.write('#include "{0}Private.h"\n'.format(self._api.get_upper_api_acronym()))

    def __write_defines(self):
        define_index = 1
        for schema in self._ecschemas:
            self._file.write('#define SCHEMA_TYPE_{0}{1:20}\n'.format(schema.get_upper_name(), define_index))
            define_index += 1

    def __write_wsg_function_definitions(self):
        self.__write_wsg_free_buffer_function_definition()
        self._write_spacing()
        self.__write_wsg_get_functions()

    def __write_wsg_free_buffer_function_definition(self):
        self._file.write("/************************************************************************************//**\n")
        self._file.write("* Free an allocated wsg buffer\n")
        self._file.write("****************************************************************************************/\n")
        self._file.write("CallStatus WSG_DataBufferFree\n")
        self._file.write("(\n")
        self._file.write("LP{0} api,\nH{0}BUFFER buf\n".format(self._api.get_upper_api_acronym()))
        self._file.write(");\n")

    def __write_wsg_get_functions(self):
        self.__write_wsg_string_accessors()
        self.__write_wsg_datetime_accessor()
        self.__write_wsg_guid_accessor()
        self.__write_wsg_bool_accessor()
        self.__write_wsg_integer_accessor()
        self.__write_wsg_double_accessor()
        self.__write_wsg_long_accessor()

    def __write_wsg_string_accessors(self):
        if self.ecschemas_have_ecclass_with_property_type('string'):
            self._file.write(self._get_wsg_buffer_accessor_function_comment("string"))
            self._file.write(self._get_wsg_buffer_accessor_function_definition("string"))
            self._write_spacing()
        if self.ecschemas_have_ecclass_with_property_type('StringLength'):
            self._file.write(self._get_wsg_buffer_accessor_function_comment("StringLength"))
            self._file.write(self._get_wsg_buffer_accessor_function_definition("StringLength"))
            self._write_spacing()

    def __write_wsg_datetime_accessor(self):
        if self.ecschemas_have_ecclass_with_property_type('dateTime'):
            self._file.write(self._get_wsg_buffer_accessor_function_comment("dateTime"))
            self._file.write(self._get_wsg_buffer_accessor_function_definition("dateTime"))
            self._write_spacing()

    def __write_wsg_guid_accessor(self):
        if self.ecschemas_have_ecclass_with_property_type('guid'):
            self._file.write(self._get_wsg_buffer_accessor_function_comment("guid"))
            self._file.write(self._get_wsg_buffer_accessor_function_definition("guid"))
            self._write_spacing()

    def __write_wsg_bool_accessor(self):
        if self.ecschemas_have_ecclass_with_property_type('boolean'):
            self._file.write(self._get_wsg_buffer_accessor_function_comment("boolean"))
            self._file.write(self._get_wsg_buffer_accessor_function_definition("boolean"))
            self._write_spacing()

    def __write_wsg_integer_accessor(self):
        if self.ecschemas_have_ecclass_with_property_type('int'):
            self._file.write(self._get_wsg_buffer_accessor_function_comment("int"))
            self._file.write(self._get_wsg_buffer_accessor_function_definition("int"))
            self._write_spacing()

    def __write_wsg_double_accessor(self):
        if self.ecschemas_have_ecclass_with_property_type('double'):
            self._file.write(self._get_wsg_buffer_accessor_function_comment("double"))
            self._file.write(self._get_wsg_buffer_accessor_function_definition("double"))
            self._write_spacing()

    def __write_wsg_long_accessor(self):
        if self.ecschemas_have_ecclass_with_property_type('long'):
            self._file.write(self._get_wsg_buffer_accessor_function_comment("long"))
            self._file.write(self._get_wsg_buffer_accessor_function_definition("long"))
            self._write_spacing()


