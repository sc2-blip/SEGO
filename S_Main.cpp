#include <csignal>
#include "Local.h"

static volatile sig_atomic_t    s_sigCaught;

static void S_SigHandler( int sig )
{
    s_sigCaught = sig;
}

int S_SignalCaught( void )
{
    return s_sigCaught;
}

void S_InitSignals( void )
{
    signal ( SIGINT, S_SigHandler ); // Ctrl + C
    signal ( SIGTERM, S_SigHandler ); // kill signal
}

int main( int argc, char **argv )
{
    Con_Init();
    Com_Init( argc, argv );

    while ( 1 )
    {
        Com_Frame();
    }

    return 0;
}
