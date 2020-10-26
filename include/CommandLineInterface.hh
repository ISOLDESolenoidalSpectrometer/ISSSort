#ifndef __COMMAND_LINE_INTERFACE
#define __COMMAND_LINE_INTERFACE

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>

using namespace std;

class CommandLineInterface {
	
public:
	CommandLineInterface();
	~CommandLineInterface(){};
	
	//main functions to check all flags from command line
	bool CheckFlags( unsigned int, char*[], const bool& Debug = false );
	
	//functions to add flags
	void Add(const char*);
	void Add(const char*, const char*, bool*);
	void Add(const char*, const char*, char**);
	void Add(const char*, const char*, string*);
	void Add(const char*, const char*, int*);
	void Add(const char*, const char*, float*);
	void Add(const char*, const char*, size_t*);
	void Add(const char*, const char*, long long*);
	void Add(const char*, const char*, double*, double factor = 1.);
	void Add(const char*, const char*, vector<char*>*);
	void Add(const char*, const char*, vector<string>*);
	void Add(const char*, const char*, vector<int>*);
	void Add(const char*, const char*, vector<long long>*);
	void Add(const char*, const char*, vector<double>*, double factor = 1.);
	
	friend ostream& operator <<(ostream &,const CommandLineInterface &);
	
private:
	unsigned int fMaximumFlagLength;
	vector<string> fFlags;
	vector<void*>  fValues;
	unsigned int fMaximumTypeLength;
	vector<string> fTypes;
	unsigned int fMaximumCommentLength;
	vector<string> fComments;
	vector<double> fFactors;
	
};

#endif
