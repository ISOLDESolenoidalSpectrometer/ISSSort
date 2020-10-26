#include "CommandLineInterface.hh"

using namespace std;

CommandLineInterface::CommandLineInterface() {
	
	fMaximumFlagLength = 0;
	fFlags.clear();
	fValues.clear();
	fMaximumTypeLength = 0;
	fTypes.clear();
	fMaximumCommentLength = 0;
	fComments.clear();
	
}

bool CommandLineInterface::CheckFlags( unsigned int argc, char* argv[], const bool& Debug ) {
	
	unsigned int i,j;
	
	if( argc == 1 ) {
		
		for( i = 0; i < fFlags.size(); i++ ) {
			
			if( fTypes[i].empty() )
				cout << fComments[i] << endl << endl;
		}
		
		cout << "use " << argv[0] << " with following flags:" << endl;
		
		for( i = 0; i < fFlags.size(); i++ ) {
			
			if( fTypes[i] == "bool" ) {
				
				cout << "        [" << setw(fMaximumFlagLength+fMaximumTypeLength);
				cout << left << fFlags[i] << "   : " << fComments[i] << "]" << endl;
				
			}
			
			else if( !fTypes[i].empty() ) {
				
				cout << "        [" << setw(fMaximumFlagLength) << left << fFlags[i] << " <" ;
				cout << setw(fMaximumTypeLength) << left << fTypes[i] << ">: " << fComments[i] << "]" << endl;
				
			}
			
		}
		
		return true;
		
	}
	
	for(i = 1; i < argc; i++) {
		
		for(j = 0; j < fFlags.size(); j++) {
			
			if(argv[i] == fFlags[j]) {
				
				//bool doesn't need any value to be read
				if(fTypes[j] == "bool") {
					
					*((bool*) fValues[j]) = true;
					break;//found the right flag for this argument so the flag loop can be stopped
					
				}
				
				//if not bool check whether there are more arguments (with values) coming
				else if(i+1 >= argc) {
					
					cerr<<"Error in CheckFlags, flag "<<fFlags[j]<<" needs additional arguments"<<endl;
					return false;
					
				}
				
				else if(fTypes[j] == "char*") {
					
					*((char**) fValues[j]) = argv[i+1];
					i++;
					break;//found the right flag for this argument so the flag loop can be stopped
					
				}
				
				else if(fTypes[j] == "string") {
					
					*((string*) fValues[j]) = argv[i+1];
					i++;
					break;//found the right flag for this argument so the flag loop can be stopped
					
				}
				
				else if(fTypes[j] == "int") {
					
					*((int*) fValues[j]) = atoi(argv[i+1]);
					i++;
					break;//found the right flag for this argument so the flag loop can be stopped
					
				}
				
				else if(fTypes[j] == "float") {
					
					*((float*) fValues[j]) = atof(argv[i+1]);
					i++;
					break;//found the right flag for this argument so the flag loop can be stopped
					
				}
				
				else if(fTypes[j] == "size_t") {
					
					*((size_t*) fValues[j]) = atoi(argv[i+1]);
					i++;
					break;//found the right flag for this argument so the flag loop can be stopped
					
				}
				
				else if(fTypes[j] == "long long") {
					
					*((long long*) fValues[j]) = atoi(argv[i+1]);
					i++;
					break;//found the right flag for this argument so the flag loop can be stopped
					
				}
				
				else if(fTypes[j] == "double") {
					
					*((double*) fValues[j]) = atof(argv[i+1])*fFactors[j];
					i++;
					break;//found the right flag for this argument so the flag loop can be stopped
					
				}
				
				else if(fTypes[j] == "vector<char*>") {
					
					i++;
					//as long as there are arguments left and no new flag is found (flags start with -) => read another value
					while(i < argc) {
						
						if(argv[i][0] != '-') {
							
							(*((vector<char*>*)fValues[j])).push_back(argv[i]);
							i++;
							
						}
						
						else break;
						
					}
					
					i--;
					break;//found the right flag for this argument so the flag loop can be stopped
					
				}
				
				else if(fTypes[j] == "vector<string>") {
					
					i++;
					//as long as there are arguments left and no new flag is found (flags start with -) => read another value
					while(i < argc) {
						
						if(argv[i][0] != '-') {
							
							(*((vector<string>*)fValues[j])).push_back(argv[i]);
							i++;
							
						}
						
						else break;
						
					}
					
					i--;
					break;//found the right flag for this argument so the flag loop can be stopped
					
				}
				
				else if(fTypes[j] == "vector<int>") {
					
					i++;
					//as long as there are arguments left and no new flag is found (flags start with -) => read another value
					while(i < argc) {
						
						if(argv[i][0] != '-') {
							
							(*((vector<int>*)fValues[j])).push_back(atoi(argv[i]));
							i++;
							
						}
						
						else break;
						
					}
					
					i--;
					break;//found the right flag for this argument so the flag loop can be stopped
				}
				
				else if(fTypes[j] == "vector<long long>") {
					
					i++;
					//as long as there are arguments left and no new flag is found (flags start with -) => read another value
					while( i < argc ) {
						
						if(argv[i][0] != '-') {
							(*((vector<long long>*)fValues[j])).push_back(atoi(argv[i]));
							i++;
						}
						
						else break;
						
					}
					
					i--;
					break;//found the right flag for this argument so the flag loop can be stopped
					
				}
				
				else if( fTypes[j] == "vector<double>" ) {
					i++;
					//as long as there are arguments left and no new flag is found (flags start with -) => read another value
					while( i < argc ) {
						
						if(argv[i][0] != '-') {
							
							(*((vector<double>*)fValues[j])).push_back(atof(argv[i])*fFactors[j]);
							i++;
							
						}
						
						else break;
						
					}
					
					i--;
					break;//found the right flag for this argument so the flag loop can be stopped
					
				}
				
			}//if(argv[i] == flags[j])
			
			
		}//for(j = 0; j < flags.size(); j++)
		
		if( j == fFlags.size() ) { //this means no matching flag was found
			
			cerr<<"flag "<<argv[i]<<" unknown"<<endl;
		}
		
		else if(Debug) cout<<"found flag "<<i<<" = "<<argv[i]<<endl;
		
	}//for(i = 1; i < argc; i++)
	
	if(Debug) cout<<*this<<endl;
	
	return true;
	
}

