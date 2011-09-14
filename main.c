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
static Image* orig_img;
static Image* myeffect_img;
static Image* sobel_img;
static Image* high_img;
static Image* grey_img;
static Image* gauss_img;
static Image* median_img;
static Image* reduce_img;
static Image* otsu_img;
static Image* ohbuchi_img;

static Ihandle* dialog;
static Ihandle *canvas;                    /* canvas handle */
static Ihandle *msgbar;                    /* message bar  handle */
static int width=640,height=480;           /* width and height of the canvas  */

int param_action(Ihandle *_dialog, int param_index, void *user_data)
{
	return IUP_DEFAULT; /* returns the control to the main loop */
}

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(*x))
Image *do_highlight(Image *orig, Image *sobel)
{
	int x,y;
	float th = 0;
	float ro,go,bo;
	float rs,gs,bs;
	Image *high;
	high = imgCopy(orig);

	if (!IupGetParam("set threshold", param_action, 0,
				"threshold: %r\n", &th, NULL))
		th = 0.1;

	for(y=0; y<imgGetHeight(orig); y++){
		for(x=0; x<imgGetWidth(orig); x++){
			imgGetPixel3f(orig, x, y, &ro, &go, &bo);
			imgGetPixel3f(sobel, x, y, &rs, &gs, &bs);

			rs = ro - th*rs;
			gs = go - th*gs;
			bs = bo - th*bs;
			/* clipping img */
			if (rs < 0) rs = 0; if (rs > 1) rs = 1;
			if (gs < 0) gs = 0; if (gs > 1) rs = 1;
			if (bs < 0) bs = 0; if (bs > 1) rs = 1;

			imgSetPixel3f(high, x, y, rs, gs, bs);
		}
	}

	return high;
}

Image *do_myeffect(Image *orig)
{
	int x,y,z;
	Image *high;
	high = imgCopy(orig);
	int remap[][2] = {
		{-1, -1},	{ 0, -1},	{ 1, -1},
		{-1,  0},	{ 0,  0},	{ 1,  0},
		{-1,  1},	{ 0,  1},	{ 1,  1},
	};

	for(y=1; y<imgGetHeight(orig)-1; y+=3){
		for(x=1; x<imgGetWidth(orig)-1; x+=3){
			float ro = 0,go = 0,bo = 0;
			float rs = 0,gs = 0,bs = 0;

			for(z=0; z<ARRAY_SIZE(remap); z++){
				imgGetPixel3f(orig, x+remap[z][0],
						y+remap[z][1],
						&ro, &go, &bo);
				rs += ro;
				gs += go;
				bs += bo;
			}
			rs = rs/ARRAY_SIZE(remap);
			gs = gs/ARRAY_SIZE(remap);
			bs = bs/ARRAY_SIZE(remap);

			for(z=0; z<ARRAY_SIZE(remap); z++)
				imgSetPixel3f(high, x+remap[z][0],
						y+remap[z][1],
						rs, gs, bs);
		}
	}
	return high;
}

void update_dialog_size(Ihandle* _dialog, Ihandle* _canvas, int w, int h )
{
	char buffer[64];
	sprintf(buffer,"%dx%d",w,h);
	IupSetAttribute(_canvas, IUP_RASTERSIZE, buffer);
	IupSetAttribute(_dialog, IUP_RASTERSIZE, NULL);
	IupShowXY(_dialog, IUP_CENTER, IUP_CENTER);
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

	cur_img = orig_img = imgReadBMP(fname);
	update_dialog_size(dialog, canvas, imgGetWidth(orig_img), imgGetHeight(orig_img));

	sobel_img = imgEdges(orig_img);
	myeffect_img = do_myeffect(orig_img);
	grey_img = imgGrey(orig_img);
	gauss_img = imgCopy(orig_img);
	imgGauss(gauss_img, orig_img);
	median_img = imgCopy(orig_img);
	imgMedian(median_img);
	reduce_img = imgCopy(orig_img);
	imgReduceColors(orig_img, reduce_img, 255);
	otsu_img = imgBinOtsu(orig_img);
	ohbuchi_img = imgBinOhbuchi(orig_img);
	resize_cb(canvas, imgGetWidth(cur_img), imgGetHeight(cur_img));
	repaint_cb(canvas);
	return IUP_DEFAULT;
}

int save_file_cb(void)
{
	char *fname = get_new_file_name();
	if (!fname) /*TODO show dialog de erro. */
		printf ("invalid file name: %s\n", fname);

	imgWriteBMP(fname, cur_img);

	return IUP_DEFAULT;
}

int show_orig_cb(Ihandle *ih, int state)
{
	cur_img = orig_img;

	repaint_cb(canvas);
	return IUP_DEFAULT;
}

int highlight_cb(Ihandle *ih, int state)
{
	imgDestroy(high_img);
	high_img = do_highlight(orig_img, sobel_img);
	cur_img = high_img;

	repaint_cb(canvas);
	return IUP_DEFAULT;
}


int sobel_cb(Ihandle *ih, int state)
{
	cur_img = sobel_img;

	repaint_cb(canvas);
	return IUP_DEFAULT;
}

int myeffect_cb(Ihandle *ih, int state)
{
	cur_img = myeffect_img;

	repaint_cb(canvas);
	return IUP_DEFAULT;
}


int  grey_cb(Ihandle *ih, int state)
{
	cur_img = grey_img;

	repaint_cb(canvas);
	return IUP_DEFAULT;
}

