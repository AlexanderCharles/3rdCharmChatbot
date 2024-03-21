
#include "Reactions.h"

#include "../utils/String.h"
#include "../utils/Keyboard.h"

/* https://github.com/sheredom/json.h */
#include <json.h>

#include <assert.h>
#include <stdio.h>

#define INPUT_MSG_S 50

#define DBG_NO_KEYBOARD 0

/* This could be improved by adding a fuzzyfinder */



typedef struct ReactionInput
{
	enum
	{
		ANOUNCEMENT_CODE_SINGULAR,
		ANOUNCEMENT_CODE_MULTI,
		
		MESSAGE_SINGULAR,
		MESSAGE_MULTI
	} input_type;
	
	union
	{
		struct
		{
			unsigned int count;
			union
			{
				char**        messages;
				unsigned int* anouncement_codes;
			} v;
		} multi;
		
		char         message[INPUT_MSG_S];
		unsigned int anouncement_code;
	} input;
	
	enum
	{
		OUTPUT_SINGULAR,
		OUTPUT_MULTI
	} output_type;
	
	union
	{
		struct
		{
			unsigned int count;
			char**       messages;
		} multi;
		
		char message[OUTPUT_MSG_S];
	} output;
	
} ReactionInput;

typedef struct ReactionContext
{
	ReactionInput* reactions;
	unsigned int   reaction_count;
} ReactionContext;



char*          ReadWholeFile    (const char*);
ReactionInput* ReadReactionJSON (const char*, unsigned int*);
ReactionInput  ReadReactionInput(struct json_array_element_s*);

char* ProduceOutput            (ReactionInput*);
int   MatchReactionToArrayIndex(ReactionContext*, const char*);
char* GetStartOfMessage        (char*);



void
React(ReactionContext* i_react, InputData* i_parsedLine)
{
	unsigned int i;
	char messageBuffer[OUTPUT_MSG_S] = { 0 };
	
	if (i_parsedLine->type == TAGGED_BOT)
	{
		int index;
		
		index = MatchReactionToArrayIndex(i_react, i_parsedLine->message);
		if (index == -1) return;
		
		BuildMessage(i_parsedLine, &i_react->reactions[index], messageBuffer);
#if DBG_NO_KEYBOARD
		printf("%s\n", messageBuffer);
#else
		WriteMessage(messageBuffer);
#endif
	}
	else if (i_parsedLine->type <= WELCOME ||
	         i_parsedLine->type >= InputDataType_Last)
	{
		return;
	}
	
	i = 0;
	while (i < i_react->reaction_count)
	{
		if (i_react->reactions[i].input_type == ANOUNCEMENT_CODE_SINGULAR &&
		    i_react->reactions[i].input.anouncement_code ==
		    i_parsedLine->type)
		{
			BuildMessage(i_parsedLine, &i_react->reactions[i], messageBuffer);
#if DBG_NO_KEYBOARD
			printf("%s\n", messageBuffer);
#else
			WriteMessage(messageBuffer);
#endif
			return;
		}
		else if (i_react->reactions[i].input_type == ANOUNCEMENT_CODE_MULTI)
		{
			unsigned int j;
			
			j = 0;
			while (j < i_react->reactions[i].input.multi.count)
			{
				if (i_react->reactions[i].input.multi.v.anouncement_codes[j] ==
				    i_parsedLine->type)
				{
					BuildMessage(i_parsedLine, &i_react->reactions[i],
					             messageBuffer);
#if DBG_NO_KEYBOARD
					printf("%s\n", messageBuffer);
#else
					WriteMessage(messageBuffer);
#endif
					return;
				}
				++j;
			}
		}
		++i;
	}
}



ReactionContext*
ReactionsInit()
{
	ReactionContext* result;
	char*            fileData;
	
	result = malloc(sizeof(ReactionContext));
	memset(result, 0, sizeof(ReactionContext));
	
	fileData = ReadWholeFile("Data/reactions.json");
	result->reactions = ReadReactionJSON(fileData, &result->reaction_count);
	
	srand(time(NULL));
	
	return(result);
}

