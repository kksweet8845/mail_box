#ifndef COM_APP_H
#define COM_APP_H

#include <stdbool.h>
#include <string.h>

#define QUEUE_l "queued"
#define UNQUEUE_l "unqueued"

/* Define the operation 1. Send 2.Recv */
#define SEND 0
#define RECV 1



static bool is_str_equal(const char* pc, const char* pcc)
{
    return (bool) strcmp(pc, pcc);
}


static bool parse_message(char* msg )
{

}

static bool create_app();





#endif  //ifndef COM_APP_H
