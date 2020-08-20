var cube;

function cube() {
	var geometry = new THREE.BoxGeometry()
	var loader = new THREE.CubeTextureLoader();
	loader.setPath( 'img/cube/' );

	var textureCube = loader.load( [
		'px.png', 'nx.png',
		'py.png', 'ny.png',
		'pz.png', 'nz.png'
	] );


	for ( var i = 0; i < geometry.faces.length; i ++ ) {
		geometry.faces[ i ].color.setHex( Math.random() * 0xffffff );
	}

	var material = new THREE.MeshBasicMaterial( { color: 0xffffff, vertexColors: true } );


	/*var material = new THREE.MeshBasicMaterial( 
		{ color: 0xffffff, envMap: textureCube }
	)*/
	
	cube = new THREE.Mesh( geometry, material )
	scene.add( cube ); 
	camera.position.z = 5; 
}
