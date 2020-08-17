/*
 * Copyright (c) 2017 Rob Clark <rclark@redhat.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "esUtil.h"


// Data read from the header of the BMP file
unsigned char header[54]; // Each BMP file begins by a 54-bytes header
unsigned int dataPos;     // Position in the file where the actual data begins
unsigned int width, height;
unsigned int imageSize;   // = width*height*3
// Actual RGB data
unsigned char * data;
GLuint Texture;

static struct {
	struct egl egl;

	GLfloat aspect;

	GLuint program;
	GLint modelviewmatrix, modelviewprojectionmatrix, normalmatrix;
	GLuint vbo;
	GLuint positionsoffset, colorsoffset, normalsoffset;
} gl;


	
static const GLfloat vVertices[] = {
		// first
		-1.0f, -1.0f, +1.0f,
		+1.0f, -1.0f, +1.0f,
		-1.0f, +1.0f, +1.0f,
		+1.0f, +1.0f, +1.0f,
		
		// second
		-1.0f + 2.0f, -1.0f, +1.0f,
		+1.0f + 2.0f, -1.0f, +1.0f,
		-1.0f + 2.0f, +1.0f, +1.0f,
		+1.0f + 2.0f, +1.0f, +1.0f,
};


/*
static const GLfloat vVertices[] = {
		// front
		-1.0f, -1.0f, +1.0f,
		+1.0f, -1.0f, +1.0f,
		-1.0f, +1.0f, +1.0f,
		+1.0f, +1.0f, +1.0f,
		// back
		+1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		+1.0f, +1.0f, -1.0f,
		-1.0f, +1.0f, -1.0f,
		// right
		+1.0f, -1.0f, +1.0f,
		+1.0f, -1.0f, -1.0f,
		+1.0f, +1.0f, +1.0f,
		+1.0f, +1.0f, -1.0f,
		// left
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, +1.0f,
		-1.0f, +1.0f, -1.0f,
		-1.0f, +1.0f, +1.0f,
		// top
		-1.0f, +1.0f, +1.0f,
		+1.0f, +1.0f, +1.0f,
		-1.0f, +1.0f, -1.0f,
		+1.0f, +1.0f, -1.0f,
		// bottom
		-1.0f, -1.0f, -1.0f,
		+1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, +1.0f,
		+1.0f, -1.0f, +1.0f,
};
*/

static const GLfloat vColors[] = {
		// first
		0.0f,  0.0f,  0.0f, // black
		0.0f,  0.0f,  0.0f,
		0.0f,  0.0f,  0.0f,
		0.0f,  0.0f,  0.0f,
		// second
		1.0f,  1.0f,  1.0f, // white
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
};

static const GLfloat vNormals[] = {
		// first
		+0.0f, +0.0f, +1.0f, // forward
		+0.0f, +0.0f, +1.0f, // forward
		+0.0f, +0.0f, +1.0f, // forward
		+0.0f, +0.0f, +1.0f, // forward
		// second
		+0.0f, +0.0f, +1.0f, // forward
		+0.0f, +0.0f, +1.0f, // forward
		+0.0f, +0.0f, +1.0f, // forward
		+0.0f, +0.0f, +1.0f, // forward
};

static const char *vertex_shader_source =
		"uniform mat4 modelviewMatrix;      \n"
		"uniform mat4 modelviewprojectionMatrix;\n"
		"uniform mat3 normalMatrix;         \n"
		"                                   \n"
		"attribute vec4 in_position;        \n"
		"attribute vec3 in_normal;          \n"
		"attribute vec4 in_color;           \n"
		"\n"
		"vec4 lightSource = vec4(2.0, 2.0, 20.0, 0.0);\n"
		"                                   \n"
		"varying vec4 vVaryingColor;        \n"
		"                                   \n"
		"void main()                        \n"
		"{                                  \n"
		"    gl_Position = modelviewprojectionMatrix * in_position;\n"
		"    vec3 vEyeNormal = normalMatrix * in_normal;\n"
		"    vec4 vPosition4 = modelviewMatrix * in_position;\n"
		"    vec3 vPosition3 = vPosition4.xyz / vPosition4.w;\n"
		"    vec3 vLightDir = normalize(lightSource.xyz - vPosition3);\n"
		"    float diff = max(0.0, dot(vEyeNormal, vLightDir));\n"
		"    vVaryingColor = vec4(diff * in_color.rgb, 1.0);\n"
		"}                                  \n";

static const char *fragment_shader_source =
		"precision mediump float;           \n"
		"                                   \n"
		"varying vec4 vVaryingColor;        \n"
		"                                   \n"
		"void main()                        \n"
		"{                                  \n"
		"    gl_FragColor = vVaryingColor;   \n"
//		"    gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);  \n"
		"}                                  \n";


