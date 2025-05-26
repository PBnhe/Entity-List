#include <iostream>
#include <chrono>
#include "Interface.h"
#include"ThreadPool.h"
#include "DataStream.h"

using namespace std;
using namespace std::chrono;

struct onOFFsensor {
	uint8_t opcode;
	bool boolean;
	uint8_t checksum;

};

struct safevec 
{
	std::vector<entity<int>*> todelete;
	std::mutex mu;
	Interface<int>* interf=nullptr;
	int counter = 0;
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

void heavyIntOp(int & value,int &id,entity<int> * ref,void* extra) 
{
	auto safev = reinterpret_cast<safevec*>(extra);
	int result = 0;
	int n = std::abs(value);
	for (int i = 2; i * i <= n; ++i) {
		while (n % i == 0) {
			result += i;
			n /= i;
		}
	}
	if (n > 1) result += n;
	value = result;
	
	/*if (value > 1000)
	{
		std::unique_lock<std::mutex> lock(safev->mu);
			//buffer->push_back(ref);
		safev->interf->removeDataIn(ref);
		safev->interf->trashControl();
		safev->counter++;
	}
	*/
   
}
void MessageToList(uint8_t OPCODE, std::byte* PAYLOAD, uint8_t checkSum, void* any) 
{
Interface<float>* entitylist = reinterpret_cast<Interface<float>*> (any);
int id;
int data;
memcpy(&id, PAYLOAD, 4);
memcpy(&data, PAYLOAD + 4, 4);
entitylist->insertData(id, data);

}


int main() 
{
	int a;
	
	std::byte buffer[8];
	/*
	DataStream<float> istream;
	std::string str = "COM4";
	istream.open(str, GENERIC_READ);
	system("Pause");*/

	ThreadPool Fpool;
	DataStream dstream;
	entityConfig config;
	config.autoGarbageRoutine = true;
	config.chunkSize = 26;
	config.enableReadBuffer = true;
	config.maxTrashBins = 3;
	config.size = 100000;

	serialConfig configserial;
	configserial.accessType = GENERIC_READ | GENERIC_WRITE;
	configserial.CBR = CBR_115200;
	configserial.COM_port = "COM6";
	dstream.open(configserial.COM_port, GENERIC_READ | GENERIC_WRITE);
	
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
	  
	entitylist->insertData(entidade->id, entidade->data);
	//entitylist->insertScalarFunction(1, heavyIntOp);
	cout << "APERTE QUALQUER TECLA PARA CONTINUAR" << endl;
	system("Pause");

	cout << "ID: " << entidade->id << endl;
	cout << "DATA: " << entidade->data << endl;
	
	
	
	cout << "COLETANDO DADOS" << endl;
	auto start = high_resolution_clock::now();
	auto end = high_resolution_clock::now();
	

	//entitylist->DoScalar(1, 0);

	cout << "pronto :(" << endl;
	while (true) 
	{
		int cas0 = 0;
		cout << "INISRA OPCAO: " << endl;
		cout << "1:: BUSCAR ID" << endl;
		cout << "2:: LIMPAR DADOS" << endl;
		cout << "3:: APLCICAR METODO" << endl;
		cout << "4:: DADOS DA LISTA" << endl;
		cout << "5:: CONTROLAR SENSOR" << endl;
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
		}
		
		}
		dstream.messageTreatment();
		





	}
	
	
	

	
	

	
	
	

	system("Pause");



}
