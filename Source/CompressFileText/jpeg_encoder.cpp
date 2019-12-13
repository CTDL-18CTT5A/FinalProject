#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <memory.h>
#include <math.h>

#include "jpeg_encoder.h"

namespace {

// Bảng lượng tử áp dụng cho DC component (Độ sáng)
const unsigned char Luminance_Quantization_Table[64] = 
{
	16,  11,  10,  16,  24,  40,  51,  61,
	12,  12,  14,  19,  26,  58,  60,  55,
	14,  13,  16,  24,  40,  57,  69,  56,
	14,  17,  22,  29,  51,  87,  80,  62,
	18,  22,  37,  56,  68, 109, 103,  77,
	24,  35,  55,  64,  81, 104, 113,  92,
	49,  64,  78,  87, 103, 121, 120, 101,
	72,  92,  95,  98, 112, 100, 103,  99
};

// Bảng lượng tử áp dụng cho AC component (Độ màu)
const unsigned char Chrominance_Quantization_Table[64] = 
{
	17,  18,  24,  47,  99,  99,  99,  99,
	18,  21,  26,  66,  99,  99,  99,  99,
	24,  26,  56,  99,  99,  99,  99,  99,
	47,  66,  99,  99,  99,  99,  99,  99,
	99,  99,  99,  99,  99,  99,  99,  99,
	99,  99,  99,  99,  99,  99,  99,  99,
	99,  99,  99,  99,  99,  99,  99,  99,
	99,  99,  99,  99,  99,  99,  99,  99
};

// Zigzag scan dùg trong chuyển đổi khối 8*8 thành vector<1x64> Dùng để sắp xếp các điểm ảnh theo chỉ số phần tử tin chắc nhất. Phần tử đầu tiên của mảng là phần tử có
// giá trị trung bình cho cả khối và nó mang giá trị quan trọng nhất
const char ZigZag[64] =
{ 
	0, 1, 5, 6,14,15,27,28,
	2, 4, 7,13,16,26,29,42,
	3, 8,12,17,25,30,41,43,
	9,11,18,24,31,40,44,53,
	10,19,23,32,39,45,52,54,
	20,22,33,38,46,51,55,60,
	21,34,37,47,50,56,59,61,
	35,36,48,49,57,58,62,63 
};     


/*
Các bảng tiêu chuẩn trong mã hóa hình ảnh theo định dạng JPEG.
Standard_DC_Luminance_NRCodes và Standard_DC_Luminance_Values, Standard_AC_Luminance_NRCodes và Standard_AC_Luminance_Values là 4 bảng dùng cho khôn gian màu Y
Standard_DC_Chrominance_NRCodes, Standard_DC_Chrominance_Values, Standard_AC_Chrominance_NRCodes, Standard_AC_Chrominance_Values 4 bảng dùng cho khôn gian màu Cb Cr

Mục đich của việc sử dụng các bảng mã này là dùng trong việc so khớp với giửa 2 bảng code length và value để ấy ra giá trị mã hóa từ bảng huffman
Mảng value chứa các giá trị từ 0-255 lưu trử các giá trị điểm ảnh đầu vào từ ảnh bmp.

Công việc chính của các mảng này là hổ trợ việc so khớp giửa các phần tử nhằm tạo ra 4 cái bảng đuọc mô tả ở phần ví dụ
*/

const char Standard_DC_Luminance_NRCodes[] = { 0, 0, 7, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 };
const unsigned char Standard_DC_Luminance_Values[] = { 4, 5, 3, 2, 6, 1, 0, 7, 8, 9, 10, 11 };

const char Standard_DC_Chrominance_NRCodes[] = { 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 };
const unsigned char Standard_DC_Chrominance_Values[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

const char Standard_AC_Luminance_NRCodes[] = { 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d };
const unsigned char Standard_AC_Luminance_Values[] = 
{
	0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
	0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
	0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
	0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
	0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
	0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
	0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
	0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
	0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
	0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
	0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
	0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
	0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
	0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
	0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
	0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
	0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
	0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
	0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
	0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
	0xf9, 0xfa
};

const char Standard_AC_Chrominance_NRCodes[] = { 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77 };
const unsigned char Standard_AC_Chrominance_Values[] =
{
	0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
	0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
	0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
	0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
	0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
	0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
	0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
	0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
	0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
	0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
	0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
	0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
	0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
	0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
	0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
	0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
	0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
	0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
	0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
	0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
	0xf9, 0xfa
};

}

// Phương thức khởi tạo mặc định không cần tham số. Nó sẽ khởi tạo tất cả các giá trị cần thiết cho quá trình mã hóa
JpegEncoder::JpegEncoder(): m_width(0), m_height(0), m_rgbBuffer(0)
{
	initHuffmanTables();
}

// Phương thức hũy
JpegEncoder::~JpegEncoder()
{
	clean();
}

// Giải phóng bộ nhớ. Renew lại các giá trị
void JpegEncoder::clean(void)
{
	if(m_rgbBuffer) delete[] m_rgbBuffer;
	m_rgbBuffer=0;

	m_width=0;
	m_height=0;
}

// Cấu trúc phần Header của ảnh JPEG
bool JpegEncoder::readFromBMP(const char* fileName)
{
	clean();

#pragma pack(push, 2)

	// Cấu trúc 1 file BMp tham khảo tại đây : https://en.wikipedia.org/wiki/BMP_file_format

	// Phần BITMAPFILEHEADER gồm 14 byte chứa các thông tin
	typedef struct {
			unsigned short	bfType; // Kí hiệu "BM"
			unsigned int	bfSize; // Kích thước file ảnh tính bằng byte
			unsigned short	bfReserved1;
			unsigned short	bfReserved2;
			unsigned int	bfOffBits;
	} BITMAPFILEHEADER;

	typedef struct {
			unsigned int	biSize; // Số byte trong DIB header
			int				biWidth; // Chiều rộng
			int				biHeight; // Chiều cao
			unsigned short	biPlanes;
			unsigned short	biBitCount; // Độ sâu màu
			unsigned int	biCompression;
			unsigned int	biSizeImage;
			int				biXPelsPerMeter;
			int				biYPelsPerMeter;
			unsigned int	biClrUsed;
			unsigned int	biClrImportant;
	} BITMAPINFOHEADER;

#pragma pack(pop)

	FILE* fp = fopen(fileName, "rb");
	if(fp==0) return false;

	bool successed=false;
	do
	{
		BITMAPFILEHEADER fileHeader;
		BITMAPINFOHEADER infoHeader;

		// Nếu ko đọc được BITMAPFILEHEADER thì dừng
		if(1 != fread(&fileHeader, sizeof(fileHeader), 1, fp)) break;

		// Nếu 2 kí tự đầu ko phải đánh dấu ảnh bitmap (BM) thì dừng
		if(fileHeader.bfType!=0x4D42) break;

		// Nếu ko đọc được BITMAPINFOHEADER thì dừng
		if(1 != fread(&infoHeader, sizeof(infoHeader), 1, fp)) break;

		// Nếu độ sâu ko phải 24 bít hoặc ảnh đã bị nén rồi thì dừng
		if(infoHeader.biBitCount!=24 || infoHeader.biCompression!=0) break;

		// Lấy chiều dài và chiều rộg của ảnh
		int width = infoHeader.biWidth;
		int height = infoHeader.biHeight < 0 ? (-infoHeader.biHeight) : infoHeader.biHeight;

		// Kiểm tra nó có phải độ dài là bội số của 8 hay không
		if((width&7)!=0 || (height&7)!=0) break;

		// Kích thước của toàn ảnh. Mỗi vị trí pixel sẽ có 3 giá trị độ sáng R G B
		int bmpSize = width*height*3;

		// Khai báo một cái mảng chứa toàn bộ các điểm ảnh của bit map theo tuyến tính
		unsigned char* buffer = new unsigned char[bmpSize];
		if(buffer==0) break;

		// Đưa con trỏ tới vị trí bắt đầu của ảnh
		fseek(fp, fileHeader.bfOffBits, SEEK_SET);

		if(infoHeader.biHeight>0)
		{
			// Đọc lần lượt width block (vector) với mỗi block có độ dài 3 byte tương ứng với giá trị RGB vào mảng buffer thứ height-1-i
			for(int i=0; i<height; i++)
			{
				if(width != fread(buffer+(height-1-i)*width*3, 3, width, fp)) 
				{
					delete[] buffer; buffer=0;
					break;
				}
			}
		}
		else
		{
			// Kiểm tra lại nếu cái vừa mới đọc có chiều dài ko khớp với khối ban đầu thì hủy nó
			if(width*height != fread(buffer, 3, width*height, fp))
			{
				delete[] buffer; buffer=0;
				break;
			}
		}

		// Cập nhật các thuộc tính cho class
		m_rgbBuffer = buffer;
		m_width = width;
		m_height = height;

		// Đánh dấu đọc thành công
		successed=true;

	}while(false);

	fclose(fp);fp=0;
	
	return successed;
}

// Đầu vào là tên File cần nén và chỉ số chất lượng khi nén. Chỉ số này cànng cao thì ảnh nén có dung lượng càng nhỏ
bool JpegEncoder::encodeToJPG(const char* fileName, int quality_scale)
{
	// Kiểm tra tính hợp lệ của giá trị
	if(m_rgbBuffer==0 || m_width==0 || m_height==0) return false;

	// Mở một tấm ảnh mới lên để ghi xuống
	FILE* fp = fopen(fileName, "wb");
	if(fp==0) return false;

	// Khởi tạo mảng Quality. Làm co giãn theo hệ số mảng Luminance và Chrominance theo tỉ lệ Quality
	initQualityTables(quality_scale);

	// Ghi Header của ảnh Jpeg xuống
	write_jpeg_header(fp);

	short prev_DC_Y = 0, prev_DC_Cb = 0, prev_DC_Cr = 0; // Mã hóa DC entropy dựa vào sự sai khác giữa vector hiện tại với vector trước nó nên ta phải lưu lại 
	int newByte=0, newBytePos=7;

	// Chia nhỏ hình ảnh theo từng block 8*8
	for(int yPos=0; yPos<m_height; yPos+=8)
	{
		for (int xPos=0; xPos<m_width; xPos+=8)
		{
			char yData[64], cbData[64], crData[64];

			// Mảnge zigzag chứa các giá trị sau khi lượng tử hóa
			short yQuant[64], cbQuant[64], crQuant[64];

			// Chuyển đổi không gian màu RGB sang Y Cb Cr
			convertColorSpace(xPos, yPos, yData, cbData, crData);

			// Mảng object outputBitString này chứa các giá trị sau khi so khớp giữa các bảng entropy huffman và ta sẽ ghi đối tượng này xuống ảnh JPEG
			BitString outputBitString[128];

			// Giá trị lưu lại độ dài của đối tượng outputBitString sau khi mã hóa DPCM trên DC và RLE trên ACs. Nó cũng được dùng làm kích thước để ghi xuống ảnh
			int bitStringCounts;

			// Biến đổi cos rời rac cho Y
			foword_FDC(yData, yQuant);
			doHuffmanEncoding(yQuant, prev_DC_Y, m_Y_DC_Huffman_Table, m_Y_AC_Huffman_Table, outputBitString, bitStringCounts); 
			write_bitstring(outputBitString, bitStringCounts, newByte, newBytePos, fp);

			// Biến đổi cos rời rac cho Cb
			foword_FDC(cbData, cbQuant);			
			doHuffmanEncoding(cbQuant, prev_DC_Cb, m_CbCr_DC_Huffman_Table, m_CbCr_AC_Huffman_Table, outputBitString, bitStringCounts);
			write_bitstring(outputBitString, bitStringCounts, newByte, newBytePos, fp);

			// Biến đổi cos rời rac cho Cr
			foword_FDC(crData, crQuant);			
			doHuffmanEncoding(crQuant, prev_DC_Cr, m_CbCr_DC_Huffman_Table, m_CbCr_AC_Huffman_Table, outputBitString, bitStringCounts);
			write_bitstring(outputBitString, bitStringCounts, newByte, newBytePos, fp);
		}
	}

	write_word(0xFFD9, fp); // Ghi kí tự kết thúc File
	
	fclose(fp);

	return true;
}

void JpegEncoder::initHuffmanTables(void)
{
	memset(&m_Y_DC_Huffman_Table, 0, sizeof(m_Y_DC_Huffman_Table)); // Khởi tạo mặc định bằng 0
	computeHuffmanTable(Standard_DC_Luminance_NRCodes, Standard_DC_Luminance_Values, m_Y_DC_Huffman_Table);

	memset(&m_Y_AC_Huffman_Table, 0, sizeof(m_Y_AC_Huffman_Table));
	computeHuffmanTable(Standard_AC_Luminance_NRCodes, Standard_AC_Luminance_Values, m_Y_AC_Huffman_Table);

	memset(&m_CbCr_DC_Huffman_Table, 0, sizeof(m_CbCr_DC_Huffman_Table));// Khởi tạo mặc định bằng 0
	computeHuffmanTable(Standard_DC_Chrominance_NRCodes, Standard_DC_Chrominance_Values, m_CbCr_DC_Huffman_Table);

	memset(&m_CbCr_AC_Huffman_Table, 0, sizeof(m_CbCr_AC_Huffman_Table));// Khởi tạo mặc định bằng 0
	computeHuffmanTable(Standard_AC_Chrominance_NRCodes, Standard_AC_Chrominance_Values, m_CbCr_AC_Huffman_Table);
}
// Đếm số lượng bít biểu diễn cho số nguyên value
JpegEncoder::BitString JpegEncoder::getBitCode(int value)
{
	BitString ret;
	int v = (value>0) ? value : -value;
	
	int length = 0;
	for(length=0; v; v>>=1) length++;

	ret.value = value>0 ? value : ((1<<length)+value-1);
	ret.length = length;

	return ret;
};

//-------------------------------------------------------------------------------
// Phương thức lượng tử hóa hình ảnh. Tham số truyền vào là độ nén của ảnh(tham số chất lượng này sẽ làm thay đôi 2 table Luminance và Chrominance để làm mất it hoặc nhiều dữ liệu hơn. Tăng giảm hiệu suất nén ảnh ). Số này càng lớn thì ảnh nén càng nhiều
// Lượng tử hóa để giảm số lượng bit biểu diễn 1 điểm ảnh
// Bước này là bước làm mất dữ liệu ảnh đầu vào lossy
// Mắt người nhạy cảm với tần số tháp(góc trên bên trái) và ít nhạy cảm với tần số cao(góc dưới bên phải). Vì thế ta cố tình gom nhóm các bít thấp về đầu mảng và cứ tăng dần tăng dần

// Thực chất bước này là kiểm tra tính hợp lệ của dữ liệu đồng thời chuyển đổi hệ màu từ RGB sang Y (Độ sáng), CbCr(Độ màu)

// Nếu ta không dùng cách này thì có thể đổi trược tiếp theo công thức sau:
/*
Y = 0.2568*R + 0.5*G + 0.098B
Cb = 128 - 0.148*R - 0.29*G + 0.439B
Cr = 128 + 0.439*R - 0.3677*G + 0.071*B
*/
void JpegEncoder::initQualityTables(int quality_scale)
{
	if(quality_scale<=0) quality_scale=1;
	if(quality_scale>=100) quality_scale=99;

	// Chia các hệ số tại mổi điểm ảnh cho hệ số tại vị trí tương ứng của mnagr Quantization
	// Fq(u)=[F(u)/Q(u)]

	for(int i=0; i<64; i++)
	{
		int temp = ((int)(Luminance_Quantization_Table[i] * quality_scale + 50) / 100);
		if (temp<=0) temp = 1;

		// Lấy 8 byte có ý nghĩa:
		/*
		ví dụ: 0 1 0 1 1 0 1 0 0 1 1 0 0 1 1 0
			   0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1
			  ---------------------------------
			   0 0 0 0 0 0 0 0 0 1 1 0 0 1 1 0
		*/

		// Ta nhóm các điểm có tần số thấp biểu trưng cho độ chói của ảnh (Y). Xắp xếp theo mảng zigzag

		// Nếu độ chói lớn hơn 255 thì độ chói gán bằng 255
		if (temp>0xFF) temp = 0xFF;
		m_YTable[ZigZag[i]] = (unsigned char)temp;

		// Lấy độ màu
		temp = ((int)(Chrominance_Quantization_Table[i] * quality_scale + 50) / 100);
		if (temp<=0) 	temp = 1;
		if (temp>0xFF) temp = 0xFF;
		m_CbCrTable[ZigZag[i]] = (unsigned char)temp;
	}
}

// Khởi tạo bảng mã hóa entropy dùng cho Y, Cb, Cr cả Luminance và Chrominance
void JpegEncoder::computeHuffmanTable(const char* nr_codes, const unsigned char* std_table, BitString* huffman_table)
{
	unsigned char pos_in_table = 0;
	unsigned short code_value = 0;

	for(int k = 1; k <= 16; k++)
	{
		for(int j = 1; j <= nr_codes[k-1]; j++)
		{
			huffman_table[std_table[pos_in_table]].value = code_value;
			huffman_table[std_table[pos_in_table]].length = k;
			pos_in_table++;
			code_value++;
		}
		code_value <<= 1;
	}  
}

void JpegEncoder::write_byte(unsigned char value, FILE* fp)
{
	write(&value, 1, fp);
}

void JpegEncoder::write_word(unsigned short value, FILE* fp)
{
	unsigned short _value = ((value>>8)&0xFF) | ((value&0xFF)<<8);
	write(&_value, 2, fp);
}

void JpegEncoder::write(const void* p, int byteSize, FILE* fp)
{
	fwrite(p, 1, byteSize, fp);
}

// https://www.slideshare.net/AishwaryaKM1/jpeg-image-compression-56894348
// DU là mảng Y, Cb, Cr hiện tại. prevDC là DU[0] của mảng trước
// HTDC là bảng lượng tử của Y đã khởi tạo từ trước từ 4 mảng trên đầu
// HTDC là bảng lượng tử của Cb Cr đã khởi tạo từ trước từ 4 mảng trên đầu

// Đầu ra là đối tượng outputBíttring và chiều dày mã hóa đuọc sau khi biến đổi DPCM trên DC và RLE trên ACs (bitStringCounts)
void JpegEncoder::doHuffmanEncoding(const short* DU, short& prevDC, const BitString* HTDC, const BitString* HTAC, BitString* outputBitString, int& bitStringCounts)
{
	BitString EOB = HTAC[0x00]; // Giá trị kết thúc chuổi RLE (0,0) = (ignore number bit 0, bit kế tiếp)
	BitString SIXTEEN_ZEROS = HTAC[0xF0];

	int index=0;

	// encode DC outputBitString[0][1]. Mỗi thằng DC sẽ đc mã hóa dựa trên thằng DC trước nó dcDiff = (int)(DU[0] - prevDC);
	// Nó sẽ đc lưu lại dưới dạng 2 kí tự đầu của outputBitString, kí tự đầu là giá trị của nó, kí tự tiếp theo là số lượng bit cần để biểu diễn giá trị đó
	int dcDiff = (int)(DU[0] - prevDC);
	prevDC = DU[0];

	// Nếu DC[0] của thằng hiện tại = 0 thì DC[hiện tại] = DC[trước nó]
	if (dcDiff == 0) {
		outputBitString[index++] = HTDC[0];
	}
	else
	{
		BitString bs = getBitCode(dcDiff);

		outputBitString[index++] = HTDC[bs.length];
		outputBitString[index++] = bs;
	}

	// encode ACs
	int endPos=63; //end0pos = vị trí th đầu tiên khác 0 từ cuối đếm lên
	while((endPos > 0) && (DU[endPos] == 0)) endPos--;

	for(int i=1; i<=endPos; )
	{
		int startPos = i;
		while((DU[i] == 0) && (i <= endPos)) i++;

		// Đếm số lượng bit 0 liên tục
		int zeroCounts = i - startPos;

		// Nếu số lượng bit 0 liên tục mà dài quá 16 thì phải cắt ra và biểu diễn mỗi làn 16 bit do ko đủ số lượng mã hóa của bảng huffman. Chỉ biểu diễn đc 16 bit
		if (zeroCounts >= 16)
		{
			for (int j=1; j<=zeroCounts/16; j++)
				outputBitString[index++] = SIXTEEN_ZEROS;
			zeroCounts = zeroCounts%16;
		}

		BitString bs = getBitCode(DU[i]); 

		//S1  =  (Run / Category) là giá trị 8 bit tổng hợp có dạng S1  = nhị phân RRRRCCCC.
		// R độ dài chạy của các hệ số 0 giữa các hệ số khác 0 (0->15)
		// C số bit cần thiết để đại diện cho biên độ của hệ số AC khác 0 (0->15)			

		// Bảng VLC Huffman

		outputBitString[index++] = HTAC[(zeroCounts << 4) | bs.length]; // Giá trị của nó
		outputBitString[index++] = bs; // Số lượng bit biểu diễn cho DU[i] != 0
		i++;
	}

	// Đánh dấu kí yuwj kếu thúc mã hóa huff
	if (endPos != 63)
		outputBitString[index++] = EOB;

	bitStringCounts = index;
}

/*
Đầu ra của bộ mã hóa entropy bao gồm một chuỗi gồm ba mã thông báo, được lặp lại cho đến khi khối hoàn thành.
Ba mã thông báo là chiều dài chạy, số lượng các số 0 liên tiếp đứng trước phần tử khác không hiện tại trong ma 
trận đầu ra DCT; số bit, số bit được sử dụng để mã hóa giá trị biên độ theo sau, như được xác định bởi sơ đồ mã hóa Huffman;
và biên độ, biên độ của hệ số DCT.
*/
// https://users.ece.utexas.edu/~ryerraballi/MSB/pdfs/M4L1.pdf

void JpegEncoder::write_bitstring(const BitString* bs, int counts, int& newByte, int& newBytePos, FILE* fp)
{
	unsigned short mask[] = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768};
	
