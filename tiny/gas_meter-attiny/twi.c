#include "twi.h"

/* Initialize TWI slave mode */
void twi_slave_init( ) {
    SET_SDA();
    TWI_INT_INIT();
    TWSR = I2C_IDLE;
}

/* Enable twi slave */
void twi_slave_enable( ) {
    TWI_INT_CLEAR_FLAG();
    TWI_INT_ENABLE();
}

/* Interrupt service routine for negative egde on SDA */
ISR(PCINT0_vect) {

    if (TWSR == I2C_IDLE) {
        get_start_condition();
    }

    /* TWI state machine */
    START:
    switch (TWSR) {

        // Own SLA_R has been received, ACK has been returned
        case TWI_SLA_REQ_R_ACK_RTD:
            i2c_buffer_idx = 0;
            TWDR = i2c_buffer[0];
            i2c_buffer_idx++;
            send_data();
            goto START;
            break;

            // data has been transmitted, ACK has been received.
        case TWI_SLA_DATA_SND_ACK_RCV:
            if (i2c_buffer_idx < I2C_BUFFER_SIZE) {
                TWDR = i2c_buffer[i2c_buffer_idx];
                i2c_buffer[0] += i2c_buffer[i2c_buffer_idx]; //checksum
                i2c_buffer[i2c_buffer_idx] = 0;
                i2c_buffer_idx++;
            } else {
                TWDR = i2c_buffer[0] + 64;
                i2c_buffer[0] = 0;
            }
            send_data();
            goto START;
            break;

            // last data has been transmitted, ACK has been received.
        case TWI_SLA_LAST_DATA_SND_ACK_RCV:

            // data has been transmitted, NACK has been received.
        case TWI_SLA_DATA_SND_NACK_RCV:

            // met stop or repeat start
        case TWI_SLA_STOP:
            TWSR = I2C_IDLE;
            break;

        case TWI_SLA_REPEAT_START:
            get_start_condition();
            goto START;

            // Idle or bus error
        case I2C_IDLE:
        default:
            break;
    }

    TWI_INT_CLEAR_FLAG();
    TWI_INT_ENABLE();

}

/* Read the slave byte after start condition  */
inline uint8_t read_byte(void) {
    uint8_t val = 0;
    uint8_t cPin = 0;

    // Let SCL go low first. MCU comes here while SCL is still high
    while (GET_SCL());

    //Read 8 bits from master, respond with ACK. 
    for (uint8_t index = 0; index < 8; index++) {

        while (!GET_SCL());
        cPin = GET_SDA();

        val = (val << 1) | cPin;
        while (GET_SCL()) {
            //if SDA changes while SCL is high, it indicates STOP or START
            if ((val & 1) != cPin) {
                if (GET_SDA())
                    TWSR = TWI_SLA_STOP;
                else
                    TWSR = TWI_SLA_REPEAT_START;
                return 0;
            } else
                cPin = GET_SDA();
        }
    }

    // Send ACK, SCL is low now
    if ((val & 0xFE) == (SLAVE_ADDRESS << 1)) {
        CLR_SDA();
        while (!GET_SCL());
        while (GET_SCL());
        SET_SDA();
        CLR_SCL();
    } else {
        TWSR = I2C_IDLE;
        return 0;
    }
    return val;
}

/* TWI slave send data */
inline void send_data(void) {
    for (uint8_t index = 0; index < 8; index++) {
        while (GET_SCL());
        if ((TWDR >> (7 - index)) & 1)
            SET_SDA();
        else
            CLR_SDA();
        SET_SCL();
        while (!GET_SCL());
    }

    // See if we get ACK or NACK
    while (GET_SCL());

    // tristate the pin to see if ack comes or not
    SET_SDA();

    while (!GET_SCL());
    if (!GET_SDA())
        TWSR = TWI_SLA_DATA_SND_ACK_RCV;
    else
        TWSR = TWI_SLA_DATA_SND_NACK_RCV;
}

/* Identify start condition */
inline void get_start_condition(void) {
    uint8_t retval = 0;

    // Make sure it is the start by checking SCL high when SDA goes low
    if (GET_SCL()) {
        TWI_INT_DISABLE();
    } else // false trigger; exit the ISR
    {
        TWI_INT_CLEAR_FLAG();
        TWI_INT_ENABLE();
        return;
    }

    // loop for one or several start conditions before a STOP
    if (TWSR == I2C_IDLE || TWSR == TWI_SLA_REPEAT_START) {
        retval = read_byte();
        if (retval == 0) {
            TWSR = I2C_IDLE;
            TWI_INT_CLEAR_FLAG();
            TWI_INT_ENABLE();
            return;
        } else {
            if (retval & 1)
                TWSR = TWI_SLA_REQ_R_ACK_RTD;
            else
                TWSR = TWI_SLA_REQ_W_ACK_RTD;
        }
    }

    TWDR = retval;

}
