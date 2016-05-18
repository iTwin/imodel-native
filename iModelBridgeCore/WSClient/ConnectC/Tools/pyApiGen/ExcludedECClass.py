class ExcludedECClass(object):
    def __init__(self, excel_row):
        self.__name = excel_row[0].value
        self.__include_create = excel_row[1].value
        self.__include_read = excel_row[2].value
        self.__include_update = excel_row[3].value
        self.__include_delete = excel_row[4].value

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

    def should_exclude_entire_class(self):
        return (not self.should_have_create()) and (not self.should_have_read()) and \
               (not self.should_have_update()) and (not self.should_have_delete())
