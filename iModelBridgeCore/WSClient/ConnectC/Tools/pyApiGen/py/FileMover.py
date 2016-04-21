import os.path
from shutil import copy2


class FileLink(object):
    def __init__(self, dest, src):
        if not os.path.isfile(src):
            raise ValueError("src file is not a file")
        self.dest = dest
        self.src = src

    def copyfile(self):
        try:
            copy2(self.src, self.dest)
        except IOError as e:
            print e


class FileMover:
    def __init__(self, filelinks):
        self.filelinks = filelinks

    def copyfiles(self):
        for filelink in self.filelinks:
            filelink.copyfile()
