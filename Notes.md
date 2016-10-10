TinyGPS++
---
*
*
*


Tack
---
* Timer based (should be checking wind)
* Check speed before Tack (gps)
* 30 ft offset on either side
* "Did I tack in recent time?" (a minute) then don't tack
* When attempting to navigate buoy, aim above by boat length

generateControlLines
---
>If going upwind

>Creates tack guidelines

* From initial point, take two points in +offset and -offset x direction
* From destination point, take two points in +offset and - offset x direction
* (dest - init) y  / (dest - init) x is measured for both sets of points
* If less than 0.5 (too steep), the y coordinates are offset in the +/- directions to widen
* _offset controls the width of lane (2*sqrt(2 offset))
* should ideally be 30 feet (150)

Autonomy::Tack
---



Tacking around buoy
----

upwind count - measuring how long wind has been blowing
basing direction based on wind_abs? angle should be relative (speed)
should be 45 into the wind
tack lines should be triggering WHEN to tack
bearing is wind based
