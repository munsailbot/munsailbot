/* A little script to calculated speed made good (SMG) vs. Position. It it 
// meant to be more of a visual representation than any actual navigational 
  
To do: 
    -Implement GPS fuctions (pointers etc.) 
    -Implement realtime calls to wind angle, speed as well 
    -Implement correct Wind Angle Call 
    -Implement Correct Rudder/Sail Position Calls 
    -Write code to convert GPS Pos to X,Y Coordinate 
        base point input 
        conversion of GPS to Cartesian 
    -loop all control functions to stabalize the control 
    -Make sure all variable are correctly names 
    -General syntax/style check 
  
*/
  
  
  
//close all; 
//clear all; 
public class BoatMovement{
	//*********** Conversions ******************************** 
	 
	//double KtoM = .514; // Conversion factors of Knots to m/s 
	  
	//double CRS = ((90-Heading)/180)* Math.PI; // converts to radians and in standard quadrant notation
	//double  absWind = CRS+((WindRel/180) * Math.PI); // Converts the relative wind speed in to absolute wind speed 
	  
	double earthRad = 6372795.0; //earth's radius in meters 
	  
	//*************** User set data ************** 
	// List of waypts 
	// Base positio0n as GPS point (used to convert all gps data to x and y) 
	  
	double D = 100; // course offset for parallel lines, user defined and may change with  
	        //distance fr. waypt. 
	double tooSlow = 1.2; // Minimum speed in m/s 
	  
	//              ***** 
	  
	  
	//****************Input Data (from the Arduino) **************** 
	  
	int wp_Id = 0; // initializes to zero, might change this 
	//XYCoord wayPoint = new XYCoord (Wp_lat,Wp_long,baseLat,baseLong); // creates xy coord object to convert WyPt lat/long to x,y 
	  
	
	double speed = 0;//get the speed in m/s 
	double DisWayPT = 0;// get the distance to the CURRENT waypoint 
	double courseOverGround = 0;//Course over ground from GPS 
	double pos_X = 0;// vessel position in meters from reference point-METHOD? 
	double pos_Y = 0;//vessel position in meters from reference point - METHOD? 
	double heading;
	  
	double Wind_Rel = 0; //direction of wind relative to boat (+/- 180 degrees) 
	double Wind_True = 0; //Get the true direction of the wind, NOT relative to boat-METHOD
	double compassAngle =0;
	double rudderAngle;
	Boat boat;
	/*
	 * Update Data
	 * This Updates the information on most of the variables.
	 */
	public void updateData(){
		courseOverGround = Math.atan((boat.getYPos()- pos_Y)/(boat.getXPos()- pos_X));
		speed = Math.sqrt(Math.pow((boat.getXPos()- pos_X), 2) + Math.pow((boat.getYPos()- pos_Y), 2));
		pos_X = boat.getXPos();
		pos_Y = boat.getYPos();
		Wind_True = boat.getWindAngle();
		heading = boat.getHeading();
		
		
	}
	
	  
	//Initializing function
	public BoatMovement(Boat thisBoat){
		boat = thisBoat;
		
	}  
	/******************** Steps 
	 * @throws InterruptedException ********************************
	 *  
	 * 
	 * 
	 */
	public void steps() throws InterruptedException{
		updateData();
		XYCoord waypoint = nextWaypoint();
		//If moving too slow
		if(speed < tooSlow){
			notMoving();
			System.out.println("Using Not Moving");
		}
		else if (Math.abs(angleToPosition(waypoint.XPos(),waypoint.YPos())-abs_Wind())<45){
			goingUpwind(waypoint.XPos(),waypoint.YPos());
			System.out.println("Using Upwind");
		}
		//Otherwise can sail Direct
		else if (Math.abs(CRStoWP(waypoint.XPos(), waypoint.YPos())-abs_Wind())>45){
			System.out.println("Using Direct Course");
			directCourse(waypoint);
		}
	}
	//****************** Checking if the boat is moving *********************** 
	public void notMoving(){		  
		if (Wind_Rel> 0){//Wind is off the PORT side of the vessel
			CrsByWind (Wind_Rel,90); //METHOD -takes the relative wind and the 
			//angle to sail
		} 
		  
		if (Wind_Rel< 0){//Wind is off the STBD side of the vessel  
			CrsByWind (Wind_Rel,-90); //METHOD -takes the relative wind and the 
			//angle to sail 
		}
	}
	  
