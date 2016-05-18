from Helpers.CppCliStruct import CppCliStruct
from HeaderWriters.HeaderWriter import HeaderWriter


class CppCliClassesHeaderWriter(HeaderWriter):
    def __init__(self, ecclasses, header_filename, api, status_codes, excluded_classes):
        super(CppCliClassesHeaderWriter, self).__init__(ecclasses, header_filename, api, status_codes, excluded_classes)
        self.__cpp_cli_structs = []
        for ecclass in self._ecclasses:
            if ecclass.attributes["typeName"].value in excluded_classes and \
                    excluded_classes[ecclass.attributes["typeName"].value].should_exclude_entire_class():
                continue
            self.__cpp_cli_structs.append(CppCliStruct(ecclass, api, self._status_codes,
                                                       excluded_classes[ecclass.attributes["typeName"].value]))

    def write_header(self):
        self._write_header_comments(True)
        self._write_spacing()
        self.__write_includes()
        self._write_spacing()
        self._write_spacing()
        self.__write_using_directives()
        self._write_spacing()
        self._file.write('namespace {0}Sharp\n'.format(self._api.get_api_name()))
        self._file.write('    {\n')
        self.__write_status_class_definition()
        self.__write_classes()
        self._file.write('    }\n')
        self._close_file()

    def __write_includes(self):
        self._file.write('#include <WebServices/ConnectC/{0}Public.h>'.format(self._api.get_api_acronym()))

    def __write_using_directives(self):
        self._file.write('using namespace System;\n')

    def __write_status_class_definition(self):
        self._file.write('    public ref class CallStatus\n')
        self._file.write('        {\n')
        self._file.write('        public:\n')
        self._file.write('            CallStatus(uint16_t id, CharCP message, CharCP description);\n')
        self._file.write('            int Id;\n')
        self._file.write('            String^ Message;\n')
        self._file.write('            String^ Description;\n')
        self._write_spacing()
        sorted_codes = sorted(self._status_codes.iteritems(), key=lambda x: x[1].code, reverse=True)
        for key, value in sorted_codes:
            self._file.write('            static const int _{0} = {1};\n'.format(key, value.code))
        self._file.write('        };\n')

    def __write_classes(self):
        for cpp_cli_struct in self.__cpp_cli_structs:
            self._write_spacing()
            self._file.write('    public ref class {0}\n'.format(cpp_cli_struct.get_name()))
            self._file.write('        {\n')
            self._file.write(cpp_cli_struct.get_property_declarations())
            self._file.write('        };\n')


