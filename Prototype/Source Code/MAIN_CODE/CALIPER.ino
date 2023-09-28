void decode() {
  sign=1;
  value=0;
  for (i=0;i<23;i++) {
    while (digitalRead(clockpin)==HIGH) { } //wait until clock returns to HIGH- the first bit is not needed
    while (digitalRead(clockpin)==LOW) {} //wait until clock returns to LOW
    if (digitalRead(datapin)==LOW) {
      if (i<20) {
        value|= 1<<i;
      }
      if (i==20) {
        sign=-1;
      }
    }
  }
  result=(value*sign)/100.00;  
  
  delay(100);
}     
