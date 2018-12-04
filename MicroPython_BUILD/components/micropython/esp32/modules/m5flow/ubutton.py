from utime import ticks_ms

# EVENT_IS_PRESSED = const(0x01)
# EVENT_WAS_PRESSED = const(0x02)
# EVENT_WAS_RELEASED = const(0x04)
# EVENT_RELEASED_FOR = const(0x08)

class M5_Button:

  def __init__(self, pin, name='none', long_name='none', dbtime=20):
    from machine import Pin
    self._pin = Pin(pin)
    self._pin.init(Pin.IN, handler=self.irq_cb, trigger=(Pin.IRQ_FALLING|Pin.IRQ_RISING))
    self._wasPressed_cb = None
    self._wasReleased_cb = None
    self._releasedFor_cb = None
    self._timeshoot = 0
    self._dbtime = dbtime
    self._lastState = False
    self._startTicks = 0
    self._timeout = 0
    self._name = name
    self._long_name = long_name
    self._event = 0


  def irq_cb(self, pin):
    pin_val = pin.value()
    if self._pin == pin:
      # FALLING
      if pin_val == 0:  
        if ticks_ms() - self._timeshoot > self._dbtime:
          self._lastState = True
          self._startTicks = ticks_ms()
          self._event |= 0x02  # EVENT_WAS_PRESSED
          if self._wasPressed_cb:
            self._wasPressed_cb()
      # RISING
      elif pin_val == 1:
        if self._lastState == True:
          self._lastState = False
          self._event |= 0x04  # EVENT_WAS_RELEASED
          if self._timeout > 0 and ticks_ms() - self._startTicks > self._timeout:
            self._event = 0
            self._event |= 0x08  # EVENT_RELEASED_FOR
            if self._releasedFor_cb:
              self._releasedFor_cb()
          elif self._wasReleased_cb:
            self._wasReleased_cb()
      self._timeshoot = ticks_ms()

  def clear(self):
    self._event = 0
    
  def read(self):
    return not self._pin.value()


  def isPressed(self):
    return self.read()


  def isReleased(self):
    return not self.read()


  def wasPressed(self, callback=None):
    if callback == None:
      if (self._event & 0x02) > 0: # EVENT_WAS_PRESSED
        self._event -= 0x02
        return True
      else:
        return False
    else:
      self._wasPressed_cb = callback


  def wasReleased(self, callback=None):
    if callback == None:
      if (self._event & 0x04 ) > 0: # EVENT_WAS_RELEASED
        self._event -= 0x04
        return True
      else:
        return False
    else:
      self._wasReleased_cb = callback


  def pressedFor(self, timeout):
    if self._lastState and ticks_ms() - self._startTicks > timeout * 1000:
      return True
    else:
      return False


  def releasedFor(self, timeout, callback=None):
    self._timeout = timeout * 1000 # second
    if callback == None:
      if (self._event & 0x08) > 0: # EVENT_RELEASED_FOR
        self._event -= 0x08
        return True
      else:
        return False
    else:
      self._releasedFor_cb = callback

class Button:
  def __init__(self):
    self.button = []

  def register(self, button):
    _button = M5_Button(button)
    self.button.append(_button)
    return _button

  def clear(self):
    for i in self.button:
      i._event = 0
      i._wasPressed_cb = None
      i._wasReleased_cb = None
      i._releasedFor_cb = None