"""Test."""

# ! /usr/bin/env python

import Tkinter as tk
import serial
import threading
import math
import time


class SailState:
    """Different sailing strategies."""

    MOVING_CHECK = 0
    MOVE_TO_POINT = 1
    DOWNWIND = 2
    UPWIND = 3
    TACK = 4
    RECOVER = 5


class Filter:
    """Filtering values."""

    def __init__(self):
        """Init."""
        self.values = [0 for i in range(3)]
        self.sample_count = 0

    def get_filtered_value(self, val, sample_count):
        """Return the filtered value."""
        idx = sample_count % 3
        self.values[idx] = val
        self.sample_count = sample_count + 1

        return ((1 / 4) * (self.values[0] + (2 *
                self.values[1]) + self.values[2]))


class RunningAverage:
    """Calculate the average."""

    def __init__(self):
        """Initialize."""
        self.total = 0
        self.sample_count = 0

    def get_running_average(self, val):
        """Calculate the average."""
        self.total = self.total + val
        self.sample_count = self.sample_count + 1

        return self.total / self.sample_count

    def reset(self):
        """Reset the self values."""
        self.total = 0
        self.sample_count = 0


def distance_between(coord1, coord2):
    """Determined distance between two coordinates."""
    delta = math.radians(coord1[1] - coord2[1])
    sdlong = math.sin(delta)
    cdlong = math.cos(delta)
    lat1 = math.radians(coord1[0])
    lat2 = math.radians(coord2[0])
    slat1 = math.sin(lat1)
    clat1 = math.cos(lat1)
    slat2 = math.sin(lat2)
    clat2 = math.cos(lat2)
    delta = (clat1 * slat2) - (slat1 * clat2 * cdlong)
    delta = delta**2
    delta += (clat2 * sdlong)**2
    delta = math.sqrt(delta)
    denom = (slat1 * slat2) + (clat1 * clat2 * cdlong)
    delta = math.atan2(delta, denom)

    return delta * 6372795


def course_to_point(coord1, coord2):
    """Determine course to point."""
    dlon = math.radians(coord2[1] - coord1[1])
    lat1 = math.radians(coord1[0])
    lat2 = math.radians(coord2[0])
    a1 = math.sin(dlon) * math.cos(lat2)
    a2 = math.sin(lat1) * math.cos(lat2) * math.cos(dlon)
    a2 = math.cos(lat1) * math.sin(lat2) - a2
    a2 = math.atan2(a1, a2)
    if(a2 < 0.0):
        a2 += 6.28318530717958647693

    return math.degrees(a2)


def angle_between(a1, a2):
    """Determine angle between two angles."""
    theta = a1 - a2

    if(math.fabs(theta) > 180):
        if(theta < 0):
            theta = theta + 360
        else:
            theta = 360 - theta
    else:
        theta = math.fabs(theta)

    return theta


def cardinal_to_standard(angle):
    """Convert cardinal value to standard value."""
    standard_angle = math.fabs(angle - 450) % 360

    return standard_angle


def angle_quadrant(angle):
    """Determine which quadrant angle is in."""
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
    """Determine the angle direction."""
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


def course_by_wind(windRel, angle):
    """Determine direction by relative angle to boat."""
    rudder = 99
    side = 1
    if(angle < 0):
        side = -side

    theta = math.fabs(angle) - math.fabs(windRel)
    rudder = math.floor((45.0 / 180.0) * theta)
    rudder = rudder * side

    if(math.fabs(windRel) < 45):
        sail = 0
    else:
        sailPos = (math.fabs(windRel) - 45) * 0.66
        sail = math.floor(sailPos)

    return (sail, rudder)


def course_by_heading(windRel, heading, course):
    """Determine direction by orientation to destination."""
    side = angle_direction(cardinal_to_standard(
        heading), cardinal_to_standard(course))

    theta = angle_between(cardinal_to_standard(
        course), cardinal_to_standard(heading))
    rudder = math.floor((45.0 / 180.0) * theta)
    rudder = rudder * side

    if(math.fabs(windRel) < 45):
        sail = 0
    else:
        sailPos = (math.fabs(windRel) - 45) * 0.66
        sail = math.floor(sailPos)

    return (sail, rudder)


