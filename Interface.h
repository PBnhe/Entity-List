

#ifndef INTERFACE_H
#define INTERFACE_H
#include"ChunkListT.h"
#include <list>
#include "hashTable.h"
#include<functional>
#include<unordered_map>
#include "ThreadPool.h"
//problemas:
//deleção
//inserção
//leak de elementos vazios em arrays
//ToDO:
//abstrair GC pra um objeto
//criar ponteiro de acesso recente para usar nos métodos contains() e returnById
//^- reduz buscas na lista sem inserir nada no cache de leitura
//introduzir uma método genérico responsável por realizar operações de SIMD 
//trabalhar com entrada e saida de memória

enum { INITIALIZED,AUTOGARBAGE,READBUFFER };


struct entityConfig 
{
	size_t size = 1000;
	size_t chunkSize = 16;
	bool autoGarbageRoutine = true;
	size_t maxTrashBins = 2;
	bool enableReadBuffer = true;
	



};




template<typename T>
class Interface
{
private:
	ChunkList<T>* firstBlock = new ChunkList<T>;//objeto a ser gerenciado
	hashTable<T>* buffer = nullptr;
	
	ChunkList<T>* trashArray = nullptr;// array de lixo  que pode ser restaurado
	
	ChunkList<T>* lastBlock = nullptr;  //autoexplicativo :(
	ChunkList<T>* FirstIn = nullptr;  //para alocação rápida
	ChunkList<T>* recentAcess = nullptr; //um bloco de acesso recente, difere do buffer de leitura recente
	
	int toDeleteCount = 0;
	int  availableMemory = 0;  //numero de elementos que podem ser usados
	int firstInIndex = 0; //elemento do array do ponteiro firstin onde o dado será escrito
	int used = 0;  //elementos usados , contando com lixo
	int blocksCount = 0;  //contagem total de blocos do sistema

	bool FLAGS[3] = { 0 };

	hashTable<T>* deletano = nullptr;  //hash table personalizada
	std::unordered_map<int, T> readBuffer; //cache de leitura rápida
	
	std::function<bool(entity<T>)> reValidateTrash; //função a ser definida que opera sob bloco de lixo
	bool hasExtraVal = false;
	 short int trashBinCount = 0;
	 short int maxTrashBin = 0;

	 ThreadPool threads;

	 //AVISO : esses métodos são usados para  modificar todos os valores do vetor sem volta 
	 //caso deseje apenas modificar um valor ou certos valores faça questao de aplicar filtros
	 //ou utilize outros métodos  de escrita por ID para tal(reWriteByID) , CUIDADO!!!!
	 std::unordered_map<int, std::function<void(T&)>> ScalarFunctions;
	 std::unordered_map<int, std::function<void(T&)>> SIMDFunctions;

	void FirstBlockAlloc(T data,int id); // escreve data no index do bloco firstblock
	void AllocNewAndInsert(T data,int id); //aloca novo bloco de memoria e escreve data
	bool refValidation(entity<T>* data);//valida se tal endereço contem lixo
	bool addressIntervalValidation(entity<T>* basePointer, entity<T>* endPointer, entity<T>* comparePointer);//verifica se ponteiro está em intervalo
	void trashControlDelete();

	void trashControlTransfer();
	void firstBlockTrashControl();
	void nBlockTrashControl();
	void insertBlockInBuffer(ChunkList<T>* toCpy);
	int trashArrayOrder();
	void returncpyID_internal(int id, std::vector<ChunkList<T>*> vector, std::atomic<bool> & found, T & data, std::mutex & data_lock,std::atomic<int> & threadcount);
	
	void DoScalar_internal(int F_id,std::vector<ChunkList<T>*> vector, std::atomic<int>& threadcount);
public:
	void initialize(entityConfig config)
	{
		if (!FLAGS[INITIALIZED])
		{

			lastBlock = firstBlock->memInit(config.size, config.chunkSize);

			availableMemory = ChunkList<T>::N * ChunkList<T>::BLOCKS_N;
			FirstIn = firstBlock;
			blocksCount = ChunkList<T>::BLOCKS_N;
			deletano = new hashTable<T>();
			buffer = new hashTable<T>();
			trashArray = new ChunkList<T>[ChunkList<T>::N];

			for (int i = 0; i < config.maxTrashBins; i++)
			{
				trashArray[i].Data = new entity<T>[ChunkList<T>::N];
			}
			maxTrashBin = config.maxTrashBins;
			FLAGS[INITIALIZED] = true;
			FLAGS[AUTOGARBAGE] = config.autoGarbageRoutine;
			FLAGS[READBUFFER] = config.enableReadBuffer;
		}

	}
	
