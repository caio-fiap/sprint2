#include <stdio.h>
#include <string.h>
#include <time.h>
//#include <unistd.h>
#include <stdlib.h>
#define LIMITE_POTENCIA  50.0

struct Carro{
    char placa[8];
    int ano;
    float bateria; //Em kW
    int porcentagem_bateria;
};

struct Vaga{
    int status; // 0 para vaga livre, 1 para ocupada
    float potencia_atual; //Potencia que o carregador está entregado, ela pode variar dependendo da quantidade de carregadores conectados 
    time_t hora_inicio;
    float energia_consumida; 
    float custo_total;
    struct Carro carro; 
};

void conectar_veiculo(struct Vaga vagas[]){
    int vaga_encontrada = -1;
    int i;
    for(i = 0; i < 5; i++){
        if(vagas[i].status == 0){
            vaga_encontrada = i;
            break;
        }
    }
    if(vaga_encontrada == -1){
        printf("Nenhuma vaga disponivel...\n");
    } else{
        printf("Vaga %d esta disponivel\n", vaga_encontrada);
        printf("Digite a placa do carro; ");
        fgets(vagas[vaga_encontrada].carro.placa, 8, stdin);
        while(getchar() != '\n');
        printf("\n");
        printf("Digite a porcentagem atual da bateria do carro: ");
        scanf("%d", &vagas[vaga_encontrada].carro.porcentagem_bateria);
        printf("\n");
        printf("Digite a potencia da bateria (em kW) do carro: ");
        scanf("%f", &vagas[vaga_encontrada].carro.bateria);


    }

}


int main(){
    system("clear"); //Comando para limpar o terminal assim que o program iniciar
    struct Vaga vagas[5];// 5 é o número de vagas 
    int opcao;
    for(int i = 0; i < 5; i++){ //Inicializa as variáveis de struct Vaga em 0, menos horario inicio. 
        vagas[i].status = 0;
        vagas[i].potencia_atual = 0;
        vagas[i].energia_consumida = 0;
        vagas[i].custo_total = 0;
    }
    do{
        printf("====== ChargeGrid Inteligence ======\n"); //Menu de funcionamento
        printf("Bem vindo!\n");
        printf("Digite uma das opcoes abaixo\n");
        printf("1 - Conectar veiculo\n");
        printf("2 - desconectar veiculo\n");
        printf("3 - ver status das vagas\n");
        printf("4 - ver relatorio\n");
        printf("5 - Simular envio OCPP\n");
        printf("0 - Encerrar o programa\n");
        printf("Resposta: ");
        scanf("%d", &opcao);
        printf("\n");

        switch(opcao){ //O loop repete até que a decisão do usuário seja 0
            case 1:
            printf("Opcao 1, conectar veiculo, selecionada\n");
            break;

            case 2: 
            printf("Opcao 2, deconectar veiculo, selecionada\n");
            break;

            case 3:
            printf("Opcao 3, ver status das vagas, selecionada\n");
            break;

            case 4: 
            printf("Opcao 4, ver relatorio, selecionada\n");
            break;

            case 5: 
            printf("Opcao 5, simular envio OCPP, selecionada\n");
            break;

            default: printf("Opcao invalida... tente novamente\n");
            break;
        }

    }while(opcao != 0);

    return 0;
}

