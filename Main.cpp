#include <iostream>
#include <chrono>
#include "Interface.h"
#include"ThreadPool.h"

using namespace std;
using namespace std::chrono;


struct safevec 
{
	std::vector<entity<int>*> todelete;
	std::mutex mu;
	Interface<int>* interf=nullptr;
	int counter = 0;
};


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


	ThreadPool Fpool;

	entityConfig config;
	config.autoGarbageRoutine = true;
	config.chunkSize = 26;
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

	cout << "INICIO DE LOOP 100kk INT" << endl;
	for (int i = 0; i < 100000; i++)
	{
		entitylist->insertData(i, i);
	}
	

	cout << endl;
	cout << "FIM DE LOOP 100kk INT" << endl;
	cout << endl;
	cout << "MEMORIA DISPONIVEL: " << entitylist->available() << endl;
	cout << "REFERENCIAS USADAS: " << entitylist->memUsed() << endl;
	cout << endl;
	safevec deletebuffer;
	deletebuffer.interf = entitylist;
	
	

	entitylist->DoScalar(1,&deletebuffer);
	auto end = high_resolution_clock::now();
	cout << '\n';
	cout << "COUNTER DA STRUCT: " << deletebuffer.counter << endl;;
	cout << "DOscalar:" << endl;
	//cout << duration_cast<milliseconds>(end - start).count() << " Milliseconds" << '\n';
	cout << "invalidando referencias" << endl;
	//entitylist->removeDataIn(deletebuffer.todelete);
	entitylist->dataD();
	cout << endl;
	
	
	// start = high_resolution_clock::now();

/*	cout << "TESTE DELECAO DE 1080 C THREAD :" << endl;
	for (int i = 0;i < 5000;i++)
	{
		vec.push_back(i * (i + 10));
		//entitylist->removeDataByID(i*(i+10));
	}
	//auto start = high_resolution_clock::now();
	entitylist->removeDataByID(vec);
	 end = high_resolution_clock::now();
	cout << '\n';
	cout << duration_cast<milliseconds>(end - start).count() << " Milliseconds" << '\n';
	*/



	cout << "REFERENCIAS DISPONIVEIS: " << entitylist->available()<<endl;
	cout << "REFERENCIAS USADAS: " << entitylist->memUsed()<<endl;
	entitylist->dataD();
	cout << "COLAPSANDO LISTA" << endl;
	entitylist->collapse();
	entitylist->dataD();

	cout << "DELETANDO TUDO: "<<endl;
	
	delete entitylist;
	cout << "TUDO FOI DELETADO" << endl;

	//deleção com thread : 28911 milliseconds  12 nucleos 3.6ghz
	//deleção sem threads: 92078 milliseconds




	/*cout << "tempo de funcaoo com threads: " << endl;
	auto start = high_resolution_clock::now();
	entitylist->DoScalar(1);
	auto end = high_resolution_clock::now();
	cout << '\n';
	cout << duration_cast<milliseconds>(end - start).count() << " Milliseconds" << '\n';
	int c = entitylist->returnDataByID(100);
	int b = entitylist->returnDataByID(2);
	cout << "INT C:" << c << endl;
	cout << "INT B: " << b << endl;*/
	

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
