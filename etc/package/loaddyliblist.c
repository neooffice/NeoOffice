#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syslimits.h>

int main( int argc, char **argv )
{
	int ret = 0;

	char line[ PATH_MAX ];
	while ( fgets( line, sizeof( line ), stdin ) )
	{
		size_t len = strlen( line );
		if ( len && line[ len - 1 ] == '\n' )
			line[ len - 1 ] = '\0';

		void *lib = dlopen( line, RTLD_NOW | RTLD_GLOBAL );
		if ( lib )
		{
			dlclose( lib );
		}
		else
		{
			fprintf( stderr, "%s:\n%s\n", line, dlerror() );
			ret = 1;
		}
	}

	exit( ret );
}
