// =================================================================================================================
//                                           Monitor de Saúde da Horta
//                              Sensores: Higrômetro (umidade), DS18B20 (temperatura)
//                     Saídas: LED RGB, Tela LCD I2C, App PlantGuard (Bluetooth), Válvula Solenoide
// =================================================================================================================


// ─── Bibliotecas Utilizadas ────────────────────────────────────────────────
#include <LCD-I2C.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>


// ─── Configuração inicial ──────────────────────────────────────────────────
  // ─── LCD I2C ─────────────────────────────────────────────
  LCD_I2C lcd_1(0x27, 16, 2);

  // Pinos Bluetooth
  SoftwareSerial Bluetooth(2, 3);

  // ─── Pinos dos sensores ──────────────────────────────────
  const int PIN_UMIDADE   = A1;
  const int PIN_TEMPERATURA   = 7;
  
  // ─── DS18B20 ─────────────────────────────────────────────
  OneWire oneWire(PIN_TEMPERATURA);
  DallasTemperature sensorTemp(&oneWire);
  
  // ─── LED RGB ─────────────────────────────────────────────
  const int PIN_RED   = 8;
  const int PIN_GREEN = 9;
  const int PIN_BLUE  = 10;
  
  // ─── Válvula solenoide ───────────────────────────────────
  const int PIN_VALVULA = 13;
  
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

  bool modoManual = false;


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
  
  // ─── Temperatura (float °C) ───────────────────────────
  String avaliarEstadoTemp(float valor) {
    if (valor >= TEMP_BOM_MIN && valor <= TEMP_BOM_MAX) return "BOM";
    if ((valor >= TEMP_MEDIO_MIN && valor < TEMP_BOM_MIN) ||
        (valor > TEMP_BOM_MAX   && valor <= TEMP_MEDIO_MAX)) return "MEDIO";
    return "CRITICO";
  }


// ─── Recomendação de ação ─────────────────────────────────────────────────────────
  // ─── Umidade ─────────────────────────────────
  String recomendacaoUmidade(const String& estado, int valor) {
    if (estado == "BOM")    return "Umidade ok";
    if (valor < UMID_BOM_MIN) return "Solo seco";
    if (valor > UMID_BOM_MAX) return "Solo encharcado";
    return "Monitorando";
  }
  
  // ─── Temperatura ────────────────────────────
  String recomendacaoTemp(const String& estado, float valor) {
    if (estado == "BOM") return "Temp ok";
    if (valor == -127.0) return "Sensor erro";
    if (valor < TEMP_BOM_MIN) {
      if (valor < TEMP_CRITICO_MIN) return "Temp critica!";
      return "Temp baixa";
    }
    if (valor > TEMP_BOM_MAX) {
      if (valor > TEMP_CRITICO_MAX) return "Temp critica!";
      return "Temp alta";
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

// ─── Exibe ações no LCD (debug) ─────────────────────────
void exibeAcoes(String acaoUmidade, String acaoTemp) {
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
void valvula(const String& estadoUmidade, int valor) {
  if (estadoUmidade == "BOM") {
    digitalWrite(PIN_VALVULA, LOW);
  } else if ((estadoUmidade == "CRITICO" || estadoUmidade == "MEDIO") && (valor < UMID_BOM_MIN)) {
    digitalWrite(PIN_VALVULA, HIGH);
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

  Bluetooth.begin(9600);

  pinMode(PIN_RED,     OUTPUT);
  pinMode(PIN_GREEN,   OUTPUT);
  pinMode(PIN_BLUE,    OUTPUT);
  pinMode(PIN_VALVULA, OUTPUT);

  digitalWrite(PIN_RED,     LOW);
  digitalWrite(PIN_GREEN,   LOW);
  digitalWrite(PIN_BLUE,    LOW);
  digitalWrite(PIN_VALVULA, LOW);
  
  sensorTemp.begin();
}

// ─── Loop principal ──────────────────────────────────────
void loop() {

  if (Serial.available() > 0) {
    char tecla = Serial.read();
    if (tecla == 't' || tecla == 'T') {
      int   adcUmidade = lerSensorMedia(PIN_UMIDADE);
      float tempC      = lerTemperaturaMedia();

      String aUmi  = recomendacaoUmidade(avaliarEstadoUmidade(adcUmidade), adcUmidade);
      String aTemp = recomendacaoTemp(avaliarEstadoTemp(tempC), tempC);

      exibeAcoes(aUmi, aTemp);
      ultimaLeitura = millis();
      return;
    }
  }

  unsigned long agora = millis();

  if (agora - ultimaLeitura >= INTERVALO) {

    int   adcUmidade   = lerSensorMedia(PIN_UMIDADE);
    float tempC        = lerTemperaturaMedia();

    String estadoUmidade = avaliarEstadoUmidade(adcUmidade);
    String estadoTemp    = avaliarEstadoTemp(tempC);

    atualizarLED(estadoUmidade, estadoTemp);
    if (!modoManual) {
      valvula(estadoUmidade, adcUmidade);
    }

    // ─── App ───────────────────────────────
      Bluetooth.print("U:");
      Bluetooth.print(adcUmidade);
      Bluetooth.print(",");
      Bluetooth.println(estadoUmidade);
  
      Bluetooth.print("T:");
      Bluetooth.print(tempC);
      Bluetooth.print(",");
      Bluetooth.println(estadoTemp);

    // ─── LCD ───────────────────────────
      // ── Página 1: Umidade ──
      lcd_1.clear();
      lcd_1.print("UMID: ");
      lcd_1.print(adcUmidade);
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

    // ─── Serial ─────────────────────
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

  // ─── Comando de irrigação pelo app ──────────────
  if (Bluetooth.available()) {
    int comandoApp = Bluetooth.read();

    if (comandoApp == 1 || comandoApp == '1') {
      modoManual = true;
      digitalWrite(PIN_VALVULA, HIGH);
    } else if (comandoApp == 0 || comandoApp == '0') {
      modoManual = true;
      digitalWrite(PIN_VALVULA, LOW);
    } else if (comandoApp == 'A' || comandoApp == 'a') {
      // Volta para o modo Automático
      modoManual = false;
    }
  }
}
