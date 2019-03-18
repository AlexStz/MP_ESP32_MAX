# This file is executed on every boot (including wake-boot from deepsleep)
import sys
sys.path[1] = '/flash:/flash/lib'

import time
import network
import machine
import gc
import nvs
import m5stack
import ota

if m5stack.buttonA.isPressed():
    if ota.set_bootpart('app0'):
        machine.reset()

wifi_cancelled = m5stack.buttonC.isPressed() # type: bool

nvs_wifi = nvs.NVS(namespace='wifi-config')
wifi_ssid     = nvs_wifi.get_str('WIFI_SSID')   # type: str
wifi_password = nvs_wifi.get_str('WIFI_PASSWD') # type: str
has_wifi_config = wifi_ssid is not None and wifi_password is not None

if has_wifi_config and not wifi_cancelled:
    s = 'Connecting to Wi-Fi AP {0}... Press C to cancel'.format(wifi_ssid)
    m5stack.lcd.print(s)
    print(s)
    w = network.WLAN()
    w.active(True)
    w.connect(wifi_ssid, wifi_password)
    while not w.isconnected() and not wifi_cancelled:
        wifi_cancelled = m5stack.buttonC.isPressed()
        time.sleep(1)

if not wifi_cancelled:
    ifconfig = w.ifconfig()
    m5stack.lcd.print(ifconfig)
    print(ifconfig)
    # Start FTP server to upload source codes.
    network.ftp.start(user='esp32', password='esp32')

gc.collect()