void
ReactionsClose(ReactionContext* io_react)
{
	int i;
	i = 0;
	while (i < (int) io_react->reactions->input.multi.count)
	{
		if (io_react->reactions->input_type == MESSAGE_MULTI)
		{
			free(io_react->reactions->input.multi.v.messages[i]);
		}
		if (io_react->reactions->input_type == ANOUNCEMENT_CODE_MULTI)
		{
			free(io_react->reactions->input.multi.v.anouncement_codes);
		}
		i++;
	}
	free(io_react->reactions->input.multi.v.messages);
	
	if (io_react->reactions->output_type == OUTPUT_MULTI)
	{
		i = 0;
		while (i < (int) io_react->reactions->output.multi.count)
		{
			free(io_react->reactions->output.multi.messages[i]);
			i++;
		}
		free(io_react->reactions->output.multi.messages);
	}
	
	free(io_react);
}



/* TODO: split the message up by adding newline chars, then handle this in
 * Keyboard.c.
 * There are a few other things too, i think i have more notes in Keyboard.c
 * to go through, a few of them pertain to this func
 * its possible backslashes could cause issues lol */
void
BuildMessage(InputData* i_parsedLine, ReactionInput* i_reaction,
             char* o_msg)
{
	strcpy(o_msg, ProduceOutput(i_reaction));
	if (strstr(o_msg, "<BOT_NAME>") != 0)
	{
		strr(o_msg, "<BOT_NAME>", BOT_NAME);
	}
	if (strstr(o_msg, "<PLAYER_NAME>") != 0)
	{
		strr(o_msg, "<PLAYER_NAME>", i_parsedLine->player.name);
	}
}



char*
ReadWholeFile(const char* i_fname)
{
	char*  fileData, *line;
	FILE*  fp;
	size_t characterCount;
	
	assert((fp = fopen(i_fname, "r")) != NULL);
	
	fseek(fp, 0, SEEK_END);
	characterCount = ftell(fp) + 1;
	rewind(fp);
	
	fileData = malloc(sizeof(char) * characterCount);
	line     = malloc(sizeof(char) * characterCount);
	memset(fileData, 0, sizeof(char) * characterCount);
	memset(line,     0, sizeof(char) * characterCount);
	
	while (fgets(line, characterCount, fp) != NULL)
	{
		strcat(fileData, line);
		memset(line, 0, sizeof(char) * characterCount);
	}
	
	free(line);
	fclose(fp);
	
	return(fileData);
}

ReactionInput*
ReadReactionJSON(const char* i_json, unsigned int* o_count)
{
	ReactionInput* result;
	
	struct json_value_s*          root;
	struct json_object_s*         data;
	struct json_object_element_s* table;
	struct json_array_s*          tableElement;
	struct json_array_element_s*  row;
	
	root = json_parse_ex
	(
		i_json, strlen(i_json),
		json_parse_flags_allow_json5,
		0, 0, 0
	);
	assert(root->type == json_type_object);
	assert(data = root->payload);
	
	assert(table = data->start);
	
	assert(table->value->type == json_type_array);
	tableElement = table->value->payload;
	
	result = malloc(sizeof(ReactionInput) * tableElement->length);
	memset(result, 0, sizeof(ReactionInput) * tableElement->length);
	
	row = tableElement->start;
	*o_count = 0;
	
	while (row != NULL)
	{
		result[*o_count] = ReadReactionInput(row);
		row = row->next;
		*o_count += 1;
	}
	
	free(root);
	
	return(result);
}

