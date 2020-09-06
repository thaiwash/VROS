var fs = require("fs")

var self = {};

var ratio = 16/9.0;

var canvasWidth = 500;
var canvasHeight = 500;

var window = {
    innerWidth: canvasWidth,
    innerHeight: canvasHeight

};
var document = {
    createElement: function(name) {
        if (name == "canvas") {
            //return new Canvas(canvasWidth, canvasHeight);
        }
        var Canvas = require('canvas')
        return new Canvas(500,500)
    },
    createElementNS: function(name) {

        var Canvas = require('canvas')
        return new Canvas(500,500)
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
        this.socket_url = "http://51.38.185.65:9774"
        this.appId = 667

        this.socket = require('socket.io-client')(this.socket_url)
        this.registerEvents()

        self.loaded = false

        this.bgColor = '#282c34'
        this.textColor = '#fff'
        this.tildeColor = '#0000ff'
        this.selectColor = '#ffffff'

        this.width = 500
        this.height = 500



        this.renderer = new THREE.CanvasRenderer();
        this.renderer.setSize(this.width, this.height);

        this.camera = new THREE.PerspectiveCamera(75, this.width / this.height, 0.001, 3000);
        this.camera.position.z = 2;




        this.scene = new THREE.Scene();

        this.scene.background = new THREE.Color( 0xECF8FF );
        this.scene.add( new THREE.HemisphereLight( 0x606060, 0x404040 ) );
        this.light = new THREE.DirectionalLight( 0xffffff );
        this.light.position.set( 1, 1, 1 ).normalize();
        this.scene.add( this.light );
        //console.log(this.scene.children)


        this.updated = false
        /*
        var geometry = new THREE.SphereGeometry( 0.1, 32, 32 );
        var material = new THREE.MeshBasicMaterial( {color: 0xFF0000} );
        this.sphere = new THREE.Mesh( geometry, material );
        this.scene.add( this.sphere );
        */
    }

    registerEvents() {
        /*
        socket.On("spirit entered", EnterSpirit);
        socket.On("spirit moved", MoveSpirit);
        socket.On("spirit exits", ExitSpirit);
        socket.On("object created", CreateObject);
        socket.On("object changed", ChangeObject);
        socket.On("object destroyed", DestroyObject);
        socket.On("texture updated", TextureUpdateEvent);
        socket.On("set clipboard", SetClipboardEvent);
        socket.On("get clipboard", GetClipboardEvent);
        socket.On("tts", TTSEvent);*/

        var self = this
        this.socket.on('connect', function(){});
        this.socket.on('disconnect', function(){});

        this.socket.on('welcome', function(){
            console.log("got connection")
            self.socket.emit("spirit entering matrix", {"user_id": self.appId});
        })

        this.socket.on("load matrix", function (data) {
            self.serverData = data
            self.loaded = true
            //console.log(self.serverData.spirits)
            self.emit('load');
        })

        this.socket.on("spirit moved", function(data){
            //console.log("moved")
            //    console.log(data)
            return
            if (self.loaded) {
                for (var i = 0; i < self.serverData.spirits.length; i++) {
                    //console.log(self.serverData.spirits[i].user_id)
                    if (self.serverData.spirits[i].user_id == data.user_id) {
                        self.serverData.spirits[i] = data;
                        //console.log("moved")
                        //console.log(data)
                    }
                }
            }
        })
        this.socket.on("controller event", function(data){
            self.controllerEventData = data;
            self.emit('controller event');
            self.controllerEvent(data)
        })
        this.socket.on("object changed", function(data){
            self.objectChangedEventData = data;
            self.emit('object changed');
            self.objectChangedEvent(data)
        })
    }


    exists(id) {
        if (typeof this.serverData === "undefined") {
            return
        }
        var exists = false;
        for (var i = 0; i < this.serverData.objects.length; i++) {
            if (this.serverData.objects[i].object_id == id) {
                exists = true;
            }
        }
        return exists
    }

    getObjectById(id) {
        if (!this.loaded) {
            return null
        }
        for (var i = 0; i < this.serverData.objects.length; i++) {
            if (this.serverData.objects[i].object_id == id) {
                return this.serverData.objects[i];
            }
        }
        return null
    }

    getSpiritById(id) {
        id = id +""
        if (!this.loaded) {
            return
        }
        var exists = false;
        for (var i = 0; i < this.serverData.spirits.length; i++) {
            if (this.serverData.spirits[i].user_id == id) {
                return this.serverData.spirits[i]
            }
        }
        return null
    }

    extractPosition(object) {
        if (typeof object.posX === "undefined"
        || typeof object.posY === "undefined"
        || typeof object.posZ === "undefined") {
            return null
        }
        return new THREE.Vector3(
            parseFloat(object.posX),
            parseFloat(object.posY),
            parseFloat(object.posZ)
        )
    }

    extractRotation(object) {
        if (typeof object.rotX === "undefined"
        || typeof object.rotY === "undefined"
        || typeof object.rotZ === "undefined"
        || typeof object.rotW === "undefined") {
            return null
        }
        var quaternion = new THREE.Quaternion(
            parseFloat(object.rotX),
            parseFloat(object.rotY),
            parseFloat(object.rotZ),
            parseFloat(object.rotW)
        )
        var vector = new THREE.Vector3(1, 0, 0);
        vector.order = 'XYZ'
        vector.applyQuaternion( quaternion, 'XYZW');

        return vector
    }

    extractScale(object) {
        if (typeof object.scaleX === "undefined"
        || typeof object.scaleY === "undefined"
        || typeof object.scaleZ === "undefined") {
            return null
        }
        return new THREE.Vector3(
            parseFloat(object.scaleX),
            parseFloat(object.scaleY),
            parseFloat(object.scaleZ)
        )
    }

    toThree(object) {
        if (typeof object == "undefined") {
            return null
        }
        var obj = object
        obj.position = this.extractPosition(object)
        obj.rotation = this.extractRotation(object)
        obj.scale = this.extractScale(object)
        var mesh = null
        var id = 0;

        if (typeof object.object_id !== "undefined") {
            id = object.object_id
            mesh = this.scene.getObjectByName(object.object_id, true)
        } else {
            //console.warn("object_id missing")
            return obj
        }

                    //console.log(object.object_id)
        if (mesh == null) {
            // create 3 primitive types
                //console.log(object.type)
            if (object.type == "sphere") {
                //console.log(object.type)
                var geometry = new THREE.SphereGeometry( obj.scale.x, 32, 32 );
                var material = new THREE.MeshBasicMaterial( {color: 0x00FF00} );
                mesh = new THREE.Mesh( geometry, material );
                mesh.name = id;
                THREEClient.scene.add( mesh );
            }
            if (object.type == "cube") {
                var material = new THREE.MeshLambertMaterial();
                var geom = new THREE.CubeGeometry(
                    obj.scale.x,
                    obj.scale.y,
                    obj.scale.z
                );
                mesh = new THREE.Mesh(geom, material);
                mesh.name = id;
                this.scene.add(mesh)
            }
            var self = this

            if (mesh == null) {
                return obj
            } else {
                this.socket.on("object changed", function(data) {
                    //console.log(data)
                    if (id == data.object_id) {
                        self.toThree(data)
                    }
                })
            }
        }

        mesh.position.copy(obj.position)
        mesh.rotation.setFromVector3( obj.rotation, 'XYZ' );
        mesh.scale.set(
            obj.scale.x,
            obj.scale.y,
            obj.scale.z
        );
        mesh.rawData = object
        return mesh;
    }

    heartBeat() {
        for (var i = 0; i < this.scene.children.length; i ++) {
            var child = this.scene.children[i]
            if (this.exists(child.name)) {
                var rawData = this.fromThree(child)
                if (JSON.stringify(child.rawData) != JSON.stringify(rawData)) {
                    line = "be changed object"
                    this.socket.emit("object changed", rawData);
                }
                //child.lastRawData = child.rawData
                child.rawData = rawData
            }
        }
    }

    sync(rawData) {
        // raw data or threejs Mesh
        //console.log(rawData)

        if (!this.exists(rawData.object_id)) {
            this.socket.emit("object created", rawData)
        }

        var self = this
        var object = this.toThree(rawData)
        var existing = this.scene.getObjectByName(rawData.object_id)

    }

    say(string) {
        this.socket.emit("tts", {"say": string})
    }

    getUserByEmail(email) {
        for (var i = 0; i < this.serverData.spirits.length; i++) {
            //console.log(this.serverData.spirits[i].user_id)
            if (this.serverData.spirits[i].registeredUser == email) {
                return this.serverData.spirits[i];
            }
        }
    }

    userEmail(id) {
        if (typeof this.serverData === "undefined") {
            return
        }
        //onsole.log(this.serverData.spirits)
        for (var i = 0; i < this.serverData.spirits.length; i++) {
            //console.log(this.serverData.spirits[i].user_id)
            if (this.serverData.spirits[i].user_id == id) {
                return this.serverData.spirits[i].registeredUser;
            }
        }
    }

    controllerEvent(data) {
        //console.log(data)
    }

    objectChangedEvent(data) {
    }

    // conver three to raw data
    fromThree(object, pos = true, rot = true, scale = true) {
        var rawData = {}
        if (pos) {
            rawData.posX = ""+object.position.x
            rawData.posY = ""+object.position.y
            rawData.posZ = ""+object.position.z
        }
        if (rot) {
            rawData.rotX = ""+object.quaternion._x
            rawData.rotY = ""+object.quaternion._y
            rawData.rotZ = ""+object.quaternion._z
            rawData.rotW = ""+object.quaternion._w
        }
        if (scale) {
            rawData.scaleX = ""+object.scale.x
            rawData.scaleY = ""+object.scale.y
            rawData.scaleZ = ""+object.scale.z
        }
        rawData.object_id = object.name
        return rawData
    }
/*
    update(self) {
        var data = {
            "object_id": this.objectId,
            "texture": this.getTexture()
        }
        //console.log(data)
        self.socket.emit("texture update", data)

        if (typeof self.avrose.serverData !== "undefined") {
            if (!self.updated) {
                self.updated = true
            }
        }
    }*/


    getTexture() {
        this.renderer.render(this.scene, this.camera);
        return this.renderer.domElement.toDataURL().substr("data:image/png;base64,".length);
    }
}

var THREEClient = new ThreeClient();
