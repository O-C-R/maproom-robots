from datetime import datetime

MSG_DELAY = 2.0

class FPS:
  def __init__(self):
    self.fps = 30.0
    self.lastTime = datetime.now()
    self.lastMessage = datetime.now()
    self.frameidx = 0

  def update(self):
    now = datetime.now()
    delta = now - self.lastTime
    self.lastTime = now
    currFps = 1.0 / delta.total_seconds()
    self.fps = self.fps + (currFps - self.fps) * 0.2
    self.frameidx = self.frameidx + 1

    shouldPrint = False
    messageDelta = now - self.lastMessage
    if messageDelta.total_seconds() > MSG_DELAY:
        shouldPrint = True
        self.lastMessage = now

    return shouldPrint, self.fps, self.frameidx
