# ChargeGrid Intelligence — Documento Técnico
## Sprint 2: Sistema Inteligente de Gerenciamento de Recarga

**Projeto acadêmico FIAP — Parceria GoodWe**  
**Turma:** 1CC | **Campus:** Av. Paulista  
**Data:** Junho de 2026

---

## 1. Visão Geral do Sistema

O ChargeGrid Intelligence é um sistema de gerenciamento de estações de recarga de veículos elétricos desenvolvido em linguagem C. O sistema simula o funcionamento de um posto de recarga com 5 vagas, controlando sessões de carregamento, distribuindo potência dinamicamente entre os carregadores ativos, aplicando tarifação dinâmica baseada nos horários da ENEL-SP e simulando comunicação com uma plataforma central via protocolo OCPP 1.6.

O sistema foi desenvolvido em parceria com a GoodWe, fabricante do carregador HCA G2, e representa a camada de software embarcado que gerencia as operações de carga em um ambiente comercial real.

---

## 2. Arquitetura do Sistema

### 2.1 Estruturas de Dados

O sistema utiliza duas structs principais para representar os dados em memória:

```c
struct Carro {
    char placa[8];           // Placa do veículo (formato Mercosul ou antigo)
    float bateria;           // Capacidade total da bateria em kWh
    int porcentagem_bateria; // Estado de carga no momento da conexão (0–99%)
}

struct Vaga {
    int status;              // 0 = livre, 1 = ocupada
    float potencia_atual;    // Potência entregue ao carregador em kW
    time_t hora_inicio;      // Timestamp de início da sessão
    float energia_consumida; // Energia total consumida na sessão (kWh)
    float custo_total;       // Custo total calculado na desconexão (R$)
    int tipo_carga;          // 1 = rápida, 2 = lenta
    int tempo_estimado;      // Tempo estimado para carga completa (minutos)
    float tarifa_kWh;        // Tarifa fixada no momento da conexão (R$/kWh)
    int transaction_id;      // ID de transação OCPP
    struct Carro carro;      // Dados do veículo conectado
}
```

### 2.2 Variáveis Globais

```c
int ocpp_msg_id = 1;              // Contador de mensagens OCPP
int ocpp_transaction_counter = 100; // Contador de IDs de transação
int total_sessoes = 0;            // Total de sessões finalizadas
float total_energia = 0;          // Energia total entregue (kWh)
float total_receita = 0;          // Receita total acumulada (R$)
```

### 2.3 Funções do Sistema

| Função | Responsabilidade |
|--------|-----------------|
| `redistribuir_potencia()` | Divide 50kW igualmente entre vagas ativas e recalcula tempo estimado |
| `determinar_tarifa()` | Retorna tarifa R$/kWh baseada em horário e tipo de carga |
| `conectar_veiculo()` | Coleta dados do veículo, registra sessão, dispara OCPP StartTransaction |
| `ver_status()` | Exibe estado em tempo real de todas as vagas com progresso de bateria |
| `calcular_tarifa()` | Calcula energia consumida e custo total no momento da desconexão |
| `desconectar_veiculo()` | Finaliza sessão, exibe resumo, dispara OCPP StopTransaction |
| `verificar_sessoes_concluidas()` | Verifica e notifica sessões que atingiram o tempo estimado |
| `ver_relatorio()` | Exibe histórico agregado e resumo das sessões ativas |
| `ocpp_boot_notification()` | Simula inicialização do charge point na plataforma central |
| `ocpp_start_transaction()` | Simula abertura de transação OCPP ao conectar veículo |
| `ocpp_stop_transaction()` | Simula encerramento de transação OCPP ao desconectar veículo |
| `simular_ocpp()` | Simula envio de MeterValues para todas as sessões ativas |

---

## 3. Controle de Demanda de Energia

### 3.1 Lógica de Redistribuição

O sistema opera com um limite fixo de **50 kW** de potência total, definido pela constante `LIMITE_POTENCIA`. A potência é dividida igualmente entre todas as vagas ativas:

