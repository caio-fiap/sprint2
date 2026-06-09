#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#define LIMITE_POTENCIA  50.0
#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLUE "\033[1;34m"
#define ORANGE "\033[38;5;208m"
#define RESET "\033[0m"

struct Carro{
    char placa[8]; 
    float bateria; //Em kW
    int porcentagem_bateria;
};

struct Vaga{
    int status; // 0 para vaga livre, 1 para ocupada
    float potencia_atual; //Potencia que o carregador está entregado, ela pode variar dependendo da quantidade de carregadores conectados 
    time_t hora_inicio;
    float energia_consumida; 
    float custo_total;
    int tipo_carga;
    int tempo_estimado; 
    float tarifa_kWh;
    struct Carro carro; 
};

void redistribuir_potencia(struct Vaga vagas[]){
    int i;
    int vagas_ocupadas = 0; //Número de vagas ocupadas
    for(i = 0; i < 5; i++){ //Verifica quantas vagas estão ocupadas
        if(vagas[i].status == 1){
            vagas_ocupadas++;
        }
    }
    if(vagas_ocupadas == 0){ // Se o número de vagas_ocupadas for 0, o programa encerra a função redistribuir potencia para não dividir por zero no proximo loop
        printf("Todas as vagas estao "GREEN"LIVRES\n"RESET);
        return;
    }
    else{
        printf("N de vagas ocupadas: "YELLOW"%d\n"RESET, vagas_ocupadas);
    }
    for(i = 0; i < 5; i++){
        if(vagas[i].status == 1){
            vagas[i].potencia_atual = LIMITE_POTENCIA / vagas_ocupadas;
            float energia_restante = vagas[i].carro.bateria * (100 - vagas[i].carro.porcentagem_bateria) / 100.0;
            vagas[i].tempo_estimado = (int)((energia_restante / vagas[i].potencia_atual) * 60);
        }
    }
}

float determinar_tarifa(struct Vaga vagas[], int idx){
    time_t agora = time(NULL);
    struct tm*horario = localtime(&agora);  
    int dia_sem = horario->tm_wday;
    int tempo_atual = horario->tm_hour * 60 + horario->tm_min;

    float tarifa; 
    if((dia_sem > 0 && dia_sem < 6) && (tempo_atual >= 1050 && tempo_atual <= 1230)){
        tarifa = 1.12;
    }
    else if((dia_sem > 0 && dia_sem < 6) && (tempo_atual >= 990 && tempo_atual <= 1290)){
        tarifa = 0.72;
    }
    else{
        tarifa = 0.51;
    }
    
    if(vagas[idx].tipo_carga == 1) tarifa *= 1.3;
    return tarifa;
}

