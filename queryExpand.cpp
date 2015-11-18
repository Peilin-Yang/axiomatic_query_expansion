#include <string>
#include <sstream>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <math.h>

#include "indri/IndexEnvironment.hpp"
#include "indri/Index.hpp"
#include "indri/QueryEnvironment.hpp"
#include "indri/CompressedCollection.hpp"
#include "indri/LocalQueryServer.hpp"
#include "indri/Porter_Stemmer.hpp"
#include "indri/KrovetzStemmer.hpp"
#include "indri/DocListIterator.hpp"

#include "queryExpand.h"
#include "file.h"



#define VERBOSE 0

vector<string> QueryExpand::split( string s, const string delimiter)
{
    vector<string> v;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        v.push_back(token);
        s.erase(0, pos + delimiter.length());
        while ((pos = s.find(delimiter)) == 0){
            s.erase(0, delimiter.length());
        }
    }
    
    v.push_back(s);
    return v;
}

string QueryExpand::stem( const string &srcStr )
{
    string term=srcStr;
    #if 1
    // porter stemmer
    // Use porter stemmer if the index building uses porter stemmer
    //cout<<" porter stem in use"<<endl;
    indri::parse::Porter_Stemmer * stemmer = new indri::parse::Porter_Stemmer();		
    char *tmpTerm = (char *)malloc(term.size()+1);
    strcpy(tmpTerm, term.c_str());
    tmpTerm[term.size()] = '\0';
    int ret = stemmer->porter_stem(tmpTerm, 0, strlen(tmpTerm) - 1);
    tmpTerm[ret+1] = '\0';
    term = tmpTerm;
    #else
    // krovetz stemmer
    // Use krovert stemmer if the index building uses krovetz stemmer
    //cout<<" k stem in use"<<endl;
    indri::parse::KrovetzStemmer * stemmer = new indri::parse::KrovetzStemmer();
    char * cstr = new char [srcStr.length()+1];
    std::strcpy (cstr, srcStr.c_str());
    char *tmpTerm = stemmer->kstem_stemmer(cstr);
    term = tmpTerm;
    #endif

    return term;
}


void QueryExpand::printIndexPaths(vector<string> &vIndex)
{
    cout << endl;
    cout << "index path:" << endl;
    for (vector<string>::iterator it=vIndex.begin(); it!=vIndex.end(); ++it){
        cout << *it << endl;
    }
    cout << endl;
}


void QueryExpand::printQueryContainer(map<int, map<string, int> > &mmQuery)
{
    cout << endl;
    for (map<int, map<string, int> >::iterator it=mmQuery.begin(); it!=mmQuery.end(); ++it){
        cout << "query number:" << it->first << endl;
        for (map<string, int>::iterator i=it->second.begin(); i!=it->second.end(); ++i){
            cout << i->first << " " << i->second << endl;
        }
        cout << "--------------------------" << endl;
    }
    cout << endl;
}


void QueryExpand::printQueryDoc(map< int, map<int, set<int> > > &mQueryDoc, bool show_details)
{
	/*
	show_details: 
	True to show details, 
	False to show only the documents size for each query.
	*/
    cout << endl;
	map< int, map<int, set<int> > >::iterator iter1;
	for( iter1 = mQueryDoc.begin(); iter1 != mQueryDoc.end(); ++iter1 )
	{
		if (show_details)
		{
			cout << iter1->first << endl;
		}
		map<int, set<int> >::iterator iter2;
		int size = 0;
		for(iter2 = iter1->second.begin(); iter2 != iter1->second.end(); ++iter2)
		{
			set<int>::iterator iter3;
			for (iter3 = iter2->second.begin(); iter3 != iter2->second.end(); ++iter3)
			{
				if (show_details)
				{
					cout << iter1->first << " " << iter2->first << " " << *iter3 << endl;
				}
				size++;
			}
		}
		cout << iter1->first << " size:" << size << endl;
	}
    cout << endl;
}


void QueryExpand::printTermsContainer(map<string, set<string> > sTerm)
{
    cout << endl;
    for (map<string, set<string> >::iterator it=sTerm.begin(); it!=sTerm.end(); ++it){
        cout << "term:" << it->first << endl;
        for (set<string>::iterator i=it->second.begin(); i!=it->second.end(); ++i){
            cout << *i << endl;
        }
        cout << "--------------------------" << endl;
    }
    cout << endl;
}



