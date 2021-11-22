/*
 * Copyright (c) 2021, Koncepto.io
 * SPDX-License-Identifier: Apache-2.0
 */
#include "as621x/as621x.h"

#define AS621X_BASE_ADDR (0x40)

#define U16_TO_BYTE_ARRAY(u, ba)                                                                   \
    do {                                                                                           \
        (ba)[0] = (u >> 8) & 0xFF;                                                                 \
        (ba)[1] = u & 0xFF;                                                                        \
    } while (0)
#define BYTE_ARRAY_TO_U16(ba, u)                                                                   \
    do {                                                                                           \
        u = ((ba)[0] << 8) | ((ba)[1]);                                                            \
    } while (0)

#define get_cfg_value(reg, name) (((reg) >> (name##_SHIFT)) & (name##_MASK))
#define get_reg_value(cfg, name) (((cfg) & (name##_MASK)) << (name##_SHIFT))

static char buf[3];

namespace sixtron {

AS621X::AS621X(I2C *bus, Add1Pin add1, Add0Pin add0): _bus(bus)
{
    this->last_reg = RegisterAddress::Max;

    assert((add1 >= Add1Pin::SCL) && (add1 <= Add1Pin::PullUp_VDD));
    assert((add0 >= Add0Pin::VSS) && (add0 <= Add0Pin::SCL));

    if (add1 == Add1Pin::PullUp_VDD) {
        this->alert_en = true;

        /* ADD1_PullUp address bits are the same as ADD1_VSS. */
        add1 = Add1Pin::VSS;
    } else {
        this->alert_en = false;
    }

    this->addr = (AS621X_BASE_ADDR | (static_cast<uint8_t>(add1) << 2)
                         | (static_cast<uint8_t>(add0) << 0))
            << 1;
}

AS621X::ErrorType AS621X::read_config(Config_t *cfg)
{
    uint16_t value;
    ErrorType err;
    err = this->read_register(RegisterAddress::Config, &value);

    if (err == ErrorType::Ok) {
        cfg->alert_bit = get_cfg_value(value, ALERT);
        cfg->cr = static_cast<ConversionRate>(get_cfg_value(value, CONVRATE));
        cfg->sleep_mode = get_cfg_value(value, SLEEPMODE);
        cfg->interrupt_mode = get_cfg_value(value, INTMODE);
        cfg->polarity = get_cfg_value(value, POLARITY);
        cfg->cf = static_cast<ConsecutiveFaults>(get_cfg_value(value, CFAULTS));
        cfg->single_shot = get_cfg_value(value, SINGLESHOT);
    }

    return err;
}

AS621X::ErrorType AS621X::write_config(Config_t *cfg)
{
    uint16_t value = 0;

    value |= get_reg_value(static_cast<uint8_t>(cfg->cr), CONVRATE);
    value |= get_reg_value(cfg->sleep_mode, SLEEPMODE);
    value |= get_reg_value(cfg->interrupt_mode, INTMODE);
    value |= get_reg_value(cfg->polarity, POLARITY);
    value |= get_reg_value(static_cast<uint8_t>(cfg->cf), CFAULTS);
    value |= get_reg_value(cfg->single_shot, SINGLESHOT);

    return this->write_register(RegisterAddress::Config, value);
}

AS621X::ErrorType AS621X::read_temperature(RegisterAddress reg, double *value)
{
    ErrorType err;
    uint16_t temp;

    if (reg == RegisterAddress::Config) {
        return ErrorType::InvalidReg;
    }

    err = this->read_register(reg, &temp);

    if (err == ErrorType::Ok) {
        *value = (double)(int16_t)temp / 128.0;
    }

    return err;
}

AS621X::ErrorType AS621X::write_temperature(RegisterAddress reg, double value)
{
    int16_t temp = round(value * 128);

    if (reg == RegisterAddress::Config || reg == RegisterAddress::Tval) {
        return ErrorType::InvalidReg;
    }

    return this->write_register(reg, (uint16_t)temp);
}

bool AS621X::is_alert_enabled()
{
    return this->alert_en;
}

AS621X::ErrorType AS621X::write_register(RegisterAddress reg, uint16_t value)
{
    ErrorType err = ErrorType::Ok;

    this->_bus->lock();

    if (reg >= RegisterAddress::Max) {
        err = ErrorType::InvalidReg;
        goto write_reg_end;
    }

    U16_TO_BYTE_ARRAY(value, buf + 1);
    buf[0] = static_cast<char>(reg);

    if (this->_bus->write(this->addr, buf, 3)) {
        err = ErrorType::I2cError;
        this->last_reg = RegisterAddress::Max;
    }
    this->last_reg = reg;

write_reg_end:
    this->_bus->unlock();
    return err;
}

AS621X::ErrorType AS621X::read_register(RegisterAddress reg, uint16_t *value)
{
    ErrorType err = ErrorType::Ok;

    char addr = static_cast<char>(reg);
    this->_bus->lock();

    if (reg >= RegisterAddress::Max) {
        err = ErrorType::InvalidReg;
        goto read_reg_end;
    }

    if (this->last_reg != reg) {
        if (this->_bus->write(this->addr, &addr, 1)) {
            err = ErrorType::I2cError;
            this->last_reg = RegisterAddress::Max;
            goto read_reg_end;
        }
        this->last_reg = reg;
    }

    if (this->_bus->read(this->addr, buf, 2)) {
        err = ErrorType::I2cError;
        this->last_reg = RegisterAddress::Max;
        goto read_reg_end;
    }

    BYTE_ARRAY_TO_U16(buf, *value);

read_reg_end:
    this->_bus->unlock();
    return err;
}

} // namespace sixtron
