#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define TIEMPO_INICIO 5000
#define VENTANA 100
#define OFF 0

volatile uint8_t color_pista;

ISR(TIMER1_COMPA_vect){
    TCCR1B = 0;
    set_speed(OFF);
    habilitar_motores(OFF);
}

ISR(TIMER2_COMPA_vect){

}

void init_pwm_motores(void){
    TCCR0A = (1 << COM0A1) | (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
    TCCR0B = (1 << CS00);
    TCNT0 = 0;
}

void set_speed(uint8_t velocidad){
    OCR0A = velocidad;
    OCR0B = velocidad;
}

void habilitar_motores(uint8_t modo){
    PORTB &= 0xEF;
    for(uint8_t i = 0; i < 8; i++){
        PORTD &= 0xEF;
        if((modo & (1 << i)) == 0){
            PORTB &= 0xFE;
        } else{
            PORTB |= 0x01;
        }
        PORTD |= 0x10;
    }
    PORTB |= 0x10;
}

void init_timer_conteo(void){
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    TIMSK1 = (1 << OCIE1A);
}

void iniciar_conteo(uint8_t tiempo){
    OCR1A = tiempo;
    TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);
}

void init_timer_sensores(void){
    TCCR2A = (1 << WGM21);
    TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);
    TCNT2 = 0;
    OCR2A = VENTANA;
    TIMSK2 = (1 << OCIE2A);
}

void init_pins(void){
    DDRB = 0x11;
    DDRC = 0x00;
    DDRD = 0xF0;
}

void determinar_pista(void){
    color_pista = 0;
    while(!color_pista){
        uint8_t estado = PINC;
        if(estado == 0x1B){
            color_pista = 1;
        }
        if(estado == 0x04){
            color_pista = 2;
        }
    }
}

void reset(void){
    init_pins();
    determinar_pista();
    init_pwm_motores();
    init_timer_conteo();
    init_timer_sensores();
    sei();
}

int main(void){
    _delay_ms(TIEMPO_INICIO);
    reset();
    while(1){
    }
    return 0;
}
