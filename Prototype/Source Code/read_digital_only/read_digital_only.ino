int i;
int sign;
long value;
float result;
int clockpin = 4;  
int datapin = 5;
unsigned long tempmicros;

void setup() {
  Serial.begin(9600);
  pinMode(clockpin, INPUT);
  pinMode(datapin, INPUT);
  Serial.println("Mulai baca caliper");
}

void loop () {
  Serial.print("hasil clockpin: ");
  Serial.print(digitalRead(clockpin));
  Serial.print(" - hasil datapin: ");
  Serial.println(digitalRead(datapin));
  delay(10);
}
