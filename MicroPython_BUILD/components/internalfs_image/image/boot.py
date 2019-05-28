# This file is executed on every boot (including wake-boot from deepsleep)

import sys, os

# Set default path
# Needed for importing modules and upip
sys.path[1] = '/flash:/flash/lib'