bool QueryExpand::readIndexPaths(vector<string> &vIndex)
{
    File file(get_index_path());
    if (!file.read(vIndex))
    {
        cout << "Cannot read index file:" << get_index_path() << endl;
		return false;    
	}

	if (vIndex.size() <= 0)
	{
		cout << "Index file MUST contain at least one line!" << endl;
		return false;
	}

    #if VERBOSE
	printIndexPaths(vIndex);
    #endif

    return true;
}


bool QueryExpand::readQuery(map<int, map<string, int> > &mQuery,
	indri::api::Parameters& params, set<int> &qIDs)
{
    try {
        indri::api::Parameters& param = indri::api::Parameters::instance();
        string queryFile = get_origin_qf();
        param.loadFile( queryFile );

        params = param;

        if( !param.exists( "query" ) )
        {
            return false;
        }

        indri::api::Parameters queries = param[ "query" ];

        if (queries.size() <= 0)
        {
			cout << "Query file MUST contain at least one query!" << endl;
			return false;
        }

        for( size_t i=0; i<queries.size(); i++ ) {
            std::string queryNumber;
            std::string queryText;
            if (queries[i].exists("text")){
                queryText = (std::string) queries[i]["text"] ;
            } else {
                cout << "TREC query MUST have text!!" << endl;
                return false;
            }
            if( queries[i].exists( "number" ) ) {
                queryNumber = (std::string) queries[i]["number"];
            } else {
                cout << "TREC query MUST have number!!" << endl;
                return false;
            }

            //cout << queryText << endl;

            vector<string> splitted_query = split(queryText);

            map<string, int> mTerm;
            map<string, int>::iterator mTermIter;

            //cout << splitted_query.size() << endl;
            for (int i = 0; i != splitted_query.size(); ++i){
                //cout << splitted_query[i] << endl;
                string stemmed_term=stem(splitted_query[i]);
                //cout <<stemmed_term<<endl;
                if( (mTermIter = mTerm.find(stemmed_term)) != mTerm.end() )
                {
    				mTermIter->second += 1;
                } else{
    				mTerm.insert(map<string, int>::value_type(stemmed_term, 1));
                }
            }

            int qid = atoi(queryNumber.c_str());
            qIDs.insert(qid);
	    mQuery.insert(map<int, map<string, int> >::value_type(qid, mTerm));
            //cout << queryNumber << queryText << endl;
        }
    } catch( lemur::api::Exception& e ) {
        LEMUR_ABORT(e);
    } catch( ... ) {
        std::cout << "Caught unhandled exception" << std::endl;
        exit(1);
    }
    
    //test
	#if VERBOSE
	printQueryContainer(mQuery);
    #endif

	return true;
}


