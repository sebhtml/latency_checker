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

#ifndef MESSAGE_H
#define MESSAGE_H

#define MAX_MESSAGE_SIZE 65536

#include <stdint.h>

/*
 * when processes communicate, they send a message
 */
struct message{
	/* who sent the message */
	int source;

	/* who received the message */
	int destination;

	/* how many elements in the body */
	int count;

	/* the message type */
	int tag;

	/* the message body */
	uint8_t buffer[MAX_MESSAGE_SIZE];
};

#endif /* MESSAGE_H */