def tack(x, windRel, initialWind, desiredWindRel):
    """Tack function."""
    global sail_state
    global started_tack

    aggression = 3.0
    # controls the peak rudder position

    if(x > 10):
        sail_state = SailState.MOVING_CHECK
        started_tack = False

    if initialWind > 0:
        sig = math.sqrt(0.05)
        mu = 0.0

        gauss = math.floor(
            (1.0 / sig) * math.exp(-((x - mu)**2.0) / 2.0 * (sig**2.0)) * 2.0)

        rudder = gauss * aggression

        if rudder > 30:
            rudder = 30

        time.sleep(0.2)

        if desiredWindRel < initialWind:
            if windRel <= desiredWindRel:
                sail_state = SailState.MOVING_CHECK
                started_tack = False
        elif desiredWindRel > initialWind:
            if windRel >= desiredWindRel:
                sail_state = SailState.MOVING_CHECK
                started_tack = False

    elif initialWind < 0:
        sig = math.sqrt(0.05)
        mu = 0.0

        gauss = -math.floor((1.0 / sig) *
                            math.exp(-((x - mu)**2.0)
                                     / 2.0 * (sig**2.0)) * 2.0)

        rudder = gauss * aggression

        if rudder < -30:
            rudder = -30

        time.sleep(0.2)

        if desiredWindRel < initialWind:
            if windRel <= desiredWindRel:
                sail_state = SailState.MOVING_CHECK
                started_tack = False
        elif desiredWindRel > initialWind:
            if windRel >= desiredWindRel:
                sail_state = SailState.MOVING_CHECK
                started_tack = False

    return rudder

################
# Upwind Stuff #
################


def polar_to_cartesian(r, a):
    """Convert polar coordinates to cartesian coordinates."""
    x = int(r * math.cos(a))
    y = int(r * math.sin(a))

    return (x, y)


def reflect_angle_y(a):
    """Reflect a cartesian angle about the y axis."""
    if(a <= 180):
        return int(math.fabs(a - 180))
    else:
        return int(math.fabs(a - 540))


def add_angle(a, b):
    """Sum value of two angles."""
    return (a + b) % 360


def subtract_angle(a, b):
    """Subtract value of two angles."""
    if ((a - b) < 0):
        return (a - b) + 360
    else:
        return (a - b)


def point_above_line(point, line):
    """Return True if a point is above a line segment."""
    m = float(line[1][1] - line[0][1]) / float(line[1][0] - line[0][0])
    b = float(line[1][1]) - m * float(line[1][0])

    if (float(point[1]) > (m * float(point[0]) + b)):
        return True


def point_below_line(point, line):
    """Return True if a point is below a line segment."""
    m = float(line[1][1] - line[0][1]) / float(line[1][0] - line[0][0])
    b = float(line[1][1]) - m * float(line[1][0])

    if (float(point[1]) < (m * float(point[0]) + b)):
        return True


def distance_between_points(p1, p2):
    """Return the distance between two points."""
    return math.sqrt((p2[0] - p1[0])**2 + (p2[1] - p1[1])**2)


def generate_control_lines(init_pos, dst_pos, offset):
    """Generate control lines using the boat's initial position."""
    l1 = ((init_pos[0] - offset, init_pos[1]),
          (dst_pos[0] - offset, dst_pos[1]))
    l2 = ((init_pos[0] + offset, init_pos[1]),
          (dst_pos[0] + offset, dst_pos[1]))
    m1 = float(l1[1][1] - l1[0][1]) / float(l1[1][0] - l1[0][0])
    m2 = float(l2[1][1] - l2[0][1]) / float(l2[1][0] - l2[0][0])
    if(m1 < 0.5 or m2 < 0.5):
        l2 = ((init_pos[0], init_pos[1] - offset),
              (dst_pos[0], dst_pos[1] - offset))
        l1 = ((init_pos[0], init_pos[1] + offset),
              (dst_pos[0], dst_pos[1] + offset))

    return (l1, l2)


