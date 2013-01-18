/*
 *  latency_checker: a latency benchmark for multi-core multi-node systems
 *  Copyright (C) 2012, 2013 Sébastien Boisvert
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

#include "process.h"

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#ifdef __linux__
#include <sys/time.h>
#endif

#ifdef ASSERT
#include <assert.h>
#endif /* ASSERT */

/*
 * Receive a message. First, the message is probed. If
 * there is a message, we pump it.
 */
void receive_message(struct process*current_process,
	struct message*received_message){

	int flag=0;
	MPI_Status status;
	int return_value=MPI_Iprobe(MPI_ANY_SOURCE,MPI_ANY_TAG,
		MPI_COMM_WORLD,&flag,&status);

	if(return_value!=MPI_SUCCESS)
		printf("MPI_Iprobe failed with code %d\n",return_value);
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

	return_value=MPI_Recv(received_message->buffer,received_message->count,
		MPI_BYTE,received_message->source,received_message->tag,
		MPI_COMM_WORLD,&status);

	if(return_value!=MPI_SUCCESS)
		printf("MPI_Recv failed with code %d\n",return_value);
}

void pick_up_arguments(struct process*current_process,int argc,char**argv){
	int i;
	int MATCH;

	MATCH=0;

	for(i=0;i<argc;i++){
		if(strcmp(argv[i],"-message-size")==MATCH && i+1<argc){
			current_process->message_size=atoi(argv[i+1]);
			if(current_process->message_size>MAX_MESSAGE_SIZE)
				current_process->message_size=MAX_MESSAGE_SIZE;
		}else if(strcmp(argv[i],"-exchanges")==MATCH && i+1<argc){
			current_process->exchanges=atoi(argv[i+1]);
			if(current_process->exchanges>MAX_EXCHANGES)
				current_process->exchanges=MAX_EXCHANGES;
		}
	}
}

void no_message_operation(struct process*current_process,struct message*received_message){
}

void start_test(struct process*current_process,struct message*received_message){
	current_process->slave_mode=SLAVE_MODE_TEST_NETWORK;
	
	#ifdef VERBOSE
	printf("rank %d is starting the test\n",current_process->rank);
	#endif
}

void read_test_message(struct process*current_process,struct message*received_message){
	#ifdef VERBOSE
	printf("received MESSAGE_TAG_TEST_MESSAGE from %d\n",
		received_message->source);
	#endif /* VERBOSE */

	send_message(current_process,current_process->buffer,
		current_process->message_size,received_message->source,
		MESSAGE_TAG_TEST_MESSAGE_REPLY);

}

void read_reply(struct process*current_process,struct message*received_message){

	current_process->received_message=true;
}

void complete_process(struct process*current_process,struct message*received_message){
	current_process->completed++;

	if(current_process->completed==current_process->size){
		current_process->completed=0;
		send_to_all(current_process,MESSAGE_TAG_GET_RESULT);
	}
}

void receive_result(struct process*current_process,struct message*receive_message){

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

	#ifdef VERBOSE
	printf("Initialized process with rank %d (size is %d)\n",
		current_process->rank,current_process->size);
	#endif

	#ifdef VERBOSE
	printf("master mode of rank %d is %d\n",current_process->rank,
		current_process->master_mode);
	#endif /* VERBOSE */

	/* configure latency test */

	current_process->exchange_number=0;
	current_process->sent_message=false;
	current_process->completed=0;
	current_process->exchanges=100000;
	current_process->message_size=4000;

	pick_up_arguments(current_process,*argc,*argv);

	if(current_process->rank==MASTER){
		printf("Rank %d latency_checker: a latency benchmark for multi-core multi-node systems\n",
			current_process->rank);
		printf("Copyright (C) 2012, 2013 Sébastien Boisvert\n");
		printf("License: GNU General Public License, version 3\n\n");

		printf("Rank %d -> message size: %d bytes (-message-size %d)",current_process->rank,
			current_process->message_size,current_process->message_size);

		printf(" exchanges: %d (-exchanges %d)\n\n",current_process->exchanges,current_process->exchanges);

		current_process->sum=0;
	}

	/* configure the process state machine */

	set_master_mode_interrupt(current_process,MASTER_MODE_NO_OPERATION,no_operation);
	set_master_mode_interrupt(current_process,MASTER_MODE_BEGIN_TEST,begin_test);

	set_slave_mode_interrupt(current_process,SLAVE_MODE_NO_OPERATION,no_operation);
	set_slave_mode_interrupt(current_process,SLAVE_MODE_TEST_NETWORK,test_network);

	set_message_tag_interrupt(current_process,MESSAGE_TAG_NO_OPERATION,no_message_operation);
	set_message_tag_interrupt(current_process,MESSAGE_TAG_BEGIN_TEST,start_test);
	set_message_tag_interrupt(current_process,MESSAGE_TAG_TEST_MESSAGE,read_test_message);
	set_message_tag_interrupt(current_process,MESSAGE_TAG_TEST_MESSAGE_REPLY,read_reply);
	set_message_tag_interrupt(current_process,MESSAGE_TAG_COMPLETED_TEST,complete_process);
	set_message_tag_interrupt(current_process,MESSAGE_TAG_GET_RESULT,get_result);
	set_message_tag_interrupt(current_process,MESSAGE_TAG_GET_RESULT_REPLY,get_result_reply);
	set_message_tag_interrupt(current_process,MESSAGE_TAG_KILL,kill_self);
}

