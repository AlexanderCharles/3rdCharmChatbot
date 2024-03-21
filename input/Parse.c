
#include "Parse.h"

#include "../utils/String.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>




/* Get the first space, grab the contents before it, then parse this as
 * the message's timestamp. Lastly update the cursor / i_line.
 * 
 * 07:36:34 <img=41>Username: Confusing: : > comment <img=00> lol
 *         ^ first space / 'firstSpaceIndex'
 * Then the cursor will be updated to '<', or (in this example) 'U' if
 * a regular account. */
unsigned int ParseTimestamp      (InputData*, char*);

/* Check the account type by reading the first character. If it is
 * a '<', then it parses the brackets, and updates the cursor.
 * 
 * 07:36:34 <img=41>Username 123: Confusing: : > comment <img=00> lol
 *          ^ first character / 'i_line[0]' */
unsigned int ParseAccountType    (InputData*, char*);

void         ProbeInputDataType  (InputData*, char*);

/* This function only does something if InputData.type == MESSAGE.
 * 
 * Within the log file, the spaces used for usernames are SOMETIMES 
 * a different character to regular spaces, and seems to be 2 chars wide.
 * But I do not know what determines which will be used, so instead it
 * gets the end of the username by looking for the colon. If there is
 * not a colon within the first 13?14? characters then it is not a message.
 * 
 * 07:36:34 <img=41>Username 123: Confusing: : > comment <img=00> lol
 *      'i_line[0]' ^           ^ last character of region
 * */
unsigned int ParseMessageUsername(InputData*, char*);

/* If it is a message then it only copies the message, otherwise if it
 * is an anouncement, it copies the whole line for processing later. */
void         ParseMessage        (InputData*, char*,
                                  unsigned int, unsigned int);

/* This extracts the names (and other values) from the anouncements. 
 * TODO: currently only the names are being stored. */
void         ParseAnouncement    (InputData*);



InputData
ParseInput(char* i_line)
{
	InputData result = { 0 };
	unsigned int cursor, timestampTrimmed;
	
	cursor = 0;
	assert(strlen(i_line) > 0 && strlen(i_line) < 150);
	
	cursor += ParseTimestamp(&result, i_line);
	timestampTrimmed = cursor;
	
	cursor += ParseAccountType    (&result, &i_line[cursor]);
	ProbeInputDataType            (&result, &i_line[cursor]);
	cursor += ParseMessageUsername(&result, &i_line[cursor]);
	ParseMessage                  (&result, i_line, cursor, timestampTrimmed);
	ParseAnouncement              (&result);
	
	return(result);
}



unsigned int
ParseTimestamp(InputData* io_inputData, char* i_line)
{
	int firstSpaceIndex;
	char timestampBuffer[9] = { 0 };
	
	assert((firstSpaceIndex = ScanForToken(i_line, ' ')) >= 0);
	strncpy(timestampBuffer, i_line, firstSpaceIndex);
	io_inputData->date = DateTimeCreateFromTimestamp(timestampBuffer);
	
	return(firstSpaceIndex + 1);
}

unsigned int
ParseAccountType(InputData* io_inputData, char* i_line)
{
	unsigned int advancedBy;
	
	if (i_line[0] == '<')
	{
		int closingBracketIndex, size;
		char digitBuffer[3] = { 0 };
		
		assert((closingBracketIndex = ScanForToken(i_line, '>')) >= 0);
		size = i_line[closingBracketIndex - 2] == '=' ? 1 : 2;
		strncpy(digitBuffer, &i_line[closingBracketIndex - size], size);
		
		assert(StrIsNumeric(digitBuffer) == true);
		io_inputData->player.account_type =
			(enum AccountType) atoi(digitBuffer);
		
		advancedBy = closingBracketIndex + 1;
	}
	else /* Anouncement or regular account. */
	{
		io_inputData->player.account_type = NORMAL;
		advancedBy = 0;
	}
	
	return(advancedBy);
}

