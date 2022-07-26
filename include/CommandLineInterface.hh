#ifndef __COMMAND_LINE_INTERFACE
#define __COMMAND_LINE_INTERFACE

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>

using namespace std;

/*!
* \brief Implements a command line interface for the ISS sort code
*
* This class adds a command line interface to ISSSort, adding some control to the sort through the command line. Different flags can be added using the CommandLineInterface::Add commands, which then populate a series of variables defined in the iss_sort.cc file. The CommandLineInterface::CheckFlags() can print information about the flags already defined, but also checks that there are no errors with the current input.
*
*/
class CommandLineInterface {
	
public:
	CommandLineInterface();		///< Constructor
	~CommandLineInterface(){};	///< Destructor (currently does nothing)
	
	//main functions to check all flags from command line
	bool CheckFlags( unsigned int, char*[], const bool& Debug = false );
	
	//functions to add flags
	void Add(const char*); ///< Add flag function [just comments]
	void Add(const char*, const char*, bool*);///< Add flag function [boolean]
	void Add(const char*, const char*, char**);///< Add flag function [char*]
	void Add(const char*, const char*, string*);///< Add flag function [string]
	void Add(const char*, const char*, int*);///< Add flag function [int]
	void Add(const char*, const char*, float*);///< Add flag function [float]
	void Add(const char*, const char*, size_t*);///< Add flag function [size_t]
	void Add(const char*, const char*, long long*);///< Add flag function [long long]
	void Add(const char*, const char*, double*, double factor = 1.);///< Add flag function [double]
	void Add(const char*, const char*, vector<char*>*);///< Add flag function [vector<char*>]
	void Add(const char*, const char*, vector<string>*);///< Add flag function [vector<string>]
	void Add(const char*, const char*, vector<int>*);///< Add flag function [vector<int>]
	void Add(const char*, const char*, vector<long long>*);///< Add flag function [vector<long long>]
	void Add(const char*, const char*, vector<double>*, double factor = 1.);///< Add flag function [vector<double>]
	
	friend ostream& operator <<(ostream &,const CommandLineInterface &);///< Overloaded << operator
	
private:
	unsigned int fMaximumFlagLength;///< Stores the maximum length of the flags (for printing purposes)
	vector<string> fFlags;///< Holds the defined flags for the CommandLineInterface
	vector<void*>  fValues;///< Holds the addresses of the variables pertinent to each flag
	unsigned int fMaximumTypeLength;///< Stores the maximum length of the types (for printing purposes)
	vector<string> fTypes;///< Holds the list of types relevant to each flag
	unsigned int fMaximumCommentLength;///< Stores the maximum length of the comments (for printing purposes)
	vector<string> fComments;///< Holds the list of comments pertinent to each flag
	vector<double> fFactors;///< Holds the factors pertinent to each flag
	
};

#endif
