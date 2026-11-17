#define RXD2 16
#define TXD2 17
#define baudrate 9600

void readFloat(){
  String Data = Serial2.readStringUntil('\n');
  return receivedData.toFloat();
}

void read3Floats(float *number1, float *number2, float *number3){
  String receivedData = Serial2.readStringUntil('\n');
  char buffer[64];
  receivedData.toCharArray(buffer, sizeof(buffer));
  // Separate values using commas
  char *token = strtok(buffer, ",");

  float number1 = atof(token);

  token = strtok(NULL, ",");
  float number2 = atof(token);

  token = strtok(NULL, ",");
  float number3 = atof(token);
}

void setup() {
  // put your setup code here, to run once:
  // Initialize UART2
    Serial2.begin(baudrate, SERIAL_8N1, RXD2, TXD2);
}

void loop() {
  // put your main code here, to run repeatedly:

}