	// Ghi từng thằng trong chuổi gộp mã hóa DC và ACs của tường thằng Y Cb Cr xuống file

	// Duyệt lần lượt từng thằng trong chuổi bs và ghi count byte màu xuống file ảnh
	for(int i=0; i<counts; i++)
	{
		int value = bs[i].value;
		int posval = bs[i].length - 1;

		while (posval >= 0)
		{
			// Nếu chưa là kí tự kết thúc cộng dồn từ vị trí mask thứ posval về vị trí 0
			if ((value & mask[posval]) != 0)
			{
				newByte = newByte  | mask[newBytePos];
			}
			posval--;
			newBytePos--;

			// Nếu đã dồn về tới 0 rồi thì ghi xuống file những thông tin
			if (newBytePos < 0)
			{
				// Ghi xuống file 1 byte màu
				write_byte((unsigned char)(newByte), fp);
				if (newByte == 0xFF)
				{
					write_byte((unsigned char)(0x00), fp);
				}

				// Khởi tạo lại giá trị mặc định
				newBytePos = 7;
				newByte = 0;
			}
		}
	}
}

// Đầu vào là index block đang xét. Ta cập nhật không gian màu sau khi chuyển đổi cho 3 khối Y, Cb, Cr theo công thức được nhắc đến trong bài
void JpegEncoder::convertColorSpace(int xPos, int yPos, char* yData, char* cbData, char* crData)
{
	for (int y=0; y<8; y++)
	{
		unsigned char* p = m_rgbBuffer + (y+yPos)*m_width*3 + xPos*3;
		for (int x=0; x<8; x++)
		{
			unsigned char B = *p++;
			unsigned char G = *p++;
			unsigned char R = *p++;

			yData[y*8+x] = (char)(0.299f * R + 0.587f * G + 0.114f * B - 128);
			cbData[y*8+x] = (char)(-0.1687f * R - 0.3313f * G + 0.5f * B );
			crData[y*8+x] = (char)(0.5f * R - 0.4187f * G - 0.0813f * B);
		}
	}
}

