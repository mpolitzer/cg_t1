/**
 *  @file IUPGL.c Simple exemple of use of OpenGL and IUP.
 *
 *  Creates a dialog with a tool bar, a canvas and a msg label bar.
 *  It is a simple WIMP program with color assigment for each pixel
 *  in the canvas, handling of mouse events and messages.
 *
 *  Link this program with:
 *  iup.lib;iupgl.lib;iupimglib.lib;comctl32.lib;opengl32.lib;glu32.lib;
 *
 *  You can find iup libs (iup.lib;iupgl.lib;iupimglib.lib;) at:
 *  http://www.tecgraf.puc-rio.br/iup/
 *  or at:
 *  http://www.tecgraf.puc-rio.br/~mgattass/tec/iup32vc8.zip
 *  the others lib (comctl32.lib;opengl32.lib;glu32.lib;) are inluded in the Visual Studio
 *
 *  Do not forget to put the path for the include and lib in the project file.
 *  Here we use: c:\tec\inc and c:\tec\lib
 *
 *  Ignore: libcmt.lib;
 *
 *  Last modification:  Marcelo Gattass, 02ago2010,10h.
 *
 **/
/*- Include lib interfaces: ANSI C, IUP and OpenGL ------*/
#include <stdio.h>
#include <stdlib.h>
#include <iup.h>        /* IUP functions*/
#include <iupgl.h>      /* IUP functions related to OpenGL (IupGLCanvasOpen,IupGLMakeCurrent and IupGLSwapBuffers) */
#include "image.h"

#ifdef WIN32
	#include <windows.h>    /* includes only in MSWindows not in UNIX */
	#include <gl/gl.h>     /* OpenGL functions*/
	#include <gl/glu.h>    /* OpenGL utilitary functions*/
#else
	#include <GL/gl.h>     /* OpenGL functions*/
	#include <GL/glu.h>    /* OpenGL utilitary functions*/
#endif

/*- Program context: -------------------------------------------------*/
static Image* cur_img;
static Image* old_img;
static Image* sobel_img;
static Image* high_img;

static int img_idx;
static Ihandle *canvas;                    /* canvas handle */
static Ihandle *msgbar;                    /* message bar  handle */
static int width=640,height=480;           /* width and height of the canvas  */

#define ARRAY_SZ(x) sizeof(x)/sizeof(*x)
Image *do_highlight(Image *orig, Image *sobel, float threshold)
{
	int x,y;
	float ro,go,bo;
	float rs,gs,bs;
	Image *high;
	high = imgCopy(orig);

	for(y=0; y<imgGetHeight(orig); y++){
		for(x=0; x<imgGetWidth(orig); x++){
			imgGetPixel3f(orig, x, y, &ro, &go, &bo);
			imgGetPixel3f(sobel, x, y, &rs, &gs, &bs);

			rs = ro - threshold*rs;
			gs = go - threshold*gs;
			bs = bo - threshold*bs;
			imgSetPixel3f(high, x, y, rs, gs, bs);
		}
	}

	return high;
}

char * get_file_name( void )
{
	Ihandle* getfile = IupFileDlg();
	char* filename = NULL;

	IupSetAttribute(getfile, IUP_TITLE, "Abertura de arquivo"  );
	IupSetAttribute(getfile, IUP_DIALOGTYPE, IUP_OPEN);
	IupSetAttribute(getfile, IUP_FILTER, "*.bmp");
	IupSetAttribute(getfile, IUP_FILTERINFO, "Arquivo de imagem (*.bmp)");
	IupPopup(getfile, IUP_CENTER, IUP_CENTER);

	filename = IupGetAttribute(getfile, IUP_VALUE);
	return filename;
}

char* get_new_file_name( void )
{
	Ihandle* getfile = IupFileDlg();
	char* filename = NULL;

	IupSetAttribute(getfile, IUP_TITLE, "Salva arquivo"  );
	IupSetAttribute(getfile, IUP_DIALOGTYPE, IUP_SAVE);
	IupSetAttribute(getfile, IUP_FILTER, "*.bmp");
	IupSetAttribute(getfile, IUP_FILTERINFO, "Arquivo de imagem (*.bmp)");
	IupPopup(getfile, IUP_CENTER, IUP_CENTER);

	filename = IupGetAttribute(getfile, IUP_VALUE);
	return filename;
}

/*------------------------------------------*/
/* IUP Callbacks                            */
/*------------------------------------------*/

/* function called when the canvas is exposed in the screen */
int repaint_cb(Ihandle *self)
{
	int x,y;
	IupGLMakeCurrent(self);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  /* black */
	glClear(GL_COLOR_BUFFER_BIT);          /* clear the color buffer */

	/* assing to each pixel of the canvas a red-green color in a given blue value (global variable) */
	if (!cur_img) return IUP_DEFAULT;

	glBegin(GL_POINTS);
	for (y=0; y < imgGetHeight(cur_img); y++) {
		for (x=0; x < imgGetWidth(cur_img); x++) {
			float r,g,b;
			imgGetPixel3f(cur_img, x, y, &r, &g, &b);
			glColor3f(r,g,b);        /* define a current color */
			glVertex2i(x,y);                  /* paint a point in the position (x,y,0) */
		}
	}
	glEnd();

	IupGLSwapBuffers(self);  /* change the back buffer with the front buffer */

	return IUP_DEFAULT; /* returns the control to the main loop */
}


