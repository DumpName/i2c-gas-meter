#pragma once

#include <utility>
#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include <esphome/components/sensor/sensor.h>

namespace esphome {
namespace i2cgasmeter {


class I2CGasMeterComponent : public sensor::Sensor, public PollingComponent, public i2c::I2CDevice{
    public:
        void set_measurement_duration_sensor(sensor::Sensor *measurement_duration_sensor) { measurement_duration_sensor_ = measurement_duration_sensor; }
        void set_pulses_sensor(sensor::Sensor *pulses_sensor) { pulses_sensor_ = pulses_sensor; }
        void set_battery_voltage_sensor(sensor::Sensor *battery_voltage_sensor ) { battery_voltage_sensor_ = battery_voltage_sensor; }

        void setup() override;
        void update() override;
        float get_setup_priority() const override { return setup_priority::DATA; }
        void dump_config() override;

    protected:
        uint32_t last_time_{0};
        uint32_t current_total_{0};
        sensor::Sensor *measurement_duration_sensor_{nullptr};
        sensor::Sensor *pulses_sensor_{nullptr};
        sensor::Sensor *battery_voltage_sensor_{nullptr};
        enum ErrorCode {
            NONE = 0,
            COMMUNICATION_FAILED,
            WRONG_CHIP_ID,
        } error_code_{NONE};
};
}
}