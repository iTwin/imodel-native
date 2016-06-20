from SchemaWriter.HeaderWriters.SchemaHeaderWriter import SchemaHeaderWriter


class SchemaBufferHeaderWriter(SchemaHeaderWriter):
    __COMMENT_Function = "/*--------------------------------------------------------------------------------------+\n" \
                         "| {0} functions\n" 																		   \
                         "+--------------------------------------------------------------------------------------*/\n"

    __COMMENT_Buffer = "/*--------------------------------------------------------------------------------------+\n"   \
                       "| Internal buffers\n"																	       \
                       "+--------------------------------------------------------------------------------------*/\n"

    def __init__(self, ecschema, header_filename, api, status_codes):
        super(SchemaBufferHeaderWriter, self).__init__(ecschema, header_filename, api, status_codes)

    def write_header(self):
        self.__write_header_comment()
        self._write_spacing()
        self.__write_includes()
        self._write_spacing()
        self.__write_defines()
        self._write_spacing()
        self.__write_buffer_free_defintion()
        self._write_spacing()
        self.__write_accessor_functions()
        self._write_spacing()
        self.__write_internal_buffers()
        self._close_file()

    def __write_header_comment(self):
        self._write_header_comments(True)

    def __write_includes(self):
        self._file.write('#include "../{0}BufferGen.h"\n'.format(self._api.get_upper_api_acronym()))
        self._file.write('#include "../{0}Private.h"\n'.format(self._api.get_upper_api_acronym()))

    def __write_defines(self):
        count = 1
        for ecclass in self._ecschema.get_classes():
            if ecclass.should_exclude_entire_class():
                continue
            self._file.write('#define BUFF_TYPE_' + ecclass.get_upper_name())
            self._file.write(' {0}'.format(count) + '\n')
            count += 1

    def __write_buffer_free_defintion(self):
        self._file.write(self.__COMMENT_Function
                         .format('Schema-level free'))
        self._file.write('CallStatus {0}_DataBufferFree\n'.format(self._ecschema.get_name()))
        self._file.write('(\n')
        self._file.write('LP{0} api,\n'.format(self._api.get_upper_api_acronym()))
        self._file.write('H{0}BUFFER buf\n'.format(self._api.get_upper_api_acronym()))
        self._file.write(');\n')

    def __write_accessor_functions(self):
        self.__write_schema_accessor_functions()
        self._write_spacing()
        self.__write_string_functions()
        self._write_spacing()
        self.__write_string_datetime_functions()
        self._write_spacing()
        self.__write_guid_functions()
        self._write_spacing()
        self.__write_bool_functions()
        self._write_spacing()
        self.__write_integer_functions()
        self._write_spacing()
        self.__write_double_functions()
        self._write_spacing()
        self.__write_long_functions()
        self._write_spacing()

    def __write_schema_accessor_functions(self):
        self._file.write(self.__COMMENT_Function
                         .format('Schema-level accessor'))
        if self._ecschema.has_ecclass_with_property_type('string'):
            self._file.write(self._ecschema.get_buffer_accessor_function_definition('string'))
            self._write_spacing()
        if self._ecschema.has_ecclass_with_property_type('StringLength'):
            self._file.write(self._ecschema.get_buffer_accessor_function_definition('StringLength'))
            self._write_spacing()
        if self._ecschema.has_ecclass_with_property_type('dateTime'):
            self._file.write(self._ecschema.get_buffer_accessor_function_definition('dateTime'))
            self._write_spacing()
        if self._ecschema.has_ecclass_with_property_type('guid'):
            self._file.write(self._ecschema.get_buffer_accessor_function_definition('guid'))
            self._write_spacing()
        if self._ecschema.has_ecclass_with_property_type('int'):
            self._file.write(self._ecschema.get_buffer_accessor_function_definition('int'))
            self._write_spacing()
        if self._ecschema.has_ecclass_with_property_type('double'):
            self._file.write(self._ecschema.get_buffer_accessor_function_definition('double'))
            self._write_spacing()
        if self._ecschema.has_ecclass_with_property_type('long'):
            self._file.write(self._ecschema.get_buffer_accessor_function_definition('long'))
            self._write_spacing()
        if self._ecschema.has_ecclass_with_property_type('boolean'):
            self._file.write(self._ecschema.get_buffer_accessor_function_definition('boolean'))
            self._write_spacing()

    def __write_string_functions(self):
        ecclasses_with_string_property = []
        for ecclass in self._ecschema.get_classes():
            if ecclass.does_contain_string() and not ecclass.should_exclude_entire_class():
                if len(ecclasses_with_string_property) == 0:
                    self._file.write(self.__COMMENT_Function.format('String'))
                ecclasses_with_string_property.append(ecclass)
                self._file.write(ecclass.get_buffer_accessor_function_definition('string'))
                self._write_spacing()
        self.__write_string_length_functions(ecclasses_with_string_property)

    def __write_string_length_functions(self, ecclasses_with_string_property):
        if len(ecclasses_with_string_property) == 0:
            return
        is_first = True
        for ecclass in ecclasses_with_string_property:
            if not ecclass.should_exclude_entire_class():
                if is_first:
                    self._file.write(self.__COMMENT_Function.format('String length'))
                    is_first = False
                self._file.write(ecclass.get_buffer_accessor_function_definition('StringLength'))
                self._write_spacing()

    def __write_string_datetime_functions(self):
        ecclasses_with_datetime_property = []
        for ecclass in self._ecschema.get_classes():
            if ecclass.does_contain_datetime() and not ecclass.should_exclude_entire_class():
                if len(ecclasses_with_datetime_property) == 0:
                    self._file.write(self.__COMMENT_Function.format('DateTime'))
                    ecclasses_with_datetime_property.append(ecclass)
                self._file.write(ecclass.get_buffer_accessor_function_definition('dateTime'))
                self._write_spacing()

    def __write_guid_functions(self):
        is_first = True
        for ecclass in self._ecschema.get_classes():
            if ecclass.does_contain_guid() and not ecclass.should_exclude_entire_class():
                if is_first:
                    self._file.write(self.__COMMENT_Function.format('Guid'))
                    is_first = False
                self._file.write(ecclass.get_buffer_accessor_function_definition('guid'))
                self._write_spacing()

    def __write_bool_functions(self):
        is_first = True
        for ecclass in self._ecschema.get_classes():
            if ecclass.does_contain_boolean() and not ecclass.should_exclude_entire_class():
                if is_first:
                    self._file.write(self.__COMMENT_Function.format('Bool'))
                    is_first = False
                self._file.write(ecclass.get_buffer_accessor_function_definition('boolean'))
                self._write_spacing()

    def __write_integer_functions(self):
        is_first = True
        for ecclass in self._ecschema.get_classes():
            if ecclass.does_contain_int() and not ecclass.should_exclude_entire_class():
                if is_first:
                    self._file.write(self.__COMMENT_Function.format('Integer'))
                    is_first = False
                self._file.write(ecclass.get_buffer_accessor_function_definition('int'))
                self._write_spacing()

    def __write_double_functions(self):
        is_first = True
        for ecclass in self._ecschema.get_classes():
            if ecclass.does_contain_double() and not ecclass.should_exclude_entire_class():
                if is_first:
                    self._file.write(self.__COMMENT_Function.format('Double'))
                    is_first = False
                self._file.write(ecclass.get_buffer_accessor_function_definition('double'))
                self._write_spacing()

    def __write_long_functions(self):
        is_first = True
        for ecclass in self._ecschema.get_classes():
            if ecclass.does_contain_long() and not ecclass.should_exclude_entire_class():
                if is_first:
                    self._file.write(self.__COMMENT_Function.format('Long'))
                    is_first = False
                self._file.write(ecclass.get_buffer_accessor_function_definition('long'))
                self._write_spacing()

    def __write_internal_buffers(self):
        if len(self._ecschema.get_classes()) == 0:
            return
        self._file.write(self.__COMMENT_Buffer)
        for ecclass in self._ecschema.get_classes():
            if ecclass.should_exclude_entire_class():
                continue
            self._file.write(ecclass.get_struct_typedef())
            self._write_spacing()
            self._file.write(ecclass.get_buffer_stuffer_function_definition())
            self._write_spacing()
