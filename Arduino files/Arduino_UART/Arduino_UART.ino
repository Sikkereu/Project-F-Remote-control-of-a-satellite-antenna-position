#define RXD2 8
#define TXD2 9
#define baudrate 115200

String readData(){
  if (Serial2.available()){
    return Serial2.readStringUntil('\n');
  }
}

void setup() {
  // put your setup code here, to run once:
  // Initialize UART2
    Serial2.begin(baudrate, SERIAL_8N1, RXD2, TXD2);
}

void loop() {
  // put your main code here, to run repeatedly:
  readData();
}
