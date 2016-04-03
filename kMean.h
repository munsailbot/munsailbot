FindNodeCenters(LinkedList GPSList, int iteration = 10, float tolerance = 0.01) //TODO: make tolerence = 1 ftell
{
	//TODO: should be vector
	// If within tolerance or max iterations, end function
	//GPSList - where location might be
	// _waypoints
	// Use waypoints array to set up start points
	// TODO: When centers are found and new points are added, use the new centers
	// as the start points instead of setting up 1/3 points
	// Should kmean be a class? (Member functions)...

	//TODO: set up defalt starting postions of center postions
	//Usually randomized, but split into one third of the hyp of field
	new GPS_cords C1Center;
	new GPS_cords C2Center;

	//center of each cluster
	new GPS_cords oldC1Center;
	new GPS_cords oldC2Center;

	// New center is found by finding center of distance between GPS clusters for each point

	int C1Count = 0, C2Count = 0; // number of points assigent to each cluster

	for(i=0, i< iteration, i++)
	{
		do  // do this for every recorded GPS point guess
		{
			if((C1Count+C2Count) == 0) // first pass
			{
				GPSNode = GPSList.startNode;
				// Vector instead
			}
			else
			{
				GPSNode = GPSNode.nextNode();
			}

			//declare nearest point in GPS.h
			if(nearestPoint(GPSNode, C1Center, C2Center) == C1Center)
			//distanceBetween(C1Center, GPSNode) < distanceBetween(C2Center, GPSNode)
			{
				//assign to cluster 1
				C1Center.Lat =  (C1Center.Lat*C1Count + GPSNode.GetGPSValLat())/(C1Count+1);
				C1Center.Lon =  (C1Center.Lon*C1Count + GPSNode.GetGPSValLon())/(C1Count+1);
				C1Count++;
			}
			else
			{
				C2Center.Lat =  (C2Center.Lat*C2Count + GPSNode.GetGPSValLat())/(C2Count+1);
				C2Center.Lon =  (C2Center.Lon*C2Count + GPSNode.GetGPSValLon())/(C2Count+1);
				C2Count++;
			}

		} while(GPSNode != GPSList.endNode); //each node
		// TODO: replace linkedlist with vector


		// tolerance should be fixed - value is too narrow
		if(distanceBetween(C1Center, oldC1Center) <= tolerance && (distanceBetween(C2Center, oldC2Center) <= tolerance))
		// if the center point is within tolerance
		{
			break; //exit for loop
		}
		else
		{
			oldC1Center = C1Center;
			oldC2Center = C2Center;
		}
	}
}

//template <typename T>
//findHypotenuse(T x, T y)
//{
//	return sqrt(x*x+y*y);
//}
