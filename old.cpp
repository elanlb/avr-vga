#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

/* VGA Connector Pin Definitions */
#define VGA_PORT PORTD
#define RED_PIN 4   // Red
#define GREEN_PIN 3 // Green
#define BLUE_PIN 2  // Blue
#define VSYNC_PIN 5 // V-Sync
#define HSYNC_PIN 6 // H-Sync


int main (void)
{
    

    DDRB |= (1 << 5); // data direction of port B
    for (int i = 0; i < 5; i++)
    {
        PORTB |= (1 << 5);
        _delay_ms(300);
        PORTB &= (0 << 5);
        _delay_ms(300);
    }

    DDRD |= 0xFF; // set data direction to output
    VGA_PORT &= 0x00; // clear all bits

    VGA_PORT |= (1 << VSYNC_PIN); // clear V-Sync bit
    VGA_PORT |= (1 << HSYNC_PIN); // clear H-Sync bit

    uint8_t colorByte = 0b00000100; // stores color info as 00000rgb
    uint32_t cycles = 0; // when to change color (replace with timer)

    // main loop
    while (1)
    {
        // sync pulse
        VGA_PORT &= (0 << VSYNC_PIN);
        _delay_ms(0.06); // (0.06 ms)
        VGA_PORT |= (1 << VSYNC_PIN);

        _delay_ms(1.02); // back porch (1.02 ms)
        
        // Horizontal scanline
        for (int line = 0; line < 525; line++)
        {
            VGA_PORT &= (0 << RED_PIN)|(0 << GREEN_PIN)|(0 << BLUE_PIN); // colors off

            // sync pulse
            VGA_PORT &= (0 << HSYNC_PIN);
            _delay_us(3.77); // (3.77 us)
            VGA_PORT |= (1 << HSYNC_PIN);

            VGA_PORT |= ((colorByte >> 2 & 1) << RED_PIN); // red
            VGA_PORT |= ((colorByte >> 1 & 1) << GREEN_PIN); // green
            VGA_PORT |= ((colorByte >> 0 & 1) << BLUE_PIN); // blue

            _delay_us(1.89); // back porch (1.89 us)
            _delay_us(25.17); // active video time (25.17 us)
            _delay_us(0.94); // front porch (0.94 us)
        }
        
        _delay_ms(0.35); // front porch (0.35 ms)

        if (cycles >= 60) // one second
        {
            colorByte++;
            cycles = 0;
        }
        cycles++;
    }
}

void drawVga ()
{
    for (int line = 0; line < 525; line++)
    {
        for (int pixel = 0; pixel < 8; pixel++)
        {

        }
    }
}