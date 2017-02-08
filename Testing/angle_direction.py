# !/usr/bin/env python

import math
import sys
import random


def angle_quadrant(angle):
    """Determine which quadrant the angle is located in."""
    a = angle % 360

    if((a >= 0) and (a < 90)):
        return 1
    if((a >= 90) and (a < 180)):
        return 2
    if((a >= 180) and (a < 270)):
        return 3
    if((a >= 270) and (a < 360)):
        return 4


def angle_direction(a1, a2):
    """The direction of the angle."""
    side = 0

    if((angle_quadrant(a1) == 1 or angle_quadrant(a1) == 4)
       and (angle_quadrant(a2) == 1 or angle_quadrant(a2) == 4)):
        if(angle_quadrant(a1) == 1 and angle_quadrant(a2) == 4):
            side = 1
        if(angle_quadrant(a2) == 1 and angle_quadrant(a1) == 4):
            side = -1

    if(angle_quadrant(a1) != angle_quadrant(a2)):
        if((360 - a1) < (360 - a2)):
            side = -1
        else:
            side = 1
    else:
        if(a2 > a1):
            side = -1
        else:
            side = 1

    return side


if __name__ == '__main__':

    for i in range(0, 10):
        heading = random.randint(0, 360)
        course = random.randint(0, 360)

        sys.stdout.write("Input (heading,course): %i %i\n" % (heading, course))

        result = angle_direction(heading, course)

        sys.stdout.write("Side: %i\n" % result)