//select top M results of each query
bool QueryExpand::readResultsFile(int M, set<int> &QIDs, vector<string> &vIndex, 
	map< int, map<int, set<int> > > &mQueryDoc)
{
	if (M <= 0)
	{
		cout << "M should be bigger than 0!" << endl;
		return false;
	}

	vector<string> vFile;
	string results_file_path = get_result_file_path();
	File file(results_file_path);
	if( !file.read(vFile) ){
        cout << "Cannot read result file:" << results_file_path << endl;
		return false;
    }

	if (vFile.size() <= 0)
	{
		cout << "Results file is Empty!" << endl;
		return false;
	}
	
	/*
	mTmpQueryDoc: a container that stores the initial parsing results of results file.
	queryID:[documentID1, documentID2, ....]
	*/
	map<int, set<string> > mTmpQueryDoc;

    vector<string>::iterator vIter;
	for( vIter = vFile.begin(); vIter != vFile.end(); ++vIter )
	{
		vector<string> one_row = split(*vIter);
		/*
		the format of results file is TREC format:
		queryID Q0 documentID rank document_score runID
		*/
		map<int, set<string> >::iterator tmpIter;
		int queryID = atoi( one_row[0].c_str() );
		if (QIDs.find(queryID) == QIDs.end())
		{
			continue;
		}

		if( (tmpIter = mTmpQueryDoc.find(queryID)) != mTmpQueryDoc.end() 
			&& tmpIter->second.size() < M)
        {
			tmpIter->second.insert(one_row[2]);
        } else{
        	set<string> sDoc;
        	sDoc.insert(one_row[2]);
			mTmpQueryDoc.insert(map< int, set<string> >::value_type(queryID, sDoc));
        }		
	}

	#if 0
    cout << "tempQueryDoc:" << mTmpQueryDoc.size() << endl;
    for (map<int, set<string> >::iterator tmpIter=mTmpQueryDoc.begin(); tmpIter!=mTmpQueryDoc.end(); tmpIter++)
    {
    	cout << tmpIter->first << endl;
    	for (set<string>::iterator i = tmpIter->second.begin(); i!=tmpIter->second.end(); i++)
    	{
    		cout << *i << endl;
    	}
    	cout << "-------------------" << endl;
    }
    #endif

	//collect all docs
	map< string, pair<int, int> > mDocIndexDocId;
	map<int, set<string> >::iterator mTmpIter;
	set<string>::iterator sDocIter;
	for( mTmpIter = mTmpQueryDoc.begin(); mTmpIter != mTmpQueryDoc.end(); ++mTmpIter )
	{
    	for( sDocIter = mTmpIter->second.begin(); sDocIter != mTmpIter->second.end(); ++sDocIter )
    	{
    		mDocIndexDocId.insert( map< string, pair<int, int> >::value_type( *sDocIter, make_pair<int,int>(-1, -1)));
        }
	}
                
    //cout << "docSize:" << mDocIndexDocId.size() << endl;
	//get the index and docId and change mTmpQueryDoc to mQueryDoc
	map< string, pair<int, int> >::iterator mDocIndexIter;
	for( int indexId = 0; indexId < vIndex.size(); ++indexId )
    {
        //cout << vIndex[indexId] << endl;
		indri::collection::Repository* repository = new indri::collection::Repository();
    	repository->openRead( vIndex[indexId] );
        indri::index::Index* index = (*(repository->indexes()))[0];
		indri::collection::CompressedCollection* collection = repository->collection();
		//cout << index->documentCount() << endl;
        int docCount = index->documentCount();
        	
		//get the docno according to docId
		string docno;
		for( int i = 1; i <= docCount; ++i )
        {
			docno = collection->retrieveMetadatum( i, "docno" );
			if( (mDocIndexIter = mDocIndexDocId.find(docno)) != mDocIndexDocId.end() )	//find the docId in current index
			{
				mDocIndexIter->second.first = indexId;
				mDocIndexIter->second.second = i;
				//cout << indexId << " docId:" << i << " docno:" << docno << endl;
			}
		}

	    collection->close();
        repository->close();
		delete repository;
    }


	for( mTmpIter = mTmpQueryDoc.begin(); mTmpIter != mTmpQueryDoc.end(); ++mTmpIter )
    {
		map<int, set<int> > mDocInfo;
		set<int> sTmpDoc;
		map<int, set<int> >::iterator mDocInfoIter;
		int queryId = mTmpIter->first, docId, indexId;
		for( sDocIter = mTmpIter->second.begin(); sDocIter != mTmpIter->second.end(); ++sDocIter )
		{
			if( ( mDocIndexIter = mDocIndexDocId.find(*sDocIter) ) != mDocIndexDocId.end() )
			{
				indexId = mDocIndexIter->second.first;
				docId = mDocIndexIter->second.second;
				if( (mDocInfoIter = mDocInfo.find(indexId) ) == mDocInfo.end()  )
				{
					sTmpDoc.clear();
					sTmpDoc.insert(docId);
					mDocInfo.insert(  map<int, set<int> >::value_type(indexId, sTmpDoc) );	
				}
				else
				{
					mDocInfoIter->second.insert(docId);
				}
			}
		}

		mQueryDoc.insert( map< int, map<int, set<int> > >::value_type( queryId, mDocInfo ) );
	}		

	//test output
	#if VERBOSE
	printQueryDoc(mQueryDoc, true);
	#endif

	return true;
}


