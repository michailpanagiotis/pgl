#ifndef BINARYMATH_H
#define BINARYMATH_H

//changed comment

inline unsigned int modulusPow2( unsigned int divident, unsigned int powerOf2Divisor)
{
    return divident & (powerOf2Divisor - 1);
}

inline unsigned int pow2( unsigned int i)
{
    return 1 << i;
}

inline unsigned int nextPowerOf2( unsigned int val)
{
    val--;
    val = (val >> 1) | val;
    val = (val >> 2) | val;
    val = (val >> 4) | val;
    val = (val >> 8) | val;
    val = (val >> 16) | val;
    val++; // Val is now the next highest power of 2.
    return val;
}

inline unsigned int maxpow2divisor( unsigned int i)
{
    unsigned int mask = 1;
    while( (i & mask) == 0)
    {
        mask <<= 1;
    }
    return mask;
}

inline unsigned int trailing_zeros2( unsigned int i)
{
    unsigned int b = i & (~i+1);   // this gives a 1 to the left of the trailing 1's
    b--;              // this gets us just the trailing 1's that need counting
    b = (b & 0x55555555) + ((b>>1) & 0x55555555);  // 2 bit sums of 1 bit numbers
    b = (b & 0x33333333) + ((b>>2) & 0x33333333);  // 4 bit sums of 2 bit numbers
    b = (b & 0x0f0f0f0f) + ((b>>4) & 0x0f0f0f0f);  // 8 bit sums of 4 bit numbers
    b = (b & 0x00ff00ff) + ((b>>8) & 0x00ff00ff);  // 16 bit sums of 8 bit numbers
    b = (b & 0x0000ffff) + ((b>>16) & 0x0000ffff); // sum of 16 bit numbers
    return b;
}


inline int trailing_zeros(unsigned x){
    int n;
    if (x == 0) return(32);
    n = 1;
    if ((x & 0x0000FFFF) == 0) {n = n +16; x = x >>16;}
    if ((x & 0x000000FF) == 0) {n = n + 8; x = x >> 8;}
    if ((x & 0x0000000F) == 0) {n = n + 4; x = x >> 4;}
    if ((x & 0x00000003) == 0) {n = n + 2; x = x >> 2;}
    return n - (x & 1); 
} 

inline int floorLog2(unsigned int n) {
        unsigned int pos = 0;
        if (n >= 1<<16) { n >>= 16; pos += 16; }
        if (n >= 1<< 8) { n >>=  8; pos +=  8; }
        if (n >= 1<< 4) { n >>=  4; pos +=  4; }
        if (n >= 1<< 2) { n >>=  2; pos +=  2; }
        if (n >= 1<< 1) {           pos +=  1; }
        return ((n == 0) ? (-1) : pos);
}

inline int ceilLog2(unsigned int n) {
        unsigned int pos = 0;
        if (n >= 1<<16) { n >>= 16; pos += 16; }
        if (n >= 1<< 8) { n >>=  8; pos +=  8; }
        if (n >= 1<< 4) { n >>=  4; pos +=  4; }
        if (n >= 1<< 2) { n >>=  2; pos +=  2; }
        if (n >= 1<< 1) {           pos +=  1; }
        return ((n == 0) ? (-1) : (pos + 1));
}

inline int isPowerOf2 (unsigned int x)
{
 while (((x & 1) == 0) && x > 1) /* While x is even and > 1 */
   x >>= 1;
 return (x == 1);
}


#endif //BINARYMATH_H
