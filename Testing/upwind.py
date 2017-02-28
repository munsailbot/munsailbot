"""Test case for the upwind algorithm."""

# !/usr/bin/env python

import sys
import math
import pygame


def screen_to_cartesian(point):
    """Convert screen space to cartesian space."""
    return (int(point[0]), int(math.fabs(point[1] - 480)))


def cartesian_to_screen(point):
    """Convert cartesian space to screen space."""
    return (int(point[0]), int(math.fabs(point[1] - 480)))


def polar_to_cartesian(r, a):
    """Distance between two poitns in cartesian."""
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
    """Sum of two angles."""
    return (a + b) % 360


def subtract_angle(a, b):
    """Subtract two angles."""
    if ((a - b) < 0):
        return (a - b) + 360
    else:
        return (a - b)


def point_above_line(point, line):
    """Return true if point is above a line."""
    m = float(line[1][1] - line[0][1]) / float(line[1][0] - line[0][0])
    b = float(line[1][1]) - m * float(line[1][0])

    if (float(point[1]) > (m * float(point[0]) + b)):
        return True


def point_below_line(point, line):
    """Return true if a point is below a line segment."""
    m = float(line[1][1] - line[0][1]) / float(line[1][0] - line[0][0])
    b = float(line[1][1]) - m * float(line[1][0])

    if (float(point[1]) < (m * float(point[0]) + b)):
        return True


def distance_between_points(p1, p2):
    """Return the distance between two points."""
    return math.sqrt((p2[0] - p1[0])**2 + (p2[1] - p1[1])**2)


def generate_control_lines(init_pos, dst_pos, offset):
    """Generate control lines using the boat's initial position."""

    m = float(init_pos[1] - dst_pos[1]) / float(init_pos[0] - dst_pos[0])

    if (m < 0.5):
        l2 = ((init_pos[0], init_pos[1] - offset),
              (dst_pos[0], dst_pos[1] - offset))
        l1 = ((init_pos[0], init_pos[1] + offset),
              (dst_pos[0], dst_pos[1] + offset))
    else:
        l1 = ((init_pos[0] - offset, init_pos[1]),
              (dst_pos[0] - offset, dst_pos[1]))
        l2 = ((init_pos[0] + offset, init_pos[1]),
              (dst_pos[0] + offset, dst_pos[1]))

    pygame.draw.line(screen, (0, 255, 0), cartesian_to_screen(
        l1[0]), cartesian_to_screen(l1[1]), 2)
    pygame.draw.line(screen, (0, 0, 255), cartesian_to_screen(
        l2[0]), cartesian_to_screen(l2[1]), 2)

    return (l1, l2)


def generate_angled_control_lines(init_pos, dst_pos, offset):
    """Generate control  using the boat's initial position."""

    m = float(init_pos[1] - dst_pos[1]) / float(init_pos[0] - dst_pos[0])

    if m > 1:
        offset = m*offset

    l1 = ((init_pos[0] - offset, init_pos[1]),
          (dst_pos[0], dst_pos[1]))
    l2 = ((init_pos[0] + offset, init_pos[1]),
          (dst_pos[0], dst_pos[1]))

    if (m < 0.5):
        l2 = ((init_pos[0], init_pos[1] - offset),
              (dst_pos[0], dst_pos[1]))
        l1 = ((init_pos[0], init_pos[1] + offset),
              (dst_pos[0], dst_pos[1]))
    else:
        l1 = ((init_pos[0] - (offset), init_pos[1]),
              (dst_pos[0], dst_pos[1]))
        l2 = ((init_pos[0] + (offset), init_pos[1]),
              (dst_pos[0], dst_pos[1]))

    m1 = abs(float(l1[1][1] - l1[0][1])/float(l1[1][0] - l1[0][0]))
    m2 = abs(float(l2[1][1] - l2[0][1])/float(l2[1][0] - l2[0][0]))

    pygame.draw.line(screen, (0, 255, 0), cartesian_to_screen(
        l1[0]), cartesian_to_screen(l1[1]), 4)
    pygame.draw.line(screen, (0, 0, 255), cartesian_to_screen(
        l2[0]), cartesian_to_screen(l2[1]), 4)

    return (l1, l2)