	void ShowDataDebug() { firstBlock->ShowDataDebug(); }
		
	void insertData(int id , T data);
	void removeDataIndex(int n);  //método existe apenas para debug
	void removeDataWhere(T data);
	void trashControl(); // TA RETORNANDO NA PRIMEIRA LINHA AVISO!!!!!!!!!
	void dataD();
	void giveTrashVal(std::function<bool(entity<T>*)> func);
	entity<T> returnCopyByIndex(int index);
	//MÉTODOS DE USUÁRIO--------------------------------------------------------------------
	T returnDataByID(int id);
	T retunrDataByID_Thread(int id);
	void removeDataByID(int id);

	int available();//retorna elementos disponiveis para escrita de dados nos blocos atuais
	int memUsed();//retorna referencias usadas
	int validUsed(); //retorna as referencias validas que estao sendo usadas (used - to delete)

	bool containsID(int id);//verifica se id existe na lista
	bool toDeleteContains(int id);//verifica se o map personalizado tem tal id 
	bool trashBinContains(int id);//verifica se a lixeira tem tal id

	void restoreID(int id);//restuara id de lista de lixo /lixeira
	void reWriteID(const int id, T data);//reescreve conteudo de id
	void insertScalarFunction(int f_ID, std::function<void(T&)> toInsert);
	void insert_SIMD_Function(int f_ID, std::function<void(T)> toInsert);

	void DoScalar(int id);//realiza uma das funções escalares inseridas previamente
	void DoScalar_semThread(int id);




};

template<typename T>
void Interface<T>::giveTrashVal(std::function<bool(entity<T>*)> func) 
{
	reValidateTrash = func;
	hasExtraVal = true;
}


template<typename T>
void Interface<T>::removeDataIndex(int n)
{
	//aqui precisamos percorrer os blocos , sem necessáriamente percorrer os vetores de dados
	//vantagi
	//o calculo de quantos blocos precisamos percorrer é feito através de n/N para o bloco e
	//n%N para o elemento do array
	int block_iterator = n / ChunkList<T>::N;
	int arrayData = n % ChunkList<T>::N;

	ChunkList<T>* Current = firstBlock;

	for (int i = 0; i < block_iterator; i++)
	{
		Current = Current->Next;
		if (!Current)
			return;

	}
	if (!deletano->hasAddress(&Current->Data[arrayData]))//verifica de a referncia já nao é lixo
	{
		//trashControl();
		//toDelete.push_back(& Current->Data[arrayData]);
		deletano->mapInsert(&Current->Data[arrayData]);
		toDeleteCount++;
		if (FLAGS[AUTOGARBAGE]) {trashControl();}
	}
}

template<typename T>
void Interface<T>::removeDataWhere(T data)
{

	//precisamos saber até que ponto a pool foi usada
	//para isso temos used/N para quantidade de blocos
	//e para o array do ultimo bloco temos used
	//precisamos saber quantos blocos completos o  iterador vai percorrer
	//antes de chegar ao bloco incompleto

	int numblock = used / ChunkList<T>::N;
	int narray = used % ChunkList<T>::N;

	ChunkList<T>* current = firstBlock;


	//no caso se used =1 o iterador nao vai fazer nada
	//já que 0 blocos completos foram usados e 0 nao é menor que 0
	for (int i = 0; i < numblock; i++)
	{
		for (int j = 0; j < ChunkList<T>::N; j++)
		{
			if (current->Data[j] == data)
			{
				if (!deletano->hasAddress(&data))//verifica se a referencia já nao é lixo
				{
					trashControl();
					//toDelete.push_back(&current->Data[j]);
					toDeleteCount++;
					deletano->mapInsert(&current->Data[j]);
					trashControl();

				}
			}
		}
		current = current->Next;
	}

	//itera somente num bloco incompleto de used
	if (narray != 0)
	{
		for (int k = 0; k < narray; k++)
		{
			if (current->Data[k] == data)
			{
				if (refValidation(&data))//verifica se a referencia já nao é lixo
				{
					trashControl();
					deletano->mapInsert(&current->Data[k]);
					toDeleteCount++;
					trashControl();

				}
			}
		}
	}

}

template<typename T>
void Interface<T>::insertData(int id,T data)
{
	

	if (availableMemory > 0)
	{
		FirstBlockAlloc(data,id);
		used++;
		return;
	}

	//já que não nos restam alternativas , só sobra ao betinha alocar um novo chunk :(
	//sendo assim vamos atualizar a memória disponivel , essa que deve estar estritamente ZERADA para chegar aqui

	AllocNewAndInsert(data,id);
	used++;
	blocksCount++;
	return;



}



