/*****************************************************************************
 * dataspy.c        data access routines
 *                 These functions can be used by user programs
 *                 to access data blocks held in shared memory
 *     This version Mar 2003  J.Cresswell and V.Pucknell
 *      Major update Dec 2014 to support use of Posix shared memory and for general use within the MIDAS data acq infrastructure
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#if (defined SOLARIS || defined POSIX)

#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#ifndef MACOSX
#ifdef __cplusplus
extern "C" {
#endif
	int shm_open(const char *, int, mode_t);
#ifdef __cplusplus
}
#endif
#endif

#else
#include <sys/types.h>
#include <sys/shm.h>
#endif

#include <DataSpy.h>

#define MAX_ID 8
#define MAX_BUFFER_SIZE 64*1024

#define SHMSIZE 0x401000
#define SHM_KEY 110205   /* base Key */


#if (defined SOLARIS || defined POSIX)
int shmkey = SHM_KEY;
char object_name[16];
#else
key_t shmkey = SHM_KEY;
#endif

void * shm_bufferarea[MAX_ID];
int shmid[MAX_ID];
BUFFER_HEADER * baseaddress;

int number_of_buffers[MAX_ID];
int buffers_offset[MAX_ID];
int next_index[MAX_ID];
unsigned long long current_age[MAX_ID];

static int verbose = 0;

void dataSpyVerbose( int opt ) { verbose = opt; }

/*****************************************************************************
 * dataSpyOpen     make a connection to an the shared file for that id
 *                 return OK or ERROR (0 or -1)
 */

int dataSpyOpen( int id ) {
	
	if( id < 0 || id >= MAX_ID ) {
		perror ("dataSpyOpen - id number out of range");
		return -1;
	}
	
#if (defined SOLARIS || defined POSIX)
	
	// create a file mapped object (MASTER) or obtain ID of existing object */
	
	sprintf( object_name, "/SHM_%d", shmkey+id );
	
	shmid[id] = shm_open( object_name, O_RDONLY, (mode_t) 0 );
	if( shmid[id] == -1 ) {
		perror("shm_open");
		exit(1);
	}
	
#else
	
	shmid[id] = shmget(shmkey+id, 0, SHM_R);
	if (shmid[id] == -1) {perror("shmget"); return -1;}
	
#endif
	
#if (defined SOLARIS || defined POSIX)
	
	// attach the memory segment
	shm_bufferarea[id] = mmap((void *) NULL, (size_t) SHMSIZE, PROT_READ, MAP_SHARED, shmid[id], (off_t) 0);
	if (shm_bufferarea[id] == (void *) MAP_FAILED) {
		perror("mmap");
		exit(1);
	}
	
	close( shmid[id] );
	
#else
	
	shm_bufferarea[id] = shmat( shmid[id], (void *) 0, SHM_RDONLY );
	if( shm_bufferarea[id] == (void *) -1 ) {
		perror("shmat");
		exit(1);
	}
	
#endif
	
	printf("dataSpy Shared buffer area %d (/SHM_%d) located at 0x%x\n", id, shmkey+id,(unsigned int)shm_bufferarea[id]);
	
	baseaddress = (BUFFER_HEADER *) shm_bufferarea[id];
	
	number_of_buffers[id] = baseaddress->buffer_number;
	buffers_offset[id] = baseaddress->buffer_offset;
	
	/* Flush all old buffers by setting current index ... */
	
	next_index[id] = baseaddress->buffer_next;
	current_age[id] = baseaddress->buffer_currentage;
	
	printf("dataSpy Current age %lld index %d\n", current_age[id],next_index[id]);
	
	return 0;
	
}

/*****************************************************************************
 * dataSpyClose       close a connection to an the shared file for that id
 *                 close the file
 *                 return OK or ERROR
 */

int dataSpyClose( int id ) {

	if( id < 0 || id >= MAX_ID ) {
		
		perror( "dataSpyClose - id number out of range" );
		return -1;
		
	}
	
#if (defined SOLARIS || defined POSIX)
	
	// detach the memory segment
	(void) munmap(shm_bufferarea[id], (size_t) SHMSIZE);
	
#else
	
	(void) shmdt(shm_bufferarea[id]);
	
#endif
	
	printf("dataSpy Shared buffer area %d located at 0x%x detached\n",id,(unsigned int)shm_bufferarea[id]);
	
	return 0;

}

/*****************************************************************************
 * dataSpyRead        get a block of data for that field
 *                    check that the id is valid and opened
 *                    check that there is any data in the queue
 *                    increment the queue read index
 *                    return length of data block or ERROR
 length is in units of bytes
 */

int dataSpyReadWithSeq( int id, char *data, unsigned int length, int *seq ) {
	
	int *bufferaddress;
	unsigned int len;
	int *ptr;
	
	
	if( id < 0 || id >= MAX_ID ) {
		perror( "dataSpyRead - id number out of range" );
		return -1;
	}
	
	len = 0;
	baseaddress = (BUFFER_HEADER *) shm_bufferarea[id];
	
retry:
	
	if( baseaddress->buffer_age[next_index[id]] != 0 ) {
		
		if( baseaddress->buffer_age[next_index[id]] >= current_age[id] ) {
	
			current_age[id] = baseaddress->buffer_age[next_index[id]];
			
			len = baseaddress->buffer_length;
			if( !len ) len = MAX_BUFFER_SIZE;
			
			bufferaddress = (int *) ((char *)shm_bufferarea[id] + buffers_offset[id] + (len * next_index[id]));
			
			if( length < len ) len = length;
			
			if( verbose )
				printf( "dataSpyRead id %d: Age %lld Index %d Buffer length %d\n",
						id, current_age[id], next_index[id], len );
			
			// copy data from shared memory to user buffer
			ptr = (int *)data;
			for( int i = 0; i < (len >> 2); i++ )
				ptr[i] = bufferaddress[i];
			*seq = (int)current_age[id];
			
			// check if the entry could have changed while copying (can happen) and if so retry
			if( current_age[id] != baseaddress->buffer_age[next_index[id]] ) {
				
				if( verbose )
					printf ("dataSpyRead id %d: Copied oldage %lld newage %lld\n", id, current_age[id], baseaddress->buffer_age[next_index[id]]);
				
				goto retry;
				
			}
			
			next_index[id] = ++next_index[id] & (number_of_buffers[id] -1);

		}
		
	}
	
	else {
		
		if( verbose )
			printf ("dataSpyRead - id %d has no data\n",id);
		
	}
	
	if( verbose )
		printf("dataSpyRead - id %d length %d\n",id,len);
	
	return len;                      /* return actual length of data block */
}
/******************************************************************************
 * dataSpyRead   get a block of data for that id
 * as for dataSpyReadWithSeq, but ignoring the sequence numbering
 * provided for compatibility with earlier version
 */

int dataSpyRead( int id, char *data, unsigned int length ) {

	int seq;
	
	return dataSpyReadWithSeq( id, data, length, &seq );
	
}
/*****************************************************************************/
