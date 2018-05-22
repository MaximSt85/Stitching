import PyTango
import time
#import numpy as np
from scipy import interpolate

devx = PyTango.DeviceProxy('p11/motor/galil.02')
devy = PyTango.DeviceProxy('p11/motor/galil.05')
devz = PyTango.DeviceProxy('p11/motor/galil.06')

xp = [500, 33000, 33000, 500]
yp = [-300, -300, -10000, -10000]
zp = [0, 120, -430, -560]

f = interpolate.interp2d(xp, yp, zp, kind = 'linear')

while True:
    x = devx.Position
    y = devy.Position
    z = f(x, y)
    devz.Position = z[0]
    #print z[0]
    time.sleep(0.1)
    





