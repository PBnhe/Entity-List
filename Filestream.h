#pragma once


#ifndef FILE_STREAM_H
#define FILE_STREAM_H
#include<fstream>
#include<iostream>

//a classe permite leitura e escrita de arquivos binários
 //temos aqui dois arquivos , um arquivo que será responsavel por fornecer dados
//outro que será alvo de dados
//dessa forma separamos as etapas do processo e permitimos rollbacks a cada etapa
//
//contem um método commit que faz o output file virar o arquivo de input
//para permitir que outro arquivo de output seja criado , gravando a proxima etapa
//do processo caso necessário

class FileStream 
{
private:
	std::ifstream input_file;
	std::string input_path;

	std::ofstream output_file;
	std::string output_path;
	


public:
	
	bool ReadFromFile(void* target,size_t targetsize);
	void WriteInFile( void* source, size_t sourceSize);
	void Commit(std::string newoutputFile);

	FileStream(std::string In_FileName,std::string Out_FileName);
	~FileStream();
};









#endif // !FILE_STREAM_H
