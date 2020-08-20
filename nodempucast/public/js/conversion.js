const MAX_ANGULAR_VELOCITY = 15
const MAX_VELOCITY = 100

function gyroToThreejsRotation(value)  {
	//value += 255
	var divider = 1/255
	return (divider * (Math.PI * 2)) * value
}

function gyroToAngularVelocity(value)  {
	//value += 255
	var divider = 1/255
	return (divider * (MAX_ANGULAR_VELOCITY)) * value
}

function accelToVelocity(value)  {
	//value += 255
	var divider = 1/255
	return (divider * (MAX_VELOCITY)) * value
}