// Biến đổi cosin rời rạc (DCT 2 chiều x, y | FDC) theo công thức https://en.wikipedia.org/wiki/JPEG
// Mục đích của bước này là tổng hợp hầu hết tín hiệu ở 1 góc trên bên trái của mảng để chuẩn bị cho bước lượng tử hóa 
// làm giảm kích thước tổng thể để nén hiệu quả hơn
// trong giai đoạn entropy

// Đầu ra của phương thức là mảng zigzag fdc_data các giá trị sau khi biết đổi DCT
void JpegEncoder::foword_FDC(const char* channel_data, short* fdc_data)
{
	const float PI = 3.1415926f;
	for(int v=0; v<8; v++)
	{
		for(int u=0; u<8; u++)
		{
			// Ta có G(u,v) = 0.25*alpha_u*alpha_v*sigma((2x+1)*pi*u/16)*sigma((2x+1)*pi*v/16) sigma(0 -> 7)
			// Trong đó alpha_u và alpha_v = 1/sqrt(2) nếu u, v == 0. Ngược lại  = 1

			float alpha_u = (u==0) ? 1/sqrt(2.0f) : 1.0f;
			float alpha_v = (v==0) ? 1/sqrt(2.0f) : 1.0f;

			float temp = 0.f;
			for(int x=0; x<8; x++)
			{
				for(int y=0; y<8; y++)
				{
					float data = channel_data[y*8+x];

					data *= cos((2*x+1)*u*PI/16.0f);
					data *= cos((2*y+1)*v*PI/16.0f);

					temp += data;
				}
			}

			temp *= 0.25f*alpha_u*alpha_v/m_YTable[ZigZag[v*8+u]];
			// Sau khi biến đổi bước này ta thấy ô ở góc trên bên trái có |khá lớn|. Ô đó được gọi là DC thành phần không đổi và đó là màu cơ bản cho toàn bộ khối
			// 63 ô còn lại là AC thành phần xen kẽ (thành phần bổ sung)

			// Cập nhật qua mảng fdc:
			fdc_data[ZigZag[v*8+u]] = (short) ((short)(temp + 16384.5) - 16384);
		}
	}
}

