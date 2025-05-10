#include <iostream>
#include <chrono>
#include "Interface.h"
#include"ThreadPool.h"

using namespace std;
using namespace std::chrono;

void vendo() 
{
	 
	{
		cout << endl;
		cout << "EU RASGO MEU CANECO E SENTO O PAU" << "Thread ID: " << std::this_thread::get_id() << endl;
	}
	
}

void heavyIntOp(int & value) 
{
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
}

int main() 
{


	ThreadPool Fpool;

	entityConfig config;
	config.autoGarbageRoutine = true;
	config.chunkSize = 16;
	config.enableReadBuffer = true;
	config.maxTrashBins = 3;
	config.size = 10000000;

	Interface<int>* entitylist;
	entitylist = new Interface<int>();

	entitylist->initialize(config);
	
	entitylist->dataD();

	entitylist->insertScalarFunction(1, heavyIntOp);


	
	int a;
	
	

	cout << "Core number: " << std::thread::hardware_concurrency() << endl;
	cout << "INICIANDO DEBUGG SECC" << endl;
	cout << endl;
	cout << "MEMORIA DISPONIVEL: " << entitylist->available()<<endl;
	cout << "REFERENCIAS USADAS: " << entitylist->memUsed() << endl;

	cout << "INICIO DE LOOP 100kk INTEIROS" << endl;
	for (int i = 0; i < 10000000; i++) 
	{
		entitylist->insertData(i, i);
	}
	cout << endl;
	cout << "FIM DE LOOP 100kk INTEIROS" << endl;
	cout << endl;
	cout << "MEMORIA DISPONIVEL: " << entitylist->available() << endl;
	cout << "REFERENCIAS USADAS: " << entitylist->memUsed() << endl;
	cout << endl;
	
	cout << "tempo de funcaoo scalar com threads: " << endl;
	auto start = high_resolution_clock::now();
	entitylist->DoScalar_semThread(1);
	auto end = high_resolution_clock::now();
	cout << '\n';
	cout << duration_cast<milliseconds>(end - start).count() << " Milliseconds" << '\n';
	int c = entitylist->returnDataByID(100);
	cout << "INT C:" << c << endl;
	

	

	/*cout << "TENTANDO ESCREVER MAIS 1 INTEIRO DE ID 12 E DATA 6" << endl;
	entitylist->insertData(-1, -1);
	cout << endl;
	cout << "INTEIRO ESCRITO" << endl;
	cout << "MEMORIA DISPONIVEL: " << entitylist->available() << endl;
	cout << "REFERENCIAS USADAS: " << entitylist->memUsed() << endl;
	cout << endl;
	cout << "TENTANDO RETORNAR INTEIRO POR ID" << endl;
	auto start = high_resolution_clock::now();
	cout << "DATA ESPERADO : -1 /// DATA OBTIDO: " << entitylist->returnDataByID(-1)<<endl;
	auto end = high_resolution_clock::now();

	cout << '\n';
	cout << duration_cast<milliseconds>(end - start).count() << " Milliseconds" << '\n';
	cout << "PROCURANDO DNV"<<endl;
	start = high_resolution_clock::now();
	cout << "DATA ESPERADO : -1 /// DATA OBTIDO: " << entitylist->returnDataByID(-1) << endl;
	end = high_resolution_clock::now();
	cout << '\n';
	cout << duration_cast<milliseconds>(end - start).count() << " Milliseconds" << '\n';
	
	int b = 0;
	cout << "TESTANDO THREADS: " << endl;
	start = high_resolution_clock::now();
	b = entitylist->retunrDataByID_Thread(-1);
	end = high_resolution_clock::now();
	cout << "TEMPO DE BUSCA :" << endl;
	cout << '\n';
	cout << duration_cast<milliseconds>(end - start).count() << " Milliseconds" << '\n';
	cout << "DATA ESPERADO EM B:  -1/// DATA OBTIDO:  " << b<<endl;
	*/
	

	
	
	

	system("Pause");



}
