#ifndef _3CCB_MESSAGE_BUILDER_REACTIONS_
#define _3CCB_MESSAGE_BUILDER_REACTIONS_

#include "../input/Parse.h"



struct ReactionContext;



void React(struct ReactionContext*, InputData*);

struct ReactionContext* ReactionsInit ();
void                    ReactionsClose();


#endif
