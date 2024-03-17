
#include "Stream.h"
#include "Parse.h"
#include "../utils/Time.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#define BATCH_SIZE 20



typedef struct StreamContext
{
	FILE* file_handle;
	InputData last_message;
} StreamContext;



/* Read the file from bottom-up.
 * While it takes in a number argument, the file may not contain that
 * many lines, so the return value is the true count. */
unsigned int GetLines   (FILE*, char[][256], unsigned int);
/* Used for identifying the last message from the previous read-batch. 
 * It's not 100%, for instance if on the same second someone gets multiple
 * levels or multiple collection logs, etc., then this match function will not
 * work correctly but this should be fine. The only issue would be with 
 * tracking account statistics (I.E. leaderboard for most collection log slots
 * would not work). But on the other hand this reduces spam without any work.
 * */
bool         SameMessage(InputData*, InputData*);
/* Getting "~/" / user home directory. */
const char*  GetHomeDir ();



StreamContext*
StreamInit(const char* i_loggedInAs)
{
	StreamContext* result;
	char pathBuffer[150] = { 0 };
	const char* homeDir;
	
	result = malloc(sizeof(StreamContext));
	memset(result, 0, sizeof(StreamContext));
	
	homeDir = GetHomeDir();
	sprintf(pathBuffer, "%s/.runelite/chatlogs/%s/clan/latest.log",
	        homeDir, i_loggedInAs);
	
	result->file_handle = fopen(pathBuffer, "r");
	if (result->file_handle == NULL)
	{
		printf("Could not open file '%s'\n", pathBuffer);
		exit(-1);
	}
	
	result->last_message.date = DateTimeNow();
	
	return(result);
}

void
StreamProcess(StreamContext* io_streamContext)
{
	char linesRead[BATCH_SIZE][256] = { 0 };
	int  linesReadCount, i;
	
	i = 0;
	linesReadCount = GetLines(io_streamContext->file_handle,
	                          linesRead, BATCH_SIZE);
	
	while (i < linesReadCount)
	{
		InputData parsed;
		
		parsed = ParseInput(linesRead[i]);
		
		if (SameMessage(&parsed, &io_streamContext->last_message))
		{
			break;
		}
		
		if (DateTimeDiff(io_streamContext->last_message.date,
		                 parsed.date) <= 0)
		{
			printf("\t%s", linesRead[i]);
			/* TODO: do something with the line */
		}
		
		++i;
	}
	
	if (linesReadCount > 0)
	{
		io_streamContext->last_message = ParseInput(linesRead[0]);
	}
}

void
StreamClose(StreamContext* io_streamContext)
{
	fclose(io_streamContext->file_handle);
}



unsigned int
GetLines(FILE* i_fp, char o_lines[][256], unsigned int i_lineCount)
{
	unsigned int result;
	char         line[512] = { 0 };
	int          i, cursor, size;
	
	result = 0;
	fseek(i_fp, 0, SEEK_END);
	size = ftell(i_fp) - 1;
	
	i = size;
	cursor = 0;
	
	while(i >= 0)
	{
		char flipped[512] = { 0 };
		int  j;
		
		memset(line, 0, sizeof(char) * 512);
		cursor = j = 0;
		
		while (i >= 0 && cursor < 512)
		{
			fseek(i_fp, i, SEEK_SET);
			line[cursor] = fgetc(i_fp);
			--i;
			if (line[cursor] == '\n') break;
			cursor++;
		}
		
		/* Do not storage blank lines (or only containing '\n'). */
		if (strlen(line) <= 1) continue;
		
		while (j < (int) strlen(line))
		{
			flipped[j] = line[strlen(line) - 1 - j];
			++j;
		}
		
		strncpy(o_lines[result++], flipped, 255);
		if (result >= i_lineCount) break;
	}
	
	return(result);
}

bool
SameMessage(InputData* i_a, InputData* i_b)
{
	/* Just checking in the fastest order. */
	if (i_a->type != i_b->type)
	{
		return(false);
	}
	
	if (strcmp(i_a->player.name, i_b->player.name) != 0)
	{
		return(false);
	}
	
	if (strcmp(i_a->date.serialised_time, i_b->date.serialised_time) != 0)
	{
		return(false);
	}
	
	if (strcmp(i_a->message, i_b->message) != 0)
	{
		return(false);
	}
	
	return(true);
}

const char*
GetHomeDir()
{
	const char* homeDir;
	
	if ((homeDir = getenv("HOME")) == NULL)
	{
		homeDir = getpwuid(getuid())->pw_dir;
	}
	
	return homeDir;
}