	//************************** Upwind Code ******************************* 
	/* This will need some fine tuning. Points: 
	-What happens if the boat isn’t on the best tack and is in fact 
	sailing away from the mark? 
	-We may want to loop the stearing a couple times before leaving 
	the STATE. This may smooth out the stearing. 
	- The CrsByWind isn’t the best and needs something that makes 
	luffing worse than bearing off - some sort of bias. 
	-Need a counter that keeps track of if we are getting closer to 
	the mark (Dist_Tracker) 
	*/ 
	public void goingUpwind(double wayPointX, double wayPointY) throws InterruptedException{
		double offsetWidth = tackingOffesetWidth(wayPointX, wayPointY);
		//Waypt Id advance 
		Line Upwind = new Line(Wind_True, wayPointX, wayPointY, offsetWidth); //create an line object 
		//that holds the line equations and that can then be extracted as we need. 

		if((pos_Y<(Upwind.CheckUpper(pos_X))) || pos_Y>(Upwind.CheckLower(pos_X)))// Vessel is between the course lines 
		{ 
			if (Wind_Rel> 0) //Wind is off the PORT side of the vessel 
			{ 
				CrsByWind (Wind_Rel,45); 
			} 

			if (Wind_Rel< 0) //Wind is off the STBD side of the vessel 
			{ 
				CrsByWind (Wind_Rel,-45); 
			} 

		} 

		if(pos_Y>Upwind.CheckUpper(pos_X) && pos_Y>Upwind.CheckLower(pos_X) 
				&& Upwind.Intersect(pos_X,pos_Y,heading,1) > pos_X) { 
			//Vessel is above the Upwind bounding line and heading away from mark 
			//needs to tack 
			Tack(); 
		} 

		if(pos_Y<Upwind.CheckUpper(pos_X) && pos_Y<Upwind.CheckLower(pos_X)
				&& Upwind.Intersect(pos_X,pos_Y,heading,0) < pos_X)//Vessel is above the Upwind bounding line and heading away from mark 
			//needs to tack 
		{ 
			Tack(); 
		}
	}
	/************* tackingOffesetWidth *******************************
	  * Calculates the optimal offset when tacking upwind.
	  * This is currently mostly a placeholder function as it holds only a fixed offset.
	  * @params wayPointX: X component of target location
	  * 		wayPointY: Y component of target location
	  * 
	  * @returns offset width in meters
	  * */

	 public double tackingOffesetWidth(double wayPointX, double wayPointY){
		 return D;
	 }
	
	/*********************** Direct Course Code************* 
	For courses beyond 45 degrees (might change) to the wind the vessel can sail directly to the mark 
	Problems: 
	-What if the vessel is facing the wrong way? 
	-Knowing that the vessel has made the mark (problem with upwind as well) 
	-Rudder stability 
	Still to do: 
	-Implement this fuction in Rudder Control portion and Tack/Gybe Portion 
	-implement correct gps distance call 
	-implement realtime calls to wind angle 
	*/
	public void directCourse(XYCoord waypoint){
		double side = 0;// Used to control which way the boat turns 
		double sailPos;
		//sail control portion ****** 
		if (Wind_Rel<45) //sail control 
		{ 
			sailPos = 0; //Sails all the way in 
		}
		else
		{ 
			sailPos = (Math.abs(Wind_Rel)-45)*.222; /*My initial attempt at proportional sail control */
		} 
		boat.setSailAngle((int)sailPos);
		
		SimpleStructure struct = Angle(courseOverGround,CRStoWP(waypoint.XPos(),waypoint.YPos()),side);
		side = struct.side;
		double turnAngle = struct.turnAngle;
		boat.setRudderAngle((int)turnAngle);//call to Angle function, Side is passed by reference

		//Rudder Control ***** 
		if(turnAngle>30)// Full rudder, 30 degrees is our max range(might be a little too Sensitive) 
		{ 
			boat.setRudderAngle((int) side*30);//not sure what the real command is (STBD = neg.) 
		} 

		else
		{ 
			double rudderPosition = 30/(31-turnAngle);// proportional rudder 
			boat.setRudderAngle((int) (side*rudderPosition));//not sure what the real command is, assume pos = turn to port 
		} 
	}
	  
	// Tacking or gybing 
	  
	//Tacking the side the vessel is turning the same as the side the wind is coming over, thus it is 
	//turning into the wind.  If it is turning greater that the angle to the wind, then it needs to tack. 
	  /*  if (Side == sign(Wind_Rel) && Wind_Rel< turnAngle){ 
	        Tack(COG,Wind_Rel); 
	    } */
	  
