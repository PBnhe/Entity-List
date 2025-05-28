#include"Filestream.h"


FileStream::FileStream(std::string InputFile, std::string outputFile)
{

	input_file.open(InputFile,std::ios::binary|std::ios::app);
	output_file.open(outputFile, std::ios::binary|std::ios::trunc);
	output_path = outputFile;
	input_path = InputFile;
}

FileStream::~FileStream() 
{
	output_file.flush();
	output_file.close();
	input_file.close();

}

void FileStream::WriteInFile( void* source, size_t sourceSize) 
{
	output_file.write(reinterpret_cast<char*>(source), sourceSize);
}

bool FileStream::ReadFromFile(void* target, size_t targetsize)
{
	if (!input_file.read(reinterpret_cast<char*>(target), targetsize)) 
	{
		return false; 
	}
	return true;

}

void FileStream::Commit(std::string NewFileName) 
{
	input_file.close();
	output_file.flush();
	output_file.close();
	input_file.open(output_path, std::ios::binary);
	output_file.open(NewFileName, std::ios::binary | std::ios::trunc);
	input_path = output_path;
	output_path = NewFileName;

}