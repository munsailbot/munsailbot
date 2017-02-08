"""Test case for tack state."""

# !/usr/bin/env python

import math
import sys
import time
import course_by_wind


if __name__ == '__main__':

    windRel = 45
    initialWind = windRel
    desiredWindRel = -windRel
    state = 0  # tack initially
    rudder = 0  # rudder at center position initially
    aggression = 2.0  # controls the peak rudder position

    x = -10  # controls peak position
    while state == 0:
        if initialWind > 0:
            sig = math.sqrt(0.05)
            mu = 0.0

            gauss = math.floor(
                (1.0 / sig) * math.exp(-((x - mu)**2.0)
                                       / 2.0 * (sig**2.0)) * 2.0)

            rudder = gauss * aggression
            windRel -= 4  # assume our movement through the wind is constant
            if rudder > 30:
                rudder = 30

            sys.stdout.write("Rudder: " + str(rudder) + "\n")
            sys.stdout.write("Wind (Relative):" + str(windRel) + "\n")
            time.sleep(0.2)

            if desiredWindRel < initialWind:
                if windRel <= desiredWindRel:
                    state = 1  # stop tack
            elif desiredWindRel > initialWind:
                if windRel >= desiredWindRel:
                    state = 1  # stop tack

            x = x + 1
        elif initialWind < 0:
            sig = math.sqrt(0.05)
            mu = 0.0

            gauss = - \
                math.floor((1.0 / sig) *
                           math.exp(-((x - mu)**2.0) / 2.0 * (sig**2.0)) * 2.0)

            rudder = gauss * aggression
            windRel += 4
            # assume our rudder movements correspond to changes in wind angle
            if rudder < -30:
                rudder = -30

            sys.stdout.write("Rudder: " + str(rudder) + "\n")
            sys.stdout.write("Wind (Relative):" + str(windRel) + "\n")
            time.sleep(0.2)

            if desiredWindRel < initialWind:
                if windRel <= desiredWindRel:
                    state = 1  # stop tack
            elif desiredWindRel > initialWind:
                if windRel >= desiredWindRel:
                    state = 1  # stop tack

            x = x + 1
    else:
        sys.stdout.write("\n finished tack \n")
        course = course_by_wind.course_by_wind(windRel, 70)
        sys.stdout.write("Output (sail,rudder): %i %i %i\n\n" % course)
