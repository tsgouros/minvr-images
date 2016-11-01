#ifndef MATRIX_H
#define MATRIX_H

typedef float Matrix [16];

#ifndef DEG2RAD
#define DEG2RAD (M_PI/180.0)
#endif

extern void identity_matrix(float *m);
extern void copy_matrix(float *m, float *m2);
extern void translate_matrix(float x, float y, float z, float m[] );
extern void rotate_matrix( float angle, float x, float y, float z,
                    float m[] );
extern void multiply_matrix( float *product, const float *a, const float *b );

extern void scale_matrix(float x, float y, float z, float m[] );
extern void uscale_matrix(float s, float m[] );
extern void multPoint(float *m, float *p);

#endif
