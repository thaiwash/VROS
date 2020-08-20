// gcc -o readi2c main.c

// read all 4k 
// readi2c [addr] 8191

// apt-get update
// apt-get install i2c-tools
// i2cdetect -y 1

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>				//Needed for I2C port
#include <fcntl.h>				//Needed for I2C port
#include <sys/ioctl.h>			//Needed for I2C port
#include <linux/i2c-dev.h>		//Needed for I2C port


void i2c(int address, int bytes_to_read){
	int file_i2c;
	int length;
	int bytes_num = bytes_to_read;
	unsigned char buffer[bytes_num];

	
	//----- OPEN THE I2C BUS -----
	char *filename = (char*)"/dev/i2c-1";
	if ((file_i2c = open(filename, O_RDWR)) < 0)
	{
		//ERROR HANDLING: you can check errno to see what went wrong
		printf("Failed to open the i2c bus");
		return;
	}
	
	int addr = address;          //<<<<<The I2C address of the slave
	if (ioctl(file_i2c, I2C_SLAVE, addr) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave.\n");
		//ERROR HANDLING; you can check errno to see what went wrong
		return;
	}
	
	//----- READ BYTES -----
	length = bytes_num;			//<<< Number of bytes to read
	if (read(file_i2c, buffer, length) != length)		//read() returns the number of bytes actually read, if it doesn't match then an error occurred (e.g. no response from the device)
	{
		//ERROR HANDLING: i2c transaction failed
		printf("Failed to read from the i2c bus.\n");
	}
	else
	{
		for (int i = 0; i < length; i ++) {
			printf("byte %i: %i\n", i, (int)buffer[i]);
		}
	}

}

/**
 * hex2int
 * take a hex string and convert it to a 32bit number (max 8 hex digits)
 */
uint32_t hex2int(char *hex) {
    uint32_t val = 0;
    while (*hex) {
        // get current character then increment
        uint8_t byte = *hex++; 
        // transform hex character to the 4bit equivalent number, using the ascii table indexes
        if (byte >= '0' && byte <= '9') byte = byte - '0';
        else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
        else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;    
        // shift 4 to make space for new digit, and add the 4 bits of the new digit 
        val = (val << 4) | (byte & 0xF);
    }
    return val;
}


int main(int argc, char *argv[])
{
   printf("Address: %s, Number of bytes to read: %s\n", argv[1], argv[2]);
   //printf("strtol: %i\n", (int)strtol(argv[1], NULL, 16));
   //printf("strtol: %i\n", (int)hex2int(argv[1]));
   i2c(hex2int(argv[1]), atoi(argv[2]));
   return 0;
}

// 8191
