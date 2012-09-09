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

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __linux__
#include <sys/time.h>
#endif

#ifdef ASSERT
#include <assert.h>
#endif /* ASSERT */

#include "process.h"

void receive_message(struct process*current_process,
	struct message*received_message){

	int flag=0;
	MPI_Status status;
	MPI_Iprobe(MPI_ANY_SOURCE,MPI_ANY_TAG,
		MPI_COMM_WORLD,&flag,&status);

	received_message->tag=MESSAGE_TAG_NO_OPERATION;

	if(!flag)
		return;

	received_message->tag=status.MPI_TAG;
	received_message->source=status.MPI_SOURCE;

	#ifdef VERBOSE
	printf("rank %d received message from %d with tag %d\n",
		current_process->rank,received_message->source,
		received_message->tag);
	#endif

	#ifdef ASSERT
	assert(received_message->source>=0);
	assert(received_message->source < current_process->size);
	#endif /* ASSERT */

	received_message->count=0;
	MPI_Get_count(&status,MPI_BYTE,&(received_message->count));

	MPI_Recv(received_message->buffer,received_message->count,
		MPI_BYTE,received_message->source,received_message->tag,
		MPI_COMM_WORLD,&status);

}

void no_message_operation(struct process*current_process,struct message*received_message){
}

void start_test(struct process*current_process,struct message*received_message){
	current_process->slave_mode=SLAVE_MODE_TEST_NETWORK;
}

void read_test_message(struct process*current_process,struct message*received_message){
	#ifdef VERBOSE
	printf("received MESSAGE_TAG_TEST_MESSAGE from %d\n",
		received_message->source);
	#endif /* VERBOSE */

	send_message(current_process,NULL,0,
		received_message->source,
		MESSAGE_TAG_TEST_MESSAGE_REPLY);

}

void read_reply(struct process*current_process,struct message*received_message){

	current_process->received_message=true;
}

void complete_process(struct process*current_process,struct message*received_message){
	current_process->completed++;

	if(current_process->completed==current_process->size)
		send_to_all(current_process,MESSAGE_TAG_KILL);

}

void process_message(struct process*current_process,struct message*received_message){

	current_process->message_tag_interrupts[received_message->tag](current_process,received_message);
}

void init_process(struct process*current_process,int*argc,char***argv){

	srand(time(NULL));

	MPI_Init(argc,argv);

	current_process->alive=true;
	current_process->master_mode=MASTER_MODE_NO_OPERATION;
	current_process->slave_mode=SLAVE_MODE_NO_OPERATION;


	MPI_Comm_rank(MPI_COMM_WORLD,&(current_process->rank));
	MPI_Comm_size(MPI_COMM_WORLD,&(current_process->size));

	if(current_process->rank==MASTER)
		current_process->master_mode=MASTER_MODE_BEGIN_TEST;

	printf("Initialized process with rank %d (size is %d)\n",
		current_process->rank,current_process->size);

	#ifdef VERBOSE
	printf("master mode of rank %d is %d\n",current_process->rank,
		current_process->master_mode);
	#endif /* VERBOSE */

	current_process->message_number=0;
	current_process->sent_message=false;
	current_process->completed=0;
	current_process->messages=100000;

	set_message_tag_interrupt(current_process,MESSAGE_TAG_NO_OPERATION,no_message_operation);
	set_message_tag_interrupt(current_process,MESSAGE_TAG_BEGIN_TEST,start_test);
	set_message_tag_interrupt(current_process,MESSAGE_TAG_TEST_MESSAGE,read_test_message);
	set_message_tag_interrupt(current_process,MESSAGE_TAG_TEST_MESSAGE_REPLY,read_reply);
	set_message_tag_interrupt(current_process,MESSAGE_TAG_COMPLETED_TEST,complete_process);
	set_message_tag_interrupt(current_process,MESSAGE_TAG_KILL,kill_self);
}

bool is_alive(struct process*current_process){
	return current_process->alive;
}

void send_messages(){
}

void process_slave_mode(struct process*current_process){
	switch(current_process->slave_mode){
		case SLAVE_MODE_NO_OPERATION:
			break;
		case SLAVE_MODE_TEST_NETWORK:
			test_network(current_process);
			break;
	}
}

