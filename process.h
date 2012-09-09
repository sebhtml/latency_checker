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

#define MASTER 0

#define MESSAGE_TAG_TEST_MESSAGE 0
#define MESSAGE_TAG_TEST_MESSAGE_REPLY 1
#define MESSAGE_TAG_COMPLETED_TEST 2
#define MESSAGE_TAG_KILL 3
#define MESSAGE_TAG_NO_OPERATION 4
#define MESSAGE_TAG_BEGIN_TEST 5

#define MASTER_MODE_NO_OPERATION 0
#define MASTER_MODE_BEGIN_TEST 1

#define SLAVE_MODE_NO_OPERATION 0
#define SLAVE_MODE_TEST_NETWORK 1

typedef void (*master_mode_interrupt)();
typedef void (*slave_mode_interrupt)();
typedef void (*message_tag_interrupt)();



struct process{

/* fields required for the process */
	int size;
	int rank;
	int alive;
	int master_mode;
	int slave_mode;

	int completed;

	master_mode_interrupt master_mode_interrupts[32];
	slave_mode_interrupt slave_mode_interrupts[32];
	message_tag_interrupt message_tag_interrupts[32];

/* stuff for the latency test */

	int messages;
	int message_number;
	bool sent_message;
	bool received_message;
	uint64_t start;

	int latencies[100000];
	uint8_t buffer[4000];

};

void init_process(struct process*current_process,int*argc,char***argv);
void set_master_mode_interrupt(struct process*current_process,int interrupt,master_mode_interrupt function);
void set_slave_mode_interrupt(struct process*current_process,int interrupt,slave_mode_interrupt function);
void set_message_tag_interrupt(struct process*current_process,int interrupt,message_tag_interrupt function);

void receive_message(struct process*current_process,struct message*received_message);
void process_message(struct process*current_process,struct message*received_message);

void process_master_mode(struct process*current_process);
void process_slave_mode(struct process*current_process);

void send_messages();
void begin_test(struct process*current_process);
void send_message(struct process*current_process,uint8_t*buffer,int count,int destination,int tag);
void main_loop(struct process*current_process);
void test_network(struct process*current_process);
uint64_t get_microseconds();
void send_to_all(struct process*current_process,int tag);
bool is_alive(struct process*current_process);

/* list of message_tag_interrupt functions */

void no_message_operation(struct process*current_process,struct message*received_message);
void start_test(struct process*current_process,struct message*received_message);
void read_test_message(struct process*current_process,struct message*received_message);
void complete_process(struct process*current_process,struct message*received_message);
void read_reply(struct process*current_process,struct message*received_message);
void kill_self(struct process*current_process,struct message*received_message);

#endif /* PROCESS_H */

