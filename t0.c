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

#ifdef WIN32
#include <windows.h>    /* includes only in MSWindows not in UNIX */
#endif

#include <GL/gl.h>     /* OpenGL functions*/
#include <GL/glu.h>    /* OpenGL utilitary functions*/

/*- Program context: -------------------------------------------------*/
Ihandle *canvas;                    /* canvas handle */
Ihandle *msgbar;                    /* message bar  handle */
int width=640,height=480;           /* width and height of the canvas  */
float blue=0.3f;                    /* current blue level */

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
  glBegin(GL_POINTS);       
  for (y=0;y<height;y++) {
      for (x=0; x<width; x++) {
          float red = ((float)x)/width;
          float green = ((float)y)/height;
          glColor3f(red,green,blue);        /* define a current color */
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

int first_cb(void)
{
     blue=0;
     /* print the blue level in the msg bar */
     IupSetfAttribute(msgbar, "TITLE", "Red [0,1] x Green [0,1] with Blue: %3.2f",blue); 
     repaint_cb(canvas);   /* repaint with new values of blue */
     return IUP_DEFAULT;
}

int previous_cb(void)
{
     blue-=0.1f;
     if (blue<0.0f) blue=0.0f;
     /* print the blue level in the msg bar */
     IupSetfAttribute(msgbar, "TITLE", "Red [0,1] x Green [0,1] with Blue: %3.2f",blue); 
     repaint_cb(canvas);   /* repaint with new values of blue */
     return IUP_DEFAULT;
}

int next_cb(void)
{
     blue+=0.1f;
     if (blue>1.0f) blue=1.0f;
     /* print the blue level in the msg bar */
     IupSetfAttribute(msgbar, "TITLE", "Red [0,1] x Green [0,1] with Blue: %3.2f",blue); 
     repaint_cb(canvas);   /* repaint with new values of blue */
     return IUP_DEFAULT;
}

int last_cb(void)
{
     blue=1;
     /* print the blue level in the msg bar */
     IupSetfAttribute(msgbar, "TITLE", "Red [0,1] x Green [0,1] with Blue: %3.2f",blue); 
     repaint_cb(canvas);   /* repaint with new values of blue */
     return IUP_DEFAULT;
}

int motion_cb(Ihandle *self, int xm, int ym, char *status){
	 int x=xm;
	 int y=height-ym;
     IupSetfAttribute(msgbar, "TITLE", "Motion: x = %d, y=%d and status=%s",x,y,status); 
     return IUP_DEFAULT;
}

int button_cb(Ihandle* self, int button, int pressed, int xm, int ym, char* status){
	 int x=xm;
	 int y=height-ym;
     IupSetfAttribute(msgbar, "TITLE", "Button: pressed=%d, x=%d, y=%d and status=%s",pressed,x,y,status); 
     return IUP_DEFAULT;
}


int exit_cb(void)
{
     printf("Function to free memory and do finalizations...\n");

     return IUP_CLOSE;
}


/*-------------------------------------------------------------------------*/
/* Incializa o programa.                                                   */
/*-------------------------------------------------------------------------*/

Ihandle* InitToolbar(void)
{
     Ihandle* toolbar;

    /* Create four buttons */
     Ihandle* first = IupButton("first", "first_action");
     Ihandle* previous = IupButton("previous", "previous_action");
     Ihandle* next = IupButton("next", "next_action");
     Ihandle* last = IupButton("last", "last_action");

     /* Associate images with this buttons */
     IupSetAttribute(first,"IMAGE","IUP_MediaGotoBegin");
     IupSetAttribute(previous,"IMAGE","IUP_MediaReverse");
     IupSetAttribute(next,"IMAGE","IUP_MediaPlay");
     IupSetAttribute(last,"IMAGE","IUP_MediaGoToEnd");

     /* Associate tip's (text that appear when the mouse is over) */
     IupSetAttribute(first,"TIP","go to first");
     IupSetAttribute(previous,"TIP","previous");
     IupSetAttribute(next,"TIP","next");
     IupSetAttribute(last,"TIP","go to last");

     /* Associate function callbacks to the button actions */
     IupSetFunction("first_action", (Icallback)first_cb);
     IupSetFunction("previous_action", (Icallback)previous_cb);
     IupSetFunction("next_action", (Icallback)next_cb);
     IupSetFunction("last_action", (Icallback)last_cb);

     toolbar=IupHbox(first,previous,next,last, IupFill(),NULL);

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
    IupOpen(&argc, &argv);                       /* opens the IUP lib */
     IupImageLibOpen();                          /* enable iup_image manipulation (for buttons) */
     IupGLCanvasOpen();                          /* enable the use of OpenGL to draw in canvas */

     dialog = InitDialog();                      /* local function to create a dialog with buttons and canvas */
     IupShowXY(dialog, IUP_CENTER, IUP_CENTER);  /* shows dialog in the center of screen */

     IupMainLoop();                              /* handle the program control to the IUP lib until a return IUP_CLOSE */

     IupClose();                                 /* closes the IUP lib */ 
}
