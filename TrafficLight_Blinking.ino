#define ledM  19
#define ledK  18
#define ledH  5

void kelip(int warna, int bilangan, int tempoh)
{ for(int i=0;i<=bilangan;i++)
  { digitalWrite(warna,HIGH);  delay(tempoh); 
    digitalWrite(warna,LOW);   delay(tempoh);
  }
}

void setup() 
{ pinMode(ledM, OUTPUT);
  pinMode(ledK, OUTPUT); 
  pinMode(ledH, OUTPUT); 
}

void loop() 
{ digitalWrite(ledK,LOW); digitalWrite(ledM,HIGH); delay(5000);
  kelip(ledM,5,200); 
  digitalWrite(ledM,LOW); digitalWrite(ledH,HIGH); delay(5000);
  kelip(ledH,5,200);
  digitalWrite(ledH,LOW); digitalWrite(ledK,HIGH); delay(2000);
  kelip(ledK,5,200);
}

