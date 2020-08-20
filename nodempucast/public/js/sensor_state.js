
sensorState = {}
var recurseCount = 0;

var Min = 0;
var Max = 0;

function recurse() {	
	$.ajax({
	  url: "/sensor",
	}).done(function( data ) {
		if ( console && console.log ) {
		  recurseCount ++
		  sensorState = data
		  setTimeout(function() {
			  recurse()
		  }, 100)
		}
	  });
}
