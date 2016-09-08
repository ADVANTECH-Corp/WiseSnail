#ifndef _CAGENT_TOPIC_H_
#define _CAGENT_TOPIC_H_

typedef struct topic_entry
{    
	char name[128];
	void* callback_func;
	struct topic_entry *prev;
	struct topic_entry *next;
} topic_entry_st;

struct topic_entry * topic_first(topic_entry_st* topiclist);
struct topic_entry * topic_last(topic_entry_st* topiclist);
struct topic_entry * topic_add(topic_entry_st** topiclist, char const * topicname, void* cbfunc);
void topic_remove(topic_entry_st** topiclist, char* topicname);
struct topic_entry * topic_find(topic_entry_st* topiclist, char const * topicname);

#endif