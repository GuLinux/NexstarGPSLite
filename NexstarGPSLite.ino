void setup() {
  while(!Serial)
    delay(100);
  Serial.begin(9600);
  Serial3.begin(9600);
  Serial.println("Initialising BT\n");
  Serial3.println("AT");
}

void loop() {
  while(Serial.available()) {
    Serial3.write(Serial.read());
  }
  while(Serial3.available()) {
    Serial.write(Serial3.read());
  }
}
