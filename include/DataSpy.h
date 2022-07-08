/*******************************************************************************
* dataspy.h     function declarations for data spy library   
*/
#ifndef INClibspyH
#define INClibspyH

extern int dataSpyOpen   (int stream);
extern int dataSpyClose  (int stream);
extern int dataSpyRead   (int stream, char *data, int length);
extern int dataSpyReadWithSeq (int stream, char *data, int length, int * tag);
extern void dataSpyVerbose (int opt);

/*    format of the data buffer area header */

#define NBLOCKS 64         /*  current number of buffers used - must be 2**n */
#define MAX_BUFFERS 128    /*  unchangeable max because of header structure */

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

#endif
