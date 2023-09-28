void tampilPengukuran(){
  //lcd.setCursor(0,0);
    lcd.write(0);
    lcd.print(result);
    lcd.print("  ");
    lcd.setCursor(8,0);
    lcd.write(1);
    //lcd.print(" ");
    lcd.print("  ");

}

void splashScreen(){
   lcd.setCursor(0,0);
   lcd.print("    SET POINT      ");
   lcd.setCursor(0,1);
    lcd.write(1);
    //lcd.print((int)ketinggianMin);
    lcd.write(3);
    lcd.print(" ");
    lcd.setCursor(6,1);
    lcd.write(5);
    //lcd.print((int)salinitasMin);
    lcd.print("");
   
}