def autonomous():
    """Autonomy."""
    global wind_filter
    global spd_filter
    global filter_count

    global autonomous_mode
    global port
    global manual_control_cmd
    global auto_control_cmd
    global set_sail, set_rudder

    global sail_state
    global downwind_count, upwind_count
    global got_init, init_lat, init_lon, init_pos

    global started_upwind, boat_up_lat, boat_up_lon, offset, event
    global boat_xy, way_xy

    global tack_time, tack_initial_wind, tack_desired_wind, started_tack

    global wpLat, wpLon, wpCourse, wpDist

    if autonomous_mode == 1:
        if set_sail < 0:
            set_sail = 0
        elif set_sail > 80:
            set_sail = 80
        if set_rudder < -45:
            set_rudder = -45
        elif set_rudder > 45:
            set_rudder = 45

        filter_count = filter_count + 1
        if(filter_count > 30):
            spd_filter.reset()
            filter_count = 0

        auto_control_cmd = '$C,' + str(set_rudder) + ',' + str(set_sail) + '\n'
        port.write(auto_control_cmd)

        latitude = 0.0
        longitude = 0.0
        knotsSpd = 0.0
        cog = 0.0
        winddirection = 0.0
        sailAngle = 0
        rudderAngle = 0
        try:
            line = port.readline()
            line_str = str(line)
            msgLine = line_str.split(',')
            longitude = float(msgLine[1])
            latitude = float(msgLine[2])

            knotsSpd = float(msgLine[3])
            knotsSpd = spd_filter.get_running_average(knotsSpd)

            cog = float(msgLine[4])

            winddirection = int(msgLine[5])
            winddirection = wind_filter.get_filtered_value(winddirection)

            sailAngle = int(msgLine[6])
            rudderAngle = int(msgLine[7])
            print "LATITUDE: " + str(longitude)
            print "LONGITUDE: " + str(latitude)
            print "SPEED: " + str(knotsSpd)
            print "COG: " + str(cog)
            print "WIND: " + str(winddirection)
            print "SAIL: " + str(sailAngle)
            print "RUDDER: " + str(rudderAngle)
            # Here comes the autonomous code
            # Take into account the
            # INPUT: long lati sog cog winddirection
            # OUTPUT: sailAngle and rudderAngle

        except:
            print "Timeout"

        if not got_init:
            if(init_lat == -1 and init_lon == -1):
                if(latitude > 0 and latitude < 99):
                    init_lat = latitude
                    init_lon = longitude
                    got_init = True

        if(sail_state == SailState.MOVING_CHECK):
            print "MOVING_CHECK"

            if(knotsSpd >= 0.5):
                sail_state = SailState.MOVE_TO_POINT
            else:
                (set_sail, set_rudder) = course_by_wind(winddirection, 55)

        elif(sail_state == SailState.MOVE_TO_POINT):
            print "MOVE_TO_POINT"

            wpDist = distance_between((latitude, longitude), (wpLat, wpLon))
            wpCourse = course_to_point((latitude, longitude), (wpLat, wpLon))
            print "Distance: " + str(wpDist)
            print "Course: " + str(wpCourse)

            wind_abs = (winddirection + math.floor(cog)) % 360
            print "Abs Wind:" + str(wind_abs)
            if(angle_between(cardinal_to_standard(wind_abs),
                             cardinal_to_standard(wpCourse)) < 45):
                sail_state = SailState.UPWIND
            else:
                sail_state = SailState.DOWNWIND

        elif(sail_state == SailState.DOWNWIND):
            print "DOWNWIND"
            downwind_count = downwind_count + 1

            if(downwind_count >= 3):
                upwind_count = 0

                # if we end up in the downwind state, we should reset the
                # upwind machine (?)
                started_upwind = False
                (set_sail, set_rudder) = course_by_heading(
                    winddirection, cog, wpCourse)
                sail_state = SailState.MOVING_CHECK
            else:
                sail_state = SailState.MOVING_CHECK

        elif(sail_state == SailState.UPWIND):
            print "UPWIND"
            upwind_count = upwind_count + 1

            if(upwind_count >= 3):
                downwind_count = 0

                if(init_lat != -1 and init_lon != -1):
                    if not started_upwind:
                        boat_r = distance_between(
                            (init_lat, init_lon), (latitude, longitude))
                        boat_a = cardinal_to_standard(course_to_point(
                            (init_lat, init_lon), (latitude, longitude)))

                        boat_xy = polar_to_cartesian(boat_r, boat_a)
                        init_pos = boat_xy

                        way_r = distance_between(
                            (init_lat, init_lon), (wpLat, wpLon))
                        way_a = cardinal_to_standard(course_to_point(
                            (init_lat, init_lon), (wpLat, wpLon)))

                        way_xy = polar_to_cartesian(way_r, way_a)

                        offset = 15
                        set_rudder = 0

                        started_upwind = True
                    else:
                        boat_r = distance_between(
                            (init_lat, init_lon), (latitude, longitude))
                        boat_a = cardinal_to_standard(course_to_point(
                            (init_lat, init_lon), (latitude, longitude)))

                        boat_xy = polar_to_cartesian(boat_r, boat_a)
                        print "boat_xy: " + str(boat_xy[0]) + ","
                        + str(boat_xy[1])

                        lines = generate_control_lines(
                            init_pos, way_xy, offset)

                        wind_abs = (winddirection + math.floor(cog)) % 360

                        if((point_below_line(boat_xy, lines[1]) and
                            point_below_line(boat_xy, lines[0])) or
                           (point_above_line(boat_xy, lines[1]) and
                                point_above_line(boat_xy, lines[0]))):
                            sail_state = SailState.TACK

                            if(event == 0):
                                offset -= 3
                                event = 1
                        else:
                            (set_sail, set_rudder) = course_by_wind(
                                winddirection, 50)
                            sail_state = SailState.MOVING_CHECK

                        if((point_below_line(boat_xy, lines[1]) and
                            point_above_line(boat_xy, lines[0])) or
                                (point_above_line(boat_xy, lines[1]) and
                                 point_below_line(boat_xy, lines[0]))):
                            event = 0
                else:
                    sail_state = SailState.MOVING_CHECK

        elif(sail_state == SailState.TACK):
            print "TACK"

            if not started_tack:
                tack_initial_wind = winddirection
                tack_desired_wind = -tack_initial_wind
                tack_time = -10

                started_tack = True
            else:
                set_rudder = tack(tack_time, winddirection,
                                  tack_initial_wind, tack_desired_wind)
                tack_time = tack_time + 1

        else:
            print "else"
    elif autonomous_mode == 0:
        port.write(manual_control_cmd)

    threading.Timer(1.0, autonomous).start()