void conectar_veiculo(struct Vaga vagas[]){
    int i;
    int vaga_escolhida = -1;

    do{
    printf("\n");
    for(i = 0; i < 5; i++){ //Loop verifica o status das vagas
        if(vagas[i].status == 1){
            printf("Vaga %d "RED"ocupada\n"RESET, i + 1);
        }
        else{
            printf("Vaga %d "GREEN"livre\n"RESET, i + 1);
        }
    }
    printf("Digite qual vaga deseja utilizar: ");
    scanf("%d", &vaga_escolhida);   
    if(vaga_escolhida >= 6 || vaga_escolhida < 1){ //Verifica se o numero digitado pelo usuário eh valido
        printf("Vaga escolhida invalida... Digite novamente a vaga desejada\n");
        vaga_escolhida = -1;
    }
    else if(vagas[vaga_escolhida -1].status == 1){ //Verifica se a vaga escolhida pelo usuario esta disponivel
        printf("A vaga escolhida esta ocupada... Por favor escolha uma vaga livre\n");
        vaga_escolhida = -1;
    }
    else{printf("Vaga %d escolhida... Prosseguindo...\n", vaga_escolhida);}
    sleep(1);
    }while(vaga_escolhida == -1); //O loop encerra quando o usuario escolhe uma vaga valida e livre
    printf("\n");

    //Coleta da placa do carro
    printf("Digite a placa do carro: ");
    scanf("%7s", vagas[vaga_escolhida -1].carro.placa);
    printf("\n");

    //Coleta a porcentagem da bateria do carro
    int confirm = 0;
    do{
    printf("Digite a porcentagem da bateria do carro: ");
    scanf("%d", &vagas[vaga_escolhida -1].carro.porcentagem_bateria);
    printf("\n");

    if(vagas[vaga_escolhida -1].carro.porcentagem_bateria < 0 || vagas[vaga_escolhida-1].carro.porcentagem_bateria > 99){
        printf("Porcentagem invalida. Digite  um valor entre"BLUE" 0"RESET" e"BLUE" 99"RESET". \n");
        confirm = 0;
        continue;
    }
    printf("Porcentagem da bateria em: "YELLOW"%d%%\n"RESET, vagas[vaga_escolhida -1].carro.porcentagem_bateria);
    printf("Por favor confirme se a porcentagem da bateria esta correta "GREEN"(1 para sim"RESET" | "RED"2 para nao)"RESET": ");
    scanf("%d", &confirm);
    printf("\n");
    if(confirm == 2){printf("Por favor digite novamente a porcentagem da bateria\n"); confirm = 0;}
    else if(confirm == 1){printf("Porcentagem da bateria confirmada... Prosseguindo\n");}
    else{
        printf(RED"Opcao invalida... "RESET"tente novamente\n"); 
        confirm = 0;
    }
    }while(confirm == 0);
    sleep(1);
    printf("\n");

    //coleta a potencia da bateria do carro
    int opcao_bateria = -1;  
    do{
    printf("Deseja inserir a potencia da bateria manualmente?\n");
    printf("* Se a sua esolha for 2 (nao) o programa automaticamente definira a poteicna da bateria em 38.8kWh\n");
    printf(GREEN"(1 para sim"RESET" | "RED"2 para nao (padrao 38.8kWh)"RESET")");
    printf("Opcao: ");
    scanf("%d", &opcao_bateria);
    printf("\n");
    if(opcao_bateria != 1 && opcao_bateria != 2){
        printf("Opcao invalida... Tente novamente\n");
        opcao_bateria = -1;
    }
    else if(opcao_bateria == 1){
        printf("Digite a potencia da bateria (kWh): ");
        scanf("%f", &vagas[vaga_escolhida -1].carro.bateria);
        printf("\n");
        printf("A potencia da bateria foi selecionada em %.2fkWh\n", vagas[vaga_escolhida -1].carro.bateria);
    }
    else{
        printf("Potencia da bateria definida pelo sistema em 38.8kWh\n");
        vagas[vaga_escolhida -1].carro.bateria = 38.8;
    }
    sleep(1);
    }while(opcao_bateria == -1);
    printf("\n");

    // Decisao sobre o tipo da carga que o usuario deseja
    printf("Escolha o tipo de carregamento\n");
    printf(RED"1 para carga rapida"RESET"  | "BLUE"2 para carga lenta\n"RESET);
    printf("Opcao: ");
    scanf("%d", &vagas[vaga_escolhida -1].tipo_carga);
    if(vagas[vaga_escolhida-1].tipo_carga != 1 && vagas[vaga_escolhida-1].tipo_carga != 2){
        vagas[vaga_escolhida-1].tipo_carga = 2; // padrão lento se invalido
        printf(RED"Opcao invalida..."RESET" Carga lenta definida automaticamente\n");
    }

    vagas[vaga_escolhida -1].tarifa_kWh = determinar_tarifa(vagas, vaga_escolhida -1);
    printf("Tarifa definida: R$%.2f/kWh\n", vagas[vaga_escolhida-1].tarifa_kWh);

    vagas[vaga_escolhida -1].status = 1;
    vagas[vaga_escolhida -1].hora_inicio = time(NULL);
    redistribuir_potencia(vagas);
    printf("Veiculo conectado com sucesso na vaga %d!\n", vaga_escolhida);
    sleep(1);
    printf("\n");
}

void ver_status(struct Vaga vagas[]){
    int i;
    for(i = 0; i < 5; i++){
        if(vagas[i].status == 0){
            printf("Vaga %d -"GREEN" LIVRE\n"RESET, i + 1);
            printf("\n");
        }
        else{
            printf("Vaga %d\n", i + 1);
            printf("PLACA: %s\n", vagas[i].carro.placa);
            printf("Tipo de carga selecionada: ");
            if(vagas[i].tipo_carga == 1){printf(ORANGE"RAPIDA\n"RESET);}
            else{printf(ORANGE"LENTA\n"RESET);}
            double segundos = difftime(time(NULL), vagas[i].hora_inicio);
            int minutos = (int)(segundos / 60);
            printf("Tempo de carregamento: %d minutos\n", minutos);
            float energia_carregada = (segundos / 3600.0) * vagas[i].potencia_atual;
            float porc_atual = vagas[i].carro.porcentagem_bateria + (energia_carregada / vagas[i].carro.bateria) * 100;
            if(porc_atual > 100) porc_atual = 100;
            int restante = vagas[i].tempo_estimado - minutos;
            if(restante < 0) restante = 0;
            printf("Porcentagem atual da bateria: ");
            if(porc_atual <= 25)printf(RED"%.2f%%\n"RESET, porc_atual);
            else if(porc_atual <=50)printf(ORANGE"%.2f%%\n"RESET, porc_atual);
            else if(porc_atual <=75)printf(YELLOW"%.2f%%\n"RESET, porc_atual);
            else printf(GREEN"%.2f%%\n"RESET, porc_atual);
            printf("Tempo restante estimado: %d minutos\n", restante);
            printf("\n");
        }
    }
    sleep(3);
}

