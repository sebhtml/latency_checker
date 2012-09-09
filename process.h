/*
 *  latency_checker: a latency benchmark for multi-core multi-node systems
 *  Copyright (C) 2012  Sébastien Boisvert
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You have received a copy of the GNU General Public License
 *  along with this program. Otherwise see <http://www.gnu.org/licenses/>
 */

#ifndef PROCESS_H
#define PROCESS_H

#include "message.h"

#ifndef bool
#define bool int
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#define NO 0
#define YES 1

#define MASTER_MODE 0
#define SLAVE_MODE 1
#define MESSAGE_TAG 2
#define ALIVE 3

#define NO_SOURCE -1

#define MESSAGE_TAG_TEST_MESSAGE 0
#define MESSAGE_TAG_TEST_MESSAGE_REPLY 1
#define MESSAGE_TAG_COMPLETED_TEST 2
#define MESSAGE_TAG_KILL 3
#define MESSAGE_TAG_NO_OPERATION 4

#define MASTER_MODE_NO_OPERATION 0

#define SLAVE_MODE_NO_OPERATION 0



struct process{
	int alive;
	int master_mode;
	int slave_mode;
	int message_tag;
};

void init_process(struct process*current_process,int*argc,char***argv);
void set_state(struct process*current_process,int state,int value);
void receive_message(struct process*current_process,struct message*received_message);
void process_message(struct process*current_process,struct message*received_message);
bool is_alive(struct process*current_process);

#endif /* PROCESS_H */

