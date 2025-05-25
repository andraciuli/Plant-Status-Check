// Cod complet cu afișare RAM log filtrată în Serial Monitor la fiecare 10 secunde
#include <Wire.h>
#include <SimpleDHT.h>

#define PHOTO_SENSOR A3
#define SOIL_SENSOR A0
#define LCD_I2C_ADDR 0x27

#define TEMP_MIN 20
#define TEMP_MAX 30
#define SOIL_MIN 400
#define SOIL_MAX 800
#define LIGHT_MIN 200

#define SPI_MOSI_PIN 2
#define SPI_MISO_PIN 3
#define SPI_SCK_PIN  4
#define SPI_SS_PIN   5

SimpleDHT11 dht11;
const int dht_pin = 10;

volatile bool conditionsAreOptimal = false;
volatile bool shouldDisplayOptimalMsg = false;
volatile uint32_t systicks = 0;

#define MAX_HISTORY 5
#define DISPLAY_INTERVAL 6000
#define RETURN_TO_MESSAGE_DELAY 2000

struct PlantState {
  byte temp;
  byte hum;
  int soil;
  int light;
};

PlantState history[MAX_HISTORY];
uint8_t historyIndex = 0;
uint8_t currentDisplayIndex = 0;
uint32_t lastDisplayTime = 0;
uint32_t lastParamDisplayStart = 0;
bool showingParameters = false;
bool shouldRefreshMainMessage = false;

#define RAM_LOG_SIZE 100
PlantState ramFile[RAM_LOG_SIZE];
uint8_t ramFileIndex = 0;

void saveToRamFile(PlantState s) {
  ramFile[ramFileIndex++] = s;
  if (ramFileIndex >= RAM_LOG_SIZE) ramFileIndex = 0;
}

void spi_init() {
  pinMode(SPI_MOSI_PIN, OUTPUT);
  pinMode(SPI_MISO_PIN, INPUT);
  pinMode(SPI_SCK_PIN, OUTPUT);
  pinMode(SPI_SS_PIN, OUTPUT);
  digitalWrite(SPI_SCK_PIN, LOW);
  digitalWrite(SPI_SS_PIN, HIGH);
}

void spi_start_transmission() {
  digitalWrite(SPI_SS_PIN, LOW);
}

void spi_end_transmission() {
  digitalWrite(SPI_SS_PIN, HIGH);
}

uint8_t spi_transfer(uint8_t data_out) {
  uint8_t data_in = 0;
  for (int i = 7; i >= 0; i--) {
    digitalWrite(SPI_SCK_PIN, LOW);
    digitalWrite(SPI_MOSI_PIN, (data_out >> i) & 1);
    delayMicroseconds(1);
    digitalWrite(SPI_SCK_PIN, HIGH);
    delayMicroseconds(1);
    data_in <<= 1;
    if (digitalRead(SPI_MISO_PIN)) {
      data_in |= 1;
    }
  }
  return data_in;
}

void storePlantStateToSPI(PlantState s) {
  spi_start_transmission();
  spi_transfer(s.temp);
  spi_transfer(s.hum);
  spi_transfer((s.soil >> 8) & 0xFF);
  spi_transfer(s.soil & 0xFF);
  spi_transfer((s.light >> 8) & 0xFF);
  spi_transfer(s.light & 0xFF);
  spi_end_transmission();
}

void wait(uint32_t ms) {
  uint32_t start = systicks;
  while ((systicks - start) < ms);
}

void waitMicro(uint32_t us) {
  wait(us / 1000 > 0 ? us / 1000 : 1);
}

void timer2_init_1ms() {
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2 = 0;
  OCR2A = 249;
  TCCR2A |= (1 << WGM21);
  TCCR2B |= (1 << CS22);
  TIMSK2 |= (1 << OCIE2A);
}

ISR(TIMER2_COMPA_vect) {
  systicks++;
}

ISR(TIMER1_COMPA_vect) {
  if (conditionsAreOptimal) {
    shouldDisplayOptimalMsg = true;
  }
}

void setup() {
  uart_init115200();
  timer2_init_1ms();
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 2499;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS11) | (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);
  sei();

  Wire.begin();
  lcd_init();
  spi_init();
}

