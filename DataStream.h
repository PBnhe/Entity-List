#ifndef MEMORY_STREAM
#define MEMORY_STREAM
#include<Windows.h>
#include<string>
#include "ChunkListT.h"
#include <chrono>
template<typename T>
class DataStream 
{
private:
	HANDLE serial_handle=0;

	size_t message_size = 0;
	size_t writeMessage_size=0;

	std::byte* read_buffer = nullptr;
	std::byte* write_buffer = nullptr;

public:
	bool open(std::string & port,DWORD accessType, DWORD baudrate = CBR_115200);//configura e abre a porta serial para comunicação via usb
	bool read(void* buffer);//escreve do readbuffer no buffer do parametro
	bool write(void * buffer);//escreve o buffer do parametro no write buffer e manda o write buffer para o serial
	/*DataStream()
	{
		serial_handle = 0;
	}*/
	void setMessageSize(size_t size)
	{
		delete[] read_buffer;
		read_buffer = new std::byte[size];
		message_size = size;
	}
	size_t getMessageSize() {
		return message_size;
	}

	void setWriteMessageSize(size_t size) {
		delete[] write_buffer;
		write_buffer = new std::byte[size];
		writeMessage_size = size;
	}
	size_t getWriteMessageSize() {
		return writeMessage_size;
	}

};

template<typename T>
bool DataStream<T>::open(std::string& port, DWORD accessType, DWORD baudrate)
{
	std::string fullPort = "\\\\.\\";
	fullPort += port;
	serial_handle = CreateFileA
	(
		fullPort.c_str(),
		accessType,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (serial_handle == INVALID_HANDLE_VALUE)
	{
		std::cout << "FALSE 1" << std::endl;
		std::cout <<"ERRO: "<< GetLastError() << std::endl;
		return false;
	}

	DCB dcbparam = { 0 };
	dcbparam.DCBlength = sizeof(dcbparam);
	if (!GetCommState(serial_handle, &dcbparam)) {
		std::cout << "FALSE 2" << std::endl;
		std::cout << GetLastError() << std::endl;
		return false;
	}
	dcbparam.BaudRate = baudrate;
	dcbparam.ByteSize = 8;
	dcbparam.StopBits = ONE5STOPBITS;
	dcbparam.Parity = NOPARITY;

	if (!SetCommState(serial_handle, &dcbparam)) {
		std::cout << "FALSE 3" << std::endl;
		std::cout << GetLastError() << std::endl;
		return false;
	}

	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout = 1;
	timeouts.ReadTotalTimeoutConstant = 1;
	timeouts.ReadTotalTimeoutMultiplier = 1;
	if (!SetCommTimeouts(serial_handle, &timeouts)) {
		std::cout << GetLastError() << std::endl;
		std::cout << "FALSE 4" << std::endl;

		return false;
	}
	std::cout << "TRUE" << std::endl;
	read_buffer = new std::byte[sizeof(entity<T>)];
	message_size = sizeof(entity<T>);
	write_buffer = new std::byte[sizeof(entity<T>)];
	writeMessage_size = sizeof(entity<T>);
	
	return true;
}

template<typename T>
bool DataStream<T>::read(void* pointer)
{//size por padrao é do tamanho de entity<T>
	auto start = std::chrono::high_resolution_clock::now();
	auto end = std::chrono::high_resolution_clock::now();
	size_t size = message_size;
	DWORD totalRead=0;
	
	
	while (totalRead < size) 
	{
		DWORD bytesRead = 0;
		if (!ReadFile(serial_handle, read_buffer+totalRead, size-totalRead, &bytesRead, NULL)) {
			//erro de leitura no handle
			return false;
		}
		totalRead += bytesRead;
		end = std::chrono::high_resolution_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() > 100.0f)
		{
			return false;//timeout de leitura
		}
		
	}
	memcpy(pointer, read_buffer, message_size);//copia os dados do buffer para o ponteiro fornecido no parametro
	return true;
}

template <typename T>
bool DataStream<T>::write(void* pointer) 
{
	memcpy(write_buffer, pointer, writeMessage_size);
	//buffer preenchido 
	auto start = std::chrono::high_resolution_clock::now();
	auto end = std::chrono::high_resolution_clock::now();
	DWORD totalWrite = 0;
	size_t size = writeMessage_size;

	while (totalWrite < size)
	{
		DWORD bytesWrite=0;
		if (!WriteFile(serial_handle, write_buffer + totalWrite, size - totalWrite, &bytesWrite, NULL)) 
		{
			std::cerr << "BAD WRITE" << std::endl;
			std::cerr << GetLastError();
			return false;
		}
		totalWrite += bytesWrite;
		end = std::chrono::high_resolution_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() > 100.0f) 
		{
			std::cerr << "WIRTE TIMEOUT" << std::endl;
			std::cerr << GetLastError() << std::endl;
			return false;
		}
	}

	return true;

}

#endif // !