```
potencia_por_vaga = 50.0 kW / numero_de_vagas_ocupadas
```

A redistribuição é chamada automaticamente em dois momentos: quando um veículo **conecta** e quando um veículo **desconecta**, garantindo que a potência seja sempre atualizada dinamicamente.

### 3.2 Cálculo de Tempo Estimado

A cada redistribuição, o tempo estimado de cada sessão ativa é recalculado:

```
energia_restante = bateria_kWh × (100 - porcentagem_atual) / 100
tempo_estimado   = (energia_restante / potencia_atual) × 60  [minutos]
```

### 3.3 Proteção contra Divisão por Zero

A função `redistribuir_potencia` possui guarda explícita para o caso de nenhuma vaga estar ocupada:

```c
if (vagas_ocupadas == 0) return;
```

---

## 4. Tarifação Dinâmica

### 4.1 Tabela de Tarifas ENEL-SP

O sistema implementa as três faixas tarifárias da ENEL-SP para o estado de São Paulo:

| Período | Dias | Horário | Tarifa |
|---------|------|---------|--------|
| Pico | Segunda a Sexta | 17h30 – 20h30 | R$ 1,12/kWh |
| Intermediário | Segunda a Sexta | 16h30 – 17h30 e 20h30 – 21h30 | R$ 0,72/kWh |
| Fora de Ponta | Todos os demais | — | R$ 0,51/kWh |

Adicionalmente, o uso de **carga rápida** aplica um multiplicador de **1,3×** sobre a tarifa base.

### 4.2 Decisão de Projeto: Tarifa Fixada na Conexão

A tarifa é determinada e salva no campo `tarifa_kWh` do struct `Vaga` **no momento da conexão do veículo**. Essa decisão evita que um veículo que conecta no horário fora de ponta seja cobrado pela tarifa de pico na desconexão, garantindo previsibilidade de cobrança ao usuário.

### 4.3 Cálculo de Custo

```
energia_consumida = (segundos_sessao / 3600) × potencia_atual   [kWh]
custo_total       = energia_consumida × tarifa_kWh               [R$]
```

---

## 5. Simulação OCPP 1.6

### 5.1 Sobre o Protocolo

O OCPP (Open Charge Point Protocol) é o protocolo padrão da indústria para comunicação entre pontos de recarga (Charge Points) e sistemas de gerenciamento centrais (Central Systems). O GoodWe HCA G2 suporta OCPP 1.6 sobre WebSocket.

### 5.2 Formato das Mensagens

O OCPP 1.6 utiliza arrays JSON com 4 campos:

```
[MessageTypeId, UniqueId, Action, Payload]

2 = CALL        (mensagem enviada pelo Charge Point)
3 = CALLRESULT  (resposta do Central System)
```

### 5.3 Ciclo de Mensagens Implementado

**Inicialização do sistema:**
```json
[OUT] [2, "1", "BootNotification", {"chargePointVendor":"GoodWe", "chargePointModel":"HCA-G2", "chargePointSerialNumber":"CG-SP-001"}]
[IN]  [3, "1", {"status":"Accepted", "currentTime":"2026-06-10T14:00:00Z", "heartbeatInterval":300}]
```

**Conexão de veículo:**
```json
[OUT] [2, "2", "StartTransaction", {"connectorId":1, "idTag":"ABC1234", "meterStart":0, "timestamp":"..."}]
[IN]  [3, "2", {"idTagInfo":{"status":"Accepted"}, "transactionId":100}]
```

**Leitura de medidor (opção 5 do menu):**
```json
[OUT] [2, "3", "MeterValues", {"connectorId":1, "transactionId":100, "meterValue":[{"sampledValue":[{"value":"1250", "unit":"Wh"}, {"value":"25.0", "unit":"kW"}]}]}]
[IN]  [3, "3", {"status":"Accepted"}]
```

**Desconexão de veículo:**
```json
[OUT] [2, "4", "StopTransaction", {"transactionId":100, "meterStop":1250, "timestamp":"...", "reason":"Local"}]
[IN]  [3, "4", {"idTagInfo":{"status":"Accepted"}}]
```

