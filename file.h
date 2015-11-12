#ifndef __FILE_H__
#define __FILE_H__

#include <string>
#include <vector>
#include <set>

using namespace std;

const char g_lineEnd = '\n';			//the end of the line

class File
{
public:
	File(){}
	File( const string &fileName )
	{m_file = fileName;}

	//read file content into a string
	bool read( string &fileContent );	

	//read file content into a vector
	bool read( vector<string> &vFile );

	bool read( set<string> &sFile );

	//write file content into m_file
	bool write( const string &fileContent );

	bool write( const set<string> &sContent );

	bool write( const vector<string> &vFile );

	void setPath( const string &path )
	{m_file = path;}
private:
	string m_file;		//the name of the file
};

#endif
