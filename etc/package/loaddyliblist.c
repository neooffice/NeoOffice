#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syslimits.h>

int main( int argc, char **argv )
{
	int ret = 0;

	// Load all dependent libraries in command line arguments first
	for ( int i = 1; i < argc; i++ )
	{
		void *lib = dlopen( argv[ i ], RTLD_NOW | RTLD_GLOBAL );
		if ( lib )
		{
			// Do not unload as it is assumed that the libraries read from
			// stdin will need this library's symbols
		}
		else
		{
			fprintf( stderr, "%s:\n%s\n", argv[ i ], dlerror() );
			ret = 1;
		}
	}

	// Load and unload all libraries read from stdin
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