	//Gybe if the vessel whats to turn away from the wind and the Turn Angle is greater that the wind 
	//angle plus 90 
	   /* if (Side != sign(Wind_Rel) && Sign((Angle(CRStoWP(), wind_Abs(), Side)*Side)==Sign(Wind_Rel))){ 
	        Gybe(COG,Wind_Rel);//Function that hasn't been written yet 
	    } 
	    //tacking statement- if vessel is headed the wrong way 
	  
	        wpID++; 
	   } 
	  
	}*/
	/*********************************
	 * This is a mostly place-holder function to 
	 * 
	 * @return: Desired angle to move to get to the next Waypoint
	 */
	public double CRStoWP(double wayPointX, double wayPointY){
		double tempAngle = Math.toDegrees(Math.atan((wayPointY - pos_Y)/(wayPointX - pos_X)));
		if (wayPointY < pos_Y && wayPointX < pos_X){
			tempAngle=180-tempAngle;
		}
		else if( wayPointX < pos_X && wayPointY > pos_Y){
			tempAngle = 180+tempAngle;
		}
		else if( wayPointX>pos_X && wayPointY> pos_Y){
			tempAngle = 360-tempAngle;
		}
		return tempAngle;
		
	}
 

	
	  

	  
	  
	  
	 // ********************************************************************* 
	//************************ METHODS *********************************** 
	  
	  
	/********************** CrsByWind **************************** 
	Method that steers the boat by the wind.  The rudder control is proportional to how 
	far off course the boat is(may need an integral controller).  The sail control is proportional to 
	the wind. 
	NOTE: This will need to be put in a loop to stabalize the control 
	@Params: WindRel  and AngleToSail 
	@return: void
	**************************************************************************/ 
	  
	public void CrsByWind (double WindRel, double AngleToSail){ 
	    int Side = 1; // variable to invert if statement when wind is off port side(negative values)
	    double sailPos;
	    if(AngleToSail<0) 
	    { 
	        Side = -1; 
	    } 
	    if ((AngleToSail-WindRel)*Side>0)// Pointing to windward of the desired angle 
	    { 
	        if((AngleToSail-WindRel)>30) 
	        {// Full rudder, 30 degrees is our max range 
		    	boat.setRudderAngle((int) (30 +.5));//not sure what the real command is 
	        } 
	  
	        else
	        { 
	            double RPos = 30/(31-(Math.abs(AngleToSail-WindRel)));// proportional rudder 
	            boat.setRudderAngle((int)(RPos+.5));//not sure what the real command is, assume pos = turn to port 
	        } 
	    } 
	  
	    if ((AngleToSail-WindRel)*Side<0) // vessel is pointing leeward of desired hdg 
	        { 
	            if(Math.abs(AngleToSail-WindRel)>30)// Full rudder, 30 degrees is our max range 
	            { 
	                boat.setRudderAngle((int) ((Side*-30) +.5));
	            } 
	  
	            else
	            { 
	            double RPos =AngleToSail-WindRel;// proportional rudder 
	            boat.setSailAngle((int)(RPos+.5));
	            //Rudder.SetPos (RPos);/*not sure what the real command is, assumes - =turn to stbd*/
	            } 
	        } 
	  
	    if (WindRel<45) //Sail Position 
	    { 
	    	sailPos = 0; //Sails all the way in 
	    } 
	  
	    else
	    { 
	    	 sailPos = ((Math.abs(Wind_Rel)-45)*.222); /*My initial attempt at proportional sail control */
	    } 
	   
	    boat.setSailAngle((int) (sailPos + 0.4999));//Set proper 
	  
	    return; 
	} 
	  
	  
	/************************  Tack ****************************** 
	Method that tacks the vessel.  It should take the currect heading and MAYBE the angle 
	to the wind we wish to sail (if that becomes a learned item).  We may also want it to 
	loop a couple of times to straighten the boat out on crs. STILL NEED TO SMOOTH OUT WIND DIR. SYNTAX 
	This code can be improved over time to create proper tacking technique( ease sheets, gain speed, turn etc.) 
	This will need to get current data as the vessel goes through the eye of the wind 
	 * @throws InterruptedException 
	@params: Current heading, Wind angle 
	@Return:void 
	*/
	public void Tack () throws InterruptedException 
	{ 
	    double Side = sign(Wind_Rel); // Used to tell code what side the wind is on 
	  
	    double Curr_heading = boat.getHeading();// Passed heading value 
	  
	    for(int i=1;i>30;i++) 
	    { 
	        int Rudder = i; 
	        boat.setRudderAngle(i); //Not sure the real command 
	        Thread.sleep(10); //Ten milisec. delay for smoother transition 
	    } 
	  
	    while (Math.abs(Wind_Rel-Wind_Rel)<50) //abs. value makes it work for both tacks (Port and STBD) 
	    { 
	    	Thread.sleep(200); // keeps the boat turning until it is through the eye of the wind 
	    } 
	  
	    for(int j=1;j<300;j++)// This is supposed to settle the vessel on the new crs 
	    { 
	        CrsByWind(Wind_Rel,-55*Side); 
	        Thread.sleep(10);
	    } 
	  
	    return; 
	} 
	  
