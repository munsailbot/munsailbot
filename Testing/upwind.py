#!/usr/bin/env python

"""
Test case for the upwind algorithm
Not 100% accurate, but gives a visual 
idea of how the algorithm would behave on the real boat

For this demo, the origin is the bottom left corner of the screen.
All other points are initially set in polar coordinates based on this origin.
In reality, the origin would be a GPS coordinate captured at bootup.
All other necessary points would be calculated polar coordinates from this origin.
"""

import sys
import math
import pygame

# converts screen space to cartesian space
def screen_to_cartesian(point):
	return (int(point[0]), int(math.fabs(point[1]-480)))

# convertes cartesian space to screen space	
def cartesian_to_screen(point):
	return (int(point[0]), int(math.fabs(point[1]-480)))

# converts polar coordinates to cartesian coordinates	
def polar_to_cartesian(r, a):
	x = int(r*math.cos(a))
	y = int(r*math.sin(a))
	
	return (x, y)

# reflects a cartesian angle about the y axis	
def reflect_angle_y(a):
	if(a <= 180):
		return int(math.fabs(a - 180))
	else:
		return int(math.fabs(a - 540))
		
def add_angle(a, b):
	return (a + b) % 360
	
def subtract_angle(a, b):
	if ((a - b) < 0):
		return (a - b) + 360
	else:
		return (a - b)

# returns true if a point is above a line segment	
def point_above_line(point, line):
	m = float(line[1][1] - line[0][1]) / float(line[1][0] - line[0][0])
	b = float(line[1][1]) - m*float(line[1][0])
	
	if (float(point[1]) > (m*float(point[0]) + b)):
		return True

# returns true is a point is below a line segment		
def point_below_line(point, line):
	m = float(line[1][1] - line[0][1]) / float(line[1][0] - line[0][0])
	b = float(line[1][1]) - m*float(line[1][0])
	
	if (float(point[1]) < (m*float(point[0]) + b)):
		return True

# returns the distance between two points		
def distance_between_points(p1, p2):
	return math.sqrt((p2[0] - p1[0])**2 + (p2[1] - p1[1])**2)	

# generates control lines using the boat's initial position, the destination position, and an offset value
# also responsible for drawing the lines
def generate_control_lines(init_pos, dst_pos, offset):
	l1 = ((init_pos[0] - offset, init_pos[1]), (dst_pos[0] - offset, dst_pos[1]))
	l2 = ((init_pos[0] + offset, init_pos[1]), (dst_pos[0] + offset, dst_pos[1]))
	m1 = float(l1[1][1] - l1[0][1]) / float(l1[1][0] - l1[0][0])
	m2 = float(l2[1][1] - l2[0][1]) / float(l2[1][0] - l2[0][0])
	if(m1 < 0.5 or m2 < 0.5):
		l2 = ((init_pos[0], init_pos[1] - offset), (dst_pos[0], dst_pos[1] - offset))
		l1 = ((init_pos[0], init_pos[1] + offset), (dst_pos[0], dst_pos[1] + offset))
	
	pygame.draw.line(screen, (0,255,0), cartesian_to_screen(l1[0]), cartesian_to_screen(l1[1]), 2)
	pygame.draw.line(screen, (0,0,255), cartesian_to_screen(l2[0]), cartesian_to_screen(l2[1]), 2) 
	
	return (l1, l2) 

