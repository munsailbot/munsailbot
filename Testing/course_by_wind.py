# !/usr/bin/env python

"""
Test case for "course by wind" function.
"""

import math
import sys
import random


def course_by_wind(windRel, angle):
    """Plan course by angle of wind relative to boat."""
    rudder = 99
    main = 99
    jib = 99

    side = 1
    if(angle < 0):
        side = -side

    theta = math.fabs(angle) - math.fabs(windRel)
    rudder = math.floor((35.0 / 180.0) * theta)
    rudder = rudder * side

    if(math.fabs(windRel) < 45):
        main = 0
        jib = 0
    else:
        sailPos = (math.fabs(windRel) - 45) * 0.66
        main = math.floor(sailPos)
        jib = math.floor(sailPos)

    return (main, jib, rudder)


def postcondition(result):
    """The condition following."""
    assert result[0] >= 0 and result[0] <= 90
    assert result[1] >= 0 and result[1] <= 90
    assert result[2] >= -35 and result[2] <= 35


if __name__ == '__main__':
    sys.stdout.write("Testing positive wind & angle values\n\n")

    for i in range(0, 10):
        wind = random.randint(0, 180)
        angle = random.randint(0, 180)

        sys.stdout.write("Input (wind,angle): %i %i\n" % (wind, angle))

        result = course_by_wind(wind, angle)
        postcondition(result)

        sys.stdout.write("Output (sail,rudder): %i %i %i\n\n" % result)

    sys.stdout.write("Testing negative wind & angle values\n\n")

    for i in range(0, 10):
        wind = random.randint(-180, 0)
        angle = random.randint(-180, 0)

        sys.stdout.write("Input (wind,angle): %i %i\n" % (wind, angle))

        result = course_by_wind(wind, angle)
        postcondition(result)

        sys.stdout.write("Output (sail,rudder): %i %i %i\n\n" % result)

    sys.stdout.write("Testing negative wind & positive angle values\n\n")

    for i in range(0, 10):
        wind = random.randint(-180, 0)
        angle = random.randint(0, 180)

        sys.stdout.write("Input (wind,angle): %i %i\n" % (wind, angle))

        result = course_by_wind(wind, angle)
        postcondition(result)

        sys.stdout.write("Output (sail,rudder): %i %i %i\n\n" % result)

    sys.stdout.write("Testing positive wind & negative angle values\n\n")

    for i in range(0, 10):
        wind = random.randint(0, 180)
        angle = random.randint(-180, 0)

        sys.stdout.write("Input (wind,angle): %i %i\n" % (wind, angle))

        result = course_by_wind(wind, angle)
        postcondition(result)

        sys.stdout.write("Output (sail,rudder): %i %i %i\n\n" % result)

    sys.stdout.write("Testing positive wind == angle values\n\n")

    for i in range(0, 10):
        wind = random.randint(0, 180)
        angle = wind

        sys.stdout.write("Input (wind,angle): %i %i\n" % (wind, angle))

        result = course_by_wind(wind, angle)
        postcondition(result)

        sys.stdout.write("Output (sail,rudder): %i %i %i\n\n" % result)

    sys.stdout.write("Testing negative wind == angle values\n\n")

    for i in range(0, 10):
        wind = random.randint(-180, 0)
        angle = wind

        sys.stdout.write("Input (wind,angle): %i %i\n" % (wind, angle))

        result = course_by_wind(wind, angle)
        postcondition(result)

        sys.stdout.write("Output (sail,rudder): %i %i %i\n\n" % result)
