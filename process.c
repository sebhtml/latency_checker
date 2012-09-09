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

#include "process.h"

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

void init_process(struct process*current_process,int*argc,char***argv){

	MPI_Init(argc,argv);

	set_state(current_process,ALIVE,YES);
	set_state(current_process,MASTER_MODE,MASTER_MODE_NO_OPERATION);
	set_state(current_process,SLAVE_MODE,SLAVE_MODE_NO_OPERATION);
}

bool is_alive(struct process*current_process){
	return current_process->alive;
}