bool is_alive(struct process*current_process){
	return current_process->alive;
}

void send_messages(){
}

void process_slave_mode(struct process*current_process){

	current_process->slave_mode_interrupts[current_process->slave_mode](current_process);
}

void process_master_mode(struct process*current_process){

	current_process->master_mode_interrupts[current_process->master_mode](current_process);
}

void no_operation(struct process*current_process){
}

void send_message(struct process*current_process,uint8_t*buffer,int count,int destination,int tag){

	#ifdef ASSERT
	assert(destination>=0);
	assert(destination<current_process->size);
	#endif

	MPI_Request request;
	int return_value=MPI_Isend(buffer,count,MPI_BYTE,destination,tag,MPI_COMM_WORLD,&request);

	if(return_value!=MPI_SUCCESS)
		printf("MPI_Isend failed with code %d\n",return_value);

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

}

void destroy_process(struct process*current_process){

	MPI_Finalize();
}

void test_network(struct process*current_process){

	if(current_process->exchange_number<current_process->exchanges){

		if(!current_process->sent_message){
	
			int destination=rand()%current_process->size;

			#ifdef ASSERT
			assert(destination>=0);
			assert(destination<current_process->size);
			#endif /* ASSERT */

			current_process->start=get_microseconds();

			send_message(current_process,current_process->buffer,
				current_process->message_size,destination,MESSAGE_TAG_TEST_MESSAGE);

			current_process->sent_message=true;
			current_process->received_message=false;
		}else if(current_process->received_message){

			uint64_t latency=get_microseconds()-current_process->start;
			current_process->latencies[current_process->exchange_number]=latency;

			current_process->exchange_number++;
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

void get_result(struct process*current_process,struct message*received_message){

	#define FREQUENCIES 1000

	int frequencies[FREQUENCIES];

	int i;

	for(i=0;i<FREQUENCIES;i++)
		frequencies[i]=0;

	for(i=0;i<current_process->exchanges;i++){
		int value=current_process->latencies[i];
		if(value<FREQUENCIES)
			frequencies[value]++;
	}

	double average=0;
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

	#ifdef VERBOSE
	printf("Rank %d -> mode roundtrip latency: %d microseconds, average roundtrip latency: %f microseconds\n",
		current_process->rank,mode,average);
	#endif

	printf("Rank %d -> average roundtrip latency: %f microseconds\n",
		current_process->rank,average);

	#ifdef VERBOSE
	printf("Round trip latency (microseconds)	frequency\n");

	for(i=0;i<FREQUENCIES;i++)
		printf("DATA	%d	%d\n",i,frequencies[i]);
	#endif

	memcpy(current_process->buffer,&average,sizeof(double));

	send_message(current_process,current_process->buffer,
		current_process->message_size,received_message->source,
		MESSAGE_TAG_GET_RESULT_REPLY);
}

void get_result_reply(struct process*current_process,struct message*received_message){

	double average=0;
	memcpy(&average,received_message->buffer,sizeof(double));
	current_process->sum+=average;

	current_process->completed++;

	if(current_process->completed!=current_process->size)
		return;

	current_process->sum/=current_process->size;

	printf("\nRank %d -> average roundtrip latency: %f microseconds\n",current_process->rank,current_process->sum);

	send_to_all(current_process,MESSAGE_TAG_KILL);
}

void kill_self(struct process*current_process,struct message*received_message){
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
	