	/******************* Angle ********************************** 
	Function that takes two angles in 0-360 notation and  computes the anlge betweeen them.  Designed to be used 
	to calculate the difference between the COG and the CRS_Wp, but could also be used for wind angle etc. 
	@params: two angles, COG and =CRS_Wp, pass by reference of SideDirect 
	@return: Angle between two headings 
	*/
	public SimpleStructure Angle (double COG, double CRS_Wp, double side){ 
	    double TurnAngle = COG-CRS_Wp; // Angle between COG and CRStoWP- Assumes both are 0-360 
	  
	  
	    if (Math.abs(TurnAngle)>180) //Corrects for the 0/360 problem 
	    { 
	        if (TurnAngle<0)    { 
	            TurnAngle = TurnAngle+360; 
	            side = -1; 
	        } 
	  
	        else
	        { 
	            TurnAngle = 360-TurnAngle; 
	            side = 1; 
	        } 
	    } 
	  
	    else {//Might be able to clean this up by Initializing "side" to 1 
	        if (TurnAngle<0){ 
	            TurnAngle = Math.abs(TurnAngle); 
	            side = -1; 
	        } 
	        else{ 
	            side =1; 
	        } 
	    } 
	    SimpleStructure structure = new SimpleStructure(TurnAngle,side);
	    return structure; 
	} 
	  
	  
	  
	/***************** GYBE ************************************ 
	The Gybe function.  Works similar to tack in that is takes the wind direction 
	and uses it to plot its course.  No attention is paid to the CRStoWP of the next 
	wayoint 
	NOTE: need to update wind inside loops 
	@params: Wind Rel and COG 
	@return: void 
	 * @throws InterruptedException 
	*/
	void Gybe (double COG, double windRel) throws InterruptedException 
	{ 
	   double side = sign(windRel); 
	   double wind = side*150; 
	   side = side*-1;//Inverts the side, used later in rudder commands 
	  
	   CrsByWind (windRel, wind); 
	  
	   boat.setSailAngle((int) 4.5);//Assuming 30 = full out 
	   Thread.sleep(300); // time to let sail out 
	  
	   for(int i=1;i>30;i++) 
	    { 
	        int Rudder = i; 
	        boat.setRudderAngle((int)side*Rudder); //Not sure the real command 
	        Thread.sleep(10); //Ten milisec. delay for smoother transition 
	    } 
	  
	  
	    while (Math.abs(Wind_Rel)<150) //abs. value makes it work for both side (Port and STBD) 
	    { 
	    	Thread.sleep(200); // keeps the boat turning until it is through the eye of the wind 
	    } 
	  
	    for(int j=1;j<300;j++)// This is supposed to settle the vessel on the new crs 
	    { 
	        CrsByWind(Wind_Rel,side*140); 
	        Thread.sleep(100); 
	    } 
	  
	    return; 
	} 
	  
	  
	  
	/****************  SIGN ******************************** 
	Little function that checks if the number is pos. or neg. 
	@params: Number to check 
	@return: sign  of number (-1 or +1) 
	*/
	public static double sign (double number) 
	{ 
	    if (number>1) 
	    { 
	        return 1; 
	    } 
	    else{ 
	        return -1; 
	    } 
	} 
	  
	  
	/****************  Absolute Wind *********************** 
	Calculates the wind direction in compass coordinates and 
	not relative to the vessel 
	@params: COG and Wind_relative 
	@return: Wind direction (0-360) 
	*/
	  
	public double abs_Wind (){ 
		double absWind = courseOverGround-Wind_Rel; 
	    if(absWind>360) 
	    { 
	        absWind=absWind-360; 
	    } 
	  
	    if(absWind<360) 
	    { 
	        absWind=absWind+360; 
	    } 
	    return absWind;
	}
	/**************** Heading **********************************
	 * Input 
	 * 
	 *
	 */
	 private double heading(){
		 return heading;
	 }
	 
	 /*Finds the angle the boat is from the inputed x and y
	  * 
	  */
	 private double angleToPosition(double x2,double y2){
		 return Math.toDegrees(Math.atan((y2-boat.getYPos())/(x2-boat.getXPos())));
	 }
	 /*******************Next Waypoint ********************
	  * 
	  * 
	  */
	 public XYCoord nextWaypoint(){
		 XYCoord waypoint = new XYCoord(400,0);
		 return waypoint;
	 }
}
