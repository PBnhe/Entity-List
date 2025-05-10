




//o objetivo dessa simples estrutura é remover a complexidade de N*N na validação de memoria 
//até agora usamos listas toDelete para verificar se uma referencia é ou nao valida
//comparando ela com todos os membros de toDelete, uma lista comum
//agora vamos usar a mesma lógica de mapeamento direto usada em arquitetura de cache ou hash tables
//cada membro desse array será um ponteiro para uma lista , que contem os ponteiros que podem ser remanejados
//onde o endreço será colocado será dado por ptr%ChunkList<T>::N
//usar threads
//deque ou list ou vector ou matriz
//bora
#ifndef D_MAP
#define D_MAP
#include<list>
#include"ChunkListT.h"

template<typename T>

class hashTable
{
private:
	std::list<entity<T> *>* Map = nullptr;
	int giveIndex(entity<T> * data);

public:
	void mapInsert(entity<T>* data); //mapeia e bota 
	void mapDelete(entity<T>* data);//remove
	bool hasAddress(entity<T>* data);//verifica existencia de endereço no mapa
	bool ifHasRemove(entity<T>* data); //se tem endereço retorna verdadeiro e o remove
	entity<T>* returnTrashAddress(entity<T>* basePointer,entity<T>* endPointer); //retorna endereço que nao esta em intervalo
	int returnSize();
	bool containsID(int id);
	hashTable();


	typename std::list<entity<T>*>::iterator findListPos(int index, entity<T>* data);

};
template <typename T>
hashTable<T>::hashTable()
{
	Map = new std::list<entity<T> *>[ChunkList<T>::N];
}

template<typename T>
void hashTable<T>::mapInsert(entity<T> * data)
{
	Map[giveIndex(data)].push_back(data);
	//aqui o dado foi mapeado na hash de acordo com o modulo do seu endereço 
}

template<typename T>
typename std::list<entity<T>*>::iterator hashTable<T>::findListPos(int index, entity<T>* data)
{
	std::list<entity<T>*>& target = Map[index];
	int size = target.size(); //operação o1 pelo contador interno da lista

	typename std::list<entity<T>*>::iterator it = target.begin();
	for (int i = 0; i < size; i++)
	{
		if (data == *it) //se a referencia para data é igual a aquilo apontador por it 
		{
			return it;
		}
		it++;
	}
	return target.end();
}


template <typename T>
void hashTable<T>::mapDelete(entity<T>* data)
{//pegar data e achar na lista seu referenciador

	int index = giveIndex(data);
	typename std::list<entity<T>*>::iterator it = findListPos(index, data);
	if (it != Map[index].end())
		Map[index].erase(it);

}

template<typename T>
int hashTable<T>::giveIndex(entity<T> * data)
{
	return reinterpret_cast<std::uintptr_t>(data) % ChunkList<T>::N;
}

template <typename T>
bool hashTable<T>::hasAddress(entity<T>* data)
{
	typename std::list<entity<T>*>::iterator it = findListPos(giveIndex(data), data);
	if (it != Map[giveIndex(data)].end())
	{
		return true;
	}
	return false;
}


template <typename T>
bool hashTable<T>::ifHasRemove(entity<T> * data)
{
	int index = giveIndex(data);
	typename std::list<entity<T>*>::iterator it = findListPos(index, data);
	if (it != Map[index].end())
	{
		Map[index].erase(it);
		return true;
	}
	return false;
}

template<typename T>
entity<T>* hashTable<T>::returnTrashAddress(entity<T>* basePointer, entity<T>* endPointer)
{
	for (int i = 0; i < ChunkList<T>::N; i++) 
	{
		if (!Map[i].empty())
		{
			typename std::list<entity<T>*>::iterator it = Map[i].begin();
			for (int j = 0; j < Map[i].size(); j++) 
			{
				if (*it<basePointer || *it>endPointer) 
				{
					entity<T>* temp = *it;
					Map[i].erase(it);
					return temp;
				}
				
				it++;
			}


		}
	}
	return nullptr;
}

template<typename T>
int hashTable<T>::returnSize() 
{
	int n = 0;
	for (int i = 0; i < ChunkList<T>::N; i++) 
	{
		n += Map[i].size();
	}
	return n;

}

template<typename T>
bool hashTable<T>::containsID(int id) 
{//como essa hash guarda referencias devemos iterar de **it em **it analisando igualdade
	for (int i = 0; i < ChunkList<T>::N; i++) 
	{
		if (!Map[i].empty()) 
		{
			auto it = Map[i].begin();
			auto end = Map[i].end();
			while (it != end) 
			{
				if ((*it)->id == id) 
				{
					return true;
				}
				it++;
			}

		}


	}
	return false;

}









#endif // !D_MAP
