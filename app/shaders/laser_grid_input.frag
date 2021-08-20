float PREFIX(makePoint)( float x, float y, float fx, float fy, float sx, float sy ) {
   float t = PREFIX(time) / 10.0;
   float xx = 1.0 * ( x + sin( t * fx ) * cos( t * sx ) / 0.2 );
   float yy = 1.0 * ( y + cos( t * fy ) * sin( t * sy ) / 0.3);
   return 0.4 / sqrt( abs( xx * yy ) );
}

vec4 FUNCNAME(vec2 tc) {
   vec2 p = vec2(tc.x * PREFIX(iwidth), tc.y * PREFIX(iheight)) / PREFIX(iwidth) * 2.0 - vec2( 1.0, PREFIX(iheight) / PREFIX(iwidth) );
   p = p * 3.0;
   float x = p.x;
   float y = p.y;

   float a = PREFIX(makePoint)( x, y, 3.3, 2.9, 1.3, 0.3 );
   a += PREFIX(makePoint)( x, y, 1.9, 2.0, 0.4, 0.4 );
   a += PREFIX(makePoint)( x, y, 0.2, 0.7, 0.4, 0.5 );

   float b = PREFIX(makePoint)( x, y, 1.2, 1.9, 0.3, 0.3 );
   b += PREFIX(makePoint)( x, y, 0.7, 2.7, 0.4, 4.0 );
   b += PREFIX(makePoint)( x, y, 1.4, 0.6, 0.4, 0.5 );
   b += PREFIX(makePoint)( x, y, 2.6, 0.4, 0.6, 0.3 );
   b += PREFIX(makePoint)( x, y, 0.1, 1.4, 0.5, 0.4 );
   b += PREFIX(makePoint)( x, y, 0.7, 1.7, 0.4, 0.4 );
   b += PREFIX(makePoint)( x, y, 0.8, 0.5, 0.4, 0.5 );
   b += PREFIX(makePoint)( x, y, 1.4, 0.9, 0.6, 0.3 );
   b += PREFIX(makePoint)( x, y, 0.7, 1.3, 0.5, 0.4 );

   float c = PREFIX(makePoint)( x, y, 3.7, 0.3, 0.3, 0.3 );
   c += PREFIX(makePoint)( x, y, 1.9, 1.3, 0.4, 0.4 );
   c += PREFIX(makePoint)( x, y, 0.8, 0.9, 0.4, 0.5 );
   c += PREFIX(makePoint)( x, y, 1.2, 1.7, 0.6, 0.3 );
   c += PREFIX(makePoint)( x, y, 0.3, 0.6, 0.5, 0.4 );
   c += PREFIX(makePoint)( x, y, 0.3, 0.3, 0.4, 0.4 );
   c += PREFIX(makePoint)( x, y, 1.4, 0.8, 0.4, 0.5 );
   c += PREFIX(makePoint)( x, y, 0.2, 0.6, 0.6, 0.3 );
   c += PREFIX(makePoint)( x, y, 1.3, 0.5, 0.5, 0.4 );

   vec3 d = vec3( b * c, a * c, a * b ) / 500.0;
   return clamp(vec4( d, 1.0 ), vec4(0.0), vec4(1.0));
}
