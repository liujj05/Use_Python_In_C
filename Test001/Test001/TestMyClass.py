class ConfigFile(object):
    def __init__(self, filename):
        self.filename = filename
    def say_ur_name(self):
        print self.filename