void QueryExpand::randomSelDoc( vector<string> &vIndex, int rm, 
	map< int, map<int, set<int> > > &mQueryDoc )
{
	int rmEach = rm/vIndex.size();
	int rmLast = rm%vIndex.size();

	srand ( time(NULL) );

	set<int> sTmpDoc;
	map< int, map<int, set<int> > >::iterator mQueryIter;
	map<int, set<int> >::iterator mIndexIter;
	set<int>::iterator docIter;
	int size = vIndex.size();
	for( int indexId = 0; indexId < size; ++indexId )
	{
		indri::collection::Repository* repository = new indri::collection::Repository();
    	repository->openRead( vIndex[indexId] );
        indri::index::Index* index = (*(repository->indexes()))[0];
        int docCount = index->documentCount();
		
		for( mQueryIter = mQueryDoc.begin(); mQueryIter != mQueryDoc.end(); ++mQueryIter )
		{
			sTmpDoc.clear();
			//cout << mQueryIter->first << " ";
			// accmulate the numbers of existed documents (for query expansion) of each query ID 
			int currentSize = 0;
			for( mIndexIter = mQueryIter->second.begin(); mIndexIter != mQueryIter->second.end(); ++mIndexIter )	
			{
				currentSize += mIndexIter->second.size();
			}

			int desSize = currentSize + rmEach;
			if( indexId == size-1 )
			{
				desSize += rmLast; //the number of docs after adding rmEach docs
			}
			while( currentSize < desSize )
			{
				int selDoc = rand() % docCount + 1;		
			
				if( (mIndexIter = mQueryIter->second.find(indexId)) != mQueryIter->second.end() )
				{
					if (mIndexIter->second.find(selDoc) == mIndexIter->second.end())
					{
						mIndexIter->second.insert(selDoc);
						currentSize++;
					}
				} else {
					set<int> sTmp;
					sTmp.insert(selDoc);
					mQueryIter->second.insert( map<int, set<int> >::value_type(indexId, sTmp) );
					currentSize++;
				}
			}
		}

        repository->close();
	}

	//test output
	#if VERBOSE
	printQueryDoc(mQueryDoc, false);
	#endif
}


void QueryExpand::extractAllTerms( map< int, set<int> > &mDoc, 
	vector<string> &vIndex, map<string, set<string> > &mTerm )
{
    map< int, set<int> >::iterator mDocIter;
    set<int>::iterator sDocIter;
    for( mDocIter = mDoc.begin(); mDocIter != mDoc.end(); ++mDocIter ) //browse each index
    {
        int indexId = mDocIter->first;
        indri::collection::Repository* repository = new indri::collection::Repository();
        repository->openRead( vIndex[indexId] );
        indri::server::LocalQueryServer local(*repository);
        indri::index::Index* index = (*(repository->indexes()))[0];
        vector<int> vDoc;

        vDoc.reserve(1000);
        for( sDocIter = mDocIter->second.begin(); sDocIter != mDocIter->second.end(); ++sDocIter )
        {
            vDoc.push_back(*sDocIter);
        }
        indri::server::QueryServerVectorsResponse* response = local.documentVectors( vDoc );

        int docNum = -1;
        ////////////
        map<string, set<string> >::iterator mTermIter;
        stringstream indexDoc;
        set<string> sTmp;

        for( sDocIter = mDocIter->second.begin(); sDocIter != mDocIter->second.end(); ++sDocIter ) 
        //browse each doc
        {
            ++docNum;
            if( response->getResults().size() )
            {
                indri::api::DocumentVector* docVector = response->getResults()[docNum];
                int termSize = docVector->positions().size();
                for( size_t i=0; i<termSize; i++ )
                {
                    string term = docVector->stems()[ docVector->positions()[i] ];
                    bool del = false;
                    int size = term.size();
                    for( int i = 0; i < size; ++i )
                    {
                        if( !( ( term[i] >= 'a' && term[i] <= 'z' ) || ( term[i] >= 'A' && term[i] <= 'Z' ) ) )
                        {
                            del = true;
                            break;
                        }
                    }

                    if(del)
                    {
                        continue;
                    }

                    indexDoc.str("");
                    indexDoc << indexId << "-" << *sDocIter; //indexId-docId

                    if( (mTermIter = mTerm.find(term)) != mTerm.end() ) //existing term
                    {
                        mTermIter->second.insert(indexDoc.str());
                    } else {
                        sTmp.clear();
                        sTmp.insert( indexDoc.str() );
                        mTerm.insert( map<string, set<string> >::value_type(term, sTmp) );
                    }
                    //std::cout << i << " " << position << " " << stem << " size:"<< docVector->positions().size() << std::endl;
                }
                delete docVector;
            }
        }
        delete response;
        repository->close();
        delete repository;
    }

    #if VERBOSE
    //printTermsContainer(mTerm);
    #endif
}