void
ProbeInputDataType(InputData* io_inputData, char* i_line)
{
	/* The trouble is that I cannot check for 13, or whatever the max name leng
	 * this, because names which contain the 2 char long space character messes
	 * this up. If a name can contain like 5 spaces then by setting this to 20 
	 * it will likely mess-up too... */
	if (ScanRangeForToken(i_line, ':', 20) > 0)
	{
		io_inputData->type = MESSAGE;
		
		if (stristr(i_line, BOT_NAME) != 0)
		{
			io_inputData->type = TAGGED_BOT;
		}
	}
	else if (strstr(i_line, "To talk in your clan's channel,") != 0)
	{
		io_inputData->type = WELCOME;
	}
	else if (strstr(i_line, "has reached a total") != 0)
	{
		io_inputData->type = TOTAL_LEVEL;
	}
	else if (strstr(i_line, "has reached combat level") != 0)
	{
		io_inputData->type = COMBAT_LEVEL;
	}
	else if (strstr(i_line, "received a new collection log item:") != 0)
	{
		io_inputData->type = COLLECTION_LOG;
	}
	else if (strstr(i_line, "has achieved a new") != 0)
	{
		io_inputData->type = PERSONAL_BEST;
	}
	else if (strstr(i_line, "has reached ") != 0)
	{
		io_inputData->type = LEVEL;
	}
	else if (strstr(i_line, "has been defeated by ") != 0)
	{
		io_inputData->type = PVP_DEATH;
	}
	else if (strstr(i_line, "has defeated") != 0)
	{
		io_inputData->type = PVP_KILL;
	}
	else if (strstr(i_line, "has opened a loot key") != 0)
	{
		io_inputData->type = OPENED_PVP_KEY;
	}
	else if (strstr(i_line, "received a drop:") != 0)
	{
		io_inputData->type = DROP;
	}
	else if (strstr(i_line, "tier of rewards from Combat Achievements") != 0)
	{
		io_inputData->type = COMBAT_ACHIEVEMENT;
	}
	else if (strstr(i_line, "has been invited into the clan by ") != 0)
	{
		io_inputData->type = INVITE;
	}
	else if (strstr(i_line, "has left the clan.") != 0)
	{
		io_inputData->type = LEFT;
	}
	else if (strstr(i_line, "received a new collection log item: ") != 0)
	{
		io_inputData->type = COLLECTION_LOG;
	}
	else if (strstr(i_line, "has completed a quest: ") != 0)
	{
		io_inputData->type = QUEST;
	}
	else if (strstr(i_line, "has completed the") != 0)
	{
		io_inputData->type = DIARY;
	}
	else
	{
		io_inputData->type = INVALID;
	}
}

unsigned int
ParseMessageUsername(InputData* io_inputData, char* i_line)
{
	int firstSpaceIndex;
	
	if (io_inputData->type != MESSAGE && io_inputData->type != TAGGED_BOT)
	{
		return(0);
	}
	
	assert((firstSpaceIndex = ScanForToken(i_line, ':')) > 0);
	strncpy(io_inputData->player.name, i_line, firstSpaceIndex);
	
	return(firstSpaceIndex + 1);
}

void
ParseMessage(InputData* io_inputData, char* i_line,
             unsigned int i_cursor, unsigned int i_timestampTrimmed)
{
	if (io_inputData->type == MESSAGE || io_inputData->type == TAGGED_BOT)
	{
		/* +1 because the cursor is on the leading space character */
		strncpy(io_inputData->message, &i_line[i_cursor + 1],
		        strlen(&i_line[i_cursor + 1]) - 1);
	}
	else
	{
		strcpy(io_inputData->message, &i_line[i_timestampTrimmed]);
	}
}

