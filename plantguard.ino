// ============================================================
//   Monitor de Saúde da Samambaia — Calibração de Sensores
//   Sensores: Fotoresistor (luz), Higrômetro (umidade), 
//             Potenciômetro (pH)
//   Planta simulada: Samambaia
// ============================================================

#include <Adafruit_LiquidCrystal.h>
#include <Servo.h>
Adafruit_LiquidCrystal lcd_1(0); 

// ─── Pinos dos sensores ──────────────────────────────────
const int PIN_LUZ      = A3;
const int PIN_UMIDADE  = A1;  
const int PIN_PH       = A2;

// ─── LED RGB ─────────────────────────────────────────────
const int PIN_RED   = 8;
const int PIN_GREEN = 9;
const int PIN_BLUE  = 10;

// ─── Servo ───────────────────────────────────────────────
Servo servo;

// ─── Faixas ideais (unidades ADC 0–1023) ─────────────────
const int LUZ_CRITICO_MIN  = 0;
const int LUZ_MEDIO_MIN    = 300;
const int LUZ_BOM_MIN      = 500;
const int LUZ_BOM_MAX      = 800;
const int LUZ_MEDIO_MAX    = 900;
const int LUZ_CRITICO_MAX  = 1023;

const int UMID_CRITICO_MIN = 0;
const int UMID_MEDIO_MIN   = 400;
const int UMID_BOM_MIN     = 600;
const int UMID_BOM_MAX     = 820;
const int UMID_MEDIO_MAX   = 900;
const int UMID_CRITICO_MAX = 1023;

const int PH_CRITICO_MIN  = 0;
const int PH_MEDIO_MIN    = 328;
const int PH_BOM_MIN      = 365;
const int PH_BOM_MAX      = 438;
const int PH_MEDIO_MAX    = 511;
const int PH_CRITICO_MAX  = 1023;

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

// ─── Função: converte ADC → pH ───────────────────────────
float adcParaPH(int adc) {
  return map(adc, 0, 1023, 0, 1400) / 100.0;
}

// ─── Função: irrigação via servo ─────────────────────────
String irrigar(int umidadeAtual) {
  if (umidadeAtual < UMID_BOM_MIN) {
    servo.write(90);
    return "Valvula aberta";
  } else {
    servo.write(5);
    return "Solo umido";
  }
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
  if (sensor == "LUZ") {
    if (valor < bomMin) return "Luz artificial";
    if (valor > bomMax) return "Reduza a luz";
  }
  if (sensor == "UMIDADE") {
    return irrigar(valor);
  }
  if (sensor == "PH") {
    if (valor < bomMin) return "Adicione calcar";
    if (valor > bomMax) return "Adicione acido";
  }
  return "Monitorando";
}

