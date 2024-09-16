#include <cmath>
#include <cstdint>

#include "i2c_gas_meter.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include <esphome/components/sensor/sensor.h>
#include <esphome/core/component.h>

static const uint8_t GAS_METER_REGISTER_CHIPID = 0xD0;


namespace esphome {
namespace i2c_gas_meter {

static const char *TAG = "i2c_gas_meter";

void I2CGasMeterComponent::setup( ) {
    ESP_LOGCONFIG(TAG, "Setting up I2CGasMeter...");
    // Mark as not failed before initializing. Some devices will turn off sensors to save on batteries
    // and when they come back on, the COMPONENT_STATE_FAILED bit must be unset on the component.
}

void I2CGasMeterComponent::update( ) {
    ESP_LOGV(TAG, "Sending conversion request...");            
    uint8_t maxFrames = 44;
    uint8_t packetChecksum;

    I2CDevice::read_byte( GAS_METER_REGISTER_CHIPID, &packetChecksum );
    uint16_t checksum = 0;
    uint8_t i2cFrames[ maxFrames ];
    uint8_t lastFrame = 0;  
    while(!I2CDevice::read_byte( GAS_METER_REGISTER_CHIPID, &i2cFrames[ lastFrame ] ) ) {
        checksum += i2cFrames[ lastFrame ] % 256;
        lastFrame++;
    }
    checksum = checksum % 256;
    if( packetChecksum != checksum ) {
        delay( 500 );
        return;
    }
    if( this->battery_voltage_sensor_ != nullptr) {
        uint16_t adcValue = int( i2cFrames[ 0 ] << 8) + int( i2cFrames[ 1 ] );
        this->battery_voltage_sensor_->publish_state(4.7f * adcValue / 1024);
    }
    if( this->measurement_duration_sensor_ != nullptr ) {
        this->measurement_duration_sensor_->publish_state( int( i2cFrames[ 2 ] << 8) + int( i2cFrames[ 3 ] ) );
    }
    if( this->pulses_sensor_ != nullptr ) {
        uint16_t total_pulses = 0;
        for( uint8_t i = 0; i < lastFrame; i++ ){
            total_pulses += int( i2cFrames[ i ] );
        
        }
        this->pulses_sensor_->publish_state( total_pulses );
    }
}

void I2CGasMeterComponent::dump_config() {
    LOG_I2C_DEVICE(this);
    ESP_LOGCONFIG(TAG, "I2CGasMeter:");
    LOG_UPDATE_INTERVAL(this);
}
}
}