int  gauss_cb(Ihandle *ih, int state)
{
	cur_img = gauss_img;

	repaint_cb(canvas);
	return IUP_DEFAULT;
}

int  median_cb(Ihandle *ih, int state)
{
	cur_img = median_img;

	repaint_cb(canvas);
	return IUP_DEFAULT;
}

int  reduce_cb(Ihandle *ih, int state)
{
	int num;
	imgDestroy(reduce_img);
	
	if (!IupGetParam("set number of colors", param_action, 0,
				"Number of colors: %i\n", &num, NULL))
		num = 255;

	reduce_img = imgCopy(orig_img);
	imgReduceColors(orig_img, reduce_img, num);
	
	cur_img = reduce_img;

	repaint_cb(canvas);
	return IUP_DEFAULT;
}

int  otsu_cb(Ihandle *ih, int state)
{
	cur_img = otsu_img;

	repaint_cb(canvas);
	return IUP_DEFAULT;
}

int  ohbuchi_cb(Ihandle *ih, int state)
{
	cur_img = ohbuchi_img;

	repaint_cb(canvas);
	return IUP_DEFAULT;
}

int motion_cb(Ihandle *self, int xm, int ym, char *status){
	int x=xm;
	int y=height-ym;
	IupSetfAttribute(msgbar, "TITLE", "Motion: x = %d, y=%d",x,y);
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

	sobel_img = cur_img = NULL;
	return IUP_CLOSE;
}


/*-------------------------------------------------------------------------*/
/* Incializa o programa.                                                   */
/*-------------------------------------------------------------------------*/

Ihandle* InitToolbar(void)
{
	Ihandle* toolbar;

	/* Create four buttons */
	Ihandle* hopen_file = IupButton("Open", "open_file_action");
	Ihandle* hsave_file = IupButton("Save", "save_file_action");
	Ihandle* hhighlight_img = IupButton("Highlight", "highlight_img_action");
	Ihandle* horig_img = IupButton("Original", "orig_img_action");
	Ihandle* hsobel_img = IupButton("Sobel", "sobel_img_action");
	Ihandle* hmyeffect_img = IupButton("Pixelize", "myeffect_img_action");
	Ihandle* hgrey_img = IupButton("Grey", "grey_img_action");
	Ihandle* hgauss_img = IupButton("Gauss", "gauss_img_action");
	Ihandle* hmedian_img = IupButton("Median", "median_img_action");
	Ihandle* hreduce_img = IupButton("Reduce", "reduce_img_action");
	Ihandle* hotsu_img = IupButton("Otsu", "otsu_img_action");
	Ihandle* hohbuchi_img = IupButton("Ohbuchi", "ohbuchi_img_action");

	/* Associate images with this buttons */
	IupSetAttribute(hopen_file,"IMAGE","IUP_FileOpen");
	IupSetAttribute(hsave_file,"IMAGE","IUP_FileSave");

	/* Associate tip's (text that appear when the mouse is over) */
	IupSetAttribute(hopen_file,"TIP","go to open_file");

	/* Associate function callbacks to the button actions */
	IupSetFunction("open_file_action", (Icallback)open_file_cb);
	IupSetFunction("save_file_action", (Icallback)save_file_cb);
	IupSetFunction("highlight_img_action", (Icallback)highlight_cb);
	IupSetFunction("orig_img_action", (Icallback)show_orig_cb);
	IupSetFunction("sobel_img_action", (Icallback)sobel_cb);
	IupSetFunction("myeffect_img_action", (Icallback)myeffect_cb);
	IupSetFunction("grey_img_action", (Icallback)grey_cb);
	IupSetFunction("gauss_img_action", (Icallback)gauss_cb);
	IupSetFunction("median_img_action", (Icallback)median_cb);
	IupSetFunction("reduce_img_action", (Icallback)reduce_cb);
	IupSetFunction("otsu_img_action", (Icallback)otsu_cb);
	IupSetFunction("ohbuchi_img_action", (Icallback)ohbuchi_cb);

	toolbar=IupHbox(hopen_file, hsave_file, horig_img, hhighlight_img,
			hsobel_img, hmyeffect_img, hgrey_img, hgauss_img,
			hmedian_img, hreduce_img, hotsu_img,
			hohbuchi_img, IupFill(),NULL);

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
	Ihandle* _dialog;   /* dialog containing the canvas */

	Ihandle* toolbar=InitToolbar(); /* buttons tool bar */
	canvas = InitCanvas();          /* canvas to paint with OpenGL */
	msgbar = IupLabel("A msg bar"); /* a msg bar */
	IupSetAttribute(msgbar,IUP_RASTERSIZE,"640x20");   /* define the size in pixels */

	/* create the dialog and set its attributes */
	_dialog = IupDialog(IupVbox(toolbar,canvas,msgbar,NULL));
	IupSetAttribute(_dialog, "TITLE", "T1 - Marcelo e Peter");
	IupSetAttribute(_dialog, "CLOSE_CB", "exit_cb");
	IupSetFunction("exit_cb", (Icallback) exit_cb);

	return _dialog;
}

/*-----------------------*/
/* Main function.        */
/*-----------------------*/
int main(int argc, char* argv[])
{
	IupOpen(&argc, &argv);
	IupImageLibOpen();
	IupGLCanvasOpen();

	dialog = InitDialog();
	IupShowXY(dialog, IUP_CENTER, IUP_CENTER);

	IupMainLoop();
	IupClose();
	return 0;
}
