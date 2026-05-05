typedef struct { 
	unsigned long long*   unix_ms_timestamps; /*  */
	unsigned char** messages;
	unsigned int* senders;
	unsigned short* message_lengths; 
	unsigned short* heights;  /* height at the current width of the message space */
	unsigned int cap; // TODO enforce expected cap = 1024
	unsigned int start;
	unsigned int length;
} Channel;

typedef struct {
	Channel* channels;
	char** channel_names;
	unsigned int* channel_update_times; /* time of the last update for each channel */
	unsigned int* channel_order; /* can be sorted according to channel_update_times [0] = 5 -> 5th channel is at the top */
} ChannelGroup;

typedef struct {
	ChannelGroup* groups;
	char** group_names;
} Server;


typedef struct {
	int y; /* y coord of the bottom of the screen, the screen moves when the user scrolls */
	Server* servers;
	unsigned int server;
	unsigned int group;
	unsigned int channel;
	unsigned int bottom_message; /* reference message for height, the one at the bottom of the screen */
	unsigned int bottom_message_y; /* y coord of the bottom of the bottom message */
	unsigned int message_space_height;
	unsigned int message_space_width;
} Client;

void frame(Client client) {
	int i, dist_start, message_y, target_y, res;
	Channel* cur_channel;
	ChannelGroup cur_group;
	Server cur_server;
	unsigned int cap, start, length, top_message;
	unsigned int* heights;
	char failed_to_query_messages;
	/* returned from new message querying */
	unsigned long long*   ret_unix_ms_timestamps;
	unsigned char** ret_messages;
	unsigned int* ret_senders;
	unsigned short* ret_message_lengths; 

	failed_to_query_messages = 0;

	/* Update the state according to inputs*/
	client.y += 10; /* scroll */

	/* Update the backend: fetch new messages... */


	
	cur_server = client.servers[client.server];
	cur_group = cur_server.groups[client.group];
	cur_channel= &cur_group.channels[client.channel];

	/* Find all the messages that need to be drawn, and query new ones if needed */
	/* Find the last message (at the top of the screen) */
	heights = cur_channel->heights;
	cap = cur_channel->cap;
	start = cur_channel->start;
	length = cur_channel->length;

	if (length > 0) 
	{
		message_y = client.bottom_message_y;
		i = client.bottom_message;
		dist_start = i - start + 1;
		message_y += heights[i]; 

		target_y = client.y + client.message_space_height;
		while (message_y < target_y)
		{
			i++;
			dist_start++;
			if (dist_start >= length)
			{
				/* Query more messages if we need more */
				res = query_older_messages_from_backend(64, &ret_unix_ms_timestamps, &ret_messages, &ret_senders, &ret_message_lengths);
				if (res == -1)
				{
					// Couldn't query more message -> no network
					failed_to_query_messages = 1;
				}
				else if (res == 0)
				{
					break;
				}
				else
				{
					/* Update the circular buffer, set the old messages to freed */
					/* res = number of messages returned */
					// TODO
				}
			}
			if (i >= cap)
			{
				i = 0;
			}
			message_y += heights[i];
		}
		top_message = i;
	}
	/* Find the message at the bottom of the screen if it changed */
	heights = cur_channel->heights;
	cap = cur_channel->cap;
	start = cur_channel->start;
	length = cur_channel->length;

	if (length > 0) 
	{
		message_y = client.bottom_message_y;
		i = client.bottom_message;
		dist_start = i - start + 1;
		message_y += heights[i]; 
		while (message_y < client.y && dist_start < length)
		{
			i++;
			dist_start++;
			if (i >= cap)
			{
				i = 0;
			}
			message_y += heights[i];
			
		}
		client.bottom_message = i;
		client.bottom_message_y = message_y - heights[i];
	}
	
	
}