ostream& operator <<(ostream &os,const CommandLineInterface &obj) {
	
	os<<"command line flags are:"<<endl;
	for( unsigned int i = 0; i < obj.fValues.size(); i++) {
		
		if(obj.fTypes[i] == "bool")
			cout<<obj.fFlags[i]<<": "<<*((bool*) obj.fValues[i])<<endl;
		
		
		else if(obj.fTypes[i] == "char*")
			cout<<obj.fFlags[i]<<": "<<*((char**) obj.fValues[i])<<endl;
		
		else if(obj.fTypes[i] == "string")
			cout<<obj.fFlags[i]<<": "<<*((string*) obj.fValues[i])<<endl;
		
		else if(obj.fTypes[i] == "int")
			cout<<obj.fFlags[i]<<": "<<*((int*) obj.fValues[i])<<endl;
		
		else if(obj.fTypes[i] == "float")
			cout<<obj.fFlags[i]<<": "<<*((float*) obj.fValues[i])<<endl;
		
		else if(obj.fTypes[i] == "long long")
			cout<<obj.fFlags[i]<<": "<<*((long*) obj.fValues[i])<<endl;
		
		else if(obj.fTypes[i] == "double")
			cout<<obj.fFlags[i]<<": "<<*((double*) obj.fValues[i])<<endl;
		
		else if(obj.fTypes[i] == "vector<char*>") {
			
			cout<<obj.fFlags[i]<<": ";
			for( unsigned int j = 0; j < ((vector<char*>*) obj.fValues[i])->size(); j++)
				cout<<(*((vector<char*>*) obj.fValues[i]))[j]<<" ";
			cout<<endl;
			
		}
		
		else if(obj.fTypes[i] == "vector<string>") {
			
			cout<<obj.fFlags[i]<<": ";
			for( unsigned int j = 0; j < ((vector<string>*) obj.fValues[i])->size(); j++)
				cout<<(*((vector<string>*) obj.fValues[i]))[j]<<" ";
			cout<<endl;
			
		}
		
		else if(obj.fTypes[i] == "vector<int>") {
			
			cout<<obj.fFlags[i]<<": ";
			for( unsigned int j = 0; j < ((vector<int>*) obj.fValues[i])->size(); j++)
				cout<<(*((vector<int>*) obj.fValues[i]))[j]<<" ";
			cout<<endl;
			
		}
		
		else if(obj.fTypes[i] == "vector<long long>") {
			
			cout<<obj.fFlags[i]<<": ";
			for( unsigned int j = 0; j < ((vector<long long>*) obj.fValues[i])->size(); j++)
				cout<<(*((vector<long long>*) obj.fValues[i]))[j]<<" ";
			cout<<endl;
			
		}
		
		else if(obj.fTypes[i] == "vector<double>") {
			
			cout<<obj.fFlags[i]<<": ";
			for( unsigned int j = 0; j < ((vector<double>*) obj.fValues[i])->size(); j++)
				cout<<(*((vector<double>*) obj.fValues[i]))[j]<<" ";
			cout<<endl;
			
		}
		
	}
	
	return os;
	
}

