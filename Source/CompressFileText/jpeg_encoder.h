#ifndef __JPEG_ENCODER_HEADER__
#define __JPEG_ENCODER_HEADER__


class JpegEncoder
{
private:
	int				m_width;		// Chiều rộng
	int				m_height;		// Chiều cao
	unsigned char* m_rgbBuffer;	// Mảng chứa toàn bộ điểm ảnh m_width*m_height*3

	unsigned char	m_YTable[64];	// Chứa độ sáng khi chuyển đổi không gian màu
	unsigned char	m_CbCrTable[64]; // Chứa độ màu khi chuyển đổi khôn gian màu bao gồm Cb và Cr

	// Một bitstring khi mã hóa sẽ được lưu dưới dạng 1 cặp giá trị (độ dài bit biểu diễn giá trị value, giá trị value)
	struct BitString
	{
		int length;
		int value;
	};

	// Bảng mã code huffman để mã hóa entropy trên DC và ACs
	BitString m_Y_DC_Huffman_Table[12];
	BitString m_Y_AC_Huffman_Table[256];

	BitString m_CbCr_DC_Huffman_Table[12];
	BitString m_CbCr_AC_Huffman_Table[256];

public:
	// Renew lại bộ đệm
	void clean(void);

	// Đọc dữ liệu ảnh BMP 24 bit deepth (Nén ảnh BMP sang chuản JPEG)
	bool readFromBMP(const char* fileName);

	// Mã hóa thành ảnh JPEG
	bool encodeToJPG(const char* fileName, int quality_scale);



private:
	// Phương thức khởi tạo các giá trị mặc định cho table
	void initHuffmanTables(void);

	// Khởi tạo các thay đổi về chất lượng nén ảnh. Tham số càng lớn thì dung lượng ảnh nén càng nhỏ. Đầu vào là một số nguyên từ 1 đến 99.
	void initQualityTables(int quality);

	// Được gọi khi đã init bảng Table. Khởi tạo ban đầu
	void computeHuffmanTable(const char* nr_codes, const unsigned char* std_table, BitString* huffman_table);

	// Phương thứuc trả về số lượng bit cần tối thiểu để biểu diễn số nguyên value
	BitString getBitCode(int value);

	// Chuyển đổi không gian màu từ RGB sang Y(Độ sáng) Cb Cr (Độ màu)
	void convertColorSpace(int xPos, int yPos, char* yData, char* cbData, char* crData);

	// biến đổi cosin rời rạc 2 chiều
	void foword_FDC(const char* channel_data, short* fdc_data);

	// Mã hóa huffman
	void doHuffmanEncoding(const short* DU, short& prevDC, const BitString* HTDC, const BitString* HTAC,  BitString* outputBitString, int& bitStringCounts);

private:
	// ghi header file JPEG
	void write_jpeg_header(FILE* fp);

	// Các bién thể của phương thức ghi xuống file
	void write_byte(unsigned char value, FILE* fp); // ghi lần 1 byte
	void write_word(unsigned short value, FILE* fp); // ghi lần 1 word (2 byte)
	void write_bitstring(const BitString* bs, int counts, int& newByte, int& newBytePos, FILE* fp); // ghi lần 1 đối tượng của mảng Bitstring theo số lượng biến count byte
	void write(const void* p, int byteSize, FILE* fp); // hia lần 1 byte nhưng đặc biệt để ghi header file

public:
	// Phương thức tạo và phương thức hủy
	JpegEncoder();
	~JpegEncoder();
};

#endif
