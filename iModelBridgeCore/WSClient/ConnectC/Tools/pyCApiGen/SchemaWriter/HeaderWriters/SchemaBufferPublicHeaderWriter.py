from SchemaWriter.HeaderWriters.SchemaHeaderWriter import SchemaHeaderWriter


class SchemaBufferPublicHeaderWriter(SchemaHeaderWriter):
    def __init__(self, ecschema, header_filename, api, status_codes):
        super(SchemaBufferPublicHeaderWriter, self).__init__(ecschema, header_filename, api, status_codes)

    def write_header(self):
        self.__write_header_comment()
        self._write_spacing()
        self.__write_buffer_enums()
        self._write_spacing()

    def __write_header_comment(self):
        self._write_header_comments(True, True)

    def __write_buffer_enums(self):
        self._file.write(self._COMMENT_GroupStart.format("BufferPropertyEnums", "{0} Buffer Property Enumerations"
                                                         .format(self._api.get_api_name())))
        for ecclass in self._ecschema.get_classes():
            if ecclass.should_exclude_entire_class():
                continue
            self._file.write(self._COMMENT_GroupBriefShort.format(ecclass.get_name()))
            self._file.write(ecclass.get_enum())
            self._write_spacing()
        self._file.write(self._COMMENT_GroupEnd)







