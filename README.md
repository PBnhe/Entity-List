PT BR:
Este projeto se trata de uma lista encadeada de blocos , onde cada nó possui um array de entidades compostas por um ID e dados genéricos a serem definidos;
a lista tem o objetivo de ser cache friendly , permitindo operações e iteraçoes rápidas sob seus dados dispostos linearmente na memória , reduzindo o acesso de ponteiros dependendo da quantidade de entidades
por bloco,também contando com suporte a multithread para SIMD (Single Instruction Multiple Data) , aplicando funções genéricas sob todos os dados com divisão de carga entre os multiplos nucleos da CPU e comunicação
via serial com componentes externos ,microcontroladores por exemplo .
Essa lista possui um sistema de lazy deletion , apenas marcando os itens a deleção para futuramente reordenar os dados dentros dos blocos , formando um bloco inteiramente composto de lixo a ser remanejado.
portanto é essencial tomar cuidado com referencias diretas para dados e o tratamento de lixo da estrutura 

EN:
This project is about a linked list of blocks, where each node contains an array of entities composed of an ID and generic data to be defined;
the list aims to be cache friendly, allowing fast operations and iterations over its data arranged linearly in memory, reducing pointer access depending on the amount of entities
per block, also featuring multithread support for SIMD (Single Instruction Multiple Data), applying generic functions over all the data with load balancing between multiple CPU cores, and communication
via serial with external components, microcontrollers for example.
This list has a lazy deletion system, only marking the items for deletion to later reorder the data inside the blocks, forming a block entirely composed of trash to be relocated.
therefore, it is essential to be careful with direct references to data and with handling the trash in the structure.

🚧
Este projeto está em desenvolvimento ||||| This project is under development
🚧
