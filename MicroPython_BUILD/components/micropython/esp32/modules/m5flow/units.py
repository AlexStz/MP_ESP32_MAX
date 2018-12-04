from micropython import const
import machine
from machine import I2C, Pin, PWM
import math
import i2c_bus
import utime as time
from m5stack import speaker
import ustruct

# PORTA = (25,13)
PORTA = (21,22)
PORTB = (26,36)
PORTC = (17,16)

class_map = {'dht12': None, 'bmp280': None, 'adc': None, 'servo': {}}

class ENV():
    def __init__(self, port=PORTA):
        global class_map
        from dht12 import DHT12
        from bmp280 import BMP280
        self.i2c = i2c_bus.get(i2c_bus.M_BUS)
        if class_map['dht12'] == None:
            class_map['dht12'] = DHT12(self.i2c)
        if class_map['bmp280'] == None:
            class_map['bmp280'] = BMP280(self.i2c)
        self.dht12 = class_map['dht12']
        self.bmp280 = class_map['bmp280']
        self.time = 0
        self.data = None

    def available(self):
        if 92 in self.i2c.scan():
            return True
        else:
            return False

    def pressure(self):
        if time.ticks_ms() - self.time > 100:
            self.time = time.ticks_ms()
            self.data = self.values
        return round(self.data[1], 2)

    def temperature(self):
        if time.ticks_ms() - self.time > 100:
            self.time = time.ticks_ms()
            self.data = self.values
        return round(self.data[0], 2)

    def humidity(self):
        if time.ticks_ms() - self.time > 100:
            self.time = time.ticks_ms()
            self.data = self.values
        return round(self.data[2], 2)

    @property
    def values(self):
        """ readable values """
        self.dht12.measure()
        h = self.dht12.humidity()
        t, p = self.bmp280.read_compensated_data()
        t /= 100
        p /= 25600
        return t, p, h

class PIR(): # 1 OUT
    def __init__(self, port=PORTB):
        self.pin = Pin(PORTB[1], Pin.IN)
        self.cb = None

    def callback(self, cb):
        self.cb = cb
        self.pin.init(handler=self._irq_cb, trigger=(Pin.IRQ_RISING | Pin.IRQ_FALLING))

    def _irq_cb(self, pin):
        self.cb(pin.value())

    def read(self):
        return self.pin.value()

class RGB(): # 0 IN 1 OUT
    def __init__(self, port=PORTB, nums=1):
        self.nums = nums*3
        self.np	= machine.Neopixel(Pin(port[0]), self.nums)

    def setColor(self, color, pos=None):
        if pos == None:
            self.np.set(1, color, num=self.nums)
        else:
            self.np.set(pos, color)

    def setHSB(self, hue, saturation, brightness, pos=None):
        if pos == None:
            self.np.setHSB(1, hue, saturation, brightness, num=self.nums)
        else:
            self.np.setHSB(pos, hue, saturation, brightness)

    def deinit(self):
        self.np.deinit()

class ANGLE(): # 1
    def __init__(self, port=PORTB):
        global class_map
        from machine import ADC
        if class_map['adc'] != None:
            class_map['adc'].deinit()
        self.adc = ADC(PORTB[1])
        self.adc.atten(ADC.ATTN_11DB)
        class_map['adc'] = self.adc

    def deinit(self):
        self.adc.deinit()

    def readraw(self):
        return 4095 - self.adc.readraw()

    def read(self):
        data = 0
        max = 0
        min = 4096
        for i in range(0, 10):
            newdata = 4095 - self.adc.readraw()
            data += newdata
            if newdata > max:
                max = newdata
            if newdata < min:
                min = newdata
        data -= (max + min)
        data >>= 3
        return round(1024 * data / 4095, 2)

