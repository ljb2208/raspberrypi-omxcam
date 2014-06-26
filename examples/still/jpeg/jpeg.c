#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include "omxcam.h"

int log_error (){
  omxcam_perror ();
  return 1;
}

int fd;

void on_data (uint8_t* buffer, uint32_t length){
  //Append the buffer to the file
  //Note: Writing the data directly to disk will slow down the capture speed
  //due to the I/O access. A posible workaround is to save the buffers into
  //memory, similar to the still/raw.c (YUV) example, and then write the
  //whole image to disk
  if (pwrite (fd, buffer, length, 0) == -1){
    fprintf (stderr, "error: pwrite\n");
    if (omxcam_still_stop ()) log_error ();
  }
}

int save (char* filename, omxcam_still_settings_t* settings){
  printf ("capturing %s\n", filename);

  fd = open (filename, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0666);
  if (fd == -1){
    fprintf (stderr, "error: open\n");
    return -1;
  }
  
  if (omxcam_still_start (settings)) return log_error ();
  
  //Close the file
  if (close (fd)){
    fprintf (stderr, "error: close\n");
    return -1;
  }
  
  return 0;
}

int main (){
  //2592x1944 by default
  omxcam_still_settings_t settings;
  
  //Capture an image with default settings
  omxcam_still_init (&settings);
  settings.on_data = on_data;
  
  settings.jpeg.exif_enable = OMXCAM_FALSE;
  settings.jpeg.thumbnail_enable = OMXCAM_FALSE;
  
  if (save ("still-default-2592x1944.jpg", &settings)) return 1;
  
  //Capture an image with shutter speed 1/4, EV -10 and some EXIF tags
  //Shutter speed in milliseconds, 1000*(1/4)
  settings.camera.shutter_speed = 250;
  //Values of color_effects.u and color_effects.v are 128 by default,
  //a gray image
  settings.camera.color_effects.enabled = OMXCAM_TRUE;
  
  //See firmware/documentation/ilcomponents/image_decode.html for valid keys
  //See http://www.media.mit.edu/pia/Research/deepview/exif.html#IFD0Tags
  //for valid keys and their description
  omxcam_exif_tag_t exif_tags[] = {
    //Manufacturer
    { "IFD0.Make", "Raspberry Pi" }
  };
  settings.jpeg.exif_tags = exif_tags;
  settings.jpeg.exif_valid_tags = 1;
  
  if (save ("still-2592x1944.jpg", &settings)) return 1;
  
  printf ("ok\n");
  
  return 0;
}