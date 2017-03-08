import threading

from pythonosc.osc_server import ThreadingOSCUDPServer
from pythonosc.dispatcher import Dispatcher

class OSCReceiver:
  def __init__(self, host='0.0.0.0', port=5300):
    self.dispatcher = Dispatcher()
    self.dispatcher.map("/state", self.setState)
    self.oscServer = ThreadingOSCUDPServer((host, port), self.dispatcher)
    self.oscServerThread = threading.Thread(target=self.oscServer.serve_forever, daemon=True)

    self.state = -1

  def getState(self):
    return self.state

  def setState(self, endpoint, newState):
    print('GOT STATE MESSAGE')
    print(newState)

    self.state = newState

  def start(self):
    self.oscServerThread.start()
    return self
