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

/* TIMER0 Setup for VGA Timing */
#define VGA_TIMER TCNT0
volatile uint32_t vgaTimerOverflow; // volatile because it changes automatically
void setupTimer0 ()
{
    TCCR0A = 0x00;
    TCCR0B |= (1 << CS00); // no prescaler
    TCNT0 = 0; // initialize timer
    TIMSK0 |= (1 << TOIE0); // enable overflow interrupt
    sei(); // enable global interrupts
    
    vgaTimerOverflow = 0; // initialize overflow counter
}
ISR(TIMER0_OVF_vect) // called whenever TCNT0 overflows
{
    vgaTimerOverflow++; // increase overflow counter
}

int main (void)
{
    DDRB |= (1 << 5);
    PORTB &= (0 << 5);

    // initialize different parts of the project
    setupVga();
    setupTimer0();

    uint8_t colorByte = 0b00000101; // stores color info as 00000rgb
    uint16_t line = 0; // current scanline on screen (0-525)
    uint16_t pixel = 0; // current pixel across line (0-800)

    // main loop
    while (1)
    {
        /* Calculating line and pixel:
        VGA uses 25.175 MHz
        Arduino uses 16 MHz
        25.175/16 = 1.5734375
        timer * 1.5734375 = VGA ticks
        -- */
        #define VGA_TIMING_CONSTANT 1 //.5734375
        line = (int) ((vgaTimerOverflow * 0xFF + VGA_TIMER) * VGA_TIMING_CONSTANT / 525);

        /* Lines:
        0-1 Front porch (2)
        2-3 Vertical sync (2)
        4-28 Back porch (25)
        29-36 Top border (8)
        37-516 Video (480)
        517-524 Bottom border (8)
        -- */
        if (line >= 2 && line <= 3)
        {
            VGA_PORT &= (0 << VSYNC_PIN); // sync pulse on (logic low)
        }
        else if (line > 3)
        {
            VGA_PORT |= (1 << VSYNC_PIN); // sync pulse off (logic high)
        }
        
        if (line >= 37 && line <= 516)
        {
            pixel = (int) ((vgaTimerOverflow * 0xFF + VGA_TIMER) * VGA_TIMING_CONSTANT) - line * 800;

            /* Pixels:
            0-7 Front porch (8)
            8-103 Horizontal sync (96)
            104-143 Back porch (40)
            144-151 Left border (8)
            152-791 Video (640)
            792-799 Right border (8)
            -- */
            VGA_PORT &= (0 << RED_PIN)|(0 << GREEN_PIN)|(0 << BLUE_PIN); // colors off

            // sync pulse
            if (line >= 8 && line <= 103)
            {
                VGA_PORT &= (0 << HSYNC_PIN); // sync pulse on (logic low)
            }
            else if (line > 103)
            {
                VGA_PORT |= (1 << HSYNC_PIN); // sync pulse off (logic high)
            }

            VGA_PORT |= ((colorByte >> 2 & 1) << RED_PIN); // red
            VGA_PORT |= ((colorByte >> 1 & 1) << GREEN_PIN); // green
            VGA_PORT |= ((colorByte >> 0 & 1) << BLUE_PIN); // blue
        }

        if (line >= 524)
        {
            VGA_TIMER = 0;
            vgaTimerOverflow = 0;
            line = 0;
        }
    }
}
