#ifndef __QUERYEXPAND_H__
#define __QUERYEXPAND_H__

#include <iostream>
#include <string>
#include <map>
#include <set>
#include <vector>

#include "file.h"

using namespace std;

class QueryExpand
{

private:
	char* origin_query_file_path; // original query file
	char* expaned_query_file_path; // output for expanded query 
	char* index_path; // a file that contains the (distributed) index that 
	                  // the original query search against. One line per index.
	                  // NOTE: THIS IS NOT THE INDEX PATH !!! IT CONTAINS THE
	                  // INDEX PATH !!!
    char* result_file_path; // result file of first search in TREC format.

public:
	QueryExpand(char* origin_qf, char* expanded_qf, char* index_qf, char* 
	result_fn):
        origin_query_file_path(origin_qf),expaned_query_file_path(expanded_qf),index_path(index_qf), 
        result_file_path(result_fn)
    {

    }

	inline string get_origin_qf(){ return origin_query_file_path; }
	inline string get_expanded_qf(){ return expaned_query_file_path; }
	inline string get_index_path(){ return index_path; }
    inline string get_result_file_path(){ return result_file_path; }

    void printIndexPaths(vector<string> &vIndex);
    void printQueryContainer(map<int, map<string, int> > &mQuery);
    void printQueryDoc(map< int, map<int, set<int> > > &mQueryDoc, bool show_details=true);
	void printTermsContainer(map<string, set<string> > sTerm);

	void expand( int M, int R, int L, int K, float beta );


	bool readOrinQuery( const string &queryFile, map<int, map<string, int> > &mmQuery );

	void printTerm( const string &indexPath, const string &segPath, string queryFile, const string &resPath, const string &name );
	void selQueryTerm( map< int, set<int> > &mDoc, vector<string> &vIndex, map<string, int> &mQuery, map<string, set<string> > &mTerm );

	void printTermTfIndexes( const string &indexPath, const string &retFile, const string &resultPath );
	void printTermTfIndexesStopword( const string &indexPath, const string &retFile, const string &resultPath );

	void calcuMI( const string &termFile, const string &termPairFile, const string &indexFile );

	//calculate term mi in the retrieved doc
	void calcuMIInRet( const string &termFile, const string &termPairFile, const string &docFile, const string &indexFile );


protected:
	/*
	split the string s into a vector of string based on delimiter.
	When splitting, if there are multiple delimiters next to each other,
	all of them are considered as delimiter.
	e.g. split("a=>b=>=>c", "=>") will give the result ["a","b","c"]
	*/
	vector<string> split(string s, const string delimiter=string(" "));
	/*
	stem a string and return the result
	*/
	string stem( const string &srcStr );
	
	//read the index paths from index file
	bool readIndexPaths(vector<string> &vIndex);

	//read the query term and tf with stemming
	bool readQuery(map<int, map<string, int> > &mmQuery, 
		indri::api::Parameters& params, set<int> &qIDs);

	//select top M results of each query
	bool readResultsFile(int M, set<int> &QIDs, vector<string> &vIndex, 
		map< int, map<int, set<int> > > &mQueryDoc);

	//randomly select R*M docs from all the indexs and insert into mQueryDoc
	void randomSelDoc( vector<string> &vIndex, int rm, 
		map< int, map<int, set<int> > > &mQueryDoc );

	//extract terms from mQueryDoc. These terms are used to generate new queries.
	void extractTerms(map< int, map<int, set<int> > > &mQueryDoc, 
	vector<string> &vIndex, map<int, map<string, int> > &mmQuery,
	int L, int K, float beta, map< int, map<string, float> > &mmTermScore);

	//select all terms in sDoc
	void extractAllTerms( map< int, set<int> > &mDoc, vector<string> &vIndex, 
		map<string, set<string> > &mTerm );  

	//calculate the mi between terms and query terms. result is quer term-term-score
	void calcuTermScore( map<string, int> &mQueryTerm, map<string, 
		set<string> > &mSetTerm, int L, float beta, 
		map< string, map<string, float> > &mmScore );

	float calcuMI( set<string> &sSrc, set<string> &sDes, set<string> &sSet );
 
	//calculate the score of each term to the whole query
	void calcuQueryScore( map<string, int> &mQueryTerm, map< string, 
		map<string, float> > &mmScore, int K, map<string, float> &mTermScore );

	//write the result of query with weight of each term
	void writeQuery( map<int, map<string, int> > &mmQuery,
		map< int, map<string, float> > &mmTermScore, 
		indri::api::Parameters& params, const string &desFile );
};

#endif
