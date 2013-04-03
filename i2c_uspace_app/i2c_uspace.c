/**
 * @brief Userspace application for I2C communication 
 		  Copyright (C) 2013 Andrei Danaila, All Rights Reserved
		  				<mailto: adanaila >at< ctrlinux[.]com>

	This file is part of i2c_app.

    i2c_app is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 2.

    i2c_app is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with i2c_app.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "linux/i2c-dev.h"

int main()
{

	int  			i2c_dev_node		= 0;
	/* ADXL345 Device Address 	  */
	int  			i2c_dev_address 	= 0x53; 
	/* ADXL345 POWER_CTL Register */
	int  			i2c_dev_reg_addr    = 0x2D;
	/* ADXL345 Adapter Device Node*/
	char 			i2c_dev_node_path[] = "/dev/i2c-4";
	unsigned short  i2c_val_to_write 	= 0x8;
	int	 			ret_val 		 	= 0;
	__s32 			read_value 			= 0;
	/* ADXL345 X Measurement Register */
	int				i2c_dev_reg_x_acc   = 0x32;

	/* Open the device node for the I2C adapter of bus 4 */
	i2c_dev_node = open(i2c_dev_node_path, O_RDWR);
	if (i2c_dev_node < 0)
	{
		perror("Unable to open device node.");
		exit(1);
	}
	/* Set I2C_SLAVE for adapter 4 */
	ret_val = ioctl(i2c_dev_node,I2C_SLAVE,i2c_dev_address);
	if (ret_val < 0)
	{
		perror("Could not set I2C_SLAVE.");
		exit(2);
	}

	/* Read byte from the 0x2D register of the I2C_SLAVE device */
	read_value = i2c_smbus_read_byte_data(i2c_dev_node, 
										  i2c_dev_reg_addr);
	if (read_value < 0)
	{
		perror("I2C Read operation failed.");
		exit(3);
	}
	ret_val = i2c_smbus_write_byte_data(i2c_dev_node,
										i2c_dev_reg_addr,
										i2c_val_to_write);

	if (ret_val < 0)
	{
		perror("I2C Write Operation failed.");
		exit(4);
	}
	/* Read the measurement from the accelerometer */
	while(1)
	{
		read_value = i2c_smbus_read_byte_data(i2c_dev_node, 
											 i2c_dev_reg_x_acc );
		/* Mask the upper 3 bits as they do not pertain to measurement*/
		read_value &= 0x1F; 

		if (read_value < 0)
		{
			perror("I2C Read operation failed.");
			exit(3);
		}
		printf("X-Measurement Val = %d\n", read_value);
	}
	return 0;
}

