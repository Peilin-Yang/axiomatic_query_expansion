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
#include "indri/Parameters.hpp"

#include "queryExpand.h"

using namespace std;

void usage() {
  cout << "***Usage:***\n"
  << "axiom_query_expansion "
  << "-oqf=<original_query_file_path> "
  << "-output=<output_path_of_the_expanded_query_file> "
  << "-index_list=<index_list_file> "
  << "-orf=<initial_result_file_path> "
  << "-M=<number of top results> "
  << "-R=<R*M documents will be randomly selected> "
  << "-L=<number of top terms> "
  << "-K=<number of finally chosen expansion terms> "
  << "-beta=<scaling factor> " << endl;

}

int main(int argc, char *argv[])
{
  try {
    indri::api::Parameters& param = indri::api::Parameters::instance();
    param.loadCommandLine(argc, argv);

    if ( !param.exists( "oqf" )) {
      usage();
      LEMUR_THROW( LEMUR_MISSING_PARAMETER_ERROR, "Must Specify the Original TREC Query File.");
    }
    if ( !param.exists( "output" )) {
      usage();
      LEMUR_THROW( LEMUR_MISSING_PARAMETER_ERROR, "Must Specify the output file path.");
    }
    if ( !param.exists( "index_list" )) {
      usage();
      LEMUR_THROW( LEMUR_MISSING_PARAMETER_ERROR, "Must Specify the Index List File.");
    }
    if ( !param.exists( "orf" )) {
      usage();
      LEMUR_THROW( LEMUR_MISSING_PARAMETER_ERROR, "Must Specify the Original TREC Result File.");
    }

    string oqf = param.get( "oqf", "");
    string output = param.get( "output", "");
    string index_list = param.get( "index_list", "");
    string orf = param.get( "orf", "");
    QueryExpand expand(oqf.c_str(), output.c_str(), index_list.c_str(), orf.c_str());
    int M = param.get( "M", 20 );
    int R = param.get( "R", 29 );
    int L = param.get( "L", 1000 );
    int K = param.get( "K", 20 );
    float beta = param.get( "beta", 1.7 );

    //cout << "M:" << M << " R:" << R << " L:" << L << " K:" << K << " beta:" << beta << endl;

    expand.expand( M, R, L, K, beta );
  } catch(lemur::api::Exception& e) {
    std::cerr << e.what() << std::endl;
    exit(-1);
  } catch ( ... ) {
    std::cout << "Caught unhandled exception" << std::endl;
    return -1;
  }
  return 0;
}

