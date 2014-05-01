#include "util.h"
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef USE_PNG
#include <png.h>
#endif
#define RANDOM_SEED

void* open_mmapped_file_read(const char* filename, int* length) {
  struct stat fs;
  int fd;
  void* region;

  //First we stat the file to get its length.
  if(stat(filename, &fs)) {
    perror("cannot read file");
    return NULL;
  }
  *length = fs.st_size;
  
  //Now get a file descriptor and mmap!
  fd = open(filename, O_RDONLY);
  region=mmap(NULL, *length, PROT_READ, MAP_SHARED, fd, 0);

  return region;
}

void* open_mmapped_file_write(const char* filename, int length) {
  struct stat fs;
  int fd;
  void* region;
  
  //Now get a file descriptor and mmap!
  fd = open(filename, O_RDWR|O_TRUNC|O_CREAT, S_IRUSR|S_IWUSR|S_IROTH|S_IWOTH);
  if(fd<0) {
    perror("couldn't open file");
    printf("file was: %s\n",filename);
  }
  ftruncate(fd,length);
  region=mmap(NULL, length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
  if(region==MAP_FAILED) {
    perror("couldn't mmap file");
  }

  return region;
}

void copy_file(const char* dest, const char* src) {
  int n;
  void* s = open_mmapped_file_read(src,&n);
  void* d = open_mmapped_file_write(dest,n);
  memcpy(d,s,n);
  munmap(s,n);
  munmap(d,n);
}

static double finv(double t) {
  return (t>(6.0/29.0))?(t*t*t):(3*(6.0/29.0)*(6.0/29.0)*(t-4.0/29.0));
}
/* Convert from L*a*b* doubles to XYZ doubles */
void lab2xyz(double* x, double* y, double* z, double l, double a, double b) {
  double sl = (l+0.16)/1.16;
  double ill[3] = {0.9643,1.00,0.8251}; //D50
  *y = ill[1] * finv(sl);
  *x = ill[0] * finv(sl + (a/5.0));
  *z = ill[2] * finv(sl - (b/2.0));
}

static double correct(double cl) {
  double a = 0.055;
  return (cl<=0.0031308)?(12.92*cl):((1+a)*pow(cl,1/2.4)-a);
}
/* Convert from XYZ doubles to sRGB bytes */
void xyz2rgb(unsigned char* r, unsigned char* g, unsigned char* b, double x, double y, double z) {
  double rl =  3.2406*x - 1.5372*y - 0.4986*z;
  double gl = -0.9689*x + 1.8758*y + 0.0415*z;
  double bl =  0.0557*x - 0.2040*y + 1.0570*z;
  int clip = (rl < 0.001 || rl > 0.999 || gl < 0.001 || gl > 0.999 || bl < 0.001 || bl > 0.999);
  if(clip) {
    rl = (rl<0.001)?0.0:((rl>0.999)?1.0:rl);
    gl = (gl<0.001)?0.0:((gl>0.999)?1.0:gl);
    bl = (bl<0.001)?0.0:((bl>0.999)?1.0:bl);
  }
  //if(clip) {rl=1.0;gl=bl=0.0;}
  *r = (unsigned char)(255.0*correct(rl));
  *g = (unsigned char)(255.0*correct(gl));
  *b = (unsigned char)(255.0*correct(bl));
}

/* Convert from LAB doubles to sRGB bytes */
void lab2rgb(unsigned char* R, unsigned char* G, unsigned char* B, double l, double a, double b) {
  double x,y,z;
  lab2xyz(&x,&y,&z,l,a,b);
  xyz2rgb(R,G,B,x,y,z);
}

void lab2pix(void* rgb, double l, double a, double b) {
  unsigned char* ptr = (unsigned char*)rgb;
  lab2rgb(ptr,ptr+1,ptr+2,l,a,b);
}

void xyz2pix(void* rgb, double x, double y, double z) {
  unsigned char* ptr = (unsigned char*)rgb;
  xyz2rgb(ptr,ptr+1,ptr+2,x,y,z);
}

void lrl(double* L, double* r, double l) {
  *L = l*0.7;   //L of L*a*b*
  *r = l*0.301+0.125; //chroma
}

/* Convert from a qualitative parameter l and a quantitative parameter c to a 24-bit pixel */
void cl2pix(void* rgb, double c, double l) {
  unsigned char* ptr = (unsigned char*)rgb;
  double L,r;
  lrl(&L,&r,l);
  double angle = TAU/6.0-c*TAU;
  double a = sin(angle)*r;
  double b = cos(angle)*r;
  lab2rgb(ptr,ptr+1,ptr+2,L,a,b);
}

void csl2lab(double* L, double* a, double* b, double c, double s, double l) {
  double r;
  *L=l*0.7;
  r=0.426*s;
  double angle = TAU/6.0-c*TAU;
  *a = sin(angle)*r;
  *b = cos(angle)*r;
}

void csl2xyz(double *x, double *y, double *z, double c, double s, double l) {
  double L, a, b;
  csl2lab(&L,&a,&b,c,s,l);
  lab2xyz(x,y,z,L,a,b);
}

void csl2pix(void* rgb, double c, double s, double l) {
  double L, a, b;
  csl2lab(&L,&a,&b,c,s,l);
  lab2pix(rgb,L,a,b);
}

void hsv2pix(void* rgb, double h, double s, double v) {
  double c = v*s;
  double r,g,b;
  h*=6;
  if(h<1) {
    r = c; g = c*h; b = 0;
  } else if(h<2) {
    r = c*(2-h); g = c; b = 0;
  } else if(h<3) {
    r = 0; g = c; b = c*(h-2);
  } else if(h<4) {
    r = 0; g = c*(4-h); b = c;
  } else if(h<5) {
    r = c*(h-4); g = 0; b = c;
  } else {
    r = c; g = 0; b = c*(6-h);
  }
  double m = v-c;
  r+=m; g+=m; b+=m;
  unsigned char* pix = (unsigned char*)rgb;
  *(pix+0) = (unsigned char)(255.0*r);
  *(pix+1) = (unsigned char)(255.0*g);
  *(pix+2) = (unsigned char)(255.0*b);
}

#ifdef USE_PNG
void export_png(char* filename, int width, int height, int bpc, void* data) {
  FILE* fp = fopen(filename,"wb");
  png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
  png_infop info = png_create_info_struct(png);
  png_init_io(png,fp);
  png_byte color_type, bit_depth;
  int bpp;
  switch(bpc) {
    case 9:  bpp=8;  /* Gray:   8+1 */ bit_depth=8; color_type=PNG_COLOR_TYPE_GRAY; break;
    case 17: bpp=16; /* Gray:  16+1 */ bit_depth=16; color_type=PNG_COLOR_TYPE_GRAY; break;
    case 10: bpp=16; /* GA:     8+2 */ bit_depth=8; color_type=PNG_COLOR_TYPE_GRAY_ALPHA; break;
    case 11: bpp=24; /* Color:  8+3 */ bit_depth=8; color_type=PNG_COLOR_TYPE_RGB; break;
    case 12: bpp=32; /* RGBA:   8+4 */ bit_depth=8; color_type=PNG_COLOR_TYPE_RGB_ALPHA; break;
    case 18: bpp=32; /* GA:    16+2 */ bit_depth=16; color_type=PNG_COLOR_TYPE_GRAY_ALPHA; break;
    case 19: bpp=48; /* RGB:   16+3 */ bit_depth=16; color_type=PNG_COLOR_TYPE_RGB; break;
    case 20: bpp=64; /* RGBA:  16+4 */ bit_depth=16; color_type=PNG_COLOR_TYPE_RGB_ALPHA; break;
  }
  png_set_IHDR(png, info, width, height, bit_depth, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  bpp/=8;
  png_bytep* rows = malloc(height*sizeof(png_bytep));
  int i;
  for(i=0;i<height;i++) rows[i]=data+bpp*width*i;
  png_set_rows(png, info, rows);
  png_write_png(png,info,PNG_TRANSFORM_SWAP_ENDIAN,NULL);
  free(rows);
  fclose(fp);
}
#endif
