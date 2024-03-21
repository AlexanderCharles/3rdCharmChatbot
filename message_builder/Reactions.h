#ifndef _3CCB_MESSAGE_BUILDER_REACTIONS_
#define _3CCB_MESSAGE_BUILDER_REACTIONS_

#include "../input/Parse.h"

#define OUTPUT_MSG_S 200



struct ReactionContext;
struct ReactionInput;



void React(struct ReactionContext*, InputData*);

struct ReactionContext* ReactionsInit ();
void                    ReactionsClose();

/* I think whenever I add commands (not sure how this will be structured just
 * yet), these funcs will be in their own file, but for now they can be here. 
 * */
void BuildMessage(InputData*, struct ReactionInput*, char*);



#endif