template<typename T>
void Interface<T>::FirstBlockAlloc(T data,int id)
{

	if (firstInIndex >= ChunkList<T>::N)
	{
		if (FirstIn->Next)
		{
			
			FirstIn = FirstIn->Next;
			firstInIndex = 0;
		}
	}
	

	FirstIn->Data[firstInIndex].data = data;
	FirstIn->Data[firstInIndex].id = id;
	

	firstInIndex++;
	availableMemory--;

	
}

template<typename T>
void Interface<T>::AllocNewAndInsert(T data,int id)
{
	//std::cout << "ALLOCNEW FLAG" << std::endl;
	ChunkList<T>* temp;
	temp = new ChunkList<T>;
	temp->Data = new entity<T>[ChunkList<T>::N];
	

	if (FirstIn == lastBlock) {
		std::cout << "true";
	}

	temp->Next = nullptr;
	lastBlock->Next = temp;
	lastBlock = temp;
	availableMemory = availableMemory + ChunkList<T>::N;
	//FirstIn->Next = temp;


	//se não sobrou nada , então firstIn agora é o ultimo bloco 
	FirstIn = lastBlock;
	firstInIndex = 0;
	//e sabe oque usa o ultimo bloco pra alocar? nem te conto
	FirstBlockAlloc(data,id);

}


template<typename T>
void Interface<T>::dataD()
{
	std::cout << "-----------------------------------------------------------------" << '\n';
	std::cout << "ELEMENTOS DISPONIVEIS: " << std::endl;
	std::cout << availableMemory << std::endl;
	std::cout << "TO DELETE COUNT:" << std::endl;
	std::cout << toDeleteCount << std::endl;
	std::cout << "VENDO SE REESCREVEU A MEMORIA MSM" << std::endl;
//	std::cout << firstBlock->Data[0] << std::endl;
	std::cout << " TAMANHO DO HASH MAP TO DELETE" << std::endl;
	std::cout << deletano->returnSize() << std::endl;
	std::cout << "TAMANHO N " << std::endl;
	std::cout << ChunkList<T>::N<<std::endl;
	std::cout << "-----------------------------------------------------------------" << '\n';



}

template<typename T>
bool Interface<T>::refValidation(entity<T>* data)
{
	/*   //a função desse método é validar se a referencia que foi passada a ele está na lista de deleção
	   //comparando o endereço inserido com cada um dos endereços da lista


	   //agora precisamos pegar o iterador interno da lista que aponta ao primeiro endereço do container
	   typename std::list<T*>::iterator it = toDelete.begin();
	   //agora percorrer a lista comparando o endereço apontado por it ou seja it* com data
	   //enquanto it for diferente de todelete.end();
	   while (it != toDelete.end())
	   {
		   if (data == *it) //caso o o ponteiro para T data for igual ao endereço apontado pelo iterador it retorne false
		   {
			   return false;
		   }
		   it++;
	   }

	   return true;
	   */


	if (deletano->hasAddress(data)) {
		return false;
	}
	return true;


}


template<typename T>
void Interface<T>::trashControl()
{
	
	//o controle de lixo deve ser realizado para tirar o stress da memoria e manter a quantidade de blocos atuais
	//proxima a quantidade de blocos requisitada já que o programa nao barra inserções que ultrapassam o exigido
	//caso a quantidade de elementos em toDelete for igual ao tamanho do array interno de cada bloco
	//primeiro uma decisao deve ser tomada de acordo com a quantidade de blocos existentes e a requisitada
	//caso o numero de blocos existentes ultrapasse o originalmente requisitado o bloco formado pelos N lixos
	//deverá ser deletado
	//caso contrario , basta colocar esse bloco de memória na ponta 
	//em ambos a variável used devera ser used-=N
	if (toDeleteCount < ChunkList<T>::N)
		return;

	if (blocksCount > ChunkList<T>::BLOCKS_N) //numero de blocos atual é maior que o requisitado = bloco precisa ser deletado
	{

		trashControlDelete();


		return;
	}

	//--------------------------------------------------------------------------------------------------

	trashControlTransfer();
	return;



}

template<typename T>
entity<T> Interface<T>::returnCopyByIndex(int index)
{
	int block_iterator = index / ChunkList<T>::N;
	int arrayData = index % ChunkList<T>::N;

	ChunkList<T>* Current = firstBlock;

	for (int i = 0; i < block_iterator; i++)
	{
		Current = Current->Next;
		if (!Current)
		{
			std::cout << "INDICE ATUAL É LIXO OU NULO OU NAO EXISTE" << std::endl;
			return entity<T>();
		}
	}
	if (refValidation(&Current->Data[arrayData]))//verifica de a referncia já nao é lixo
	{
		return Current->Data[arrayData];
	}

	std::cout << "INDICE ATUAL É LIXO OU NULO OU NAO EXISTE" << std::endl;
	return entity<T>();
}


