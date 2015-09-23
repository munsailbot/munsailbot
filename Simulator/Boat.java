
public class  Boat extends JPanel{
	//exact forms of boat variables
	private double heading = 270; //boat heading direction counterclowise from right
	private double x = 400;
	private double y = 400;
	private double sailAngle = 90; //counterclockwise angle from heading position
	private double rudderAngle = 0; //0 degrees is when rudder parallel to heading 
									//clockwise is positive
	private double windDir = 75; //Wind angle counterclockwise from right or west
	private double windSpeed = 3;
	private double velocityX = 0;
	private double velocityY = 0;
	
	private double boatSpeed  = 2; //for boat on motor
	private double time = 0;
	private int arrayLength = 0;
	double[][] positions = new double [2][10000];
	private double rudderAngleGoal = rudderAngle;
	private double sailAngleGoal = sailAngle;
	
	private double windConstant = .1;
	
	//Drawing Variables
	Line2D drawLine =new Line2D.Double(10, 10, x, y);
	private static int windowWidth = 800;
	private static int windowHeight = 800;
	private double shipLength1 = 35;
	private double boatWidth = 20;
	private double sailLength =10;
	int temp;
	
	public static void main(String arg[]) throws InterruptedException{
		JFrame frame = new JFrame("Boat Testing Enviroment");
		double[][] positions = new double [2][10000];
		
		Boat b = new Boat();
		BoatMovement boat = new BoatMovement(b);
		
		frame.add(b);
		frame.setSize(windowWidth,windowHeight);
	    frame.setVisible(true);
	    frame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
	    frame.setBackground(Color.BLUE);
		
		for(int i=0; i<50; i++){
			b.repaint();
			boat.steps();
			b.moveBoat();
			Thread.sleep(40);
			System.out.println();
			System.out.println("Step: " + i);
			System.out.println("Sail Angle Goal: " + b.sailAngleGoal);
			System.out.println("Sail Angle: " + b.sailAngle);
			System.out.println("Rudder Angle Goal: " + b.rudderAngleGoal);
			System.out.println("Rudder Angle: " + b.rudderAngle);
			System.out.println("Heading: " + b.heading);
			
		};
	}
	
	@Override
	public void paint(Graphics g) {
		//initialize graphics
		Graphics2D g2d = (Graphics2D) g;
		
		//Clear Screen
		g2d.setColor(Color.BLUE);
		g2d.fillRect(0, 0, windowWidth, windowHeight);
		
		//draw base circle 
		g2d.setColor(Color.WHITE);
		Ellipse2D.Double circle = new Ellipse2D.Double(0, 0, 50, 50);
		g2d.fill(circle);
		
		//Draw wind Direction
		drawLine.setLine(25, 25, 25+23*Math.cos(windDir), 25+23*Math.sin(windDir));
		g2d.setColor(Color.RED);
		g2d.draw(drawLine);
		
		//Draw boat
		g2d.setColor(Color.BLACK);
		Polygon shipDrawing = shipPoints(x,y);
		g2d.drawPolygon(shipDrawing);
		g2d.fillPolygon(shipDrawing);
			
		//Draw Sail
		g2d.setColor(Color.WHITE);
		double radSailAngle = Math.toRadians(sailAngle);
		g2d.drawLine((int)(x+Math.cos(radSailAngle)*sailLength),(int) (y+Math.sin(radSailAngle)*sailLength),
				(int)(x+Math.cos(radSailAngle+ Math.PI )*sailLength),(int) (y+Math.sin(radSailAngle+ Math.PI )*sailLength));
	}
	
	//This draws a polygon to represent the ship
	public Polygon shipPoints(double xPos, double yPos){
		Polygon ship = new Polygon();
		double radHeading = Math.toRadians(heading);
		double rightAngle = Math.toRadians(90);
		ship.addPoint((int)(xPos+shipLength1 * Math.cos(radHeading)), (int)(yPos-shipLength1 * Math.sin(radHeading)));
		ship.addPoint((int)(xPos+(boatWidth/2)*Math.cos(radHeading)+(boatWidth/2)*Math.cos(radHeading-rightAngle)),
				(int)(yPos-(boatWidth/2)*Math.sin(radHeading)-(boatWidth/2)*Math.sin(radHeading-rightAngle)));
		ship.addPoint((int)(xPos-(boatWidth/2)*Math.cos(radHeading)+(boatWidth/2)*Math.cos(radHeading-rightAngle)), //inversed  first part
				(int)(yPos+(boatWidth/2)*Math.sin(radHeading)-(boatWidth/2)*Math.sin(radHeading-rightAngle)));
		ship.addPoint((int)(xPos-(boatWidth/2)*Math.cos(radHeading)-(boatWidth/2)*Math.cos(radHeading-rightAngle)),
				(int)(yPos+(boatWidth/2)*Math.sin(radHeading)+(boatWidth/2)*Math.sin(radHeading-rightAngle)));
		ship.addPoint((int)(xPos+(boatWidth/2)*Math.cos(radHeading)-(boatWidth/2)*Math.cos(radHeading-rightAngle)),
				(int)(yPos-(boatWidth/2)*Math.sin(radHeading)+(boatWidth/2)*Math.sin(radHeading-rightAngle)));
		
		return ship;
	}
	
