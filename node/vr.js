// this is a small hack for three.js to work on node.js
// this example demonstrates how one could create a frame and store it to a png file
const { exec } = require("child_process");
var put_pixel = require("put_pixel")
                                                                                                                
var screen_width = 1440;
var screen_height = 2560;

var fs = require("fs")
const { createCanvas, loadImage } = require('canvas')

var canvas = createCanvas(screen_width, screen_height/2)
var context = canvas.getContext('2d')

var self = {};

var ratio = 16/9.0;

var canvasWidth = screen_width;
var canvasHeight = screen_height;

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

const ImageDataURI = require('image-data-uri');
var THREE = require("./threejs/three.js")
eval(fs.readFileSync("threejs/additionalRenderers.js").toString())
eval(fs.readFileSync("threejs/SceneUtils.js").toString())

const EventEmitter = require('events');

//var OS = new ShereOS()

class ThreeClient extends EventEmitter {
    constructor() {
        super()
        var self = this


        self.loaded = false

        this.bgColor = '#282c34'
        this.textColor = '#fff'
        this.tildeColor = '#0000ff'
        this.selectColor = '#ffffff'

        this.width = screen_height/2
        this.height = screen_width



        this.renderer = new THREE.CanvasRenderer();
        this.renderer.setSize(this.width, this.height);

        this.camera = new THREE.PerspectiveCamera(75, this.width / this.height, 0.001, 3000);
        this.camera.position.z = 5;
        this.camera.position.x = 0.1;

        this.camera2 = new THREE.PerspectiveCamera(75, this.width / this.height, 0.001, 3000);
        this.camera2.position.z = 5;
        this.camera2.position.x = -0.1;




        this.scene = new THREE.Scene();

        this.scene.background = new THREE.Color( 0xECF8FF );
        this.scene.add( new THREE.HemisphereLight( 0x606060, 0x404040 ) );
        this.light = new THREE.DirectionalLight( 0xffffff );
        this.light.position.set( 1, 1, 1 ).normalize();
        this.scene.add( this.light );
        //console.log(this.scene.children)


        this.updated = false
        //var geometry = new THREE.BoxGeometry( 1, 1, 1 );
        //var material = new THREE.MeshBasicMaterial( { color: 0x00ff00 } );
        
        var geometry = new THREE.BoxGeometry( 1, 1, 1 );
        
        for ( var i = 0; i < geometry.faces.length; i ++ ) {
            geometry.faces[ i ].color.setHex( 0xFF00FF );
        }
    
        var material = new THREE.MeshBasicMaterial( { color: 0xFF0000, vertexColors: true } );
        this.cube = new THREE.Mesh( geometry, material );
        this.scene.add( this.cube );

        
    }




    createLeftEyeImage() {
        this.cube.rotation.x += 0.1;
        this.cube.rotation.y += 0.1;
    
        this.renderer.render(this.scene, this.camera);
        //'use strict';


        //const dataURI = 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAMAAAACCAIAAAFlEcHbAAAAB3RJTUUH1gMWFjk7nUWcXQAAAAlwSFlzAABOIAAATiABFn2Z3gAAAARnQU1BAACxjwv8YQUAAAAeSURBVHjaY7h79y7DhAkTGIA04/Tp0xkYGJ49ewYAgYwLV/R7bDQAAAAASUVORK5CYII=';
        const fileName = '/var/memfs/left_frame.png';

        ImageDataURI.outputFile(this.renderer.domElement.toDataURL(), fileName);

        //console.log(this.renderer.domElement.toDataURL())
        //var data = this.renderer.domElement.toDataURL().substr("data:image/png;base64,".length)
        //const buffer = canvas.toBuffer('image/png')
        //fs.writeFileSync('./image.png', buffer)
        
        //console.log(data)
        //var buf = new Buffer(data, 'base64');
        //fs.writeFile('image.png', buf);
        //return this.renderer.domElement.toDataURL().substr("data:image/png;base64,".length);
        
    }

    createRightEyeImage() {
        this.renderer.render(this.scene, this.camera2)
	var fileName = '/var/memfs/right_frame.png';

        ImageDataURI.outputFile(this.renderer.domElement.toDataURL(), fileName)

    }
}

function barrel_distort() {
	exec('convert /var/memfs/left_frame.png -virtual-pixel gray -distort Barrel "0.2 0.0 0.0 1.0" /var/memfs/distorted_left_frame.png', (error, stdout, stderr) => {
	    if (error) {
		console.log(`error: ${error.message}`);
		return;
	    }
	    if (stderr) {
		console.log(`stderr: ${stderr}`);
		return;
	    }
	});

	exec('convert /var/memfs/right_frame.png -virtual-pixel gray -distort Barrel "0.2 0.0 0.0 1.0" /var/memfs/distorted_right_frame.png', (error, stdout, stderr) => {
	    if (error) {
		console.log(`error: ${error.message}`);
		return;
	    }
	    if (stderr) {
		console.log(`stderr: ${stderr}`);
		return;
	    }
	});
}


function framebuffer() {
	loadImage('/var/memfs/distorted_right_frame.png').then((rightImg) => {
		loadImage('/var/memfs/distorted_left_frame.png').then((leftImg) => {
			var right = rightImage.getImageData(0, 0, screen_width, screen_height / 2)
        	var left = leftImage.getImageData(0, 0, screen_width, screen_height / 2)

            var i = 0
            for (;i < screen_width * (screen_height / 2); i ++) {
                    var iter = i*3
                    var iter2 = i*4
                    frame_buffer[iter] = left.data[iter2];
                    frame_buffer[iter+1] = left.data[iter2+1];
                    frame_buffer[iter+2] = left.data[iter2+2];
            }
            
            for (;i < screen_width * screen_height; i ++) {
                    var iter = i*3
                    var iter2 = i*4
                    frame_buffer[iter] = right.data[iter2];
                    frame_buffer[iter+1] = right.data[iter2+1];
                    frame_buffer[iter+2] = right.data[iter2+2];
            }
                
                
            put_pixel(frame_buffer)
		})
	})
}

var THREEClient = new ThreeClient();

setInterval(function() {
    THREEClient.createLeftEyeImage()
    THREEClient.createRightEyeImage()
    barrel_distort()
    framebuffer()
    console.log("updated");
}, 1000)
