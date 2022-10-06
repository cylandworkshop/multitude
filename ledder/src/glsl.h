typedef struct {
  uint16_t x;
  uint16_t y;
} vec2;

enum {
  _A1=3370945099UL,
  _B1=2746362156UL,
  _C1=292421UL
};
enum {n=13, p=32, q=31, r=3, a=12};
    
int16_t fpsin(int16_t i)
{
    /* Convert (signed) input to a value between 0 and 8192. (8192 is pi/2, which is the region of the curve fit). */
    /* ------------------------------------------------------------------- */
    i <<= 1;
    uint8_t c = i<0; //set carry for output pos/neg

    if(i == (i|0x4000)) // flip input value to corresponding value in range [0..8192)
        i = (1<<15) - i;
    i = (i & 0x7FFF) >> 1;
    /* ------------------------------------------------------------------- */

    /* The following section implements the formula:
     = y * 2^-n * ( _A1 - 2^(q-p)* y * 2^-n * y * 2^-n * [_B1 - 2^-r * y * 2^-n * _C1 * y]) * 2^(a-q)
    Where the constants are defined as follows:
    */
    

    uint32_t y = (_C1*((uint32_t)i))>>n;
    y = _B1 - (((uint32_t)i*y)>>r);
    y = (uint32_t)i * (y>>n);
    y = (uint32_t)i * (y>>n);
    y = _A1 - (y>>(p-q));
    y = (uint32_t)i * (y>>n);
    y = (y+(1UL<<(q-a-1)))>>(q-a); // Rounding

    return c ? -y : y;
}

int16_t sphere(vec2 uv, vec2 center, int16_t r) {
  return(center.x - uv.x) * (center.x - uv.x) + (center.y - uv.y) * (center.y - uv.y) - r * r;
}
