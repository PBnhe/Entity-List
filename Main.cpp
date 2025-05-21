#include <iostream>
#include <chrono>
#include "Interface.h"
#include"ThreadPool.h"
#include "DataStream.h"

using namespace std;
using namespace std::chrono;


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

	entityConfig config;
	config.autoGarbageRoutine = true;
	config.chunkSize = 26;
	config.enableReadBuffer = true;
	config.maxTrashBins = 3;
	config.size = 100000;

	serialConfig configserial;
	configserial.accessType = GENERIC_READ | GENERIC_WRITE;
	configserial.CBR = CBR_115200;
	configserial.COM_port = "COM4";
	configserial.message_size = sizeof(entity<float>);

	Interface<float>* entitylist=nullptr;
	entitylist = new Interface<float>();
	entitylist->openSerial(configserial);
	entitylist->insertScalarFunction(1, readTemperature);
	
	entitylist->initialize(config);
	
	entitylist->dataD();
	entitylist->setSerialMessageSize(sizeof(entity<float>));
	
	
	entity<float>* entidade = nullptr;
	entidade = new entity<float>;
	cout << "ENTIDADE CRIADA" << endl;
	if (!entitylist->readSerial(entidade, false)) 
	{
		std::cerr << "ERRO EM READ SERIAL " << endl;
	}
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
	while (duration_cast<milliseconds>(end - start).count()<10000)
	{
		if (!entitylist->readSerial(entidade,1))
		{
			cout << "BAD READ" << endl;
			start = high_resolution_clock::now();
			continue;
		}
		entidade = reinterpret_cast<entity<float>*>(buffer);
		entitylist->insertData(entidade->id, entidade->data);

		//cout << "ID: " << entidade->id << endl;
		//cout << "DATA: " << entidade->data << endl;
		//cout << "TEMPERATURA: " << readTemperature(entidade->data)<<endl;
		//cout << "-----------------------------------------------------------------------" << endl;

		end = high_resolution_clock::now();
	}

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
			int id = 0;
			cout << "INSIRA ID DO METODO" << endl;
			cin >> id;
			entitylist->DoScalar(1, 0);
			break;
		}
		case 4: 
		{
			cout<<"MEM DISPO: "<<entitylist->available()<<endl;
			cout << "MEM USADA: "<<entitylist->memUsed()<<endl;
			entitylist->dataD();
			break;

		}
		
		}







	}
	
	
	

	
	

	
	
	

	system("Pause");



}
