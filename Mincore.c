/********************************************** 
 * mincore
 *   show pages that are in memory (core) for the specified file(s)
 *
 * Author: Tim Bird   <tim.bird (at) am.sony.com>
 * Copyright 2010 Sony Network Entertainment, Inc.
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
 * NOTES:
 * Use the '-r' parameter to output in readahead_list.txt format
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>

#define OUTPUT_NORMAL		0	
#define OUTPUT_READAHEAD	1

int do_mincore(char *filename, int output_style)
{
    int fd;
    struct stat file_stat;
    void *file_mmap;
    unsigned char *mincore_vec;
    size_t page_size = getpagesize();
    size_t page_i;
    unsigned int num_pages;
    int cached;
    size_t len;
    int in_area;

    fd = open(filename, O_RDONLY);
    if (fd<0) {
	perror("Couldn't open file\n");
	exit(1);
    }
    fstat(fd, &file_stat);
    file_mmap = mmap((void *)0, file_stat.st_size, PROT_NONE, MAP_SHARED,
	fd, 0);

    num_pages =(file_stat.st_size+page_size-1)/page_size;
    mincore_vec = calloc(1, num_pages);
    mincore(file_mmap, file_stat.st_size, mincore_vec);

    switch(output_style) {
	case OUTPUT_READAHEAD:
	    cached = 0;
	    for (page_i = 0; page_i < num_pages; page_i++) {
		if (mincore_vec[page_i]&1) {
		    cached++;
		    break;
		}
	    }
	    if (!cached) {
		break;
	    }
	    printf("%s ", filename);
	    /* a simple state machine to output the area list */
	    /* format is: offset:len,offset2:len2,... */

	    in_area = 0;
	    len = 0;
	    for (page_i = 0; page_i < num_pages; page_i++) {
		if (mincore_vec[page_i]&1) {
		    if (in_area) {
			len = len+page_size;		
		    } else {
			len = page_size;
			printf("%lu:", (unsigned long)page_i*page_size);
			in_area = 1;
		    }
		} else {
		    if (in_area) {
			printf("%u,", len);
			len = 0;
			in_area = 0;
		    }
		}
	    }
	    if (in_area)
		printf("%u", len);
	    printf("\n");
	    break;
	case OUTPUT_NORMAL:
	default:
	    printf("Cached Blocks of %s: ",filename);

	    printf("[");
	    cached = 0;
	    for (page_i = 0; page_i < num_pages; page_i++) {
		if (mincore_vec[page_i]&1) {
		    printf("#");
		    cached++;
		} else {
		    printf(" ");
		}
	    }
	    printf("]\n");
	    printf("   %d/%u pages in cache.\n", cached, num_pages);
	    break;
    }
	
    free(mincore_vec);
    munmap(file_mmap, file_stat.st_size);
    close(fd);
    return 0;
}

int main(int argc, char *argv[]) {
    int i;
    int output_style;

    output_style=OUTPUT_NORMAL;
    for(i=1; i<argc; i++) {
	if ((argv[i][0]=='-') && (argv[i][1]=='r'))
	    output_style=OUTPUT_READAHEAD;
	else 
	    do_mincore(argv[i], output_style);
    }
    return 0;
}
