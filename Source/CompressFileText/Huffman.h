#pragma once
#include<stdio.h>
struct NODE
{
	int _freq;
	char _text;
	NODE* pLeft;
	NODE* pRight;
};
//Lưu dữ liệu trong file
struct HuffData
{
	char* s;
	unsigned long long* wei;
};


struct HuffMap
{
	int** BitArray;
	char* charater;
};

struct HuffmanTree
{
	int size;
	//Tạo ra một mảng con trỏ node , mỗi phần tử chứa một cây root
	NODE** Array;

};



HuffData ReadFileBin(FILE* p, unsigned long long freq[256], int& size);

HuffData ReadFileExe(FILE* p, char* filename, unsigned long long freq[256]);



NODE* newNode(char data, int freq);

HuffmanTree* InitHuffTree(int size);

void SwapNode(NODE*& a, NODE*& b);

void MinHeapify(HuffmanTree* hufftree, int index);

NODE* getNodeMin(HuffmanTree* hufftree);

void insertHuffTree(HuffmanTree* hufftree, NODE* newnode);



void BuildMinHeap(HuffmanTree* hufftree);

bool isMinSize(HuffmanTree* hufftree);


HuffmanTree* CreateHeapHuffman(HuffData map, int size);

NODE* builfHuffmanTree(HuffData map, int size);