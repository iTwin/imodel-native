from Helpers.CApiStruct import CApiStruct
from SourceWriters.SourceWriter import SourceWriter


class CApiSourceWriter(SourceWriter):
    def __init__(self, schema_name, ecclasses, source_filename, api, status_codes, excluded_classes):
        super(CApiSourceWriter, self).__init__(ecclasses, source_filename, api, status_codes, excluded_classes)
        self.__schema_name = schema_name
        self.__api_structs = []
        for ecclass in self._ecclasses:
            if ecclass.attributes["typeName"].value in excluded_classes and \
                    excluded_classes[ecclass.attributes["typeName"].value].should_exclude_entire_class():
                continue
            self.__api_structs.append(CApiStruct(self.__schema_name, ecclass, api, status_codes,
                                                 excluded_classes[ecclass.attributes["typeName"].value]))

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
        self.__write_api_handle_free_function()
        self._write_spacing()
        self.__write_api_gws_functions()

    def __write_api_handle_free_function(self):
        self._file.write(self._COMMENT_BsiMethod)
        self._file.write('CallStatus {1}_FreeApi({0}HANDLE apiHandle)\n'.format(self._api.get_upper_api_acronym(),
                                                                                self._api.get_api_name()))
        self._file.write('    {\n')
        self._file.write('    if (nullptr == apiHandle)\n')
        self._file.write('        return INVALID_PARAMETER;\n\n')
        self._file.write('    LP{0} api = (LP{0}) apiHandle;\n'.format(self._api.get_upper_api_acronym()))
        self._file.write('    delete api;\n')
        self._file.write('    return SUCCESS;\n')
        self._file.write('    }\n')

    def __write_api_gws_functions(self):
        for api_struct in self.__api_structs:
            self.__write_api_gws_read_list_implementation(api_struct)
            if api_struct.get_name() in self._excluded_classes:
                if self._excluded_classes[api_struct.get_name()].should_have_create():
                    self.__write_api_gws_create_implementation(api_struct)
                if self._excluded_classes[api_struct.get_name()].should_have_read():
                    self.__write_api_gws_read_implementation(api_struct)
                if self._excluded_classes[api_struct.get_name()].should_have_update():
                    self.__write_api_gws_update_implementation(api_struct)
                if self._excluded_classes[api_struct.get_name()].should_have_delete():
                    self.__write_api_gws_delete_implementation(api_struct)
            else:
                self.__write_api_gws_create_implementation(api_struct)
                self.__write_api_gws_read_implementation(api_struct)
                self.__write_api_gws_update_implementation(api_struct)
                self.__write_api_gws_delete_implementation(api_struct)

    def __write_api_gws_read_list_implementation(self, api_struct):
        self._file.write(self._COMMENT_BsiMethod)
        self._file.write(api_struct.get_api_gws_read_list_implementation())
        self._write_spacing()

    def __write_api_gws_create_implementation(self, api_struct):
        self._file.write(self._COMMENT_BsiMethod)
        self._file.write(api_struct.get_api_gws_create_implementation())
        self._write_spacing()

    def __write_api_gws_read_implementation(self, api_struct):
        self._file.write(self._COMMENT_BsiMethod)
        self._file.write(api_struct.get_api_gws_read_implementation())
        self._write_spacing()

    def __write_api_gws_update_implementation(self, api_struct):
        self._file.write(self._COMMENT_BsiMethod)
        self._file.write(api_struct.get_api_gws_update_implementation())
        self._write_spacing()

    def __write_api_gws_delete_implementation(self, api_struct):
        self._file.write(self._COMMENT_BsiMethod)
        self._file.write(api_struct.get_api_gws_delete_implementation())
        self._write_spacing()



