/**
* Version.h - Defines the VideoGrace version constants
*
* Author: Anton (ud) Golovkov, udattsk@gmail.com
* Copyright (C), Infinity Video Soft LLC, 2018 - 2024
*/

#pragma once

#define SYSTEM_NAME "VideoGrace"
#define COMPANY_NAME "Infinity Video Soft LLC"
#define PRODUCT_COPYRIGHT "Copyright (c) 2014 - 2024, IVS LLC"

#define CLOUD_ADDRESS "https://cloud.videograce.com:8778"
#define BROWSER_PROTO "vg"
#define CLIENT_USER_FOLDER "vgclient"
#define SERVER_USER_FOLDER "vgserver"

#define CLIENT_INSTALLER_GUID "{8ba36c3a-32d6-48b2-a476-724ba774bb56}"

#define MAJOR_VERSION 2
#define MINOR_VERSION 0
#define RELEASE 240113

#define CLIENT_VERSION 527
#define SERVER_VERSION 316

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define SHORT_VERSION STR(MAJOR_VERSION) "." STR(MINOR_VERSION)
#define SYSTEM_VERSION STR(MAJOR_VERSION) "." STR(MINOR_VERSION) "." STR(RELEASE)
