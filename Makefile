OBJS = main.o file.o queryExpand.o 
CC = g++
CFLAGS = -c
PROJECT = trec_query_expansion

prefix =  /infolab/node6/zwei/home/
indriPath = /infolab/node6/zwei/work/trec/tool/indri-combine
exec_prefix = ${prefix}
libdir = ${exec_prefix}/lib
includedir = ${prefix}/include
INCPATH=-I$(includedir)
LIBPATH=-L$(libdir) -L$(indriPath)/obj -L$(indriPath)/contrib/lemur/obj -L$(indriPath)/contrib/xpdf/obj -L$(indriPath)/contrib/antlr/obj

CXXFLAGS=-DPACKAGE_NAME=\"Indri\" -DPACKAGE_TARNAME=\"indri\" -DPACKAGE_VERSION=\"2.9\" -DPACKAGE_STRING=\"Indri\ 2.9\" -DPACKAGE_BUGREPORT=\"indri-info@ciir.cs.umass.edu\" -DYYTEXT_POINTER=1 -DINDRI_STANDALONE=1 -DHAVE_LIBM=1 -DHAVE_LIBPTHREAD=1 -DHAVE_LIBZ=1 -DHAVE_NAMESPACES= -DISNAN_IN_NAMESPACE_STD= -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -D_FILE_OFFSET_BITS=64 -DHAVE_FSEEKO=1 -DHAVE_BITS_ATOMICITY_H=1 -DP_NEEDS_GNU_CXX_NAMESPACE=1 -DHAVE_MKSTEMP=1 -DNDEBUG=1  -g -O3 $(INCPATH) $(LIBPATH)
CPPLDFLAGS  =  -lindri -lz -lpthread -lm


##$(PROJECT) : $(OBJS)
	##$(CC) $(OBJS) -o $(PROJECT)

$(PROJECT) : $(OBJS)
	$(CC) $(CXXFLAGS) $(OBJS) -o $(PROJECT) $(CPPLDFLAGS) $(LIBPATH) $(CPPLDFLAGS)

#processQuery.o : queryProcess.cpp file.h
#	$(CC) $(CFLAGS) queryProcess.cpp
 
#trainParam.o : trainParam.cpp file.h
#	$(CC) $(CFLAGS) trainParam.cpp

queryExpand.o : queryExpand.cpp file.h
	$(CC) $(CXXFLAGS) $(CFLAGS) queryExpand.cpp 

#mqAnalyze.o : mqAnalyze.cpp file.h
#	$(CC) $(CXXFLAGS) $(CFLAGS) mqAnalyze.cpp

file.o : file.h
	$(CC) $(CFLAGS) file.cpp

.PHONY : clean
clean : 
	rm trec_query_expansion main.o file.o queryExpand.o 

