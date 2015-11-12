#include <string>
#include <iostream>
#include <sstream>
#include <math.h>
#include "indri/Index.hpp"
#include "indri/IndexEnvironment.hpp"
#include "indri/Index.hpp"
#include "indri/QueryEnvironment.hpp"
#include "indri/CompressedCollection.hpp"
#include "indri/LocalQueryServer.hpp"
#include "indri/DocListIterator.hpp"
#include "indri/Repository.hpp"
#include "indri/CompressedCollection.hpp"
#include "indri/LocalQueryServer.hpp"
#include "indri/ScopedLock.hpp"
#include "indri/QueryEnvironment.hpp"

#include "file.h"
//#include "queryProcess.h"
//#include "trainParam.h"
#include "queryExpand.h"
//#include "mqAnalyze.h"

using namespace std;


int main(int argc, char *argv[])
{
	if (argc != 10){
		cout << "Usage: \n./trec_query_expansion original_query_file_path "
		<< "expanded_query_file_path index_list_file initial_result_file_path "
		<< "M(number of top results) R(R*M documents will be randomly selected) "
		<< "L(number of top terms) K(number of finally chosen expansion terms) "
		<< "beta(scaling factor)" << endl;
		exit(1);
	}

	QueryExpand expand(argv[1], argv[2], argv[3], argv[4]);
	int M = atoi(argv[5]);
	int R = atoi(argv[6]); 
	int L = atoi(argv[7]);
	int K = atoi(argv[8]);
	float beta = atof(argv[9]);

	cout << "M:" << M << " R:" << R << " L:" << L << " K:" << K 
	<< " beta:" << beta << endl;

	expand.expand( M, R, L, K, beta );
	return 0;
}

