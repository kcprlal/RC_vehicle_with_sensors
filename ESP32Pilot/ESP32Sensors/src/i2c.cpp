#include "bme280.hpp"
#include "i2c.hpp"
#include <Arduino.h>
namespace kl
{
    I2C::I2C(i2c_port_t i2c_num, uint8_t sda_pin, uint8_t scl_pin, uint32_t baud_rate)
    {
        _i2c_num = i2c_num;
        _sda_pin = sda_pin;
        _scl_pin = scl_pin;
        _baud_rate = baud_rate;
    }

    void I2C::begin()
    {
        _conf.mode = I2C_MODE_MASTER;
        _conf.sda_io_num = (gpio_num_t)_sda_pin;
        _conf.scl_io_num = (gpio_num_t)_scl_pin;
        _conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
        _conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
        _conf.master.clk_speed = _baud_rate;

        i2c_param_config(_i2c_num, &_conf);
        i2c_driver_install(_i2c_num, I2C_MODE_MASTER, 0, 0, 0);
    }
    // przeladowane funkcje
    //  tu dla bme280
    bool I2C::write(uint8_t dest_addr, uint8_t reg_addr, uint8_t data)
    {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (dest_addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd, reg_addr, true);
        i2c_master_write_byte(cmd, data, true);
        i2c_master_stop(cmd);

        esp_err_t err = i2c_master_cmd_begin(_i2c_num, cmd, pdMS_TO_TICKS(1000));
        i2c_cmd_link_delete(cmd);
        if (err != ESP_OK)
        {
            return false;
            // tu dodac error handling
            // zmienie cala funckje na uint8_t i bedzie zwracac lcizbe
        }
        return true;
    }
    // wersja dla sgp30
    bool I2C::write(uint8_t dest_addr, const uint8_t *data, uint8_t len)
    {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (dest_addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write(cmd, data, len, true);
        i2c_master_stop(cmd);

        esp_err_t err = i2c_master_cmd_begin(_i2c_num, cmd, pdMS_TO_TICKS(1000));
        i2c_cmd_link_delete(cmd);
        return err == ESP_OK;
        // tutaj te moze dodam jakis error handling
    }
    // dla bme280
    bool I2C::read(uint8_t dest_addr, uint8_t reg_addr, uint8_t *data)
    {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (dest_addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd, reg_addr, true);
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (dest_addr << 1) | I2C_MASTER_READ, true);
        i2c_master_read_byte(cmd, data, I2C_MASTER_NACK);
        i2c_master_stop(cmd);
        esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(1000));
        i2c_cmd_link_delete(cmd);

        if (ret != ESP_OK)
        {
            return false; // error handling
        }
        return true;
    }

    // dla sgp30
    bool I2C::read(uint8_t dest_addr, const uint8_t *command, uint8_t cmd_len, uint8_t *data, uint8_t len)
    {

        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (dest_addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write(cmd, command, cmd_len, true);
        i2c_master_stop(cmd);
        esp_err_t err = i2c_master_cmd_begin(_i2c_num, cmd, pdMS_TO_TICKS(1000));
        i2c_cmd_link_delete(cmd);

        if (err != ESP_OK)
        {
            return false;
        }

        delay(12);

        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (dest_addr << 1) | I2C_MASTER_READ, true);
        if (len > 1)
        {
            i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);
        }
        i2c_master_read_byte(cmd, &data[len - 1], I2C_MASTER_NACK);
        i2c_master_stop(cmd);
        err = i2c_master_cmd_begin(_i2c_num, cmd, pdMS_TO_TICKS(1000));
        i2c_cmd_link_delete(cmd);

        if (err != ESP_OK)
        {
            return false;
        }

        return true;
    }

    bool I2C::ping(uint8_t dest_addr)
    {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (dest_addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        esp_err_t err = i2c_master_cmd_begin(_i2c_num, cmd, pdMS_TO_TICKS(100));
        i2c_cmd_link_delete(cmd);

        return (err == ESP_OK);
    }
}
