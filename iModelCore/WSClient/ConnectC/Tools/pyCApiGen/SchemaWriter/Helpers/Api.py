class Api(object):
    def __init__(self, acronym, name):
        self.__acronym = acronym
        self.__name = name

    def get_api_acronym(self):
        return self.__acronym

    def get_api_name(self):
        return self.__name

    def get_upper_api_name(self):
        return self.get_api_name().upper()

    def get_upper_api_acronym(self):
        return self.get_api_acronym().upper()