	//A simple boat movement system.
	public void moveBoat(){
		heading -= rudderAngle/10;
		if(heading<0){
			heading += 360;
		}
		heading = heading % 360;
		x += Math.cos(Math.toRadians(heading)) * boatSpeed + Math.cos(Math.toRadians((windDir))) * windSpeed;
		y += Math.sin(Math.toRadians(heading)) * boatSpeed + Math.cos(Math.toRadians((windDir))) * windSpeed;
		moveRudder();
		moveSail();
		store();
	}
	
	//to be created advanced move function
	public void advancedMove(){
		heading -= rudderAngle/10; 
		addFriction();
		Vector wind = windForce();
		Vector keel = keelForce(wind);
		
		velocityX += wind.forceX() + keel.forceX();
		velocityY += wind.forceY() + keel.forceY();
		
		x += velocityX;
		y -= velocityY;
		
		System.out.println("Heading:" + heading);
		moveRudder();
		moveSail();
		store();
	}
	
	public void addFriction(){
		velocityX *= .93 - rudderAngle/40; 
		velocityY *= .93 - rudderAngle/40; 
	}
	
	private double windSpeed(){
		return Math.abs(Math.cos(Math.toRadians(time))) / 2;
	}
	private Vector windForce(){
		
		Vector wind = new Vector();
		double sailDir = (sailAngle - heading) % 180; //gets the sail angle with respect to CCW from right
		
		wind.dir = sailDir - 90;
		
		/*
		//Simply determines wind direction
		if((windDir>sailDir) && (windDir<sailDir +180)){
			wind.dir = sailDir + 90;
			System.out.println("1");
		}
		else{
			wind.dir = sailDir - 90;
		}
		*/
		
		//Makes sure wind direction is not negative
		if(wind.dir<0){
			wind.dir += 360;
		}
		
		//formula for wind. wind Constant is completly arbitrary 
		wind.force = windConstant * (windSpeed * windSpeed) * Math.sin(Math.toRadians(sailDir-windDir));
		//System.out.println("Angular constant:" + Math.cos(Math.toRadians(wind.dir-windDir)));
		System.out.println("Wind Force:" + wind.force);
		System.out.println("Wind Direction:" + wind.dir);
		return wind;		
	}
	private Vector keelForce(Vector windVector){
		Vector keel = new Vector();
		
		double absHeading = heading % 180;
		keel.dir = absHeading - 90;
		/*
		  
		if((windVector.dir > absHeading) && (windVector.dir < absHeading + 180)){
			keel.dir = absHeading + 90;
		}
		else{
			keel.dir = absHeading - 90;
		}*/
		
		if (keel.dir<0){
			keel.dir +=360;
		}
		
		keel.force = .98 * Math.sin(Math.toRadians(absHeading-windVector.dir)) * windVector.force;
		
		return keel;
	}
	
	private void store(){
		positions[0][arrayLength] = x;
		positions[1][arrayLength] = y;
		arrayLength++;
	}
	
	public int getCompassAngle(){
		if(heading>=90){
			return (int) (heading - 90 + 0.4999);
		}
		else{
			return (int) (heading + 270 + 0.4999);
		}
		
	}
	
	public double setRudderAngle(int newAngle){
			rudderAngleGoal = newAngle;
			return rudderAngleGoal;
	}
	
	public void setSailAngle(int newAngle){
		sailAngleGoal = newAngle;
	}
	
	public double getXPos(){
		return x;
	}
	
	public double getYPos(){
		return y;
	}
	
	public int getRudderAngle(){
		return (int) (rudderAngle + 0.4999);
	}
	
	public int getWindSpeed(){
		return (int) windSpeed; 
	}
	
	public int getWindAngle(){
		return (int) windDir;
	}
	
	public int getHeading(){
		return (int) heading;
	}
	
	private void moveRudder(){
		if(Math.abs(rudderAngle)<=180){
			if(Math.abs(rudderAngle-rudderAngleGoal) <= 3.0){
				rudderAngle = rudderAngleGoal;
			}
			else if (rudderAngle> rudderAngleGoal){
				rudderAngle -= 3;
			}
			else{
				rudderAngle +=3;
			}
		}
		else{
			System.out.println("Rudder Angle set beyond 180 degrees");
		}
	}
	
	private void moveSail(){
		if(Math.abs(sailAngle)<=180){
			if(Math.abs(sailAngle-sailAngleGoal) < 3.0){
				sailAngle = sailAngleGoal;
			}
			else if (sailAngle> sailAngleGoal){
				sailAngle -= 3;
			}
			else{
				sailAngle += 3;
			}
		}
		else{
			System.out.println("Sail Angle set beyond 180 degrees");
		}
	}
}
