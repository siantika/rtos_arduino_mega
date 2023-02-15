#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <TinyGPSPlus.h>


#define PIN_ANALOG A0


SemaphoreHandle_t xSerialSemaphore;
TinyGPSPlus gps;

/* ****** Deklarasi awal fungsi  ****** */
void TaskGps( void *pvParameters );
void TaskCelular( void *pvParameters );
void TaskAnalogRead( void *pvParameters );
void displayInfo(void);



/* ********* Driver Code *********** */
void setup() {
  Serial.begin(115200);
  Serial1.begin(9600); // GPS
  Serial2.begin(9600); // Celular


  /* Membuat mutex untuk objek Serial */
  if ( xSerialSemaphore == NULL )
  {
    xSerialSemaphore = xSemaphoreCreateMutex();
    if ( ( xSerialSemaphore ) != NULL )
      xSemaphoreGive( ( xSerialSemaphore ) );
  }


  xTaskCreate(
    TaskGps
    ,  "readGps"
    ,  256
    ,  NULL
    ,  1
    ,  NULL );

  xTaskCreate(
    TaskCelular
    ,  "Celular"
    ,  256
    ,  NULL
    ,  2
    ,  NULL );

  xTaskCreate(
    TaskAnalogRead
    ,  "AnalogRead"
    ,  128
    ,  NULL
    ,  1
    ,  NULL );


}

void loop() {
  // kosong
}

/* ********* Driver Code berakhir di sini *********** */



/* ********* TASKS dan fungsi ********** */

/*  Nama        : TaskGps
    Deskripsi   : membaca data dari GPS ublox 6. Komunikasi Serial1
    Param       : -
    Return value: -
*/

void TaskGps( void *pvParameters __attribute__((unused)) )  {

  for (;;)
  {


    if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )

      while (Serial1.available() > 0)
        if (gps.encode(Serial1.read()))
          displayInfo();

    if (millis() > 5000 && gps.charsProcessed() < 10)
    {
      Serial.println(F("No GPS detected: check wiring."));
      while (true);
    }

    xSemaphoreGive( xSerialSemaphore );
  }

  vTaskDelay(1000);
}


/*  Nama        : TaskAnalogRead
    Deskripsi   : membaca nilai analog pada pin
    Param       : -
    Return value: -
*/

void TaskAnalogRead( void *pvParameters __attribute__((unused)) )  {

  for (;;)
  {
    int sensorValue = analogRead(PIN_ANALOG);

    if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
      Serial.println(sensorValue);

    xSemaphoreGive( xSerialSemaphore );
  }

  vTaskDelay(1);
}


/*  Nama        : TaskCelular
    Deskripsi   : mengirim perintah 'AT' dan menunggu respon dari sim800. Komunikasi Serial2
    Param       : -
    Return value: -
*/

void TaskCelular( void *pvParameters __attribute__((unused))  )
{

  for (;;)
  {
    if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
    {
      Serial2.println("AT");
      if (Serial2.available())
      {
        Serial.println(Serial2.readString());
      }

      xSemaphoreGive( xSerialSemaphore );
    }

    vTaskDelay(1000);
  }
}


/*  Nama        : displayInfo
    Deskripsi   : Fungsi untuk menampilkan waktu GPS
    Param       : -
    Return value: -
*/

void displayInfo()
{
  Serial.print(F("Location: "));
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.println();
}
/* ********* TASKS dan fungsi berakhir di sini ********** */
