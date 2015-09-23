public class Vector {
	public double force;
	public double dir; //direction in degrees
	Vector(double intDir, double intForce){
		force=intForce;
		dir = intDir;
	}
	
	Vector(){
		force = 0;
		dir = 0;
	}
	
	public double forceX(){
		return Math.cos(Math.toRadians(dir)) * force;
	}
	public double forceY(){
		return Math.sin(Math.toRadians(dir)) * force;
	}
}

