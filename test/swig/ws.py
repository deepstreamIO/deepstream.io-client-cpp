import deepstream

class PyWS(deepstream.WSHandler):

    def __init__(self):
        print('init')
        self.uri = None

    def URI(self, uri = None):
        if uri == None:
            return self.uri

        print('set uri ', uri)
        self.uri = uri

    def open(self):
        print('open')

    def close(self):
        print('close')

    def reconnect(self):
        print('reconnect')

    def shutdown(self):
        print('shutdown')

    def send(self, buff):
        print('send:', buff)

class ErrH(deepstream.ErrorHandler):
    def __init__(self):
        super()

    def on_error(self, err):
        print('error:', err)

wsh = PyWS()
errh = deepstream.ErrorHandler()
# client = deepstream.Client('', wsh, errh)
