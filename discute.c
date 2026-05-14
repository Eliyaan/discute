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
	char* channel_unreads; /* TODO something cleaner once there will be more bools */
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


typedef struct {
	int y; /* y coord of the bottom of the screen, the screen moves when the user scrolls */
	Server* servers;
	int* 	server_ids; /* ids must be delivered by the backend in ascending order, and stored here likewise */
	char*   server_unreads; /* TODO something cleaner once there will be more bools */
	char** 	server_names;
	unsigned int length; /* Server array length */
	unsigned int cap; /* Server array cap */
	unsigned int server;
	unsigned int group;
	unsigned int channel;
	unsigned int bottom_message; /* reference message for height, the one at the bottom of the screen */
	unsigned int bottom_message_y; /* y coord of the bottom of the bottom message */
	unsigned int message_space_height;
	unsigned int message_space_width;
} Client;

enum UpdateTypes {
	UPDATE_servers = 1;
	UPDATE_groups = 1 << 1;
	UPDATE_channels = 1 << 2;
	
}

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

Client init(Client client)
{
	// TODO
}

Client frame(Client client)
{
	int i, dist_start, message_y, target_y, res, count, first_part, second_part, overwrite_part;
	int server_i, group_i;
	Channel* cur_channel;
	ChannelGroup cur_group;
	Server cur_server;
	unsigned int cap, start, length, top_message;
	unsigned int* heights;
	char failed_to_query_messages;
	ServerUpdate server_update;
	GroupUpdate group_update;
	ChannelUpdate group_update;
	/* returned from new message querying */
	unsigned long long*   ret_unix_ms_timestamps;
	unsigned char** ret_messages;
	unsigned int* ret_senders;
	unsigned short* ret_message_lengths; 

	failed_to_query_messages = 0;

	/* Update the state according to inputs*/
	client.y += 10; /* scroll */

	/* Update the backend: fetch new messages... and update the frontend accordingly (create new channels if needed) */ // TODO
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
				if (i == -1 || client.server_ids[i] != server_update.server_id) // TODO adjust this according to the dichotomy results
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
				if (server_i == -1 || client.server_ids[server_i] != group_update.server_id) // TODO adjust this according to the dichotomy results
					error(Server not found in group update);
				}
				cur_server = client.servers[server_i];
				i = dichotomy_int(group_update.group_id, cur_server.groups, cur_server.length);
				if (i == -1 || cur_server.group_ids[i] != group_update.group_id) // TODO adjust this according to the dichotomy results
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
				/* Check if it is an already existing group */
				server_i = dichotomy_int(channel_update.server_id, client.server_ids, client.length)
				if (server_i == -1 || client.server_ids[server_i] != channel_update.server_id) // TODO adjust this according to the dichotomy results
					error(Server not found in group update);
				}
				cur_server = client.servers[server_i];
				group_i = dichotomy_int(channel_update.group_id, cur_server.groups, cur_server.length);
				if (group_i == -1 || cur_server.group_ids[group_i] != channel_update.group_id) // TODO adjust this according to the dichotomy results
					error(Group not found in channel update);
				}
				cur_group = cur_server.groups[group_i];
				i = dichotomy_int(channel_update.channel_id, cur_group.channels, cur_group.length);
				if (i == -1 || cur_group.channel_ids[i] != channel_update.channel_id) // TODO adjust this according to the dichotomy results
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
		/* New messages... TODO */
	}
	

	
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
				
					/* Find where the messages will be stored and which ones will be replaced */
					if (cap < length + res) /* check for overwrite */
					{
						second_part = length + start + res - cap;
						first_part = res - second_part;
						overwrite_part = cap - length - res;

						/* Mark the messages as freed by the frontend */
						backend_message_mark_unused(&cur_channel->unix_ms_timestamps[start], overwrite_part);

						/* Store the new messages */
						memcpy(&cur_channel->unix_ms_timestamps[start + length], ret_unix_ms_timestamps, (sizeof *cur_channel->unix_ms_timestamps) * first_part);
						memcpy(&cur_channel->messages[start + length], ret_messages, (sizeof *cur_channel->messages) * first_part);
						memcpy(&cur_channel->senders[start + length], ret_senders, (sizeof *cur_channel->senders) * first_part);
						memcpy(&cur_channel->message_lengths[start + length], ret_message_lengths, (sizeof *cur_channel->message_lengths) * first_part);
						/* start of the circular buffer */
						memcpy(&cur_channel->unix_ms_timestamps[0], ret_unix_ms_timestamps[res], (sizeof *cur_channel->unix_ms_timestamps) * second_part);
						memcpy(&cur_channel->messages[0], ret_messages[res], (sizeof *cur_channel->messages) * second_part);
						memcpy(&cur_channel->senders[0], ret_senders[res], (sizeof *cur_channel->senders) * second_part);
						memcpy(&cur_channel->message_lengths[0], ret_message_lengths[res], (sizeof *cur_channel->message_lengths) * second_part);

						/* Update channel length, start */
						length = cap;
						start = second_part;
						cur_channel->length = length;
						cur_channel->start = start;
					}
					else if (cap - start < length + res) /* check for wrap around with length */
					{
						/* Store the new messages */
						second_part = length + start + res - cap;
						first_part = res - second_part;
						/* end of the circular buffer */
						memcpy(&channel->unix_ms_timestamps[start + length], ret_unix_ms_timestamps, (sizeof *cur_channel->unix_ms_timestamps) * first_part);
						memcpy(&channel->messages[start + length], ret_messages, (sizeof *cur_channel->messages) * first_part);
						memcpy(&channel->senders[start + length], ret_senders, (sizeof *cur_channel->senders) * first_part);
						memcpy(&channel->message_lengths[start + length], ret_message_lengths, (sizeof *cur_channel->message_lengths) * first_part);
						/* start of the circular buffer */
						memcpy(&channel->unix_ms_timestamps[0], ret_unix_ms_timestamps[res], (sizeof *cur_channel->unix_ms_timestamps) * second_part);
						memcpy(&channel->messages[0], ret_messages[res], (sizeof *cur_channel->messages) * second_part);
						memcpy(&channel->senders[0], ret_senders[res], (sizeof *cur_channel->senders) * second_part);
						memcpy(&channel->message_lengths[0], ret_message_lengths[res], (sizeof *cur_channel->message_lengths) * second_part);
						/* Update channel length */
						length += res;
						cur_channel->length = length;
					}
					else
					{
						/* Store the new messages */
						memcpy(&cur_channel->unix_ms_timestamps[start + length], ret_unix_ms_timestamps, (sizeof *cur_channel->unix_ms_timestamps) * res);
						memcpy(&cur_channel->messages[start + length], ret_messages, (sizeof *cur_channel->messages) * res);
						memcpy(&cur_channel->senders[start + length], ret_senders, (sizeof *cur_channel->senders) * res);
						memcpy(&cur_channel->message_lengths[start + length], ret_message_lengths, (sizeof *cur_channel->message_lengths) * res);
						/* Update channel length */
						length += res;
						cur_channel->length = length;
					}
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
	
	/* Draw user interface */ TODO
	/* Draw the servers */ TODO
	/* Draw the channels & scroll bar */ TODO
	/* Draw the messages & scroll bar */ TODO
	/* Draw the input bar */ TODO

	/* Draw floating elements (on top of the rest) */ TODO maybe none?
}

