#ifndef MEMORY_STREAM
#define MEMORY_STREAM
#include<Windows.h>
#include<string>
#include "ChunkListT.h"
#include <chrono>
#include<unordered_map>

#define OP_SEND_RESSINC_REQUEST 0x01
#define OP_SEND_RESEND_REQUEST  0x02
#define OP_SEND_OK				0x03

#pragma pack(push, 1)
template<typename T>
struct	package
{
uint8_t OPCODE;
T payload;
uint8_t checksum;
};
#pragma pack(pop)






class DataStream 
{
private:
	HANDLE serial_handle=0;

	/*size_t message_size = 0;
	size_t writeMessage_size=0;

	std::byte* read_buffer = nullptr;
	std::byte* write_buffer = nullptr;*/

	std::unordered_map<uint8_t, size_t> Op_size;
	std::unordered_map < uint8_t, std::function<void(uint8_t OPCODE, std::byte* PAYLOAD, uint8_t checkSum,void* any)>> OP_FUNCTION;
	std::unordered_map<uint8_t, void*> OP_EXTRA;

	bool read(uint8_t* opcode, std::byte* & payload,uint8_t * checksum);//escreve nos parametros os dados obtidos de readFile
	
	bool ressinc();
	bool sinc = true;

public:
	bool open(std::string  port,DWORD accessType, DWORD baudrate = CBR_115200);//configura e abre a porta serial para comunica��o via usb
	void messageTreatment();
	uint8_t generateCheksum(uint8_t opcode, uint8_t* payload, size_t size);

const bool write(void*pointer, size_t size);//escreve o buffer do parametro no write buffer e manda o write buffer para o serial
void MapOPCODE(uint8_t opcode, size_t size,std::function<void(uint8_t OPOCDE,std::byte * PAYLOAD,uint8_t checkSum,void* any)> OP_FUNC,void * EXTRA);
	/*
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
	*/
};


bool DataStream::open(std::string port, DWORD accessType, DWORD baudrate)
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
		//throw std::runtime_error("Falha em abrir PORTA serial");
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
	dcbparam.StopBits = ONESTOPBIT;
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
	/*read_buffer = new std::byte[64];
	message_size = 64;
	write_buffer = new std::byte[64];
	writeMessage_size =64;*/
	
	return true;
}


