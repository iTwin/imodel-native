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
        self.__write_internal_struct_typedef()

    def __write_includes(self):
        self._file.write('#include <Bentley/bvector.h>\n')

    def __write_internal_struct_typedef(self):
        self._file.write('typedef struct _{0}BUFFER\n'.format(self._api.get_upper_api_acronym()))
        self._file.write('{\n')
        self._file.write('uint32_t       lClassType;\n')
        self._file.write('uint32_t       lSchemaType;\n')
        self._file.write('uint64_t       lCount;\n')
        self._file.write('bvector<void*> lItems;\n')
        self._file.write('}} {0}BUFFER, *LP{0}BUFFER;\n\n'.format(self._api.get_upper_api_acronym()))
        self._file.write('typedef LP{0}BUFFER H{0}BUFFER;\n'.format(self._api.get_upper_api_acronym()))

    def __write_defines(self):
        define_index = 1
        for schema in self._ecschemas:
            self._file.write('#define SCHEMA_TYPE_{0}{1:20}\n'.format(schema.get_upper_name(), define_index))
            define_index += 1

