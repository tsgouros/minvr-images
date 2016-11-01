#include <stdio.h>
#include <math.h>

#ifndef DEG2RAD
#define DEG2RAD (M_PI/180.0)
#endif

void identity_matrix(float *m) {
  int i;
  for (i =0;  i< 16; i++) {
    m[i] = 0;
  }
  m[0] = m[5] = m[10] = m[15] = 1.0;
}

void multPoint(float *m, float *p){
	double tmp [4];
	tmp[0] = m[0] * p[0] + m[4] * p[1] + m[8] * p[2] + m[12] ;
	tmp[1] = m[1] * p[0] + m[5] * p[1] + m[9] * p[2] + m[13] ;
	tmp[2] = m[2] * p[0] + m[6] * p[1] + m[10] * p[2] + m[14] ;
	tmp[3] = m[3] * p[0] + m[7] * p[1] + m[11] * p[2] + m[15] ;
	
	p[0] = tmp[0] / tmp[3];
	p[1] = tmp[1] / tmp[3];
	p[2] = tmp[2] / tmp[3];
}


void multiply_matrix( float *product, const float *a, const float *b )
{
   /* This matmul was contributed by Thomas Malik */
   int i;
   float p[16];
#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define P(row,col)  p[(col<<2)+row]

   /* i-te Zeile */
   for (i = 0; i < 4; i++) {
      float ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
      P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
      P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
      P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
      P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
   }
   
   for (i = 0; i<16; i++) product[i] = p[i];

#undef A
#undef B
#undef P
}


void translate_matrix(float x, float y, float z, float *m) {
  m[12] += x;
  m[13] += y;
  m[14] += z;
}

void scale_matrix(float x, float y, float z, float m[] ) {
  m[0] = m[0] * x;
  m[5] = m[5] * y;
  m[10] = m[10] * z;
}


void uscale_matrix(float s, float m[] ) {
  scale_matrix (s, s, s, m);
}


void copy_matrix(float *m, float *m2) {
  int i;
  for (i = 0; i < 16; i++) {
    m2[i] = m[i];
  }
}

/*
 * Generate a 4x4 transformation matrix from glRotate parameters.
 */
void rotate_matrix( float angle, float x, float y, float z,
                         float *m )
{
   /* This function contributed by Erich Boleyn (erich@uruk.org) */
   float mag, s, c;
   float xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c;

   s = sin( angle * DEG2RAD );
   c = cos( angle * DEG2RAD );

   mag = sqrt( x*x + y*y + z*z );

   if (mag == 0.0) {
      /* generate an identity matrix and return */
/*      //MEMCPY(m, Identity, sizeof(float)*16); */
      return;
   }

   x /= mag;
   y /= mag;
   z /= mag;

#define M(row,col)  m[col*4+row]

   xx = x * x;
   yy = y * y;
   zz = z * z;
   xy = x * y;
   yz = y * z;
   zx = z * x;
   xs = x * s;
   ys = y * s;
   zs = z * s;
   one_c = 1.0F - c;

   M(0,0) = (one_c * xx) + c;
   M(0,1) = (one_c * xy) - zs;
   M(0,2) = (one_c * zx) + ys;
   M(0,3) = 0.0F;

   M(1,0) = (one_c * xy) + zs;
   M(1,1) = (one_c * yy) + c;
   M(1,2) = (one_c * yz) - xs;
   M(1,3) = 0.0F;

   M(2,0) = (one_c * zx) - ys;
   M(2,1) = (one_c * yz) + xs;
   M(2,2) = (one_c * zz) + c;
   M(2,3) = 0.0F;

   M(3,0) = 0.0F;
   M(3,1) = 0.0F;
   M(3,2) = 0.0F;
   M(3,3) = 1.0F;

#undef M
}
