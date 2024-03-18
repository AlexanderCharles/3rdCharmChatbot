#ifndef _3CCB_INPUT_PARSE_
#define _3CCB_INPUT_PARSE_

#include "../utils/Time.h"

/* The "tagged" name, not the actual account name. */
#define BOT_NAME "bot"



typedef struct InputData
{
	DateTime date;
	
	struct Player
	{
		/* https://runescape.wiki/w/Character_name#Limitations */
		char name[13];
		
		/* This is the account type of the messsage. This variable will not keep
		 * track of de-iron-ing and changing from a speedrunning world to a 
		 * regular one. It does not care. If I handle this at any point then it
		 * will not be in file. */
		enum AccountType
		{
			NORMAL = 0,
			IM     = 2, HCIM = 10,
			UIM    = 3,
			LEAGUE = 22, /* No way to test this. */
			GIM    = 41,
			HCGIM  = 42, UGIM = 43,
			SPEED  = 52
		} account_type;
	} player;
	
	/* This could contain the username, so the buffer size must be larger than
	 * the in-game maximum message size. */
	char message[150];
	
	enum InputDataType
	{
		INVALID = 0,
		
		/* Messages (Reactions) and potentially Commands. */
		MESSAGE,
		TAGGED_BOT,
		
		/* Meta / Clan Anouncements. */
		WELCOME,
		INVITE, LEFT,
		
		/* Pvp. */
		PVP_DEATH, PVP_KILL,
		OPENED_PVP_KEY,
		
		/* Achievement Anouncements. */
		DROP,
		QUEST, DIARY,
		COLLECTION_LOG, COMBAT_ACHIEVEMENT,
		LEVEL, TOTAL_LEVEL, COMBAT_LEVEL,
		PERSONAL_BEST,
		
		/* Any new values need to be entered here. */
		
		/* This is here because if more InputDataType values are added, it would
		 * otherwise require other parts of the codebase to be updated. */
		InputDataType_Last
	} type;
	
	union AnouncementData
	{
		char invited_username[15];
		
		/* I do not see any reason why drops should not including the quantity. */
		struct
		{
			char name_and_quantity[30];
			char item_value[15];
			char boss_name[20];
		} drop;
		
		char quest_name[50];
		
		struct
		{
			enum
			{
				Ardougne,
				Desert,
				Falador,
				Fremennik,
				Kandarin,
				Karamja,
				KourendKebos,
				LumbridgeDraynor,
				Morytania,
				Varrock,
				WesternProvinces,
				Wilderness
			} region;
			enum
			{
				EASY, MEDIUM, HARD, ELITE
			} tier;
		} diary;
		
		char collection_log_item[30];
		
		enum
		{
			/* < 5 is are already defined in diary tiers. */
			MASTER = 5, GRANDMASTER = 6
		} combat_achievement_tier;
		
		unsigned int level;
		
		struct
		{
			char time[10];
			char activity[30];
		} personal_best;
		
	} anouncement_data;
	
} InputData;



InputData ParseInput(char*);



#endif
