from datetime import datetime

class FPS:
  def __init__(self):
    self.fps = 30.0
    self.lastTime = datetime.now()
    self.frameidx = 0

  def update(self):
    now = datetime.now()
    delta = now - self.lastTime
    self.lastTime = now
    currFps = 1.0 / delta.total_seconds()
    self.fps = self.fps + (currFps - self.fps) * 0.1
    self.frameidx = self.frameidx + 1
    return self.frameidx % 30 == 0, self.fps, self.frameidx
