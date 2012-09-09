
#include <stdlib.h>
#include <mpi.h>
#include <stdio.h>
#include <stdint.h>

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

struct message{
	int source;
	int destination;
	uint8_t buffer[4000];
	int count;
	int tag;
};

void set_state(struct process*current_process,int state,int value){
	switch(state){
		case MASTER_MODE:
			current_process->master_mode=value;
			break;
		case SLAVE_MODE:
			current_process->slave_mode=value;
			break;
		case MESSAGE_TAG:
			current_process->message_tag=value;
			break;
		case ALIVE:
			current_process->alive=value;
			break;
	}
}



void receive_message(struct process*current_process,
	struct message*received_message){

	int flag=0;
	MPI_Status status;
	MPI_Iprobe(MPI_ANY_SOURCE,MPI_ANY_TAG,
		MPI_COMM_WORLD,&flag,&status);

	set_state(current_process,MESSAGE_TAG,MESSAGE_TAG_NO_OPERATION);

	if(!flag)
		return;

	received_message->tag=status.MPI_TAG;
	received_message->source=status.MPI_SOURCE;
	received_message->count=0;
	MPI_Get_count(&status,MPI_BYTE,&(received_message->count));

	MPI_Recv(received_message->buffer,received_message->count,
		MPI_BYTE,received_message->source,received_message->tag,
		MPI_COMM_WORLD,&status);

	set_state(current_process,MESSAGE_TAG,received_message->tag);
}

void process_message(struct process*current_process,struct message*received_message){

	if(received_message->source==NO_SOURCE)
		return;
	
}

void init_process(struct process*current_process,
	int*argc,char***argv){

	MPI_Init(argc,argv);

	set_state(current_process,ALIVE,YES);
	set_state(current_process,MASTER_MODE,MASTER_MODE_NO_OPERATION);
	set_state(current_process,SLAVE_MODE,SLAVE_MODE_NO_OPERATION);
}

bool is_alive(struct process*current_process){
	return current_process->alive;
}

int main(int argc,char**argv){

	struct message received_message;
	struct message outbox[1024];
	struct process current_process;

	init_process(&current_process,&argc,&argv);

	while(is_alive(&current_process)){

		receive_message(&current_process,&received_message);

		process_message(&current_process,&received_message);

		current_process.alive=false;
	}

	MPI_Finalize();

	return EXIT_SUCCESS;
}

