
#include "Keyboard.h"

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>

#include <string.h>
#include <stdbool.h>



/* 
 * It does not work with letters like þ/é/ß but it does not crash either, so
 * whatever lol.
 * TODO: need a banned words list
 * */

/* It might be weird for this to contain the only global variable but I 
 * cba to refactor it. */
Display* gDisplay;



void IssueKeyPress(unsigned int);
void IssueKeyRelease(unsigned int);

KeySym GetKey(char);
bool IsCaps(char);



/* I have 0 idea how the anti-cheat works. It is possible that it requires a
 * random delay inbetween key presses, key releases, and inbetween the next
 * key press. */
/* TODO: this needs to account for message size and sentence slicing for 
 * messages which are too long (idek what the cap is) */
void
WriteMessage(const char* i_msg)
{
	int i, s, isCaps, shiftKeyCode, enterKeyCode;
	
	i = 0;
	s = strlen(i_msg);
	
	shiftKeyCode = XKeysymToKeycode(gDisplay, XK_Shift_L);
	enterKeyCode = XKeysymToKeycode(gDisplay, XK_Return);
	
	while (i < s)
	{
		if ((isCaps = IsCaps(i_msg[i])) == true)
		{
			IssueKeyPress(shiftKeyCode);
		}
		IssueKeyPress(GetKey(i_msg[i]));
		IssueKeyRelease(GetKey(i_msg[i]));
		if (isCaps == true)
		{
			IssueKeyRelease(shiftKeyCode);
		}
		++i;
	}
	
	IssueKeyPress(enterKeyCode);
	IssueKeyRelease(enterKeyCode);
}



void
IssueKeyPress(unsigned int i_keyCode)
{
	XTestFakeKeyEvent(gDisplay, i_keyCode, true, 0);
	XFlush(gDisplay);
}

void
IssueKeyRelease(unsigned int i_keyCode)
{
	XTestFakeKeyEvent(gDisplay, i_keyCode, false, 0);
	XFlush(gDisplay);
}

KeySym
GetKey(char i_c)
{
	return(XKeysymToKeycode(gDisplay, (KeySym) i_c));
}

bool
IsCaps(char i_c)
{
	if (i_c >= 'A' && i_c <= 'Z') return(true);
	return(false);
}


