/*
 * File:   LAB0.c
 * Author: Jos? Santizo
 *
 * Creado el 17 de enero de 2022
 * 
 * Descripci?n: Carrera entre 2 jugadores
 */

//---------------------Bits de configuraci?n-------------------------------
// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.


//-----------------Librer?as utilizadas en c?digo-------------------- 
#include <xc.h>
#include <stdint.h>
#include <stdio.h>

//-----------------------Constantes----------------------------------
#define  valor_tmr0 61                                          // valor_tmr0 = 61 (0.05 ms)

//-----------------------Variables------------------------------------
int cont = 0;                                                   // Variable de contador             
int cont_display = 3;                                           // Variable con la que empieza el display de 7 segmentos en el sem?foro
int timer_iniciado = 0;                                         // Variable que indica si se inici? el timer de la cuenta atr?s
int cuenta_atras = 1;                                           // Variable que indica si se inici? la cuenta atr?s del sem?foro
int inicio_carrera = 0;                                         // Variable que indica si se pueden mover los jugadores en la carrera

//------------Funciones sin retorno de variables----------------------
void setup(void);                                               // Funci?n de setup
void tmr0(void);                                                // Funci?n para reiniciar TMR0

//-------------Funciones que retornan variables-----------------------
int  tabla(int a);                                              // Tabla para traducir valores a displays de 7 segmentos
int  tabla_p(int a);                                            // Tabla que traduce valores a displays de 7 segmentos pero con punto decimal incluido

//----------------------Interrupciones--------------------------------
void __interrupt() isr(void){
    if(T0IF){                                                   // Si la bandera del timer 0 se activa, entonces proceder a realizar el c?digo siguiente
        
        //------------------Sem?foro-----------------------------------
        if(cuenta_atras == 1){                                  // Si bandera de cuenta atr?s est? activa, entonces: 
            if(timer_iniciado == 0){                            // Revisar si el timer para cuenta atr?s ha iniciado
                while (PORTEbits.RE0 == 0);                     // Si timer no ha iniciado, revisar si se ha presionado el bot?n en el bit 0 del puerto E
                timer_iniciado = 1;                             // Si bot?n en bit 0 del puerto E es presionado, activar la bandera de timer iniciado
            }
            tmr0();                                             // Reiniciar TMR0
            cont ++;                                            // Incrementar variable cont en 1 cada 0.05 segundos
            if(cont == 20){                                     // Si pasa 1 segundo (0.05 segundos * 20 = 1 segundo),
                PORTD = 1;                                      //      entonces PORTD = 1
                cont_display -= 1;                              //      y cont_display = 2
            }
            else if(cont == 40){                                // Si pasan 2 segundos (0.05 segundos * 40 = 2 segundos),
                PORTD = 3;                                      //      entonces PORTD = 3
                cont_display -= 1;                              //      y cont_display = 1
                PORTA = 1;                                      // Se colocan a los jugadores 1 y 2 en su posici?n inicial
                PORTB = 1;
            }
            else if(cont >= 60){                                // Si pasan 3 segundos o m?s (0.05 segundos * 60 = 3 segundos),
                PORTD = 7;                                      //      entonces PORTD = 7
                cont = 0;                                       //      contador de segundos = 0
                cont_display = 0;                               //      cont_display = 0
                cuenta_atras = 0;                               //      cuenta atr?s finaliza
                inicio_carrera = 1;                             //      Se activa la bandera que indica si se puede iniciar la carrera o no
            }
            PORTC = tabla(cont_display);                        // Todo el tiempo, el puerto C toma el valor de cont_display luego de ser traducido a la tabla de 7 segmentos
        }
        
        //----------------Carrera-------------------------------
        else if(cuenta_atras == 0){
            if(PORTEbits.RE1 == 1 && inicio_carrera == 1){      // Si la bandera de inicio de carrera est? activada y se presiona el bot?n del jugador 1, entonces realizar todo el siguiente procedimiento
                while(PORTEbits.RE1);                           // Antirebote para que no se realice nada hasta que el bot?n en RE1 se suelte
                PORTA = PORTA*2;                                // Se hace el contador de d?cadas multiplicando por 2 el valor del puerto. Esto genera ascensos de 2 en 2
            }
            if(PORTEbits.RE2 == 1 && inicio_carrera == 1){      // Si la bandera de inicio de carrera est? activada y se presiona el bot?n del jugador 2, entonces realizar todo el siguiente procedimiento
                while(PORTEbits.RE2);                           // Antirebote para que no se realice nada hasta que el bot?n en RE2 se suelte
                PORTB = PORTB*2;                                // Se hace el contador de d?cadas multiplicando por 2 el valor del puerto. Esto genera ascensos de 2 en 2
            }
            if(PORTA == 128 && PORTB < 128){                    // Si puerto A = 128 y puerto B es menor a 128, entonces
                PORTDbits.RD6 = 1;                              //      Bit 6 del puerto D se prende, indicando que el ganador es el jugador 1
                PORTC = 0b00000110;                             //      PORTC = 6 => n?mero 1 en display de 7 segmentos
            }
            if(PORTB == 128 && PORTA < 128){                    // Si puerto B = 128 y puerto A es menor a 128, entonces
                PORTDbits.RD7 = 1;                              //      Bit 7 del puerto D se prende, indicando que el ganador es el jugador 2
                PORTC = 0b01011011;                             //      PORTC = 91 => n?mero 2 en display de 7 segmentos
            }
        }
        
        //---------------------Bot?n para reiniciar el juego---------------------
        if(PORTEbits.RE3){                                      // Si el bot?n en RE3 es presionado, entonces:
            while(PORTEbits.RE3);                               //      Antirebote para que no reinicie r?pidamente
            cont_display = 3;                                   //      cont_display adquiere valor inicial de 3
            timer_iniciado = 0;                                 //      timer_iniciado adquiere valor inicial de 0
            cuenta_atras = 1;                                   //      cuenta_atras adquiere valor inicial de 1
            inicio_carrera = 0;                                 //      inicio_carrera adquiere valor inicial de 0
            cont = 0;                                           //      cont adquiere valor inicial de 0
            PORTA = 0;                                          //      PORTA adquiere valor inicial de 0
            PORTB = 0;                                          //      PORTB adquiere valor inicial de 0
            PORTC = 0;                                          //      PORTC adquiere valor inicial de 0
            PORTD = 0;                                          //      PORTD adquiere valor inicial de 0
        }
        
    }
    
}

