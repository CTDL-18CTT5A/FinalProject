#include<thread>
#include<iostream>
#include<vector>
#include<sstream>
#include<fstream>
#include<bitset>
#include<map>
#include"Huffman.h"
#include <iterator>
#include <algorithm>
#include <atomic>
#include <future>
#include <chrono>
#define MAX_BYTE 1024*1024
using namespace std::chrono_literals;
using namespace std;
NODE* newNode(char data, int freq)
{
	NODE* temp = new NODE;
	temp->pLeft = NULL;
	temp->pRight = NULL;
	temp->_text = data;
	temp->_freq = freq;
	return temp;

}




bool IsExist(char s, vector<char> str)
{
	for (vector<char>::iterator iter_name = str.begin(); iter_name != str.end(); iter_name++)
	{
		if (s == *iter_name)
			return true;
	}
	return false;

}

//Đếm số lương kí tự khác nhau trong file
long long NumberOfCharFile(FILE* p)
{
	if (p == NULL)
		return 0;
	int count = 0;
	fseek(p, 0, SEEK_END);
	count = ftell(p);
	return count;
}



HuffData ReadFileBin(FILE* p , unsigned long long freq[256] , int &sl)
{
	if (p == NULL)
	{
		cout << "Khong the doc file";
		exit(0);
	}
	HuffData haf;
	long long n = 500;
	rewind(p);
	haf.s = new char[n];
	haf.wei = new unsigned long long[n];

	int row = 0;




	int size = NumberOfCharFile(p);

	cout << "So byte : " << size << endl;

	rewind(p);


	//unsigned long long freq[256];

	//Gán toàn bộ phần tử trong bảng tần số = 0
	memset(freq, 0, 256 * sizeof(unsigned long long));

	//Đọc toàn bộ byte trong file nén
	unsigned char* Memory = new unsigned char[MAX_BYTE];

	fseek(p, 0, SEEK_END);

	unsigned long long len = ftell(p);

	fseek(p, 0, SEEK_SET);

	while (len >= MAX_BYTE) 
	{
		fread(Memory, MAX_BYTE, 1, p);

		for (unsigned int i = 0; i < MAX_BYTE; ++i)
			freq[Memory[i]]++;

		len -= MAX_BYTE;
	}

	if (len > 0) 
	{
		fread(Memory, len, 1, p);
		for (unsigned int i = 0; i < len; ++i)
			freq[Memory[i]]++;
	}
	//Gán bảng tần số vào cây huffman

	int count = 0;
	for (int i = 0; i < 256; i++)
	{
		if (freq[i] != 0)
		{
			haf.wei[count] = freq[i];
			haf.s[count] = i;
			//cout << haf.wei[count] << " va " << haf.s[count] << endl;
			count++;
		}
	}
	sl = count;
	
	haf.s[count] = '\0';

	delete[] Memory;


	return haf;

	
	

}


//Read data file exe
vector<unsigned char> readfile(const char* filename)
{
	ifstream infile(filename, ios::binary);
	vector<unsigned char> result;
	copy(istream_iterator<unsigned char>(infile),
		istream_iterator<unsigned char>(),
		back_inserter(result));
	return result;
}


HuffData ReadFileExe(FILE* p , char * filename , unsigned long long freq[256])
{
	if (p == NULL)
	{
		cout << "Khong the doc file";
		exit(0);
	}
	HuffData haf;
	long long n = 500;
	rewind(p);
	haf.s = new char[n];
	haf.wei = new unsigned long long[n];



	int size = NumberOfCharFile(p);

	rewind(p);
	//vector<char> Mem = readfile(filename);

	int row = 0;
	rewind(p);


	//unsigned long long freq[256];

	//Gán toàn bộ phần tử trong bảng tần số = 0
	memset(freq, 0, 256 * sizeof(unsigned long long));


	vector<unsigned char> Memory = readfile(filename);


	for (int i = 0; i < Memory.size(); i++)
	{
		freq[Memory[i]]++;
	}

	//Gán bảng tần số vào cây huffman

	int count = 0;
	for (int i = 0; i < 256; i++)
	{

		if (freq[i] != 0)
		{
			haf.wei[count] = freq[i];
			haf.s[count] = i;
			count++;
		}
	}
	haf.s[count] = '\0';

	return haf;
}