void
ParseAnouncement(InputData* io_inputData)
{
	switch (io_inputData->type)
	{
		case INVALID:
			/*assert(0);*/
			break;
			
		case MESSAGE:
			break;
		
		case TAGGED_BOT:
			break;
			
		case WELCOME:
			break;
		case INVITE:
		{
			char thepersonwhohasbeeninvited[20] = { 0 };
			sscanf
			(
				io_inputData->message,
				"%s has been invited into the clan by %s.",
				thepersonwhohasbeeninvited,
				io_inputData->player.name
			);
			/* This is needed because its including the full stop in the username. */
			io_inputData->player.name
			[
				strlen(io_inputData->player.name) - 1
			] = 0;
		} break;
		case LEFT:
		{
			sscanf
			(
				io_inputData->message,
				"%s has left the clan.",
				io_inputData->player.name
			);
		} break;
		
		case PVP_DEATH:
		{
			char moneyLost[20] = { 0 };
			char killedBy[20]  = { 0 };
			/* What happens if it's 1gp or even nothing? */
			sscanf
			(
				io_inputData->message,
				"%s has been defeated by %s in The Wilderness and lost (%s) worth of loot.",
				io_inputData->player.name,
				killedBy,
				moneyLost
			);
		} break;
		case PVP_KILL:
		{
			char moneyLost[20] = { 0 };
			char killedBy[20]  = { 0 };
			/* What happens if it's 1gp or even nothing? */
			sscanf
			(
				io_inputData->message,
				"%s has defeated %s and received (%s) worth of loot!",
				io_inputData->player.name,
				killedBy,
				moneyLost
			);
		} break;
		case OPENED_PVP_KEY:
		{
			char moneyLost[20] = { 0 };
			/* What happens if it's 1gp or even nothing? */
			sscanf
			(
				io_inputData->message,
				"%s has opened a loot key worth %s coins!",
				io_inputData->player.name,
				moneyLost
			);
		} break;
		
		case DROP:
		{
			char mobnamethatithinkonlyappearsforwildybosses[20] = { 0 };
			char itemname[20] = { 0 }; /* sometimes includes quantity */
			char itemvalue[20] = { 0 };
			sscanf
			(
				io_inputData->message,
				"%s received a drop: %s (%s coins) from %s.",
				io_inputData->player.name,
				itemname, itemvalue, mobnamethatithinkonlyappearsforwildybosses
			);
		} break;
		case QUEST:
		{
			char questname[20] = { 0 };
			sscanf
			(
				io_inputData->message,
				"%s has completed a quest: %s",
				io_inputData->player.name,
				questname
			);
		} break;
		case DIARY:
		{
			char diarynameandtier[30] = { 0 };
			sscanf
			(
				io_inputData->message,
				"%s  has completed the %s.",
				io_inputData->player.name,
				diarynameandtier
			);
		} break;
		case COLLECTION_LOG:
		{
			char itemname[20] = { 0 };
			sscanf
			(
				io_inputData->message,
				"%s received a new collection log item: %s (",
				io_inputData->player.name,
				itemname
			);
		} break;
		case COMBAT_ACHIEVEMENT:
		{
			char tier[10] = { 0 };
			sscanf
			(
				io_inputData->message,
				"%s has unlocked the %s tier of rewards from Combat Achievements!",
				io_inputData->player.name,
				tier
			);
		} break;
		case LEVEL:
		{
			char skillname[20] = { 0 };
			int level;
			sscanf
			(
				io_inputData->message,
				"%s has reached %s level %d.",
				io_inputData->player.name,
				skillname,
				&level
			);
		} break;
		case TOTAL_LEVEL:
		{
			int level;
			sscanf
			(
				io_inputData->message,
				"%s has reached a total level of %d.",
				io_inputData->player.name,
				&level
			);
		} break;
		case COMBAT_LEVEL:
		{
			int level;
			sscanf
			(
				io_inputData->message,
				"%s has reached combat level %d.",
				io_inputData->player.name,
				&level
			);
		} break;
		case PERSONAL_BEST:
		{
			char time[20] = { 0 };
			char task[20] = { 0 };
			sscanf
			(
				io_inputData->message,
				"%s has achieved a new %s personal best: %s",
				io_inputData->player.name,
				task, time
			);
		} break;
		case InputDataType_Last:
			break;
	}
}

