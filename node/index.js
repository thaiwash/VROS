

var fs = require("fs")
const { createCanvas, loadImage } = require('canvas')
const canvas = createCanvas(128, 128)
const ctx = canvas.getContext('2d')

var self = {};

var ratio = 16/9.0;

var canvasWidth = 128;
var canvasHeight = 128;

var window = {
    innerWidth: canvasWidth,
    innerHeight: canvasHeight

};
var document = {
    createElement: function(name) {
        if (name == "canvas") {
            //return new Canvas(canvasWidth, canvasHeight);
        }
        return canvas;
    },
    createElementNS: function(name) {
        return canvas;
    }
};

var THREE = require("./threejs/three.js")
eval(fs.readFileSync("threejs/additionalRenderers.js").toString())
eval(fs.readFileSync("threejs/SceneUtils.js").toString())

module.exports = function() {
	return THREE;
}