ReactionInput
ReadReactionInput(struct json_array_element_s* i_rowCursor)
{
	ReactionInput result = { 0 };
	
	struct json_object_s*         columnData;
	struct json_object_element_s* column;
	struct json_string_s*         left;
	
	columnData = i_rowCursor->value->payload;
	column     = columnData->start;
	left       = column->name;
	
	if (strcmp(left->string, "input") == 0)
	{
		if (column->value->type == json_type_string)
		{
			struct json_string_s* right;
			
			result.input_type = MESSAGE_SINGULAR;
			right = column->value->payload;
			strcpy(result.input.message, right->string);
			assert(left->string_size < INPUT_MSG_S);
		}
		else if (column->value->type == json_type_array)
		{
			struct json_array_s*         right;
			struct json_array_element_s* current;
			int i;
			
			i = 0;
			result.input_type = MESSAGE_MULTI;
			right = column->value->payload;
			
			result.input.multi.v.messages = malloc(sizeof(char*) *
			                                       right->length);
			while (i < (int) right->length)
			{
				result.input.multi.v.messages[i] =
					malloc(sizeof(char) * INPUT_MSG_S);
				memset(result.input.multi.v.messages[i], 0,
				       sizeof(char) * INPUT_MSG_S);
				++i;
			}
			
			current = right->start;
			
			while (current != NULL)
			{
				struct json_string_s* input;
				
				assert(current->value->type == json_type_string);
				input = current->value->payload;
				
				assert(input->string_size < INPUT_MSG_S);
				strcpy(result.input.multi.v.messages[result.input.multi.count++],
				       input->string);
				
				current = current->next;
			}
		}
		else
		{
			printf("Invalid input type\n");
		}
	}
	else if (strcmp(left->string, "input_code") == 0)
	{
		if (column->value->type == json_type_number)
		{
			struct json_string_s* right;
			
			result.input_type = ANOUNCEMENT_CODE_SINGULAR;
			right = column->value->payload;
			assert(column->value->type == json_type_number);
			result.input.anouncement_code = atoi(right->string);
		}
		else if (column->value->type == json_type_array)
		{
			struct json_array_s*         right;
			struct json_array_element_s* current;
			
			result.input_type = ANOUNCEMENT_CODE_MULTI;
			right = column->value->payload;
			
			result.input.multi.v.anouncement_codes =
				malloc(sizeof(unsigned int) * right->length);
			memset(result.input.multi.v.anouncement_codes, 0,
			       sizeof(unsigned int) * right->length);
			
			current = right->start;
			
			while (current != NULL)
			{
				struct json_string_s* input;
				
				assert(current->value->type == json_type_number);
				input = current->value->payload;
				
				result.input.multi.v.anouncement_codes[result.input.multi.count++] =
					atoi(input->string);
				
				current = current->next;
			}
		}
	}
	else
	{
		printf("Invalid table\n");
	}
	
	column = column->next;
	left = column->name;
	
	if (column->value->type == json_type_string)
	{
		struct json_string_s* right;
		
		result.output_type = OUTPUT_SINGULAR;
		right = column->value->payload;
		assert(right->string_size < OUTPUT_MSG_S);
		strcpy(result.output.message, right->string);
	}
	else if (column->value->type == json_type_array)
	{
		struct json_array_s*         right;
		struct json_array_element_s* current;
		int i;
		
		i = 0;
		result.output_type = OUTPUT_MULTI;
		right = column->value->payload;
		
		result.output.multi.messages = malloc(sizeof(char*) * right->length);
		while (i < (int) right->length)
		{
			result.output.multi.messages[i] =
				malloc(sizeof(char) * OUTPUT_MSG_S);
			memset(result.output.multi.messages[i], 0,
			       sizeof(char) * OUTPUT_MSG_S);
			++i;
		}
		
		current = right->start;
		
		while (current != NULL)
		{
			struct json_string_s* output;
			
			assert(current->value->type == json_type_string);
			output = current->value->payload;
			
			assert(output->string_size < OUTPUT_MSG_S);
			strcpy(result.output.multi.messages[result.output.multi.count++],
			       output->string);
			
			current = current->next;
		}
	}
	
	return(result);
}



char*
ProduceOutput(ReactionInput* i_reactInput)
{
	char* result;
	
	assert(i_reactInput);
	if (i_reactInput->output_type == OUTPUT_SINGULAR)
	{
		return(i_reactInput->output.message);
	}
	else
	{
		int randomIndex;
		randomIndex = rand() % i_reactInput->output.multi.count;
		return(i_reactInput->output.multi.messages[randomIndex]);
	}
	
	assert(result != NULL);
	return(result);
}


int
MatchReactionToArrayIndex(ReactionContext* i_react,
                          const char* i_msg)
{
	unsigned int i;
	char*        start;
	
	i = 0;
	assert((start = GetStartOfMessage((char*) i_msg)) != NULL);
	
	while (i < i_react->reaction_count)
	{
		if (i_react->reactions[i].input_type == MESSAGE_SINGULAR)
		{
			if (stristr(i_msg, i_react->reactions[i].input.message) != NULL)
			{
				return(i);
			}
		}
		else if (i_react->reactions[i].input_type == MESSAGE_MULTI)
		{
			unsigned int j;
			
			j = 0;
			while (j < i_react->reactions[i].input.multi.count)
			{
				if (stristr(i_msg,
				    i_react->reactions[i].input.multi.v.messages[j]) != NULL)
				{
					return(i);
				}
				++j;
			}
		}
		++i;
	}
	
	return(-1);
}

char*
GetStartOfMessage(char* io_msg)
{
	int l, bnl, index;
	
	l   = strlen(io_msg);
	bnl = strlen(BOT_NAME);
	if (l < (bnl + 2)) return (NULL);
	
	index = ScanForToken(io_msg, ' ');
	return(&io_msg[index + 1]);
}


