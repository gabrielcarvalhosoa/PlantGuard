// =================================================================================================================
//                                           Monitor de Saúde da Horta
//                              Sensores: Higrômetro (umidade), DS18B20 (temperatura)
//                              Saída: Buzzer (sinalização sonora)
// =================================================================================================================


// ─── Bibliotecas Utilizadas ────────────────────────────────────────────────
#include <OneWire.h>
#include <DallasTemperature.h>


// ─── Configuração inicial ──────────────────────────────────────────────────
  // ─── Pinos dos sensores ──────────────────────────────────
  const int PIN_UMIDADE     = A1;
  const int PIN_TEMPERATURA = 7;

  // ─── DS18B20 ─────────────────────────────────────────────
  OneWire oneWire(PIN_TEMPERATURA);
  DallasTemperature sensorTemp(&oneWire);

  // ─── LED RGB ─────────────────────────────────────────────
  const int PIN_RED   = 8;
  const int PIN_GREEN = 9;
  const int PIN_BLUE  = 10;

  // ─── Válvula solenoide ───────────────────────────────────
  const int PIN_VALVULA = 13;

  // ─── Buzzer ────────────────────────────────────────────────
  const int PIN_BUZZER = 6;

  // ─── Faixas de umidade do solo para horta (ADC) ──────────
  const int UMID_CRITICO_MIN = 0;    // MUITO ÚMIDO
  const int UMID_MEDIO_MIN   = 200;
  const int UMID_BOM_MIN     = 350;  // | FAIXA
  const int UMID_BOM_MAX     = 650;  // | IDEAL
  const int UMID_MEDIO_MAX   = 800;
  const int UMID_CRITICO_MAX = 1023; // MUITO SECO

  // ─── Faixas de temperatura para horta (°C) ───────────────
  const float TEMP_CRITICO_MIN = 5.0;
  const float TEMP_MEDIO_MIN   = 10.0;
  const float TEMP_BOM_MIN     = 18.0;
  const float TEMP_BOM_MAX     = 28.0;
  const float TEMP_MEDIO_MAX   = 35.0;
  const float TEMP_CRITICO_MAX = 40.0;

  const int N_AMOSTRAS = 10;
  const unsigned long INTERVALO = 2000;
  unsigned long ultimaLeitura   = 0;

  // Guardam a última leitura de umidade para que o buzzer possa ser
  // atualizado a cada passagem do loop(), sem depender do INTERVALO
  int    adcUmidadeAtual    = 0;
  String estadoUmidadeAtual = "BOM";

// ─── Buzzer: parâmetros do padrão sonoro ─────────────────────────────────
  const unsigned int FREQ_ALTA  = 2500; // Hz - o solo está seco
  const unsigned int FREQ_BAIXA = 400;  // Hz - o solo está úmido

  const unsigned long DURACAO_APITO      = 200; // duração de cada apito (ms)
  const unsigned long PAUSA_ENTRE_APITOS = 200; // pausa entre os 2 apitos de um mesmo par (ms)

  const unsigned long INTERVALO_CRITICO     = 3000;  // pausa antes do próximo par de apitos (crítico)
  const unsigned long INTERVALO_SECO_MEDIO  = 6000;  // intervalo entre apitos (seco - médio)
  const unsigned long INTERVALO_UMIDO_MEDIO = 10000; // intervalo entre apitos (úmido - médio)

  // Máquina de estados não bloqueante do buzzer
  enum FaseBuzzer { FASE_ESPERA, FASE_APITO1, FASE_PAUSA_CURTA, FASE_APITO2 };
  FaseBuzzer    faseBuzzer         = FASE_ESPERA;
  unsigned long buzzerMarcaTempo   = 0;
  String        padraoBuzzerAtivo  = "";


// ─── Leituras dos sensores com média ─────────────────────────────────────
  // ─── Umidade  ─────────────────────────────────
  int lerSensorMedia(int pino) {
    long soma = 0;
    for (int i = 0; i < N_AMOSTRAS; i++) {
      soma += analogRead(pino);
      delay(5);
    }
    return (int)(soma / N_AMOSTRAS);
  }

  // ─── Temperatura ──────────────────────────────
  // ─ Retorna já em °C ──────────
  float lerTemperaturaMedia() {
    float soma = 0.0;
    int leituras = 0;
    for (int i = 0; i < N_AMOSTRAS; i++) {
      sensorTemp.requestTemperatures();
      float t = sensorTemp.getTempCByIndex(0);
      if (t != DEVICE_DISCONNECTED_C) {
        soma += t;
        leituras++;
      }
      delay(10);
    }
    if (leituras == 0) return -127.0;  // Erro
    return soma / leituras;
  }

