# Modelagem Matemática e Computacional
## Challenge Sprint II - Máximo e mínimo na tarifação dinâmica do ChargeGrid 

Orientações gerais:    
1\. &nbsp;&nbsp;&nbsp;&nbsp;Esta atividade deve ser realizada em grupos; 
2\. &nbsp;&nbsp;&nbsp;&nbsp;A Challenge Sprint II vale 10 pontos; 
3\. &nbsp;&nbsp;&nbsp;&nbsp;Entrega no formato de Jupyter Notebook (.ipynb) com códigos em Pyhthon.  

Introdução:  
O ChargeGrid Intelligence registra as sessões de recarga e pode aplicar políticas de cobrança dinâmica. Em um teste simplificado, a equipe observou que, quando a tarifa por kWh aumenta, a demanda tende a cair.  
Considere que a tarifa cobrada seja (p), em reais por kWh, e que o lucro diário estimado do eletroposto seja dado por:  

L(p) = -35p$^{2}$ + 298.5p - 406, para 1.5 ≤ p ≤ 6.5  

em que:
*   'p' é a tarifa cobrada por kWh;
*   'L(p)' é o lucro diário estimado, em reais;
*   o intervalo 1.5 ≤ p ≤ 6.5 representa os limites aceitáveis de tarifa definidos pela empresa.

# Questões
1. Faça o gráfico de L(p) no intervalo indicado e calcule o seu valor nos extremos do intervalo (p = 1.6 e p = 6.5)
2. Calcule a derivada L'(p) simbolicamente com Sympy. Indique as regras de derivação necessárias no caso de resolver essa derivada analiticamente.
3. Encontre o(s) ponto(s) críticos(s) da função. Faça o gráfico de L'(p) e indique esse(s) ponto(s).
4. Calcule a segunda derivada L''(p) simbolicamente com Sympy e faça seu gráfico.
5. Use a segunda derivada L''(p) para verificar se o(s) ponto(s) crítico(s) representa(m) máximo(s) ou mínimo(s) local(is). Determine também o máximo e o mínimo global da função no intervalo dado. 
6. Interprete o resultado no contexto do ChargeGrid: qual tarifa deveria ser escolhida se o objetivo fosse maximizar o lucro diário?  
