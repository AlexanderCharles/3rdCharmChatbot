
#include "message_builder/Reactions.h"
#include "input/Stream.h"
#include "utils/Keyboard.h"

#include <X11/Xlib.h>
#include <sys/select.h>

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>



int
main(int argc, char** args)
{
	struct ReactionContext* reactionData;
	struct StreamContext*   fileStreamData;
	
	extern Display* gDisplay;
	
	char* loggedInAs;
	
	const int sleepDuration = 2;
	fd_set    rfds;
	struct    timeval tv = { 0 };
	
	if (argc < 2)
	{
		puts("Enter the username of the account which is being logged-in to.");
		return(-1);
	}
	else if (argc > 2)
	{
		puts("If your username contains spaces, " \
		     "please use quotation marks (\"username\").");
		return(-1);
	}
	else
	{
		loggedInAs = args[1];
	}
	
	gDisplay = XOpenDisplay(NULL);
	
	fileStreamData = StreamInit(loggedInAs);
	reactionData   = ReactionsInit();
	
	InputData dummyInput = ParseInput("18:52:43 <img=41>Germanic: c3butler hello");
	React(reactionData, &dummyInput);
	return 0;
	tv.tv_sec  = sleepDuration;
	puts("Now running. You can press 'q' to quit.");
	
	while (true)
	{
		char input;
		int  result;
		
		FD_ZERO(&rfds);
		FD_SET(STDIN_FILENO, &rfds);
		result = select(fileno(stdin) + 1, &rfds, NULL, NULL, &tv);
		
		if (result > 0)
		{
			input = fgetc(stdin);
			if (input == 'q') break;
		}
		else
		{
			tv.tv_sec = sleepDuration;
		}
		
		StreamProcess(fileStreamData, reactionData);
	}
	
	ReactionsClose(reactionData);
	StreamClose(fileStreamData);
	XCloseDisplay(gDisplay);
	
	return(0);
}
