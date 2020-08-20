var rigidBody;

function initPhysics() {

	// Setup our world
	var world = new CANNON.World();
	world.gravity.set(0, 0, 0); // m/s²

	// Create a sphere
	var radius = 1; // m
	rigidBody = new CANNON.Body({
	   mass: 5, // kg
	   position: new CANNON.Vec3(0, 0, 0), // m
	   shape: new CANNON.Sphere(radius)
	});
	
	world.addBody(rigidBody);


	var fixedTimeStep = 1.0 / 60.0; // seconds
	var maxSubSteps = 3;

	// Start the simulation loop
	var lastTime;
	(function simloop(time){
	  requestAnimationFrame(simloop);
	  if(lastTime !== undefined){
	     var dt = (time - lastTime) / 1000;
	     world.step(fixedTimeStep, dt, maxSubSteps);
	  }
//	  console.log("Sphere z position: " + rigidBody.);
	  lastTime = time;
	})();
}
