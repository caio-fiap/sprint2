**Sprint 2 — Desenvolvimento e Entrega do Chatbot GoodWe**

**Objetivos Principais**

Implementar o chatbot planejado na Sprint 1, com modelo de IA configurado com contexto para o problema da GoodWe.
Entregar o chatbot funcional rodando em Google Colab ou IDE de preferência do grupo, com interface de interação demonstrável.
Validar o funcionamento do chatbot utilizando o modelo de teste elaborado na Sprint 1.

**Objetivos Específicos**

Implementar o sistema de contexto via system prompt, garantindo que o modelo responda dentro do escopo definido (ChargeGrid Intelligence ou EV ChargeOps).
Desenvolver o fluxo de conversa com memória de contexto (histórico de mensagens), permitindo diálogos contínuos e coerentes com o usuário.
Testar o chatbot com o modelo de perguntas e respostas da Sprint 1 e documentar os resultados obtidos.
Iterar sobre o system prompt e os parâmetros do modelo com base nos testes realizados.

**Tarefa**

Implementar o chatbot em Python utilizando a API de IA escolhida na Sprint 1 (Llama, OpenAI, Gemini ou outra), com gerenciamento de histórico de mensagens e injeção de contexto via system prompt.
Executar os 5 casos de teste do modelo elaborado na Sprint 1 e registrar: pergunta enviada, resposta obtida e avaliação qualitativa (adequada / parcialmente adequada / inadequada).
Atualizar o README do repositório com instruções de execução, dependências, variáveis de ambiente necessárias e exemplos de uso.
Gravar um vídeo de demonstração de até 3 minutos mostrando o chatbot em funcionamento, com pelo menos 3 interações relevantes ao contexto do EV Challenge 2026.



**Observações Importantes**

O uso de API Key deve ser feito via variável de ambiente ou Google Colab Secrets — nenhuma chave deve aparecer exposta no código ou no repositório.
O chatbot será avaliado pela qualidade das respostas dentro do contexto GoodWe, não apenas por funcionar tecnicamente.
Grupos que utilizarem técnicas adicionais como RAG, few-shot prompting ou function calling terão esse diferencial reconhecido na avaliação.

**Critérios de Avaliação**

Até 40 pontos: funcionamento correto do chatbot com contexto injetado e respostas coerentes ao problema da GoodWe.
Até 30 pontos: qualidade da implementação técnica (organização do código, gerenciamento de histórico, boas práticas de uso de API).
Até 30 pontos: resultado dos testes documentados e qualidade do vídeo de demonstração.

**Entregável**

APENAS um arquivo **.txt** contendo: nome e RM de cada integrante, link do repositório GitHub atualizado com o código e link do vídeo de demonstração no YouTube (não listado).

**Condições de Entrega**

A integridade do arquivo é responsabilidade do grupo.
Arquivos vazios ou corrompidos não serão considerados.
Não serão aceitos envios pelo Teams ou fora do prazo.