template<typename T>
bool Interface<T>::addressIntervalValidation(entity<T>* basePointer, entity<T>* endPointer, entity<T>* comparePointer)
{


	if (comparePointer >= basePointer && comparePointer < endPointer)
	{
		return false;//endereço está no intervalo e não deve ser usado
	}

	return true;


}

template<typename T>
void Interface<T>::trashControlDelete() 
{

	//pegar current , um bloco cheio
	ChunkList<T>* current = firstBlock;
	ChunkList<T>* connector = firstBlock;
	while (current->Next != FirstIn) 
	{
		current = current->Next;
	}
	//pegamos bloco cheio
	//pegar connector
	while (connector->Next != current) 
	{
		connector = connector->Next;
	}
	//validar ref ->validar alvo-> trocar -> deleção
	//caso um dado for escrito na ponta no processo vamos zerar sua memoria para nao dar conflito com métodos futuros
	for (int i = 0; i < ChunkList<T>::N; i++) 
	{

		//ver se ref nao é lixo
		if (deletano->ifHasRemove(&current->Data[i])) 
		{
			continue; //se tem endereço , delete e pule a proxima iteração //para evitat encadeamentos de if else
		}
		//tenta encontrar endereço válido para copiar dados

		entity<T>* target = deletano->returnTrashAddress(current->Data, current->Data + ChunkList<T>::N);
		if (target) 
		{
			entity<T> temp = *target; //temp recebe conteudo de target
			*target = current->Data[i]; // conteudo de target recebe dados
			current->Data[i] = temp; //conteudo reescrito por lixo

		}
		if (!target) {
			insertData(current->Data[i].id,current->Data[i].data); //seguindo a lógica do programa para remanejar lixo deve existir um bloco completo de lixos para
		} //remanejar , mas só por segurança

	}
	//agora o vetor data de current está completo de dados lixo podemos aplicar uma unica verificação sob cada um dos current
	//se o método a ser definido retornar verdadeiro , o dado será inserido na ponta

	if (hasExtraVal) 
	{
		for (int i = 0; i < ChunkList<T>::N; i++) 
		{
			if (reValidateTrash(current->Data[i])) {
				insertData(current->Data[i].id, current->Data[i].data);
			}
		}
	}
	//aqui temos um bloco completamente validado, agora vamos inserir esses blocos em um dos buffers de lixo

	
    
	std::memcpy(trashArray[trashArrayOrder()].Data, current->Data, sizeof(entity<T>) * ChunkList<T>::N);
	trashBinCount++;

	//copiamos current a ao buffer de lixo
	//deletar o bloco atual e conectar o anterior ao proximo
	connector->Next = current->Next;
	delete[] current->Data;
	delete current;
	used -= ChunkList<T>::N;
	blocksCount--;
	toDeleteCount -= ChunkList<T>::N;
	return;

}

template<typename T>
void Interface<T>::trashControlTransfer() 
{
	ChunkList<T>* current = firstBlock;
	while (current->Next != FirstIn)
	{
		current = current->Next;
	}
	if (current == firstBlock) 
	{
		//o bloco a ser remanejado é o primeiro bloco da lista
		firstBlockTrashControl();
		return;
	}
	//o bloco a ser remanejado não é o primeiro bloco da lista
	nBlockTrashControl();
	return;



}