def key(event):
    """Show key or tk code for the key."""
    global autonomous_mode, sail_adjust, rudder_adjust
    global port
    global set_sail, set_rudder
    global manual_control_cmd, auto_control_cmd

    if event.keysym == 'Escape':
        root.destroy()
    if event.char == event.keysym:
        # normal number and letter characters
        # print( 'Normal Key %r' % event.char )
        if event.char == '1':
            autonomous_mode = 1
            print 'Autonomous Mode'
        elif event.char == '0':
            autonomous_mode = 0
            print 'Manual Mode'
        elif event.char == 'a':
            sail_adjust = -10
        elif event.char == 'd':
            sail_adjust = 10
        elif event.char == 'w':
            rudder_adjust = 5
        elif event.char == 's':
            rudder_adjust = -5
    elif len(event.char) == 1:
        # charcters like []/.,><#$ also Return and ctrl/key
        print('Punctuation Key %r (%r)' % (event.keysym, event.char))
    else:
        # f1 to f12, shift keys, caps lock, Home, End, Delete ...
        print('Special Key %r' % event.keysym)

    set_sail = set_sail + sail_adjust
    if set_sail < 0:
        set_sail = 0
    elif set_sail > 80:
        set_sail = 80
    set_rudder = set_rudder + rudder_adjust
    if set_rudder < -45:
        set_rudder = -45
    elif set_rudder > 45:
        set_rudder = 45

    sail_adjust = 0
    rudder_adjust = 0

    manual_control_cmd = '$R,' + str(set_rudder) + ',' + str(set_sail) + '\n'
    auto_control_cmd = '$C,' + str(set_rudder) + ',' + str(set_sail) + '\n'
    # print manual_control_cmd
    print auto_control_cmd
    # port.write(control_cmd)


if __name__ == "__main__":
    global wind_filter
    wind_filter = Filter()

    global spd_filter
    spd_filter = RunningAverage()

    global filter_count
    filter_count = 0

    global autonomous_mode, sail_adjust, rudder_adjust
    global port
    global set_sail, set_rudder
    global manual_control_cmd, auto_control_cmd
    global sail_state
    global downwind_count, upwind_count
    global got_init, init_lat, init_lon, init_pos

    global started_upwind, boat_up_lat, boat_up_lon, offset, event
    global boat_xy, way_xy

    global tack_time, tack_initial_wind, tack_desired_wind, started_tack
    tack_time = -10
    started_tack = False

    init_lat = init_lon = -1
    got_init = False
    started_upwind = False
    event = 0

    global wpLat
    wpLat = 47.578015
    global wpLon
    wpLon = -52.733616

    global wpCourse, wpDist

    sail_state = SailState.MOVE_TO_POINT
    downwind_count = 0
    upwind_count = 0
    offset = 15

    manual_control_cmd = '$R,' + str(0.0) + ',' + str(0.0) + '\n'
    auto_control_cmd = '$C,' + str(0.0) + ',' + str(0.0) + '\n'

    port = serial.Serial("/dev/tty.usbmodem1423",
                         baudrate=115200, timeout=1.0)

    # Initial values
    autonomous_mode = 0
    sail_adjust = 0
    rudder_adjust = 0

    set_sail = 0
    set_rudder = 0

    autonomous()

    root = tk.Tk()
    print("Start controlling the sailboat!")
    root.bind_all('<Key>', key)

    # don't show the tk window
    # root.withdraw()

    root.mainloop()
