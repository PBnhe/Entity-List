PT BR:
Este projeto se trata de uma lista encadeada de blocos , onde cada nó possui um array de entidades compostas por um ID e dados genéricos a serem definidos;
a lista tem o objetivo de ser cache friendly , permitindo operações e iteraçoes rápidas sob seus dados dispostos linearmente na memória , reduzindo o acesso de ponteiros dependendo da quantidade de entidades
por bloco,também contando com suporte a multithread para SIMD (Single Instruction Multiple Data) , aplicando funções genéricas sob todos os dados com divisão de carga entre os multiplos nucleos da CPU e comunicação
via serial com componentes externos ,microcontroladores por exemplo .
Essa lista possui um sistema de lazy deletion , apenas marcando os itens a deleção para futuramente reordenar os dados dentros dos blocos , formando um bloco inteiramente composto de lixo a ser remanejado.
portanto é essencial tomar cuidado com referencias diretas para dados e o tratamento de lixo da estrutura 

Sobre o serial:
O desing de datastream visa ser simples e genérico , tendo uma estrutura padrão de :
[uint8_t OPCODE] [uint8_t[N] PAYLOAD] [uint8_t CHECKSUM]
Ao receber um byte, o sistema espera que ele esteja mapeado e associado a um tamanho fixo de payload, uma função e parâmetros extras opcionais. Isso permite interpretar corretamente os bytes seguintes, completar a leitura da mensagem e, então, continuar a leitura com o próximo OPCODE.
Caso ocorra erro de bit, o CHECKSUM geralmente detecta a falha. Nessa situação, o sistema envia uma mensagem de ressincronização pela serial. O dispositivo remoto deve então responder com a sequência específica:
0x01 0x00 0x00 0x01
A falha de sincronização também pode ocorrer quando é lido um OPCODE não mapeado, pois o sistema perde a referência de quantos bytes deveria ler para completar a mensagem atual.


EN:
This project is about a linked list of blocks, where each node contains an array of entities composed of an ID and generic data to be defined;
the list aims to be cache friendly, allowing fast operations and iterations over its data arranged linearly in memory, reducing pointer access depending on the amount of entities
per block, also featuring multithread support for SIMD (Single Instruction Multiple Data), applying generic functions over all the data with load balancing between multiple CPU cores, and communication
via serial with external components, microcontrollers for example.
This list has a lazy deletion system, only marking the items for deletion to later reorder the data inside the blocks, forming a block entirely composed of trash to be relocated.
therefore, it is essential to be careful with direct references to data and with handling the trash in the structure.

About the serial:
The datastream design aims to be simple and generic, using a standard structure of:
[uint8_t OPCODE] [uint8_t[N] PAYLOAD] [uint8_t CHECKSUM]
Upon receiving a byte, the system expects it to be mapped and associated with a fixed payload size, a function, and optional extra parameters. This allows the system to correctly interpret the following bytes, complete the message reading, and then proceed with the next OPCODE.
If a bit error occurs, the CHECKSUM will usually detect the failure. In this situation, the system sends a resynchronization message through the serial port. The remote device must then respond with the specific sequence:
0x01 0x00 0x00 0x01
Desynchronization can also occur when an unmapped OPCODE is read, as the system loses reference to how many additional bytes it should read to complete the current message.


