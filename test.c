//Copyright (c) 2011-2020 <>< Charles Lohr - Under the MIT/x11 or NewBSD License you choose.
// NO WARRANTY! NO GUARANTEE OF SUPPORT! USE AT YOUR OWN RISK

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "os_generic.h"
#include <GLES3/gl3.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android_native_app_glue.h>
#include <android/sensor.h>
#include <byteswap.h>
#include <errno.h>
#include <fcntl.h>
#include "CNFGAndroid.h"

//#define CNFA_IMPLEMENTATION
#define CNFG_IMPLEMENTATION
#define CNFG3D

//#include "cnfa/CNFA.h"
#include "CNFG.h"

#define WEBVIEW_NATIVE_ACTIVITY_IMPLEMENTATION
#include "webview_native_activity.h"

void AndroidDisplayKeyboard(int pShow);

int lastbuttonx = 0;
int lastbuttony = 0;
int lastmotionx = 0;
int lastmotiony = 0;
int lastbid = 0;
int lastmask = 0;
int lastkey, lastkeydown;

static int keyboard_up;
uint8_t buttonstate[8];

void HandleKey( int keycode, int bDown )
{
	lastkey = keycode;
	lastkeydown = bDown;
	if( keycode == 10 && !bDown ) { keyboard_up = 0; AndroidDisplayKeyboard( keyboard_up );  }

	if( keycode == 4 ) { AndroidSendToBack( 1 ); } //Handle Physical Back Button.
}

void HandleButton( int x, int y, int button, int bDown )
{
	buttonstate[button] = bDown;
	lastbid = button;
	lastbuttonx = x;
	lastbuttony = y;

	if( bDown ) { keyboard_up = !keyboard_up; AndroidDisplayKeyboard( keyboard_up ); }
}

void HandleMotion( int x, int y, int mask )
{
	lastmask = mask;
	lastmotionx = x;
	lastmotiony = y;
}

int HandleDestroy()
{
	printf( "Destroying\n" );
	return 0;
}

volatile int suspended;

void HandleSuspend()
{
	suspended = 1;
}

void HandleResume()
{
	suspended = 0;
}

struct Notes {
  Note[] notes;
};

struct Note {
  int position;
  char[] title;
  char[] text;
  char[] dateTime;  
};

struct VisualPosition {
  int x;
  int y;
};

//drawing notes
void drawNoteTile( int x, int y, int height, int width, struct note);
VisualPosition calcVisualNotePosition( int position );

//adding / removing notes
void deleteNoteTile( int position );
void addNoteTile( struct note );

//note editor popup
void drawNotePopup( Note note );
void closeNotePopup();
void drawTextField( int x, int y, char[] label );
void drawTextArea( int x, int y, char[] label );
void drawLabel( int x, int y, int size );
void drawButton( int x, int y, char[] buttonText, void (*f)() action );

//add note function
void addNotePressed();

void drawButton() {
	//TODO
}

void addNotePressed() {
	//TODO
}

int main( int argc, char ** argv )
{
	CNFGSetupFullscreen( "Notes app", 0 );
	while(CNFGHandleInput())
	{
		CNFGBGColor = 0xffffffff;
		CNFGClearFrame();
		CNFGColor( 0x000080ff ); 
		CNFGPenX = 1; CNFGPenY = 1;
		CNFGSetLineWidth( 5 );
		CNFGDrawText( "WRITE SOME NOTES", 20 );
		
		//draw add button
		//TODO
		// get size of text
		// find bottom and centre of screen
		// set addButtonX = centre of screen - half of size of text
		// set addButtonY = bottom of screen - 10 pixels
		//drawButton( addButtonX, addButtonY, "+Add Note", addNotePressed);
		
		CNFGSwapBuffers();		
	}
}