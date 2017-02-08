# !/usr/bin/env python

"""
Test case for "cardinal to standard" function

Takes an angle where 0 degrees represents north,
and converts it to the corresponding angle on the unit circle.
"""

import math
import sys
import random


def cardinal_to_standard(angle):
    standard_angle = math.fabs(angle - 450) % 360

    return standard_angle


if __name__ == '__main__':

    for i in range(0, 5):
        angle = random.randint(0, 360)

        sys.stdout.write("Input: %i\n" % angle)

        result = cardinal_to_standard(angle)

        sys.stdout.write("Output: %i\n" % result)