template<typename T>
void Interface<T>::firstBlockTrashControl() 
{
	//o código só entra nesse método caso o bloco a ser remanejado seja o primeiro bloco da lista
	ChunkList<T>* current = firstBlock;
	for (int i = 0; i < ChunkList<T>::N; i++)
	{

		//ver se ref nao é lixo
		if (deletano->ifHasRemove(&current->Data[i]))
		{
			continue; //se tem endereço , delete e pule a proxima iteração //para evitat encadeamentos de if else
		}
		//tenta encontrar endereço válido para copiar dados

		entity<T>* target = deletano->returnTrashAddress(current->Data, current->Data + ChunkList<T>::N);
		if (target)
		{
			entity<T> temp = *target; //temp recebe conteudo de target
			*target = current->Data[i]; // conteudo de target recebe dados
			current->Data[i] = temp; //conteudo reescrito por lixo

		}
		if (!target) {
			insertData(current->Data[i].id,current->Data[i].data); //seguindo a lógica do programa para remanejar lixo deve existir um bloco completo de lixos para
		} //remanejar , mas só por segurança

	}
	//agora o vetor data de current está completo de dados lixo podemos aplicar uma unica verificação sob cada um dos current
	//se o método a ser definido retornar verdadeiro , o dado será inserido na ponta

	if (hasExtraVal)
	{
		for (int i = 0; i < ChunkList<T>::N; i++)
		{
			if (reValidateTrash(current->Data[i])) {
				insertData(current->Data[i].id, current->Data[i].data);
			}
		}
	}
	//aqui temos um bloco completamente validado, agora vamos inserir esses blocos em um dos buffers de lixo

	if (trashBinCount == 1)
		trashBinCount = 0;

	std::memcpy(trashArray[trashArrayOrder()].Data, current->Data, sizeof(entity<T>) * ChunkList<T>::N);
	trashBinCount++;
	//a diferença desse método é que jogamos o bloco atual para o final da lista
	//aumentamos a memoria disponivel já que há um bloco a ser reescrito
	//ai mas ent tudo isso devia virar função 
	//nao ligo KKKKKKKKKKK

	//se o a variável estática deve receber o proximo bloco e e o atual deve ir ao começo da lista , assim como se tornar o ponteiro a ultimo bloco
	firstBlock = current->Next;
	current->Next = nullptr;
	//o primeiro bloco agora é o segundo
	//current agora está solto sem algo que o referencie 
	//então vamos colocar current no final da lista
	lastBlock->Next = current;
	lastBlock = current;
	used -= ChunkList<T>::N;
	availableMemory += ChunkList<T>::N;
	toDeleteCount -= ChunkList<T>::N;
	return;
}

template<typename T>
void Interface<T>::nBlockTrashControl() 
{
	//entra caso não seja o primeiro bloco da lista
	//nesse caso precisamos de connector 
	ChunkList<T>* current = firstBlock;
	ChunkList<T>* connector = firstBlock;
	while (current->Next != FirstIn)
	{
		current = current->Next;
	}
	while (connector->Next != current)
	{
		connector = connector->Next;
	}

	for (int i = 0; i < ChunkList<T>::N; i++)
	{

		//ver se ref nao é lixo
		if (deletano->ifHasRemove(&current->Data[i]))
		{
			continue; //se tem endereço , delete e pule a proxima iteração //para evitat encadeamentos de if else
		}
		//tenta encontrar endereço válido para copiar dados

		entity<T>* target = deletano->returnTrashAddress(current->Data, current->Data + ChunkList<T>::N);
		if (target)
		{
			entity<T> temp = *target; //temp recebe conteudo de target
			*target = current->Data[i]; // conteudo de target recebe dados
			current->Data[i] = temp; //conteudo reescrito por lixo

		}
		if (!target) 
		{
			insertData(current->Data[i].id, current->Data[i].data); //seguindo a lógica do programa para remanejar lixo deve existir um bloco completo de lixos para
		} //remanejar , mas só por segurança

	}
	//agora o vetor data de current está completo de dados lixo podemos aplicar uma unica verificação sob cada um dos current
	//se o método a ser definido retornar verdadeiro , o dado será inserido na ponta

	if (hasExtraVal)
	{
		for (int i = 0; i < ChunkList<T>::N; i++)
		{
			if (reValidateTrash(current->Data[i])) {
				insertData(current->Data[i].id,current->Data[i].data);
			}
		}
	}
	//aqui temos um bloco completamente validado, agora vamos inserir esses blocos em um dos buffers de lixo

	if (trashBinCount == 1)
		trashBinCount = 0;

	std::memcpy(trashArray[trashArrayOrder()].Data, current->Data, sizeof(entity<T>) * ChunkList<T>::N);
	trashBinCount++;
	//a diferença desse método para o outro é que firstblock continua sendo ele mesmo
	

	connector->Next = current->Next;//current solto em mem
	lastBlock->Next = current;
	lastBlock = current;
	current->Next = nullptr;

	used -= ChunkList<T>::N;
	availableMemory += ChunkList<T>::N;
	toDeleteCount -= ChunkList<T>::N;
	return;

}

