#define ledM  19
#define ledK  18
#define ledH  5
void setup() 
{ pinMode(ledM, OUTPUT);
  pinMode(ledK, OUTPUT); 
  pinMode(ledH, OUTPUT); 
}
void loop() 
{ digitalWrite(ledK,LOW); digitalWrite(ledM,HIGH); delay(5000); 
  digitalWrite(ledM,LOW); digitalWrite(ledH,HIGH); delay(5000);
  digitalWrite(ledH,LOW); digitalWrite(ledK,HIGH); delay(2000);
}
