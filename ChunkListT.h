

#ifndef CHUNK_LIST
#define CHUNK_LIST
#include<iostream>
#include <emmintrin.h>
#include<cmath>
#include <cstdlib>
#include <new>


template<typename T>
class entity 
{
public:
	int id;
	T data;
};


template<typename T>
class ChunkList
{
private:




public:
	entity<T>* Data = nullptr; //ponteiro para vetor de entidade genérica
	ChunkList<T>* Next = nullptr;//ponteiro para próximo item da estrutura 

	ChunkList<T>* memInit(int size,size_t chunkSize=16); //size é o numero total de elementos
	
	void ShowDataDebug();

	static int N;   //inteiro que define os N elementos do vetor de dados 
	static int BLOCKS_N;
	

};



template<typename T>
int ChunkList<T>::BLOCKS_N = 0;



template<typename T>
int ChunkList<T>::N = 0;

template<typename T>
ChunkList<T>* ChunkList<T>::memInit(int size,size_t chunkSize)
{
	N = chunkSize; //definindo o tamanho do vetor da familia
	int N_BLOCK = size / N;
	int N_ARRAY_LAST = size % N;
	BLOCKS_N = N_BLOCK;

	//this = new ChunkList<T>;
	/*Data = new entity<T>[N];

	Next = new ChunkList<T>();

	ChunkList<T>* ATUAL = Next;*/

	/*ChunkList<T>* ATUAL = this;

	for (int i = 0; i < N_BLOCK; i++)
	{
		ATUAL->Next = new ChunkList<T>();
		ATUAL->Data = new entity<T>[N];



		ATUAL = ATUAL->Next;

	}

	

	ATUAL->Next = nullptr;
	return ATUAL;*/

	ChunkList<T>* ATUAL = this;
	ChunkList<T>* LAST_VALID = this;

	for (int i = 0; i < N_BLOCK; i++)
	{
		ATUAL->Data = new entity<T>[N];

		// só aloca próximo se ainda vai iterar
		if (i < N_BLOCK - 1)
		{
			ATUAL->Next = new ChunkList<T>();
			ATUAL = ATUAL->Next;
			LAST_VALID = ATUAL; 
		}
		else
		{
			ATUAL->Next = nullptr;
			LAST_VALID = ATUAL; 
		}
	}

	return LAST_VALID;





}











template<typename T>
void ChunkList<T>::ShowDataDebug()
{
	std::cout << "NUMERO DE INDICES VETOR: " << N << std::endl;
	std::cout << "NUMERO DE CHUNKS: " << BLOCKS_N << std::endl;


}




//template class ChunkList<int>;
// Especialização para `int`

#endif
