import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor
from esphome.const import (
    CONF_ID,
    CONF_COUNT,
    CONF_BATTERY_VOLTAGE,
    CONF_MEASUREMENT_DURATION,
    ICON_BATTERY,
    ICON_TIMER,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_VOLUME,
    DEVICE_CLASS_DURATION,
    UNIT_VOLT,
    UNIT_CUBIC_METER,
    UNIT_SECOND,
    ENTITY_CATEGORY_DIAGNOSTIC,
)

AUTO_LOAD = [ "sensor" ]

DEPENDENCIES = ["i2c"]

i2cgasmeter_ns = cg.esphome_ns.namespace( "i2cgasmeter" )
I2CGasMeterComponent = i2cgasmeter_ns.class_( 
    "I2CGasMeterComponent", cg.PollingComponent, i2c.I2CDevice
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID( ): cv.declare_id( I2CGasMeterComponent ),
            cv.Optional( CONF_BATTERY_VOLTAGE ): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                icon=ICON_BATTERY,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_BATTERY,
                state_class=STATE_CLASS_MEASUREMENT
            ),
            cv.Optional( CONF_COUNT ): sensor.sensor_schema(
                unit_of_measurement=UNIT_CUBIC_METER,
                icon="mdi:meter-gas",
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_VOLUME,
                state_class=STATE_CLASS_TOTAL_INCREASING
            ),
            cv.Optional( CONF_MEASUREMENT_DURATION ): sensor.sensor_schema(
                unit_of_measurement=UNIT_SECOND,
                icon=ICON_TIMER,
                accuracy_decimals=0,
                state_class=STATE_CLASS_TOTAL_INCREASING,
                device_class=DEVICE_CLASS_DURATION,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            )
        }
    )
    .extend( i2c.i2c_device_schema(0x5D))
    .extend( cv.polling_component_schema("30m"))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device( var, config )

    if battery_voltage_config := config.get( CONF_BATTERY_VOLTAGE ):
        sens = await sensor.new_sensor(battery_voltage_config)
        cg.add(var.set_battery_voltage_sensor(sens))
    
    if count_config := config.get(CONF_COUNT):
        sens = await sensor.new_sensor(count_config)
        cg.add(var.set_count_sensor(sens))

    if measurement_duration_config := config.get( CONF_MEASUREMENT_DURATION ):
        sens = await sensor.new_sensor(measurement_duration_config)
        cg.add(var.set_measurement_duration_sensor(sens))