---

## 6. Menu Interativo

O sistema apresenta um menu em loop contínuo com as seguintes opções:

```
1 - Conectar veículo
2 - Desconectar veículo
3 - Ver status das vagas
4 - Ver relatório
5 - Simular envio OCPP
0 - Encerrar o programa
```

Cada opção possui validação de entrada, mensagens de erro claras e feedback colorido no terminal usando códigos ANSI.

Comportamentos específicos do menu:

- **Opção 0:** Exibe mensagem de encerramento com agradecimento ao usuário antes de finalizar o programa.
- **Opções 3 e 4:** Após exibir as informações, aguardam confirmação explícita do usuário (digitar `1`) antes de retornar ao menu principal, evitando que o menu sobreponha os dados exibidos.
- **Entradas inválidas:** Exibem mensagem de erro em vermelho e retornam ao menu sem encerrar o programa.

---

## 7. Relatório do Sistema

### 7.1 Histórico Agregado

O sistema mantém acumuladores globais atualizados a cada desconexão:

- Total de sessões finalizadas
- Energia total entregue (kWh)
- Receita total acumulada (R$)

### 7.2 Vagas Ativas

Para cada vaga ocupada, o relatório calcula e exibe em tempo real:

- Placa do veículo
- Tempo decorrido desde o início da sessão
- Energia consumida até o momento (calculada on-the-fly)
- Custo estimado até o momento

---

## 8. Decisões Técnicas Relevantes

| Decisão | Justificativa |
|---------|--------------|
| Array estático de 5 vagas | Número fixo de vagas físicas do posto; `malloc` desnecessário |
| `time_t hora_inicio` no struct | Permite cálculo preciso de duração com `difftime()` |
| Tarifa fixada na conexão | Evita cobrança incorreta por mudança de faixa horária durante a sessão |
| `gmtime` nos timestamps OCPP | Protocolo OCPP exige timestamps em UTC (ISO 8601) |
| `scanf` sem `fgets` | Evita problemas de buffer residual em leitura mista |
| Sem `malloc` / ponteiros dinâmicos | Simplicidade e segurança para sistema embarcado acadêmico |

---

## 9. Limitações Conhecidas

- **Single-thread:** O sistema não executa tarefas em paralelo. Temporizadores e verificações são feitas sob demanda, não em background.
- **Sem persistência:** Dados são perdidos ao encerrar o programa. Não há gravação em arquivo.
- **Sem autenticação:** O campo `idTag` do OCPP é preenchido com a placa do veículo por simplicidade.
- **Potência constante:** A redistribuição assume entrega constante e igual para todos os carregadores. Sistemas reais consideram estado de carga, temperatura e capacidade do veículo.

---

## 10. Como Executar

### Compilação

```bash
gcc -o chargegrid chargegrid.c
```

### Execução

```bash
./chargegrid
```

### Cenário de demonstração recomendado

1. Ao iniciar, observe o `BootNotification` OCPP
2. Conecte 3 veículos nas vagas 1, 2 e 3 — observe a redistribuição de potência (50kW ÷ 3 ≈ 16,6kW por vaga)
3. Acesse a opção 3 (Ver status) para visualizar progresso de bateria e tempo restante
4. Acesse a opção 5 (Simular OCPP) para ver os MeterValues das 3 sessões ativas
5. Acesse a opção 4 (Ver relatório) para ver o resumo das vagas ativas
6. Desconecte o veículo da vaga 1 — observe redistribuição (50kW ÷ 2 = 25kW por vaga) e StopTransaction OCPP
7. Acesse o relatório novamente para ver o histórico acumulado

---

## 11. Tecnologias Utilizadas

- **Linguagem:** C (padrão C99)
- **Bibliotecas:** `stdio.h`, `string.h`, `time.h`, `stdlib.h`, `unistd.h`
- **Protocolo simulado:** OCPP 1.6 (Open Charge Point Protocol)
- **Referência tarifária:** ENEL Distribuição São Paulo
- **Hardware de referência:** GoodWe HCA G2