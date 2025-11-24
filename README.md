**Plant status check 
***Ciulinca Andra Stefania - 334CA


***Introducere


Dispozitiv pentru a asigura conditiile optime pentru plante (temperatura aerului, umiditate in sol, lumina). Daca toate conditiile sunt implinite, feedback ul este indicat pozitiv pe ecran, altfel se afiseaza conditiile esuate. 

Scopul proiectului este de a crea un dispozitiv autonom de monitorizare a plantelor, util pentru persoane care doresc să asigure automat condițiile optime de mediu pentru plantele lor, fără a verifica manual fiecare factor.



***Descriere
<img width="1536" height="1024" alt="image" src="https://github.com/user-attachments/assets/d57ae068-8f81-45ed-8dbb-0543d8b4cb7e" />


***Hardware design 
Lista de piese:
  *  LCD 1602 cu interfata I2C si Backlight Galben-Verde - [[https://www.optimusdigital.ro/ro/optoelectronice-lcd-uri/62-lcd-1602-cu-interfata-i2c-si-backlight-galben-verde.html?search_query=lcd+i2c&results=17|External Link]]
  *  Senzor de temperatura si umiditate a aerului - [[https://www.optimusdigital.ro/ro/senzori-senzori-de-temperatura/584-senzor-de-temperatura-dht11.html?search_query=senzor+temperatura&results=253|External Link]]
  *  Senzor de umiditate a solului - [[https://sigmanortec.ro/Senzor-umiditate-sol-higrometru-p125814620|External Link]]
  *  Fotorezistor - [[https://www.optimusdigital.ro/ro/componente-electronice-altele/1863-fotorezistor-tip-5528.html?search_query=fotorezistor&results=23|External Link]]
  *  Placa de dezvoltare compatibila cu arduino UNO R3 - [[https://www.optimusdigital.ro/ro/placi-avr/4561-placa-de-dezvoltare-compatibila-cu-arduino-uno-r3-atmega328p-atmega16u2-cablu-50-cm.html?search_query=arduino+uno&results=129|External Link]]
  * Male-to-Male, Female-to-Male wires
  * Rezistenta de 5K ohm
  * Rezistenta de 1k ohm

Descrierea legăturilor făcute și a pinilor utilizați:
  * Ecranul LCD 1602 (comunică prin I2C ⇒ am folosit pinii dedicați de pe plăcuță, conform instrucțiunilor de pe site-ul de unde l-am cumpărat (vezi tabel componente din secțiunea anterioară))
      * GND - GND (firul maro)
      * VCC - 5V (firul gri)
      * SDA - A4 (firul galben)
      * SCL - A5 (firul alb)
  * Senzor de Temperatura DHT11
      * Pin-ul 1 se conectează la pin-ul de 5V al plăcii de dezvoltare. (firul rosu)
      * Pin-ul 2 se conectează la un pin digital - pinul 10. (firul albastru)
      * Pin-ul 4 se conectează la un pin GND al plăcii de dezvoltare. (firul negru)
      * Senzorul nu are inclus un rezistor de pull-up, așa că am folosit un rezistor de 5kohm. Acest rezistor de pull-up se conectează între pin-ul de alimentare și pin-ul de date.
  * Senzor de umiditate a solului
      * VCC - 5V (firul verde)
      * GND - GND (firul verde)
      * AO analog output interface - A0 (firul albastru)
  * Fotorezistor
      * Conectat la (+) si (-) pe breadboard
      * Am facut si un divizor de tensiune folosind o tensiune de 1kohm conectata la pinul analog A3 (firul albastru)


***Schema Hardware

<img width="1850" height="835" alt="image" src="https://github.com/user-attachments/assets/821b47b4-4690-4a52-b72f-eb34fcc5fd40" />

<img width="1178" height="933" alt="image" src="https://github.com/user-attachments/assets/2c2bd224-caf8-4a57-bba4-519207caa4d6" />

In schema atasata senzorul de temperatura nu este DHT(cel folosit in implementare), dar conectarea este facuta conform schemei reale, cu DHT.


***Implementare

<img width="1600" height="1200" alt="image" src="https://github.com/user-attachments/assets/1cc157fd-76ba-4b30-8495-d3272c779eea" />


Pe ecran se afiseaza parametrii care nu sunt respecta conditiile impuse exemplu: "Probleme: Sol Lumina" sau "Conditii optime Planta OK", daca totul este in regula.



***Software design 
Descrierea codului aplicaţiei (firmware):
  * Mediu de dezvoltare: Arduino IDE
  * Librării şi surse 3rd-party: “Wire.h”, “LiquidCrystal_I2C.h”
În această etapă, am implementat complet funcționalitatea de bază pentru monitorizarea și afișarea condițiilor optime pentru o plantă. Codul:

Citește datele de la trei senzori: de temperatură și umiditate (DHT11), umiditate a solului și lumină ambientală.

Evaluează dacă fiecare parametru se află într-un interval optim.

Afișează mesaje corespunzătoare pe un ecran LCD I2C în funcție de starea curentă.

Salvează ultimele 5 stări măsurate și le afișează ciclic, la intervale de timp mai mari, fără a deranja afișajul principal.

Oferă feedback serial în UART pentru monitorizarea valorilor în timp real.

  * Motivația alegerii bibliotecilor
Wire.h – permite comunicarea I2C între placa principală și afișajul LCD.

SimpleDHT.h – o bibliotecă eficientă pentru citirea rapidă a datelor de la senzorul DHT11.

Codul LCD este scris „bare-metal” pentru o mai bună înțelegere a protocolului I2C și controlul precis al afișajului, fără biblioteci standard (ex: LiquidCrystal_I2C).

  * Elementul de noutate
Proiectul nu doar că oferă un feedback în timp real, dar:

Stochează istoric recent al parametrilor și îl afișează ciclic, fără a perturba mesajele de stare esențiale.

Interfața este non-blocking, folosind timere hardware și întreruperi precise, evitând delay() și blocaje.

Afișajul este dinamic și adaptiv: se revine automat la mesajul „Condiții optime” sau „Probleme” după fiecare afișare de parametri istorici.

  * Utilizarea funcționalităților din laborator
Lab 1: USART

Lab 2: Intreruperi 

Lab 5: SPI

Lab 6: I2C

Am integrat:

Timer2 pentru incrementarea unui cronometru systicks (1ms).

Timer1 configurat pentru generare de întreruperi la fiecare 10ms pentru a semnaliza optimizarea plantelor.

USART implementat manual pentru trimiterea valorilor în consola serială fără biblioteci externe.

Citirea senzorilor analogici și integrarea unui senzor digital (DHT11) prin protocoale precise.

Simulare SPI + Salvare RAM:

  *  Implementare completă a unui protocol SPI software (bit-banging) care transmite fiecare stare `PlantState` (temperatură, umiditate, umiditate sol și lumină).
  * Configurația pinilor SPI este definită manual (`MOSI`, `MISO`, `SCK`, `SS`), conform laboratoarelor PM.
  * Fiecare stare este transmisă în format binar (6 octeți) prin SPI la fiecare măsurătoare.

Stocare locală în RAM (simulare fișier)

- Fiecare `PlantState` este salvat într-un buffer circular `ramFile[100]`, cu actualizare constantă.

- Acest buffer acționează ca o simulare de fișier temporar în memorie RAM.

Afișare automată RAM în Serial Monitor

  * - La fiecare **10 secunde**, conținutul RAM (`ramFile[]`) este afișat în Serial Monitor.
  * - Sunt afișate doar înregistrările reale (nu cele cu valori implicite 0).
  * - După afișare, programul revine la comportamentul normal, cu afișare la 2 secunde a valorilor curente.


  * Structura proiectului și validare funcțională
Structura este modulară și include:

  *  Senzori:

SimpleDHT pentru citirea temperaturii și umidității.

analogRead pentru senzorii de lumină și umiditate sol.

  * Afișare:

Funcții personalizate pentru controlul LCD I2C fără biblioteci.

Două tipuri de afișaje: mesaje de stare + afișare ciclică din istoric.

  * Control:

systicks și întreruperi pentru temporizare neblocantă.

Structură PlantState și buffer circular pentru istoric.

  * Validare:

Verificare serială a valorilor citite.

Testare prin variarea parametrilor senzorilor.

Observarea schimbării automate a mesajelor pe LCD.

  * Optimizări efectuate
Eliminarea folosirii delay(): toată logica se bazează pe timere precise cu systicks.

Logica ciclică pentru afișare istoric fără a afecta mesajele importante.

Afișarea pe LCD se face doar când este nevoie, evitând suprascrieri inutile.

UART implementat manual fără Serial.begin(), pentru compatibilitate cu hardware custom.


===== Rezultate obtinute =====

[[https://github.com/andraciuli/Plant-Status-Check|GitHub - codul complet]]

[[https://www.youtube.com/watch?v=zlGk5lrqPbM|YouTube - demo]]



===== Resurse =====
[[https://docs.arduino.cc/resources/datasheets/A000066-datasheet.pdf|Datasheet Arduino]]
[[https://www.instructables.com/Arduino-Soil-Moisture-Sensor/|Senzor sol]]
[[https://www.optimusdigital.ro/ro/optoelectronice-lcd-uri/62-lcd-1602-cu-interfata-i2c-si-backlight-galben-verde.html?search_query=lcd+1602&results=17|LCD1602 cu interfata I2C]]


===== Jurnal =====

  * 11.05 - montare si testare senzor de temperatura si umiditate sol + testare ecran LCD 1602 cu interfata I2C
  * 13.05 - montare si testare fotorezistor, incheiere schema hardware
  * 14.05 - implementare software