/* function called in the event of changes in the width or in the height of the canvas */
int resize_cb(Ihandle *self, int new_width, int new_height)
{
	IupGLMakeCurrent(self);  /* Make the canvas current in OpenGL */

	/* define the entire canvas as the viewport  */
	glViewport(0,0,new_width,new_height);

	/* transformation applied to each vertex */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();           /* identity, i. e. no transformation */

	/* projection transformation (orthographic in the xy plane) */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D (0.0, (GLsizei)(new_width), 0.0, (GLsizei)(new_height));  /* window of interest [0,w]x[0,h] */

	/* update canvas size and repaint */
	width=new_width;
	height=new_height;
	repaint_cb(canvas);

	return IUP_DEFAULT; /* return to the IUP main loop */
}

int open_file_cb(void)
{
	char *fname = get_file_name();
	if (!fname) /*TODO show dialog de erro. */
		printf ("invalid file name: %s\n", fname);
	old_img = imgReadBMP(fname);
	// sobel_img = cur_img = do_sobel(old_img);
	sobel_img = cur_img = imgEdges(old_img);
	high_img = do_highlight(old_img, sobel_img, 0.1);
	resize_cb(canvas, imgGetWidth(cur_img), imgGetHeight(cur_img));
	repaint_cb(canvas);   /* repaint with new values of blue */
	return IUP_DEFAULT;
}

int toggle_cb(Ihandle *ih, int state)
{
	if (cur_img == high_img) cur_img = sobel_img;
	else cur_img = high_img;

	printf ("%p\n", cur_img);
	repaint_cb(canvas);
	return IUP_DEFAULT;
}

int motion_cb(Ihandle *self, int xm, int ym, char *status){
	int x=xm;
	int y=height-ym;
	IupSetfAttribute(msgbar, "TITLE", "Motion: x = %d, y=%d - %s",x,y,
			cur_img == sobel_img ? "old" : "cur");
	return IUP_DEFAULT;
}

int button_cb(Ihandle* self, int button, int pressed, int xm, int ym, char* status){
	int x=xm;
	int y=height-ym;
	IupSetfAttribute(msgbar, "TITLE", "pressed=%d, x=%d, y=%d, status=%s",
			pressed, x, y, status);
	return IUP_DEFAULT;
}


int exit_cb(void)
{
	printf("Function to free memory and do finalizations...\n");
	imgDestroy(sobel_img);
	imgDestroy(old_img);

	old_img = sobel_img = cur_img = NULL;
	return IUP_CLOSE;
}


/*-------------------------------------------------------------------------*/
/* Incializa o programa.                                                   */
/*-------------------------------------------------------------------------*/

Ihandle* InitToolbar(void)
{
	Ihandle* toolbar;

	/* Create four buttons */
	Ihandle* open_file = IupButton("open", "open_file_action");
	Ihandle* toggle_img = IupButton("toggle", "toggle_img_action");

	/* Associate images with this buttons */
	IupSetAttribute(open_file,"IMAGE","IUP_FileOpen");

	/* Associate tip's (text that appear when the mouse is over) */
	IupSetAttribute(open_file,"TIP","go to open_file");

	/* Associate function callbacks to the button actions */
	IupSetFunction("open_file_action", (Icallback)open_file_cb);
	IupSetFunction("toggle_img_action", (Icallback)toggle_cb);

	toolbar=IupHbox(open_file, toggle_img, IupFill(),NULL);

	return toolbar;
}

/* Create a IUP canvas */
Ihandle* InitCanvas(void)
{
	Ihandle* _canvas = IupGLCanvas("repaint_cb");        /* create _canvas and define its repaint callback */
	IupSetAttribute(_canvas,IUP_RASTERSIZE,"640x480");   /* define the size in pixels */
	IupSetAttribute(_canvas,IUP_BUFFER,IUP_DOUBLE);      /* define that this OpenGL _canvas has double buffer (front and back) */
	IupSetAttribute(_canvas, "RESIZE_CB", "resize_cb");  /* define callback action associate with the change in size of the _canvas */
	IupSetAttribute(_canvas, "BUTTON_CB","button_cb");
	IupSetAttribute(_canvas, "MOTION_CB","motion_cb");

	/* bind callback actions with callback functions */
	IupSetFunction("repaint_cb", (Icallback) repaint_cb);
	IupSetFunction("resize_cb", (Icallback) resize_cb);
	IupSetFunction("button_cb",(Icallback) button_cb);
	IupSetFunction("motion_cb",(Icallback) motion_cb);
	return _canvas;
}


Ihandle* InitDialog(void)
{
	Ihandle* dialog;   /* dialog containing the canvas */

	Ihandle* toolbar=InitToolbar(); /* buttons tool bar */
	canvas = InitCanvas();          /* canvas to paint with OpenGL */
	msgbar = IupLabel("A msg bar"); /* a msg bar */
	IupSetAttribute(msgbar,IUP_RASTERSIZE,"640x20");   /* define the size in pixels */

	/* create the dialog and set its attributes */
	dialog = IupDialog(IupVbox(toolbar,canvas,msgbar,NULL));
	IupSetAttribute(dialog, "TITLE", "IUP_OpenGL");
	IupSetAttribute(dialog, "CLOSE_CB", "exit_cb");
	IupSetFunction("exit_cb", (Icallback) exit_cb);

	return dialog;
}

/*-----------------------*/
/* Main function.        */
/*-----------------------*/
int main(int argc, char* argv[])
{
	Ihandle* dialog;
	IupOpen(&argc, &argv);
	IupImageLibOpen();
	IupGLCanvasOpen();

	dialog = InitDialog();
	IupShowXY(dialog, IUP_CENTER, IUP_CENTER);

	IupMainLoop();
	IupClose();
	return 0;
}
