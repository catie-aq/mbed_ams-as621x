/*
 * Copyright (c) 2021, Koncepto.io
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef CATIE_SIXTRON_AS621X_H_
#define CATIE_SIXTRON_AS621X_H_

#include "mbed.h"

namespace sixtron {

#define ALERT_SHIFT (5)
#define ALERT_MASK (0x1)
#define CONVRATE_SHIFT (6)
#define CONVRATE_MASK (0x3)
#define SLEEPMODE_SHIFT (8)
#define SLEEPMODE_MASK (0x1)
#define INTMODE_SHIFT (9)
#define INTMODE_MASK (0x1)
#define POLARITY_SHIFT (10)
#define POLARITY_MASK (0x1)
#define CFAULTS_SHIFT (11)
#define CFAULTS_MASK (0x3)
#define SINGLESHOT_SHIFT (15)
#define SINGLESHOT_MASK (0x1)

class As621x {
public:
    enum class ErrorType : uint8_t {
        Ok = 0,
        I2cError = 1,
        InvalidReg = 2,
    };

    enum class Add0Pin : uint8_t {
        VSS = 0,
        VDD = 1,
        SDA = 2,
        SCL = 3,
    };
    enum class Add1Pin : uint8_t {
        SCL = 1,
        VSS = 2,
        PullUp_VDD = 3,
    };

    enum class RegisterAddress : uint8_t {
        Tval = 0,
        Config = 1,
        TLow = 2,
        THigh = 3,
        Max = 4,
    };
    As621x(I2C *bus, Add1Pin add1, Add0Pin add0);

    ErrorType read_config(uint16_t *value);
    ErrorType write_config(uint16_t value);

    ErrorType read_temperature(RegisterAddress reg, double *value);
    ErrorType write_temperature(RegisterAddress reg, double value);

private:
    I2C *_bus;
    uint8_t addr;
    RegisterAddress last_reg;
    bool alert_en;

    ErrorType write_register(RegisterAddress reg, uint16_t value);
    ErrorType read_register(RegisterAddress reg, uint16_t *value);
};

} // namespace sixtron

#endif // CATIE_SIXTRON_AS621X_H_
