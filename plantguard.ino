// ============================================================
//   Monitor de Saúde da Horta
//   Sensor: Higrômetro (umidade)
// ============================================================

#include <LCD-I2C.h>
#include <Servo.h>

// ALTERADO: construtor da LCD-I2C recebe (endereçoI2C, colunas, linhas)
LCD_I2C lcd_1(0x27, 16, 2);

// ─── Pinos dos sensores ──────────────────────────────────
const int PIN_UMIDADE  = A1;

// ─── LED RGB ─────────────────────────────────────────────
const int PIN_RED   = 8;
const int PIN_GREEN = 9;
const int PIN_BLUE  = 10;

// ─── Válvula solenoide ────────────────────────────────────
const int PIN_VALVULA = 13;

// ─── Servo ───────────────────────────────────────────────
Servo servo;

const int UMID_CRITICO_MIN = 0;
const int UMID_MEDIO_MIN   = 400;
const int UMID_BOM_MIN     = 600;
const int UMID_BOM_MAX     = 820;
const int UMID_MEDIO_MAX   = 900;
const int UMID_CRITICO_MAX = 1023;

const int N_AMOSTRAS = 10;
const unsigned long INTERVALO = 2000;
unsigned long ultimaLeitura   = 0;

// ─── Função: leitura com média ───────────────────────────
int lerSensorMedia(int pino) {
  long soma = 0;
  for (int i = 0; i < N_AMOSTRAS; i++) {
    soma += analogRead(pino);
    delay(5);
  }
  return (int)(soma / N_AMOSTRAS);
}

// ─── Função: avalia estado de saúde ─────────────────────
String avaliarEstado(int valor,
                     int criMin, int medMin, int bomMin,
                     int bomMax, int medMax, int criMax) {
  if (valor >= bomMin && valor <= bomMax) return "BOM";
  if ((valor >= medMin && valor < bomMin) ||
      (valor > bomMax && valor <= medMax)) return "MEDIO";
  return "CRITICO";
}

// ─── Função: recomendação de ação ────────────────────────
String recomendacao(const String& sensor, const String& estado, int valor,
                    int bomMin, int bomMax) {
  if (estado == "BOM") return "Tudo bem";

  if (sensor == "UMIDADE") {
    if (valor < bomMin) return "Regar a planta";
    if (valor > bomMax) return "Solo encharcado";
  }

  return "Monitorando";
}

// ─── Função: atualiza LED RGB conforme pior estado ───────
void atualizarLED(const String& estadoUmidade) {
  String piorEstado = "BOM";

  if (estadoUmidade == "CRITICO") {
    piorEstado = "CRITICO";
  } else if (estadoUmidade == "MEDIO") {
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

// ─── Exibe ações no LCD (modo teste) ─────────────────────
void exibeAcoes(String acaoUmidade) {
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
  lcd_1.print("VOLTANDO...");
  delay(500);
}

// ─── Válvula Solenoide ────────────────────────────────────
void valvula(const String& estadoUmidade) {
  if (estadoUmidade == "BOM") {
    digitalWrite(PIN_VALVULA, LOW);
  } else if (estadoUmidade == "CRITICO" || estadoUmidade == "MEDIO") {
    digitalWrite(PIN_VALVULA, HIGH);
  }
}

// ─── Setup ───────────────────────────────────────────────
void setup() {
  // ALTERADO: LCD-I2C inicializa com begin(&Wire), depois liga display e backlight
  lcd_1.begin(&Wire);
  lcd_1.display();
  lcd_1.backlight();

  lcd_1.setCursor(0, 0);
  lcd_1.print("PlantGuard");
  lcd_1.setCursor(0, 1);
  lcd_1.print("Versao 1.0");
  delay(1500);
  lcd_1.clear();
  lcd_1.setCursor(0, 0);
  lcd_1.print("Planta:");
  lcd_1.setCursor(0, 1);
  lcd_1.print("Samambaia");
  delay(1500);
  lcd_1.clear();

  Serial.begin(9600);

  pinMode(PIN_RED,     OUTPUT);
  pinMode(PIN_GREEN,   OUTPUT);
  pinMode(PIN_BLUE,    OUTPUT);
  pinMode(PIN_VALVULA, OUTPUT);

  digitalWrite(PIN_RED,     LOW);
  digitalWrite(PIN_GREEN,   LOW);
  digitalWrite(PIN_BLUE,    LOW);
  digitalWrite(PIN_VALVULA, LOW);

  servo.attach(6);
}

// ─── Loop principal ──────────────────────────────────────
void loop() {

  if (Serial.available() > 0) {
    char tecla = Serial.read();
    if (tecla == 't' || tecla == 'T') {
      int adcUmidade = lerSensorMedia(PIN_UMIDADE);

      String aUmi = recomendacao(
        "UMIDADE",
        avaliarEstado(adcUmidade, UMID_CRITICO_MIN, UMID_MEDIO_MIN, UMID_BOM_MIN,
                      UMID_BOM_MAX, UMID_MEDIO_MAX, UMID_CRITICO_MAX),
        adcUmidade, UMID_BOM_MIN, UMID_BOM_MAX
      );

      exibeAcoes(aUmi);
      ultimaLeitura = millis();
      return;
    }
  }

  unsigned long agora = millis();

  if (agora - ultimaLeitura >= INTERVALO) {

    int adcUmidade = lerSensorMedia(PIN_UMIDADE);

    String estadoUmidade = avaliarEstado(
      adcUmidade, UMID_CRITICO_MIN, UMID_MEDIO_MIN, UMID_BOM_MIN,
      UMID_BOM_MAX, UMID_MEDIO_MAX, UMID_CRITICO_MAX
    );

    atualizarLED(estadoUmidade);
    valvula(estadoUmidade);

    lcd_1.clear();
    lcd_1.print("UMID: ");
    lcd_1.print(adcUmidade);
    lcd_1.setCursor(0, 1);
    lcd_1.print("Status: ");
    lcd_1.print(estadoUmidade);

    Serial.print("Umidade: ");
    Serial.print(adcUmidade);
    Serial.print(" | Status: ");
    Serial.println(estadoUmidade);

    ultimaLeitura = millis();
  }
}
