#include "util.h"
#include "parameter.h"


	
AnimationKey::AnimationKey( int typeKey, double position, double value )
{
	keyType = typeKey;
	x = position;
	y = value;
}



double AnimationGraph::valueAt( double time )
{
	int i;
	for ( i = keys.count() - 1; i > -1; --i ) {
		if ( keys[i].x <= time ) {
			AnimationKey k1 = keys[i];
			if ( i == keys.count() - 1 )
				return k1.y;
				
			AnimationKey k2 = keys[i+1];
			switch ( k1.keyType ) {
				case AnimationKey::LINEAR:
					return linearInterpolate( k1.y, k2.y, ( time - k1.x ) / ( k2.x - k1.x ) );
				case AnimationKey::CURVE:
					return cosineInterpolate( k1.y, k2.y, ( time - k1.x ) / ( k2.x - k1.x ) );
				default:
					return k1.y;
			}
		}
	}
	return keys[0].y;
}
