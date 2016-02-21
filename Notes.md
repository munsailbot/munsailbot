## Acronyms

* _sog_ : "Speed Over Ground"
> The speed of the vessel relative to the Ground
* _tmg_ : "Track Made Good"
> The actual distance traveled by the vessel

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