void loop() {
  byte temperature = 0;
  byte humidity = 0;
  dht11.read(dht_pin, &temperature, &humidity, NULL);

  int soil = analogRead(SOIL_SENSOR);
  int light = analogRead(PHOTO_SENSOR);

  bool tempOk = (temperature >= TEMP_MIN && temperature <= TEMP_MAX);
  bool soilOk = (soil >= SOIL_MIN && soil <= SOIL_MAX);
  bool lightOk = (light > LIGHT_MIN);

  conditionsAreOptimal = tempOk && soilOk && lightOk;

  PlantState s = {temperature, humidity, soil, light};
  history[historyIndex] = s;
  saveToRamFile(s);
  storePlantStateToSPI(s);
  historyIndex = (historyIndex + 1) % MAX_HISTORY;

  if (systicks - lastDisplayTime >= DISPLAY_INTERVAL) {
    showingParameters = true;
    lastDisplayTime = systicks;
    lastParamDisplayStart = systicks;

    lcd_set_cursor(0, 0);
    lcd_write_string("T:");
    printIntToLcd(history[currentDisplayIndex].temp);
    lcd_write_string(" H:");
    printIntToLcd(history[currentDisplayIndex].hum);
    lcd_write_string("      ");

    lcd_set_cursor(1, 0);
    lcd_write_string("S:");
    printIntToLcd(history[currentDisplayIndex].soil);
    lcd_write_string(" L:");
    printIntToLcd(history[currentDisplayIndex].light);
    lcd_write_string("     ");

    currentDisplayIndex = (currentDisplayIndex + 1) % MAX_HISTORY;
  }

  if (showingParameters && (systicks - lastParamDisplayStart >= RETURN_TO_MESSAGE_DELAY)) {
    showingParameters = false;
    shouldRefreshMainMessage = true;
  }

  if (!showingParameters && (shouldRefreshMainMessage || shouldDisplayOptimalMsg)) {
    lcd_set_cursor(0, 0);
    if (!conditionsAreOptimal) {
      lcd_write_string("Probleme:       ");
      lcd_set_cursor(1, 0);
      if (!tempOk) lcd_write_string("Temp ");
      if (!soilOk) lcd_write_string("Sol ");
      if (!lightOk) lcd_write_string("Lumina ");
      lcd_write_string("               ");
    } else {
      lcd_write_string("CONDITII OPTIME ");
      lcd_set_cursor(1, 0);
      lcd_write_string("PLANTA OK       ");
    }
    shouldRefreshMainMessage = false;
    shouldDisplayOptimalMsg = false;
  }

  static uint32_t lastRamPrint = 0;
  if (systicks - lastRamPrint >= 10000) {
    printToSerial("--- RAM log ---\r\n");
    for (int i = 0; i < ramFileIndex; i++) {
      printToSerial("["); printInt(i); printToSerial("] T:");
      printInt(ramFile[i].temp);
      printToSerial(" H:"); printInt(ramFile[i].hum);
      printToSerial(" S:"); printInt(ramFile[i].soil);
      printToSerial(" L:"); printInt(ramFile[i].light);
      printToSerial("\r\n");
    }
    printToSerial("--- END RAM log ---\r\n\r\n");
    lastRamPrint = systicks;
  } else {
    printToSerial("Temp: "); printInt(temperature);
    printToSerial(" C, Hum: "); printInt(humidity);
    printToSerial(" %, Sol: "); printInt(soil);
    printToSerial(", Light: "); printInt(light);
    printToSerial("\r\n\r\n");
  }

  wait(2000);
}

void uart_init115200() {
  unsigned int ubrr = 8;
  UBRR0H = (unsigned char)(ubrr >> 8);
  UBRR0L = (unsigned char)ubrr;
  UCSR0B = (1 << TXEN0);
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void printToSerial(const char* text) {
  while (*text) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = *text++;
  }
}

void printInt(int value) {
  char buffer[7];
  int i = 0;
  if (value == 0) {
    printToSerial("0");
    return;
  }
  if (value < 0) {
    printToSerial("-");
    value = -value;
  }
  while (value > 0) {
    buffer[i++] = '0' + (value % 10);
    value /= 10;
  }
  for (int j = i - 1; j >= 0; j--) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = buffer[j];
  }
}

void printIntToLcd(int value) {
  char buf[7];
  itoa(value, buf, 10);
  lcd_write_string(buf);
}

void lcd_send_command(uint8_t cmd) {
  Wire.beginTransmission(LCD_I2C_ADDR);
  Wire.write((cmd & 0xF0) | 0x0C); Wire.write((cmd & 0xF0) | 0x08);
  Wire.write((cmd << 4 & 0xF0) | 0x0C); Wire.write((cmd << 4 & 0xF0) | 0x08);
  Wire.endTransmission();
  waitMicro(50);
}

void lcd_send_data(uint8_t data) {
  Wire.beginTransmission(LCD_I2C_ADDR);
  Wire.write((data & 0xF0) | 0x0D); Wire.write((data & 0xF0) | 0x09);
  Wire.write((data << 4 & 0xF0) | 0x0D); Wire.write((data << 4 & 0xF0) | 0x09);
  Wire.endTransmission();
  waitMicro(50);
}

void lcd_init() {
  wait(50);
  Wire.beginTransmission(LCD_I2C_ADDR);
  Wire.write(0x30); Wire.write(0x30); Wire.write(0x30);
  Wire.endTransmission();
  wait(5);
  lcd_send_command(0x33);
  wait(5);
  lcd_send_command(0x32);
  wait(5);
  lcd_send_command(0x28);
  wait(5);
  lcd_send_command(0x0C);
  wait(5);
  lcd_send_command(0x06);
  wait(5);
  lcd_send_command(0x01);
  wait(5);
}

void lcd_set_cursor(uint8_t row, uint8_t col) {
  uint8_t addr = (row == 0) ? 0x80 + col : 0xC0 + col;
  lcd_send_command(addr);
}

void lcd_write_string(const char* str) {
  while (*str) {
    lcd_send_data(*str++);
  }
}