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

#include <stdlib.h>
#include <mpi.h>
#include <stdio.h>
#include <stdint.h>

#include "message.h"
#include "process.h"

int main(int argc,char**argv){
	
	struct process current_process;

	init_process(&current_process,&argc,&argv);

	main_loop(&current_process);

	destroy_process(&current_process);

	return EXIT_SUCCESS;
}