// Theo định dạng từ wiki: https://en.wikipedia.org/wiki/JPEG_File_Interchange_Format

// Định nghĩ 3 phương thức ghi ghi theo word (2 byte), ghi theo byte(số byte), ghi bình thường (1 byte)
void JpegEncoder::write_jpeg_header(FILE* fp)
{
	//SOI
	write_word(0xFFD8, fp);		

	//APPO
	write_word(0xFFE0,fp);		
	write_word(16, fp);			
	write("JFIF", 5, fp);			// Mô tả định dạng file JPEG
	write_byte(1, fp);			
	write_byte(1, fp);			
	write_byte(0, fp);			
	write_word(1, fp);			
	write_word(1, fp);			
	write_byte(0, fp);			
	write_byte(0, fp);			

	//DQT
	write_word(0xFFDB, fp);		// Định nghĩa bảng Quntization
	write_word(132, fp);			
	write_byte(0, fp);			
									
									
	write(m_YTable, 64, fp);		
	write_byte(1, fp);			
	write(m_CbCrTable, 64, fp);	

	//SOFO
	write_word(0xFFC0, fp);			// Băt đầu bảng DCT
	write_word(17, fp);				
	write_byte(8, fp);				
	write_word(m_height&0xFFFF, fp);	
	write_word(m_width&0xFFFF, fp);	
	write_byte(3, fp);				

	write_byte(1, fp);				
	write_byte(0x11, fp);				
	write_byte(0, fp);				

	write_byte(2, fp);				
	write_byte(0x11, fp);				
	write_byte(1, fp);				

	write_byte(3, fp);				
	write_byte(0x11, fp);				
	write_byte(1, fp);				
	
	//DHT
	write_word(0xFFC4, fp); // Định nghĩa bảng huffman entropy
	write_word(0x01A2, fp);
	write_byte(0, fp);	
							
							
	write(Standard_DC_Luminance_NRCodes, sizeof(Standard_DC_Luminance_NRCodes), fp);	
	write(Standard_DC_Luminance_Values, sizeof(Standard_DC_Luminance_Values), fp);	
	write_byte(0x10, fp);			//HTYACinfo
	write(Standard_AC_Luminance_NRCodes, sizeof(Standard_AC_Luminance_NRCodes), fp);
	write(Standard_AC_Luminance_Values, sizeof(Standard_AC_Luminance_Values), fp); //we'll use the standard Huffman tables
	write_byte(0x01, fp);			
	write(Standard_DC_Chrominance_NRCodes, sizeof(Standard_DC_Chrominance_NRCodes), fp);
	write(Standard_DC_Chrominance_Values, sizeof(Standard_DC_Chrominance_Values), fp);
	write_byte(0x11, fp);		
	write(Standard_AC_Chrominance_NRCodes, sizeof(Standard_AC_Chrominance_NRCodes), fp);
	write(Standard_AC_Chrominance_Values, sizeof(Standard_AC_Chrominance_Values), fp);

	//SOS
	write_word(0xFFDA, fp);	// Vị trí bắt đầu quét
	write_word(12, fp);		
	write_byte(3, fp);		

	write_byte(1, fp);		
	write_byte(0, fp);		
								
	write_byte(2, fp);		
	write_byte(0x11, fp);		

	write_byte(3, fp);		
	write_byte(0x11, fp);		

	write_byte(0, fp);		
	write_byte(0x3F, fp);		
	write_byte(0, fp);		
}
