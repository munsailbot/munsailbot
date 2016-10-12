public class Line {
	//generic line variable mx + b
	double m;
	double x;
	double b1;
	double b2;
	
	public Line (double direction, double x, double y, double offsetDistance){ 
	   double m = direction; //Xint_lower 
	   double b = y-Math.tan(direction)*x; //calculates the base line of the wind through the mark 
	   double offset = offsetDistance*Math.sqrt(Math.pow(Math.tan(direction),2)+1); /* Pure math, search offsetting 
	   lines to get more, offsetDistance = offset amount in meters */
	 
	   // sets the b-values for the two restricting lines 
	   double b1 = b+offset; //Top line 
	   double b2 = b-offset; //Bottom line 
	 }
	// Method to return upper-line 
	public double CheckUpper(double posXL) { 
		double y_up = m*posXL+b1; 
		return y_up; 
	} 
	// Method to return lower-line 
	public double CheckLower(double posXL) { 
		double y_lower = m*posXL+b2;
		return y_lower;
	} 
}

