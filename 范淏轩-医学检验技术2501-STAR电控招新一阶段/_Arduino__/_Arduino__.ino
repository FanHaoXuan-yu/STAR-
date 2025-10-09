void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int onTime=1000;
  int offTime=1000;
  digitalWrite(LED_BUILTIN,HIGH);
  delay(onTime);
  digitalWrite(LED_BUILTIN,LOW);
  delay(offTime);
}
