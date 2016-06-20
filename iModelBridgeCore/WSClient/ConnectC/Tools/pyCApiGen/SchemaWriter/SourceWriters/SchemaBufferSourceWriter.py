from SchemaWriter.Helpers.PropertyTypeError import PropertyTypeError
from SchemaWriter.SourceWriters.SchemaSourceWriter import SchemaSourceWriter


class SchemaBufferSourceWriter(SchemaSourceWriter):
    def __init__(self, ecschema, source_filename, api, status_codes):
        super(SchemaBufferSourceWriter, self).__init__(ecschema, source_filename, api, status_codes)

    def write_source(self):
        self._write_header_comments()
        self._write_spacing()
        self.__write_includes()
        self._write_spacing()
        self.__write_utility_functions()
        self._write_spacing()
        self.__write_schema_functions()
        self._write_spacing()
        self.__write_class_functions()
        self._close_file()

    def __write_includes(self):
        self._file.write('#include "{0}Internal.h"\n'.format(self._api.get_upper_api_acronym()))

    def __write_utility_functions(self):
        self._write_string_to_wstring_function()

    def __write_schema_functions(self):
        self.__write_schema_buffer_free_function()
        self._write_spacing()
        self.__write_schema_buffer_accessor_functions()

    def __write_class_functions(self):
        self.__write_class_buffer_accessor_functions()
        self._write_spacing()
        self.__write_class_buffer_stuffer_functions()

    def __write_schema_buffer_free_function(self):
        self._file.write(self._COMMENT_BsiMethod)
        self._file.write(self._ecschema.get_buffer_free_function_implementation())

    def __write_schema_buffer_accessor_functions(self):
        if self.__write_schema_buffer_accessor('string'):
            self._write_spacing()
            if self.__write_schema_buffer_accessor('StringLength'):
                self._write_spacing()
        if self.__write_schema_buffer_accessor('dateTime'):
            self._write_spacing()
        if self.__write_schema_buffer_accessor('guid'):
            self._write_spacing()
        if self.__write_schema_buffer_accessor('boolean'):
            self._write_spacing()
        if self.__write_schema_buffer_accessor('int'):
            self._write_spacing()
        if self.__write_schema_buffer_accessor('double'):
            self._write_spacing()
        if self.__write_schema_buffer_accessor('long'):
            self._write_spacing()

    def __write_schema_buffer_accessor(self, property_type):
        if self._ecschema.has_ecclass_with_property_type(property_type):
            self._file.write(self._COMMENT_BsiMethod)
            self._file.write(self._ecschema.get_buffer_accessor_function_implementation(property_type))
            return True
        return False

    def __write_class_buffer_accessor_functions(self):
        if self.__write_class_buffer_accessors("string") > 0:
            self._write_spacing()
            if self.__write_class_buffer_accessors("StringLength") > 0:
                self._write_spacing()
        if self.__write_class_buffer_accessors("dateTime") > 0:
            self._write_spacing()
        if self.__write_class_buffer_accessors("guid") > 0:
            self._write_spacing()
        if self.__write_class_buffer_accessors("boolean") > 0:
            self._write_spacing()
        if self.__write_class_buffer_accessors("int") > 0:
            self._write_spacing()
        if self.__write_class_buffer_accessors("double") > 0:
            self._write_spacing()
        if self.__write_class_buffer_accessors("long") > 0:
            self._write_spacing()

    def __write_class_buffer_accessors(self, property_type):
        num_written = 0
        for ecclass in self._ecschema.get_classes():
            if ecclass.should_exclude_entire_class():
                continue
            if ecclass.does_contain_property_type(property_type):
                self._file.write(self._COMMENT_BsiMethod)
                self._file.write(ecclass.get_buffer_accessor_function_implementation(property_type))
                self._write_spacing()
                num_written += 1
        return num_written

    def __write_class_buffer_stuffer_functions(self):
        for ecclass in self._ecschema.get_classes():
            if ecclass.should_exclude_entire_class():
                continue
            self._file.write(self._COMMENT_BsiMethod)
            self._file.write(ecclass.get_buffer_stuffer_function_implementation())