float QueryExpand::calcuMI( set<string> &sSrc, set<string> &sDes, set<string> &sSet )
{
	int total = sSet.size();
	int x1 = sSrc.size(), y1 = sDes.size(); //document that x occurres
	int x0 = total-x1, y0 = total-y1; //document num that x doesn't occurres
		
	if( x1 == 0 || x0 == 0 || y1 ==0 || y0 == 0 )
	{
		return 0;
	}

  	float pX0 = (float)x0/(float)total;
  	float pX1 = (float)x1/(float)total;
  	float pY0 = (float)y0/(float)total;
  	float pY1 = (float)y1/(float)total;
	//cout << "total:" << total << " x1:" << x1 << " x0:" << x0 << " y1:" << y1 << " y0:" << y0 << endl;	

	//get the co-occurence doc num
	//cout <<"\n" << "x1:" <<sSrc.size() << " y1:" << sDes.size() << endl;
	int numXY11 = 0, numXY10 = 0, numXY01 = 0, numXY00 = 0;
	set<string>::const_iterator sIter, sIter_end = sSrc.end(), sDesIter_end = sDes.end();
	for( sIter = sSrc.begin(); sIter != sIter_end; ++sIter )
	{
		if( sDes.find(*sIter) != sDesIter_end )
		{	
			numXY11 += 1; //doc num that x and y cooccurre
		}
	}

	numXY10 = x1 - numXY11;	//doc num that x occure but y doesn't
	numXY01 = y1 - numXY11;	//doc num that y occures but x doesn't 
	numXY00 = total - numXY11 - numXY10 - numXY01; //doc num that neither x nor y occurres
	//cout << "xy11:" << numXY11 << " 10:" << numXY10 << " 01:" << numXY01 << " 00:" << numXY00 << endl;	

	float pXY11 = (float)numXY11/(float)total;
	float pXY10 = (float)numXY10/(float)total;
	float pXY01 = (float)numXY01/(float)total;
	float pXY00 = (float)numXY00/(float)total;
	//cout << " 11: " <<pX1 << " " << pY1 << " " << pXY11 << " " << pXY11/(pX1*pY1) << " " << log(pXY11/(pX1*pY1)) <<" " << pXY11*log(pXY11/(pX1*pY1)) << endl;
	//cout << " 10: " <<pX1 << " " << pY0 << " " << pXY10 << " " << pXY10/(pX1*pY0) << " " << log(pXY10/(pX1*pY0)) <<" " << pXY10*log(pXY10/(pX1*pY0)) << endl;
	//cout << " 01: " <<pX0 << " " << pY1 << " " << pXY01 << " " << pXY01/(pX0*pY1) << " " << log(pXY01/(pX0*pY1)) <<" " << pXY01*log(pXY01/(pX0*pY1)) << endl;
	//cout << " 00: " <<pX0 << " " << pY0 << " " << pXY00 << " " << pXY00/(pX0*pY0) << " " << log(pXY00/(pX0*pY0)) <<" " << pXY00*log(pXY00/(pX0*pY0)) << endl;

	float m00 = 0, m01 = 0, m10 = 0, m11 = 0;
    if( pXY00 != 0 )
    {
    	m00 = pXY00*log( pXY00/(pX0*pY0) );
   	}

	if( pXY01 != 0 )
    {
    	m01 = pXY01*log( pXY01/(pX0*pY1) );
   	}

	if( pXY10 != 0 )
    {
    	m10 = pXY10*log( pXY10/(pX1*pY0) );
    }

	if( pXY11 != 0 )
    {
    	m11 = pXY11*log( pXY11/(pX1*pY1) );
    }

	//cout << "m11:" << m11 << " m01:" << m01 << " m10:" << m10 << " m00:" << m00 << endl;
	//cout << "result:" << m00+m10+m01+m11 << endl;
	return m00+m10+m01+m11;	
}


