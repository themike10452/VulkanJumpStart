#include <assert.h>

#define REQUIRE_PTR( PTR ) assert( PTR );

#define REQUIRE_PTR_OR_RETURN( PTR ) if (!PTR) return;

#define VTEXT( TXT ) ( (char*) TXT )
