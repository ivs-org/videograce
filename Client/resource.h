/**
 * resource.h - Contains resource defines
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2014 - 2022
 */

#pragma once

#ifdef _WIN32

#define ICO_ACTIVE                     101
#define ICO_INACTIVE                   102

#define TXT_DARK_THEME                 200
#define TXT_LIGHT_THEME                201

#define TXT_LOCALE_EN                  210
#define TXT_LOCALE_RU                  211
#define TXT_LOCALE_KK                  212

#define IMG_LOGO                       4010
#define IMG_CONNECTION_CLOUD           4028
#define IMG_CONNECTION_LOCAL           4029
#define IMG_CHECK_ON                   4030
#define IMG_CHECK_OFF                  4031
#define IMG_CL_SYM_CONF_MY             4037
#define IMG_CL_SYM_CONF                4038
#define IMG_CL_ASYM_CONF               4039
#define IMG_CL_ASYM_CONF_MY            4040
#define IMG_CL_ASYM_SS_CONF            4041
#define IMG_CL_ASYM_SS_CONF_MY         4042
#define IMG_CL_GROUP_EXPANDED          4043
#define IMG_CL_GROUP_ROLLED            4044
#define IMG_CL_CONFERENCES_EXPANDED    4045
#define IMG_CL_CONFERENCES_ROLLED      4046
#define IMG_CL_ACCOUNT_DELETED         4047
#define IMG_CL_ACCOUNT_ONLINE          4048
#define IMG_CL_ACCOUNT_OFFLINE         4049
#define IMG_CALLIN                     4070
#define IMG_CALLOUT                    4071
#define IMG_MSG_CREATED                4074
#define IMG_MSG_SENDED                 4075
#define IMG_MSG_DELIVERED              4076
#define IMG_MSG_READED                 4077
#define IMG_REPLY                      4080
#define IMG_TB_CALL                    4210
#define IMG_TB_CONFERENCE              4211
#define IMG_TB_HANGUP                  4212
#define IMG_TB_SEPARATOR               4213
#define IMG_TB_EXPAND                  4214
#define IMG_TB_CAMERA                  4215
#define IMG_TB_CAMERA_DISABLED         4216
#define IMG_TB_MICROPHONE              4217
#define IMG_TB_MICROPHONE_DISABLED     4218
#define IMG_TB_LOUDSPEAKER             4219
#define IMG_TB_LOUDSPEAKER_DISABLED    4220
#define IMG_TB_SCREENCAPTURE           4221
#define IMG_TB_SCREENCAPTURE_DISABLED  4222
#define IMG_TB_BIGGRID                 4223
#define IMG_TB_EVENGRID                4224
#define IMG_TB_HAND                    4225
#define IMG_TB_LISTPANEL               4226
#define IMG_TB_LISTPANEL_DISABLED      4227
#define IMG_TB_CONTENTPANEL            4228
#define IMG_TB_CONTENTPANEL_DISABLED   4229
#define IMG_TB_FULLSCREEN              4230
#define IMG_TB_MENU                    4231
#define IMG_TB_SEND                    4236
#define IMG_TB_CL_ADD                  4250
#define IMG_TB_CL_DEL                  4251
#define IMG_TB_CL_EDIT                 4252
#define IMG_TB_CL_SEARCH               4253
#define IMG_TB_CL_SEPARATOR            4254
#define IMG_TB_ML_ADD                  4260
#define IMG_TB_ML_DEL                  4261
#define IMG_TB_ML_UP                   4262
#define IMG_TB_ML_MUTE_ALL             4263
#define IMG_TB_ML_SPEAK                4264
#define IMG_TB_ML_DEVICES              4265
#define IMG_TB_ML_CAMERA               4266
#define IMG_TB_ML_MICROPHONE           4267
#define IMG_TB_ML_SCREENCAPTURE        4268
#define IMG_TB_ML_SEPARATOR            4269
#define IMG_ML_OWNER                   4280
#define IMG_ML_MODERATOR               4281
#define IMG_ML_ORDINARY                4282
#define IMG_ML_READONLY                4283
#define IMG_ML_DEAF                    4284
#define IMG_ML_CAMERA_MICROPHONE_ENABLED 4285
#define IMG_ML_CAMERA_ENABLED          4286
#define IMG_ML_MICROPHONE_ENABLED      4287
#define IMG_ML_SPEAK_ENABLED           4288
#define IMG_MC_ADD_TO_MEMBERS          4320
#define IMG_MC_UP_MEMBER               4321
#define IMG_MC_DOWN_MEMBER             4322
#define IMG_MC_DEL_MEMBER              4323
#define IMG_MC_CLEAR_MEMBERS           4324
#define IMG_MW_CAMERA_ON               4330
#define IMG_MW_CAMERA_OFF              4331
#define IMG_MW_MICROPHONE_ON           4332
#define IMG_MW_MICROPHONE_OFF          4333
#define IMG_SETTING_CAMERA             4340
#define IMG_SETTING_MICROPHONE         4341
#define IMG_SETTING_LOUDSPEAKER        4342
#define IMG_SETTING_ACCOUNT            4343
#define IMG_SETTING_CONNECTION         4345
#define IMG_SETTING_PREFERENCES        4346
#define IMG_SETTING_RECORD             4347
#define IMG_SETTING_NO_SIGNAL          4348
#define IMG_TB_REMOTE_CONTROL          4349
#define IMG_TB_SCALE                   4350
#define IMG_LOADING_01                 4360
#define IMG_LOADING_02                 4361
#define IMG_LOADING_03                 4362
#define IMG_LOADING_04                 4363

