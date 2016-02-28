## Acronyms

* _sog_ : "Speed Over Ground"
> The speed of the vessel relative to the Ground
* _tmg_ : "Track Made Good"
> The actual distance traveled by the vessel

# STATES

## Moving Check

* Always checking "Am I close enough to the waypoint?"
* Checking "Do I have enough speed/momentum?"
* If wind is coming close to dead center front, break case
* In the case that wind is coming at greater than 15 degrees in either direction, sail with the wind

## Move to Point

* Get the direction of the true wind angle
* If wind is coming at an angle less than 75 degrees from the waypoint, switch to UPWIND
* If not, then switch to DOWNWIND
* This ensures the best sailing strategy to take when attempting to reach the point

## Downwind

* If downwind conditions have been detected for over 3 cycles, sets course using it's new heading
* The course that it has been set on by the downwind conditions will be its course
* Downwind tacking should be set up to use tack at 30 degree angles
* If less than three cycles of downwind have been detected, continue moving check

## Upwind

* Sets up tacking lines to stay within, and approach at an angle until passed
* Then the boat will tack until the wind felt is the opposite angle of initial
* The width of the tack lines are controlled using the offset variable (Should be 30 ft either side)
* Angled control lines are created at 60 angle to target when closer to waypoint

## Tack


## gpsHeading

> currentState.gpsHeading = atof(tmg.value());

* Measured in degrees
* The degree value of Travel Made Good
* atof converts string to double

## __windSpeed__ _new_

> TinyGPSCustom windSpeed(*tinyGPS, 'WIMWV', 3)

* Apparent wind speed, in knots

> state.windSpeed = atof(windSpeed.value)

* Pass into VectorFilter.h functions to find angles

> TinyGPSCustom trueWindSpeed(*tinyGPS, 'WIVWT', 3)

* True wind speed in knots, 5 for meters/s

## windDirection

> TinyGPSCustom trueWindDirection(*tinyGPS, 'WIVWT', 1)

* True wind angle in degrees from center bow __(L or R?)__

> TinyGPSCustom windDirection(*tinyGPS, 'WIMWV', 1)

* Returns wind angle in degrees 0 - 359.9 from the bow's centerline
* This is the _Apparent Wind_

> state.windDirection = windFilter.getFilteredValue(atof(windDirection.value))

* Uses Bayes filter to remove noise and returns angle of wind

## Tack

* Timer based (should be checking wind)
* Check speed before Tack (gps)
* 30 ft offset on either side
* "Did I tack in recent time?" (a minute) then don't tack
* When attempting to navigate buoy, aim above by boat length

## generateControlLines

>If going upwind

>Creates tack guidelines

* From initial point, take two points in +offset and -offset x direction
* From destination point, take two points in +offset and - offset x direction
* (dest - init) y  / (dest - init) x is measured for both sets of points
* If less than 0.5 (too steep), the y coordinates are offset in the +/- directions to widen
* \_offset controls the width of lane (2*sqrt(2 offset))
* should ideally be 30 feet (150)


## Tacking around buoy

* upwind count - measuring how long wind has been blowing
* basing direction based on wind_abs? angle should be relative (speed)
* should be 45 into the wind
* tack lines should be triggering WHEN to tack
* bearing is wind based
