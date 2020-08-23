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
#include <png.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

#include "common.h"
#include "esUtil.h"



// Data read from the header of the BMP file
unsigned char header[54]; // Each BMP file begins by a 54-bytes header
unsigned int dataPos;     // Position in the file where the actual data begins
int width = 128;
int height = 128;
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
		
				

		"vec2 brownConradyDistortion(in vec2 uv, in float k1, in float k2)\n"
		"{ \n"
		"    uv = uv * 2.0 - 1.0;\n"
		"    // positive values of K1 give barrel distortion, negative give pincushion\n"
		"    float r2 = uv.x*uv.x + uv.y*uv.y;\n"
		"    uv *= 1.0 + k1 * r2 + k2 * r2 * r2;\n"
		"    uv = (uv * .5 + .5);	\n" 
		"    return uv; \n"
		"} \n"
		
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
int memoryAllocationSize;

void setPixel(int set_x, int set_y, float r, float g, float b) {
	int cnt = 0;
	for (int y = 0; y < height; y ++) {
		for (int x = 0; x < width; x ++) {
			// each pixel has 4 vertices
			for (int v = 0; v < 4; v ++) {
				for (int rgb = 0; rgb < 3; rgb ++) {
					if (x == set_x && y == set_y) {
						if (rgb == 0) {
							pColors[cnt] = r;
						} else if (rgb == 1) {
							pColors[cnt] = g;
						} else if (rgb == 2) {
							pColors[cnt] = b;
						}
					}
					
					cnt ++;
				}
			}
		}
	}
}

