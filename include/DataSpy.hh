// function declarations for data spy library
#ifndef _DataSpy_hh
#define _DataSpy_hh

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if( defined SOLARIS || defined POSIX )

#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#ifndef MACOSX
# ifdef __cplusplus
extern "C" {
# endif
	int shm_open(const char *, int, mode_t);
# ifdef __cplusplus
}
# endif
#endif
#else
# include <sys/types.h>
# include <sys/shm.h>
#endif

// format of the data buffer area header
#define NBLOCKS 64         // current number of buffers used - must be 2**n
#define MAX_BUFFERS 128    // unchangeable max because of header structure

#define MAX_ID 8
#define MAX_BUFFER_SIZE 64*1024

#define SHMSIZE 0x401000
#define SHM_KEY 110205   /* base Key */


typedef struct s_buffer_header {
    int buffer_offset;                 //    offset to first buffer
    int buffer_number;                 //    number of buffers
    int buffer_length;                 //    length of buffers
    int buffer_next;                   //    next buffer to be written
    int buffer_max;                    //    MAX_BUFFERS
    int buffer_spare1;
    int buffer_spare2;
    int buffer_spare3;
#ifdef WIN32
    int buffer_currentage;
    int padding1;
    int buffer_age[MAX_BUFFERS];
    int padding2[MAX_BUFFERS];
#else
    unsigned long long buffer_currentage;
    unsigned long long buffer_age[MAX_BUFFERS];
#endif
    int buffer_status[MAX_BUFFERS];
}  BUFFER_HEADER;


class DataSpy {
	
public:
	
	DataSpy(){};
	~DataSpy(){};
	
	inline void Verbose( int opt ) { verbose = opt; }
	
	int Open( int id );
	int Close( int id );
	int ReadWithSeq( int id, char *data, unsigned int length, int *seq );
	int Read( int id, char *data, unsigned int length );
	
#if( defined SOLARIS || defined POSIX )
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

	int verbose;

};


#endif
