#include "bme280.hpp"
#include "i2c.hpp"

extern kl::I2C i2c;
namespace kl
{
	BME280::BME280() : t_fine(0), adc_T(0), adc_P(0), adc_H(0), press(0), temp(0), hum(0) {}

	void BME280::init()
	{
		writeRegister(CTRL_MEAS, 0xb7);
		writeRegister(CTRL_HUM, 0x05);
		for (volatile int i = 0; i < 1000000; ++i)
			; // tymczasowy delay
		readCalibrations();
	}

	void BME280::readCalibrations()
	{
		readRegister(ADDR_DIG_T1 + 1, &utempdig);
		dig_T1 = (uint16_t)utempdig << 8;
		readRegister(ADDR_DIG_T1, &utempdig);
		dig_T1 |= utempdig;

		readRegister(ADDR_DIG_T2 + 1, &utempdig);
		dig_T2 = (int16_t)utempdig << 8;
		readRegister(ADDR_DIG_T2, &utempdig);
		dig_T2 |= utempdig;

		readRegister(ADDR_DIG_T3 + 1, &utempdig);
		dig_T3 = (int16_t)utempdig << 8;
		readRegister(ADDR_DIG_T3, &utempdig);
		dig_T3 |= utempdig;

		const uint8_t calib[] = {
			ADDR_DIG_P1, ADDR_DIG_P2, ADDR_DIG_P3, ADDR_DIG_P4,
			ADDR_DIG_P5, ADDR_DIG_P6, ADDR_DIG_P7, ADDR_DIG_P8, ADDR_DIG_P9};
		int16_t *calibVars[] = {
			(int16_t *)&dig_P1, &dig_P2, &dig_P3, &dig_P4,
			&dig_P5, &dig_P6, &dig_P7, &dig_P8, &dig_P9};
		for (int i = 0; i < 9; ++i)
		{
			readRegister(calib[i] + 1, &utempdig);
			*calibVars[i] = (int16_t)(utempdig << 8);
			readRegister(calib[i], &utempdig);
			*calibVars[i] |= utempdig;
		}

		readRegister(ADDR_DIG_H1, &dig_H1);

		uint8_t buf[2];
		readRegisters(ADDR_DIG_H2, buf, 2);
		dig_H2 = (int16_t)(buf[1] << 8 | buf[0]);

		readRegister(ADDR_DIG_H3, &dig_H3);

		uint8_t e4, e5, e6;
		readRegister(ADDR_DIG_H4, &e4);
		readRegister(ADDR_DIG_H5, &e5);
		readRegister(ADDR_DIG_H5 + 1, &e6);
		dig_H4 = (int16_t)((e4 << 4) | (e5 & 0x0F));
		dig_H5 = (int16_t)((e6 << 4) | (e5 >> 4));

		readRegister(ADDR_DIG_H6, &utempdig);
		dig_H6 = (int8_t)utempdig;
	}

	void BME280::makeMeasurement()
	{
		readPTH();
		press = compensatePressure();
		temp = compensateTemperature();
		hum = compensateHumidity();
	}

	void BME280::readPTH()
	{
		uint8_t msb, lsb, xlsb;

		// Temp
		readRegister(TEMP_MSB, &msb);
		readRegister(TEMP_LSB, &lsb);
		readRegister(TEMP_XLSB, &xlsb);
		adc_T = (msb << 12) | (lsb << 4) | (xlsb >> 4);
		temp = adc_T;

		// Pressure
		readRegister(PRESS_MSB, &msb);
		readRegister(PRESS_LSB, &lsb);
		readRegister(PRESS_XLSB, &xlsb);
		adc_P = (msb << 12) | (lsb << 4) | (xlsb >> 4);
		press = adc_P;

		// Humidity
		readRegister(HUM_MSB, &msb);
		readRegister(HUM_LSB, &lsb);
		adc_H = (msb << 8) | lsb;
		hum = adc_H;
	}

	int32_t BME280::compensateTemperature()
	{
		int32_t var1, var2, T;
		var1 = (((adc_T >> 3) - ((int32_t)dig_T1 << 1)) * ((int32_t)dig_T2)) >> 11;
		var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) *
				  ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
		t_fine = var1 + var2;
		T = (t_fine * 5 + 128) >> 8;
		return T;
	}

	uint32_t BME280::compensatePressure()
	{
		int64_t var1, var2, p;
		var1 = ((int64_t)t_fine) - 128000;
		var2 = var1 * var1 * (int64_t)dig_P6;
		var2 += (var1 * (int64_t)dig_P5) << 17;
		var2 += ((int64_t)dig_P4) << 35;
		var1 = (((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12));
		var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;

		if (var1 == 0)
			return 0;

		p = 1048576 - adc_P;
		p = (((p << 31) - var2) * 3125) / var1;
		var1 = (((int64_t)dig_P9) * (p >> 13)) >> 25;
		var2 = (((int64_t)dig_P8) * p) >> 19;
		p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
		return (uint32_t)p / 25600;
	}

	uint32_t BME280::compensateHumidity()
	{
		int32_t v_x1_u32r = t_fine - 76800;
		v_x1_u32r = (((((adc_H << 14) - (((int32_t)dig_H4) << 20) -
						(((int32_t)dig_H5) * v_x1_u32r)) + 16384) >> 15) * (((((((v_x1_u32r * ((int32_t)dig_H6)) >> 10) *
						  (((v_x1_u32r * ((int32_t)dig_H3)) >> 11) + 32768)) >> 10) + 2097152) * ((int32_t)dig_H2) + 8192) >> 14));
		v_x1_u32r -= (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
					   ((int32_t)dig_H1)) >> 4);
		v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
		v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
		return (uint32_t)(v_x1_u32r >> 12);
	}

	bool BME280::readRegister(uint8_t reg, uint8_t *data)
	{
		return i2c.read(I2C_ADDR, reg, data);
	}

	bool BME280::writeRegister(uint8_t reg, uint8_t value)
	{
		return i2c.write(I2C_ADDR, reg, value);
	}

	bool BME280::readRegisters(uint8_t reg, uint8_t *buffer, uint8_t length)
	{
		for (uint8_t i = 0; i < length; ++i)
		{
			if (!i2c.read(I2C_ADDR, reg + i, &buffer[i]))
			{
				return false;
			}
		}
		return true;
	}
}