const binding = require('bindings')('array_buffer_to_native');


var pixel_array_size = 1440*2560*3
const array = new Int32Array(pixel_array_size)

function clear_screen(color) {
	for (var i = 0; i < pixel_array_size; i ++) {
		array[i] = color
	}

	binding.AcceptArrayBuffer(array.buffer)
}



var color = 0
var time = (new Date()).getTime()

for (var i = 0; i < 100; i ++) {
	if (color == 0) {
		color = 255
	} else {
		color = 0
	}
	clear_screen(color)
}
console.log("How many millsecs does it take to draw 100 frames of black and white flicker")
console.log(((new Date()).getTime() - time)+"\n")


