
void SpawnPixelPlane() {
	
	
	GLfloat px_sqare[] = {
		// x    y      z
		-1.0f, -1.0f, +1.0f,
		+1.0f, -1.0f, +1.0f,
		-1.0f, +1.0f, +1.0f,
		+1.0f, +1.0f, +1.0f,
	};
	
	GLfloat defaultNormal[] = {
		// x    y      z
		+0.0f, +0.0f, +1.0f,
		+0.0f, +0.0f, +1.0f,
		+0.0f, +0.0f, +1.0f,
		+0.0f, +0.0f, +1.0f,
	};
	
	
	
	memoryAllocationSize = sizeof(px_sqare) * width * height;
	
	pVertices = (GLfloat*)malloc(memoryAllocationSize);
	pColors = (GLfloat*)malloc(memoryAllocationSize);
	pNormals = (GLfloat*)malloc(memoryAllocationSize);
	
	printf("memspace %i\n", sizeof(px_sqare) * width * height);
	
	// fill with vertex data
	unsigned int filler = 0;
	char XYZloop = 1;
	for (int y = 0; y < height; y ++) {
		for (int x = 0; x < width; x ++) {
			for (unsigned short int px = 0; px < (unsigned short int)sizeof(px_sqare)/sizeof(float); px ++) {
				//printf("%i\n", (int)sizeof(px_sqare)/sizeof(float));
				//printf("pVertice[%i] = px_sqare[%i] = %f\n", filler, px, px_sqare[px]);
				
				pNormals[filler] = defaultNormal[px];
				pVertices[filler] = px_sqare[px];
				pColors[filler] = 0.0f;
				
				if (XYZloop == 1) { // X
					pVertices[filler] += 2.0f * x;
					
					//printf("lets add 2.0f * %i\n", x);
				}
				if (XYZloop == 2) { // y
					pVertices[filler] += 2.0f * y;
				}
				if (XYZloop == 3) {
					XYZloop=0;
				} 
				XYZloop ++;
				filler ++;
			}
		} 
	}	
}
