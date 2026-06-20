# PlantGuard v1.1 - Monitor de Saúde da Horta 🌿

O **PlantGuard** é um sistema automatizado baseado em Arduino para monitoramento de solo e irrigação inteligente. Configurado inicialmente para cuidar de uma **Samambaia**, o projeto lê a umidade do solo, categoriza a saúde da planta em tempo real, atualiza um display LCD, altera as cores de um LED RGB para alertas visuais e aciona uma válvula solenoide automaticamente quando a planta precisa de água.

---

## 🚀 Funcionalidades

* **Leitura Estabilizada:** Realiza uma média de 10 amostras consecutivas do sensor de umidade para evitar falsos positivos e ruídos analógicos.
* **Controle de Irrigação Automático:** Aciona uma válvula solenoide (ou bomba d'água) sempre que o solo estiver com umidade baixa ou média.
* **Feedback Visual Instantâneo:** * **LCD 16x2:** Exibe o valor bruto lido e o status textual.
  * **LED RGB:** Muda de cor (Verde, Amarelo ou Vermelho) conforme a gravidade da situação.
* **Modo Teste via Serial:** Permite forçar uma verificação manual de ações e recomendações ao enviar a tecla `t` ou `T` no Monitor Serial.

---

## 🛠️ Componentes Utilizados

| Componente | Função |
| :--- | :--- |
| **Arduino** (Uno/Nano ou compatível) | Cérebro do sistema |
| **Higrômetro de Solo (A1)** | Medição da umidade da terra |
| **Display LCD 16x2 (I2C - Adafruit)** | Interface gráfica com o usuário |
| **LED RGB** | Indicador visual de status (Bom/Médio/Crítico) |
| **Válvula Solenoide / Relé (Pino 13)** | Atuador para liberação de água |

---

## 📌 Mapeamento de Pinos (Pinout)

* **A1:** Sensor de Umidade (Higrômetro)
* **Pino 8:** LED RGB (Vermelho)
* **Pino 9:** LED RGB (Verde)
* **Pino 10:** LED RGB (Azul)
* **Pino 13:** Válvula Solenoide / Módulo Relé

---

## 📊 Níveis de Umidade e Comportamento do Sistema

O sistema trabalha com faixas de valores analógicos (0 a 1023) para determinar a saúde do solo:

| Faixa de Valor | Estado | LED RGB | Válvula Solenoide |
| :--- | :--- | :--- | :--- |
| **600 a 820** | **BOM** | Verde | DESLIGADA (`LOW`) |
| **400 a 599** ou **821 a 900** | **MÉDIO** | Amarelo | LIGADA (`HIGH`) |
| **< 400** ou **> 900** | **CRÍTICO** | Vermelho | LIGADA (`HIGH`) |
