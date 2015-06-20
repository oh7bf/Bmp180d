#include "bmp180d.h"

// read given number of bytes from register at i2c address with repeated start
// return 1 in success and negative in case of failure, set global 'cont' 
// to zero in more serious failure
int I2cRReadRegBytes(int address, unsigned char reg, unsigned char *buffer, int nbytes)
{
  int fd, rd;
  int cnt = 0, result = 0;
  char message[200]="";

// from Lauri Pirttiaho
  struct i2c_msg rdwr_msgs[2] =
  {
    {  // Start address
      .addr = address,
      .flags = 0, // write
      .len = 1,
      .buf = &reg
    },
    { // Read buffer
      .addr = address,
      .flags = I2C_M_RD, // read
      .len = nbytes,
      .buf = buffer
    }
  };

  struct i2c_rdwr_ioctl_data rdwr_data =
  {
    .msgs = rdwr_msgs,
    .nmsgs = 2
  };

  if( (fd = open(i2cdev, O_RDWR) ) < 0 )
  {
    syslog(LOG_ERR|LOG_DAEMON, "Failed to open i2c port");
    return -1;
  }
  else
  {
    rd = flock(fd, LOCK_EX|LOCK_NB);

    cnt = I2LOCK_MAX;
    while( ( rd == 1 ) && ( cnt > 0 ) ) // try again if port locking failed
    {
      sleep(1);
      rd = flock(fd, LOCK_EX|LOCK_NB);
      cnt--;
    }

    if( rd )
    {
      syslog(LOG_ERR|LOG_DAEMON, "Failed to open i2c port");
      close( fd );
      return -2;
    }
    else
    {
      if( ioctl(fd, I2C_SLAVE, address) < 0 )
      {
        syslog(LOG_ERR|LOG_DAEMON, "Unable to get bus access to talk to slave");
        close( fd );
        return -3;
      }
      else
      {
        result = ioctl(fd, I2C_RDWR, &rdwr_data);
        if( result < 0 )
        {
          sprintf(message, "rdwr ioctl error: %d", errno);
          syslog(LOG_ERR|LOG_DAEMON, "%s", message);
          close( fd );
          cont = 0;
          return -4;
        }
        else
        {
          sprintf(message, "read %02x%02x%02x%02x%02x%02x", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
          syslog(LOG_DEBUG, "%s", message);
        }
      }
    }
  }

  close( fd );
  return 1;
}