template<typename T>
T Interface<T>::returnDataByID(int id) 
{
	//primeiro procurar no buffer se há bloco compativel
	
	if (readBuffer.count(id)) 
	{
		return	readBuffer.at(id);
		//se o id existir no buffer retorne
	}
	

	//pegar numero de blocos que vamos e iterar e se terá um proximo com base em used
	int n_blocks = used / ChunkList<T>::N;
	int n_array = used % ChunkList<T>::N;
	ChunkList<T>* current = firstBlock;
	int debg = 0;
	//std::cout << "NBLOCKS" << n_blocks<<std::endl;
	//std::cout << "NARRAY" << n_array<<std::endl;
	
	
	for (int i = 0; i < n_blocks; i++) 
	{
		for (int j = 0; j < ChunkList<T>::N; j++) 
		{
			if (current->Data[j].id == id) 
			{
				if (refValidation(&current->Data[j])) 
				{
					readBuffer.clear();
					//colocar bloco em buffer
					insertBlockInBuffer(current);
					return current->Data[j].data;
				}
			}


		}
		current = current->Next;
		 debg = i;
	}
	std::cout << "DEBUG INT:"<<debg<<std::endl;
	
	if (n_array != 0)
	{
	
		for (int k = 0; k < n_array; k++)
		{
			
			if (current->Data[k].id == id)
			{
				
				if (refValidation(&current->Data[k]))
				{
					readBuffer.clear();
					insertBlockInBuffer(current);
					return current->Data[k].data;
				}
			}
		}
	}
	
	return T();//num acho 
	
}


template<typename T>
void Interface<T>::insertBlockInBuffer(ChunkList<T>* tocpy) 
{//recebe bloco e bota no buffer ain
	for (int i = 0; i < ChunkList<T>::N; i++) 
	{
		if (refValidation(&tocpy->Data[i]))
		{
			readBuffer.emplace(tocpy->Data[i].id, tocpy->Data[i].data);
		}
	}
}


template <typename T>
void Interface<T>::removeDataByID(int id) 
{
	//remover do buffer de leitura caso haja compatível
	if (readBuffer.count(id)) 
	{
		readBuffer.erase(id);
	}
	int n_blocks = used / ChunkList<T>::N;
	int n_array = used % ChunkList<T>::N;
	ChunkList<T>* current = firstBlock;

	for (int i = 0; i < n_blocks; i++)
	{
		for (int j = 0; j < ChunkList<T>::N; j++)
		{
			if (current->Data[j].id == id)
			{
				if (refValidation(&current->Data[j]))
				{
					deletano->mapInsert(&current->Data[j]);
					toDeleteCount++;
					if (FLAGS[AUTOGARBAGE]) { trashControl(); }
					
				}
			}


		}
		if (current)
		{
			current = current->Next;
		}

	}
	//agora iterar sob bloco extra 
	if (n_array != 0)
	{
		for (int k = 0; k < n_array; k++)
		{
			if (current->Data[k].id == id)
			{
				if (refValidation(current->Data[k]))
				{
					deletano->mapInsert(&current->Data[k]);
					toDeleteCount++;
					if (FLAGS[AUTOGARBAGE]) { trashControl(); }
					

				}
			}
		}
	}
	
}

template<typename T>
int Interface<T>::available() 
{
	return availableMemory;
}
template<typename T>
int Interface<T>::memUsed() 
{
	return used;
}
template <typename T>
int Interface<T>::validUsed() {
	return used - toDeleteCount;
}
template<typename T>
bool Interface<T>::containsID(int id) 
{//primeiro checar se o buffer tem , se nao loop
	if (readBuffer.count(id)) {
		return true;// o readbuffer contem apenas referencias válidas
		//se algo está ali ele também está na lista
	}

	int n_blocks = used / ChunkList<T>::N;
	int n_array = used % ChunkList<T>::N;
	ChunkList<T>* current = firstBlock;
	if (recentAcess)
	{
		for (int j = 0; j < ChunkList<T>::N; j++)
		{
			if (recentAcess->Data[j].id == id)
			{
				if (refValidation(&recentAcess->Data[j]))
				{
					recentAcess = recentAcess;
					return true;

				}
			}


		}
	}

	for (int i = 0; i < n_blocks; i++)
	{
		for (int j = 0; j < ChunkList<T>::N; j++)
		{
			if (current->Data[j].id == id)
			{
				if (refValidation(&current->Data[j]))
				{
					recentAcess = current;
					return true;

				}
			}


		}
		if (current->Next)
		{
			current = current->Next;
		}

	}
	//agora iterar sob bloco extra 
	if (n_array != 0)
	{
		for (int k = 0; k < n_array; k++)
		{
			if (current->Data[k].id == id)
			{
				if (refValidation(&current->Data[k]))
				{
					recentAcess = current;
					return true;
				}
			}
		}
	}
	return false;

}

template<typename T>
bool Interface<T>::toDeleteContains(int id) 
{//verificar se no deletano há um conteudo compatível com o id passado
	return deletano->containsID(id);
}

template<typename T>
bool Interface<T>::trashBinContains(int id) 
{//verificar em todos os arrays de lixo se há um compativel

	for (int i = 0; i < trashBinCount; i++) 
	{
		for (int j = 0; j < ChunkList<T>::N; j++) 
		{
			if (trashArray[i].Data[j].id == id)
			{
				return true;
			}
		}
	}
	return false;

}

