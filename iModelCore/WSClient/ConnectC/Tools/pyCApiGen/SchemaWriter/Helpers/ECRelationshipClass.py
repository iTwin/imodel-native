#!Python
#--------------------------------------------------------------------------------------
#
#     $Source: ConnectC/Tools/pyCApiGen/SchemaWriter/Helpers/ECRelationshipClass.py $
#
#  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
from SchemaWriter.Helpers.ECProperty import ECProperty

#-------------------------------------------------------------------------------------------
# bsiclass                                                      Robert.Priest      04/2017
#-------------------------------------------------------------------------------------------
class ECRelObject (object):
    
    #-------------------------------------------------------------------------------------------
    # bsimethod                                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def __init__(self, name, type):
        self.name = name
        self.type = type

    #-------------------------------------------------------------------------------------------
    # bsimethod                                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def get_name(self):
        return self.name
        
    #-------------------------------------------------------------------------------------------
    # bsimethod                                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def get_type(self):
        return self.type
        
#-------------------------------------------------------------------------------------------
# bsiclass                                                      Robert.Priest      04/2017
#-------------------------------------------------------------------------------------------
class ECRelSource(object):
    
    #-------------------------------------------------------------------------------------------
    # bsimethod                                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def __init__(self, cardinality, polymorphic, relationship_object):
        self.cardinality = cardinality
        self.polymorphic = polymorphic
        self.relationship_object = relationship_object

    #-------------------------------------------------------------------------------------------
    # bsimethod                                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def get_name(self):
        return self.relationship_object.get_name()

#-------------------------------------------------------------------------------------------
# bsiclass                                                      Robert.Priest      04/2017
#-------------------------------------------------------------------------------------------
class ECRelTarget(object):
    
    #-------------------------------------------------------------------------------------------
    # bsimethod                                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def __init__(self, cardinality, polymorphic, relationship_object_list):
        self.cardinality = cardinality
        self.polymorphic = polymorphic
        self.relationship_object_list = []
        self.relationship_object_list = relationship_object_list;

    #-------------------------------------------------------------------------------------------
    # bsimethod                                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def add_rel_object(self, name, type):
        self.relationship_object_list.append(ECRelObject(name, type))

    #-------------------------------------------------------------------------------------------
    # bsimethod                                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def relates_to_object(self, objectName, objectType="Class"):
        matching = [ o for o in self.relationship_object_list if o.get_name().lower() == objectName.lower() ]
        if ( len(matching) > 0):
            return True
        return False

#-------------------------------------------------------------------------------------------
# bsiclass                                                      Robert.Priest      04/2017
#-------------------------------------------------------------------------------------------
class ECRelationshipClass(object):
    
    #-------------------------------------------------------------------------------------------
    # bsimethod                                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def __init__(self, api, status_codes, schema_name, name, is_domain_class, strength, strength_direction, source, target):
        self.name = name
        self.ecschema_name = schema_name
        self.include_create = False
        self.include_read = False
        self.include_update = False
        self.include_delete = False
        self.include_read_list = False
        self.is_domain_class = is_domain_class
        self.strength = strength
        self.strength_direction = strength_direction
        self.source = source;
        self.target = target;
        self.ecproperties = []
        #An ECRelationshipClass always has an id property for the source object
        self.ecproperties.append(ECProperty("{0}Id".format(source.get_name()), "string", False, False))
        self.__api = api
        self.__status_codes = status_codes
        self.var_name = None

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_schema_name(self):
        return self.ecschema_name

    #-------------------------------------------------------------------------------------------
    # bsimethod                                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def get_name(self):
        return self.name
    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_lower_name(self):
        return self.get_name().lower()

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_upper_name(self):
        return self.get_name().upper()

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def set_var_name(self, name):
        self.var_name = name

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_var_name(self):
        lowerCaseFirstLetter = lambda s: s[:1].lower() + s[1:] if s else ''
        if self.var_name != None:
            return lowerCaseFirstLetter(self.var_name)
        
        return lowerCaseFirstLetter(self.name)
         
    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def set_include(self, on_create, on_read, on_update, on_delete, on_read_all):
        self.include_create = on_create
        self.include_read = on_read
        self.include_update = on_update
        self.include_read_list = on_read_all

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def should_have_create(self):
        return self.include_create

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def should_have_read(self):
        return self.include_read

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def should_have_update(self):
        return self.include_update

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def should_have_delete(self):
        return self.include_delete

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def should_have_readlist(self):
        return self.include_read_list

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def should_exclude_entire_class(self):
        return (not self.should_have_create()) and (not self.should_have_read()) and \
               (not self.should_have_update()) and (not self.should_have_delete() and
                                                    (not self.should_have_readlist()))

    #-------------------------------------------------------------------------------------------
    # bsimethod                                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def get_source(self):
        return self.source
    
    #-------------------------------------------------------------------------------------------
    # bsimethod                                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def get_target(self):
        return self.target

    #-------------------------------------------------------------------------------------------
    # bsimethod                                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def get_properties(self):
        return self.ecproperties

    #-------------------------------------------------------------------------------------------
    # bsimethod                                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def add_property(self, property):
        self.ecproperties.append(property)

    #-------------------------------------------------------------------------------------------
    # bsimethod                                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def get_unique_property_types(self):
        unique_properties = []
        for ecproperty in self.get_properties():
            if ecproperty.type not in unique_properties:
                unique_properties.append(ecproperty.type)
        return unique_properties

    #-------------------------------------------------------------------------------------------
    # bsimethod                                                     Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def does_contain_property_type(self, property_type):
        if property_type == "StringLength":
            property_type = 'string'
        return property_type in self.get_unique_property_types()

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            			Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def does_contain_property_type(self, property_type):
        if property_type == "StringLength":
            property_type = 'string'
        return property_type in self._get_unique_property_types()

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            			Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def does_contain_string(self):
        return self.does_contain_property_type('string')

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            			Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def does_contain_guid(self):
        return self.does_contain_property_type('guid')

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            			Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def does_contain_boolean(self):
        return self.does_contain_property_type('boolean')

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            			Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def does_contain_int(self):
        return self.does_contain_property_type('int')

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            			Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def does_contain_double(self):
        return self.does_contain_property_type('double')

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            			Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def does_contain_long(self):
        return self.does_contain_property_type('long')

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            			Robert.Priest    04/2017
    #-------------------------------------------------------------------------------------------
    def does_contain_datetime(self):
        return self.does_contain_property_type('dateTime')

