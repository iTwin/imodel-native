from Helpers.CBufferStruct import CBufferStruct
from HeaderWriters.HeaderWriter import HeaderWriter


class CBufferHeaderWriter(HeaderWriter):
    __COMMENT_Function = "/*--------------------------------------------------------------------------------------+\n" \
                         "| {0} functions\n" 																		   \
                         "+--------------------------------------------------------------------------------------*/\n"

    __COMMENT_Buffer = "/*--------------------------------------------------------------------------------------+\n"   \
                       "| Internal buffers\n"																	       \
                       "+--------------------------------------------------------------------------------------*/\n"

    def __init__(self, ecclasses, header_filename, api, status_codes, excluded_classes):
        super(CBufferHeaderWriter, self).__init__(ecclasses, header_filename, api, status_codes, excluded_classes)
        self.__buffer_structs = []
        for ecclass in self._ecclasses:
            if ecclass.attributes["typeName"].value in excluded_classes and \
                    excluded_classes[ecclass.attributes["typeName"].value].should_exclude_entire_class():
                continue
            self.__buffer_structs.append(CBufferStruct(ecclass, api, self._status_codes))

    def write_header(self):
        self.__write_header_comment()
        self._write_spacing()
        self.__write_includes()
        self._write_spacing()
        self.__write_defines()
        self._write_spacing()
        self.__write_parent_buffer()
        self._write_spacing()
        self.__write_accessor_functions()
        self._write_spacing()
        self.__write_internal_buffers()
        self._close_file()

    def __write_header_comment(self):
        self._write_header_comments(True)

    def __write_includes(self):
        self._file.write('#include <WebServices/ConnectC/{0}Public.h>\n'.format(self._api.get_api_acronym()))
        self._file.write('#include <DgnClientFx/Utils/Http/ProxyHttpHandler.h>\n')
        self._file.write('#include <Bentley/bvector.h>\n')

    def __write_defines(self):
        count = 1
        for ecclass in self._ecclasses:
            if ecclass.attributes["typeName"].value in self._excluded_classes and \
                    self._excluded_classes[ecclass.attributes["typeName"].value].should_exclude_entire_class():
                continue
            self._file.write('#define BUFF_TYPE_' + ecclass.attributes["typeName"].value.upper())
            self._file.write(' {0}'.format(count) + '\n')
            count += 1

    def __write_parent_buffer(self):
        self._file.write('typedef struct _{0}BUFFER\n'.format(self._api.get_api_acronym()))
        self._file.write('   {\n')
        self._file.write('   uint32_t       lType;\n')
        self._file.write('   uint64_t       lCount;\n')
        self._file.write('   bvector<void*> lItems;\n')
        self._file.write('   }} {0}BUFFER, *LP{0}BUFFER;\n\n'.format(self._api.get_api_acronym()))
        self._file.write('typedef LP{0}BUFFER H{0}BUFFER;'.format(self._api.get_api_acronym()))

    def __write_accessor_functions(self):
        self.__write_string_functions()
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

    def __write_string_functions(self):
        buffer_structs_with_string_property = []
        for buffer_struct in self.__buffer_structs:
            if buffer_struct.does_contain_string():
                if len(buffer_structs_with_string_property) == 0:
                    self._file.write(self.__COMMENT_Function.format('String'))
                buffer_structs_with_string_property.append(buffer_struct)
                self._file.write(buffer_struct.get_accessor_function_definition('string'))
                self._write_spacing()
        self.__write_string_length_functions(buffer_structs_with_string_property)

    def __write_string_length_functions(self, buffer_structs_with_string_property):
        if len(buffer_structs_with_string_property) == 0:
            return
        self._file.write(self.__COMMENT_Function.format('String length'))
        for bufferStruct in buffer_structs_with_string_property:
            self._file.write(bufferStruct.get_accessor_function_definition('StringLength'))
            self._write_spacing()

    def __write_guid_functions(self):
        count = 0
        for bufferStruct in self.__buffer_structs:
            if bufferStruct.does_contain_guid():
                if count == 0:
                    self._file.write(self.__COMMENT_Function.format('Guid'))
                    count += 1
                self._file.write(bufferStruct.get_accessor_function_definition('guid'))
                self._write_spacing()

    def __write_bool_functions(self):
        count = 0
        for bufferStruct in self.__buffer_structs:
            if bufferStruct.does_contain_boolean():
                if count == 0:
                    self._file.write(self.__COMMENT_Function.format('Bool'))
                    count += 1
                self._file.write(bufferStruct.get_accessor_function_definition('boolean'))
                self._write_spacing()

    def __write_integer_functions(self):
        count = 0
        for bufferStruct in self.__buffer_structs:
            if bufferStruct.does_contain_int():
                if count == 0:
                    self._file.write(self.__COMMENT_Function.format('Integer'))
                    count += 1
                self._file.write(bufferStruct.get_accessor_function_definition('int'))
                self._write_spacing()

    def __write_double_functions(self):
        count = 0
        for bufferStruct in self.__buffer_structs:
            if bufferStruct.does_contain_double():
                if count == 0:
                    self._file.write(self.__COMMENT_Function.format('Double'))
                    count += 1
                self._file.write(bufferStruct.get_accessor_function_definition('double'))
                self._write_spacing()

    def __write_long_functions(self):
        count = 0
        for bufferStruct in self.__buffer_structs:
            if bufferStruct.does_contain_long():
                if count == 0:
                    self._file.write(self.__COMMENT_Function.format('Long'))
                    count += 1
                self._file.write(bufferStruct.get_accessor_function_definition('long'))
                self._write_spacing()

    def __write_internal_buffers(self):
        if len(self.__buffer_structs) == 0:
            return
        self._file.write(self.__COMMENT_Buffer)
        for bufferStruct in self.__buffer_structs:
            self._file.write(bufferStruct.get_struct_typedef())
            self._write_spacing()
            self._write_spacing()
            self._file.write(bufferStruct.get_stuffer_definition())
            self._write_spacing()