if __name__ == '__main__':

    global screen
    global init_pos
    # captured boat position from when it first starts travelling upwind
    pygame.init()
    myfont = pygame.font.SysFont("monospace", 50)

    screen = pygame.display.set_mode((640, 480))

    # set the wind and initial sailing angle
	# both values are absolute
    wind = math.radians(245)
    boat = math.radians(subtract_angle(math.degrees(wind), 45))
    sail_angle = 55

	# set the initial boat and waypoint position in polar coordinates
    boat_r = 400
    boat_a = math.radians(45)

    way_r = 300
    way_a = math.radians(75)

    # convert polar to cartesian
    boat_xy = polar_to_cartesian(boat_r, boat_a)
    init_pos = boat_xy
    way_xy = polar_to_cartesian(way_r, way_a)

    # initial control state and offset
    state = 0
    event = 0
    offset = 64

    start_ticks = pygame.time.get_ticks()  # starter tick
    timer = 1

    while 1:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                sys.exit()
        if timer == 1:
            seconds = float((pygame.time.get_ticks() - start_ticks) / 10)

        screen.fill((255, 255, 255))

        time = '{0:.2f}'.format(seconds / 100)
        label = myfont.render(time, 1, (0, 0, 0))
        screen.blit(label, (10, 10))

        if(distance_between_points(boat_xy, way_xy) > 20):
            # update boat position
            boat_x = boat_xy[0] + (math.cos(boat))
            boat_y = boat_xy[1] + (math.sin(boat))
            boat_xy = (boat_x, boat_y)

            # draw heading vector
            pygame.draw.line(screen, (0, 0, 255), cartesian_to_screen(boat_xy),
                             cartesian_to_screen(
                (boat_xy[0] + (32 * math.cos(boat)),
                    boat_xy[1] + (32 * math.sin(boat)))), 2)

            # draw wind vector
            pygame.draw.line(screen, (255, 255, 0),
                             cartesian_to_screen(boat_xy), cartesian_to_screen(
                (boat_xy[0] + (32 * math.cos(wind)), boat_xy[1]
                 + (32 * math.sin(wind)))), 2)

            # draw markers for our boat and waypoint
            pygame.draw.circle(screen, (255, 0, 0),
                               cartesian_to_screen(boat_xy), 6)
            pygame.draw.circle(screen, (0, 255, 0),
                               cartesian_to_screen(way_xy), 6)

            control = [generate_angled_control_lines(
                init_pos, way_xy, offset), generate_control_lines(init_pos, way_xy, offset)]

            for lines in control:
                if(point_below_line(boat_xy, lines[1]) and point_below_line(boat_xy, lines[0])):
    				if((wind > 90) and (wind < 270)):
    					boat = math.radians(add_angle(math.degrees(wind), sail_angle))
    				else:
    					boat = math.radians(subtract_angle(math.degrees(wind), sail_angle))
    				if(event == 0):
    					event = 1

                elif(point_above_line(boat_xy, lines[1]) and point_above_line(boat_xy, lines[0])):

                    if((wind > 90) and (wind < 270)):
    					boat = math.radians(subtract_angle(math.degrees(wind), sail_angle))
                    else:
                        boat = math.radians(add_angle(math.degrees(wind), sail_angle))
                    if(event == 0):
    					event = 1

                if((point_below_line(boat_xy, lines[1]) and point_above_line(boat_xy, lines[0])) or (point_above_line(boat_xy, lines[1]) and point_below_line(boat_xy, lines[0]))):
    				event = 0

        else:
            pygame.draw.circle(screen, (255, 0, 0),
                               cartesian_to_screen(boat_xy), 16, 2)
            timer = 0

        pygame.time.delay(50)
        pygame.display.flip()
