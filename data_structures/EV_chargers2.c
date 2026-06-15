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

int ocpp_msg_id = 1;
int ocpp_transaction_counter = 100;
int total_sessoes = 0;
float total_energia = 0;
float total_receita = 0;

struct Carro{
    char placa[8]; 
    float bateria; //Em kWh
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
    int transaction_id;
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
    if(vagas_ocupadas == 0) return;
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

// Inicio da área de OCPP

void ocpp_boot_notification(){
    time_t agora = time(NULL);
    struct tm *t = gmtime(&agora);
    char timestamp[25];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", t);

    printf(BLUE"\n[OCPP OUT] [2, \"%d\", \"BootNotification\", " RESET, ocpp_msg_id);
    printf(BLUE"{\"chargePointVendor\":\"goodWe\", \"chargePointModel\":\"HCA-G2\", "RESET);
    printf(BLUE"\"chargePointSerialNumber\":\"CG-SP-001\"}]\n"RESET);
    printf(GREEN"[OCPP IN]  [3, \"%d\", {\"status\":\"Accepted\", "RESET, ocpp_msg_id);
    printf(GREEN"\"currentTime\":\"%s\", \"heartbeatInterval\":300}]\n\n"RESET, timestamp);
    ocpp_msg_id++;
}

void ocpp_start_transaction(struct Vaga vagas[], int idx){
    time_t agora = time(NULL);
    struct tm *t = gmtime(&agora);
    char timestamp[25];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", t);

    vagas[idx].transaction_id = ocpp_transaction_counter++;

    printf(BLUE"\n[OCPP OUT] [2, \"%d\", \"StartTransaction\", "RESET, ocpp_msg_id);
    printf(BLUE"{\"connectorId\":%d, \"idTag\":\"%s\", \"meterStart\":0, \"timestamp\":\"%s\"}]\n"RESET,
           idx + 1, vagas[idx].carro.placa, timestamp);
    printf(GREEN"[OCPP IN]  [3, \"%d\", {\"idTagInfo\":{\"status\":\"Accepted\"}, \"transactionId\":%d}]\n\n"RESET,
           ocpp_msg_id, vagas[idx].transaction_id);
    ocpp_msg_id++;
}

void ocpp_stop_transaction(struct Vaga vagas[], int idx){
    time_t agora = time(NULL);
    struct tm *t = gmtime(&agora);
    char timestamp[25]; 
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", t);

    int meter_stop = (int)(vagas[idx].energia_consumida * 1000); //converte kWh para Wh

    printf(BLUE"\n[OCPP OUT] [2, \"%d\", \"StopTransaction\", "RESET, ocpp_msg_id);
    printf(BLUE"{\"transactionId\":%d, \"meterStop\":%d, \"timestamp\":\"%s\", \"reason\":\"Local\"}]\n"RESET,
           vagas[idx].transaction_id, meter_stop, timestamp);
    printf(GREEN"[OCPP IN]  [3, \"%d\", {\"idTagInfo\":{\"status\":\"Accepted\"}}]\n\n"RESET, ocpp_msg_id);
    ocpp_msg_id++;
}

void simular_ocpp(struct Vaga vagas[]){
    int i; 
    int tem_ativa = 0; 

    printf(BLUE"\n=== SIMULACAO OCPP 1.6 - MeterValues ===\n"RESET);

    for(i = 0; i < 5; i++){
        if(vagas[i].status == 1){
            tem_ativa = 1;
            double segundos = difftime(time(NULL), vagas[i].hora_inicio);
            float energia_atual = (segundos / 3600.0) * vagas[i].potencia_atual;
            int meter_wh = (int)(energia_atual * 1000);

            time_t agora = time(NULL);
            struct tm *t = gmtime(&agora);
            char timestamp[25];
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", t);

            printf(BLUE"[OCPP OUT] [2, \"%d\", \"MeterValues\", "RESET, ocpp_msg_id);
            printf(BLUE"{\"connectorId\":%d, \"transactionId\":%d, "RESET, i + 1, vagas[i].transaction_id);
            printf(BLUE"\"meterValue\":[{\"timestamp\":\"%s\", \"sampledValue\":["RESET, timestamp);
            printf(BLUE"{\"value\":\"%d\", \"unit\":\"Wh\", \"measurand\":\"Energy.Active.Import.Register\"}, "RESET, meter_wh);
            printf(BLUE"{\"value\":\"%.1f\", \"unit\":\"kW\", \"measurand\":\"Power.Active.Import\"}]}]}]\n"RESET, vagas[i].potencia_atual);
            printf(GREEN"[OCPP IN]  [3, \"%d\", {\"status\":\"Accepted\"}]\n\n"RESET, ocpp_msg_id);
            ocpp_msg_id++;
        }
    }
    sleep(2);

    if(tem_ativa == 0){
        printf(YELLOW"Nenhuma sessao ativa para enviar MeterValues.\n"RESET);
    }
    printf("\n");
    sleep(1);
}
// Fim da área OCPP

void conectar_veiculo(struct Vaga vagas[]){
    int i;
    int vaga_escolhida = -1;
    int tem_ocupada = 0; 
    
    printf("\n");
    for(i = 0; i < 5; i++){ //Loop verifica o status das vagas
        if(vagas[i].status == 1){
            printf("Vaga %d "RED"ocupada\n"RESET, i + 1);
            tem_ocupada += 1;
        }
        else{
            printf("Vaga %d "GREEN"livre\n"RESET, i + 1);
        }
    }
    if(tem_ocupada == 5){
        printf("Todas as vagas estao "RED"OCUPADAS\n"RESET);
        printf("Por favor, aguarde por uma vaga livre\n");
        return;
    }
   do{
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
    printf("Por favor confirme se a porcentagem da bateria esta correta "GREEN"(1 para sim"RESET" | "RED"2 para nao)\n"RESET);
    printf("Opcao: ");
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
    printf(GREEN"(1 para sim"RESET" | "RED"2 para nao (padrao 38.8kWh)"RESET")\n");
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
    ocpp_start_transaction(vagas, vaga_escolhida -1);
    printf("Veiculo conectado com sucesso na vaga %d!\n", vaga_escolhida);
    sleep(1);
    printf("\n");
}

void ver_status(struct Vaga vagas[]){
    int i;
    int confirm = 0;
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
    do{
        printf("Digite 1 para retornar ao menu: ");
        scanf("%d", &confirm);
        printf("\n");
        if(confirm != 1){
            printf("Opcao digitada invalida... tente novamente\n");
        }
    }while(confirm != 1);
    sleep(1);
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
        printf("\n");
        sleep(3);
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
    ocpp_stop_transaction(vagas, vaga_escolhida -1);
    total_sessoes += 1;
    total_energia += vagas[vaga_escolhida -1].energia_consumida;
    total_receita += vagas[vaga_escolhida -1].custo_total;
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

void ver_relatorio(struct Vaga vagas[]){
    int i;
    int confirm = 0;
    printf(ORANGE"\n===RELATORIO DAS SESSOES DE CARREGAMENTO===\n"RESET);
    printf("Relatorio geral: \n");
    printf("Total de sessoes: "YELLOW"%d\n"RESET, total_sessoes);
    printf("Total de energia utilizada: "YELLOW"%.2f"RESET"kWh\n", total_energia);
    printf("Ganhos totais: R$"YELLOW"%.2f"RESET"\n", total_receita);
    printf("\n");
    printf("Relatorio de vagas ativas no momento:\n");
    for(i = 0; i < 5; i++){
        if(vagas[i].status == 0){
            printf("Vaga %d "GREEN"LIVRE\n"RESET, i + 1);
            printf("\n");
        }
        else{
            printf("Vaga %d:\n", i +1);
            printf("Placa: %s\n", vagas[i].carro.placa);
            double segundos = difftime(time(NULL), vagas[i].hora_inicio);
            int minutos = (int)(segundos / 60);
            float energia_atual = (segundos / 3600.0) * vagas[i].potencia_atual; 
            float custo_total = energia_atual * vagas[i].tarifa_kWh;
            printf("Tempo desde o inicio da sessao: %d minutos\n", minutos);
            printf("Energia consumida ate o momento: "YELLOW"%.2f"RESET"kWh\n", energia_atual);
            printf("Custo estimado: R$%.2f\n", custo_total);
            printf("\n");
        }
    }
    printf("\n");
    do{
        printf("Digite 1 para sair do menu: ");
        scanf("%d", &confirm);
        printf("\n");
        if(confirm != 1){
            printf("Opcao digitada invalida... tente novamente\n");
        }
    }while(confirm != 1);
    sleep(1);
}

int main(){
    system("clear"); //Comando para limpar o terminal assim que o program iniciar * Apenas no MacOS e Linux
    //system("cls"); //Comando para limpar o terminal assim que o programa iniciar * No Windows
    struct Vaga vagas[5];// 5 é o número de vagas 
    int opcao;
    for(int i = 0; i < 5; i++){ //Inicializa as variáveis de struct Vaga em 0, menos horario inicio. 
        vagas[i].status = 0;
        vagas[i].potencia_atual = 0;
        vagas[i].energia_consumida = 0;
        vagas[i].custo_total = 0;
        vagas[i].transaction_id = 0;
    }
        printf("\n");
        printf(RED"====== ChargeGrid Inteligence ======\n"RESET); //Menu de funcionamento
        printf("Bem vindo!\n");
        ocpp_boot_notification();
    do{
        system("clear");
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
            printf("\n");
            ver_status(vagas);
            break;

            case 4: 
            printf("Opcao 4, ver relatorio, selecionada\n");
            ver_relatorio(vagas);
            break;

            case 5: 
            printf("Opcao 5, simular envio OCPP, selecionada\n");
            simular_ocpp(vagas);
            break;

            case 0: 
            printf("Opcao 0, encerrar o programa, selecionada\n");
            printf(GREEN"Obrigado por ter usado o programa ChargeGrid Intelligence\n");
            printf("Ate breve!\n"RESET);
            sleep(2);
            break;

            default: printf(RED"Opcao invalida... tente novamente\n"RESET);
            printf("\n");
            sleep(2);
            break;
        }

    }while(opcao != 0);

    return 0;
}

