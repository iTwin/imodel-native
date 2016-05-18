from Helpers.CppCliStruct import CppCliStruct
from SourceWriters.SourceWriter import SourceWriter


class CppCliClassesSourceWriter(SourceWriter):

    __CLASS_MemberVariable = "        public {0} {1} {{ get; set; }}\n"

    def __init__(self, ecclasses, source_filename, api, status_codes, excluded_classes):
        super(CppCliClassesSourceWriter, self).__init__(ecclasses, source_filename, api, status_codes, excluded_classes)
        self.__cpp_cli_structs = []
        for ecclass in self._ecclasses:
            if ecclass.attributes["typeName"].value in excluded_classes and \
                    excluded_classes[ecclass.attributes["typeName"].value].should_exclude_entire_class():
                continue
            self.__cpp_cli_structs.append(CppCliStruct(ecclass, api, status_codes,
                                                       excluded_classes[ecclass.attributes["typeName"].value]))

    def write_source(self):
        self._write_header_comments()
        self._write_spacing()
        self.__write_includes()
        self._write_spacing()
        self._write_spacing()
        self._file.write("namespace {0}Sharp\n".format(self._api.get_api_name()))
        self._file.write("    {\n")
        self.__write_status_class_implementation()
        for cpp_cli_struct in self.__cpp_cli_structs:
            self._write_spacing()
            self._file.write(self._COMMENT_BsiMethod)
            self._file.write(cpp_cli_struct.get_constructor_implementation())
        self._file.write("    }")
        self._close_file()

    def __write_includes(self):
        self._file.write('#include "stdafx.h"')

    def __write_status_class_implementation(self):
        self._file.write(self._COMMENT_BsiMethod)
        self._file.write('    CallStatus::CallStatus(uint16_t id, CharCP message, CharCP description)\n')
        self._file.write('        {\n')
        self._file.write('        Id = id;\n')
        self._file.write('        Message = gcnew String(message);\n')
        self._file.write('        Description = gcnew String(description);\n')
        self._file.write('        }\n')