template<typename T>
int Interface<T>::trashArrayOrder() 
{
	
	if (trashBinCount >= maxTrashBin) 
	{
		trashBinCount = 0;
	}
	return trashBinCount;
}

template <typename T>
void Interface<T>::restoreID(int id)
{
	if (deletano->ifHasRemove(id)) {
		return;
	}
	//se a referencia está no hash então ela ainda está na lista , apenas indicamos que esse endereço é valid

		for (int i = 0; i < trashBinCount; i++)
		{
			for (int j = 0; j < ChunkList<T>::N; j++)
			{
				if (trashArray[i].Data[j].id == id)
				{
					insertData(trashArray[i].Data[j].id,trashArray[i].Data[j].data);
				}
			}
		}
	
}

template<typename T>
void Interface<T>::reWriteID(int id, T data) 
{
	int n_blocks = used / ChunkList<T>::N;
	int n_array = used % ChunkList<T>::N;
	ChunkList<T>* current=firstBlock;
	//primeiro procura em acesso recente
	if (recentAcess)
	{
		for (int j = 0; j < ChunkList<T>::N; j++)
		{
			if (recentAcess->Data[j].id == id)
			{
				if (refValidation(&recentAcess->Data[j]))
				{
					
					recentAcess->Data[j].data = data;
					if (readBuffer.count(id))
					{
						readBuffer[id] = data;
					}
					return;
				}
			}
		}
	}

	//se nao achou itera

	for (int i = 0; i < n_blocks; i++)
	{
		for (int j = 0; j < ChunkList<T>::N; j++)
		{
			if (current->Data[j].id == id)
			{
				if (refValidation(&current->Data[j]))
				{
					recentAcess = current;
					current->Data[j].data = data;
					if (readBuffer.count(id))
					{
						readBuffer[id] = data;
					}
					return;
					

				}
			}


		}
		if (current->Next)
		{
			current = current->Next;
		}

	}
	//agora iterar sob bloco extra 
	if (n_array != 0)
	{
		for (int k = 0; k < n_array; k++)
		{
			if (current->Data[k].id == id)
			{
				if (refValidation(&current->Data[k]))
				{
					recentAcess = current;
					current->Data[k].data = data;
					if (readBuffer.count(id))
					{
						readBuffer[id] = data;
					}
					return;
				}
			}
		}
	}
	

}

template<typename T>
void Interface<T>::insertScalarFunction(int f_ID, std::function<void(T&)> toInsert) 
{
	ScalarFunctions[f_ID] = toInsert;
}

template<typename T>
T Interface<T>::retunrDataByID_Thread(int id)
{//o método deve separar a carga de trabalho entre várias threads de acordo com o numero de threads
int n_threads =	ThreadPool::N_threads;
//agora que temos o numero de threads vamos calcular quantos blocos cada uma vai iterar sobre
int nblock = used / ChunkList<T>::N;
int lasblock = used % ChunkList<T>::N;

int n_each = nblock / n_threads;
int n_left = nblock % n_threads;
std::atomic<bool> found=false;
std::mutex data_mutex;
std::atomic<int> threads_end_count = n_threads;
ChunkList<T>* current = firstBlock;
T  data;
 for (int j = 0; j < n_threads; j++)
 {
	std::vector<ChunkList<T>*> address;
	for (int i = 0; i < n_each ; i++)
	{
		address.push_back(current);
		current = current->Next;
	}
	/*threads.enqueueTask([=, &found, &data,&data_mutex,&threads_end_count]() {
		returncpyID_internal(id, address, found, data,data_mutex,threads_end_count);
		});*/
	auto task_blocks = std::move(address);
	threads.enqueueTask([=, &found, &data, &data_mutex, &threads_end_count]()  {
		returncpyID_internal(id, task_blocks, found, data, data_mutex, threads_end_count);
		});
 }

 while (!found&&threads_end_count!=0) 
 {
	 std::this_thread::yield();
 }
 //ao chegar aqui todos as threads devem ter acabado
 //restando os blocos que não foram inseridos em threads de forma escalar
 for (int i = 0; i < n_left; i++) 
 {
	 for (int j = 0; j < ChunkList<T>::N; j++) 
	 {
		 if (current->Data[j].id == id) 
		 {
			 if (refValidation(&current->Data[j])) 
			 {
				 insertBlockInBuffer(current);
				 data = current->Data[j].data;
			 }
		 }
	 }
	 current = current->Next;
 }
 if (lasblock != 0) 
 {
	 for (int i = 0; i < ChunkList<T>::N; i++) 
	 {
		 if (current->Data[i].id == id)
		 {
			 if (refValidation(&current->Data[i]))
			 {
				 insertBlockInBuffer(current);
				 data = current->Data[i].data;
			 }
		 }

	 }
 }


 return data;

}