void process_master_mode(struct process*current_process){

	switch(current_process->master_mode){
		case MASTER_MODE_NO_OPERATION:
			break;
		case MASTER_MODE_BEGIN_TEST:
			begin_test(current_process);
			break;
	}
}

void send_message(struct process*current_process,uint8_t*buffer,int count,int destination,int tag){

	#ifdef ASSERT
	assert(destination>=0);
	assert(destination<current_process->size);
	#endif

	MPI_Request request;
	MPI_Isend(buffer,count,MPI_BYTE,destination,tag,MPI_COMM_WORLD,&request);
	MPI_Request_free(&request);

}

void begin_test(struct process*current_process){

	send_to_all(current_process,MESSAGE_TAG_BEGIN_TEST);

	current_process->master_mode=MASTER_MODE_NO_OPERATION;
}

void send_to_all(struct process*current_process,int tag){
	int i;

	for(i=0;i<current_process->size;i++){

		send_message(current_process,NULL,0,i,tag);
	}
}

void main_loop(struct process*current_process){

	struct message received_message;

	while(is_alive(current_process)){

		receive_message(current_process,&received_message);

		process_message(current_process,&received_message);

		process_master_mode(current_process);

		process_slave_mode(current_process);

		send_messages();
	}

	MPI_Finalize();

}

void test_network(struct process*current_process){

	if(current_process->message_number<current_process->messages){

		if(!current_process->sent_message){
	
			int destination=rand()%current_process->size;

			#ifdef ASSERT
			assert(destination>=0);
			assert(destination<current_process->size);
			#endif /* ASSERT */

			current_process->start=get_microseconds();

			send_message(current_process,NULL,0,destination,MESSAGE_TAG_TEST_MESSAGE);

			current_process->sent_message=true;
			current_process->received_message=false;
		}else if(current_process->received_message){

			uint64_t latency=get_microseconds()-current_process->start;
			current_process->latencies[current_process->message_number]=latency;

			current_process->message_number++;
			current_process->sent_message=false;
			current_process->received_message=false;

		}
	}else{

		send_message(current_process,NULL,0,MASTER,MESSAGE_TAG_COMPLETED_TEST);

		current_process->slave_mode=SLAVE_MODE_NO_OPERATION;
	}
}

uint64_t get_microseconds(){

	#ifdef __linux__
	struct timeval theTime;
	gettimeofday(&theTime,NULL);
	uint64_t seconds=theTime.tv_sec;
	uint64_t microSeconds=theTime.tv_usec;

	return seconds*1000*1000+microSeconds;

	#else

	return 0;

	#endif

}

void kill_self(struct process*current_process,struct message*received_message){

	#define FREQUENCIES 1000

	int frequencies[FREQUENCIES];

	int i;

	for(i=0;i<FREQUENCIES;i++)
		frequencies[i]=0;

	for(i=0;i<current_process->messages;i++){
		int value=current_process->latencies[i];
		if(value<FREQUENCIES)
			frequencies[value]++;
	}
	
	printf("Exchanges: %d\n",current_process->messages);

	int average=0;
	int mode=0;
	int eligible_entries=0;


	for(i=0;i<FREQUENCIES;i++){
		int frequency=frequencies[i];

		if(frequency>frequencies[mode])
			mode=i;

		average+=i*frequency;
		eligible_entries+=frequency;

	}

	if(eligible_entries)
		average/=eligible_entries;

	printf("mode round trip latency: %d microseconds, average round trip latency: %d microseconds\n",
		mode,average);

	printf("Round trip latency (microseconds)	frequency\n");

	for(i=0;i<FREQUENCIES;i++)
		printf("DATA	%d	%d\n",i,frequencies[i]);

	current_process->alive=false;
}

void set_master_mode_interrupt(struct process*current_process,int interrupt,master_mode_interrupt function){
	current_process->master_mode_interrupts[interrupt]=function;
}

void set_slave_mode_interrupt(struct process*current_process,int interrupt,slave_mode_interrupt function){
	current_process->slave_mode_interrupts[interrupt]=function;
}

void set_message_tag_interrupt(struct process*current_process,int interrupt,message_tag_interrupt function){
	current_process->message_tag_interrupts[interrupt]=function;
}

