#define RXD2 6
#define TXD2 7
#define baudrate 115200

String readData(){
  String received;
  if (Serial1.available() > 0){
    Serial.println("entered function");
    received = Serial1.readStringUntil('\n');
    Serial.println(received);
  }
  return received;
}

void getIDPayload(String &ID, char *payload, int payloadSize){
  String data = readData();
  int seperator = data.indexOf(',');
  if(seperator != -1){
    ID = data.substring(0,seperator);
    data = data.substring(seperator + 1);
    data.toCharArray(payload,payloadSize);
    Serial.println(ID);
    Serial.println(payload);
    data = "";
    seperator = -1;
  }
}

void setup() {
  // put your setup code here, to run once:
  // Initialize UART
  Serial.begin(9600);
  Serial1.begin(baudrate, SERIAL_8N1, RXD2, TXD2);
  delay(5000);
  Serial.println("Started");
    
}

String ID;
char payload[128];
void loop() {
  // put your main code here, to run repeatedly:
  //getIDPayload(ID, payload, sizeof(payload));
  //readData();
  Serial1.println("3.14");
  delay(3000);
}