template<typename T>
void Interface<T>::returncpyID_internal(int id, std::vector<ChunkList<T>*> vector,  std::atomic<bool> & found,T & data, std::mutex& data_lock,std::atomic<int> & threadcount)
{
	int size = vector.size();
	ChunkList<T>* current = nullptr;
	for (int i = 0; i < size; i++) 
	{
		if (found) 
		{
			return;
		}
		current = vector[i];
		for (int j = 0; j < ChunkList<T>::N; j++) 
		{
			if (current->Data[j].id == id) 
			{
				
				if (refValidation(&current->Data[j])) 
				{
					data = current->Data[j].data;
					found = true;
					insertBlockInBuffer(current);
					return;

				}
			}
		}
	}
	threadcount.fetch_sub(1, std::memory_order_relaxed);

}


template<typename T>
void Interface<T>::DoScalar(int id)
{//o método deve separar a carga de trabalho entre várias threads de acordo com o numero de threads
	int n_threads = ThreadPool::N_threads;
	//agora que temos o numero de threads vamos calcular quantos blocos cada uma vai iterar sobre
	int nblock = used / ChunkList<T>::N;
	int lasblock = used % ChunkList<T>::N;

	int n_each = nblock / n_threads;
	int n_left = nblock % n_threads;
	
	//std::mutex data_mutex;
	std::atomic<int> threads_end_count = n_threads;
	ChunkList<T>* current = firstBlock;
	
	std::function<void(T&)> task;
	if (ScalarFunctions.count(id)) 
	{
		task = ScalarFunctions.at(id);
	}
	if (!ScalarFunctions.count(id)) 
	{
		return;
	}

	for (int j = 0; j < n_threads; j++)
	{
		std::vector<ChunkList<T>*> address;
		for (int i = 0; i < n_each; i++)
		{
			address.push_back(current);
			current = current->Next;
		}
		/*threads.enqueueTask([=, &found, &data,&data_mutex,&threads_end_count]() {
			returncpyID_internal(id, address, found, data,data_mutex,threads_end_count);
			});*/
		auto task_blocks = std::move(address);
		threads.enqueueTask([=, &threads_end_count]() 
			{
			DoScalar_internal(id, task_blocks, threads_end_count); 
			});
	}

	while (threads_end_count != 0)
	{
		std::this_thread::yield();
	}
	//ao chegar aqui todos as threads devem ter acabado
	//restando os blocos que não foram inseridos em threads de forma escalar
	for (int i = 0; i < n_left; i++)
	{
		for (int j = 0; j < ChunkList<T>::N; j++)
		{
			//EXECUTA FUNÇÃO C DATA
			task(current->Data[j].data);
		}
		current = current->Next;
	}
	if (lasblock != 0)
	{
		for (int i = 0; i < ChunkList<T>::N; i++)
		{
			//executa função c data
			task(current->Data[i].data);
		}
	}
}


template<typename T>
void Interface<T>::DoScalar_internal(int F_id,std::vector<ChunkList<T>*> vector, std::atomic<int> & threadcount)
{
	std::function<void(T&)> task = ScalarFunctions.at(F_id);
	int size = vector.size();
	ChunkList<T>* current = nullptr;
	for (int i = 0; i < size; i++)
	{
		
		current = vector[i];
		for (int j = 0; j < ChunkList<T>::N; j++)
		{
			
			task(current->Data[j].data);

				//realiza função
			
		}
	}
	threadcount.fetch_sub(1, std::memory_order_acq_rel);

}

template<typename T>
void Interface<T>::DoScalar_semThread(int id) 
{
	//pegar numero de blocos que vamos e iterar e se terá um proximo com base em used
	int n_blocks = used / ChunkList<T>::N;
	int n_array = used % ChunkList<T>::N;
	ChunkList<T>* current = firstBlock;
	
	//std::cout << "NBLOCKS" << n_blocks<<std::endl;
	//std::cout << "NARRAY" << n_array<<std::endl;
	std::function<void(T&)> task;
	if (ScalarFunctions.count(id))
	{
		task = ScalarFunctions.at(id);
	}
	if (!ScalarFunctions.count(id))
	{
		return;
	}


	for (int i = 0; i < n_blocks; i++)
	{
		for (int j = 0; j < ChunkList<T>::N; j++)
		{
			task(current->Data[j].data);
		}
		current = current->Next;
		
	}
	

	if (n_array != 0)
	{

		for (int k = 0; k < n_array; k++)
		{

			if (current->Data[k].id == id)
			{
				task(current->Data[k].data);
			}
		}
	}



}



#endif // !
