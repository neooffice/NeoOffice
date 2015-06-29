#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

int main( int argc, char **argv )
{
	int ret = 0;

	char *line = NULL;
	size_t len = 0;
	ssize_t read = 0;
	while ( ( read = getline( &line, &len, stdin ) ) >= 0 )
	{
		if ( read && line[read - 1] == '\n' )
			line[read - 1] = '\0';
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

	if ( line )
		free( line );

	exit( ret );
}