class Servo:
    """
    A simple class for controlling hobby servos.

    Args:
        pin (machine.Pin): The pin where servo is connected. Must support PWM.
        freq (int): The frequency of the signal, in hertz.
        min_us (int): The minimum signal length supported by the servo.
        max_us (int): The maximum signal length supported by the servo.
        angle (int): The angle between the minimum and maximum positions.

    """
    def __init__(self, port=PORTA, freq=50, min_us=600, max_us=2400, angle=180):
        self.min_us = min_us
        self.max_us = max_us
        self.us = 0
        self.freq = freq
        self.angle = angle
        self.port = port
        if port[0] in class_map['servo'].keys():
            class_map['servo'][port[0]].deinit()
    
        self.pwm = PWM(port[0], freq=freq, duty=0, timer=3)
        class_map['servo'][port[0]] = self.pwm


    def write_us(self, us):
        """Set the signal to be ``us`` microseconds long. Zero disables it."""
        if us == 0:
            self.pwm.duty(0)
            return
        us = min(self.max_us, max(self.min_us, us))
        duty = us * 100 * self.freq / 1000000
        self.pwm.duty(duty)

    def write_angle(self, degrees=None, radians=None):
        """Move to the specified angle in ``degrees`` or ``radians``."""
        if degrees is None:
            degrees = math.degrees(radians)
        degrees = degrees % 360
        total_range = self.max_us - self.min_us
        us = self.min_us + total_range * degrees / self.angle
        self.write_us(us)


rgb_muti_porta = None
rgb_muti_portb = None
rgb_muti_portc = None

class RGB_Multi:
    # TODO if not sleep, will appear some bug
    def __init__(self, port=PORTB, number=143):
        global rgb_muti_porta, rgb_muti_portb, rgb_muti_portc
        if port == PORTB and rgb_muti_portb != None:
            rgb_muti_portb.deinit()
        elif port == PORTA and rgb_muti_porta != None:
            rgb_muti_porta.deinit()
        elif port == PORTC and rgb_muti_portc != None:
            rgb_muti_portc.deinit()
        self.port = port
        self.pin = port[0]
        self.number = number
        self.np = machine.Neopixel(self.pin, self.number)
        self.np.brightness(10, update=False)
        self.np.show()
        time.sleep_ms(10)

        if port == PORTA:
            rgb_muti_porta = self.np
        elif port == PORTB:
            rgb_muti_portb = self.np
        elif port == PORTC:
            rgb_muti_portc = self.np
    
    def setColor(self, num, color):
        self.np.set(num, color)
        time.sleep_ms(10)

    def setColorFrom(self, begin, end, color):
        begin = min(self.number, max(begin, 1))
        end = min(self.number, max(end, 1))
        for i in range(begin, end+1):
            self.np.set(i, color, update=False)
        self.np.show()
        time.sleep_ms(10)

    def setColorAll(self, color):
        for i in range(1, self.number+1):
            self.np.set(i, color, update=False)
        self.np.show()
        time.sleep_ms(10)

    def setBrightness(self, brightness):
        brightness = min(255, max(0, brightness))
        self.np.brightness(brightness, update=False)
        self.np.show()
        time.sleep_ms(10)
    
    def deinit(self):
        global rgb_muti_porta, rgb_muti_portb, rgb_muti_portc
        self.np.deinit()
        if self.port == PORTA:
            rgb_muti_porta = None
        elif self.port == PORTB:
            rgb_muti_portb = None
        elif self.port == PORTC:
            rgb_muti_portc = None

class Joystick():
    def __init__(self, port=PORTC):
        self.i2c = i2c_bus.get(port)
        self.time = 0
        self.value = None

    @property    
    def X(self):
        if time.ticks_ms() - self.time > 100:
            self.time = time.ticks_ms()
            try:
                self.value = self.i2c.readfrom(0x52, 3)
            except:
                pass
        return self.value[0]

    @property    
    def Y(self):
        if time.ticks_ms() - self.time > 100:
            self.time = time.ticks_ms()
            try:
                self.value = self.i2c.readfrom(0x52, 3)
            except:
                pass
        return self.value[1]

    @property        
    def Press(self):
        if time.ticks_ms() - self.time > 100:
            self.time = time.ticks_ms()
            try:
                self.value = self.i2c.readfrom(0x52, 3)
            except:
                pass

        return self.value[2] if self.value[2] < 2 else 0