void CommandLineInterface::Add(const char* comment)
{
	fFlags.push_back(string());
	fValues.push_back((void*) NULL);
	fTypes.push_back(string());
	if(strlen(comment) > fMaximumCommentLength)
		fMaximumCommentLength = strlen(comment);
	fComments.push_back(string(comment));
	fFactors.push_back(1.);
}

void CommandLineInterface::Add(const char* flag, const char* comment, bool* value)
{
	if(strlen(flag) > fMaximumFlagLength)
		fMaximumFlagLength = strlen(flag);
	fFlags.push_back(string(flag));
	fValues.push_back((void*) value);
	if(strlen("bool") > fMaximumTypeLength)
		fMaximumTypeLength = strlen("bool");
	fTypes.push_back(string("bool"));
	if(strlen(comment) > fMaximumCommentLength)
		fMaximumCommentLength = strlen(comment);
	fComments.push_back(string(comment));
	fFactors.push_back(1.);
}

void CommandLineInterface::Add(const char* flag, const char* comment, char** value)
{
	if(strlen(flag) > fMaximumFlagLength)
		fMaximumFlagLength = strlen(flag);
	fFlags.push_back(string(flag));
	fValues.push_back((void*) value);
	if(strlen("char*") > fMaximumTypeLength)
		fMaximumTypeLength = strlen("char*");
	fTypes.push_back(string("char*"));
	if(strlen(comment) > fMaximumCommentLength)
		fMaximumCommentLength = strlen(comment);
	fComments.push_back(string(comment));
	fFactors.push_back(1.);
}

void CommandLineInterface::Add(const char* flag, const char* comment, string* value)
{
	if(strlen(flag) > fMaximumFlagLength)
		fMaximumFlagLength = strlen(flag);
	fFlags.push_back(string(flag));
	fValues.push_back((void*) value);
	if(strlen("string") > fMaximumTypeLength)
		fMaximumTypeLength = strlen("string");
	fTypes.push_back(string("string"));
	if(strlen(comment) > fMaximumCommentLength)
		fMaximumCommentLength = strlen(comment);
	fComments.push_back(string(comment));
	fFactors.push_back(1.);
}

void CommandLineInterface::Add(const char* flag, const char* comment, int* value)
{
	if(strlen(flag) > fMaximumFlagLength)
		fMaximumFlagLength = strlen(flag);
	fFlags.push_back(string(flag));
	fValues.push_back((void*) value);
	if(strlen("int") > fMaximumTypeLength)
		fMaximumTypeLength = strlen("int");
	fTypes.push_back(string("int"));
	if(strlen(comment) > fMaximumCommentLength)
		fMaximumCommentLength = strlen(comment);
	fComments.push_back(string(comment));
	fFactors.push_back(1.);
}

void CommandLineInterface::Add(const char* flag, const char* comment, float* value)
{
	if(strlen(flag) > fMaximumFlagLength)
		fMaximumFlagLength = strlen(flag);
	fFlags.push_back(string(flag));
	fValues.push_back((void*) value);
	if(strlen("float") > fMaximumTypeLength)
		fMaximumTypeLength = strlen("float");
	fTypes.push_back(string("float"));
	if(strlen(comment) > fMaximumCommentLength)
		fMaximumCommentLength = strlen(comment);
	fComments.push_back(string(comment));
	fFactors.push_back(1.);
}

void CommandLineInterface::Add(const char* flag, const char* comment, size_t* value)
{
	if(strlen(flag) > fMaximumFlagLength)
		fMaximumFlagLength = strlen(flag);
	fFlags.push_back(string(flag));
	fValues.push_back((void*) value);
	if(strlen("int") > fMaximumTypeLength)
		fMaximumTypeLength = strlen("size_t");
	fTypes.push_back(string("size_t"));
	if(strlen(comment) > fMaximumCommentLength)
		fMaximumCommentLength = strlen(comment);
	fComments.push_back(string(comment));
	fFactors.push_back(1.);
}

