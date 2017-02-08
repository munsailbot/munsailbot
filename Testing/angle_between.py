"""Test case for "angle between" function."""

# !/usr/bin/env python

import math
import sys
import random


def angle_between(a1, a2):
    """Compute the theta between two angles."""
    theta = a1 - a2

    if(math.fabs(theta) > 180):
        if(theta < 0):
            theta = theta + 360
        else:
            theta = 360 - theta
    else:
        theta = math.fabs(theta)

    return theta


if __name__ == '__main__':

    for i in range(0, 5):
        angle1 = random.randint(0, 360)
        angle2 = random.randint(0, 360)

        sys.stdout.write("Input: %i %i\n" % (angle1, angle2))

        result = angle_between(angle1, angle2)

        sys.stdout.write("Output: %i\n" % result)