HuffmanTree* InitHuffTree(int size)
{
	HuffmanTree* hufftree = new HuffmanTree;

	hufftree->size = 0;

	hufftree->Array = (NODE**)malloc(size * sizeof(NODE*));

	return hufftree;

}

void SwapNode(NODE*& a, NODE*& b)
{
	NODE* temp = a;
	a = b;
	b = temp;
}

void MinHeapify(HuffmanTree* hufftree, int index)
{
	int min = index;
	int left = 2 * index + 1;
	int right = 2 * index + 2;

	if (left < hufftree->size && hufftree->Array[left]->_freq < hufftree->Array[min]->_freq)
	{
		min = left;
	}
	if (right < hufftree->size && hufftree->Array[right]->_freq < hufftree->Array[min]->_freq)
	{
		min = right;
	}
	if (min != index)
	{
		SwapNode(hufftree->Array[min], hufftree->Array[index]);
		MinHeapify(hufftree, min);
	}
}

NODE* getNodeMin(HuffmanTree* hufftree)
{
	NODE* node = hufftree->Array[0];
	//Lấy ra thì phải đưa cuối lên đầu và trừ bớt size(giống heap sort)
	hufftree->Array[0] = hufftree->Array[hufftree->size - 1];

	--hufftree->size;
	MinHeapify(hufftree, 0);

	return node;
}

void insertHuffTree(HuffmanTree* hufftree, NODE* newnode)
{
	++hufftree->size;
	//Vị trí cuối cùng để thêm vào phần tử mới
	int posInsert = hufftree->size - 1;

	//Nếu node insert nhỏ hơn root thì phải chuyển lên đầu để tiếp tục tạo root rồi chèn vào
	 //Nó sẽ so với root mà chưa cây con đó(tại vì khi thêm vào chỉ cần check tại nhanh đó thôi)
	//Đẩu thằng root nhỏ nhất luôn lên vị trí pos = 0;
	while (posInsert && newnode->_freq < hufftree->Array[(posInsert - 1) / 2]->_freq)
	{
		hufftree->Array[posInsert] = hufftree->Array[(posInsert - 1) / 2];
		posInsert = (posInsert - 1) / 2;
	}

	hufftree->Array[posInsert] = newnode;

}



void BuildMinHeap(HuffmanTree* hufftree)
{
	int n = hufftree->size - 1;
	for (int i = (n - 1) / 2; i >= 0; i--)
	{
		MinHeapify(hufftree, i);
	}
}

bool isMinSize(HuffmanTree* hufftree)
{
	return (hufftree->size == 1);
}


HuffmanTree* CreateHeapHuffman(HuffData map, int size)
{

	HuffmanTree* hufftree = InitHuffTree(size);
	hufftree->size = size;


	for (int i = 0; i < size; i++)
	{
		hufftree->Array[i] = newNode(map.s[i], map.wei[i]);
	}

	BuildMinHeap(hufftree);

	return hufftree;
}




NODE* builfHuffmanTree(HuffData map, int size)
{

	NODE* left, * right, * root;
	HuffmanTree* hufftree = CreateHeapHuffman(map, size);


	while (!isMinSize(hufftree))
	{
		left = getNodeMin(hufftree);
		right = getNodeMin(hufftree);

	

		root = newNode(' ', left->_freq + right->_freq);

		root->pLeft = left;
		root->pRight = right;

		insertHuffTree(hufftree, root);
	}

	//Sau khi đã liên kết lại hết với nhau thì chỉ cần lấy ra root

	return getNodeMin(hufftree);

}