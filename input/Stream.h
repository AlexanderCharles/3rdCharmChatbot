#ifndef _3CCB_INPUT_STREAM_
#define _3CCB_INPUT_STREAM_

#include "../message_builder/Reactions.h"



struct StreamContext;



struct StreamContext* StreamInit   ();
void                  StreamProcess(struct StreamContext*,
                                    struct ReactionContext*);
void                  StreamClose  (struct StreamContext*);



#endif