bool DataStream::read(uint8_t* opcode, std::byte* & payload,uint8_t * checksum)
{
	auto start = std::chrono::high_resolution_clock::now();
	auto end = std::chrono::high_resolution_clock::now();
	size_t size = 1;
	DWORD totalRead=0;
	//ler 1 byte para determinar opcode
	//pegar tamanho para o payload
	//pegar checksum
	//tratar erros e send resinc requests
	while (totalRead < 1)
	{
		DWORD bytesRead = 0;
		if (!ReadFile(serial_handle, opcode, size - totalRead, &bytesRead, NULL)) {
			//erro de leitura no handle
			return false;
		}
		totalRead += bytesRead;
		end = std::chrono::high_resolution_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() > 10.0f)
		{
			return false;//timeout de leitura
		}
	}//lemos um byte e jogamos no opcode
	
	if (!Op_size.count(*opcode)) 
	{
		sinc = false;
		package<uint8_t> resinc_request;
		resinc_request.OPCODE = OP_SEND_RESSINC_REQUEST;
		resinc_request.payload = OP_SEND_RESSINC_REQUEST;
		resinc_request.checksum = generateCheksum(resinc_request.OPCODE, &resinc_request.payload, sizeof(uint8_t));
		write(&resinc_request, sizeof(package< uint8_t>));
		std::cerr << "OPCODE FALSE" << std::endl;
		return false;
	}
	
	size = Op_size.at(*opcode);
	payload = new std::byte[size];
	totalRead = 0;
	start = std::chrono::high_resolution_clock::now();
	while (totalRead < size) 
	{
		DWORD bytesRead = 0;
		if (!ReadFile(serial_handle, payload+totalRead, size-totalRead, &bytesRead, NULL)) {
			//erro de leitura no handle
			delete[] payload;
			payload = nullptr;
			return false;
		}
		totalRead += bytesRead;
		end = std::chrono::high_resolution_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() > 10.0f)
		{
			delete[] payload;
			payload = nullptr;
			return false;//timeout de leitura
		}
	}

	totalRead = 0;
	start = std::chrono::high_resolution_clock::now();
	while (totalRead < sizeof(uint8_t)) 
	{
		DWORD bytesRead = 0;
		if (!ReadFile(serial_handle, checksum + totalRead, sizeof(uint8_t) - totalRead, &bytesRead, NULL))
		{
			//erro de leitura no handle
			delete[] payload;
			payload = nullptr;
			return false;
		}
		totalRead += bytesRead;
		end = std::chrono::high_resolution_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() > 10.0f)
		{
			delete[] payload;
			payload = nullptr;
			return false;//timeout de leitura
		}
	}
	//todos os ponteiros estão preenchidos
	if (*checksum != generateCheksum(*opcode, reinterpret_cast<uint8_t*>(payload), size))
	{//check sum não bate
	//então precisamos ressincronizar o programa para que após a ressinc ele envie a mensagem novamente , request será feita no ressinc()
		sinc = false;
		package<uint8_t> resinc_request;
		resinc_request.OPCODE = OP_SEND_RESSINC_REQUEST;
		resinc_request.payload = OP_SEND_RESSINC_REQUEST;
		resinc_request.checksum = generateCheksum(resinc_request.OPCODE, &resinc_request.payload, sizeof(uint8_t));
		write(&resinc_request, sizeof(package< uint8_t>));
		delete[] payload;
		payload = nullptr;
		std::cerr << "CHECKSUM FALSE" << std::endl;

		return false;
	}
	
	
	return true;
}


const bool DataStream::write(void* pointer,size_t size)
{
	//memcpy(write_buffer, pointer, writeMessage_size);
	//buffer preenchido 
	std::byte * arithmeticPointer = static_cast<std::byte*>(pointer);

	//memcpy(buffer, pointer, size);
	auto start = std::chrono::high_resolution_clock::now();
	auto end = std::chrono::high_resolution_clock::now();
	DWORD totalWrite = 0;
	//size_t size = writeMessage_size;
	
	while (totalWrite < size)
	{
		DWORD bytesWrite=0;
		if (!WriteFile(serial_handle, arithmeticPointer + totalWrite, size - totalWrite, &bytesWrite, NULL)) 
		{
			std::cerr << "BAD WRITE" << std::endl;
			std::cerr << GetLastError();
			//delete[] buffer;
			return false;
		}
		totalWrite += bytesWrite;
		end = std::chrono::high_resolution_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() > 100.0f) 
		{
			std::cerr << "WRITE TIMEOUT" << std::endl;
			std::cerr << GetLastError() << std::endl;
			//delete[] buffer;
			return false;
		}
	}
	//delete[] buffer;
	return true;

}

void DataStream::messageTreatment() 
{
DWORD errors;
COMSTAT status;
if (!ClearCommError(serial_handle,&errors,&status))
{
	std::cerr << "FALHA EM CLEARCOM" << std::endl;
	return;
}
if (status.cbInQue < 2) 
{
	return;
}
	uint8_t opcode;
	std::byte* payload=nullptr;
	uint8_t checksum;
	if (sinc)
	{
		//std::cerr << "sinc" << std::endl;
		if (!read(&opcode, payload, &checksum)) {
			std::cerr << "FailtoRead" << std::endl;
			return;
		}
	}
	if (!sinc) 
	{
		std::cerr << "desinc" << std::endl;
		if (!ressinc()) 
		{
			package<uint8_t> resinc_request;
			resinc_request.OPCODE = OP_SEND_RESSINC_REQUEST;
			resinc_request.payload = OP_SEND_RESSINC_REQUEST;
			resinc_request.checksum = generateCheksum(resinc_request.OPCODE, &resinc_request.payload, sizeof(uint8_t));
			write(&resinc_request, sizeof(package< uint8_t>));
			return;
		}
		sinc = true;
		
	}
	if (!payload) 
	{
		return;
	}
//para deixar as funções + dinamicas vamos trabalhar com unordereds , parecido com doScalar
//cada função recebe opcode , payload e checksum , mesmo que o hash delas seja um opcode e ele provavel que nao seja util
//tudo por cópia para segurança
	std::function<void(uint8_t OPCODE, std::byte* PAYLOAD, uint8_t checkSum, void* any)> task;
	task = OP_FUNCTION.at(opcode);
	task(opcode, payload, checksum, OP_EXTRA.at(opcode));
	


	if (payload) 
	{
		delete[] payload;
	}
}