//calculate the mi between terms and query terms. result is quer term-term-score
void QueryExpand::calcuTermScore( map<string, int> &mQueryTerm, 
	map<string, set<string> > &mSetTerm, int L, float beta, 
	map< string, map<string, float> > &mmScore )
{
	//create document list from mSetTerm
	set<string> sDoc;
	map<string, set<string> >::iterator mTermIter;
	set<string>::iterator sDocIter;
	for( mTermIter = mSetTerm.begin(); mTermIter != mSetTerm.end(); ++mTermIter )
	{	
		for( sDocIter = mTermIter->second.begin(); sDocIter != mTermIter->second.end(); ++sDocIter )
		{
			sDoc.insert(*sDocIter);
		}
	}

	//calculate mi between term with each query term
	map<string, int>::iterator mQueryIter;
	for( mQueryIter = mQueryTerm.begin(); mQueryIter != mQueryTerm.end(); ++mQueryIter ) //browse each query term
	{
		string queryTerm = mQueryIter->first;
		int qtf = mQueryIter->second; // query term frequency
		map<string, set<string> >::iterator mSetTermIter, mQueryTermIter;
		if( (mQueryTermIter = mSetTerm.find(queryTerm) ) != mSetTerm.end() )		
		{
			multimap< float, string, greater<float> > mTermScore; //score-term
			for( mSetTermIter = mSetTerm.begin(); mSetTermIter != mSetTerm.end(); ++mSetTermIter )	//browse each candidate term
			{
				string setTerm = mSetTermIter->first;
				float score = 0.0;
				if( setTerm.compare(queryTerm) != 0 ) //don't consider the similarity of the query term itself
				{
					//cout << "queryTerm:" << queryTerm << " setTerm:" << setTerm << " ";
					float betMi = calcuMI( mQueryTermIter->second, mSetTermIter->second, sDoc );
					float selfMi = calcuMI( mQueryTermIter->second, mQueryTermIter->second, sDoc );
					//cout << mQueryTermIter->first << "-" << mSetTermIter->first << " betMi:" << betMi << " selfMi:" << selfMi << endl;
					score = beta * (float)qtf * betMi / selfMi;
	
					if( score != score )
					{
						score = 0;
					}
					//if( score != score )
					//	cout << "qtf:" << qtf << " betMI:" << betMi << " selfMi:" << selfMi << endl;					
					//insert result
				}
				else
				{
					score = 1.0*qtf;
				}
				mTermScore.insert( multimap< float, string, greater<float> >::value_type( score, setTerm ) );

			}

			map<string, float> mSetTermScore;
			multimap< float, string, greater<float> >::iterator mulIter;
			for( mulIter = mTermScore.begin(); mulIter != mTermScore.end() && mSetTermScore.size() < L; ++mulIter )
			{
				mSetTermScore.insert( map<string, float>::value_type( mulIter->second, mulIter->first ) );
			}
			
			mmScore.insert( map< string, map<string, float> >::value_type( queryTerm, mSetTermScore ) );
		}
       else{
            ;
            //cout<<" cannot find term "<<queryTerm<<endl;
        }
		
	}
	
		
	/*	
	map< string, map<string, float> >::iterator mmIter;
	for( mmIter = mmScore.begin(); mmIter != mmScore.end(); ++mmIter )
	{
		cout << "query:" << mmIter->first << endl;
		map<string, float>::iterator mIter;
		for( mIter = mmIter->second.begin(); mIter != mmIter->second.end(); ++mIter )
			cout << mIter->first << " " << mIter->second << endl;
		cout << endl;
	}
	*/
	
}


