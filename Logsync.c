/********************************************** 
 * hello
 *     Hello world test program
 *
 * Author: Tim Bird   <tim.bird (at) am.sony.com>
 * Copyright 2011 Sony Network Entertainment, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <string.h>

void write_klog(const char *tag, char *message)
{
	int fd;
	char buffer[4096];

	fd = open("/dev/kmsg", O_RDWR);
	if( fd<0 ) {
		printf("problem opening /dev/kmsg\n");
		return;
	}
	strcpy(buffer, tag);
	strcat(buffer, " : ");
	strncat(buffer, message, sizeof(buffer)-1);
	write(fd, buffer, strlen(buffer)+1);
	close(fd);
}

#define ANDROID_LOG_INFO	4

void write_alog(const char *tag, char *message)
{
	int fd;
	unsigned char priority=ANDROID_LOG_INFO;
	struct iovec vec[3];

	fd = open("/dev/log/main", O_WRONLY);
	if( fd<0 ) {
		printf("problem opening /dev/log/main\n");
		return;
	}
	vec[0].iov_base	= (unsigned char *)&priority;
	vec[0].iov_len	= 1;
	vec[1].iov_base	= (void *)tag;
	vec[1].iov_len	= strlen(tag)+1;
	vec[2].iov_base	= (void *)message;
	vec[2].iov_len	= strlen(message)+1;

	writev(fd, vec, 3);
	close(fd);
}

int main(int argc, char *argv[])
{
	int i;
	const char *tag = "logsync";
	char buffer[4096];
	
	for( i=1; i<argc; i++) {
		strncat(buffer, argv[i], sizeof(buffer)-1);
		strncat(buffer, " ", sizeof(buffer)-1);
	}
	strcat(buffer, "\n");
	
	printf("%s : %s", tag, buffer);
	write_klog(tag, buffer);
	write_alog(tag, buffer);
    	return 0;
}
