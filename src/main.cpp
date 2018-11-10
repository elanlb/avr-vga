#ifndef F_CPU
  #define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>

/* VGA Connector Pin Definitions */
#define VGA_PORT PORTD
#define RED_PIN 4   // Red
#define GREEN_PIN 3 // Green
#define BLUE_PIN 2  // Blue
#define VSYNC_PIN 5 // V-Sync
#define HSYNC_PIN 6 // H-Sync
void setupVga ()
{
    DDRD |= 0xFF; // set data direction to output
    VGA_PORT &= 0x00; // clear all bits

    VGA_PORT |= (1 << VSYNC_PIN); // clear V-Sync bit
    VGA_PORT |= (1 << HSYNC_PIN); // clear H-Sync bit
}

/* TIMER1 Setup for VGA Timing */
#define VGA_TIMER TCNT1
volatile uint16_t line; // volatile because it changes automatically
void setupVgaTimer ()
{
    TCCR1A = 0x00;
    TCCR1B = 0x00;
    TCCR1B |= (1 << WGM12)|(1 << CS00); // Mode: CTC, Prescaler: 0
    TCNT1 = 0; // initialize timer
    OCR1A = 5; // Compare value A: 5
    OCR1B = 508; // Compare value B: 800/1.5734375=508
    TIMSK1 |= (1 << OCIE1A)|(1 << OCIE1B); //
    sei(); // enable global interrupts
}
ISR(TIMER1_COMPA_vect)
{
    VGA_PORT &= ~(1 << HSYNC_PIN);
    TCNT1 = 6; // don't reset the timer
}
ISR(TIMER1_COMPB_vect)
{
    line++;
    TCNT1 = 0;
}

int main (void)
{
    DDRB |= (1 << 5);
    PORTB &= ~(1 << 5);

    // initialize different parts of the project
    setupVga();
    setupVgaTimer();

    uint8_t colorByte = 0b00000101; // stores color info as 00000rgb
    line = 0; // current scanline on screen (0-525)

    // main loop
    while (1)
    {
        /* Calculating pixel:
        VGA uses 25.175 MHz
        Arduino uses 16 MHz
        25.175/16 = 1.5734375
        timer * 1.5734375 = VGA ticks
        -- */
        
        /* 800 Pixels:
        0-7 Front porch (8)
        8-103 Horizontal sync (96)
        104-143 Back porch (40)
        144-151 Left border (8)
        152-791 Video (640)
        792-799 Right border (8)
        -- */
        
        if (~(VGA_PORT >> HSYNC_PIN) & 1)
        {
            VGA_PORT |= (1 << HSYNC_PIN);
        }
        /*
        if (VGA_TIMER > 69) // 103/1.5=69
        {
            VGA_PORT |= (1 << HSYNC_PIN);
        } */

        /* 525 Lines:
        0-1 Front porch (2)
        2-3 Vertical sync (2)
        4-28 Back porch (25)
        29-36 Top border (8)
        37-516 Video (480)
        517-524 Bottom border (8)
        -- */
        //if (line >= 2 && line <= 3)
        if (line == 2)
        {
            VGA_PORT &= ~(1 << VSYNC_PIN); // sync pulse on (logic low)
        }
        else
        {
            VGA_PORT |= (1 << VSYNC_PIN); // sync pulse off (logic high)
        }

        if (line > 524)
        {
            line = 0;
        }

        // main draw
        if (line >= 37 && line <= 516)
        {
            VGA_PORT |= ((colorByte >> 2 & 1) << RED_PIN); // red
            VGA_PORT |= ((colorByte >> 1 & 1) << GREEN_PIN); // green
            VGA_PORT |= ((colorByte >> 0 & 1) << BLUE_PIN); // blue
        }
        else
        {
            VGA_PORT &= ~(1 << RED_PIN)|(1 << GREEN_PIN)|(1 << BLUE_PIN); // colors off
        }
    }
}