void SpawnPixelPlane() {
	
	
	GLfloat px_sqare[] = {
		// x    y      z
		-1.0f, -1.0f, 0.0f,
		+1.0f, -1.0f, 0.0f,
		-1.0f, +1.0f, 0.0f,
		+1.0f, +1.0f, 0.0f,
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
	int filler = 0;
	char XYZloop = 1;
	GLfloat scale = 0.01f;
	GLfloat x_offset = -300.0f;
	GLfloat y_offset = 200.0f;
	
	for (int y = 0; y < height; y ++) {
		for (int x = 0; x < width; x ++) {
			for (int px = 0; px < 12; px ++) {
				//printf("%i\n", (int)sizeof(px_sqare)/sizeof(float));
				//printf("pVertices[%i] = px_sqare[%i] = %f\n", filler, px, px_sqare[px]);
				
				pNormals[filler] = defaultNormal[px];
				pVertices[filler] = px_sqare[px];
				pColors[filler] = 0.0f;
				if (filler < 12) {
					pColors[filler] = 1.0f;
				}
				
				if (XYZloop == 1) { // X
					pVertices[filler] += x_offset;
					pVertices[filler] += 2.0f * x;
					
					pVertices[filler] *= scale;
					
					//printf("lets add 2.0f * %i\n", x);
				}
				if (XYZloop == 2) { // y
					pVertices[filler] += y_offset;
					pVertices[filler] -= 2.0f * y;
					
					pVertices[filler] *= scale;
				}
				if (XYZloop == 3) {
					XYZloop=0;
				} 
				XYZloop ++;
				filler ++;
				if (filler > width * height * 12) {
					printf("memory overflow error fix");
					break;
				}
			}
		} 
	}	
}



/*
void loadPNG() {
	const char * png_file = "chessboard.png";
    png_structp	png_ptr;
    png_infop info_ptr;
    FILE * fp;
    png_uint_32 width;
    png_uint_32 height;
    int bit_depth;
    int color_type;
    int interlace_method;
    int compression_method;
    int filter_method;
    int j;
    png_bytepp rows;
    fp = fopen (png_file, "rb");
    if (! fp) {
		fatal_error ("Cannot open '%s': %s\n", png_file, strerror (errno));
    }
    png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (! png_ptr) {
		fatal_error ("Cannot create PNG read structure");
    }
    info_ptr = png_create_info_struct (png_ptr);
    if (! png_ptr) {
		fatal_error ("Cannot create PNG info structure");
    }
    png_init_io (png_ptr, fp);
    png_read_png (png_ptr, info_ptr, 0, 0);
    png_get_IHDR (png_ptr, info_ptr, & width, & height, & bit_depth,
		  & color_type, & interlace_method, & compression_method,
		  & filter_method);
    rows = png_get_rows (png_ptr, info_ptr);
    printf ("Width is %d, height is %d\n", width, height);
    int rowbytes;
    rowbytes = png_get_rowbytes (png_ptr, info_ptr);
    printf ("Row bytes = %d\n", rowbytes);
    for (j = 0; j < height; j++) {
		int i;
		png_bytep row;
		row = rows[j];
		for (i = 0; i < width; i++) {
			png_bytep px = &(row[i * 4]);
			// Do something awesome for each pixel here...
			printf("%4d, %4d = RGBA(%3d, %3d, %3d, %3d)\n", i, j, px[0], px[1], px[2], px[3]);
			png_byte pixel;
			pixel = row[i];
			if (pixel < 64) {
				printf ("##");
			}
			else if (pixel < 128) {
				printf ("**");
			}
			else if (pixel < 196) {
				printf ("..");
			}
			else {
				printf ("  ");
			}

		}
		printf ("\n");
	}
}*/


int png_width, png_height;
png_byte color_type;
png_byte bit_depth;
png_bytep *row_pointers = NULL;

void read_png_file(char *filename) {
  FILE *fp = fopen(filename, "rb");

  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(!png) abort();

  png_infop info = png_create_info_struct(png);
  if(!info) abort();

  if(setjmp(png_jmpbuf(png))) abort();

  png_init_io(png, fp);

  png_read_info(png, info);

  png_width      = png_get_image_width(png, info);
  png_height     = png_get_image_height(png, info);
  color_type = png_get_color_type(png, info);
  bit_depth  = png_get_bit_depth(png, info);

  // Read any color_type into 8bit depth, RGBA format.
  // See http://www.libpng.org/pub/png/libpng-manual.txt

  if(bit_depth == 16)
    png_set_strip_16(png);

  if(color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(png);

  // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
  if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand_gray_1_2_4_to_8(png);

  if(png_get_valid(png, info, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png);

  // These color_type don't have an alpha channel then fill it with 0xff.
  if(color_type == PNG_COLOR_TYPE_RGB ||
     color_type == PNG_COLOR_TYPE_GRAY ||
     color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

  if(color_type == PNG_COLOR_TYPE_GRAY ||
     color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png);

  png_read_update_info(png, info);

  if (row_pointers) abort();

  row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * png_height);
  for(int y = 0; y < png_height; y++) {
    row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
  }

  png_read_image(png, row_pointers);

  fclose(fp);

  png_destroy_read_struct(&png, &info, NULL);
}

float btof(double byte) {
	return (1.0f / 255) * byte;
}


void process_png_file() {
  int count = 0;
  for(int y = 0; y < png_height; y++) {
    png_bytep row = row_pointers[y];
    for(int x = 0; x < png_width; x++) {
      png_bytep px = &(row[x * 4]);
      
	for(int whole_pixel = 0; whole_pixel < 4; whole_pixel++) {
	      pColors[count] = (1.0f / 255) * px[0];
	      count ++;
	      pColors[count] = (1.0f / 255) * px[1];
	      count ++;
	      pColors[count] = (1.0f / 255) * px[2];
	      count ++;
	}
       
      // Do something awesome for each pixel here...
      //printf("%4d, %4d = RGBA(%3d, %3d, %3d, %3d)\n", x, y, px[0], px[1], px[2], px[3]);
      //setPixel(x, y, btof(px[0]), btof(px[1]), btof(px[2]));
    }
  }
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
*/

// width and height
static GLubyte checkImage[128][128][4];

// https://stackoverflow.com/questions/12969971/is-it-possible-to-manually-create-image-data-for-opengl-texture-use
void makeCheckImage()
{
   int i, j, c;

   for (i = 0; i < height; i++) {
      for (j = 0; j < width; j++) {
         c = ((((i&0x8)==0)^((j&0x8))==0))*255;
	 
         checkImage[i][j][0] = (GLubyte) c;
         checkImage[i][j][1] = (GLubyte) c;
         checkImage[i][j][2] = (GLubyte) c;
         checkImage[i][j][3] = (GLubyte) 255;
      }
   }
}

// https://open.gl/textures
// https://open.gl/content/code/c3_exercise_1.txt
/*
GLuint texture[1];
void createTexture() {
    // generate texture
    glGenTextures(1, texture);
    
    // Activate texture as in this is what were using now
    glActiveTexture(GL_TEXTURE0);
    // Bind the generated texture to the active texture
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    // https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glTexImage2D.xml
    makeCheckImage();
     

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, checkImage);
    
    
    // Create and compile the vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    // Create and compile the fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    // Link the vertex and fragment shader into a shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);
    
    
    // this function needs research
    glUniform1i(glGetUniformLocation(shaderProgram, "texKitten"), 0);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    
    

}

*/


static void draw_cube_smooth()
{
    	//read_png_file("/var/memdrive/frame.png");
    	//process_png_file();
    	//read_png_file("/var/memdrive/frame.png");
    	//process_png_file();
    
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

	for (int i = 0; i < width*height; i++) {
		glDrawArrays(GL_TRIANGLE_STRIP, i*4, 4);
	}


}
const struct egl * init_cube_smooth(const struct gbm *gbm, int samples)
{
	SpawnPixelPlane();
    	//read_png_file("/var/memdrive/frame.png");
	read_png_file("chessboard.png");
    	process_png_file();
	//printf("Setting pixel 0,0 to white");
     	//setPixel(1, 1, 1.0f, 1.0f, 1.0f);
	
	//createTexture();
	//exit(0);
	//init_bmp();
	//for (int i = 0; i < 24; i ++) {
	//	printf("hello i:%i a:%f b:%f\n", i, pVertices[i], vVertices[i]);
	//}
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
