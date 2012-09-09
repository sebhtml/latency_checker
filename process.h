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

#define MAX_EXCHANGES 1000000
#define MAX_MESSAGE_SIZE 4000


/*
 * This is a MPI process.
 */
struct process{


	/* the number of process */
	int size;

	/* the identifier of the process */
	int rank;

	/* is the process alive ? */
	int alive;

	/* the master mode of the process */
	int master_mode;

	/* the slave mode of the process */
	int slave_mode;

	/* the number of completed processes */
	int completed;

	/* interrupt table for master modes */
	master_mode_interrupt master_mode_interrupts[32];

	/* interrupt table for slave modes */
	slave_mode_interrupt slave_mode_interrupts[32];

	/* interrupt table for message tags */
	message_tag_interrupt message_tag_interrupts[32];


	/* the number of exchanges */
	int exchanges;

	/* the current message */
	int exchange_number;

	/* has the message been sent ? */
	bool sent_message;

	/* has the message been received ? */
	bool received_message;

	/* the starting time for the current message */
	uint64_t start;

	/* list of latencies */
	int latencies[MAX_EXCHANGES];

	/* buffer for all exchanges */
	uint8_t buffer[MAX_MESSAGE_SIZE];

	/* message size */
	int message_size;
};

void init_process(struct process*current_process,int*argc,char***argv);
void pick_up_arguments(struct process*current_process,int argc,char**argv);
void destroy_process(struct process*current_process);

void set_master_mode_interrupt(struct process*current_process,int interrupt,master_mode_interrupt function);
void set_slave_mode_interrupt(struct process*current_process,int interrupt,slave_mode_interrupt function);
void set_message_tag_interrupt(struct process*current_process,int interrupt,message_tag_interrupt function);

void main_loop(struct process*current_process);

void receive_message(struct process*current_process,struct message*received_message);
void process_message(struct process*current_process,struct message*received_message);

void process_master_mode(struct process*current_process);
void process_slave_mode(struct process*current_process);

void send_messages();
void send_to_all(struct process*current_process,int tag);
void send_message(struct process*current_process,uint8_t*buffer,int count,int destination,int tag);
uint64_t get_microseconds();
bool is_alive(struct process*current_process);

/* list of master_mode_interrupt functions */
void no_operation(struct process*current_process);
void begin_test(struct process*current_process);

/* list of slave_mode_interrupt functions */
void test_network(struct process*current_process);

/* list of message_tag_interrupt functions */
void no_message_operation(struct process*current_process,struct message*received_message);
void start_test(struct process*current_process,struct message*received_message);
void read_test_message(struct process*current_process,struct message*received_message);
void complete_process(struct process*current_process,struct message*received_message);
void read_reply(struct process*current_process,struct message*received_message);
void kill_self(struct process*current_process,struct message*received_message);

#endif /* PROCESS_H */

