/*
 * wiringPiSPI.c:
 *	Simplified SPI access routines
 *	Copyright (c) 2012-2015 Gordon Henderson
 ***********************************************************************
 * This file is part of wiringPi:
 *	https://github.com/WiringPi/WiringPi/
 *
 *    wiringPi is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as
 *    published by the Free Software Foundation, either version 3 of the
 *    License, or (at your option) any later version.
 *
 *    wiringPi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with wiringPi.
 *    If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
 */


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <asm/ioctl.h>
#include <linux/spi/spidev.h>

#include "wiringPi.h"

#include "wiringPiSPI.h"


// The SPI bus parameters
//	Variables as they need to be passed as pointers later on

//static const char       *spiDev0  = "/dev/spidev0.0" ;
//static const char       *spiDev1  = "/dev/spidev0.1" ;
static const uint8_t     spiBPW   = 8 ;
static const uint16_t    spiDelay = 0 ;

static uint32_t    spiSpeeds [3] ;
static int         spiFds [3] ;


/*
 * wiringPiSPIGetFd:
 *	Return the file-descriptor for the given channel
 *********************************************************************************
 */

int wiringPiSPIGetFdBus (int bus, int channel)
{
  return spiFds [(bus * 2) + channel & 1] ;
}

int wiringPiSPIGetFd (int channel)
{
    return wiringPiSPIGetFdBus(0, 1);
}



/*
 * wiringPiSPIDataRW:
 *	Write and Read a block of data over the SPI bus.
 *	Note the data ia being read into the transmit buffer, so will
 *	overwrite it!
 *	This is also a full-duplex operation.
 *********************************************************************************
 */

int wiringPiSPIDataRWBus (int bus, int channel, unsigned char *data, int len)
{
  struct spi_ioc_transfer spi ;

  channel &= 1 ;

// Mentioned in spidev.h but not used in the original kernel documentation
//	test program )-:

  memset (&spi, 0, sizeof (spi)) ;

  spi.tx_buf        = (unsigned long)data ;
  spi.rx_buf        = (unsigned long)data ;
  spi.len           = len ;
  spi.delay_usecs   = spiDelay ;
  spi.speed_hz      = spiSpeeds [(bus*2) + channel] ;
  spi.bits_per_word = spiBPW ;

//  LOGD("ioctl (spiFds [(%d*2) + %d], SPI_IOC_MESSAGE(1), %d)", bus, channel, spiSpeeds[(bus*2 + channel)]);
  int test = ioctl (spiFds [(bus*2) + channel], SPI_IOC_MESSAGE(1), &spi);
//  LOGD("value: %d", test);
  return test ;
}

int wiringPiSPIDataRW (int channel, unsigned char *data, int len) {
    return wiringPiSPIDataRWBus(0, channel, data, len);
}

/*
 * wiringPiSPISetupMode:
 *	Open the SPI device, and set it up, with the mode, etc.
 *********************************************************************************
 */

int wiringPiSPISetupMode (int bus, int channel, int speed, int mode)
{
  int fd ;
  char spiDev [32] ;

  mode    &= 3 ;	// Mode is 0, 1, 2 or 3
//  LOGD("bud: %d, channel: %d, speed: %d", bus, channel, speed);

// Channel can be anything - lets hope for the best
//  channel &= 1 ;	// Channel is 0 or 1

  snprintf (spiDev, 31, "/dev/spidev%d.%d", bus, channel) ;

  if ((fd = open (spiDev, O_RDWR)) < 0)
    return wiringPiFailure (WPI_ALMOST, "Unable to crank SPI device (%s): %s\n", spiDev, strerror (errno)) ;

  spiSpeeds [(bus*2) + channel] = speed ;
  spiFds    [(bus*2) + channel] = fd ;

// Set SPI parameters.

  if (ioctl (fd, SPI_IOC_WR_MODE, &mode)            < 0)
    return wiringPiFailure (WPI_ALMOST, "SPI Mode Change failure: %s\n", strerror (errno)) ;
  
  if (ioctl (fd, SPI_IOC_WR_BITS_PER_WORD, &spiBPW) < 0)
    return wiringPiFailure (WPI_ALMOST, "SPI BPW Change failure: %s\n", strerror (errno)) ;

  if (ioctl (fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed)   < 0)
    return wiringPiFailure (WPI_ALMOST, "SPI Speed Change failure: %s\n", strerror (errno)) ;

  return fd ;
}

/*
 * wiringPiSPISetup:
 *	Open the SPI device, and set it up, etc. in the default MODE 0
 *********************************************************************************
 */

int wiringPiSPISetupBus (int bus, int channel, int speed)
{
  return wiringPiSPISetupMode (bus, channel, speed, 0) ;
}

/*
 * wiringPiSPISetup:
 *	Open the SPI device, and set it up, etc. in the default MODE 0
 *********************************************************************************
 */

int wiringPiSPISetup (int channel, int speed)
{
  return wiringPiSPISetupMode (0, channel, speed, 0) ;
}
