#ifndef _3CCB_INPUT_STREAM_
#define _3CCB_INPUT_STREAM_



struct StreamContext;



struct StreamContext* StreamInit   ();
void                  StreamProcess(struct StreamContext*);
void                  StreamClose  (struct StreamContext*);



#endif