#-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def __get_class_properties_for_function_def(self):
        property_str = ""
        for ecproperty in self.get_properties():
            if ecproperty.should_be_excluded:
                continue
            if ecproperty.is_read_only:
                continue
            property_str += ',\n'
            property_type = ecproperty.type
            if property_type == "string":
                property_str += "WCharCP "
            elif property_type == "dateTime":
                property_str += "WCharCP "
            elif property_type == "guid":
                property_str += "WCharCP "
            elif property_type == "boolean":
                property_str += "bool* "
            elif property_type == "int":
                property_str += "int32_t* "
            elif property_type == "long":
                property_str += "int64_t* "
            else:
                property_str += ecproperty.type + "* "
            property_str += ecproperty.name
        return property_str

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_struct_typedef(self):
        struct_str = 'typedef struct _' + self.__api.get_api_acronym() + '_'
        struct_str += self.get_upper_name() + '_BUFFER \n'
        struct_str += '    {\n'
        struct_str += '    bmap<WString, bool> IsSet;\n'
        struct_str += '    WString ObjectId;\n'
        for ecproperty in self.get_properties():
            if ecproperty.should_be_excluded:
                continue
            if ecproperty.type == "string":
                struct_str += "    WString "
            elif ecproperty.type == "dateTime":
                struct_str += "    WString "
            elif ecproperty.type == "guid":
                struct_str += "    WString "
            elif ecproperty.type == "boolean":
                struct_str += "    bool "
            elif ecproperty.type == "int":
                struct_str += "    int32_t "
            elif ecproperty.type == "long":
                struct_str += "    int64_t "
            else:
                struct_str += "    " + ecproperty.type + " "
            struct_str += ecproperty.name + ";\n"
        struct_str += "    } " + self.__api.get_api_acronym() + self.get_upper_name()
        struct_str += "BUFFER, *LP" + self.__api.get_api_acronym()
        struct_str += self.get_upper_name() + "BUFFER;\n"
        return struct_str


    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_enum(self):
        enum_str = "typedef enum\n"
        enum_str += "    {\n"
        enum_count = 1
        #add the objectid
        enum_str += "    {0}_BUFF_{1:30} = {2}, /**< \\b {3}. */\n".format(self.get_upper_name(), "OBJECTID", enum_count, "ObjectId (generated property for instance id) ")
        enum_count += 1
        #add other properties
        for ecproperty in self.get_properties():
            if ecproperty.should_be_excluded:
                continue
            enum_str += "    {0}_BUFF_{1:30} = {2}, /**< \\b {3}. */\n"\
                .format(self.get_upper_name(), ecproperty.name.upper(), enum_count,
                        ecproperty.name)
            enum_count += 1
        enum_str += "    }} {0}_BUFF_PROPERTY;\n".format(self.get_upper_name())
        return enum_str

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def __get_buf_init_function_def(self):
        get_request_str = "CallStatus {0}_Init{1}Buffer\n".format(self.__api.get_api_name(), self.get_name())
        get_request_str += "(\n"
        get_request_str += "{0}DATABUFHANDLE* {1}Buffer".format(self.__api.get_upper_api_acronym(), self.get_lower_name())
        get_request_str += self.__get_class_properties_for_function_def()
        get_request_str += "\n)"
        return get_request_str

    #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_buf_init_definition(self):
        return "{0}_EXPORT ".format(self.__api.get_upper_api_acronym()) + self.__get_buf_init_function_def() + ";\n"

     #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_buf_init_implementation(self):
        get_request_str = self.__get_buf_init_function_def() + "\n"
        get_request_str += "    {\n"
        get_request_str += "    if ({0}Buffer == nullptr)\n".format(self.get_lower_name())
        get_request_str += "        {\n" 
        get_request_str += "        return {0};\n        }}\n\n" \
                    .format("INVALID_PARAMETER")
        get_request_str += "    LP{0}{1}BUFFER {2}Buf = new {0}{1}BUFFER;\n" \
            .format(self.__api.get_api_acronym(), self.get_upper_name(), self.get_lower_name())                    
        get_request_str += "    {0}BUFFER* buf = ({0}BUFFER*) calloc(1, sizeof({0}BUFFER));\n".format(self.__api.get_api_acronym())
        get_request_str += "    if (buf == nullptr)\n"
        get_request_str += "        {\n"
        get_request_str += "        free(buf);\n"         
        get_request_str += "        return {0};\n".format("INTERNAL_MEMORY_ERROR")
        get_request_str += "        }\n\n"      
        get_request_str += self.get_buffer_stuffer_function_implementation()  
        get_request_str += "    buf->lCount = 1;\n"
        get_request_str += "    buf->lClassType = BUFF_TYPE_{0};\n".format(self.get_upper_name())
        get_request_str += "    buf->lSchemaType = SCHEMA_TYPE_{0};\n".format(self.get_schema_name().upper())
        get_request_str += "    buf->isWSGBuffer = true;\n"
        get_request_str += "    buf->lItems = {{{0}Buf}};\n".format(self.get_lower_name())
        get_request_str += "    *{0}Buffer = ({1}DATABUFHANDLE) buf;\n\n".format(self.get_lower_name(), self.__api.get_upper_api_acronym())
        get_request_str += '    return {0};\n'.format("SUCCESS")
        get_request_str += "    }\n"
        return get_request_str

   #-------------------------------------------------------------------------------------------
    # bsimethod                                            						 06/2016
    #-------------------------------------------------------------------------------------------
    def get_buffer_stuffer_function_implementation(self):
        stuffer_str = ""
        for ecproperty in self.get_properties():
            if ecproperty.should_be_excluded:
                continue
            if ecproperty.type == "string":
                stuffer_str += '    {0}Buf->{1} = WString({1});\n' \
                    .format(self.get_lower_name(), ecproperty.name)
                stuffer_str += '    {0}Buf->IsSet[WString("{1}", true)] = true;\n' \
                    .format(self.get_lower_name(), ecproperty.name)
            elif ecproperty.type == "dateTime":
                stuffer_str += '    {0}Buf->{1} = WString("{1}", true);\n' \
                    .format(self.get_lower_name(), ecproperty.name)
                stuffer_str += '    {0}Buf->IsSet[WString("{1}", true)] = true;\n' \
                    .format(self.get_lower_name(), ecproperty.name)
            elif ecproperty.type == "guid":
                stuffer_str += '    {0}Buf->{1} = WString("{1}", true);\n' \
                    .format(self.get_lower_name(), ecproperty.name)
                stuffer_str += '    {0}Buf->IsSet[WString("{1}", true)] = true;\n' \
                    .format(self.get_lower_name(), ecproperty.name)
            elif ecproperty.type == "boolean":
                stuffer_str += '    {0}Buf->{1} = {1};\n' \
                    .format(self.get_lower_name(), ecproperty.name)
                stuffer_str += '    {0}Buf->IsSet[WString("{1}", true)] = true;\n' \
                    .format(self.get_lower_name(), ecproperty.name)
            elif ecproperty.type == "int" or ecproperty.type == "long":
                stuffer_str += '    {0}Buf->{1} = {1};\n' \
                    .format(self.get_lower_name(), ecproperty.name)
                stuffer_str += '    {0}Buf->IsSet[WString("{1}", true)] = true;\n' \
                    .format(self.get_lower_name(), ecproperty.name)
            elif ecproperty.type == "double":
                stuffer_str += '    {0}Buf->{1} = {1};\n' \
                    .format(self.get_lower_name(), ecproperty.name)
                stuffer_str += '    {0}Buf->IsSet[WString("{1}", true)] = true;\n' \
                    .format(self.get_lower_name(), ecproperty.name)
            else:
                raise PropertyTypeError("Property type {0} not accepted".format(ecproperty.type))
        stuffer_str += "\n"
        return stuffer_str


