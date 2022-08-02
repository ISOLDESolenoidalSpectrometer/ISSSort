/// DataSpy.cc        data access routines
///                 These functions can be used by user programs
///                 to access data blocks held in shared memory
///    This version Mar 2003  J.Cresswell and V.Pucknell
///     Major update Dec 2014 to support use of Posix shared memory and for general use within the MIDAS data acq infrastructure
///		Edits by L. P. Gaffney to C++ -ify it, with help from D. Zemlianska - July 2022


#include <DataSpy.hh>

/// dataSpyOpen	make a connection to an the shared file for that id
///				return OK or ERROR (0 or -1)
int DataSpy::Open( int id ) {
	
	if( id < 0 || id >= MAX_ID ) {
		perror ("dataSpyOpen - id number out of range");
		return -1;
	}
	
#if( defined SOLARIS || defined POSIX )
	
	// create a file mapped object (MASTER) or obtain ID of existing object
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
	
#if( defined SOLARIS || defined POSIX )
	
	// attach the memory segment
	shm_bufferarea[id] = mmap((void *) NULL, (size_t) SHMSIZE, PROT_READ, MAP_SHARED, shmid[id], (off_t) 0);
	if( shm_bufferarea[id] == (void *) MAP_FAILED ) {
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
	
	printf( "DataSpy Shared buffer area %d (/SHM_%d) located at 0x%lx\n", id, shmkey+id, (unsigned long)shm_bufferarea[id]) ;
	
	baseaddress = (BUFFER_HEADER *) shm_bufferarea[id];
	
	number_of_buffers[id] = baseaddress->buffer_number;
	buffers_offset[id] = baseaddress->buffer_offset;
	
	/* Flush all old buffers by setting current index ... */
	
	next_index[id] = baseaddress->buffer_next;
	current_age[id] = baseaddress->buffer_currentage;
	
	printf("DataSpy Current age %lld index %d\n", current_age[id],next_index[id]);
	
	return 0;
	
}

/// dataSpyClose		close a connection to an the shared file for that id
///					close the file
///					return OK or ERROR
int DataSpy::Close( int id ) {

	if( id < 0 || id >= MAX_ID ) {
		
		perror( "DataSpy::Close - id number out of range" );
		return -1;
		
	}
	
#if (defined SOLARIS || defined POSIX)
	
	// detach the memory segment
	(void) munmap(shm_bufferarea[id], (size_t) SHMSIZE);
	
#else
	
	(void) shmdt(shm_bufferarea[id]);
	
#endif
	
	printf("DataSpy Shared buffer area %d located at 0x%lx detached\n",id,(unsigned long)shm_bufferarea[id]);
	
	return 0;

}

/// DataSpy::Read		get a block of data for that field
///					check that the id is valid and opened
///					check that there is any data in the queue
///					increment the queue read index
///					return length of data block or ERROR
///					length is in units of bytes
int DataSpy::ReadWithSeq( int id, char *data, unsigned int length, int *seq ) {
	
	int *bufferaddress;
	unsigned int len;
	int *ptr;
	
	
	if( id < 0 || id >= MAX_ID ) {
		perror( "DataSpy::Read - id number out of range" );
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
				printf( "DataSpy::Read id %d: Age %lld Index %d Buffer length %d\n",
						id, current_age[id], next_index[id], len );
			
			// copy data from shared memory to user buffer
			ptr = (int*)data;
			for( unsigned int i = 0; i < (len >> 2); i++ )
				ptr[i] = bufferaddress[i];
			*seq = (int)current_age[id];
			
			// check if the entry could have changed while copying (can happen) and if so retry
			if( current_age[id] != baseaddress->buffer_age[next_index[id]] ) {
				
				if( verbose )
					printf ("DataSpy::Read id %d: Copied oldage %lld newage %lld\n", id, current_age[id], baseaddress->buffer_age[next_index[id]]);
				
				goto retry;
				
			}
			
			next_index[id] = ++next_index[id] & (number_of_buffers[id] -1);

		}
		
	}
	
	else {
		
		if( verbose )
			printf ("DataSpy::Read - id %d has no data\n",id);
		
	}
	
	if( verbose )
		printf("DataSpy::Read - id %d length %d\n",id,len);
	
	return len;                      /* return actual length of data block */
}
/// DataSpy::Read	get a block of data for that id
/// 			as for DataSpy::ReadWithSeq, but ignoring the sequence numbering
/// 			provided for compatibility with earlier version
int DataSpy::Read( int id, char *data, unsigned int length ) {

	int seq;
	
	return ReadWithSeq( id, data, length, &seq );
	
}
/*****************************************************************************/