void CommandLineInterface::Add(const char* flag, const char* comment, long long* value)
{
	if(strlen(flag) > fMaximumFlagLength)
		fMaximumFlagLength = strlen(flag);
	fFlags.push_back(string(flag));
	fValues.push_back((void*) value);
	if(strlen("long long") > fMaximumTypeLength)
		fMaximumTypeLength = strlen("long long");
	fTypes.push_back(string("long long"));
	if(strlen(comment) > fMaximumCommentLength)
		fMaximumCommentLength = strlen(comment);
	fComments.push_back(string(comment));
	fFactors.push_back(1.);
}

void CommandLineInterface::Add(const char* flag, const char* comment, double* value, double factor)
{
	if(strlen(flag) > fMaximumFlagLength)
		fMaximumFlagLength = strlen(flag);
	fFlags.push_back(string(flag));
	fValues.push_back((void*) value);
	if(strlen("double") > fMaximumTypeLength)
		fMaximumTypeLength = strlen("double");
	fTypes.push_back(string("double"));
	if(strlen(comment) > fMaximumCommentLength)
		fMaximumCommentLength = strlen(comment);
	fComments.push_back(string(comment));
	fFactors.push_back(factor);
}

void CommandLineInterface::Add(const char* flag, const char* comment, vector<char*>* value)
{
	if(strlen(flag) > fMaximumFlagLength)
		fMaximumFlagLength = strlen(flag);
	fFlags.push_back(string(flag));
	fValues.push_back((void*) value);
	if(strlen("vector<char*>") > fMaximumTypeLength)
		fMaximumTypeLength = strlen("vector<char*>");
	fTypes.push_back(string("vector<char*>"));
	if(strlen(comment) > fMaximumCommentLength)
		fMaximumCommentLength = strlen(comment);
	fComments.push_back(string(comment));
	fFactors.push_back(1.);
}

void CommandLineInterface::Add(const char* flag, const char* comment, vector<string>* value)
{
	if(strlen(flag) > fMaximumFlagLength)
		fMaximumFlagLength = strlen(flag);
	fFlags.push_back(string(flag));
	fValues.push_back((void*) value);
	if(strlen("vector<string>") > fMaximumTypeLength)
		fMaximumTypeLength = strlen("vector<string>");
	fTypes.push_back(string("vector<string>"));
	if(strlen(comment) > fMaximumCommentLength)
		fMaximumCommentLength = strlen(comment);
	fComments.push_back(string(comment));
	fFactors.push_back(1.);
}

void CommandLineInterface::Add(const char* flag, const char* comment, vector<int>* value)
{
	if(strlen(flag) > fMaximumFlagLength)
		fMaximumFlagLength = strlen(flag);
	fFlags.push_back(string(flag));
	fValues.push_back((void*) value);
	if(strlen("vector<int>") > fMaximumTypeLength)
		fMaximumTypeLength = strlen("vector<int>");
	fTypes.push_back(string("vector<int>"));
	if(strlen(comment) > fMaximumCommentLength)
		fMaximumCommentLength = strlen(comment);
	fComments.push_back(string(comment));
	fFactors.push_back(1.);
}

void CommandLineInterface::Add(const char* flag, const char* comment, vector<long long>* value)
{
	if(strlen(flag) > fMaximumFlagLength)
		fMaximumFlagLength = strlen(flag);
	fFlags.push_back(string(flag));
	fValues.push_back((void*) value);
	if(strlen("vector<long long>") > fMaximumTypeLength)
		fMaximumTypeLength = strlen("vector<long long>");
	fTypes.push_back(string("vector<long long>"));
	if(strlen(comment) > fMaximumCommentLength)
		fMaximumCommentLength = strlen(comment);
	fComments.push_back(string(comment));
	fFactors.push_back(1.);
}

void CommandLineInterface::Add(const char* flag, const char* comment, vector<double>* value, double factor)
{
	if(strlen(flag) > fMaximumFlagLength)
		fMaximumFlagLength = strlen(flag);
	fFlags.push_back(string(flag));
	fValues.push_back((void*) value);
	if(strlen("vector<double>") > fMaximumTypeLength)
		fMaximumTypeLength = strlen("vector<double>");
	fTypes.push_back(string("vector<double>"));
	if(strlen(comment) > fMaximumCommentLength)
		fMaximumCommentLength = strlen(comment);
	fComments.push_back(string(comment));
	fFactors.push_back(factor);
}
