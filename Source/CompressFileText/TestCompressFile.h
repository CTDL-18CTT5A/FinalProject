
#pragma once
#include<iostream>
#include<string>
#include"Huffman.h"
using namespace std;




bool isLeaf(NODE* root);

string getType(char* s);

void ArrayOutput(HuffMap& map, int index, int a[], int n);


void CompressFile(HuffMap& map, NODE* root, int arr[], int top , int &index);


void HuffmanCompress(FILE* fileInput, FILE* fileOut, HuffMap& map, HuffData  data, int size , char* nameFile , unsigned long long freq[256]);




//Decode


void ReadHeaderFile(FILE* fileIN, HuffData& data, int& sl);


void WriteHeaderFile(FILE* Output, HuffData data, unsigned long long freq[256]);



void Decode(FILE* in, FILE* out);


//void EncodeFile();

void EncodeFile(string linkFile);


//void EncodeMultiFile(char* filename);

//void ExportFile();

void ExportFile(string filename);


void DecodeFolder(FILE* in, FILE* out, int vtLE);


//Suport

char* ToCharArray(string name);


