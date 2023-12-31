#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define TIEMPO_INICIO 5000
#define VENTANA 100
#define APAGADO 31250
#define ACELERAR 220
#define DOBLAR_SUAVE 210
#define DOBLAR_FUERTE 190
#define ADELANTE 0x03
#define IZQUIERDA 0x02
#define DERECHA 0x01
#define OFFSET_RUEDA 20
#define OFFSET_SUAVE 20
#define OFF 0

volatile uint8_t linea_negra;

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

void set_speed(uint8_t rueda_1, uint8_t rueda_2){
    OCR0A = rueda_1;
    OCR0B = rueda_2;
}

void iniciar_conteo(uint16_t tiempo){
    OCR1A = tiempo;
    TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);
}

void apagar_conteo(void){
    TCCR1B = 0;
    TCNT1 = 0;    
}

ISR(TIMER1_COMPA_vect){
    set_speed(OFF,OFF);
    habilitar_motores(OFF);
    TCCR1B = 0;
}

ISR(TIMER2_COMPA_vect){
    uint8_t estado = PINC & 0x1F;
    if(linea_negra){
        estado = ~estado & 0x1F;
    }
    if(estado == 0x00){
        iniciar_conteo(APAGADO);
        return;
    }
    apagar_conteo();
    if(estado & 0x01){
        set_speed(DOBLAR_FUERTE - OFFSET_RUEDA, DOBLAR_FUERTE);
        habilitar_motores(DERECHA);
        return;
    }
    if(estado & 0x10){
        set_speed(DOBLAR_FUERTE - OFFSET_RUEDA, DOBLAR_FUERTE);
        habilitar_motores(IZQUIERDA);
        return;
    }
    if(estado & 0x02){
        set_speed(DOBLAR_SUAVE - OFFSET_RUEDA, DOBLAR_SUAVE - OFFSET_SUAVE);
        habilitar_motores(ADELANTE);
        return;
    }
    if(estado & 0x08){
        set_speed(DOBLAR_SUAVE - OFFSET_RUEDA - OFFSET_SUAVE, DOBLAR_SUAVE);
        habilitar_motores(ADELANTE);
        return;
    }
    if(estado & 0x04){
        set_speed(ACELERAR - OFFSET_RUEDA, ACELERAR);
        habilitar_motores(ADELANTE);
        return;
    }
}

void init_pwm_motores(void){
    TCCR0A = (1 << COM0A1) | (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
    TCCR0B = (1 << CS00);
    TCNT0 = 0;
}

void init_timer_conteo(void){
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    TIMSK1 = (1 << OCIE1A);
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
    while(1){
        uint8_t estado = PINC & 0x1F;
        if(estado == 0x04){
            linea_negra = 0;
            break;
        }
        if(estado == 0x1B){
            linea_negra = 1;
            break;
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
