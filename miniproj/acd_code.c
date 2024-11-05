#include <lpc17xx.h>
#include <stdio.h>

#define digitalMax 0xFFF      // ADC maximum value (12-bit)
#define RS_CTRL  0x00000100    // P0.8 (RS pin for LCD)
#define EN_CTRL  0x00000200    // P0.9 (EN pin for LCD)
#define DT_CTRL  0x000000F0    // P0.4 to P0.7 (Data pins for LCD)

unsigned long int init_command[] = {0x30, 0x30, 0x30, 0x20, 0x28, 0x0c, 0x06, 0x01, 0x80};
unsigned long int temp1 = 0, temp2 = 0, i, j;
unsigned char flag1 = 0, flag2 = 0;
unsigned char msg[] = {"ADC Value:"};
unsigned char no_signal_msg[] = {"No Signal"};

void lcd_init(void);
void lcd_write(void);
void port_write(void);
void delay(unsigned int);
void lcd_print_msg(void);
void buzz(void);

void buzz(){
		int i;
    // Set P0.4 as output
    LPC_GPIO0->FIODIR |= (1 << 15);  // Configure P0.4 as output

    // Set P0.4 high
    LPC_GPIO0->FIOSET = (1 << 15);    // Set P0.4 high
		for (i = 0; i < 10000; i++);
}

int main(void) {
    unsigned int mqReading, j;
    char adcStr[14];

    // System Initialization
    SystemInit();
    SystemCoreClockUpdate();

    LPC_PINCON->PINSEL1 |= 1<<14; // Set P0.23 as analog input (AD0.0)
    LPC_SC->PCONP |= (1<<12);    // Enable ADC peripheral power
    LPC_GPIO0->FIODIR = DT_CTRL | RS_CTRL | EN_CTRL | (1 << 5); // Configure pins as output for LCD and buzzer (P0.5)

    lcd_init();
    lcd_print_msg();

    while(1) {
        float total = 0;
        int samples = 10; // Number of samples to average

        // Take multiple ADC readings to average
        for (j = 0; j < samples; j++) {
            LPC_ADC->ADCR = (1<<0) | (1<<21) | (1<<24); // Start ADC conversion for channel 0 (P0.23)
            while(((mqReading = LPC_ADC->ADGDR) & 0X80000000) == 0); // Wait for conversion to complete
            mqReading = LPC_ADC->ADGDR;
            mqReading >>= 4; // Extract ADC result
            total += mqReading; // Sum readings
            delay(1000); // Short delay between readings
        }
       
        mqReading = total / samples; // Calculate average reading

        // If the ADC value is below a threshold, consider it as "no signal"
        if (mqReading < 10) {
            // Display "No Signal" if ADC reading is too low (likely floating)
            sprintf(adcStr, "%s", no_signal_msg);
        } else {
            // Convert ADC value to string
            sprintf(adcStr, "%d", mqReading);
        }

        // Clear the second line of the LCD before displaying the ADC value
        temp1 = 0xC0; // Move cursor to the second line
        flag1 = 0; // Command mode
        lcd_write();
        delay(16000); // Delay to ensure the LCD has time to update

        // Display the ADC value or "No Signal" on LCD
        i = 0;
        flag1 = 1; // Data mode
        while (adcStr[i] != '\0') {
            temp1 = adcStr[i];
            lcd_write();
            i += 1;
        }
				//buzz();
        // Buzzer activation when ADC exceeds a threshold (e.g., 2000)
        if (mqReading > 300) {
            buzz(); // Activate buzzer if ADC value is above threshold
        }

        // Delay to keep the ADC reading visible for a while
        delay(1000000); // Delay to keep the ADC displayed before the next reading
    }
}

void lcd_init(void) {
    unsigned int x;
    flag1 = 0; // Command Mode
    for (x = 0; x < 9; x++) {
        temp1 = init_command[x];
        lcd_write();
    }
    flag1 = 1; // Data Mode
}

void lcd_write(void) {
    flag2 = (flag1 == 1) ? 0 : ((temp1 == 0x30) || (temp1 == 0x20)) ? 1 : 0; // Determine if sending command or data
    temp2 = temp1 & 0xf0; // Most significant 4 bits
    port_write(); // Write to LCD
    if (flag2 == 0) { // Send least significant 4 bits only when it is data/command other than 0x30/0x20
        temp2 = temp1 & 0x0f;
        temp2 = temp2 << 4;
        port_write();
    }
}

void port_write(void) {
    LPC_GPIO0->FIOPIN = temp2; // Write data to LCD
    if (flag1 == 0)  
        LPC_GPIO0->FIOCLR = RS_CTRL; // Command mode
    else
        LPC_GPIO0->FIOSET = RS_CTRL; // Data mode
    LPC_GPIO0->FIOSET = EN_CTRL; // Enable LCD
    delay(5000); // Delay to allow LCD to process
    LPC_GPIO0->FIOCLR = EN_CTRL; // Disable LCD
    delay(600000); // Additional delay
}

void delay(unsigned int r1) {
    unsigned int r;
    for (r = 0; r < r1 * 2; r++); // Adjusted delay
}

void lcd_print_msg(void) {
    unsigned int a;
    // Print static message "ADC Value:" to LCD
    for (a = 0; msg[a] != '\0'; a++) {
        temp1 = msg[a];
        lcd_write();
    }
}

