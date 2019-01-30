#include "SSD2119.h"


extern unsigned short cursorX;
extern unsigned short cursorY;

int x = 0;
int y = 0;
int i = 0;  

const unsigned char testSprite4BPP[] = {
 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x1F,
 0xF1, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x1F,
 0xF1, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x1F,
 0xF1, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x1F,
 0xF1, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x1F,
 0xF1, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x1F,   
 0xF1, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x1F,
 0xF1, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF    
};    


void PrintAsciiChars(){
    unsigned char i;
    LCD_SetCursor(0, 0);
    for (i = 0x20; i < 0x7F; i++){
        LCD_PrintChar(i);
    }   
}

void BlastChars(){
    LCD_SetCursor(0, 0);
    for (;;){
        LCD_PrintChar((Random() % 96) + 0x20);
        //LCD_SetTextColor(Random() % 255, Random() % 255, Random() % 255);
    }   
}

void TestStringOutput(){
    LCD_SetCursor(0, 0);
    LCD_PrintString("This is just a test string, please ignore.\nI repeat, this is just a test string, please ignore.");
}

void RandomRectangles(){
    LCD_DrawRect(Random() % 200, Random() % 160, Random() % 160, Random() %100, convertColor(Random()%3 * 127, Random()%3 * 127, Random()%3 * 127));
}

void MovingColorBars(){
    LCD_DrawPixelRGB(x, y, 0, 0, 255);
    LCD_DrawPixelRGB(x+30, y, 0, 255, 0);
    LCD_DrawPixelRGB(x+60, y, 255, 0, 0);

    x++;
    y++;
    if (x > 320) { x = 0; };
    if (y > 240) { y = 0; };    
}

void RandomColorFill(){
    long i;
    //long color = convertColor(Random() % 255,Random() % 255,Random() % 255);        
    long color = convertColor(255,Random() % 255,Random() % 255);         
    for(i = 0; i < (320 * 240); i++)
    {
        LCD_WriteData(color);
    }    
}

void BWWaitFill(){
    LCD_ColorFill(convertColor(255,255,255));
    for (i = 0; i < 5000000; i++) {}
    LCD_ColorFill(convertColor(0,0,0));
    for (i = 0; i < 5000000; i++) {}            
}

void LineSpin(){
    long color = convertColor(Random() % 256,Random() % 256,Random() % 256);
    for (x = 0; x < 320; x++){
        LCD_DrawLine(160, 120, x, 0, color);
    }
    color = convertColor(Random() % 256,Random() % 256,Random() % 256);        
    for (y = 0; y < 240; y++) {
        LCD_DrawLine(160, 120, 320, y, color);
    }
    color = convertColor(Random() % 256,Random() % 256,Random() % 256);        
    for (x = 320; x > 0; x--){
        LCD_DrawLine(160, 120, x, 240, color);
    }
    color = convertColor(Random() % 256,Random() % 256,Random() % 256);        
    for (y = 240; y > 0; y--) {
        LCD_DrawLine(160, 120, 0, y, color);
    }
}

void printfTest(){
    unsigned char x = 51;
    LCD_SetCursor(0,0);
    printf("hihi %s %c %d\nDecimal: %d\nHex:     %x\nBinary:  %b\n", "Bob", 'a', x, i, i, i);
    i++;
}

void charTest(){
    unsigned char i = 0;
    
    LCD_Goto(0,0);
    //for(i=0;i<52;i++){    
        printf("%d", i%10);
    //}
    //LCD_Goto(2,0);
    //for(i=0;i<27;i++){    
    //    printf("%d", i%10);
    //    LCD_Goto(2, i+1);
    //}
}

void sizeofStuff(){
    printf("Size of       INT: %d\n", sizeof(int));
    printf("Size of      CHAR: %d\n", sizeof(char));
    printf("Size of     SHORT: %d\n", sizeof(short));
    printf("Size of      LONG: %d\n", sizeof(long));
    printf("Size of     FLOAT: %d\n", sizeof(float));
    printf("Size of    DOUBLE: %d\n", sizeof(double));
}

void RandomCircle(){
    LCD_DrawFilledCircle(Random()%280 + 20, Random()%200 + 20, 20, Random()%0xFFFF);    
}

void Random4BPPTestSprite(){
    LCD_DrawImage(testSprite4BPP, Random()%304, Random()%224, 16, 16, 4);
}



/////////////////////////////////////////////// TOUCHSCREEN STUFF ////////////////////////////////////

long yVal = 0;
long xVal = 0;
long z1Val = 0;
long z2Val = 0;
long Touched = 0;
long TouchCounter = 0;

void touchDebug(){
        long temp, blahX, blahY; 
        
        
        // do stuff every however often here
        yVal = Touch_ReadY();      // 24-14
        xVal = Touch_ReadX();      // 29-16
        z1Val = Touch_ReadZ1();
        //z2Val = Touch_ReadZ2();

        temp = Touch_GetCoords();
        blahX = (temp >> 16);
        blahY = temp & 0xFFFF;
//        if (Touched) {
//            if (TouchCounter == 0){
//                LCD_Goto(20, 20);
//                printf("touched!");
//            }
//            TouchCounter++;
//            if (TouchCounter >= 10){
//                LCD_Goto(20,20);
//                printf("        ");
//                TouchCounter = 0;
//                Touched = 0;
//            }   
//        }
        
        //Print_TouchCoords();
//        if (xVal < 100) xVal = 0;
//        if (yVal < 170) yVal = 0;
        
        LCD_Goto(0,0);
        printf("xVal = %d   \nyVal = %d   \nZ1Val = %d   \nZ2Val = %d   \nxPos = %d   \nyPos = %d   \n", xVal, yVal, z1Val, z2Val, blahX, blahY);
}

//void Print_TouchCoords(void){
//    coord temp;
//    
//    temp = Touch_GetCoords();
//    
//    LCD_Goto(0,0);
//    printf("xVal = %d  \nyVal = %d   \n%d", temp.x, temp.y, n);
//    
//}
