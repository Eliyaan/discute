typedef struct { 
	unsigned long long*   unix_ms_timestamps; /*  */
	unsigned char** messages;
	unsigned int* senders;
	unsigned short* message_lengths; 
	unsigned short* heights;  /* height at the current width of the message space */
	unsigned int cap; // TODO enforce expected cap = 1024, at least > 64 (fetch size for the moment)
	unsigned int start;
	unsigned int length;
} Channel;

typedef struct {
	Channel* channels;
	char** channel_names;
	char* channel_unreads; /* TODO something cleaner once there will be more bools */
	// TODO scrolls
	int* channel_ids; /* ids must be delivered by the backend in ascending order, and stored here likewise */
	unsigned int length;
	unsigned int cap;

/*
	unsigned int* channel_update_times; /* time of the last update for each channel */
	unsigned int* channel_order; /* can be sorted according to channel_update_times [0] = 5 -> 5th channel is at the top */
*/
} ChannelGroup;

typedef struct {
	ChannelGroup* groups;
	char**	group_names;
	char*   group_unreads; /* TODO something cleaner once there will be more bools */
	int* 	group_ids; /* ids must be delivered by the backend in ascending order, and stored here likewise */
	unsigned int length;
	unsigned int cap;
} Server;



enum UpdateTypes {
	UPDATE_servers = 1;
	UPDATE_groups = 1 << 1;
	UPDATE_channels = 1 << 2;
	UPDATE_messages = 1 << 3;
}

typedef struct {
	int server_id;
	int group_id;
	int channel_id;
} MessageUpdate;

typedef struct {
	int server_id;
	int group_id;
	int channel_id;
	char*	channel_name;
} ChannelUpdate;

typedef struct {
	int server_id;
	int group_id;
	char*	group_name;
} GroupUpdate;

typedef struct {
	int server_id;
	char* server_name;
} ServerUpdate;

typedef struct {
	int y; /* y coord of the bottom of the screen, the screen moves when the user scrolls */
	Server* servers;
	int* 	server_ids; /* ids must be delivered by the backend in ascending order, and stored here likewise */
	char*   server_unreads; /* TODO something cleaner once there will be more bools */
	// TODO scrolls
	char** 	server_names;
	unsigned int length; /* Server array length */
	unsigned int cap; /* Server array cap */
	unsigned int server;
	unsigned int group;
	unsigned int channel;
	unsigned int bottom_message; /* reference message for height, the one at the bottom of the screen */
	unsigned int bottom_message_y; /* y coord of the bottom of the bottom message */
	unsigned int message_area_height;
	unsigned int message_space_width;
} Client;

/* Returns -1 if not found */
int dichotomy_int(int searched, int* array, unsigned int length)
{
	int i, begin, end, res;
	begin = 0;
	end = length - 1;
	i = (begin + end) >> 1;
	res = array[i];
	while (searched != res)
	{
		if (searched > res)
		{
			begin = i + 1;
		}
		else
		{
			end = i - 1;
		}
		if (begin > end)
		{
			i = -1;
			break;
		}	
		i = (begin + end) >> 1;
		res = array[i];
	}
	return i;
} 

Client init(Client client)
{
	// TODO
}

