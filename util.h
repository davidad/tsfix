#ifndef _UTILS
#define _UTILS

#define TAU 6.283185307179586476925287

#define FOREACH3(M) \
  M(2) \
  M(1) \
  M(0)

void* open_mmapped_file_read(const char*, int*);
void* open_mmapped_file_write(const char*, int);
void copy_file(const char* dest, const char* src);

void lab2xyz(double* x, double* y, double* z, double l, double a, double b);
void xyz2rgb(unsigned char* r, unsigned char* g, unsigned char* b, double x, double y, double z);
void lab2rgb(unsigned char* R, unsigned char* G, unsigned char* B, double l, double a, double b);
void lab2pix(void* rgb, double l, double a, double b);
void xyz2pix(void* rgb, double x, double y, double z);
void cl2pix(void* rgb, double c, double l);
void csl2lab(double* L, double* a, double* b, double c, double s, double l);
void csl2pix(void* rgb, double c, double s, double l);
void lab2pix(void* rgb, double L, double a, double b);
void hsv2pix(void* rgb, double h, double s, double v);
#ifdef USE_PNG
void export_png(char* filename, int width, int height, int bpc, void* data);
#endif

#endif
