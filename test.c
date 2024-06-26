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

struct Pressable {
	int id;
	int topLeftCornerX;
	int topLeftCornerY;
	int bottomRightCornerX;
	int bottomRightCornerY;
	void (*action)();
};

int pressableCount = 0;
struct Pressable pressables[30];

int is_inside( int pointX, int pointY, int topLeftCornerX, int topLeftCornerY, int bottomRightCornerX, int bottomRightCornerY );
void checkPress( struct Pressable* pressable );

void HandleKey( int keycode, int bDown )
{
	lastkey = keycode;
	lastkeydown = bDown;
	//if( keycode == 10 && !bDown ) { keyboard_up = 0; AndroidDisplayKeyboard( keyboard_up );  }

	if( keycode == 4 ) { AndroidSendToBack( 1 ); } //Handle Physical Back Button.
}

void HandleButton( int x, int y, int button, int bDown )
{
	buttonstate[button] = bDown;
	lastbid = button;
	lastbuttonx = x;
	lastbuttony = y;

	//if( bDown ) { keyboard_up = !keyboard_up; AndroidDisplayKeyboard( keyboard_up ); }
	if (bDown) { 
	    for (int i = 0; i < pressableCount; i++) {
			checkPress(&pressables[i]);
		}
	}
}

void checkPress( struct Pressable* pressable ) {
	if (is_inside(lastbuttonx, lastbuttony, pressable->topLeftCornerX, pressable->topLeftCornerY, pressable->bottomRightCornerX, pressable->bottomRightCornerY)) {
		pressable->action();
	}
}