Client frame(Client client)
{
	int i, dist_start, message_y, target_y, res, count, first_part_len, second_part_len, overwrite_len;
	int server_i, group_i, write_i, bottom_of_current_channel;
	Channel* cur_channel;
	ChannelGroup cur_group;
	Server cur_server;
	unsigned int cap, start, length, top_message;
	unsigned int* heights;
	char failed_to_query_messages;
	ServerUpdate server_update;
	GroupUpdate group_update;
	ChannelUpdate group_update;
	MessageUpdate message_update;
	/* returned from new message querying */
	unsigned long long*   ret_unix_ms_timestamps;
	unsigned char** ret_messages;
	unsigned int* ret_senders;
	unsigned short* ret_message_lengths; 

	failed_to_query_messages = 0;

	/* Update the state according to inputs*/
	client.y += 10; /* scroll TODO user input */

	/* Update the backend: fetch new informations.. and update the frontend accordingly (create new channels if needed) */ // TODO
	res = backend_updates_fetch();
	if (res != 0)
	{
		/* Updates to the main data structures' metadata (~names and children) : servers, channel groups, channels */
		if (res & UPDATE_servers)
		{
			count = 1;
			/* count = how many are left to fetch, often 0 */ 
			while (count != 0)
			{
				count = backend_server_update_fetch(&server_update); /* count is often 0 */
				/* Check if it is an already existing server */
				i = dichotomy_int(server_update.server_id, client.server_ids, client.length)
				if (i == -1) 
				{
					/* New -> create data */
					if (client.length == client.cap)
					{
						/* Grow arrays */
						client.cap = (client.cap + 1) * 3 / 2;
						realloc(client.server_ids, (sizeof *client.server_ids) * client.cap);
						realloc(client.server_names, (sizeof *client.server_names) * client.cap);
						realloc(client.server_unreads, (sizeof *client.server_unreads) * client.cap);
						realloc(client.servers, (sizeof *client.servers) * client.cap);
					}
					client.server_ids[client.length] = server_update.server_id;
					client.server_names[client.length] = server_update.server_name;
					client.server_unreads[client.length] = 0;
					client.servers[client.length].cap = 0;
					client.servers[client.length].length = 0;
					/* The other fields will get filled by the next updates for groups etc.. */
					client.length++;
				}
				else
				{
					backend_server_name_mark_unused(client.server_names[i], i);
					client.server_names[i] = server_update.server_name;
				}
			}
		}
		if (res & UPDATE_groups)
		{
			count = 1;
			/* count = how many are left to fetch, often 0 */ 
			while (count != 0)
			{
				count = backend_group_update_fetch(&group_update); /* count is often 0 */
				/* Check if it is an already existing group */
				server_i = dichotomy_int(group_update.server_id, client.server_ids, client.length)
				if (server_i == -1) 
					error(Server not found in group update);
				}
				cur_server = client.servers[server_i];
				i = dichotomy_int(group_update.group_id, cur_server.groups, cur_server.length);
				if (i == -1) 
				{
					/* New -> create data */
					if (cur_server.length == cur_server.cap)
					{
						/* Grow arrays */
						cur_server.cap = (cur_server.cap + 1) * 3 / 2;
						realloc(cur_server.group_ids, (sizeof *cur_server.group_ids) * cur_server.cap);
						realloc(cur_server.group_names, (sizeof *cur_server.group_names) * cur_server.cap);
						realloc(cur_server.group_unreads, (sizeof *cur_server.group_unreads) * cur_server.cap);
						realloc(cur_server.groups, (sizeof *cur_server.groups) * cur_server.cap);
					}
					cur_server.group_ids[cur_server.length] = group_update.group_id;
					cur_server.group_names[client.length] = group_update.group_name;
					cur_server.group_unreads[client.length] = 0;
					cur_server.groups[cur_server.length].cap = 0;
					cur_server.groups[cur_server.length].length = 0;
					/* The other fields will get filled by the next updates for groups etc.. */
					cur_server.length++;
				}
				else
				{
					backend_group_name_mark_unused(cur_server.group_names[i], server_i, i);
					cur_server.group_names[i] = group_update.group_name;
				}
				client.servers[server_i] = cur_server;
			}
		} 
		if (res & UPDATE_channels)
		{
			count = 1;
			/* count = how many are left to fetch, often 0 */ 
			while (count != 0)
			{
				count = backend_channel_update_fetch(&channel_update); /* count is often 0 */
				/* Check if it is an already existing channel */
				server_i = dichotomy_int(channel_update.server_id, client.server_ids, client.length)
				if (server_i == -1)
					error(Server not found in group update);
				}
				cur_server = client.servers[server_i];
				group_i = dichotomy_int(channel_update.group_id, cur_server.groups, cur_server.length);
				if (group_i == -1) 
					error(Group not found in channel update);
				}
				cur_group = cur_server.groups[group_i];
				i = dichotomy_int(channel_update.channel_id, cur_group.channels, cur_group.length);
				if (i == -1) 
				{
					/* New -> create data */
					if (cur_group.length == cur_group.cap)
					{
						/* Grow arrays */
						cur_group.cap = (cur_group.cap + 1) * 3 / 2;
						realloc(cur_group.channel_ids, (sizeof *cur_group.channel_ids) * cur_group.cap);
						realloc(cur_group.channel_names, (sizeof *cur_group.channel_names) * cur_group.cap);
						realloc(cur_group.channel_unreads, (sizeof *cur_group.channel_unreads) * cur_group.cap);
						realloc(cur_group.channels, (sizeof *cur_group.channels) * cur_group.cap);
					}
					cur_group.channel_ids[cur_group.length] = channel_update.channel_id;
					cur_group.channel_names[client.length] = channel_update.channel_name;
					cur_group.channel_unreads[client.length] = 0;
					cur_group.channels[cur_group.length].cap = 0;
					cur_group.channels[cur_group.length].length = 0;
					/* The other fields will get filled by the next updates for channels etc.. */
					cur_group.length++;
				}
				else
				{
					backend_channel_name_mark_unused(cur_group.channel_names[i], server_i, group_i, i);
					cur_group.channel_names[i] = channel_update.channel_name;
				}
				cur_server.groups[group_i] = cur_group;
			}
		}
		/* New messages */
		if (res & UPDATE_messages)
		{
			count = 1;
			/* count = how many are left to fetch, often 0 */ 
			while (count != 0)
			{
				count = backend_message_update_fetch(&message_update); /* count is often 0 */
				TODO
			}
			
		}
	}
	

	
	cur_server = client.servers[client.server];
	cur_group = cur_server.groups[client.group];
	cur_channel= &cur_group.channels[client.channel];

	/* Find all the messages that need to be drawn, and query new ones if needed */
	heights = cur_channel->heights;
	cap = cur_channel->cap;
	start = cur_channel->start;
	length = cur_channel->length;

	/* Find the message at the bottom of the screen by going downwards */ 
	message_y = client.bottom_message_y; /* bottom of the message */
	i = client.bottom_message; 
	if (i < start) /* if wrapped by the circular buffer */
	{
		dist_start = i + cap - start + 1;
	}
	else
	{
		dist_start = i - start + 1;
	}

	target_y = client.y;
	while (message_y > target_y)
	{
		/* Search upwards */
		i--;
		dist_start--;
		message_y -= heights[i];
		if (i == 0)
		{
			i = cap - 1;
		}
		if (dist_start == 0)
		{
			/* Query more messages if we need more */
			res = backend_newer_messages_query(64, client.server, client.group, client.channel, &ret_unix_ms_timestamps, &ret_messages, &ret_senders, &ret_message_lengths);
			if (res == -1)
			{
				// Couldn't query more message -> no network
				failed_to_query_messages = 1;
				break;
			}
			else if (res == 0)
			{
				break;
			}
			else
			{
				/* Update the circular buffer, set the old messages to freed */
				/* res = number of messages returned */
			
				/* Find where the messages will be stored and which ones will be replaced if any */
				new_data_i = start - res;
				if (new_data_i < 0) /* check if the new data will cross the beginning of the buffer / wrap around */
				{
					new_data_i += cap;
					first_part_len = res - start; /* first part of the res buffer */
					second_part_len = start; /* second part that is wrapped around */
					/* Copy the new messages in 2 times */
					/* end of the circular buffer */
					memcpy(&cur_channel->unix_ms_timestamps[new_data_i], ret_unix_ms_timestamps, (sizeof *cur_channel->unix_ms_timestamps) * first_part_len);
					memcpy(&cur_channel->messages[new_data_i], ret_messages, (sizeof *cur_channel->messages) * first_part_len);
					memcpy(&cur_channel->senders[new_data_i], ret_senders, (sizeof *cur_channel->senders) * first_part_len);
					memcpy(&cur_channel->message_lengths[new_data_i], ret_message_lengths, (sizeof *cur_channel->message_lengths) * first_part_len);
					messages_calculate_heights(client, &cur_channel->messages[new_data_i], &cur_channel->heights[new_data_i], first_part_len); TODO
					/* start of the circular buffer */
					memcpy(cur_channel->unix_ms_timestamps, &ret_unix_ms_timestamps[first_part_len], (sizeof *cur_channel->unix_ms_timestamps) * second_part_len);
					memcpy(cur_channel->messages, &ret_messages[first_part_len], (sizeof *cur_channel->messages) * second_part_len);
					memcpy(cur_channel->senders, &ret_senders[first_part_len], (sizeof *cur_channel->senders) * second_part_len);
					memcpy(cur_channel->message_lengths, &ret_message_lengths[first_part_len], (sizeof *cur_channel->message_lengths) * second_part_len);
					messages_calculate_heights(client, cur_channel->messages, cur_channel->heights, second_part_len);
					/* overwrite */
					overwrite_len = res + length - cap;
					if (overwrite_len > 0)
					{
						backend_message_mark_unused_old(overwrite_len);
						length = cap;
					}
					else
					{
						length += res;
					}
					start -= res;
					dist_start += res;
					cur_channel->length = length;
					cur_channel->start= start;
				}
				else
				{
					/* end of the circular buffer */
					memcpy(&cur_channel->unix_ms_timestamps[new_data_i], ret_unix_ms_timestamps, (sizeof *cur_channel->unix_ms_timestamps) * res);
					memcpy(&cur_channel->messages[new_data_i], ret_messages, (sizeof *cur_channel->messages) * res);
					memcpy(&cur_channel->senders[new_data_i], ret_senders, (sizeof *cur_channel->senders) * res);
					memcpy(&cur_channel->message_lengths[new_data_i], ret_message_lengths, (sizeof *cur_channel->message_lengths) * res);
					messages_calculate_heights(client, &cur_channel->messages[new_data_i], &cur_channel->heights[new_data_i], res);
					/* overwrite */
					overwrite_len = res + length - cap;
					if (overwrite_len > 0)
					{
						backend_message_mark_unused_old(overwrite_len);
						length = cap;
					}
					else
					{
						length += res;
					}
					start -= res;
					dist_start += res;
					cur_channel->length = length;
					cur_channel->start= start;
				}
			} 
			/* the backend knows it can clean those up if needed */
			backend_newer_messages_query_finished(64, client.server, client.group, client.channel, &ret_unix_ms_timestamps, &ret_messages, &ret_senders, &ret_message_lengths);
		}
	}
	bottom_of_current_channel = message_y > target_y; /* if true: autoscroll for new messages */
	if (bottom_of_current_channel)
	{
		client.bottom_message_y = target_y;
	}
	else
	{
		client.bottom_message_y = message_y;
	}
	client.bottom_message = i; 
	
	/* Find the message at the bottom & top of the screen by going upwards */
	i = client.bottom_message;
	message_y = client.bottom_message_y; 
	message_y += heights[i]; /* top of the message */
	if (i < start) /* if wrapped by the circular buffer */
	{
		dist_start = i + cap - start + 1;
	}
	else
	{
		dist_start = i - start + 1;
	}
	
	target_y = client.y;
	if (message_y < target_y)
	{
		client.bottom_message = -1;
	}
	else
	{
		/* Already found the bottom message */
		target_y = client.y + client.message_area_height;
	}

	while (message_y < target_y || client.bottom_message == -1)
	{
		if (client.bottom_message == -1 && message_y > target_y)
		{
			client.bottom_message_y = message_y - height[i]; /* bottom of the message */
			client.bottom_message = i		
			target_y = client.y + client.message_area_height;
		}
		/* Search upwards */
		i++;
		dist_start++;
		if (i == cap)
		{
			i = 0;
		}
		if (dist_start == length)
		{
			/* Query more messages if we need more */ TODO handle message heights
			res = backend_older_messages_query(64, client.server, client.group, client.channel, &ret_unix_ms_timestamps, &ret_messages, &ret_senders, &ret_message_lengths);
			if (res == -1)
			{
				// Couldn't query more message -> no network
				failed_to_query_messages = 1;
				break;
			}
			else if (res == 0)
			{
				break;
			}
			else
			{
				/* Update the circular buffer, set the old messages to freed */
				/* res = number of messages returned */
			
				/* Find where the messages will be stored and which ones will be replaced if any */
				new_data_i = start + length;
				if (new_data_i > cap) /* check if current circular buffer is wrapping around */
				{
					new_data_i -= cap;
					new_data_end = new_data_i + res;
					if (new_data_end > cap) /* check if the new data will cross the end of the buffer / wrap around */
					{
						first_part_len = cap - new_data_i; /* first part of the res buffer */
						second_part_len = new_data_end - cap; /* second part that is wrapped around */
						/* Copy the new messages in 2 times */
						/* end of the circular buffer */
						memcpy(&cur_channel->unix_ms_timestamps[new_data_i], ret_unix_ms_timestamps, (sizeof *cur_channel->unix_ms_timestamps) * first_part_len);
						memcpy(&cur_channel->messages[new_data_i], ret_messages, (sizeof *cur_channel->messages) * first_part_len);
						memcpy(&cur_channel->senders[new_data_i], ret_senders, (sizeof *cur_channel->senders) * first_part_len);
						memcpy(&cur_channel->message_lengths[new_data_i], ret_message_lengths, (sizeof *cur_channel->message_lengths) * first_part_len);
						messages_calculate_heights(client, &cur_channel->messages[new_data_i], &cur_channel->heights[new_data_i], first_part_len);
						/* start of the circular buffer */
						memcpy(cur_channel->unix_ms_timestamps, &ret_unix_ms_timestamps[first_part_len], (sizeof *cur_channel->unix_ms_timestamps) * second_part_len);
						memcpy(cur_channel->messages, &ret_messages[first_part_len], (sizeof *cur_channel->messages) * second_part_len);
						memcpy(cur_channel->senders, &ret_senders[first_part_len], (sizeof *cur_channel->senders) * second_part_len);
						memcpy(cur_channel->message_lengths, &ret_message_lengths[first_part_len], (sizeof *cur_channel->message_lengths) * second_part_len);
						messages_calculate_heights(client, cur_channel->messages, cur_channel->heights, second_part_len);
						/* overwrite is certain */
						if (start <= second_part_len) /* overwrite not making the start wrap around */
						{
							overwrite_len = second_part_len - start;
						}
						else /* overwrite making the start wrap around */
						{
							overwrite_len = (cap - start) + second_part_len; /* The overwrite at the end + the overwrite at the begginning */
						}
						backend_message_mark_unused_recent(overwrite); /* messages at the start of the buffer */
						start = second_part_len;
						dist_start -= overwrite;
						length = cap;
						cur_channel->length = length;
						cur_channel->start= start;
					} 
					else
					{
						/* end of the circular buffer */
						memcpy(&cur_channel->unix_ms_timestamps[new_data_i], ret_unix_ms_timestamps, (sizeof *cur_channel->unix_ms_timestamps) * res);
						memcpy(&cur_channel->messages[new_data_i], ret_messages, (sizeof *cur_channel->messages) * res);
						memcpy(&cur_channel->senders[new_data_i], ret_senders, (sizeof *cur_channel->senders) * res);
						memcpy(&cur_channel->message_lengths[new_data_i], ret_message_lengths, (sizeof *cur_channel->message_lengths) * res);
						messages_calculate_heights(client, &cur_channel->messages[new_data_i], &cur_channel->heights[new_data_i], res);
						/* check if the new data overwrites `start` of the circular buffer */
						if (new_data_end > start) 
						{
							overwrite_len = new_data_end - start;
							backend_message_mark_unused_recent(overwrite_len); /* messages at the `start` of the circular buffer */
							dist_start -= overwrite_len;
							start = new_data_end;
							length = cap;
							cur_channel->start = start;
							cur_channel->length = length;
						}
						else
						{
							length += res;
							cur_channel->length = length;
						}
					}
				}
				else
				{
					new_data_end = new_data_i + res;
					if (new_data_end > cap)  /* check if the new data will cross the end of the buffer / wrap around */
					{
						first_part_len = cap - new_data_i; /* first part of the res buffer */
						second_part_len = new_data_end - cap; /* second part that is wrapped around */
						/* Copy the new messages in 2 times */
						/* end of the circular buffer */
						memcpy(&cur_channel->unix_ms_timestamps[new_data_i], ret_unix_ms_timestamps, (sizeof *cur_channel->unix_ms_timestamps) * first_part_len);
						memcpy(&cur_channel->messages[new_data_i], ret_messages, (sizeof *cur_channel->messages) * first_part_len);
						memcpy(&cur_channel->senders[new_data_i], ret_senders, (sizeof *cur_channel->senders) * first_part_len);
						memcpy(&cur_channel->message_lengths[new_data_i], ret_message_lengths, (sizeof *cur_channel->message_lengths) * first_part_len);
						messages_calculate_heights(client, &cur_channel->messages[new_data_i], &cur_channel->heights[new_data_i], first_part_len);
						/* start of the circular buffer */
						memcpy(cur_channel->unix_ms_timestamps, &ret_unix_ms_timestamps[first_part_len], (sizeof *cur_channel->unix_ms_timestamps) * second_part_len);
						memcpy(cur_channel->messages, &ret_messages[first_part_len], (sizeof *cur_channel->messages) * second_part_len);
						memcpy(cur_channel->senders, &ret_senders[first_part_len], (sizeof *cur_channel->senders) * second_part_len);
						memcpy(cur_channel->message_lengths, &ret_message_lengths[first_part_len], (sizeof *cur_channel->message_lengths) * second_part_len);
						messages_calculate_heights(client, cur_channel->messages, cur_channel->heights, second_part_len);
						if (second_part_len > start) /* check if overwrite */
						{
							overwrite_len = second_part_len - start;
							backend_message_mark_unused_recent(overwrite_len); /* messages at the start of the buffer */
							dist_start -= overwrite_len;
							start = second_part;
							length = cap
							cur_channel->length = length;
							cur_channel->start= start;
						}
						else
						{
							length += res;
							cur_channel->length = length;
						}
					} 
					else
					{
						/* end of the circular buffer */
						memcpy(&cur_channel->unix_ms_timestamps[new_data_i], ret_unix_ms_timestamps, (sizeof *cur_channel->unix_ms_timestamps) * res);
						memcpy(&cur_channel->messages[new_data_i], ret_messages, (sizeof *cur_channel->messages) * res);
						memcpy(&cur_channel->senders[new_data_i], ret_senders, (sizeof *cur_channel->senders) * res);
						memcpy(&cur_channel->message_lengths[new_data_i], ret_message_lengths, (sizeof *cur_channel->message_lengths) * res);
						messages_calculate_heights(client, &cur_channel->messages[new_data_i], &cur_channel->heights[new_data_i], res);

						length += res;
						cur_channel->length = length;
					}
					
				}
			}
			/* the backend knows it can clean those up if needed */
			backend_older_messages_query_finished(64, client.server, client.group, client.channel, &ret_unix_ms_timestamps, &ret_messages, &ret_senders, &ret_message_lengths);
		}
		message_y += heights[i];
	}
	if (client.bottom_message == -1)
	{
		client.bottom_message = i;
		/* No message visible on the screen */
		/* Maybe block the scroll ? */	
	}
	top_message = i;
	
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
	
	/* Draw user interface */ TODO
	/* Draw the servers */ TODO
	/* Draw the channels & scroll bar */ TODO
	/* Draw the messages & scroll bar */ TODO
	/* Draw the input bar */ TODO

	/* Draw floating elements (on top of the rest) */ TODO maybe none?
}