// ─── Avaliação do estado de saúde ───────────────────────────────────────
  // ─── Umidade (int ADC) ────────────────────────────────
  String avaliarEstadoUmidade(int valor) {
    if (valor >= UMID_BOM_MIN && valor <= UMID_BOM_MAX) return "BOM";
    if ((valor >= UMID_MEDIO_MIN && valor < UMID_BOM_MIN) ||
        (valor > UMID_BOM_MAX   && valor <= UMID_MEDIO_MAX)) return "MEDIO";
    return "CRITICO";
  }

  // ─── Direção do desvio de umidade (define alta/baixa frequência do buzzer) ──
  String direcaoUmidade(int valor) {
    if (valor < UMID_BOM_MIN) return "UMIDO"; // abaixo da faixa boa = solo mais úmido
    if (valor > UMID_BOM_MAX) return "SECO";  // acima da faixa boa = solo mais seco
    return "NEUTRO";
  }

  // ─── Temperatura (float °C) ───────────────────────────
  String avaliarEstadoTemp(float valor) {
    if (valor >= TEMP_BOM_MIN && valor <= TEMP_BOM_MAX) return "BOM";
    if ((valor >= TEMP_MEDIO_MIN && valor < TEMP_BOM_MIN) ||
        (valor > TEMP_BOM_MAX   && valor <= TEMP_MEDIO_MAX)) return "MEDIO";
    return "CRITICO";
  }


// ─── Função: atualiza LED RGB conforme pior estado ───────
void atualizarLED(const String& estadoUmidade, const String& estadoTemp) {
  String piorEstado = "BOM";

  if (estadoUmidade == "CRITICO" || estadoTemp == "CRITICO") {
    piorEstado = "CRITICO";
  } else if (estadoUmidade == "MEDIO" || estadoTemp == "MEDIO") {
    piorEstado = "MEDIO";
  }

  if (piorEstado == "CRITICO") {
    digitalWrite(PIN_RED,   HIGH);
    digitalWrite(PIN_GREEN, LOW);
    digitalWrite(PIN_BLUE,  LOW);
  } else if (piorEstado == "MEDIO") {
    digitalWrite(PIN_RED,   HIGH);
    digitalWrite(PIN_GREEN, HIGH);
    digitalWrite(PIN_BLUE,  LOW);
  } else {
    digitalWrite(PIN_RED,   LOW);
    digitalWrite(PIN_GREEN, HIGH);
    digitalWrite(PIN_BLUE,  LOW);
  }
}

// ─── Válvula Solenoide (só abre quando o solo estiver seco) ──
void valvula(const String& estadoUmidade, int valor) {
  if (estadoUmidade == "BOM") {
    digitalWrite(PIN_VALVULA, LOW);
  } else if ((estadoUmidade == "CRITICO" || estadoUmidade == "MEDIO") && (valor < UMID_BOM_MIN)) {
    digitalWrite(PIN_VALVULA, HIGH);
  }
}

// ─── Buzzer: define o padrão sonoro conforme o estado do solo ───────────
void configurarPadraoBuzzer(const String& estadoUmidade, const String& direcao,
                             int &qtdApitos, unsigned int &freq, unsigned long &intervalo) {
  if (estadoUmidade == "CRITICO" && direcao == "SECO") {
    // Solo muito seco: dois apitos agudos, pausa de 3s antes do próximo par
    qtdApitos = 2; freq = FREQ_ALTA;  intervalo = INTERVALO_CRITICO;
  } else if (estadoUmidade == "MEDIO" && direcao == "SECO") {
    // Solo seco: um apito agudo a cada 6s
    qtdApitos = 1; freq = FREQ_ALTA;  intervalo = INTERVALO_SECO_MEDIO;
  } else if (estadoUmidade == "MEDIO" && direcao == "UMIDO") {
    // Solo úmido: um apito grave a cada 10s
    qtdApitos = 1; freq = FREQ_BAIXA; intervalo = INTERVALO_UMIDO_MEDIO;
  } else if (estadoUmidade == "CRITICO" && direcao == "UMIDO") {
    // Solo muito úmido: dois apitos graves, pausa de 3s antes do próximo par
    qtdApitos = 2; freq = FREQ_BAIXA; intervalo = INTERVALO_CRITICO;
  } else {
    // Solo OK (BOM): sem apito
    qtdApitos = 0; freq = 0; intervalo = 0;
  }
}

