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

/*!
 *  \class AS621X
 *  AS621X temperature sensor driver
 */
class AS621X {
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

    enum class ConversionRate : uint8_t {
        _4s = 0,
        _1s = 1,
        _250ms = 2,
        _125ms = 3,
    };

    enum class ConsecutiveFaults : uint8_t {
        _1CF = 0,
        _2CF = 1,
        _3CF = 2,
        _4CF = 3,
    };

    enum class RegisterAddress : uint8_t {
        Tval = 0,
        Config = 1,
        TLow = 2,
        THigh = 3,
        Max = 4,
    };

    /*!
     * \brief Configuration register data
     */
    typedef struct {
        bool alert_bit;
        ConversionRate cr;
        bool sleep_mode;
        bool interrupt_mode;
        bool polarity;
        ConsecutiveFaults cf;
        bool single_shot;
    } Config_t;

    /*! Constructor
     *
     * \param bus pointer to mbed I2C object
     * \param add1 ADD1 pin connection
     * \param add0 ADD0 pin connection
     */
    AS621X(I2C *bus, Add1Pin add1, Add0Pin add0);

    /*!
     * \brief Read configuration register
     *
     * \param cfg pointer to data to store read configuration
     *
     * \return Ok on success, the reason of failure otherwise
     */
    ErrorType read_config(Config_t *cfg);

    /*!
     * \brief Write configuration register
     *
     * \param cfg pointer to configuration structure to use
     *
     * \return Ok on success, the reason of failure otherwise
     */
    ErrorType write_config(Config_t *cfg);

    /*!
     * \brief Read temperature register
     *
     * \param reg temperature register to read from
     * \param value pointer to the space to store the temperature read
     *
     * \return Ok on success, the reason of failure otherwise
     */
    ErrorType read_temperature(RegisterAddress reg, double *value);

    /*!
     * \brief Write temperature register
     *
     * \param reg temperature register to write to
     * \param value temperature to write
     *
     * \return Ok on success, the reason of failure otherwise
     */
    ErrorType write_temperature(RegisterAddress reg, double value);

    /*!
     * \brief Read alert pin enablement status
     *
     * \return true if alert pin is enabled, false otherwise
     */
    bool is_alert_enabled();

private:
    I2C *_bus;
    uint8_t addr;
    RegisterAddress last_reg;
    bool alert_en;

    /*! Set register value
     *
     * \param RegisterAddress register address
     * \param value value to write
     *
     * \return Ok on success, the reason of failure otherwise
     */
    ErrorType write_register(RegisterAddress reg, uint16_t value);

    /*! Get register value
     *
     * \param RegisterAddress register address
     * \param value pointer to store read value
     *
     * \return Ok on success, the reason of failure otherwise
     */
    ErrorType read_register(RegisterAddress reg, uint16_t *value);
};

} // namespace sixtron

#endif // CATIE_SIXTRON_AS621X_H_