void DataStream::MapOPCODE(uint8_t opcode, size_t PAYLOAD_SIZE, std::function<void(uint8_t OPOCDE, std::byte* PAYLOAD, uint8_t checkSum, void* any)> OP_FUNCTION,void * EXTRA)
{
	if (opcode == 0x01 || opcode == 0x02 || opcode == 0x03) {
		std::cerr << "OPs reservados ao sistema" << std::endl;
		return;
	}

	if (Op_size.count(opcode)) 
	{
		std::cerr << "OPCODE DUPLICADO" << std::endl;
		std::cerr << "O OPCODE : " << opcode << "JA ESTA INSERIDO , OPCODES DEVEM SER UNICOS " << std::endl;
		return;
	}

	Op_size.emplace(opcode, PAYLOAD_SIZE);
	this->OP_FUNCTION.emplace(opcode, OP_FUNCTION);
	OP_EXTRA.emplace(opcode, EXTRA);



}

uint8_t DataStream::generateCheksum(uint8_t opcode, uint8_t * payload, size_t PAYLOAD_size) 
{
	uint8_t sum = opcode;
	for (uint8_t i = 0;i < PAYLOAD_size;i++)
	{
		sum += payload[i];
	}
	return sum;
}

bool DataStream::ressinc() 
{//esse método tenta ressincronizar os seriais , tentando achar os bytes 0x01 0x00 0x00 0x01
//ou seja um op 0x01 , dois uint 0x00 de payload e o sum sendo a soma deles , que é 0x01
//portanto [0x01] [0x00] [0x00] [0x01]
//ler de bit em bit , se não for algum dos desejados descarte
	auto start = std::chrono::high_resolution_clock::now();
	auto end = std::chrono::high_resolution_clock::now();


	bool found=false;
	DWORD totalRead = 0;
	uint8_t bytes[4];
	int i = 0;
	uint8_t targets[4]{ 0x01,0x00,0x00,0x01 };

	while (i<4) 
	{
		totalRead = 0;
		while (totalRead < sizeof(uint8_t))
		{
			DWORD bytesRead = 0;
			if (!ReadFile(serial_handle, &bytes[i] + totalRead, sizeof(uint8_t) - totalRead, &bytesRead, NULL))
			{
				//erro de leitura no handle
				std::cerr << "FALHA READFILE NO METODO RESSINC" << std::endl;
				return false;
			}
			totalRead += bytesRead;
			end = std::chrono::high_resolution_clock::now();
			if (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() > 10.0f)
			{
				std::cerr << "READ TIMEOUT EM RESSINC" << std::endl;
				return false;//timeout de leitura
			}
		}
		
		if (bytes[i] == targets[i])
		{
			i++;
		}
		else {
			i = 0;
		}

	}
//last check
	//PurgeComm(serial_handle, PURGE_RXCLEAR);
	package<uint8_t> sendOK;
	sendOK.OPCODE = OP_SEND_OK;
	sendOK.payload = 0x00;
	sendOK.checksum = generateCheksum(sendOK.OPCODE, &sendOK.payload, sizeof(uint8_t));
	write(&sendOK, sizeof(package<uint8_t>));
	std::cerr << "ressinc" << std::endl;
	return true;
}

#endif // !
