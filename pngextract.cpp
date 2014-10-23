/*
	pngextract.cpp
	Matt Penny 2014

	Searches for and extracts PNG files from binary files.
	Originally made to extract android and iOS emoji, because I'm cool like that.
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

const int CD_BUF_SIZE = 1024;

const unsigned char PNG_HEADER[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A}; //"\211PNG\r\n\032\n"
const char END_CHUNK_TYPE[] = {0x49, 0x45, 0x4E, 0x44, 0x00}; //"IEND"

void findPNGs(std::ifstream*);
void copyChunks(std::ifstream*, std::ofstream*);

int main(int argc, char** argv)
{
	std::cout << "PNGExtract" << std::endl << "Matt Penny 2014" << std::endl << std::endl;

	if (argc > 1)
	{
		std::cout << "Extracting PNGs from " << argv[1] << "..." << std::endl;

		//Find those pesky PNGs
		std::ifstream in(argv[1], std::ios::binary);
		findPNGs(&in);
		in.close();

		std::cout << "Done." << std::endl;
	}
	else
		std::cout << "Specify file name as argument." << std::endl;
	return 0;
}

/*
	Finds and saves PNGs in a binary file.

	in - Pointer to ifstream associated with file containing PNGs.
*/
void findPNGs(std::ifstream* in)
{
	//Save old position
	std::streamoff oldStreamPos = in->tellg();
	in->seekg(0);

	int headLength = sizeof(PNG_HEADER)/sizeof(char);
	int headPos=0, PNGCount=0;

	while (!in->eof())
	{
		//Made it the end of header sequence. We got one!
		if (headPos == headLength)
		{
			//Create PNG file to store data
			std::stringstream ss;
			ss << PNGCount << ".png";
			std::ofstream out(ss.str().c_str(), std::ios::binary);

			//Write PNG data
			out.write((char*)PNG_HEADER, headLength);
			copyChunks(in, &out);
			out.close();

			PNGCount++;
			headPos = 0;
		}

		//Search for PNG header
		if ((unsigned char)in->get() != PNG_HEADER[headPos++])
			headPos = 0;
	}
	std::cout << "Wrote " << PNGCount << " PNG file(s)." << std::endl;
	in->seekg(oldStreamPos);
}

/*
	Copies chunks from a binary file to a PNG file.

	in  - Pointer to ifstream associated with file containing PNGs, at start of first chunk.
	out - pointer to ofstream associated with PNG to write, at start of first chunk.
*/
void copyChunks(std::ifstream* in, std::ofstream* out)
{
	bool lastChunk = false;
	while (!lastChunk)
	{
		//Copy chunk size
		unsigned char cs[4];
		in->read((char*)cs, 4);
		out->write((char*)cs, 4);
		unsigned int chunkSize = (cs[0]<<24) | (cs[1]<<16) | (cs[2]<<8) | cs[3]; //Endianness is stupid

		//Copy chunk type
		char chunkType[5];
		in->read(chunkType, 4);
		out->write(chunkType, 4);
		chunkType[4] = '\0';					        //Pretty hacky,
		lastChunk = !strcmp(chunkType, END_CHUNK_TYPE); //but it works

		//Copy chunk data
		char buf[CD_BUF_SIZE];
		int toRead = chunkSize;
		while (toRead > 0)
		{
			int numBytes = CD_BUF_SIZE < toRead ? CD_BUF_SIZE : toRead;
			in->read(buf, numBytes);
			out->write(buf, numBytes);
			toRead -= numBytes;
		}

		//Copy chunk CRC
		unsigned char chunkCRC[4];
		in->read((char*)chunkCRC, 4);
		out->write((char*)chunkCRC, 4);
	}
}