GLfloat* pVertices;
GLfloat* pColors;
GLfloat* pNormals;
unsigned short int memoryAllocationSize;

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
	
	
	
	unsigned short int width = 2;
	unsigned short int height = 1;
	
	memoryAllocationSize = sizeof(px_sqare) * width * height;
	
	pVertices = (GLfloat*)malloc(sizeof(px_sqare) * width * height);
	printf("memspace %i\n", sizeof(px_sqare) * width * height);
	
	// fill with vertex data
	unsigned int filler = 0;
	char XYZloop = 1;
	for (int y = 0; y < height; y ++) {
		for (int x = 0; x < width; x ++) {
			for (unsigned short int px = 0; px < (unsigned short int)sizeof(px_sqare)/sizeof(float); px ++) {
				printf("%i\n", (int)sizeof(px_sqare)/sizeof(float));
				printf("pVertice[%i] = px_sqare[%i] = %f\n", filler, px, px_sqare[px]);
				
				pNormals[filler] = defaultNormal[px];
				pVertices[filler] = px_sqare[px];
				pColors[filler] = 0.0f;
				
				if (XYZloop == 1) { // X
					pVertices[filler] += 2.0f * x;
					
					printf("lets add 2.0f * %i\n", x);
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




static void draw_cube_smooth()
{
    
	ESMatrix modelview;

	/* clear the color buffer */
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	
	
	/*bmp load dont work yet
	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	*/
	

	esMatrixLoadIdentity(&modelview);
	esTranslate(&modelview, 0.0f, 0.0f, -8.0f);
	//esRotate(&modelview, 45.0f + (0.25f * i), 1.0f, 0.0f, 0.0f);
	//esRotate(&modelview, 45.0f - (0.5f * i), 0.0f, 1.0f, 0.0f);
	//esRotate(&modelview, 10.0f + (0.15f * i), 0.0f, 0.0f, 1.0f);

	ESMatrix projection;
	esMatrixLoadIdentity(&projection);
	esFrustum(&projection, -2.8f, +2.8f, -2.8f * gl.aspect, +2.8f * gl.aspect, 6.0f, 10.0f);

	ESMatrix modelviewprojection;
	esMatrixLoadIdentity(&modelviewprojection);
	esMatrixMultiply(&modelviewprojection, &modelview, &projection);

	float normal[9];
	normal[0] = modelview.m[0][0];
	normal[1] = modelview.m[0][1];
	normal[2] = modelview.m[0][2];
	normal[3] = modelview.m[1][0];
	normal[4] = modelview.m[1][1];
	normal[5] = modelview.m[1][2];
	normal[6] = modelview.m[2][0];
	normal[7] = modelview.m[2][1];
	normal[8] = modelview.m[2][2];

	glUniformMatrix4fv(gl.modelviewmatrix, 1, GL_FALSE, &modelview.m[0][0]);
	glUniformMatrix4fv(gl.modelviewprojectionmatrix, 1, GL_FALSE, &modelviewprojection.m[0][0]);
	glUniformMatrix3fv(gl.normalmatrix, 1, GL_FALSE, normal);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 12, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 16, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 20, 4);


}


/*
GLuint loadBMP_custom(const char * imagepath) {
	// Open the file
	FILE * file = fopen(imagepath,"rb");
	if (!file){printf("Image could not be opened\n"); return 0;}

	if ( fread(header, 1, 54, file)!=54 ){ // If not 54 bytes read : problem
		printf("Not a correct BMP file\n");
		return false;
	}
	
	if ( header[0]!='B' || header[1]!='M' ){
		printf("Not a correct BMP file\n");
		return 0;
	}
	
	// Read ints from the byte array
	dataPos    = *(int*)&(header[0x0A]);
	imageSize  = *(int*)&(header[0x22]);
	width      = *(int*)&(header[0x12]);
	height     = *(int*)&(header[0x16]);
	
	// Some BMP files are misformatted, guess missing information
	if (imageSize==0)    imageSize=width*height*3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos==0)      dataPos=54; // The BMP header is done that way

	// Create a buffer
	data = new unsigned char [imageSize];

	// Read the actual data from the file into the buffer
	fread(data,1,imageSize,file);

	//Everything is in memory now, the file can be closed
	fclose(file);
}



void init_bmp() {
	Texture = loadBMP_custom("./chessboard.bmp");
}
*/

const struct egl * init_cube_smooth(const struct gbm *gbm, int samples)
{
	//init_bmp();
	SpawnPixelPlane();
	for (int i = 0; i < 24; i ++) {
		printf("hello i:%i a:%f b:%f\n", i, pVertices[i], vVertices[i]);
	}
	//exit(0);
	int ret;

	ret = init_egl(&gl.egl, gbm, samples);
	if (ret)
		return NULL;

	gl.aspect = (GLfloat)(gbm->height) / (GLfloat)(gbm->width);

	ret = create_program(vertex_shader_source, fragment_shader_source);
	if (ret < 0)
		return NULL;

	gl.program = ret;

	glBindAttribLocation(gl.program, 0, "in_position");
	glBindAttribLocation(gl.program, 1, "in_normal");
	glBindAttribLocation(gl.program, 2, "in_color");

	ret = link_program(gl.program);
	if (ret)
		return NULL;

	glUseProgram(gl.program);

	gl.modelviewmatrix = glGetUniformLocation(gl.program, "modelviewMatrix");
	gl.modelviewprojectionmatrix = glGetUniformLocation(gl.program, "modelviewprojectionMatrix");
	gl.normalmatrix = glGetUniformLocation(gl.program, "normalMatrix");

	glViewport(0, 0, gbm->width, gbm->height);
	glEnable(GL_CULL_FACE);

	gl.positionsoffset = 0;
	gl.colorsoffset = memoryAllocationSize;
	gl.normalsoffset = memoryAllocationSize * 2;
	glGenBuffers(1, &gl.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, gl.vbo);
	glBufferData(GL_ARRAY_BUFFER, memoryAllocationSize *3, 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, gl.positionsoffset, memoryAllocationSize, &pVertices[0]);
	glBufferSubData(GL_ARRAY_BUFFER, gl.colorsoffset, memoryAllocationSize, &pColors[0]);
	glBufferSubData(GL_ARRAY_BUFFER, gl.normalsoffset, memoryAllocationSize, &pNormals[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(intptr_t)gl.positionsoffset);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(intptr_t)gl.normalsoffset);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(intptr_t)gl.colorsoffset);
	glEnableVertexAttribArray(2);

	gl.egl.draw = draw_cube_smooth;

	return &gl.egl;
}
