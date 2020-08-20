#ifndef DISTORTION_H_INCLUDED
#define DISTORTION_H_INCLUDED
#define SIGN(x) (x < 0 ? (-1) : 1)
#define PID2 1.5707963268
GLuint EndTexture;
GLuint EmptyTexture() {											// Create An Empty Texture
	GLuint txtnumber;											// Texture ID
	unsigned int* data;											// Stored Data
	// Create Storage Space For Texture Data (128x128x4)
	data = (unsigned int*)new GLuint[((ScreenSizeX * ScreenSizeY)* 4 * sizeof(unsigned int))];
	ZeroMemory(data,((ScreenSizeX * ScreenSizeY)* 4 * sizeof(unsigned int)));	// Clear Storage Memory
	glGenTextures(1, &txtnumber);								// Create 1 Texture
	glBindTexture(GL_TEXTURE_2D, txtnumber);					// Bind The Texture
	glTexImage2D(GL_TEXTURE_2D, 0, 4, ScreenSizeX, ScreenSizeY, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, data);						// Build Texture Using Information In data
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	delete [] data;												// Release data
	return txtnumber;											// Return The Texture ID
}
#define DISTORT_NONE      0
#define DISTORT_SINE_XY   1
#define DISTORT_SINE_R    2
#define DISTORT_SQUARE_XY 3
#define DISTORT_SQUARE_R  4
#define DISTORT_ASIN_XY   5
#define DISTORT_ASIN_R    6
typedef unsigned short DistortMode;
void Transform(DistortMode dm, int i, int j, double *ix, double *iy) {
    double x, y, xnew, ynew;
    double r, theta, rnew, thetanew;
    
    x = i / (ScreenSizeX/2.0) - 1;
    y = j / (ScreenSizeY/2.0) - 1;
    r = sqrt(x*x + y*y);
    theta = atan2(y, x);
    
    switch (dm) {
        case DISTORT_NONE:
            xnew = x;
            ynew = y;
            break;
        case DISTORT_SINE_XY:
            xnew = sin(PID2*x);
            ynew = sin(PID2*y);
            break;
        case DISTORT_SINE_R:
            rnew = sin(PID2*r);
            thetanew = theta;
            xnew = rnew * cos(thetanew);
            ynew = rnew * sin(thetanew);
            break;
        case DISTORT_SQUARE_XY:
            xnew = x*x*SIGN(x);
            ynew = y*y*SIGN(y);
            break;
        case DISTORT_SQUARE_R:
            rnew = r*r;
            thetanew = theta;
            xnew = rnew * cos(thetanew);
            ynew = rnew * sin(thetanew);
            break;
        case DISTORT_ASIN_XY:
            xnew = asin(x) / PID2;
            ynew = asin(y) / PID2;
            break;
        case DISTORT_ASIN_R:
            rnew = asin(r) / PID2;
            thetanew = theta;
            xnew = rnew * cos(thetanew);
            ynew = rnew * sin(thetanew);
            break;
    }
    
    *ix = (xnew + 1) / 2.0;
    *iy = (ynew + 1) / 2.0;
}
void CreateGrid(int StartX, int StartY, int SizeX, int SizeY, DistortMode dm, int dd) {
    glViewport(StartX, StartY, SizeX, SizeY);
    
    glMatrixMode(GL_PROJECTION);							// Select Projection
	glPushMatrix();											// Push The Matrix
	glLoadIdentity();										// Reset The Matrix
	glOrtho( 0, ScreenSizeX, ScreenSizeY, 0,-1, 1);			// Select Ortho Mode
	glMatrixMode(GL_MODELVIEW);								// Select Modelview Matrix
	glPushMatrix();											// Push The Matrix
	glLoadIdentity();										// Reset The Matrix
    
    int i, j;
    double x, y;
    
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBindTexture(GL_TEXTURE_2D, EndTexture);
    for (i = 0; i <= ScreenSizeX; i += dd) {
        glBegin(GL_TRIANGLE_STRIP);
        for (j = 0; j <= ScreenSizeY; j += dd) {
            Transform(dm, i, j, &x, &y);
            if (x >= 0 && x <= 1) { if (y >= 0 && y <= 1) {
                glTexCoord2f(x, y); glVertex3f( i, ScreenSizeY-j, 0);
            }}
            Transform(dm, i+dd, j, &x, &y);
            if (x >= 0 && x <= 1) { if (y >= 0 && y <= 1) {
                glTexCoord2f(x, y); glVertex3f((i+dd), ScreenSizeY-j, 0);
            }}
        }
        glEnd();
    }
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	
    glMatrixMode(GL_PROJECTION);							// Select Projection
	glPopMatrix();											// Pop The Matrix
	glMatrixMode(GL_MODELVIEW);								// Select Modelview
	glPopMatrix();											// Pop The Matrix
	
	glViewport(0, 0, ScreenSizeX, ScreenSizeY);
}
void DrawGLScene();
void RenderToTexture(int StartX, int StartY, int SizeX, int SizeY) {
	glBindTexture(GL_TEXTURE_2D, EndTexture);
	// Copy Our ViewPort
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, StartX, StartY, SizeX, SizeY, 0);
}
void Distort(double StX, double StY, double SzX, double SzY, DistortMode dm, int dd) {
    int StartX = (int)(ScreenSizeX*StX);
    int StartY = (int)(ScreenSizeY*StY);
    int SizeX  = (int)(ScreenSizeX*SzX);
    int SizeY  = (int)(ScreenSizeY*SzY);
    
    RenderToTexture(StartX, StartY, SizeX, SizeY);
    CreateGrid(StartX, StartY, SizeX, SizeY, dm, dd);
}
#endif