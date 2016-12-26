#include "topic.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

struct topic_entry * topic_first(topic_entry_st* topiclist)
{
	struct topic_entry *target = topiclist;
	return target;
}

struct topic_entry * topic_last(topic_entry_st* topiclist)
{
	struct topic_entry *topic = topiclist;
	struct topic_entry *target = NULL;
	while(topic != NULL)
	{
		target = topic;
		topic = topic->next;
	}
	return target;
}

struct topic_entry * topic_add(topic_entry_st** topiclist, char const * topicname, void* cbfunc)
{
	struct topic_entry *topic = NULL;
	
	topic = (struct topic_entry *)malloc(sizeof(*topic));
	
	if (topic == NULL)
		return NULL;

	strncpy(topic->name, topicname, strlen(topicname)+1);
	topic->callback_func = cbfunc;
	topic->next = NULL;	
	topic->prev = NULL;	

	if(*topiclist == NULL)
	{
		*topiclist = topic;
	} else {
		struct topic_entry *lasttopic = topic_last(*topiclist);
		lasttopic->next = topic;
		topic->prev = lasttopic;
	}
	return topic;
}

void topic_remove(topic_entry_st** topiclist, char* topicname)
{
	struct topic_entry *topic = *topiclist;
	struct topic_entry *target = NULL;
	while(topic != NULL)
	{
		if(strcmp(topic->name, topicname) == 0)
		{
			if(*topiclist == topic)
				*topiclist = topic->next;
			if(topic->prev != NULL)
				topic->prev->next = topic->next;
			if(topic->next != NULL)
				topic->next->prev = topic->prev;
			target = topic;
			break;
		}
		topic = topic->next;
	}
	if(target!=NULL)
	{
		free(target);
		target = NULL;
	}
}

struct topic_entry * topic_find(topic_entry_st* topiclist, char const * topicname)
{
	struct topic_entry *topic = topiclist;
	struct topic_entry *target = NULL;

	if(topicname == NULL) return target;

	while(topic != NULL)
	{
		if(topic->name == NULL || strlen(topic->name)<=0)
		{
		}
		else if(strchr(topic->name, '+')>0)
		{
			if(strchr(topicname, '+')>0)
			{
				if(strcmp(topic->name, topicname) == 0)
				{
					target = topic;
					break;
				}
			}
			else
			{
				char *delim = "+";
				char *p = NULL;
				char tName[128] = {0};
				bool match = true;
				char* ss = (char *)topicname;
				strcpy(tName, topic->name);
				p = strtok(tName, delim);
				while(p)
				{
					ss = strstr(ss, p);
					if(ss > 0)
						ss += strlen(p);
					else
					{
						match = false;
						break;
					}
					p=strtok(NULL,delim);
				}
				if(match)
					target = topic;
			}
		}
		else
		{
			if(strcmp(topic->name, topicname) == 0)
			{
				target = topic;
				break;
			}
		}
		topic = topic->next;
	}
	return target;
}