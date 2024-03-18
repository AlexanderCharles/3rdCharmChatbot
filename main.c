
#include "message_builder/Reactions.h"
#include "input/Stream.h"
#include "utils/Keyboard.h"

#include <X11/Xlib.h>

#include <stdio.h>
#include <unistd.h>


int
main(void)
{
	struct ReactionContext* reactionData;
	struct StreamContext*   fileStreamData;
	InputData dummyInput;
	extern Display* gDisplay;
	
	const char* loggedInAs = "";
	
	gDisplay = XOpenDisplay(NULL);
	
	fileStreamData = StreamInit(loggedInAs);
	reactionData = ReactionsInit();
	
#if 0
	while (true)
	{
		StreamProcess(fileStreamData, reactionData);
		sleep(5);
	}
#else
	dummyInput = ParseInput("14:28:20 A123 has reached Magic level 77.");
	React(reactionData, &dummyInput);
	dummyInput = ParseInput("14:28:20 <img=41>A123: @bot ping\n");
	React(reactionData, &dummyInput);
	dummyInput = ParseInput("14:28:20 A123: @bot hello\n");
	React(reactionData, &dummyInput);
	dummyInput = ParseInput("14:28:20 A123: bot Hi.\n");
	React(reactionData, &dummyInput);
#endif
	
	ReactionsClose(reactionData);
	StreamClose(fileStreamData);
	XCloseDisplay(gDisplay);
	
	return(0);
}