void calcular_tarifa(struct Vaga vagas[], int idx){ //Calcula a tarifa  
    double segundos = difftime(time(NULL), vagas[idx].hora_inicio);
    float tarifa_kWh = vagas[idx].tarifa_kWh;   
    vagas[idx].energia_consumida = (segundos / 3600.0) * vagas[idx].potencia_atual;
    vagas[idx].custo_total = vagas[idx].energia_consumida * tarifa_kWh;
}

void desconectar_veiculo(struct Vaga vagas[]){
    int i; 
    int vaga_escolhida = -1; 
    int tem_ocupada = 0;
    printf("\n");
    for(i = 0; i < 5; i++){
        if(vagas[i].status == 1){
            printf("Vaga %d -"RED" OCUPADA\n"RESET, i + 1);
            tem_ocupada = 1;
        }
    }
    if(tem_ocupada == 0){
        printf(RED"Nenhum veiculo conectado no momento. \n"RESET);
        return;
    }

    do{
    printf("\n");
    printf("Digite em qual vaga esta o veiculo que deseja desconectar: ");
    scanf("%d", &vaga_escolhida);
    if(vaga_escolhida >= 6 || vaga_escolhida < 1){
        printf(RED"Vaga escolhda invalida..."RESET" Digite novamente a vaga desejada\n"); //Verifica se a vaga selecionada é válida 
        vaga_escolhida = -1;
    }
    else if(vagas[vaga_escolhida -1].status == 0){ //Verifica se a vaga selecionada está ocupadas   
        printf(YELLOW"A vaga selecionada esta livre..."RESET" nao ha carros para desconectar nesta vaga\n");
        vaga_escolhida = -1;
    }
    else{
        printf("Vaga %d escolhida... Prosseguindo para a desconexao...\n", vaga_escolhida);
    }
    }while(vaga_escolhida == -1);
    
    calcular_tarifa(vagas, vaga_escolhida -1);  
    printf(ORANGE"===RESUMO DA SESSAO===\n"RESET);
    printf("Placa: %s\n", vagas[vaga_escolhida -1].carro.placa);
    printf("Energia consumida: %.2f kWh\n", vagas[vaga_escolhida -1].energia_consumida);
    printf("Custo total: R$ %.2f\n", vagas[vaga_escolhida -1].custo_total);
    vagas[vaga_escolhida -1].status = 0;
    vagas[vaga_escolhida -1].energia_consumida = 0;
    vagas[vaga_escolhida -1].custo_total = 0;
    vagas[vaga_escolhida -1].potencia_atual = 0;
    vagas[vaga_escolhida -1].carro.placa[0] = '\0';
    redistribuir_potencia(vagas);
    sleep(3);
    printf("\n");
}

void verificar_sessoes_concluidas(struct Vaga vagas[]){
    int i;
    for(i = 0; i < 5; i++){
        if(vagas[i].status == 1){
            int decorrido = (int)(difftime(time(NULL), vagas[i].hora_inicio) / 60);
            if(decorrido >= vagas[i].tempo_estimado){
                printf(GREEN"[AVISO] Vaga %d - carregamento concluido! Placa: %s\n"RESET, i+1, vagas[i].carro.placa);
            }
        }
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
        printf("\n");
        printf(RED"====== ChargeGrid Inteligence ======\n"RESET); //Menu de funcionamento
        printf("Bem vindo!\n");
    do{
        verificar_sessoes_concluidas(vagas);
        printf("Digite uma das opcoes abaixo\n");
        printf("1 - Conectar veiculo\n");
        printf("2 - Desconectar veiculo\n");
        printf("3 - Ver status das vagas\n");
        printf("4 - Ver relatorio\n");
        printf("5 - Simular envio OCPP\n");
        printf("0 - Encerrar o programa\n");
        printf("Opcao: ");
        scanf("%d", &opcao);
        printf("\n");

        switch(opcao){ //O loop repete até que a decisão do usuário seja 0
            case 1:
            printf("Opcao 1, conectar veiculo, selecionada\n");
            conectar_veiculo(vagas);
            break;

            case 2: 
            printf("Opcao 2, deconectar veiculo, selecionada\n");
            desconectar_veiculo(vagas);
            break;

            case 3:
            printf("Opcao 3, ver status das vagas, selecionada\n");
            ver_status(vagas);
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

