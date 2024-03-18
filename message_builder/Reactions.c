
#include "Reactions.h"

#include "../utils/String.h"

/* https://github.com/sheredom/json.h */
#include <json.h>

#include <assert.h>
#include <stdio.h>

#define INPUT_MSG_COUNT  50
#define OUTPUT_MSG_COUNT 200



typedef struct ReactionInput
{
	enum
	{
		ANOUNCEMENT_CODE,
		TRIGGER_MESSAGE,
		TRIGGER_MULTI_MESSAGES
	} input_type;
	
	union
	{
		struct
		{
			unsigned int count;
			char**       messages;
		} multi;
		
		char         message[INPUT_MSG_COUNT];
		unsigned int anouncement_code;
	} input;
	
	enum
	{
		SINGULAR_OUTPUT,
		MULTI_OUTPUT
	} output_type;
	
	union
	{
		struct
		{
			unsigned int count;
			char**       messages;
		} multi;
		
		char message[OUTPUT_MSG_COUNT];
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



void
React(ReactionContext* i_react, InputData* i_parsedLine)
{
	if (i_parsedLine->type > WELCOME &&
	    i_parsedLine->type < InputDataType_Last)
	{
		unsigned int i;
		
		i = 0;
		while (i < i_react->reaction_count)
		{
			if (i_react->reactions[i].input_type == ANOUNCEMENT_CODE &&
			    i_react->reactions[i].input.anouncement_code ==
			    i_parsedLine->type)
			{
				printf("%s\n", ProduceOutput(&i_react->reactions[i]));
				return;
			}
			++i;
		}
	}
	else if (i_parsedLine->type == TAGGED_BOT)
	{
		int index;
		
		index = MatchReactionToArrayIndex(i_react, i_parsedLine->message);
		if (index == -1) return;
		printf("'%s'\n", ProduceOutput(&i_react->reactions[index]));
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
	if (io_react->reactions->input_type == TRIGGER_MULTI_MESSAGES)
	{
		i = 0;
		while (i < (int) io_react->reactions->input.multi.count)
		{
			free(io_react->reactions->input.multi.messages[i]);
			i++;
		}
		free(io_react->reactions->input.multi.messages);
	}
	
	if (io_react->reactions->output_type == MULTI_OUTPUT)
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
			
			result.input_type = TRIGGER_MESSAGE;
			right = column->value->payload;
			strcpy(result.input.message, right->string);
			assert(left->string_size < INPUT_MSG_COUNT);
		}
		else if (column->value->type == json_type_array)
		{
			struct json_array_s*         right;
			struct json_array_element_s* current;
			int i;
			
			i = 0;
			result.input_type = TRIGGER_MULTI_MESSAGES;
			right = column->value->payload;
			
			result.input.multi.messages = malloc(sizeof(char*) * right->length);
			while (i < (int) right->length)
			{
				result.input.multi.messages[i] =
					malloc(sizeof(char) * INPUT_MSG_COUNT);
				memset(result.input.multi.messages[i], 0,
				       sizeof(char) * INPUT_MSG_COUNT);
				++i;
			}
			
			current = right->start;
			
			while (current != NULL)
			{
				struct json_string_s* input;
				
				assert(current->value->type == json_type_string);
				input = current->value->payload;
				
				assert(input->string_size < INPUT_MSG_COUNT);
				strcpy(result.input.multi.messages[result.input.multi.count++],
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
		struct json_string_s* right;
		
		result.input_type = ANOUNCEMENT_CODE;
		right = column->value->payload;
		assert(column->value->type == json_type_number);
		result.input.anouncement_code = atoi(right->string);
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
		
		result.output_type = SINGULAR_OUTPUT;
		right = column->value->payload;
		assert(right->string_size < OUTPUT_MSG_COUNT);
		strcpy(result.output.message, right->string);
	}
	else if (column->value->type == json_type_array)
	{
		struct json_array_s*         right;
		struct json_array_element_s* current;
		int i;
		
		i = 0;
		result.output_type = MULTI_OUTPUT;
		right = column->value->payload;
		
		result.output.multi.messages = malloc(sizeof(char*) * right->length);
		while (i < (int) right->length)
		{
			result.output.multi.messages[i] =
				malloc(sizeof(char) * OUTPUT_MSG_COUNT);
			memset(result.output.multi.messages[i], 0,
			       sizeof(char) * OUTPUT_MSG_COUNT);
			++i;
		}
		
		current = right->start;
		
		while (current != NULL)
		{
			struct json_string_s* output;
			
			assert(current->value->type == json_type_string);
			output = current->value->payload;
			
			assert(output->string_size < OUTPUT_MSG_COUNT);
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
	if (i_reactInput->output_type == SINGULAR_OUTPUT)
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

int
MatchReactionToArrayIndex(ReactionContext* i_react,
                          const char* i_msg)
{
	unsigned int i;
	char* start;
	
	i = 0;
	assert((start = GetStartOfMessage((char*) i_msg)) != NULL);
	
	while (i < i_react->reaction_count)
	{
		if (i_react->reactions[i].input_type == TRIGGER_MESSAGE)
		{
			if (stristr(i_msg, i_react->reactions[i].input.message) != NULL)
			{
				return(i);
			}
		}
		else if (i_react->reactions[i].input_type == TRIGGER_MULTI_MESSAGES)
		{
			unsigned int j;
			
			j = 0;
			while (j < i_react->reactions[i].input.multi.count)
			{
				if (stristr(i_msg,
				    i_react->reactions[i].input.multi.messages[j]) != NULL)
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


