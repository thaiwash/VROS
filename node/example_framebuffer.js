// this is a small hack for three.js to work on node.js
// this example demonstrates how one could create a frame and store it to a png file


var fs = require("fs")
const { createCanvas, loadImage } = require('canvas')
var put_pixel = require("put_pixel")


var self = {};

var ratio = 16/9.0;

var canvasWidth = 1440;
var canvasHeight = 2560;

const canvas = createCanvas(canvasWidth, canvasHeight)
const ctx = canvas.getContext('2d')

var pixel_array_size = canvasWidth*canvasHeight*3
const frame_buffer = new Int32Array(pixel_array_size);

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

// view-source:http://www.decarpentier.nl/downloads/lensdistortion-webgl/lensdistortion-webgl.html
eval(fs.readFileSync("threejs/CopyShader.js").toString())
eval(fs.readFileSync("threejs/EffectComposer.js").toString())
eval(fs.readFileSync("threejs/MaskPass.js").toString())
eval(fs.readFileSync("threejs/RenderPass.js").toString())
eval(fs.readFileSync("threejs/ShaderPass.js").toString())

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

        this.width = canvasWidth
        this.height = canvasHeight



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

        
        this.composer = new THREE.EffectComposer( this.renderer );
        this.composer.addPass( new THREE.RenderPass( this.scene, this.camera ) );

        var effect = new THREE.ShaderPass( this.getDistortionShaderDefinition() );
        this.composer.addPass( effect );
        effect.renderToScreen = true;
        
        this.setupDistortionEffect(effect)
    }			
    
    setupDistortionEffect(effect)
    {
        var self=this
        var guiParameters = {
            horizontalFOV:		140,
            strength: 			1,
            cylindricalRatio:	2,
        };				

        var updateDistortionEffect = function( ) {
            
            var height = Math.tan(THREE.Math.degToRad(guiParameters.horizontalFOV) / 2) / self.camera.aspect;

            self.camera.fov = Math.atan(height) * 2 * 180 / 3.1415926535;
            self.camera.updateProjectionMatrix();
            
            effect.uniforms[ "strength" ].value = guiParameters.strength;
            effect.uniforms[ "height" ].value = self.height;
            effect.uniforms[ "aspectRatio" ].value = self.camera.aspect;
            effect.uniforms[ "cylindricalRatio" ].value = guiParameters.cylindricalRatio;
        };		
        
        updateDistortionEffect();
    }
    getDistortionShaderDefinition()
    {
        return {
        
            uniforms: {
                "tDiffuse": 		{ type: "t", value: null },
                "strength": 		{ type: "f", value: 0 },
                "height": 			{ type: "f", value: 1 },
                "aspectRatio":		{ type: "f", value: 1 },
                "cylindricalRatio": { type: "f", value: 1 }
            },

            vertexShader: [
                "uniform float strength;",          // s: 0 = perspective, 1 = stereographic
                "uniform float height;",            // h: tan(verticalFOVInRadians / 2)
                "uniform float aspectRatio;",       // a: screenWidth / screenHeight
                "uniform float cylindricalRatio;",  // c: cylindrical distortion ratio. 1 = spherical
                 
                "varying vec3 vUV;",                // output to interpolate over screen
                "varying vec2 vUVDot;",             // output to interpolate over screen
                 
                "void main() {",
                    "gl_Position = projectionMatrix * (modelViewMatrix * vec4(position, 1.0));",
                 
                    "float scaledHeight = strength * height;",
                    "float cylAspectRatio = aspectRatio * cylindricalRatio;",
                    "float aspectDiagSq = aspectRatio * aspectRatio + 1.0;",
                    "float diagSq = scaledHeight * scaledHeight * aspectDiagSq;",
                    "vec2 signedUV = (2.0 * uv + vec2(-1.0, -1.0));",
                 
                    "float z = 0.5 * sqrt(diagSq + 1.0) + 0.5;",
                    "float ny = (z - 1.0) / (cylAspectRatio * cylAspectRatio + 1.0);",
                 
                    "vUVDot = sqrt(ny) * vec2(cylAspectRatio, 1.0) * signedUV;",
                    "vUV = vec3(0.5, 0.5, 1.0) * z + vec3(-0.5, -0.5, 0.0);",
                    "vUV.xy += uv;",
                "}"
            ].join("\n"),
            
            fragmentShader: [
                "uniform sampler2D tDiffuse;",      // sampler of rendered scene�s render target
                "varying vec3 vUV;",                // interpolated vertex output data
                "varying vec2 vUVDot;",             // interpolated vertex output data

                "void main() {",
                    "vec3 uv = dot(vUVDot, vUVDot) * vec3(-0.5, -0.5, -1.0) + vUV;",
                    "gl_FragColor = texture2DProj(tDiffuse, uv);",
                "}"
            ].join("\n")

        };
    }

//https://glitch.com/edit/#!/pyrite-paint?path=public%2Fclient.js%3A15%3A48
    update_framebufferV2() {
        this.cube.rotation.x += 0.1;
        this.cube.rotation.y += 0.1;
    
        this.composer.render();
        this.renderer.render(this.scene, this.camera);
            
        //var data = this.renderer.domElement.toDataURL()
        //data = data.slice(22)
            
        var imgd = ctx.getImageData(0, 0, canvasWidth, canvasHeight);
        //var imgd = this.renderer.domElement.getImageData(0, 0, canvasWidth, canvasHeight);
        
        
        for (var i = 0; i < canvasWidth * canvasHeight; i ++) {
                var iter = i*3
                var iter2 = i*4
                frame_buffer[iter] = imgd.data[iter2];
                frame_buffer[iter+1] = imgd.data[iter2+1];
                frame_buffer[iter+2] = imgd.data[iter2+2];
        }
            
        put_pixel(frame_buffer)
    }



    update_framebuffer() {
        this.cube.rotation.x += 0.1;
        this.cube.rotation.y += 0.1;
    
        this.renderer.render(this.scene, this.camera);
        
        var imgd = ctx.getImageData(0, 0, canvasWidth, canvasHeight);
        //var imgd = this.renderer.domElement.getImageData(0, 0, canvasWidth, canvasHeight);
        for (var i = 0; i < canvasWidth * canvasHeight *3; i ++) {
                frame_buffer[i] = imgd.data[i];
        }
            
        put_pixel(frame_buffer)
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

while(true) {
        
    THREEClient.update_framebufferV2();
}
/*
setInterval(function() {
    THREEClient.update_framebufferV2();
    //THREEClient.getTexture();
    //console.log("updated");
}, 10)
*/