// ─── Buzzer ─────────────────────────────────────────────────────────────
void gerenciarBuzzer(const String& estadoUmidade, int valorUmidade) {
  String direcao = direcaoUmidade(valorUmidade);

  int qtdApitos;
  unsigned int freq;
  unsigned long intervalo;
  configurarPadraoBuzzer(estadoUmidade, direcao, qtdApitos, freq, intervalo);

  String chavePadrao = estadoUmidade + "_" + direcao;

  if (chavePadrao != padraoBuzzerAtivo) {
    padraoBuzzerAtivo = chavePadrao;
    noTone(PIN_BUZZER);
    buzzerMarcaTempo = millis();

    if (qtdApitos > 0) {
      tone(PIN_BUZZER, freq, DURACAO_APITO);
      faseBuzzer = FASE_APITO1;
    } else {
      faseBuzzer = FASE_ESPERA;
    }
    return;
  }

  if (qtdApitos == 0) {
    return; // Solo OK: buzzer permanece desligado
  }

  unsigned long agora = millis();

  switch (faseBuzzer) {
    case FASE_ESPERA:
      if (agora - buzzerMarcaTempo >= intervalo) {
        tone(PIN_BUZZER, freq, DURACAO_APITO);
        buzzerMarcaTempo = agora;
        faseBuzzer = FASE_APITO1;
      }
      break;

    case FASE_APITO1:
      if (agora - buzzerMarcaTempo >= DURACAO_APITO) {
        buzzerMarcaTempo = agora;
        faseBuzzer = (qtdApitos == 2) ? FASE_PAUSA_CURTA : FASE_ESPERA;
      }
      break;

    case FASE_PAUSA_CURTA:
      if (agora - buzzerMarcaTempo >= PAUSA_ENTRE_APITOS) {
        tone(PIN_BUZZER, freq, DURACAO_APITO);
        buzzerMarcaTempo = agora;
        faseBuzzer = FASE_APITO2;
      }
      break;

    case FASE_APITO2:
      if (agora - buzzerMarcaTempo >= DURACAO_APITO) {
        buzzerMarcaTempo = agora;
        faseBuzzer = FASE_ESPERA;
      }
      break;
  }
}

// ─── Setup ───────────────────────────────────────────────
void setup() {
  Serial.begin(9600);

  pinMode(PIN_RED,     OUTPUT);
  pinMode(PIN_GREEN,   OUTPUT);
  pinMode(PIN_BLUE,    OUTPUT);
  pinMode(PIN_VALVULA, OUTPUT);
  pinMode(PIN_BUZZER,  OUTPUT);

  digitalWrite(PIN_RED,     LOW);
  digitalWrite(PIN_GREEN,   LOW);
  digitalWrite(PIN_BLUE,    LOW);
  digitalWrite(PIN_VALVULA, LOW);

  // Inicia biblioteca do DS18B20
  sensorTemp.begin();
}

// ─── Loop principal ──────────────────────────────────────
void loop() {

  unsigned long agora = millis();

  if (agora - ultimaLeitura >= INTERVALO) {

    int   adcUmidade = lerSensorMedia(PIN_UMIDADE);
    float tempC       = lerTemperaturaMedia();

    String estadoUmidade = avaliarEstadoUmidade(adcUmidade);
    String estadoTemp    = avaliarEstadoTemp(tempC);

    atualizarLED(estadoUmidade, estadoTemp);
    valvula(estadoUmidade, adcUmidade);

    adcUmidadeAtual    = adcUmidade;
    estadoUmidadeAtual = estadoUmidade;

    Serial.print("Umidade ADC: ");
    Serial.print(adcUmidade);
    Serial.print(" | Status: ");
    Serial.print(estadoUmidade);
    Serial.print(" || Temp: ");
    Serial.print(tempC, 1);
    Serial.print(" C | Status: ");
    Serial.println(estadoTemp);

    ultimaLeitura = millis();
  }
  gerenciarBuzzer(estadoUmidadeAtual, adcUmidadeAtual);
}
