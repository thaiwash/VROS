const binding = require('bindings')('array_buffer_to_native');

var pixel_array_size = 1440*2560*3
const array = new Int32Array(pixel_array_size);


module.exports = function(array) {
	binding.AcceptArrayBuffer(array.buffer);
}
