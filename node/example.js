// this is a small hack for three.js to work on node.js
// this example demonstrates how one could create a frame and store it to a png file


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

        this.width = 128
        this.height = 128



        this.renderer = new THREE.CanvasRenderer();
        this.renderer.setSize(this.width, this.height);

        this.camera = new THREE.PerspectiveCamera(75, this.width / this.height, 0.001, 3000);
        this.camera.position.z = 5;




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




    getTexture() {
        this.cube.rotation.x += 0.1;
        this.cube.rotation.y += 0.1;
    
        this.renderer.render(this.scene, this.camera);
        'use strict';

        const ImageDataURI = require('image-data-uri');

        const dataURI = 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAMAAAACCAIAAAFlEcHbAAAAB3RJTUUH1gMWFjk7nUWcXQAAAAlwSFlzAABOIAAATiABFn2Z3gAAAARnQU1BAACxjwv8YQUAAAAeSURBVHjaY7h79y7DhAkTGIA04/Tp0xkYGJ49ewYAgYwLV/R7bDQAAAAASUVORK5CYII=';
        const fileName = 'example.png';

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
}

var THREEClient = new ThreeClient();

setInterval(function() {
    THREEClient.getTexture();
    console.log("updated");
}, 1000)
