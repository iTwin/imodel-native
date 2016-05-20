class CStruct(object):
    def __init__(self, ecclass_dom, api, status_codes, excluded_ecclass):
        self._ecclass = ecclass_dom
        self._api = api
        self._status_codes = status_codes
        self._excluded_ecclass = excluded_ecclass

    def _get_unique_property_types(self):
        unique_properties = []
        for ecproperty in self.get_properties():
            if self._excluded_ecclass.should_filter_property(ecproperty.attributes["propertyName"].value):
                continue
            if ecproperty.attributes["typeName"].value not in unique_properties:
                unique_properties.append(ecproperty.attributes["typeName"].value)
        return unique_properties

    def get_properties(self):
        return self._ecclass.getElementsByTagName('ECProperty')

    def does_contain_property_type(self, property_type):
        if property_type == "StringLength":
            property_type = 'string'
        return property_type in self._get_unique_property_types()

    def does_contain_string(self):
        return self.does_contain_property_type('string')

    def does_contain_guid(self):
        return self.does_contain_property_type('guid')

    def does_contain_boolean(self):
        return self.does_contain_property_type('boolean')

    def does_contain_int(self):
        return self.does_contain_property_type('int')

    def does_contain_double(self):
        return self.does_contain_property_type('double')

    def does_contain_long(self):
        return self.does_contain_property_type('long')

    def get_upper_name(self):
        return self.get_name().upper()

    def get_lower_name(self):
        return self.get_name().lower()

    def get_name(self):
        return self._ecclass.attributes["typeName"].value
