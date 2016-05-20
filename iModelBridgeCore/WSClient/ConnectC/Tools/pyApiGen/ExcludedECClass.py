class ExcludedECClass(object):
    def __init__(self, row):
        self.__name = row[0].value
        self.__include_create = row[1].value
        self.__include_read = row[2].value
        self.__include_update = row[3].value
        self.__include_delete = row[4].value
        self.__include_read_list = row[5].value
        self.__excluded_properties = [] if row[6].value is None else row[6].value.split(',')
        self.__excluded_properties = [ecproperty.strip() for ecproperty in self.__excluded_properties]

    def get_name(self):
        return self.__name

    def should_have_create(self):
        return self.__include_create

    def should_have_read(self):
        return self.__include_read

    def should_have_update(self):
        return self.__include_update

    def should_have_delete(self):
        return self.__include_delete

    def should_have_read_list(self):
        return self.__include_read_list

    def should_exclude_entire_class(self):
        return (not self.should_have_create()) and (not self.should_have_read()) and \
               (not self.should_have_update()) and (not self.should_have_delete() and
                                                    (not self.should_have_read_list()))

    def should_filter_property(self, property_to_filter):
        return property_to_filter in self.__excluded_properties