#define SND_CALLIN                     5001
#define SND_CALLOUT                    5002
#define SND_SCHEDULECONNECT            5003
#define SND_DIAL                       5004
#define SND_HANGUP                     5005
#define SND_NEW_MESSAGE                5006

#else

static constexpr int TXT_DARK_THEME = 0, TXT_LIGHT_THEME = 0, TXT_LOCALE_EN = 0, TXT_LOCALE_RU = 0, TXT_LOCALE_KK = 0;

static constexpr const char* ICO_ACTIVE                      = "active_icon.png";
static constexpr const char* ICO_INACTIVE                    = "inactive_icon.png";

static constexpr const char* IMG_LOGO                        = "logo.png";
static constexpr const char* IMG_CONNECTION_CLOUD            = "Connection/cloud.png";
static constexpr const char* IMG_CONNECTION_LOCAL            = "Connection/local.png";
static constexpr const char* IMG_CHECK_ON                    = "check_on.png";
static constexpr const char* IMG_CHECK_OFF                   = "check_off.png";
static constexpr const char* IMG_CL_SYM_CONF                 = "ContactList/sym_conf.png";
static constexpr const char* IMG_CL_SYM_CONF_MY              = "ContactList/sym_conf_my.png";
static constexpr const char* IMG_CL_ASYM_CONF                = "ContactList/asym_conf.png";
static constexpr const char* IMG_CL_ASYM_CONF_MY             = "ContactList/asym_conf_my.png";
static constexpr const char* IMG_CL_ASYM_SS_CONF             = "ContactList/asym_ss_conf.png";
static constexpr const char* IMG_CL_ASYM_SS_CONF_MY          = "ContactList/asym_ss_conf_my.png";
static constexpr const char* IMG_CL_GROUP_EXPANDED           = "ContactList/group_expanded.png";
static constexpr const char* IMG_CL_GROUP_ROLLED             = "ContactList/group_rolled.png";
static constexpr const char* IMG_CL_CONFERENCES_EXPANDED     = "ContactList/conferences_expanded.png";
static constexpr const char* IMG_CL_CONFERENCES_ROLLED       = "ContactList/conferences_rolled.png";
static constexpr const char* IMG_CL_ACCOUNT_DELETED          = "ContactList/account_deleted.png";
static constexpr const char* IMG_CL_ACCOUNT_ONLINE           = "ContactList/account_online.png";
static constexpr const char* IMG_CL_ACCOUNT_OFFLINE          = "ContactList/account_offline.png";
static constexpr const char* IMG_CALLIN                      = "call_in.png";
static constexpr const char* IMG_CALLOUT                     = "call_out.png";
static constexpr const char* IMG_MSG_CREATED                 = "ContentPanel/msg_created.png";
static constexpr const char* IMG_REPLY                       = "ContentPanel/reply.png";
static constexpr const char* IMG_MSG_SENDED                  = "ContentPanel/msg_sended.png";
static constexpr const char* IMG_MSG_DELIVERED               = "ContentPanel/msg_delivered.png";
static constexpr const char* IMG_MSG_READED                  = "ContentPanel/msg_readed.png";
static constexpr const char* IMG_TB_CALL                     = "MainToolbar/call.png";
static constexpr const char* IMG_TB_CONFERENCE               = "MainToolbar/conference.png";
static constexpr const char* IMG_TB_HANGUP                   = "MainToolbar/hangup.png";
static constexpr const char* IMG_TB_SEPARATOR                = "MainToolbar/separator.png";
static constexpr const char* IMG_TB_EXPAND                   = "MainToolbar/expand.png";
static constexpr const char* IMG_TB_CAMERA                   = "MainToolbar/camera.png";
static constexpr const char* IMG_TB_CAMERA_DISABLED          = "MainToolbar/camera_disabled.png";
static constexpr const char* IMG_TB_MICROPHONE               = "MainToolbar/microphone.png";
static constexpr const char* IMG_TB_MICROPHONE_DISABLED      = "MainToolbar/microphone_disabled.png";
static constexpr const char* IMG_TB_LOUDSPEAKER              = "MainToolbar/loudspeaker.png";
static constexpr const char* IMG_TB_LOUDSPEAKER_DISABLED     = "MainToolbar/loudspeaker_disabled.png";
static constexpr const char* IMG_TB_SCREENCAPTURE            = "MainToolbar/screencapture.png";
static constexpr const char* IMG_TB_SCREENCAPTURE_DISABLED   = "MainToolbar/screencapture_disabled.png";
static constexpr const char* IMG_TB_BIGGRID                  = "MainToolbar/biggrid.png";
static constexpr const char* IMG_TB_EVENGRID                 = "MainToolbar/evengrid.png";
static constexpr const char* IMG_TB_HAND                     = "MainToolbar/hand.png";
static constexpr const char* IMG_TB_LISTPANEL                = "MainToolbar/listpanel.png";
static constexpr const char* IMG_TB_LISTPANEL_DISABLED       = "MainToolbar/listpanel_disabled.png";
static constexpr const char* IMG_TB_CONTENTPANEL             = "MainToolbar/contentpanel.png";
static constexpr const char* IMG_TB_CONTENTPANEL_DISABLED    = "MainToolbar/contentpanel_disabled.png";
static constexpr const char* IMG_TB_FULLSCREEN               = "MainToolbar/fullscreen.png";
static constexpr const char* IMG_TB_MENU                     = "MainToolbar/menu.png";
static constexpr const char* IMG_TB_SEND                     = "ContentPanel/send.png";
static constexpr const char* IMG_TB_CL_ADD                   = "ContactList/add.png";
static constexpr const char* IMG_TB_CL_DEL                   = "ContactList/del.png";
static constexpr const char* IMG_TB_CL_EDIT                  = "ContactList/edit.png";
static constexpr const char* IMG_TB_CL_SEARCH                = "ContactList/search.png";
static constexpr const char* IMG_TB_CL_SEPARATOR             = "ContactList/separator.png";
static constexpr const char* IMG_TB_ML_ADD                   = "MemberList/ml_add.png";
static constexpr const char* IMG_TB_ML_DEL                   = "MemberList/ml_del.png";
static constexpr const char* IMG_TB_ML_UP                    = "MemberList/ml_up.png";
static constexpr const char* IMG_TB_ML_MUTE_ALL              = "MemberList/ml_mute_all.png";
static constexpr const char* IMG_TB_ML_SPEAK                 = "MemberList/ml_speak.png";
static constexpr const char* IMG_TB_ML_DEVICES               = "MemberList/ml_devices.png";
static constexpr const char* IMG_TB_ML_CAMERA                = "MemberList/ml_camera.png";
static constexpr const char* IMG_TB_ML_MICROPHONE            = "MemberList/ml_microphone.png";
static constexpr const char* IMG_TB_ML_SCREENCAPTURE         = "MemberList/ml_screencapture.png";
static constexpr const char* IMG_TB_ML_SEPARATOR             = "MemberList/ml_separator.png";
static constexpr const char* IMG_ML_OWNER                    = "MemberList/owner_member.png";
static constexpr const char* IMG_ML_MODERATOR                = "MemberList/moderator_member.png";
static constexpr const char* IMG_ML_ORDINARY                 = "MemberList/ordinary_member.png";
static constexpr const char* IMG_ML_READONLY                 = "MemberList/readonly_member.png";
static constexpr const char* IMG_ML_DEAF                     = "MemberList/deaf_member.png";
static constexpr const char* IMG_ML_CAMERA_MICROPHONE_ENABLED = "MemberList/ml_camera_microphone_enabled.png";
static constexpr const char* IMG_ML_CAMERA_ENABLED           = "MemberList/ml_camera_enabled.png";
static constexpr const char* IMG_ML_MICROPHONE_ENABLED       = "MemberList/ml_microphone_enabled.png";
static constexpr const char* IMG_ML_SPEAK_ENABLED            = "MemberList/ml_speak_enabled.png";
static constexpr const char* IMG_MC_ADD_TO_MEMBERS           = "ModifyConference/add_to_members.png";
static constexpr const char* IMG_MC_UP_MEMBER                = "ModifyConference/up_member.png";
static constexpr const char* IMG_MC_DOWN_MEMBER              = "ModifyConference/down_member.png";
static constexpr const char* IMG_MC_DEL_MEMBER               = "ModifyConference/del_member.png";
static constexpr const char* IMG_MC_CLEAR_MEMBERS            = "ModifyConference/clear_members.png";
static constexpr const char* IMG_MW_CAMERA_ON                = "MiniWindow/camera_on.png";
static constexpr const char* IMG_MW_CAMERA_OFF               = "MiniWindow/camera_off.png";
static constexpr const char* IMG_MW_MICROPHONE_ON            = "MiniWindow/microphone_on.png";
static constexpr const char* IMG_MW_MICROPHONE_OFF           = "MiniWindow/microphone_off.png";
static constexpr const char* IMG_SETTING_CAMERA              = "Settings/camera.png";
static constexpr const char* IMG_SETTING_MICROPHONE          = "Settings/microphone.png";
static constexpr const char* IMG_SETTING_LOUDSPEAKER         = "Settings/loudspeaker.png";
static constexpr const char* IMG_SETTING_ACCOUNT             = "Settings/account.png";
static constexpr const char* IMG_SETTING_CONNECTION          = "Settings/connection.png";
static constexpr const char* IMG_SETTING_PREFERENCES         = "Settings/preferences.png";
static constexpr const char* IMG_SETTING_RECORD              = "Settings/record.png";
static constexpr const char* IMG_SETTING_NO_SIGNAL           = "Settings/no_signal.png";
static constexpr const char* IMG_TB_REMOTE_CONTROL           = "ScreenCapture/remote_control.png";
static constexpr const char* IMG_TB_SCALE                    = "ScreenCapture/scale.png";
static constexpr const char* IMG_LOADING_01                  = "Loading/01.png";
static constexpr const char* IMG_LOADING_02                  = "Loading/02.png";
static constexpr const char* IMG_LOADING_03                  = "Loading/03.png";
static constexpr const char* IMG_LOADING_04                  = "Loading/04.png";

static constexpr const char* SND_CALLIN                      = "sounds/CallIn.wav";
static constexpr const char* SND_CALLOUT                     = "sounds/CallOut.wav";
static constexpr const char* SND_SCHEDULECONNECT             = "sounds/ScheduleConnect.wav";
static constexpr const char* SND_DIAL                        = "sounds/Dial.wav";
static constexpr const char* SND_HANGUP                      = "sounds/Hangup.wav";
static constexpr const char* SND_NEW_MESSAGE                 = "sounds/NewMessage.wav";

#endif