//calculate the score of each term to the whole query
void QueryExpand::calcuQueryScore( map<string, int> &mQueryTerm, 
	map< string, map<string, float> > &mmScore, int K, 
	map<string, float> &mTermScore )
{
	//calculate |Q|
	int querySize = 0;
	map<string, int>::iterator mQueryTermIter;
	for( mQueryTermIter = mQueryTerm.begin(); 
		mQueryTermIter != mQueryTerm.end(); ++mQueryTermIter )
	{
		querySize += mQueryTermIter->second;
	}
	
	map<string, float> mTmpScore;
	map<string, float>::iterator mTmpScoreIter;
	map< string, map<string, float> >::iterator mmScoreIter;
	for( mmScoreIter = mmScore.begin(); mmScoreIter != mmScore.end(); ++mmScoreIter )
	{
		map<string, float>::iterator mTermScoreIter;
		for( mTermScoreIter = mmScoreIter->second.begin(); 
			mTermScoreIter != mmScoreIter->second.end(); ++mTermScoreIter )
		{
			string term = mTermScoreIter->first;
			float score = mTermScoreIter->second;
			if( (mTmpScoreIter = mTmpScore.find(term)) != mTmpScore.end() )	 
			{
				//existing term
				mTmpScoreIter->second += score;
			}
			else
			{
				//sum score
				mTmpScore.insert( map<string, float>::value_type(term, score) );	
			}
		}
	}

	//cout << "querySize:" << querySize<< endl;
	//cout << "delSize:" << sDel.size() << endl;
	multimap<float, string, greater<float> > mulScore;
	for( mTmpScoreIter = mTmpScore.begin(); 
	mTmpScoreIter != mTmpScore.end(); ++mTmpScoreIter )	//insert terms (can be the query term)
	{
		mulScore.insert( multimap<float, string, greater<float> >::value_type( 
		mTmpScoreIter->second/(float)querySize, mTmpScoreIter->first ) );	//sumscore/|Q|
	}

	//cout << "mulScore:" << mulScore.size() << endl;	

	multimap<float, string, greater<float> >::iterator mulIter;
	for( mulIter = mulScore.begin(); mulIter != mulScore.end() && mTermScore.size() < K; ++mulIter )
	{
		bool del = false;

		//delete the term if it contains non-letter pounction
		int size = mulIter->second.size();
		for( int i = 0; i < size; ++i )
		{
			if( !( ( mulIter->second[i] >= 'a' && mulIter->second[i] <= 'z' ) 
				|| ( mulIter->second[i] >= 'A' && mulIter->second[i] <= 'Z' ) ) )
			{
				del = true;
			}	
		}

		if( !del )
		{
			mTermScore.insert( map<string, float>::value_type(mulIter->second, mulIter->first) );	
		}
	}

	/*
	cout << "addTermSize:" << mTermScore.size() << endl;
	//print
	map<string, float>::iterator mIter;
	for( mIter = mTermScore.begin(); mIter != mTermScore.end(); ++mIter )
	{
		cout << mIter->first << " " << mIter->second << endl;
	}
	*/
}


void QueryExpand::extractTerms(map< int, map<int, set<int> > > &mQueryDoc, 
	vector<string> &vIndex, map<int, map<string, int> > &mmQuery,
	int L, int K, float beta, map< int, map<string, float> > &mmTermScore)
{
	for( map< int, map<int, set<int> > >::iterator mQueryIter = mQueryDoc.begin(); 
		mQueryIter != mQueryDoc.end(); ++mQueryIter )		
	{
		/* 
		sTerm: container for all alphabeta terms in mQueryDoc of ONE query.
		The format is:
		{term:[indexId-docId]}
			
		indexId-docId indicates where is the term.
		A term may have multiple indexId-docIds.
		*/
		map<string, set<string> > sTerm;

		extractAllTerms( mQueryIter->second, vIndex, sTerm );

		#if VERBOSE
		cout << "query:" << mQueryIter->first << " size:" << sTerm.size() << endl;
		#endif
		map<int, map<string, int> >::iterator mmQueryIter;
		if( (mmQueryIter = mmQuery.find(mQueryIter->first)) != mmQuery.end() )
		{
			map< string, map<string, float> > mTermScore;
			calcuTermScore( mmQueryIter->second, sTerm, L, beta, mTermScore );
			map<string, float> mAddTerm;
			calcuQueryScore( mmQueryIter->second, mTermScore, K, mAddTerm );
			mmTermScore.insert( map< int, map<string, float> >::value_type( mQueryIter->first, mAddTerm ) );
		}
		
	}
}


