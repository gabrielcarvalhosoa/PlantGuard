// =================================================================================================================
//                                         Monitor de Saúde da Horta
//                               Sensores: Higrômetro (umidade simulada por potenciômetro), Potenciômetro (simula temperatura)
// =================================================================================================================

// ─── Configuração inicial 
#include <Wire.h>

LCD_I2C lcd_1(0x27, 16, 2);

// ─── Pinos dos sensores ──────────────────────────────────
const int PIN_UMIDADE     = A1;
const int PIN_TEMPERATURA = A0; 

// ─── LED RGB ─────────────────────────────────────────────
const int PIN_RED   = 8;
const int PIN_GREEN = 9;
const int PIN_BLUE  = 10;

// ─── Válvula solenoide ───────────────────────────────────
const int PIN_VALVULA = 13;

// ─── AJUSTADO: Faixas de umidade do solo em PORCENTAGEM (%) ──────────
const float UMID_CRITICO_MIN = 0.0;
const float UMID_MEDIO_MIN   = 30.0;
const float UMID_BOM_MIN     = 50.0;  // | FAIXA
const float UMID_BOM_MAX     = 80.0;  // | IDEAL
const float UMID_MEDIO_MAX   = 90.0;
const float UMID_CRITICO_MAX = 100.0; // MUITO ÚMIDO ENCHARCADO

// ─── Faixas de temperatura para horta (°C) ───────────────
const float TEMP_CRITICO_MIN = 5.0;
const float TEMP_MEDIO_MIN   = 10.0;
const float TEMP_BOM_MIN     = 18.0;
const float TEMP_BOM_MAX     = 28.0;
const float TEMP_MEDIO_MAX   = 35.0;
const float TEMP_CRITICO_MAX = 40.0;

// ─── Faixa simulada de temperatura do potenciômetro ──────
const float TEMP_SIMULADA_MIN = -10.0; // Temperatura correspondente ao ADC 0
const float TEMP_SIMULADA_MAX = 50.0;  // Temperatura correspondente ao ADC 1023

const int N_AMOSTRAS = 10;
const unsigned long INTERVALO = 2000;
unsigned long ultimaLeitura   = 0;

// ─── Leituras dos sensores com média ─────────────────────────────────────
#define UMIDADE_MIN 0.0
#define UMIDADE_MAX 100.0

float lerUmidadeMedia(int pino) {
  long soma = 0;
  for (int i = 0; i < N_AMOSTRAS; i++) {
    soma += analogRead(pino);
    delay(5);
  }
  
  int mediaADC = soma / N_AMOSTRAS;

  // Converte ADC (0-1023) -> % de Umidade
  long umidadeX10 = map(mediaADC, 0, 1023,
                        (long)(UMIDADE_MIN * 10),
                        (long)(UMIDADE_MAX * 10));

  return umidadeX10 / 10.0;
}

// ─── Temperatura (via Potenciômetro) ──────────────────────────
float lerTemperaturaMedia(int pino) {
  long soma = 0;
  int leituras = 0;
  for (int i = 0; i < N_AMOSTRAS; i++) {
    soma += analogRead(pino);
    leituras++;
    delay(10);
  }
  if (leituras == 0) return -127.0;

  int mediaADC = soma / leituras;

  long tempX10 = map(mediaADC, 0, 1023,
                      (long)(TEMP_SIMULADA_MIN * 10),
                      (long)(TEMP_SIMULADA_MAX * 10));

  return tempX10 / 10.0;
}

// ─── Avaliação do estado de saúde ───────────────────────────────────────
 String avaliarEstadoUmidade(float valor) {
    if (valor >= UMID_BOM_MIN && valor <= UMID_BOM_MAX) return "BOM";
    if ((valor >= UMID_MEDIO_MIN && valor < UMID_BOM_MIN) ||
        (valor > UMID_BOM_MAX   && valor <= UMID_MEDIO_MAX)) return "MEDIO";
    return "CRITICO";
  }

String avaliarEstadoTemp(float valor) {
  if (valor >= TEMP_BOM_MIN && valor <= TEMP_BOM_MAX) return "BOM";
  if ((valor >= TEMP_MEDIO_MIN && valor < TEMP_BOM_MIN) ||
      (valor > TEMP_BOM_MAX   && valor <= TEMP_MEDIO_MAX)) return "MEDIO";
  return "CRITICO";
}

// ─── Recomendação de ação ─────────────────────────────────────────────────────────
String recomendacaoUmidade(const String& estado, float valor) {
  if (estado == "BOM")    return "Umidade ok";
  if (valor < UMID_BOM_MIN) return "Regar a horta";
  if (valor > UMID_BOM_MAX) return "Solo encharcado";
  return "Monitorando";
}

