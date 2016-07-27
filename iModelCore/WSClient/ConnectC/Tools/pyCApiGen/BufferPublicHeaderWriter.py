from Writer import Writer


class BufferPublicHeaderWriter(Writer):
    def __init__(self, ecschemas, header_filename, api, status_codes):
        super(BufferPublicHeaderWriter, self).__init__(ecschemas, header_filename, api, status_codes)

    def write_header(self):
        self._write_header_comments(True, True)
        self._write_spacing()
        self.__write_includes()
        self._write_spacing()
        self.__write_api_function_definitions()

    def __write_includes(self):
        self._file.write('#include <WebServices/ConnectC/{0}.h>\n'.format(self._api.get_upper_api_acronym()))

    def __write_api_function_definitions(self):
        self._file.write(self._COMMENT_GroupStart.format("BufferFunctions", "{0} Data Buffer Functions".format(self._api.get_api_name())))
        self.__write_api_free_buffer_function_definition()
        self.__write_api_count_buffer_function_definition()
        self.__write_api_get_functions()
        self._file.write(self._COMMENT_GroupEnd)

    def __write_api_free_buffer_function_definition(self):
        self._file.write(self._COMMENT_GroupBriefLong
                         .format("Free an allocated data buffer",
                                 "\param[in] apiHandle Handle to api\n* \param[in] dataBuffer Data buffer"))
        self._file.write("{0}_EXPORT CallStatus {1}_DataBufferFree\n".format(self._api.get_upper_api_acronym(), self._api.get_api_name()))
        self._file.write("(\n")
        self._file.write("{0}HANDLE apiHandle,\n{0}DATABUFHANDLE dataBuffer\n".format(self._api.get_upper_api_acronym()))
        self._file.write(");\n")

    def __write_api_count_buffer_function_definition(self):
        self._file.write(self._COMMENT_GroupBriefLong
                         .format("Get a count of the number of items in a data buffer",
                                 "\param[in] dataBuffer Data buffer\n* \\return Object count"))
        self._file.write("{0}_EXPORT uint64_t {1}_DataBufferGetCount\n".format(self._api.get_upper_api_acronym(), self._api.get_api_name()))
        self._file.write("(\n")
        self._file.write("{0}DATABUFHANDLE dataBuffer\n".format(self._api.get_upper_api_acronym()))
        self._file.write(");\n")

    def __write_api_get_functions(self):
        self.__write_api_string_accessors()
        self.__write_api_datetime_accessor()
        self.__write_api_guid_accessor()
        self.__write_api_bool_accessor()
        self.__write_api_integer_accessor()
        self.__write_api_double_accessor()
        self.__write_api_long_accessor()

    def __write_api_string_accessors(self):
        if self.ecschemas_have_ecclass_with_property_type('string'):
            self._file.write(self._get_api_buffer_accessor_function_comment("string"))
            self._file.write(self._get_api_buffer_accessor_function_definition("string"))
            self._write_spacing()
        if self.ecschemas_have_ecclass_with_property_type('StringLength'):
            self._file.write(self._get_api_buffer_accessor_function_comment("StringLength"))
            self._file.write(self._get_api_buffer_accessor_function_definition("StringLength"))
            self._write_spacing()

    def __write_api_datetime_accessor(self):
        if self.ecschemas_have_ecclass_with_property_type('dateTime'):
            self._file.write(self._get_api_buffer_accessor_function_comment("dateTime"))
            self._file.write(self._get_api_buffer_accessor_function_definition("dateTime"))
            self._write_spacing()

    def __write_api_guid_accessor(self):
        if self.ecschemas_have_ecclass_with_property_type('guid'):
            self._file.write(self._get_api_buffer_accessor_function_comment("guid"))
            self._file.write(self._get_api_buffer_accessor_function_definition("guid"))
            self._write_spacing()

    def __write_api_bool_accessor(self):
        if self.ecschemas_have_ecclass_with_property_type('boolean'):
            self._file.write(self._get_api_buffer_accessor_function_comment("boolean"))
            self._file.write(self._get_api_buffer_accessor_function_definition("boolean"))
            self._write_spacing()

    def __write_api_integer_accessor(self):
        if self.ecschemas_have_ecclass_with_property_type('int'):
            self._file.write(self._get_api_buffer_accessor_function_comment("int"))
            self._file.write(self._get_api_buffer_accessor_function_definition("int"))
            self._write_spacing()

    def __write_api_double_accessor(self):
        if self.ecschemas_have_ecclass_with_property_type('double'):
            self._file.write(self._get_api_buffer_accessor_function_comment("double"))
            self._file.write(self._get_api_buffer_accessor_function_definition("double"))
            self._write_spacing()

    def __write_api_long_accessor(self):
        if self.ecschemas_have_ecclass_with_property_type('long'):
            self._file.write(self._get_api_buffer_accessor_function_comment("long"))
            self._file.write(self._get_api_buffer_accessor_function_definition("long"))
            self._write_spacing()