//----------------------Main Loop--------------------------------
void main(void) {
    setup();                                                    // Subrutina de setup
    while(1){                                                   // Loop principal
        
    }
}

//----------------------Subrutinas--------------------------------
void setup(void){
    
    //Configuraci?n de entradas y salidas
    ANSEL = 0;                                                  // Pines 0 y 1 de PORTA como anal?gicos
    ANSELH = 0;
    
    TRISA = 0;                                                  // PORTA como salida
    TRISB = 0;                                                  // PORTB como salida
    TRISC = 0;                                                  // PORTC como salida
    TRISD = 0;                                                  // PORTD como salida                           
    TRISE = 0b00000111;                                         // PORTE como entrada
    
    PORTA = 0;                                                  // Limpiar PORTA
    PORTD = 0;                                                  // Limpiar PORTD
    PORTB = 0;                                                  // Limpiar PORTB
    PORTC = 0;                                                  // Limpiar PORTC
    
    //Configuraci?n de oscilador
    OSCCONbits.IRCF = 0b0110;                                   // Oscilador a 4 MHz = 110
    OSCCONbits.SCS = 1;
    
    //Configuraci?n de TMR0
    OPTION_REGbits.T0CS = 0;                                    // bit 5  TMR0 Clock Source Select bit...0 = Internal Clock (CLKO) 1 = Transition on T0CKI pin
    OPTION_REGbits.T0SE = 0;                                    // bit 4 TMR0 Source Edge Select bit 0 = low/high 1 = high/low
    OPTION_REGbits.PSA = 0;                                     // bit 3  Prescaler Assignment bit...0 = Prescaler is assigned to the WDT
    OPTION_REGbits.PS2 = 1;                                     // bits 2-0  PS2:PS0: Prescaler Rate Select bits
    OPTION_REGbits.PS1 = 1;
    OPTION_REGbits.PS0 = 1;
    TMR0 = valor_tmr0;                                          // preset for timer register
    
    //Configuraci?n de interrupciones
    INTCONbits.T0IF = 0;                                        // Habilitada la bandera de TIMER 0      
    INTCONbits.T0IE = 1;                                        // Habilitar las interrupciones de TIMER 0
    INTCONbits.GIE = 1;                                         // Habilitar interrupciones globales
    INTCONbits.PEIE = 1;                                        // Interrupciones perif?ricas activadas
    
    return;
}

void tmr0(void){
    INTCONbits.T0IF = 0;                                        // Limpiar bandera de TIMER 0
    TMR0 = valor_tmr0;                                          // TMR0 = 61
    return;
}

int tabla(int a){
    switch (a){
        case 0:
            return 0b00111111;                                  // Si se presenta n?mero 0 en binario, retornar 00111111 (n?mero 0 en display de 7 segmentos)
            break;
        case 1:
            return 0b00000110;                                  // Si se presenta n?mero 1 en binario, retornar 00000110 (n?mero 1 en display de 7 segmentos)
            break;
        case 2:
            return 0b01011011;                                  // Si se presenta n?mero 2 en binario, retornar 01011011 (n?mero 2 en display de 7 segmentos)     
            break;
        case 3:
            return 0b01001111;                                  // Si se presenta n?mero 3 en binario, retornar 01001111 (n?mero 3 en display de 7 segmentos)
            break;
        case 4:
            return 0b01100110;                                  // Si se presenta n?mero 4 en binario, retornar 01100110 (n?mero 4 en display de 7 segmentos)
            break;
        case 5:
            return 0b01101101;                                  // Si se presenta n?mero 5 en binario, retornar 01101101 (n?mero 5 en display de 7 segmentos)
            break;  
        case 6:
            return 0b01111101;                                  // Si se presenta n?mero 6 en binario, retornar 01111101 (n?mero 6 en display de 7 segmentos)
            break;
        case 7:
            return 0b00000111;                                  // Si se presenta n?mero 7 en binario, retornar 00000111 (n?mero 7 en display de 7 segmentos)
            break;
        case 8:
            return 0b01111111;                                  // Si se presenta n?mero 8 en binario, retornar 01111111 (n?mero 8 en display de 7 segmentos)
            break;
        case 9:
            return 0b01101111;                                  // Si se presenta n?mero 9 en binario, retornar 01101111 (n?mero 8 en display de 7 segmentos)
            break;
        default:
            break;
            
    }
}