int is_inside ( int pointX, int pointY, int topLeftCornerX, int topLeftCornerY, int bottomRightCornerX, int bottomRightCornerY )
{
	return ((pointX >= topLeftCornerX ) &&
	(pointX <= bottomRightCornerX) &&
	(pointY >= topLeftCornerY ) &&
	(pointY <= bottomRightCornerY));
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

struct Note;

struct Notes {
  struct Note* notes;
};

struct Note {
  int position;
  char* title;
  char* text;
  char* dateTime;  
};

struct VisualPosition {
  int x;
  int y;
};

//extend rawdraw with fancy rounded corners
void drawBox( short topLeftCornerX, short topLeftCornerY, short bottomRightCornerX, short bottomRightCornerY, int borderRadius );
void drawRectangle( short topLeftCornerX, short topLeftCornerY, short bottomRightCornerX, short bottomRightCornerY, int borderRadius );

//drawing notes
void drawNoteTile( int x, int y, int height, int width, struct Note note);
struct VisualPosition calcVisualNotePosition( int position );

//adding / removing notes
void deleteNoteTile( int position );
void addNoteTile( struct Note note );

//note editor popup
void drawNotePopup( struct Note note );
void closeNotePopup();
void drawTextField( int x, int y, char* label );
void drawTextArea( int x, int y, char* label );
void drawLabel( int x, int y, int size );
int drawButton( int x, int y, char* buttonText, void (*action)(), int buttonId );

//add note function
void addNotePressed();

void drawBox( short topLeftCornerX, short topLeftCornerY, short bottomRightCornerX, short bottomRightCornerY, int borderRadius ) {
	uint32_t lc = CNFGLastColor;
	CNFGColor( lc );
	drawRectangle( topLeftCornerX, topLeftCornerY, bottomRightCornerX, bottomRightCornerY, borderRadius );
}

void drawRectangle( short topLeftCornerX, short topLeftCornerY, short bottomRightCornerX, short bottomRightCornerY, int borderRadius ) {
	int f, ddF_x, ddF_y, xx, yy;
	
	f = 1 - borderRadius;
	ddF_x = 1;
	ddF_y = -2 * borderRadius;
	xx = 0;
	yy = borderRadius;
	
	while(xx < yy) {
		if (f >= 0) {
			yy-=1;
			ddF_y += 2;
			f += ddF_y;
		}
		xx+=1;
		ddF_x += 2;
		f += ddF_x;
		CNFGTackPixel(bottomRightCornerX + xx - borderRadius, bottomRightCornerY + yy - borderRadius);
		CNFGTackPixel(bottomRightCornerX + yy - borderRadius, bottomRightCornerY + xx - borderRadius);
		CNFGTackPixel(topLeftCornerX - xx + borderRadius, bottomRightCornerY + yy - borderRadius);
		CNFGTackPixel(topLeftCornerX - yy + borderRadius, bottomRightCornerY + xx - borderRadius);
		CNFGTackPixel(bottomRightCornerX + xx - borderRadius, topLeftCornerY - yy + borderRadius);
		CNFGTackPixel(bottomRightCornerX + yy - borderRadius, topLeftCornerY - xx + borderRadius);
		CNFGTackPixel(topLeftCornerX - xx + borderRadius, topLeftCornerY - yy + borderRadius);
		CNFGTackPixel(topLeftCornerX - yy + borderRadius, topLeftCornerY - xx + borderRadius);
	}
	CNFGTackSegment(topLeftCornerX + borderRadius, topLeftCornerY, bottomRightCornerX - borderRadius, topLeftCornerY); // top side
	CNFGTackSegment(topLeftCornerX + borderRadius, bottomRightCornerY, bottomRightCornerX - borderRadius, bottomRightCornerY); // bottom side
	CNFGTackSegment(topLeftCornerX, topLeftCornerY+borderRadius, topLeftCornerX, bottomRightCornerY-borderRadius); // left side
	CNFGTackSegment(bottomRightCornerX, topLeftCornerY+borderRadius, bottomRightCornerX, bottomRightCornerY-borderRadius); // right side
}

int drawButton( int buttonCentreX, int buttonCentreY, char* text, void (*action)(), int buttonId ) {
	// get size of text
	int textWidth, textHeight;
	CNFGGetTextExtents( text, &textWidth, &textHeight, 20 );
	// draw button outline
	int topLeftCornerX = buttonCentreX - (textWidth / 2) - 20;
	int topLeftCornerY = buttonCentreY - (textHeight / 2) - 20;
	int bottomRightCornerX = buttonCentreX + (textWidth / 2) + 20;
	int bottomRightCornerY = buttonCentreY + (textHeight / 2) + 20;
	drawBox( topLeftCornerX, topLeftCornerY, bottomRightCornerX, bottomRightCornerY, 15 );
	// write text
	CNFGPenX = topLeftCornerX + 20; CNFGPenY = topLeftCornerY + 30;
	CNFGDrawText( text, 20 );
	//TODO
	// attach action function
	int pressableIndex = pressableCount;
	pressables[pressableIndex].id = pressableCount+1;
	pressables[pressableIndex].topLeftCornerX = topLeftCornerX;
	pressables[pressableIndex].topLeftCornerY = topLeftCornerY;
	pressables[pressableIndex].bottomRightCornerX = bottomRightCornerX;
	pressables[pressableIndex].bottomRightCornerY = bottomRightCornerY;
	pressables[pressableIndex].action = addNotePressed;
	if ( buttonId == 0 ) {
		pressableCount++;
	}
	buttonId = pressables[pressableIndex].id;
	return buttonId;
}

int debugAddNotePressed = 0;
void addNotePressed() {
	//TODO
	debugAddNotePressed++;
}

#define ADD_BUTTON_LABEL "+Add Note"

int main( int argc, char ** argv )
{
	CNFGSetupFullscreen( "Notes app", 0 );
	int addButtonId = 0;
	while(CNFGHandleInput())
	{
		CNFGBGColor = 0xffffffff;
		CNFGClearFrame();
		CNFGColor( 0x000080ff ); 
		CNFGPenX = 1; CNFGPenY = 1;
		CNFGSetLineWidth( 5 );
		CNFGDrawText( "WRITE SOME NOTES", 20 );
		
		// draw add button
		// get size of text
		int addButtonTextWidth, addButtonTextHeight;
		CNFGGetTextExtents( ADD_BUTTON_LABEL, &addButtonTextWidth, &addButtonTextHeight, 20 );
		// find bottom and centre of screen
		int screenBottom, screenCentre;
		short screenWidth, screenHeight;
		CNFGGetDimensions( &screenWidth, &screenHeight );
		screenBottom = screenHeight;
		screenCentre = screenWidth / 2;
		// place button in centre and slightly up from the bottom
		int addButtonX = screenCentre;
		int addButtonY = screenBottom - addButtonTextHeight - 10;
		addButtonId = drawButton( addButtonX, addButtonY, "+Add Note", addNotePressed, addButtonId );
		
		// debug text
		char debug[25];
		sprintf( debug, "debugAddNotePressed: %i\n", debugAddNotePressed);
		CNFGPenX = 100; CNFGPenY = 100;
		CNFGDrawText( debug, 10 );
		
		CNFGSwapBuffers();		
	}
}