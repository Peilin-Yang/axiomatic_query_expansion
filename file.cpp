#include "file.h"
#include <stdio.h> 
#include <iostream> 
#include <fstream> 
#include <stdlib.h>

  bool File::read(string & fileContent) {
    //open the file
    fileContent = "";

    FILE * pFile;
    pFile = fopen(m_file.c_str(), "rb");
    if (pFile == NULL) {
      cerr << "can't open file:" << m_file << endl;
      return false;
    }

    //get the length
    fseek(pFile, 0, SEEK_END);
    long size = ftell(pFile);
    if (size <= 0)
      return true;

    //read file
    fseek(pFile, 0, SEEK_SET);
    char * buf = (char * ) malloc((size + 1) * sizeof(char));
    buf[size] = '\0';
    if (fread(buf, size, 1, pFile) <= 0) {
      free(buf);
      fclose(pFile);
      return false;
    }

    fileContent = buf;
    free(buf);
    fclose(pFile);
    return true;
  }

bool File::read(vector < string > & vFile) {
  ifstream inFile(m_file.c_str());
  if (!inFile) {
    cerr << "can't open:" << m_file << endl;
    return false;
  }

  string textline;
  while (getline(inFile, textline)) {
    vFile.push_back(textline);
  }

  inFile.close();
  return true;

}

bool File::read(set < string > & sFile) {
  ifstream inFile(m_file.c_str());
  if (!inFile) {
    cerr << "can't open:" << m_file << endl;
    return false;
  }

  string textline;
  while (getline(inFile, textline)) {
    sFile.insert(textline);
  }

  inFile.close();
  return true;

}

bool File::write(const string & fileContent) {
  ofstream outFile(m_file.c_str());
  if (!outFile) {
    cerr << "can't open:" << m_file << endl;
    return false;
  }

  outFile << fileContent;

  outFile.close();
  return true;
}

bool File::write(const set < string > & sContent) {
  ofstream outFile(m_file.c_str());
  if (!outFile) {
    cerr << "can't open:" << m_file << endl;
    return false;
  }

  set < string > ::const_iterator sIter, sIter_end = sContent.end();
  for (sIter = sContent.begin(); sIter != sIter_end; ++sIter)
    outFile << * sIter << endl;;

  outFile.close();
  return true;
}
