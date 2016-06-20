from SchemaWriter.SourceWriters.SchemaSourceWriter import SchemaSourceWriter


class SchemaApiSourceWriter(SchemaSourceWriter):
    def __init__(self, ecschema, source_filename, api, status_codes):
        super(SchemaApiSourceWriter, self).__init__(ecschema, source_filename, api, status_codes)

    def write_source(self):
        self._write_header_comments()
        self._write_spacing()
        self.__write_includes()
        self._write_spacing()
        self.__write_api_function_definitions()
        self._close_file()

    def __write_includes(self):
        self._file.write('#include "{0}Internal.h"\n'.format(self._api.get_upper_api_acronym()))

    def __write_api_function_definitions(self):
        self.__write_class_function_implementations()

    def __write_class_function_implementations(self):
        for ecclass in self._ecschema.get_classes():
            if ecclass.should_exclude_entire_class():
                continue
            if ecclass.should_have_read_list():
                self.__write_read_class_list_implementation(ecclass)
            if ecclass.should_have_create():
                self.__write_create_class_implementation(ecclass)
            if ecclass.should_have_read():
                self.__write_read_class_implementation(ecclass)
            if ecclass.should_have_update():
                self.__write_update_class_implementation(ecclass)
            if ecclass.should_have_delete():
                self.__write_delete_class_implementation(ecclass)

    def __write_read_class_list_implementation(self, ecclass):
        self._file.write(self._COMMENT_BsiMethod)
        self._file.write(ecclass.get_read_class_list_implementation())
        self._write_spacing()

    def __write_create_class_implementation(self, ecclass):
        self._file.write(self._COMMENT_BsiMethod)
        self._file.write(ecclass.get_create_class_implementation())
        self._write_spacing()

    def __write_read_class_implementation(self, ecclass):
        self._file.write(self._COMMENT_BsiMethod)
        self._file.write(ecclass.get_read_class_implementation())
        self._write_spacing()

    def __write_update_class_implementation(self, ecclass):
        self._file.write(self._COMMENT_BsiMethod)
        self._file.write(ecclass.get_update_class_implementation())
        self._write_spacing()

    def __write_delete_class_implementation(self, ecclass):
        self._file.write(self._COMMENT_BsiMethod)
        self._file.write(ecclass.get_delete_class_implementation())
        self._write_spacing()



