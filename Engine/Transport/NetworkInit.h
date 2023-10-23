/**
 * NetworkInit.h - Contains header of network initializer
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014
 */

#pragma once

#ifdef _WIN32
#include <winsock2.h>

namespace Transport
{
    class NetworkInit
    {
    public:
        NetworkInit()
        {
            WORD wVersionRequested;
            WSADATA wsaData;
            int err;

            wVersionRequested = MAKEWORD( 2, 2 );

            err = WSAStartup( wVersionRequested, &wsaData );

            if ( err != 0 )
            {
                /* Tell the user that we could not find a usable */
                /* WinSock DLL.                                  */
                return;
            }

            /* Confirm that the WinSock DLL supports 2.2.*/
            /* Note that if the DLL supports versions greater    */
            /* than 2.2 in addition to 2.2, it will still return */
            /* 2.2 in wVersion since that is the version we      */
            /* requested.                                        */

            if ( LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 2 )
            {
                /* Tell the user that we could not find a usable */
                /* WinSock DLL.                                  */
                WSACleanup( );
                return;
            }
        }

        ~NetworkInit()
        {
            WSACleanup();
        }
    };
}
#else
namespace Transport
{
    class NetworkInit
    {
    public:
        NetworkInit()
        {
        }

        ~NetworkInit()
        {
        }
    };
}
#endif