//write the result of query with weight of each term
void QueryExpand::writeQuery( map<int, map<string, int> > &mmQuery,
	map< int, map<string, float> > &mmTermScore, 
	indri::api::Parameters& params, const string &desFile )
{
	indri::api::Parameters queries = params["query"];
	for( size_t i = 0; i < queries.size(); i++)
	{
		int queryId = atoi(((string)queries[i]["number"]).c_str());
		stringstream query;

		query.str("");
		query << "#weight( ";
		map< int, map<string, float> >::iterator mmTermScoreIter;
		if( ( mmTermScoreIter = mmTermScore.find(queryId) ) != mmTermScore.end() )
		{
			map<string, float>::iterator mTermScoreIter;
			for( mTermScoreIter = mmTermScoreIter->second.begin(); 
				mTermScoreIter != mmTermScoreIter->second.end(); ++mTermScoreIter )
			{
				query << mTermScoreIter->second << " " << mTermScoreIter->first << " ";
			}
			 //query << mTermScoreIter->second << " " << mTermScoreIter->first << " ";
		}
		query << ")";

		queries[i].set("text", query.str());

		//cout << (string)queries[i]["number"] << " " << (string)queries[i]["text"] << endl;
	}

	params.writeFile(desFile);
}


/*
M - number of top documents in initial results
R - factor that used in extracting random documents, we will extract R*M randomly select documents
L - top similar terms
K - number of expansion terms
beta - scaling parameter
*/
void QueryExpand::expand( int M=20, int R=29, int L=1000, int K=20, float beta=1.7 )
{                           
    string index = get_index_path(); //index file with on index path on each line
    string queryFile = get_origin_qf(); //original query file
    string desFile = get_expanded_qf();	//expanded query file (output)
    string resultFile = get_result_file_path(); //initial results file path

    /*
	params: The parameters in query file.
    */
	indri::api::Parameters params;

    /* 
	mmQuery: container for the query. 
	The format is:
	{QueryID:{query_term:numbers of occurance}}
		
	e.g.
	q1 cat 1 dog 2
	q2 book 3 store 2
    ...
	*/ 
	map<int, map<string, int> > mmQuery;

	/*
	QIDS: all query IDs.
	*/
	set<int> QIDs;

	/*
	vIndex: vector that contains (distributed) index paths.
	*/
	vector<string> vIndex;

	/* 
	mQueryDoc: container for the results. Used for store the internal document 
	IDs in the results.
	The format is:
	{QueryID:{indexID:[internal document IDs]}}
		
	For each queryID, we first iterate all (distributed) index.
	Then for each (distributed) index we list the unique internal document IDs.
	*/
	map< int, map<int, set<int> > > mQueryDoc;


	/*
	
	*/
	map< int, map<string, float> > mmTermScore;


	if (!readQuery(mmQuery, params, QIDs))
	{
		cout << "Error when read query file:" << queryFile << endl;
		exit(1);
	}

	if (!readIndexPaths(vIndex))
	{
		cout << "Error when read index file:" << index << endl;
		exit(1);		
	}

	if(!readResultsFile(M, QIDs, vIndex, mQueryDoc))
	{
		cout << "Error when read results file:" << resultFile << endl;
		exit(1);	
	}

    //debug purpose: print queries:
    //map<int, map<string, int> >::iterator it1;
    //map<string, int>::iterator it2;
    //for(it1=mmQuery.begin(); it1!=mmQuery.end(); ++it1){
    //    cout<<"query: "<<it1->first<<endl;
    //    for(it2=it1->second.begin();it2!=it1->second.end(); ++it2){
    //        cout<<"\t "<<it2->first<<" : "<<it2->second<<endl;       
    //    }
    //
    //}
    //cout<< "\n\n\n\n"<<endl;

	// Randomly select R*M documents from indexes and append to mQueryDoc.
	randomSelDoc( vIndex, R*M, mQueryDoc );

	extractTerms(mQueryDoc, vIndex, mmQuery, L, K, beta, mmTermScore);

    //ofstream outFile(desFile.c_str()); 
    writeQuery( mmQuery, mmTermScore, params, desFile );
	//outFile.close();
}


