#include <iostream>
#include <chrono>
#include "Interface.h"
#include"ThreadPool.h"
#include "DataStream.h"
#include"Filestream.h"

using namespace std;
using namespace std::chrono;

struct onOFFsensor {
	uint8_t opcode;
	bool boolean;
	uint8_t checksum;

};


void readTemperature(float& value, int& id, entity<float>* ref, void* extra) {
	//cout << "DEBEUG: " << adcValue << endl;
	int adcValue = value;
	const float BETA = 3435.0;       // Constante B do termistor
	const float R0 = 10000.0;        // Resistência nominal a 25°C (10)
	const float T0 = 298.15;         // Temperatura de referência em Kelvin (25°C)
	const float SERIES_RESISTOR = 10000.0; // Resistor fixo no divisor (10)
	const float ADC_MAX = 1023.0;    // Valor máximo do ADC (10 bits)
	const float VCC = 5.0;           // Tensão de alimentação (5V ou 3.3V)

	// 1. Calcula a tensão a partir da leitura do ADC
	float voltage = (adcValue / ADC_MAX) * VCC;

	// 2. Calcula a resistência do termistor (NTC)
	float resistance = SERIES_RESISTOR * (VCC / voltage - 1);

	// 3. Calcula a temperatura em Kelvin usando a equação de Steinhart-Hart
	float temperatureK = 1.0 / (1.0 / T0 + (1.0 / BETA) * log(resistance / R0));

	// 4. Converte a temperatura para Celsius
	float temperatureC = temperatureK - 273.15;

	value= temperatureC;
}


void MessageToList(uint8_t OPCODE, std::byte* PAYLOAD, uint8_t checkSum, void* any) 
{
Interface<float>* entitylist = reinterpret_cast<Interface<float>*> (any);
int id;
float data;
memcpy(&id, PAYLOAD, 4);
memcpy(&data, PAYLOAD + 4, 4);
//std::cerr << "ID: " << id<<std::endl;
//std::cerr << "DADOS: " << data<<std::endl;
entitylist->insertData(id, data);

}


int main() 
{
	
	DataStream dstream;
	entityConfig config;
	config.autoGarbageRoutine = true;
	config.chunkSize = 26;
	config.maxTrashBins = 3;
	config.size = 100000;

	
	dstream.open("COM6", GENERIC_READ | GENERIC_WRITE,1000000);
	
	Interface<float>* entitylist=nullptr;

	entitylist = new Interface<float>();
	uint8_t op = 0x04;
	dstream.MapOPCODE(op, 8, MessageToList, entitylist);
	

	
	entitylist->insertScalarFunction(1, readTemperature);
	
	entitylist->initialize(config);
	
	entitylist->dataD();
	
	
	
	entity<float>* entidade = nullptr;
	entidade = new entity<float>;
	cout << "ENTIDADE CRIADA" << endl;
	
	cout << "Pass" << endl;
	  
	
	
	cout << "APERTE QUALQUER TECLA PARA CONTINUAR" << endl;
	system("Pause");
	entitylist->createInOutFiles("Inpute", "Ortepute");

	while (true) 
	{
		int cas0 = 0;
		cout << "INISRA OPCAO: " << endl;
		cout << "1:: BUSCAR ID" << endl;
		cout << "2:: LIMPAR DADOS" << endl;
		cout << "3:: APLICAR METODO" << endl;
		cout << "4:: DADOS DA LISTA" << endl;
		cout << "5:: CONTROLAR SENSOR" << endl;
		cout << "6::COLETA DADOS POR 10 SEGUNDOS" << std::endl;
		cout << "7::LIMPAR CONTADOR DE ID DA PLACA" << std::endl;
		cout << "8:: ESCREVA" << std::endl;
		cout << "9::LEIA INPUTFILE " << std::endl;
		cin >> cas0;
		switch (cas0)
		{
		case 1: 
		{
			int id = 0;
			cout << "INSIRA ID A SER BUSCADO" << endl;
			cin >> id;
			cout << "DADOS EM " << id << ": " << entitylist->returnDataByID(id) << endl;
			break;
		}
		case 2: 
		{
			cout << "LIMPO" << endl;
			entitylist->clear();
			break;
		}
		case 3: 
		{
			cin.clear();
			cin.ignore(1000000, '\n');
			int ide = 0;
			cout << "INSIRA ID DO METODO" << endl;
			cin >> ide;
			entitylist->DoScalar(ide, 0);
			break;
		}
		case 4: 
		{
			cout<<"MEM DISPO: "<<entitylist->available()<<endl;
			cout << "MEM USADA: "<<entitylist->memUsed()<<endl;
			entitylist->dataD();
			cin.clear();
			break;

		}
		case 5: 
		{
			int dec;
			cout << "INSIRA 1 PARA ATIVAR E 0 PARA DESATIVAR" << endl;
			cin >> dec;
			package<bool> message;
			message.OPCODE = 0x05;
			message.payload = dec;
			message.checksum = dstream.generateCheksum(message.OPCODE, reinterpret_cast<uint8_t*>(&message.payload), sizeof(bool));
			dstream.write(&message, sizeof(onOFFsensor));
			break;
		}
		case 6: 
		{
			package<bool> message;
			message.OPCODE = 0x05;
			message.payload = true;
			message.checksum = dstream.generateCheksum(message.OPCODE, reinterpret_cast<uint8_t*>(&message.payload), sizeof(bool));
			dstream.write(&message, sizeof(onOFFsensor));
			auto start = high_resolution_clock::now();
			auto end = high_resolution_clock::now();
			while (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() < 10000.0f) 
			{
				dstream.messageTreatment();
				end = high_resolution_clock::now();
			}
			//package<bool> message;
			message.OPCODE = 0x05;
			message.payload = false;
			message.checksum = dstream.generateCheksum(message.OPCODE, reinterpret_cast<uint8_t*>(&message.payload), sizeof(bool));
			dstream.write(&message, sizeof(onOFFsensor));
			break;
		}
		case 7: 
		{
			package<uint8_t> message;
			message.OPCODE = 0x07;
			message.payload = false;
			message.checksum = dstream.generateCheksum(message.OPCODE, reinterpret_cast<uint8_t*>(&message.payload), sizeof(bool));
			dstream.write(&message, sizeof(package<uint8_t>));
			std::cout << "LIMPO" << std::endl;
			break;
		}
		case 8: 
		{
			cout << "Case 8" << endl;
			entitylist->WriteInOutputFile();
			cout << "Scrito" << endl;
			entitylist->commitInOutFile("Teste");
			break;
		}

		case 9: 
		{
			cout << "Case 9" << endl;
			entitylist->ReadFromInputFile();
			break;

		}

		}
		

		system("Pause");
		system("cls");
		
		//dstream.messageTreatment();
		





	}
	
	
	

	
	

	
	
	

	system("Pause");



}