if __name__ == '__main__':
	
	global screen
	global init_pos #captured boat position from when it first starts travelling upwind
	
	screen = pygame.display.set_mode((640,480))
	
	#set the wind and initial sailing angle
	#both values are absolute
	wind = math.radians(245)
	#boat = math.radians(160)
	#boat = math.radians(math.degrees(wind) - 45)
	boat = math.radians(subtract_angle(math.degrees(wind), 45))
	sail_angle = 55
	
	#set the initial boat and waypoint position in polar coordinates
	boat_r = 600
	boat_a = math.radians(45)
	
	way_r = 400
	way_a = math.radians(45)
	
	#convert polar to cartesian
	boat_xy = polar_to_cartesian(boat_r, boat_a)
	init_pos = boat_xy
	
	way_xy = polar_to_cartesian(way_r, way_a)
	
	#initial control state and offset
	"""
	if(math.degrees(wind) < 180):
		state = 0
	else:
		state = 1
	"""
	state = 0
	event = 0
		
	offset = 64
	
	while 1:
		for event in pygame.event.get():
			if event.type == pygame.QUIT:
				sys.exit()
			
		screen.fill((255, 255, 255))
		
		if(distance_between_points(boat_xy, way_xy) > 20):
			#update boat position
			boat_x = boat_xy[0]+(math.cos(boat))
			boat_y = boat_xy[1]+(math.sin(boat))
			boat_xy = (boat_x, boat_y)
			
			#draw markers for our boat and waypoint
			pygame.draw.circle(screen, (255,0,0), cartesian_to_screen(boat_xy), 6)
			pygame.draw.circle(screen, (0,255,0), cartesian_to_screen(way_xy), 6)
			
			#line between points
			#pygame.draw.line(screen, (0,0,0), cartesian_to_screen(boat_xy), cartesian_to_screen(way_xy), 2)			
			
			#draw heading vector
			pygame.draw.line(screen, (0,0,255), cartesian_to_screen(boat_xy), cartesian_to_screen((boat_xy[0]+(32*math.cos(boat)), boat_xy[1]+(32*math.sin(boat)))), 2)
			
			#draw wind vector
			pygame.draw.line(screen, (255,255,0), cartesian_to_screen(boat_xy), cartesian_to_screen((boat_xy[0]+(32*math.cos(wind)), boat_xy[1]+(32*math.sin(wind)))), 2)	
				
			lines = generate_control_lines(init_pos, way_xy, offset)
			
			#determine which control line we should check
			#idea is to switch back and forth as we cross each one
			if(point_below_line(boat_xy, lines[1]) and point_below_line(boat_xy, lines[0])):
				if((wind > 90) and (wind < 270)):
					boat = math.radians(add_angle(math.degrees(wind), sail_angle))
				else:
					boat = math.radians(subtract_angle(math.degrees(wind), sail_angle))
				
				if(event == 0):
					offset -= 8
					event = 1
			elif(point_above_line(boat_xy, lines[1]) and point_above_line(boat_xy, lines[0])):
				if((wind > 90) and (wind < 270)):
					boat = math.radians(subtract_angle(math.degrees(wind), sail_angle))
				else:
					boat = math.radians(add_angle(math.degrees(wind), sail_angle))
				
				if(event == 0):
					offset -= 8
					event = 1
			
			if((point_below_line(boat_xy, lines[1]) and point_above_line(boat_xy, lines[0])) or (point_above_line(boat_xy, lines[1]) and point_below_line(boat_xy, lines[0]))):
				event = 0
				
			"""
			if(math.degrees(wind) < 180):
				if(state == 0):
					if(point_below_line(boat_xy, lines[1])):
						#boat = math.radians(reflect_angle_y(math.degrees(boat)))
						boat = math.radians(add_angle(math.degrees(wind), sail_angle))
						state = 1
						offset -= 8
				elif(state == 1):
					if(point_above_line(boat_xy, lines[0])):
						#boat = math.radians(reflect_angle_y(math.degrees(boat)))
						boat = math.radians(subtract_angle(math.degrees(wind), sail_angle))
						state = 0
						offset -= 8
			else:
				if(state == 0):
					if(point_below_line(boat_xy, lines[1])):
						#boat = math.radians(reflect_angle_y(math.degrees(boat)))
						boat = math.radians(subtract_angle(math.degrees(wind), sail_angle))
						state = 1
						offset -= 8
				elif(state == 1):
					if(point_above_line(boat_xy, lines[0])):
						#boat = math.radians(reflect_angle_y(math.degrees(boat)))
						boat = math.radians(add_angle(math.degrees(wind), sail_angle))
						state = 0
						offset -= 8
			"""				

		else:
			pygame.draw.circle(screen, (255,0,0), cartesian_to_screen(boat_xy), 16, 2)
		
		pygame.time.delay(50)	
		pygame.display.flip()