String recomendacaoTemp(const String& estado, float valor) {
  if (estado == "BOM") return "Temperatura ok";
  if (valor == -127.0) return "Sensor erro";
  if (valor < TEMP_BOM_MIN) {
    if (valor < TEMP_CRITICO_MIN) return "Temperatura critica!";
    return "Temp baixa";
  }
  if (valor > TEMP_BOM_MAX) {
    if (valor > TEMP_CRITICO_MAX) return "Temperatura critica!";
    return "Temperatura alta";
  }
  return "Monitorando";
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

// ─── Exibe ações no Monitor Serial (modo teste) ─────────────────────
void exibeAcoes(String acaoUmidade, String acaoTemp) {
  Serial.println("TESTES:");
  Serial.print("ACAO UMIDADE: ");
  Serial.println(acaoUmidade);

  Serial.print("ACAO TEMP: ");
  Serial.println(acaoTemp);

  lcd_1.clear();
  lcd_1.setCursor(0, 0);
  lcd_1.print("TESTES:");
  delay(1000);

  lcd_1.clear();
  lcd_1.print("ACAO UMIDADE:");
  lcd_1.setCursor(0, 1);
  lcd_1.print(acaoUmidade);
  delay(2000);

  lcd_1.clear();
  lcd_1.print("ACAO TEMP:");
  lcd_1.setCursor(0, 1);
  lcd_1.print(acaoTemp);
  delay(2000);

  lcd_1.clear();
  lcd_1.print("VOLTANDO...");
  delay(500);
}

// ─── Válvula Solenoide (só abre quando o solo estiver seco) ──
void valvula(const String& estadoUmidade, float valor) {
  if ((estadoUmidade == "CRITICO" || estadoUmidade == "MEDIO") && (valor < UMID_BOM_MIN)) {
    digitalWrite(PIN_VALVULA, HIGH);
  } else {
    digitalWrite(PIN_VALVULA, LOW);
  }
}
// ─── Setup ───────────────────────────────────────────────
void setup() {

  Wire.begin();
  lcd_1.begin(&Wire);
  lcd_1.display();
  lcd_1.backlight();

  lcd_1.setCursor(0, 0);
  lcd_1.print("PlantGuard");
  lcd_1.setCursor(0, 1);
  lcd_1.print("Versao 2.0");
  delay(1500);
  lcd_1.clear();
  lcd_1.setCursor(0, 0);
  lcd_1.print("Planta:");
  lcd_1.setCursor(0, 1);
  lcd_1.print("Horta");
  delay(1500);
  lcd_1.clear();

  Serial.begin(9600);

  Serial.println("PlantGuard");
  Serial.println("Versao 2.0");
  Serial.print("Planta: ");
  Serial.println("Horta");

  pinMode(PIN_RED,     OUTPUT);
  pinMode(PIN_GREEN,   OUTPUT);
  pinMode(PIN_BLUE,    OUTPUT);
  pinMode(PIN_VALVULA, OUTPUT);

  digitalWrite(PIN_RED,     LOW);
  digitalWrite(PIN_GREEN,   LOW);
  digitalWrite(PIN_BLUE,    LOW);
  digitalWrite(PIN_VALVULA, LOW);
}

// ─── Loop principal ──────────────────────────────────────
void loop() {

  if (Serial.available() > 0) {
    char tecla = Serial.read();
    if (tecla == 't' || tecla == 'T') {
      float umidadePorcentagem = lerUmidadeMedia(PIN_UMIDADE);
      float tempC = lerTemperaturaMedia(PIN_TEMPERATURA);

      String aUmi  = recomendacaoUmidade(avaliarEstadoUmidade(umidadePorcentagem), umidadePorcentagem);
      String aTemp = recomendacaoTemp(avaliarEstadoTemp(tempC), tempC);

      exibeAcoes(aUmi, aTemp);
      ultimaLeitura = millis();
      return;
    }
  }

  unsigned long agora = millis();

  if (agora - ultimaLeitura >= INTERVALO) {

    float umidadePorcentagem = lerUmidadeMedia(PIN_UMIDADE);
    float tempC             = lerTemperaturaMedia(PIN_TEMPERATURA);

    String estadoUmidade = avaliarEstadoUmidade(umidadePorcentagem);
    String estadoTemp    = avaliarEstadoTemp(tempC);

    atualizarLED(estadoUmidade, estadoTemp);
    valvula(estadoUmidade, umidadePorcentagem);

     // ── Página 1: Umidade ──
    lcd_1.clear();
    lcd_1.print("UMID: ");
    lcd_1.print(umidadePorcentagem);
    lcd_1.setCursor(0, 1);
    lcd_1.print("St: ");
    lcd_1.print(estadoUmidade);
    delay(2000);
    
    // ── Página 2: Temperatura ──
    lcd_1.clear();
    lcd_1.print("TEMP: ");
    if (tempC == -127.0) {
      lcd_1.print("ERRO");
    } else {
      lcd_1.print(tempC, 1);
      lcd_1.print(" C");
    }
    lcd_1.setCursor(0, 1);
    lcd_1.print("St: ");
    lcd_1.print(estadoTemp);
    delay(2000);

    Serial.print("UMID: ");
    Serial.print(umidadePorcentagem, 1); // Exibe com 1 casa decimal
    Serial.print("% | Status: ");
    Serial.println(estadoUmidade);

    Serial.print("TEMP: ");
    if (tempC == -127.0) {
      Serial.println("ERRO");
    } else {
      Serial.print(tempC, 1);
    }
    Serial.print(" C | Status: ");
    Serial.println(estadoTemp);

    ultimaLeitura = millis();
  }
}
