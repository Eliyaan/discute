typedef struct {
	unsigned long long*   unix_ms_timestamps; /*  */
	unsigned char** messages;
	unsigned int* senders;
	unsigned short* message_lengths; 
	unsigned short* widths;
	unsigned short* heights; 
	unsigned int cap;
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
	unsigned int bottom_message_y; /* 
} Client;

void frame(Client client) {
	/* Update the state according to inputs*/
	client.y += 10; /* scroll */
	
	/* Find all the messages that need to be drawn, and query new ones if needed */
	
	
}