class Light:
    def __init__(self, port=PORTB):
        global class_map
        from machine import ADC, Pin
        if class_map['adc'] != None:
            class_map['adc'].deinit()
        self.adc = ADC(PORTB[1])
        self.adc.atten(ADC.ATTN_11DB)
        class_map['adc'] = self.adc
        self.d_pin = Pin(PORTB[0], Pin.IN, Pin.PULL_UP)

    def a_read(self):
        data = 0
        max = 0
        min = 4096
        for i in range(0, 10):
            newdata = 4095 - self.adc.readraw()
            data += newdata
            if newdata > max:
                max = newdata
            if newdata < min:
                min = newdata
        data -= (max + min)
        data >>= 3
        return round(1024 * data / 4095, 2)
    
    def d_read(self):
        return self.d_pin.value()

Earth = Light

MAKEY_I2C_ADDR = const(0x51)
class Makey:
    def __init__(self, port=PORTA):
        self._i2c = i2c_bus.get(port)
        self.sing_map = [261, 293, 329, 349, 392, 440, 494, 294]

    def _get_value(self):
        value = 0
        data = self._i2c.readfrom(MAKEY_I2C_ADDR, 2)
        value = data[0]|(data[1] << 8)
        return value

    def write(self, data):
        self._i2c.write(MAKEY_I2C_ADDR, data)
    
    def value(self):
        value = self._get_value()
        for i in range(14):
            if (value >> i) & 0x01:
                return i
        return -1


    def play_piano(self, beat):
        key_value = self._get_value()
        time.sleep_ms(1)
        for i in range(8):
            if (key_value >> i) & 0x01:
                speaker.sing(self.sing_map[i], beat)
                break

class IR():  # 1
    def __init__(self, port=PORTB):
        pass

class Weigh:
    def __init__(self, port=PORTA):
        self.pinclk = Pin(port[0], Pin.OUT)
        self.pindata = Pin(port[1], Pin.IN)
        self.zero_value = 0
        self.gap_value = 250
        self.pinclk.value(0)

    def zero(self):
        self.zero_value = self._weight

    @property        
    def _weight(self):
        pinclk = self.pinclk
        pindata = self.pindata
        val, times, count = 1, 0, 0
        pinclk.value(0)


        while val:
            val = pindata.value()
            if val:
                times += 1
                time.sleep_ms(10)
                if times > 20: 
                    return 0
        
        for i in range(0, 24):
            pinclk.value(1)
            count = count << 1;
            pinclk.value(0)
            
            val = pindata.value()
            if val == 1:
                count = count + 1

        pinclk.value(1)
        a = 1
        pinclk.value(0)
        count = count ^ 0x800000
        return int(count / self.gap_value)

    def get_weight(self):
        countdata = self._weight - self.zero_value
        return countdata

class Tracker:
    def __init__(self, port=i2c_bus.PORTA):
        self._addr = 0x5a
        self._i2c = i2c_bus.get(port)

    def _register_char(self, register, value=None, buf=bytearray(1)):
        if value is None:
            self._i2c.readfrom_mem_into(self._addr, register, buf)
            return buf[0]

        ustruct.pack_into("<b", buf, 0, value)
        return self._i2c.writeto_mem(self._addr, register, buf)

    def _register_short(self, register, value=None, buf=bytearray(2)):
        if value is None:
            self._i2c.readfrom_mem_into(self._addr, register, buf)
            return ustruct.unpack(">h", buf)[0]

        ustruct.pack_into(">h", buf, 0, value)
        return self._i2c.writeto_mem(self._addr, register, buf)

    def get_analog_value(self, number):
        if number > 4 or number < 0:
            return 0
        
        return self._register_short(0x00 | number)

    def set_analog_value(self, number, value):
        if number > 4 or number < 0:
            return 0
        
        self._register_short(0x10 | number, value)

    def get_digital_value(self, number):
        if number > 4 or number < 0:
            return 0

        return self._register_char(0x00) & (1 << (number - 1))