// ─── Função: atualiza LED RGB conforme pior estado ───────
// CRITICO → Vermelho  (R=HIGH, G=LOW,  B=LOW)
// MEDIO   → Amarelo   (R=HIGH, G=HIGH, B=LOW)
// BOM     → Verde     (R=LOW,  G=HIGH, B=LOW)
void atualizarLED(const String& estadoLuz,
                  const String& estadoUmidade,
                  const String& estadoPH) {

  String piorEstado = "BOM";

  if (estadoLuz == "CRITICO" || estadoUmidade == "CRITICO" || estadoPH == "CRITICO") {
    piorEstado = "CRITICO";
  } else if (estadoLuz == "MEDIO" || estadoUmidade == "MEDIO" || estadoPH == "MEDIO") {
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
void exibeAcoes(String acaoUmidade, String acaoLuz, String acaoPH) {
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
  lcd_1.print("ACAO LUZ:");
  lcd_1.setCursor(0, 1);
  lcd_1.print(acaoLuz);
  delay(2000);

  lcd_1.clear();
  lcd_1.print("ACAO PH:");
  lcd_1.setCursor(0, 1);
  lcd_1.print(acaoPH);
  delay(2000);

  lcd_1.clear();
  lcd_1.print("VOLTANDO...");
  delay(500);
}

// ─── Setup ───────────────────────────────────────────────
void setup() {
  lcd_1.begin(16, 2);
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

  pinMode(PIN_RED,   OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE,  OUTPUT);

  digitalWrite(PIN_RED,   LOW);
  digitalWrite(PIN_GREEN, LOW);
  digitalWrite(PIN_BLUE,  LOW);

  servo.attach(6);
}

// ─── Loop principal ──────────────────────────────────────
void loop() {

  if (Serial.available() > 0) {
    char tecla = Serial.read();
    if (tecla == 't' || tecla == 'T') {
      int adcLuz     = lerSensorMedia(PIN_LUZ);
      int adcUmidade = lerSensorMedia(PIN_UMIDADE);
      int adcPH      = lerSensorMedia(PIN_PH);

      String aLuz = recomendacao("LUZ",     avaliarEstado(adcLuz,     LUZ_CRITICO_MIN,  LUZ_MEDIO_MIN,  LUZ_BOM_MIN,  LUZ_BOM_MAX,  LUZ_MEDIO_MAX,  LUZ_CRITICO_MAX),  adcLuz,     LUZ_BOM_MIN,  LUZ_BOM_MAX);
      String aUmi = recomendacao("UMIDADE", avaliarEstado(adcUmidade, UMID_CRITICO_MIN, UMID_MEDIO_MIN, UMID_BOM_MIN, UMID_BOM_MAX, UMID_MEDIO_MAX, UMID_CRITICO_MAX), adcUmidade, UMID_BOM_MIN, UMID_BOM_MAX);
      String apH  = recomendacao("PH",      avaliarEstado(adcPH,      PH_CRITICO_MIN,   PH_MEDIO_MIN,   PH_BOM_MIN,   PH_BOM_MAX,   PH_MEDIO_MAX,   PH_CRITICO_MAX),   adcPH,      PH_BOM_MIN,   PH_BOM_MAX);

      exibeAcoes(aUmi, aLuz, apH);
      ultimaLeitura = millis();
      return;
    }
  }

  unsigned long agora = millis();

  if (agora - ultimaLeitura >= INTERVALO) {

    int adcLuz     = lerSensorMedia(PIN_LUZ);
    int adcUmidade = lerSensorMedia(PIN_UMIDADE);
    int adcPH      = lerSensorMedia(PIN_PH);
    float ph       = adcParaPH(adcPH);

    String estadoLuz     = avaliarEstado(adcLuz,     LUZ_CRITICO_MIN,  LUZ_MEDIO_MIN,  LUZ_BOM_MIN,  LUZ_BOM_MAX,  LUZ_MEDIO_MAX,  LUZ_CRITICO_MAX);
    String estadoUmidade = avaliarEstado(adcUmidade, UMID_CRITICO_MIN, UMID_MEDIO_MIN, UMID_BOM_MIN, UMID_BOM_MAX, UMID_MEDIO_MAX, UMID_CRITICO_MAX);
    String estadoPH      = avaliarEstado(adcPH,      PH_CRITICO_MIN,   PH_MEDIO_MIN,   PH_BOM_MIN,   PH_BOM_MAX,   PH_MEDIO_MAX,   PH_CRITICO_MAX);

    // ── Atualiza LED conforme pior estado ──
    atualizarLED(estadoLuz, estadoUmidade, estadoPH);

    // Luz
    lcd_1.clear();
    lcd_1.print("LUZ: "); lcd_1.print(adcLuz);
    lcd_1.setCursor(0, 1);
    lcd_1.print("Status: "); lcd_1.print(estadoLuz);
    delay(1500);

    // Umidade
    irrigar(adcUmidade);
    lcd_1.clear();
    lcd_1.print("UMID: "); lcd_1.print(adcUmidade);
    lcd_1.setCursor(0, 1);
    lcd_1.print("Status: "); lcd_1.print(estadoUmidade);
    delay(1500);

    // pH
    lcd_1.clear();
    lcd_1.print("PH: "); lcd_1.print(ph, 1);
    lcd_1.setCursor(0, 1);
    lcd_1.print("Status: "); lcd_1.print(estadoPH);
    delay(1500);

    ultimaLeitura = millis();
  }
}
