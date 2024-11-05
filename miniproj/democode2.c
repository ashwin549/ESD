#include <LPC17xx.h>
#include <stdio.h>

int temp1, temp2, flag1;
int lcd_init[] = {0x30, 0x30, 0x30, 0x20, 0x28, 0x0C, 0x01, 0x80, 0x06};
int digital_input;

void port_write() {
    int i;
    LPC_GPIO0->FIOPIN = temp2 << 23;

    if (flag1 == 0)
        LPC_GPIO0->FIOCLR = 1 << 27; // Clear RS
    else
        LPC_GPIO0->FIOSET = 1 << 27; // Set RS

    LPC_GPIO0->FIOSET = 1 << 28; // Set EN
    for (i = 0; i < 50; i++);
    LPC_GPIO0->FIOCLR = 1 << 28; // Clear EN

    for (i = 0; i < 30000; i++);
}

void LCD_write() {
    temp2 = temp1 >> 4;
    port_write();
    temp2 = temp1 & 0xF;
    port_write();
}

int main() {
    int i;
    char digital_status[16];

    // System Initialization
    SystemInit();
    SystemCoreClockUpdate();

    // Configure GPIO pins for LCD
    LPC_PINCON->PINSEL1 = 0; // Clear pin settings
    LPC_GPIO0->FIODIR = 0xF << 23 | 1 << 27 | 1 << 28; // Set LCD control pins

    // Initialize LCD
    flag1 = 0;
    for (i = 0; i <= 8; i++) {
        temp1 = lcd_init[i];
        LCD_write();
    }
    flag1 = 1;

    // Set P0.4 as input and P0.5 as output
    LPC_GPIO0->FIODIR &= ~(1 << 4); // P0.4 as input
    LPC_GPIO0->FIODIR |= (1 << 5);  // P0.5 as output

    // Display initial message on LCD
    flag1 = 0;
    temp1 = 0x80; // First line
    LCD_write();
    flag1 = 1;

    sprintf(digital_status, "Digital IN: ");
    for (i = 0; digital_status[i] != '\0'; i++) {
        temp1 = digital_status[i];
        LCD_write();
    }

    // Main loop
    while (1) {
        // Read digital input from P0.4
        digital_input = (LPC_GPIO0->FIOPIN >> 4) & 0x1; 

        // Update digital status
        if (digital_input) {
            sprintf(digital_status, "HIGH      ");
            LPC_GPIO0->FIOSET = (1 << 5); // Set P0.5 high
        } else {
            sprintf(digital_status, "LOW       ");
            LPC_GPIO0->FIOCLR = (1 << 5); // Set P0.5 low
        }

        // Display digital input status
        flag1 = 0;
        temp1 = 0xC0; // Move to second line
        LCD_write();
        flag1 = 1;

        for (i = 0; digital_status[i] != '\0'; i++) {
            temp1 = digital_status[i];
            LCD_write();
        }

        // Delay
        for (i = 0; i < 100000; i++);
    }
}