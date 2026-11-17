#define RXD2 6
#define TXD2 7
#define baudrate 115200

void readData(){
  /*
  if (Serial2.available()){
    Serial.println("entered function");
    String temp = Serial2.readStringUntil('\n');
    Serial.println(temp);
    return temp;
  }
  */
  while (Serial2.available())
    {
        Serial.println("entered function");
        char received = Serial2.read();
        Serial.print(received);
    }
    

}

void setup() {
  // put your setup code here, to run once:
  // Initialize UART
  Serial.begin(115200);
  Serial2.begin(baudrate, SERIAL_8N1, RXD2, TXD2);
  delay(3000);
  Serial.println("Started");
    
}

void loop() {
  // put your main code here, to run repeatedly:
  readData();

}
