# PlantGuard - Monitor de Saúde da Horta 🌿
 
O **PlantGuard** é um sistema automatizado baseado em Arduino para monitoramento de solo, temperatura e irrigação inteligente de uma horta (tomate, alface, cenoura, pimentão). Através de sensores, o projeto lê a umidade do solo e a temperatura ambiente, categoriza a saúde da planta em tempo real, atualiza um display LCD, altera as cores de um LED RGB para alertas visuais e aciona uma válvula solenoide e um servo motor automaticamente conforme a necessidade.
 
---
 
## 🚀 Funcionalidades
 
* **Monitoramento Duplo:** Leitura simultânea de umidade do solo (sensor resistivo HD-38) e temperatura ambiente (sensor digital DS18B20).
* **Avaliação Combinada:** O LED RGB reflete o pior estado entre os dois sensores — se um estiver crítico, o alerta visual é crítico, mesmo que o outro esteja bom.
* **Feedback Visual no LCD:** Display 16x2 I2C que alterna automaticamente entre a página de umidade e a página de temperatura.
* **Controle de Irrigação Automático:** Aciona uma válvula solenoide (montada externamente na tubulação, com apenas os fios entrando na caixa) sempre que o solo estiver com umidade baixa ou média.
* **Atuação por Servo Motor:** Aciona um servo motor conforme a lógica de resposta do sistema.
* **LED RGB:** Muda de cor (Verde, Amarelo ou Vermelho) conforme a gravidade da situação do solo e da temperatura.
* **Caixa Impressa em 3D:** Gabinete próprio modelado para acomodar a eletrônica, protegendo os componentes internos (a válvula solenoide fica fora da caixa).
### 🔊 Modo alternativo (somente buzzer)
 
Existe uma versão alternativa (branch VersaoAlternativa) do código que utiliza **apenas um buzzer** para os alertas, sem depender do módulo Bluetooth e/ou da tela LCD. Esse modo deve ser usado **somente** em caso de falta ou mau funcionamento de um desses dois componentes, servindo como fallback para garantir que o sistema continue sinalizando o estado da horta mesmo sem o display ou a conectividade Bluetooth.
 
---
 
## 🛠️ Componentes Utilizados
 
| Componente | Função |
| :--- | :--- |
| **Arduino Uno R3** | Cérebro do sistema |
| **HD-38 (Higrômetro de Solo)** | Medição da umidade da terra |
| **DS18B20 (Sensor de Temperatura)** | Medição da temperatura ambiente/solo |
| **Display LCD 16x2 I2C (LCD-I2C)** | Interface gráfica, alternando umidade e temperatura |
| **LED RGB** | Indicador visual de status (Bom/Médio/Crítico) |
| **Válvula Solenoide** | Atuador para liberação de água (externa à caixa) |
 
---
 
## 📌 Mapeamento de Pinos (Pinout)
 
* **A1:** Sensor de Umidade (HD-38)
* **D7:** Sensor de Temperatura (DS18B20)
* **Pino 6:** Servo Motor
* **LCD 16x2 I2C:** conectado via barramento I2C (`Wire`), endereço `0x27` ou `0x3F` (confirmar com sketch de I2C Scanner)
* Demais pinos (LED RGB, válvula solenoide) conforme definido no sketch principal
---
 
## 📊 Níveis de Umidade e Comportamento do Sistema
 
O sistema trabalha com faixas de valores analógicos (0 a 1023) lidos do HD-38, onde **solo mais seco resulta em valores mais altos**. Os limites foram calibrados para o contexto de horta e podem não ser adequados para outros tipos de plantas. Abaixo está o comportamento do LED RGB e da válvula solenoide:
 
| Faixa de Valor | Estado | LED RGB | Válvula Solenoide |
| :--- | :--- | :--- | :--- |
| **Faixa intermediária (ideal)** | **BOM** | Verde | DESLIGADA (`LOW`) |
| **Faixas próximas ao limite** | **MÉDIO** | Amarelo | LIGADA (`HIGH`) |
| **Extremos (muito seco ou encharcado)** | **CRÍTICO** | Vermelho | LIGADA (`HIGH`) |
 
> A avaliação final do LED RGB considera também a leitura de temperatura do DS18B20, exibindo sempre o pior estado entre umidade e temperatura.
 
---
Link do vídeo: https://drive.google.com/file/d/1bevwheS0CeeBoGKLSneq2bmuX4bQqyMS/view?usp=sharing

Link Slides: https://canva.link/pxrfy62i2e0yojq

Link relatório: https://docs.google.com/document/d/1NSt_E15qf-I3b7oIXnGUtw7i_61VQec1/edit?usp=drivesdk&ouid=110813830369956828366&rtpof=true&sd=trueLinks to an external site.
 
### Projeto desenvolvido para a disciplina Trabalho Interdisciplinar I na PUC Minas Coração Eucarístico
