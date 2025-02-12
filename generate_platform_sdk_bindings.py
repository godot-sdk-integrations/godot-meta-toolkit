
import os
import glob
import re

# Don't parse these OVR headers.
EXCLUDE_OVR_HEADERS = [
    'OVR_Platform_Defs.h',
    'OVR_Types.h',
    # This one is PC only.
    'OVR_Requests_ApplicationLifecycle.h',
]

# Exclude these OVR functions from both source and header generation.
EXCLUDE_OVR_FUNCTIONS = [
    # All of the initialization functions are handled specially.
    'ovr_PlatformInitializeAndroid',
    'ovr_PlatformInitializeAndroidWithOptions',
    'ovr_PlatformInitializeAndroidAsynchronous',
    'ovr_InitConfigOption_CreateBool',
    'ovrInitConfigOption_toString',
    'ovr_Platform_InitializeStandaloneOculus',
    'ovr_Platform_InitializeStandaloneOculusEx',
    'ovr_PlatformInitializeWithAccessToken',
    'ovr_PlatformInitializeWithAccessTokenEx',
    'ovr_PlatformInitializeWithAccessTokenAndOptions',
    'ovr_PlatformInitializeWindows',
    'ovr_PlatformInitializeWindowsEx',
    'ovr_PlatformInitializeWindowsAsynchronous',
    'ovr_PlatformInitializeWindowsAsynchronousEx',
    'ovr_SetDeveloperAccessToken',

    # Message handling isn't directly exposed.
    'ovr_PopMessage',
    'ovr_FreeMessage',

    # We want to cache the type in the constructor.
    'ovr_Message_GetType',

    # We want to move this to the MetaPlatformSDK_Message class.
    'ovrMessageType_IsNotification',

    # We need to hand write these so we don't generate them as async.
    'ovr_Message_GetRequestID',
    'ovr_HttpTransferUpdate_GetID',

    # These functions should return PackedByteArray instead of String
    'ovr_ChallengeEntry_GetExtraData',
    'ovr_LeaderboardEntry_GetExtraData',

    # Exclude the deprecated Platform VoIP functions.
    # Note: We need to keep a few that are associated with System VoIP.
    'ovr_Voip_Accept',
    'ovr_Voip_CreateDecoder',
    'ovr_Voip_CreateEncoder',
    'ovr_Voip_DestroyDecoder',
    'ovr_Voip_DestroyEncoder',
    'ovr_Voip_GetIsConnectionUsingDtx',
    'ovr_Voip_GetLocalBitrate',
    'ovr_Voip_GetOutputBufferMaxSize',
    'ovr_Voip_GetPCM',
    'ovr_Voip_GetPCMFloat',
    'ovr_Voip_GetPCMSize',
    'ovr_Voip_GetPCMWithTimestamp',
    'ovr_Voip_GetPCMWithTimestampFloat',
    'ovr_Voip_GetRemoteBitrate',
    'ovr_Voip_GetSyncTimestamp',
    'ovr_Voip_GetSyncTimestampDifference',
    'ovr_Voip_IsConnectionUsingDtx',
    'ovr_Voip_SetMicrophoneFilterCallback',
    'ovr_Voip_SetMicrophoneMuted',
    'ovr_Voip_SetNewConnectionOptions',
    'ovr_Voip_SetOutputSampleRate',
    'ovr_Voip_Start',
    'ovr_Voip_Stop',
    'ovr_VoipOptions_SetBitrateForNewConnections',
    'ovr_VoipOptions_SetCreateNewConnectionUseDtx',
    'ovr_VoipOptions_Create',
    'ovr_VoipOptions_Destroy',
    'ovr_VoipDecoder_Decode',
    'ovr_VoipDecoder_GetDecodedPCM',
    'ovr_VoipEncoder_AddPCM',
    'ovr_VoipEncoder_GetCompressedData',
    'ovr_VoipEncoder_GetCompressedDataSize',
    'ovr_Microphone_Create',
    'ovr_Microphone_Destroy',
    'ovr_Microphone_GetNumSamplesAvailable',
    'ovr_Microphone_GetOutputBufferMaxSize',
    'ovr_Microphone_GetPCM',
    'ovr_Microphone_GetPCMFloat',
    'ovr_Microphone_ReadData',
    'ovr_Microphone_SetAcceptableRecordingDelayHint',
    'ovr_Microphone_SetAudioDataAvailableCallback',
    'ovr_Microphone_Start',
    'ovr_Microphone_Stop',

    # These functions contained the word "DEPRECATED" in their docs in the first version of the
    # Platform SDK that we bound, so they should be omitted.
    'ovr_ApplicationOptions_SetRoomId',
    'ovr_AssetFileDeleteResult_GetAssetFileId',
    'ovr_AssetFileDownloadCancelResult_GetAssetFileId',
    'ovr_AssetFileDownloadUpdate_GetAssetFileId',
    'ovr_AssetFileDownloadUpdate_GetBytesTotal',
    'ovr_AssetFileDownloadUpdate_GetBytesTransferred',
    'ovr_AssetFile_Delete',
    'ovr_AssetFile_Download',
    'ovr_AssetFile_DownloadCancel',
    'ovr_AssetFile_Status',
    'ovr_Challenges_Create',
    'ovr_Challenges_Delete',
    'ovr_Challenges_UpdateInfo',
    'ovr_Purchase_GetPurchaseID',
    'ovr_RichPresenceOptions_Create',
    'ovr_RichPresenceOptions_Destroy',
    'ovr_RichPresenceOptions_SetApiName',
    'ovr_RichPresenceOptions_SetArgsString',
    'ovr_RichPresenceOptions_ClearArgs',
    'ovr_RichPresenceOptions_SetCurrentCapacity',
    'ovr_RichPresenceOptions_SetDeeplinkMessageOverride',
    'ovr_RichPresenceOptions_SetEndTime',
    'ovr_RichPresenceOptions_SetExtraContext',
    'ovr_RichPresenceOptions_SetInstanceId',
    'ovr_RichPresenceOptions_SetIsIdle',
    'ovr_RichPresenceOptions_SetIsJoinable',
    'ovr_RichPresenceOptions_SetJoinableId',
    'ovr_RichPresenceOptions_SetLobbySessionId',
    'ovr_RichPresenceOptions_SetMatchSessionId',
    'ovr_RichPresenceOptions_SetMaxCapacity',
    'ovr_RichPresenceOptions_SetStartTime',
    'ovr_RichPresence_Clear',
    'ovr_RichPresence_Set',
]

# Exclude these OVR functions from source generation only (the headers will still be generated).
EXCLUDE_OVR_FUNCTION_SOURCE = [
    'ovr_HttpTransferUpdate_GetBytes',
    'ovr_Packet_GetBytes',
]

# Maps the OVR message type constants to the OVR function to get their data.
# Note: This is using the OVR names just in case we change how we generate our names.
OVR_FUNCTION_TO_MESSAGE_TYPES = {
    "ovr_Message_GetAchievementDefinitionArray": [
        "ovrMessage_Achievements_GetAllDefinitions",
        "ovrMessage_Achievements_GetDefinitionsByName",
        "ovrMessage_Achievements_GetNextAchievementDefinitionArrayPage",
    ],
    "ovr_Message_GetAchievementProgressArray": [
        "ovrMessage_Achievements_GetAllProgress",
        "ovrMessage_Achievements_GetNextAchievementProgressArrayPage",
        "ovrMessage_Achievements_GetProgressByName",
    ],
    "ovr_Message_GetAchievementUpdate": [
        "ovrMessage_Achievements_AddCount",
        "ovrMessage_Achievements_AddFields",
        "ovrMessage_Achievements_Unlock",
    ],
    "ovr_Message_GetAppDownloadProgressResult": [
        "ovrMessage_Application_CheckAppDownloadProgress",
    ],
    "ovr_Message_GetAppDownloadResult": [
        "ovrMessage_Application_CancelAppDownload",
        "ovrMessage_Application_InstallAppUpdateAndRelaunch",
        "ovrMessage_Application_StartAppDownload",
    ],
    "ovr_Message_GetApplicationInviteArray": [
        "ovrMessage_GroupPresence_GetNextApplicationInviteArrayPage",
        "ovrMessage_GroupPresence_GetSentInvites",
    ],
    "ovr_Message_GetApplicationVersion": [
        "ovrMessage_Application_GetVersion",
    ],
    "ovr_Message_GetAssetDetails": [
        "ovrMessage_AssetFile_Status",
        "ovrMessage_AssetFile_StatusById",
        "ovrMessage_AssetFile_StatusByName",
        "ovrMessage_LanguagePack_GetCurrent",
    ],
    "ovr_Message_GetAssetDetailsArray": [
        "ovrMessage_AssetFile_GetList",
    ],
    "ovr_Message_GetAssetFileDeleteResult": [
        "ovrMessage_AssetFile_Delete",
        "ovrMessage_AssetFile_DeleteById",
        "ovrMessage_AssetFile_DeleteByName",
    ],
    "ovr_Message_GetAssetFileDownloadCancelResult": [
        "ovrMessage_AssetFile_DownloadCancel",
        "ovrMessage_AssetFile_DownloadCancelById",
        "ovrMessage_AssetFile_DownloadCancelByName",
    ],
    "ovr_Message_GetAssetFileDownloadResult": [
        "ovrMessage_AssetFile_Download",
        "ovrMessage_AssetFile_DownloadById",
        "ovrMessage_AssetFile_DownloadByName",
        "ovrMessage_LanguagePack_SetCurrent",
    ],
    "ovr_Message_GetAssetFileDownloadUpdate": [
        "ovrMessage_Notification_AssetFile_DownloadUpdate",
    ],
    "ovr_Message_GetAvatarEditorResult": [
        "ovrMessage_Avatar_LaunchAvatarEditor",
    ],
    "ovr_Message_GetBlockedUserArray": [
        "ovrMessage_User_GetBlockedUsers",
        "ovrMessage_User_GetNextBlockedUserArrayPage",
    ],
    "ovr_Message_GetChallenge": [
        "ovrMessage_Challenges_Create",
        "ovrMessage_Challenges_DeclineInvite",
        "ovrMessage_Challenges_Get",
        "ovrMessage_Challenges_Join",
        "ovrMessage_Challenges_Leave",
        "ovrMessage_Challenges_UpdateInfo",
    ],
    "ovr_Message_GetChallengeArray": [
        "ovrMessage_Challenges_GetList",
        "ovrMessage_Challenges_GetNextChallenges",
        "ovrMessage_Challenges_GetPreviousChallenges",
    ],
    "ovr_Message_GetChallengeEntryArray": [
        "ovrMessage_Challenges_GetEntries",
        "ovrMessage_Challenges_GetEntriesAfterRank",
        "ovrMessage_Challenges_GetEntriesByIds",
        "ovrMessage_Challenges_GetNextEntries",
        "ovrMessage_Challenges_GetPreviousEntries",
    ],
    "ovr_Message_GetCowatchViewerArray": [
        "ovrMessage_Cowatching_GetNextCowatchViewerArrayPage",
        "ovrMessage_Cowatching_GetViewersData",
    ],
    "ovr_Message_GetCowatchViewerUpdate": [
        "ovrMessage_Notification_Cowatching_ViewersDataChanged",
    ],
    "ovr_Message_GetCowatchingState": [
        "ovrMessage_Cowatching_IsInSession",
        "ovrMessage_Notification_Cowatching_InSessionChanged",
    ],
    "ovr_Message_GetDestinationArray": [
        "ovrMessage_RichPresence_GetDestinations",
        "ovrMessage_RichPresence_GetNextDestinationArrayPage",
    ],
    # These messages result in a "success" boolean, which is `ovr_Message_IsError()` negated.
    "!ovr_Message_IsError": [
        "ovrMessage_AbuseReport_ReportRequestHandled",
        "ovrMessage_ApplicationLifecycle_RegisterSessionKey",
        "ovrMessage_Challenges_Delete",
        "ovrMessage_Cowatching_JoinSession",
        "ovrMessage_Cowatching_LaunchInviteDialog",
        "ovrMessage_Cowatching_LeaveSession",
        "ovrMessage_Cowatching_RequestToPresent",
        "ovrMessage_Cowatching_ResignFromPresenting",
        "ovrMessage_Cowatching_SetPresenterData",
        "ovrMessage_Cowatching_SetViewerData",
        "ovrMessage_Entitlement_GetIsViewerEntitled",
        "ovrMessage_GroupPresence_Clear",
        "ovrMessage_GroupPresence_LaunchMultiplayerErrorDialog",
        "ovrMessage_GroupPresence_LaunchRosterPanel",
        "ovrMessage_GroupPresence_Set",
        "ovrMessage_GroupPresence_SetDeeplinkMessageOverride",
        "ovrMessage_GroupPresence_SetDestination",
        "ovrMessage_GroupPresence_SetIsJoinable",
        "ovrMessage_GroupPresence_SetLobbySession",
        "ovrMessage_GroupPresence_SetMatchSession",
        "ovrMessage_IAP_ConsumePurchase",
        "ovrMessage_Notification_MarkAsRead",
        "ovrMessage_RichPresence_Clear",
        "ovrMessage_RichPresence_Set",
        "ovrMessage_UserAgeCategory_Report",
    ],
    "ovr_Message_GetGroupPresenceJoinIntent": [
        "ovrMessage_Notification_GroupPresence_JoinIntentReceived",
    ],
    "ovr_Message_GetGroupPresenceLeaveIntent": [
        "ovrMessage_Notification_GroupPresence_LeaveIntentReceived",
    ],
    "ovr_Message_GetInvitePanelResultInfo": [
        "ovrMessage_GroupPresence_LaunchInvitePanel",
    ],
    "ovr_Message_GetLaunchBlockFlowResult": [
        "ovrMessage_User_LaunchBlockFlow",
    ],
    "ovr_Message_GetLaunchFriendRequestFlowResult": [
        "ovrMessage_User_LaunchFriendRequestFlow",
    ],
    "ovr_Message_GetLaunchInvitePanelFlowResult": [
        "ovrMessage_Notification_GroupPresence_InvitationsSent",
    ],
    "ovr_Message_GetLaunchUnblockFlowResult": [
        "ovrMessage_User_LaunchUnblockFlow",
    ],
    "ovr_Message_GetLeaderboardArray": [
        "ovrMessage_Leaderboard_Get",
        "ovrMessage_Leaderboard_GetNextLeaderboardArrayPage",
    ],
    "ovr_Message_GetLeaderboardEntryArray": [
        "ovrMessage_Leaderboard_GetEntries",
        "ovrMessage_Leaderboard_GetEntriesAfterRank",
        "ovrMessage_Leaderboard_GetEntriesByIds",
        "ovrMessage_Leaderboard_GetNextEntries",
        "ovrMessage_Leaderboard_GetPreviousEntries",
    ],
    "ovr_Message_GetLeaderboardUpdateStatus": [
        "ovrMessage_Leaderboard_WriteEntry",
        "ovrMessage_Leaderboard_WriteEntryWithSupplementaryMetric",
    ],
    "ovr_Message_GetLivestreamingStatus": [
        "ovrMessage_Notification_Livestreaming_StatusChange",
    ],
    "ovr_Message_GetMicrophoneAvailabilityState": [
        "ovrMessage_Voip_GetMicrophoneAvailability",
    ],
    "ovr_Message_GetNetSyncConnection": [
        "ovrMessage_Notification_NetSync_ConnectionStatusChanged",
    ],
    "ovr_Message_GetNetSyncSessionsChangedNotification": [
        "ovrMessage_Notification_NetSync_SessionsChanged",
    ],
    "ovr_Message_GetOrgScopedID": [
        "ovrMessage_User_GetOrgScopedID",
    ],
    "ovr_Message_GetParty": [
        "ovrMessage_Party_GetCurrent",
    ],
    "ovr_Message_GetPartyUpdateNotification": [
        "ovrMessage_Notification_Party_PartyUpdate",
    ],
    "ovr_Message_GetPidArray": [
        "ovrMessage_ApplicationLifecycle_GetRegisteredPIDs",
    ],
    "ovr_Message_GetProductArray": [
        "ovrMessage_IAP_GetNextProductArrayPage",
        "ovrMessage_IAP_GetProductsBySKU",
    ],
    "ovr_Message_GetPurchase": [
        "ovrMessage_IAP_LaunchCheckoutFlow",
    ],
    "ovr_Message_GetPurchaseArray": [
        "ovrMessage_IAP_GetNextPurchaseArrayPage",
        "ovrMessage_IAP_GetViewerPurchases",
        "ovrMessage_IAP_GetViewerPurchasesDurableCache",
    ],
    "ovr_Message_GetRejoinDialogResult": [
        "ovrMessage_GroupPresence_LaunchRejoinDialog",
    ],
    "ovr_Message_GetSdkAccountArray": [
        "ovrMessage_User_GetSdkAccounts",
    ],
    "ovr_Message_GetSendInvitesResult": [
        "ovrMessage_GroupPresence_SendInvites",
    ],
    "ovr_Message_GetShareMediaResult": [
        "ovrMessage_Media_ShareToFacebook",
    ],
    "ovr_Message_GetString": [
        "ovrMessage_ApplicationLifecycle_GetSessionKey",
        "ovrMessage_Application_LaunchOtherApp",
        "ovrMessage_Cowatching_GetPresenterData",
        "ovrMessage_DeviceApplicationIntegrity_GetIntegrityToken",
        "ovrMessage_Notification_AbuseReport_ReportButtonPressed",
        "ovrMessage_Notification_ApplicationLifecycle_LaunchIntentChanged",
        "ovrMessage_Notification_Cowatching_ApiNotReady",
        "ovrMessage_Notification_Cowatching_ApiReady",
        "ovrMessage_Notification_Cowatching_Initialized",
        "ovrMessage_Notification_Cowatching_PresenterDataChanged",
        "ovrMessage_Notification_Cowatching_SessionStarted",
        "ovrMessage_Notification_Cowatching_SessionStopped",
        "ovrMessage_Notification_Voip_MicrophoneAvailabilityStateUpdate",
        "ovrMessage_Notification_Vrcamera_GetDataChannelMessageUpdate",
        "ovrMessage_Notification_Vrcamera_GetSurfaceUpdate",
        "ovrMessage_User_GetAccessToken",
    ],
    "ovr_Message_GetSystemVoipState": [
        "ovrMessage_Voip_SetSystemVoipSuppressed",
        "ovrMessage_Notification_Voip_SystemVoipState",
    ],
    "ovr_Message_GetUser": [
        "ovrMessage_User_Get",
        "ovrMessage_User_GetLoggedInUser",
    ],
    "ovr_Message_GetUserAccountAgeCategory": [
        "ovrMessage_UserAgeCategory_Get",
    ],
    "ovr_Message_GetUserArray": [
        "ovrMessage_GroupPresence_GetInvitableUsers",
        "ovrMessage_User_GetLoggedInUserFriends",
        "ovrMessage_User_GetNextUserArrayPage",
    ],
    "ovr_Message_GetUserCapabilityArray": [
        "ovrMessage_User_GetNextUserCapabilityArrayPage",
    ],
    "ovr_Message_GetUserProof": [
        "ovrMessage_User_GetUserProof",
    ],
    "ovr_Message_GetHttpTransferUpdate": [
        "ovrMessage_Notification_HTTP_Transfer",
    ],
    "ovr_Message_GetPlatformInitialize": [
        "ovrMessage_PlatformInitializeWithAccessToken",
        "ovrMessage_Platform_InitializeStandaloneOculus",
        "ovrMessage_PlatformInitializeAndroidAsynchronous",
        "ovrMessage_PlatformInitializeWindowsAsynchronous",
    ]
}

FUNCTION_RENAMES = {
    'MetaPlatformSDK': {
        # For consistency with the async versions.
        'get_logged_in_user_id': 'user_get_logged_in_user_id',
        'get_logged_in_user_locale': 'user_get_logged_in_user_locale',

        # Not caught by current filters to remove 'ovr_'.
        'ovr_message_type_is_notification': 'message_type_is_notification',
    },
    'MetaPlatformSDK_AssetFileDownloadUpdate': {
        'get_bytes_total_long': 'get_bytes_total',
        'get_bytes_transferred_long': 'get_bytes_transferred',
    },
}

# Enums using their Godot names (not the OVR ones) to keep, even though they are unused.
KEEP_UNUSED_ENUMS = [
    'MessageType',
]


def parse_headers(path):
    parsed = {}
    for header_path in glob.glob(os.path.join(path, '*.h')):
        header = os.path.split(header_path)[-1]
        if header in EXCLUDE_OVR_HEADERS:
            continue

        with open(header_path, 'rt') as fd:
            parsed[header] = HeaderParser().parse(fd.readlines())

    return parsed


class HeaderParser:
    INCLUDE_RE = re.compile(r'^#include [<"]([^>"]*)[<"]')
    DOC_COMMENT_RE = re.compile(r'\s*///(.*)')
    HANDLE_RE = re.compile(r'^typedef struct [a-zA-Z0-9]*\s*\*\s*([^;]*);')
    ENUM_START_RE = re.compile(r'^typedef enum')
    ENUM_VALUE_RE = re.compile(r'\s*([a-zA-Z0-9_]*)(?:\s*=\s*)?(.*),')
    ENUM_STOP_RE = re.compile(r'^} ([^;]*);')
    FUNCTION_RE = re.compile(r'(OVRPL?_PUBLIC_FUNCTION)\(([^\)]*)\) ([^\(]*)\(([^\)]*)\)')

    def __init__(self):
        self._docs = []
        self._in_enum = False
        self._enum_values = []
        self._enum_docs = []

    def parse(self, lines):
        parsed = {
            'dependencies': [],
        }

        for line in lines:
            # Skip these kinds of lines.
            if line == '\n':
                self._docs.clear()
                continue
            if line.startswith('// '):
                self._docs.clear()
                continue
            if line.startswith('#ifndef'):
                self._docs.clear()
                continue
            if line.startswith('#define'):
                self._docs.clear()
                continue

            # Documentation comments.
            m = HeaderParser.DOC_COMMENT_RE.match(line)
            if m:
                self._docs.append(m[1].strip())
                continue

            # Enumerations.
            if self._in_enum:
                m = HeaderParser.ENUM_STOP_RE.match(line)
                if m:
                    if 'enums' not in parsed:
                        parsed['enums'] = {}

                    value = {
                        'values': self._enum_values
                    }
                    self._enum_values = []

                    if len(self._enum_docs) > 0:
                        value['docs'] = self._enum_docs
                        self._enum_docs = []

                    parsed['enums'][m[1]] = value
                    self._in_enum = False
                    continue

                m = HeaderParser.ENUM_VALUE_RE.match(line)
                if m:
                    value = {
                        'name': m[1]
                    }
                    if len(m[2]) > 0:
                        value['value'] = m[2]

                    if len(self._docs) > 0:
                        value['docs'] = self._docs
                        self._docs = []

                    self._enum_values.append(value)
                    continue

                continue
            else:
                m = HeaderParser.ENUM_START_RE.match(line)
                if m:
                    self._in_enum = True
                    self._enum_docs = self._docs
                    self._docs = []
                    continue

            # Handles.
            m = HeaderParser.HANDLE_RE.match(line)
            if m:
                if 'handles' not in parsed:
                    parsed['handles'] = [];
                parsed['handles'].append(m[1])
                continue

            # Functions.
            m = HeaderParser.FUNCTION_RE.match(line)
            if m:
                if 'functions' not in parsed:
                    parsed['functions'] = {}

                value = {
                    'name': m[3].strip(),
                    'type': m[1],
                    'return': m[2].replace(' *', '*'),
                }

                arguments = []
                for argument in [a.strip() for a in str(m[4]).split(',') if a != '']:
                    parts = argument.split(' ')
                    if len(parts) > 1:
                        name = parts[-1]
                        while name.startswith('*'):
                            name = name[1:]

                        type = argument[:-len(name)].strip()
                        type = type.replace(' *', '*')

                        arguments.append({
                            'type': type,
                            'name': name,
                        })
                    else:
                        arguments.append({
                            'type': argument,
                        })
                value['arguments'] = arguments

                if len(self._docs) > 0:
                    value['docs'] = self._docs
                    self._docs = []

                parsed['functions'][value['name']] = value
                continue


            # Include dependencies.
            m = HeaderParser.INCLUDE_RE.match(line)
            if m:
                include = m[1]
                if include.startswith('OVR_'):
                    parsed['dependencies'].append(include)
                continue

            self._docs.clear()

        return parsed


def camel_to_snake_case(name):
    # Fix specifically for "ID"
    name = name.replace("PID", "Pid")
    name = name.replace("ID", "Id")

    name = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    name = re.sub('([a-z0-9])([A-Z])', r'\1_\2', name)
    name = re.sub('__', '_', name)
    return name.lower()


def rename_function(class_name, function_name, ovr_function, is_array=False):
    if ovr_function['return'] == 'ovrRequest':
        function_name += '_async'

    if is_array and function_name == 'get_size':
        function_name = 'size'
    elif class_name in FUNCTION_RENAMES and function_name in FUNCTION_RENAMES[class_name]:
        function_name = FUNCTION_RENAMES[class_name][function_name]

    return function_name


def make_codegen_plan(headers, verbose=False):
    plan = {
        'enums': {},
        'classes': {
            'MetaPlatformSDK': {
                'name': 'MetaPlatformSDK',
                'type': 'singleton',
                'ovr_handle': None,
                'ovr_headers': ['OVR_Platform.h'],
                'functions': {},
                'function_map': {},
            }
        },
        'map': {},
    }

    # Make a first pass, trying to get all the parts in the right place.
    for header_name, header in headers.items():
        # Remove the leading 'OVR_' and trailing '.h'
        base_name = header_name[4:-2]

        if 'enums' in header:
            for orig_enum_name, orig_enum in header['enums'].items():
                enum = {
                    'name': base_name,
                    'values': [],
                    'value_map': {},
                    'ovr_header': header_name,
                    'ovr_name': orig_enum_name,
                }

                if 'docs' in orig_enum:
                    enum['docs'] = orig_enum['docs']

                for orig_value in orig_enum['values']:
                    orig_value_name = orig_value['name']
                    value = orig_value.copy()

                    if value['name'].startswith('ovr'):
                        value['name'] = value['name'][3:]
                    value['name'] = camel_to_snake_case(value['name']).upper()

                    enum['values'].append(value)
                    enum['value_map'][orig_value_name] = value['name']

                plan['enums'][enum['name']] = enum
                plan['map'][orig_enum_name] = enum['name']

                # @todo Check that the only functions are the to/from string.

        if 'functions' in header:
            functions = header['functions'].copy()
            functions = {k: v for k, v in functions.items() if k not in EXCLUDE_OVR_FUNCTIONS}

            # Remove the to/from string functions for enums.
            if 'enums' in header:
                for orig_enum_name, orig_enum in header['enums'].items():
                    to_string_name = orig_enum_name + '_ToString'
                    if to_string_name in functions:
                        del functions[to_string_name]

                    from_string_name = orig_enum_name + '_FromString'
                    if from_string_name in functions:
                        del functions[from_string_name]

            if len(functions) > 0:
                # These are functions for a model or result class.
                if 'handles' in header:
                    for handle_name in header['handles']:
                        handle_base_name = handle_name
                        if handle_base_name.startswith('ovr'):
                            handle_base_name = handle_base_name[3:]
                        if handle_base_name.endswith('Handle'):
                            handle_base_name = handle_base_name[:-6]

                        class_name = 'MetaPlatformSDK_' + handle_base_name
                        class_def = {
                            'name': class_name,
                            'ovr_handle': handle_name,
                            'ovr_headers': [header_name],
                            'functions': {},
                            'function_map': {},
                        }

                        create_func_name = 'ovr_' + handle_base_name + '_Create'
                        destroy_func_name = 'ovr_' + handle_base_name + '_Destroy'
                        get_size_func_name = 'ovr_' + handle_base_name + '_GetSize'
                        get_element_func_name = 'ovr_' + handle_base_name + '_GetElement'
                        if create_func_name in functions and destroy_func_name in functions:
                            class_def['type'] = 'model'

                            class_def['create_func'] = functions[create_func_name]
                            del functions[create_func_name]
                            class_def['destroy_func'] = functions[destroy_func_name]
                            del functions[destroy_func_name]

                        else:
                            class_def['type'] = 'result'
                            class_def['is_array'] = get_size_func_name in functions and get_element_func_name in functions

                            free_func_name = 'ovr_' + handle_base_name + '_Free'
                            if free_func_name in functions:
                                class_def['free_func'] = functions[free_func_name]
                                del functions[free_func_name]

                            # Because we're both excluding this function and it's not in the same header file,
                            # we need to set this one manually.
                            if class_name == 'MetaPlatformSDK_Message':
                                class_def['free_func'] = {
                                    'name': 'ovr_FreeMessage',
                                }

                        for function_name, function in functions.items():
                            new_function_name = function_name
                            if new_function_name.startswith('ovr_' + handle_base_name):
                                new_function_name = new_function_name[len(handle_base_name) + 5:]
                            new_function_name = camel_to_snake_case(new_function_name)

                            new_function_name = rename_function(class_name, new_function_name, function, 'is_array' in class_def and class_def['is_array'])

                            new_function = {
                                'name': new_function_name,
                                'return': '',
                                'arguments': [],
                                'ovr_function': function,
                            }
                            if function_name in EXCLUDE_OVR_FUNCTION_SOURCE:
                                new_function['exclude_source'] = True

                            class_def['functions'][new_function_name] = new_function
                            class_def['function_map'][function_name] = new_function_name

                        plan['classes'][class_name] = class_def
                        plan['map'][handle_name] = class_name

                # These can be promoted to the singleton.
                else:
                    for function_name, function in functions.items():
                        new_function_name = function_name
                        if new_function_name.startswith('ovr_'):
                            new_function_name = new_function_name[4:]
                        new_function_name = camel_to_snake_case(new_function_name)

                        new_function_name = rename_function('MetaPlatformSDK', new_function_name, function)

                        plan['classes']['MetaPlatformSDK']['functions'][new_function_name] = {
                            'name': new_function_name,
                            'return': '',
                            'arguments': [],
                            'ovr_function': function,
                        }
                        if header_name not in plan['classes']['MetaPlatformSDK']['ovr_headers']:
                            plan['classes']['MetaPlatformSDK']['ovr_headers'].append(header_name)

                        plan['classes']['MetaPlatformSDK']['function_map'][function_name] = new_function_name

    # Make a second pass over functions to convert their arguments and return values.
    for class_name, class_def in plan['classes'].items():
        class_def['dependencies'] = {
            'classes': [],
            'enums': [],
        }

        for function_name, function in class_def['functions'].items():
            ovr_function = function['ovr_function']

            function['return'] = convert_argument_type(ovr_function['return'], plan, class_def['dependencies'], True)
            function['arguments'] = []

            ovr_argument_count = len(ovr_function['arguments'])
            ovr_argument_index = 0
            while ovr_argument_index < ovr_argument_count:
                ovr_argument = ovr_function['arguments'][ovr_argument_index]

                # See if the argument seems to represent an array.
                looks_like_array = False
                if ovr_argument['type'].endswith('*') and ovr_argument_index + 1 < ovr_argument_count:
                    next_ovr_argument = ovr_function['arguments'][ovr_argument_index + 1]
                    if next_ovr_argument['type'] == 'unsigned int':
                        looks_like_array = True
                    elif next_ovr_argument['type'] in ['int'] and next_ovr_argument['name'] in ['count']:
                        looks_like_array = True

                if looks_like_array:
                    if ovr_argument['type'] == 'const void*':
                        array_type = 'const PackedByteArray &'
                    elif ovr_argument['type'] == 'const char**':
                        array_type = 'const PackedStringArray &'
                    else:
                        individual_type = convert_argument_type(ovr_argument['type'][:-1], plan, class_def['dependencies'], True)
                        if individual_type in ['uint64_t', 'int64_t']:
                            array_type = 'const PackedInt64Array &'
                        else:
                            array_type = f'TypedArray<{individual_type}>'

                    function['arguments'].append({
                        'type': array_type,
                        'name': make_argument_name(ovr_argument),
                        'is_array': True,
                    })

                    ovr_argument_index += 2
                else:
                    if ovr_argument_index == 0 and class_def['ovr_handle'] != None and ovr_argument['type'].endswith(class_def['ovr_handle']):
                        # Skip the "self" argument.
                        pass
                    else:
                        function['arguments'].append({
                            'type': convert_argument_type(ovr_argument['type'], plan, class_def['dependencies']),
                            'name': make_argument_name(ovr_argument),
                        })
                    ovr_argument_index += 1

        # Setup reverse dependency references.
        for class_dep_name in class_def['dependencies']['classes']:
            class_dep = plan['classes'][class_dep_name]
            if 'used_by' not in class_dep:
                class_dep['used_by'] = []
            if class_name not in class_dep['used_by']:
                class_dep['used_by'].append(class_name)
        for enum_dep_name in class_def['dependencies']['enums']:
            enum_dep = plan['enums'][enum_dep_name]
            if 'used_by' not in enum_dep:
                enum_dep['used_by'] = []
            if class_name not in enum_dep['used_by']:
                enum_dep['used_by'].append(class_name)

    # Distribute the enums among the classes.
    unused_enums = []
    for enum_name, enum in plan['enums'].items():
        if 'used_by' not in enum and enum_name not in KEEP_UNUSED_ENUMS:
            unused_enums.append(enum_name)
            enum_class = None

        #elif 'used_by' in enum and len(enum['used_by']) == 1:
        #    enum_class = plan['classes'][enum['used_by'][0]]
        #else:
        #    enum_class = plan['classes']['MetaPlatformSDK']

        else:
            # For now, let's put all the enums on the singleton.
            enum_class = plan['classes']['MetaPlatformSDK']

        if enum_class != None:
            enum['parent_class'] = enum_class['name']
            if 'local_enums' not in enum_class:
                enum_class['local_enums'] = []
            enum_class['local_enums'].append(enum_name)

    for enum_name in unused_enums:
        if verbose:
            print("Discarding unused enum:", enum_name)
        del plan['enums'][enum_name]

    return plan


def convert_argument_type(ovr_type, plan, dependency_list, is_return=False):
    if ovr_type == 'void' and is_return:
        return 'void'
    elif ovr_type in ['bool', 'int32_t', 'uint32_t', 'int64_t', 'uint64_t', 'float']:
        return ovr_type
    elif ovr_type == 'int':
        return 'int32_t'
    elif ovr_type == 'unsigned int':
        return 'uint32_t'
    elif ovr_type == 'long long':
        return 'int64_t'
    elif ovr_type in ['size_t', 'unsigned long long']:
        return 'uint64_t'
    elif ovr_type == 'const char*':
        if is_return:
            return 'String'
        else:
            return 'const String &'
    elif ovr_type == 'const void*':
        if is_return:
            return 'PackedByteArray'
        else:
            return 'const PackedByteArray &'
    elif ovr_type == 'ovrRequest':
        if is_return:
            return 'Ref<MetaPlatformSDK_Request>'
        else:
            return 'const Ref<MetaPlatformSDK_Request> &'
    elif ovr_type == 'ovrID':
        return 'uint64_t'

    if ovr_type.startswith('const '):
        ovr_type = ovr_type[6:]

    # Map to class or enum.
    mapped = plan['map'][ovr_type]
    if mapped in plan['classes']:
        if mapped not in dependency_list['classes']:
            dependency_list['classes'].append(mapped)
        if is_return:
            return f'Ref<{mapped}>'
        else:
            return f'const Ref<{mapped}> &'
    if mapped in plan['enums']:
        if mapped not in dependency_list['enums']:
            dependency_list['enums'].append(mapped)
        # For now, all enums are on the singleton.
        return 'MetaPlatformSDK::' + mapped

    raise Exception("Unable to map OVR type: %s" % ovr_type)


def make_argument_name(argument):
    if 'name' in argument:
        return 'p_' + camel_to_snake_case(argument['name'])
    return 'p_' + camel_to_snake_case(argument['type'])


def generate_code(plan, output_path):
    header_path = os.path.join(output_path, 'include', 'platform_sdk')
    source_path = os.path.join(output_path, 'src')

    os.makedirs(header_path, exist_ok=True)
    os.makedirs(source_path, exist_ok=True)

    for class_name, class_def in plan['classes'].items():
        file_base = camel_to_snake_case(class_name)
        with open(os.path.join(header_path, file_base + '.h'), 'wt') as fd:
            lines = generate_header(class_name, class_def, plan)
            fd.write('\n'.join(lines))

        with open(os.path.join(source_path, file_base + '.cpp'), 'wt') as fd:
            lines = generate_source(class_name, class_def, plan)
            fd.write('\n'.join(lines))


def make_function_decl(func_name, func_def, class_name=None):
    func_decl = func_def['return']
    func_decl += ' '
    if class_name:
        func_decl += class_name + '::'
    func_decl += func_name
    func_decl += '('

    arg_decls = []
    for argument in func_def['arguments']:
        arg_string = argument['type']
        if not arg_string.endswith('&'):
            arg_string += ' '
        arg_string += argument['name']
        arg_decls.append(arg_string)
    func_decl += ', '.join(arg_decls)
    func_decl += ')'
    if func_name.startswith('get_') or func_name.startswith('is_') or func_name in ['size', 'has_next_page']:
        func_decl += ' const';

    return func_decl


def make_null_value(type, plan):
    if type == 'bool':
        return 'false'
    elif type == 'String' or type.startswith('Ref<') or type.startswith('Packed') or type.startswith('TypedArray<'):
        return type + '()'
    elif type.startswith('MetaPlatformSDK::'):
        # The first value in the enum.
        enum_name = type.split('::')[1]
        return 'MetaPlatformSDK::' + plan['enums'][enum_name]['values'][0]['name']

    # Otherwise, it's probably a number.
    return '0'


def convert_argument_value_to_ovr(name, ovr_type, godot_type, plan):
    if godot_type == 'const String &':
        return f'{name}.utf8().ptr()'
    elif godot_type == 'const PackedStringArray &':
        return f'({ovr_type})CharStringList({name}).pointers.ptr(), {name}.size()'
    elif godot_type.startswith('const Packed'):
        return f'({ovr_type}){name}.ptr(), {name}.size()'
    elif godot_type.startswith('MetaPlatformSDK::'):
        return f'({ovr_type}){name}'
    elif godot_type.startswith('const Ref<'):
        return f'{name}.is_valid() ? {name}->_get_ovr_handle() : nullptr'

    # Default to just using it as-is - the compiler will tell us if this is OK.
    return name


def convert_return_value_from_ovr(name, ovr_type, godot_type, plan):
    if godot_type == 'String':
        return f'String({name})'
    elif godot_type == 'Ref<MetaPlatformSDK_Request>':
        return f'MetaPlatformSDK::get_singleton()->_create_request({name})'
    elif godot_type.startswith('MetaPlatformSDK::'):
        return f'({godot_type}){name}'
    elif godot_type.startswith('Ref<'):
        m = re.match(r'Ref<([^>]*)>', godot_type)
        return f'{m[1]}::_create_with_ovr_handle({name})'

    # Default to just using it as-is - the compiler will tell us if this is OK.
    return name


def make_property_info_from_return_type(name, godot_type, plan):
    if godot_type == 'String':
        return f'Variant::STRING, "{name}"'
    elif godot_type == 'bool':
        return f'Variant::BOOL, "{name}"'
    elif godot_type in ['int', 'size_t', 'int64_t', 'uint64_t', 'int32_t', 'uint32_t']:
        return f'Variant::INT, "{name}"'
    elif godot_type in ['float', 'double']:
        return f'Variant::FLOAT, "{name}"'
    elif godot_type.startswith('Packed'):
        return f'Variant::{camel_to_snake_case(godot_type).upper()}, "{name}"'
    elif godot_type.startswith('MetaPlatformSDK::'):
        return f'Variant::INT, "{name}"'
    elif godot_type.startswith('Ref<'):
        m = re.match(r'Ref<([^>]*)>', godot_type)
        return f'Variant::OBJECT, "{name}", PROPERTY_HINT_RESOURCE_TYPE, "{m[1]}"'

    raise Exception("Cannot make property out of return value %s" % godot_type)


def generate_header(class_name, class_def, plan):
    lines = []

    lines.append('// Copyright (c) 2024-present Meta Platforms, Inc. and affiliates. All rights reserved.')

    lines.append('')
    lines.append('// Automatically generated by generate_platform_sdk_bindings.py - DO NOT EDIT!')
    lines.append('')
    lines.append('#pragma once')
    lines.append('')

    if class_name != 'MetaPlatformSDK':
        lines.append('#include "platform_sdk/meta_platform_sdk.h"')
        lines.append('')
        lines.append('#ifdef ANDROID_ENABLED')
        for ovr_header in class_def['ovr_headers']:
            lines.append(f'#include <{ovr_header}>')
        lines.append('#endif // ANDROID_ENABLED')
    else:
        lines.append('#include <godot_cpp/classes/ref.hpp>')
        lines.append('#include <godot_cpp/templates/hash_map.hpp>')
        lines.append('')
        lines.append('#ifdef ANDROID_ENABLED')
        lines.append('#include <OVR_Types.h>')
        lines.append('#endif // ANDROID_ENABLED')
        lines.append('')
        lines.append('#include "platform_sdk/meta_platform_sdk_request.h"')
    lines.append('')

    # Dependencies.
    if len(class_def['dependencies']['classes']):
        for class_dep_name in class_def['dependencies']['classes']:
            if class_name != 'MetaPlatformSDK':
                lines.append(f'#include "platform_sdk/{camel_to_snake_case(class_dep_name)}.h"')
            else:
                lines.append(f'class {class_dep_name};')
        if class_name == 'MetaPlatformSDK':
            lines.append(f'class MetaPlatformSDK_Message;')
        lines.append('')

    lines.append('using namespace godot;')
    lines.append('')

    if class_def['type'] == 'singleton':
        parent_class_name = 'Object'
    else:
        parent_class_name = 'RefCounted'

    lines.append(f'class {class_name} : public {parent_class_name} {{')
    lines.append(f'\tGDCLASS({class_name}, {parent_class_name});')
    lines.append('')

    if class_def['type'] == 'singleton':
        lines.append(f'\tstatic {class_name} *singleton;')
        lines.append('')
        lines.append('#ifdef ANDROID_ENABLED')
        lines.append('\tbool _platform_initialized = false;')
        lines.append('\tHashMap<ovrRequest, Ref<MetaPlatformSDK_Request>> requests;')
        lines.append('#endif // ANDROID_ENABLED')
        lines.append('')
    else:
        lines.append('#ifdef ANDROID_ENABLED')
        lines.append(f'\t{class_def["ovr_handle"]} handle = nullptr;')
        lines.append('#endif // ANDROID_ENABLED')
        lines.append('')
    if class_name == 'MetaPlatformSDK_Message':
        lines.append('\tMetaPlatformSDK::MessageType type = MetaPlatformSDK::MESSAGE_UNKNOWN;')
        lines.append('\tmutable Variant data;')
        lines.append('')

    lines.append('protected:')
    lines.append('\tstatic void _bind_methods();')
    lines.append('\tString _to_string() const;')
    lines.append('')
    lines.append('public:')

    if class_def['type'] == 'singleton':
        lines.append(f'\tstatic {class_name} *get_singleton();')
        lines.append('')

    # Enums.
    if 'local_enums' in class_def:
        for enum_name in class_def['local_enums']:
            enum = plan['enums'][enum_name]
            lines.append(f'\tenum {enum_name} {{')

            for value in enum['values']:
                value_str = '\t\t' + value['name']
                if 'value' in value:
                    value_str += ' = ' + value['value']
                value_str += ','

                lines.append(value_str)

            lines.append('\t};')
            lines.append('')

    # Custom functions.
    if class_name == 'MetaPlatformSDK':
        lines.append(f'\tstatic void _register_generated_classes();')
        lines.append('')
        lines.append('#ifdef ANDROID_ENABLED')
        lines.append(f'\tvoid _initialize_platform();')
        lines.append(f'\tvoid _initialize_platform_async(const Ref<MetaPlatformSDK_Message> &p_message);')
        lines.append(f'\tRef<MetaPlatformSDK_Request> _create_request(ovrRequest p_request);')
        lines.append(f'\tvoid _process_messages();')
        lines.append('#endif // ANDROID_ENABLED')
        lines.append('')
        lines.append(f'\tPlatformInitializeResult initialize_platform(const String &p_app_id, const Dictionary &p_options);')
        lines.append(f'\tRef<MetaPlatformSDK_Request> initialize_platform_async(const String &p_app_id);')
    else:
        lines.append('#ifdef ANDROID_ENABLED')
        lines.append(f'\tstatic Ref<{class_name}> _create_with_ovr_handle({class_def["ovr_handle"]} p_handle);')
        lines.append(f'\tinline {class_def["ovr_handle"]} _get_ovr_handle() {{ return handle; }}')
        lines.append('#endif // ANDROID_ENABLED')
        lines.append('')
    if class_name == 'MetaPlatformSDK_Message':
        lines.append('\tinline MetaPlatformSDK::MessageType get_type() const { return type; }')
        lines.append('\tVariant get_data() const;')
        lines.append('\tbool is_success() const { return !is_error(); }')
        lines.append('\tbool is_notification() const;')
        lines.append('\tuint64_t get_request_id() const;')
        lines.append('\tString get_type_as_string() const;')
        lines.append('')
    if class_name == 'MetaPlatformSDK_HttpTransferUpdate':
        lines.append('\tuint64_t get_id() const;')
        lines.append('')
    if class_name == 'MetaPlatformSDK_ChallengeEntry':
        lines.append('\tPackedByteArray get_extra_data() const;')
        lines.append('')
    if class_name == 'MetaPlatformSDK_LeaderboardEntry':
        lines.append('\tPackedByteArray get_extra_data() const;')
        lines.append('')

    # Iterator functions for arrays.
    if class_def['type'] == 'result' and class_def['is_array']:
        lines.append('\tbool _iter_init(Array p_iter);')
        lines.append('\tbool _iter_next(Array p_iter);')
        lines.append('\tVariant _iter_get(uint64_t p_iter);')
        lines.append('')

    # Generated functions.
    for func_name, func_def in class_def['functions'].items():
        lines.append('\t' + make_function_decl(func_name, func_def) + ';')
    lines.append('')
    lines.append(f'\t{class_name}();')
    lines.append(f'\t~{class_name}();')
    lines.append('};')
    lines.append('')

    if 'local_enums' in class_def:
        for enum_name in class_def['local_enums']:
            lines.append(f'VARIANT_ENUM_CAST({class_name}::{enum_name});')
        lines.append('')

    return lines


def generate_source(class_name, class_def, plan):
    lines = []

    lines.append('// Copyright (c) 2024-present Meta Platforms, Inc. and affiliates. All rights reserved.')
    lines.append('')
    lines.append('// Automatically generated by generate_platform_sdk_bindings.py - DO NOT EDIT!')
    lines.append('')

    lines.append(f'#include "platform_sdk/{camel_to_snake_case(class_name)}.h"')
    lines.append('')
    lines.append('#include "util.h"')
    lines.append('')
    lines.append('#include <godot_cpp/core/class_db.hpp>')
    lines.append('#include <godot_cpp/variant/utility_functions.hpp>')

    if class_name == 'MetaPlatformSDK':
        lines.append('')
        lines.append('#ifdef ANDROID_ENABLED')
        for ovr_header in class_def['ovr_headers']:
            lines.append(f'#include <{ovr_header}>')
        lines.append('#endif // ANDROID_ENABLED')
        lines.append('')

        # Include all the other classes so we can register them.
        for other_class_name in plan['classes'].keys():
            if other_class_name == 'MetaPlatformSDK':
                continue
            lines.append(f'#include "platform_sdk/{camel_to_snake_case(other_class_name)}.h"')
    elif class_name == 'MetaPlatformSDK_Message':
        lines.append('#ifdef ANDROID_ENABLED')
        # Needed for ovr_FreeMessage().
        lines.append(f'#include <OVR_Platform.h>')
        lines.append('#endif // ANDROID_ENABLED')

    lines.append('')

    if class_def['type'] == 'singleton':
        lines.append(f'{class_name} *{class_name}::singleton = nullptr;')
        lines.append('')

        lines.append(f'{class_name} *{class_name}::get_singleton() {{')
        lines.append('\tif (singleton == nullptr) {')
        lines.append(f'\t\tsingleton = memnew({class_name}());')
        lines.append('\t}')
        lines.append('\treturn singleton;')
        lines.append('}')
        lines.append('')

    # Generate _bind_methods().
    lines.append(f'void {class_name}::_bind_methods() {{')
    for function_name, function in class_def['functions'].items():
        bind_string = f'\tClassDB::bind_method(D_METHOD("{function_name}"'
        if len(function['arguments']) > 0:
            friendly_arg_names = ['"' + a['name'][2:] + '"' for a in function['arguments']]
            bind_string += ", " + ", ".join(friendly_arg_names)
        bind_string += f'), &{class_name}::{function_name});'
        lines.append(bind_string)
        # Add properties on most result classes.
        if class_def['type'] == 'result' and class_name != 'MetaPlatformSDK_Message' and function_name.startswith('get_') and len(function['arguments']) == 0:
            prop_name = function_name[4:]
            prop_info = make_property_info_from_return_type(prop_name, function['return'], plan)
            lines.append(f'\tADD_PROPERTY(PropertyInfo({prop_info}), "", "{function_name}");')
    if 'local_enums' in class_def:
        for enum_name in class_def['local_enums']:
            enum = plan['enums'][enum_name]
            for enum_value in enum['values']:
                lines.append(f'\tBIND_ENUM_CONSTANT({enum_value["name"]});')
    if class_name == 'MetaPlatformSDK':
        lines.append('\tClassDB::bind_method(D_METHOD("initialize_platform", "app_id", "options"), &MetaPlatformSDK::initialize_platform, DEFVAL(Dictionary()));')
        lines.append('\tClassDB::bind_method(D_METHOD("initialize_platform_async", "app_id"), &MetaPlatformSDK::initialize_platform_async);')
        lines.append('\tADD_SIGNAL(MethodInfo("notification_received", PropertyInfo(Variant::OBJECT, "message", PROPERTY_HINT_RESOURCE_TYPE, "MetaPlatformSDK_Message")));')
    elif class_name == 'MetaPlatformSDK_Message':
        lines.append('\tClassDB::bind_method(D_METHOD("get_type"), &MetaPlatformSDK_Message::get_type);')
        lines.append('\tClassDB::bind_method(D_METHOD("get_data"), &MetaPlatformSDK_Message::get_data);')
        lines.append('\tADD_PROPERTY(PropertyInfo(Variant::INT, "type"), "", "get_type");')
        lines.append('\tADD_PROPERTY(PropertyInfo(Variant::NIL, "data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_NIL_IS_VARIANT), "", "get_data");')
        lines.append('\tADD_PROPERTY(PropertyInfo(Variant::NIL, "error", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_NIL_IS_VARIANT), "", "get_error");')
        lines.append('\tClassDB::bind_method(D_METHOD("is_success"), &MetaPlatformSDK_Message::is_success);')
        lines.append('\tClassDB::bind_method(D_METHOD("get_request_id"), &MetaPlatformSDK_Message::get_request_id);')
        lines.append('\tADD_PROPERTY(PropertyInfo(Variant::INT, "request_id"), "", "get_request_id");')
        lines.append('\tClassDB::bind_method(D_METHOD("get_type_as_string"), &MetaPlatformSDK_Message::get_type_as_string);')
    elif class_name == 'MetaPlatformSDK_HttpTransferUpdate':
        lines.append('\tClassDB::bind_method(D_METHOD("get_id"), &MetaPlatformSDK_HttpTransferUpdate::get_id);')
        lines.append('\tADD_PROPERTY(PropertyInfo(Variant::INT, "id"), "", "get_id");')
    if class_name == 'MetaPlatformSDK_ChallengeEntry':
        lines.append('\tClassDB::bind_method(D_METHOD("get_extra_data"), &MetaPlatformSDK_ChallengeEntry::get_extra_data);')
        lines.append('\tADD_PROPERTY(PropertyInfo(Variant::PACKED_BYTE_ARRAY, "extra_data"), "", "get_extra_data");')
    if class_name == 'MetaPlatformSDK_LeaderboardEntry':
        lines.append('\tClassDB::bind_method(D_METHOD("get_extra_data"), &MetaPlatformSDK_LeaderboardEntry::get_extra_data);')
        lines.append('\tADD_PROPERTY(PropertyInfo(Variant::PACKED_BYTE_ARRAY, "extra_data"), "", "get_extra_data");')
    if class_def['type'] == 'result' and class_def['is_array']:
        lines.append(f'\tClassDB::bind_method(D_METHOD("_iter_init"), &{class_name}::_iter_init);')
        lines.append(f'\tClassDB::bind_method(D_METHOD("_iter_next"), &{class_name}::_iter_next);')
        lines.append(f'\tClassDB::bind_method(D_METHOD("_iter_get"), &{class_name}::_iter_get);')
    lines.append('}')
    lines.append('')

    # Generate the _to_string() function.
    lines.append(f'String {class_name}::_to_string() const {{')
    if class_name == 'MetaPlatformSDK_Message':
        lines.append(f'\tString s = String("[{class_name}:") + itos(get_request_id());')
        lines.append('\ts += String(" type=") + get_type_as_string();')
        lines.append('\ts += String(" is_success=") + UtilityFunctions::str(is_success());')
        lines.append('\tif (is_error()) {')
        lines.append('\t\ts += String(" error=") + UtilityFunctions::str(get_error());')
        lines.append('\t} else {')
        lines.append('\t\ts += String(" data=") + UtilityFunctions::str(get_data());')
        lines.append('\t}')
        lines.append(f'\treturn s + String("]");')
    elif class_def['type'] == 'result':
        lines.append(f'\tString s = String("[{class_name}:") + itos(get_instance_id());')
        if class_def['is_array']:
            lines.append('\ts += String(" size=") + itos(size());')
            if "has_next_page" in class_def['functions']:
                lines.append('\ts += String(" has_next_page=") + UtilityFunctions::str(has_next_page());')
        else:
            for function_name, function in class_def['functions'].items():
                if class_def['type'] == 'result' and class_name != 'MetaPlatformSDK_Message' and function_name.startswith('get_') and len(function['arguments']) == 0:
                    prop_name = function_name[4:]
                    lines.append(f'\ts += " {prop_name}=" + UtilityFunctions::str({function_name}());')
        lines.append(f'\treturn s + String("]");')
    else:
        lines.append(f'\treturn String("[{class_name}:") + itos(get_instance_id()) + String("]");')
    lines.append('}')
    lines.append('')

    # Iterator functions for arrays.
    if class_def['type'] == 'result' and class_def['is_array']:
        lines.append(f'bool {class_name}::_iter_init(Array p_iter) {{')
        lines.append('\tp_iter[0] = 0;')
        lines.append('\treturn (uint64_t)p_iter[0] < size();')
        lines.append('}')
        lines.append('')
        lines.append(f'bool {class_name}::_iter_next(Array p_iter) {{')
        lines.append('\tp_iter[0] = (uint64_t)p_iter[0] + 1;')
        lines.append('\treturn (uint64_t)p_iter[0] < size();')
        lines.append('}')
        lines.append('')
        lines.append(f'Variant {class_name}::_iter_get(uint64_t p_iter) {{')
        lines.append('\treturn get_element(p_iter);')
        lines.append('}')
        lines.append('')

    # Generate all the OVR functions
    for function_name, function in class_def['functions'].items():
        if 'exclude_source' in function and function['exclude_source']:
            continue

        ovr_function = function['ovr_function']
        null_return_value = make_null_value(function['return'], plan)

        lines.append(make_function_decl(function_name, function, class_name) + ' {')
        lines.append('#ifdef ANDROID_ENABLED')

        # Check that we are initialized.
        if class_def['ovr_handle']:
            if function['return'] != 'void':
                lines.append(f'\tERR_FAIL_NULL_V(handle, {null_return_value});')
            else:
                lines.append(f'\tERR_FAIL_NULL(handle);')
        else:
            if function['return'] != 'void':
                lines.append(f'\tERR_FAIL_COND_V(!ovr_IsPlatformInitialized(), {null_return_value});')
            else:
                lines.append(f'\tERR_FAIL_COND(!ovr_IsPlatformInitialized());')
        if class_name == 'MetaPlatformSDK_Message':
            if ovr_function['name'] in OVR_FUNCTION_TO_MESSAGE_TYPES:
                valid_types = []
                for valid_type in OVR_FUNCTION_TO_MESSAGE_TYPES[ovr_function['name']]:
                    valid_types.append(f"type != MetaPlatformSDK::MessageType::{plan['enums']['MessageType']['value_map'][valid_type]}")
                valid_type_expression = ' && '.join(valid_types)
                if function['return'] != 'void':
                    lines.append(f'\tERR_FAIL_COND_V({valid_type_expression}, {null_return_value});')
                else:
                    lines.append(f'\tERR_FAIL_COND({valid_type_expression});')

                lines.append('');
                lines.append('\tif (data.get_type() != Variant::NIL) {')
                lines.append('\t\treturn data;')
                lines.append('\t}')
        lines.append('')

        # Call to the OVR function.
        func_call = "\t"
        if function['return'] != 'void':
            func_call += ovr_function['return'] + ' result = '
        func_call += ovr_function['name'] + '('
        func_call_args = []
        ovr_argument_index = 0
        if class_def['ovr_handle']:
            # Add the self argument.
            func_call_args.append('handle')
            ovr_argument_index += 1
        for argument in function['arguments']:
            func_call_args.append(convert_argument_value_to_ovr(argument['name'], ovr_function['arguments'][ovr_argument_index]['type'], argument['type'], plan))
            if 'is_array' in argument and argument['is_array']:
                ovr_argument_index += 2
            else:
                ovr_argument_index += 1
        func_call += ", ".join(func_call_args)
        func_call += ');'
        lines.append(func_call);

        if function['return'] != 'void':
            lines.append('')

            return_conversion = convert_return_value_from_ovr('result', ovr_function['return'], function['return'], plan)
            if class_name == 'MetaPlatformSDK_Message' and ovr_function['name'] in OVR_FUNCTION_TO_MESSAGE_TYPES:
                    lines.append(f"\t{function['return']} ret = {return_conversion};")
                    lines.append('\tdata = ret;')
                    lines.append('\treturn ret;')
            else:
                lines.append(f'\treturn {return_conversion};')

        # When we aren't on Android.
        if function['return'] != 'void':
            lines.append('#else')
            lines.append(f'\treturn {null_return_value};')

        lines.append('#endif // ANDROID_ENABLED')
        lines.append('}')
        lines.append('')

    # Constructor.
    lines.append(f'{class_name}::{class_name}() {{')
    if class_def['type'] == 'singleton':
        lines.append(f'\tERR_FAIL_COND_MSG(singleton != nullptr, "{class_name} singleton already exists.");')
        lines.append('');
        lines.append('\tsingleton = this;')
    elif class_def['type'] == 'model':
        lines.append('#ifdef ANDROID_ENABLED')
        lines.append(f"\thandle = {class_def['create_func']['name']}();")
        lines.append('#endif // ANDROID_ENABLED')
    lines.append('}')
    lines.append('')

    # Creation from handle.
    if class_def['type'] == 'result':
        lines.append('#ifdef ANDROID_ENABLED')
        lines.append(f'Ref<{class_name}> {class_name}::_create_with_ovr_handle({class_def["ovr_handle"]} p_handle) {{')
        lines.append(f'\tRef<{class_name}> inst;')
        lines.append('\tif (p_handle != nullptr) {')
        lines.append('\t\tinst.instantiate();')
        lines.append('\t\tinst->handle = p_handle;')
        if class_name == 'MetaPlatformSDK_Message':
            lines.append('\t\tinst->type = (MetaPlatformSDK::MessageType)ovr_Message_GetType(p_handle);')
        lines.append('\t}')
        lines.append('\treturn inst;')
        lines.append('}')
        lines.append('#endif // ANDROID_ENABLED')
        lines.append('')

    # Destructor.
    lines.append(f'{class_name}::~{class_name}() {{')
    if class_def['type'] == 'singleton':
        lines.append('\tsingleton = nullptr;')
    elif class_def['type'] == 'model':
        lines.append('#ifdef ANDROID_ENABLED')
        lines.append(f"\t{class_def['destroy_func']['name']}(handle);")
        lines.append('#endif // ANDROID_ENABLED')
    elif class_def['type'] == 'result' and 'free_func' in class_def:
        lines.append('#ifdef ANDROID_ENABLED')
        lines.append('\tif (handle) {')
        lines.append(f"\t\t{class_def['free_func']['name']}(handle);")
        lines.append('\t}')
        lines.append('#endif // ANDROID_ENABLED')
    lines.append('}')
    lines.append('')

    if class_name == 'MetaPlatformSDK':
        lines.append('void MetaPlatformSDK::_register_generated_classes() {')
        for other_class_name in plan['classes'].keys():
            if other_class_name == 'MetaPlatformSDK':
                # We need to make sure we do this one last.
                continue
            lines.append(f'\tGDREGISTER_CLASS({other_class_name});')
        lines.append(f'\tGDREGISTER_CLASS(MetaPlatformSDK);')

        lines.append('}')
        lines.append('')
    elif class_name == 'MetaPlatformSDK_Message':
        #
        # MetaPlatformSDK_Message::get_data()
        #

        lines.append('Variant MetaPlatformSDK_Message::get_data() const {')
        lines.append('#ifdef ANDROID_ENABLED')
        lines.append('\tERR_FAIL_COND_V(type == MetaPlatformSDK::MessageType::MESSAGE_UNKNOWN, Variant());')
        lines.append('')
        lines.append('\tif (data.get_type() != Variant::NIL) {')
        lines.append('\t\treturn data;')
        lines.append('\t}')
        lines.append('')

        lines.append('\tswitch (type) {')
        for ovr_function, ovr_types in OVR_FUNCTION_TO_MESSAGE_TYPES.items():
            for ovr_type in ovr_types:
                type_name = plan['enums']['MessageType']['value_map'][ovr_type]
                lines.append(f'\t\tcase MetaPlatformSDK::MessageType::{type_name}:')

            if ovr_function== '!ovr_Message_IsError':
                lines.append(f'\t\t\tdata = is_success();')
            else:
                function_name = class_def['function_map'][ovr_function]
                lines.append(f'\t\t\t{function_name}();')

            lines.append(f'\t\t\tbreak;')
            lines.append('')

        lines.append('\t\tcase MetaPlatformSDK::MessageType::MESSAGE_UNKNOWN:')
        lines.append('\t\tdefault:')
        lines.append('\t\t\tERR_PRINT(vformat("MetaPlatformSDK_Message: Cannot get data for unknown message type %s", type));')
        lines.append('\t}')
        lines.append('')
        lines.append('\treturn data;')
        lines.append('#else')
        lines.append('\treturn Variant();')
        lines.append('#endif // ANDROID_ENABLED')
        lines.append('}')
        lines.append('')

        #
        # MetaPlatformSDK_Message::get_type_as_string()
        #

        lines.append('String MetaPlatformSDK_Message::get_type_as_string() const {')
        lines.append('\tswitch (type) {')
        for value in plan['enums']['MessageType']['values']:
            lines.append(f'\t\tcase MetaPlatformSDK::MessageType::{value["name"]}:')
            lines.append(f'\t\t\treturn "{value["name"]}";')
        lines.append('\t}')
        lines.append('\treturn "MESSAGE_UNKNOWN";')
        lines.append('}')
        lines.append('')

    return lines


def get_xml_text(node_list):
    rc = []
    for node in node_list:
        if node.nodeType == node.TEXT_NODE:
            rc.append(node.data)
    return ''.join(rc)


def set_xml_text_with_docs(document, node, docs, tab_count=4):
    # Remove any existing children.
    for child in node.childNodes:
        node.removeChild(child)

    # Manage whitespace to maintain Godot's formatting.
    if tab_count >= 1:
        indent = '\t' * tab_count
        outdent = '\n' + ('\t' * (tab_count - 1))
    else:
        indent = ''
        outdent = '\n'
    text = '\n' + '\n'.join([indent + x for x in docs]) + outdent

    # Add the new text node.
    text_node = document.createTextNode(text)
    node.appendChild(text_node)


def process_docs(docs, function_map=None, ovr_message_type_info=None):
    # Combine and split lines for Godot docs.
    processed = []
    current = ''
    # For making nice docs on request functions.
    ovr_message_type = None
    for d in docs:
        if current != '' and (d == '' or d[0] == '\\' or d[0] == '-'):
            # Try to ensure that all lines end in a period.
            if current[-1] not in ['.',':',']']:
                current += '.'

            processed.append(current)
            current = ''

        if d == '':
            continue

        # Skip some lines from request functions.
        if d.startswith('A message with type ::'):
            m = re.search(r'type ::([A-Za-z0-9_]+)', d)
            if m:
                ovr_message_type = m[1]
            continue
        elif 'message will contain a payload of type const char *' in d:
            ovr_message_type = 'String'
            continue
        elif d.startswith('This response has no payload. If no error occurred, the request was successful. Yay!'):
            ovr_message_type = 'bool'
            continue
        elif d.startswith('First call ::ovr_Message_IsError'):
            continue
        elif d.startswith('If no error occurred, the message will contain a payload of type ::'):
            continue
        elif d.startswith('Extract the payload from the message handle with ::'):
            continue
        # Skip some lines from the message type enum values.
        elif d.startswith('The message will contain a payload of type'):
            continue

        # Convert some limited HTML.
        d = re.sub(r'<b>(.*?)</b>', r'[b]\1[/b]', d)

        if '\\' in d:
            # Convert some of the Doxygen syntax to Godot's formatting.
            d = re.sub(r'^\\param ([A-Za-z0-9_]+) ', lambda m: '- [param ' + camel_to_snake_case(m[1]) + ']: ', d)
            d = re.sub(r'\\b ', '', d)

        if function_map:
            for ovr_name, new_name in function_map.items():
                d = d.replace(ovr_name + '()', '[method ' + new_name + ']')

        # Append to the current line.
        if current == '' or current[-1] == '-':
            current += d
        else:
            current += ' ' + d
    if current != '':
        processed.append(current)

    if ovr_message_type_info is not None and ovr_message_type is not None and (ovr_message_type in ovr_message_type_info or ovr_message_type in ['bool', 'String']):
        processed.append('Returns a [MetaPlatformSDK_Request] which will emit the [signal MetaPlatformSDK_Request.completed] signal on completion with a [MetaPlatformSDK_Message] object.')
        if ovr_message_type == 'bool':
            processed.append('Check if this request was successful by calling [method MetaPlatformSDK_Message.is_success] or accessing the [member MetaPlatformSDK_Message.data] property, which will be a [code]bool[/code] containing the same value in this case.')
        else:
            if ovr_message_type == 'String':
                info = {
                    "message_function": "get_string",
                    "data_type": "String",
                }
            else:
                info = ovr_message_type_info[ovr_message_type]
            processed.append('First call [method MetaPlatformSDK_Message.is_error] to check if the request resulted in an error, or was successful.')
            processed.append(f'If successful, obtain the result by calling [method MetaPlatformSDK_Message.{info["message_function"]}] or accessing the [member MetaPlatformSDK_Message.data] property, which will be a [{info["data_type"]}] in this case.')

    return processed

def update_docs_xml(plan, docs_path, overwrite_docs=False):
    import xml.dom.minidom

    # Build up some useful info to use for request functions.
    ovr_message_type_info = {}
    for rf, ql in OVR_FUNCTION_TO_MESSAGE_TYPES.items():
        for q in ql:
            info = {}
            for mf in plan['classes']['MetaPlatformSDK_Message']['functions'].values():
                if mf['ovr_function']['name'] == rf:
                    info['message_function'] = mf['name']
                    info['data_type'] = mf['return'][4:-1]
                    break
            if len(info) > 0:
                ovr_message_type_info[q] = info

    # Go through each class and update docs.
    for class_name, class_def in plan['classes'].items():
        xml_path = os.path.join(docs_path, f'{class_name}.xml')
        if not os.path.exists(xml_path):
            continue

        document = xml.dom.minidom.parse(xml_path)
        updated = False

        for method_element in document.getElementsByTagName('method'):
            description_element = method_element.getElementsByTagName('description')[0]
            if not overwrite_docs and get_xml_text(description_element.childNodes).strip() != '':
                # Only update functions with empty descriptions.
                continue

            function_name = method_element.getAttribute('name')
            if not function_name in class_def['functions']:
                # Skip any functions not in our plan.
                continue

            function = class_def['functions'][function_name]
            docs = function['ovr_function']['docs'] if 'docs' in function['ovr_function'] else []

            if len(docs) > 0:
                set_xml_text_with_docs(document, description_element, process_docs(docs, class_def['function_map'], ovr_message_type_info))
                updated = True

        for member_element in document.getElementsByTagName('member'):
            if not overwrite_docs and get_xml_text(member_element.childNodes).strip() != '':
                # Only update members with empty descriptions.
                continue

            function_name = member_element.getAttribute('getter')
            if function_name in class_def['functions']:
                function = class_def['functions'][function_name]
                docs = function['ovr_function']['docs'] if 'docs' in function['ovr_function'] else []

                if len(docs) > 0:
                    set_xml_text_with_docs(document, member_element, process_docs(docs, class_def['function_map'], ovr_message_type_info), 3)
                    updated = True

        if 'local_enums' in class_def:
            for constant_element in document.getElementsByTagName('constant'):
                if not overwrite_docs and get_xml_text(constant_element.childNodes).strip() != '':
                    # Only update constants with empty descriptions.
                    continue

                constant_name = constant_element.getAttribute('name')

                # Find the enum value in the plan, and add docs if we have any.
                enum_found = False
                for enum_name in class_def['local_enums']:
                    for enum_value in plan['enums'][enum_name]['values']:
                        if enum_value['name'] == constant_name:
                            enum_found = True
                            if 'docs' in enum_value:
                                set_xml_text_with_docs(document, constant_element, process_docs(enum_value['docs'], class_def['function_map']), tab_count=3)
                                updated = True
                            break
                    if enum_found:
                        break

        if updated:
            # Do a little post-processing to the XML docs so it matches Godot's output.
            xml_data = document.toxml()
            xml_data = re.sub(r'<\?xml version="1.0" \?>', r'<?xml version="1.0" encoding="UTF-8" ?>\n', xml_data)
            xml_data = re.sub(r'^<class (xmlns:xsi="[^"]*" )(.*) xsi:', r'<class \2 \1xsi:', xml_data, flags=re.MULTILINE)
            xml_data = re.sub(r'(\S)/>', r'\1 />', xml_data)
            xml_data = re.sub(r'>([^<]+)<', lambda m: '>'+m.group(1).replace('&quot;', '"')+'<', xml_data)
            xml_data += "\n"

            with open(xml_path, 'wt') as fd:
                fd.write(xml_data)


def scons_emit_files(target, source, env):
    sdk_headers_path = str(source[0])
    if not os.path.exists(sdk_headers_path) or not os.path.isdir(sdk_headers_path):
        raise Exception("SDK headers path '%s' doesn't exist or isn't a directory" % sdk_headers_path)
    if not os.path.exists(os.path.join(sdk_headers_path, 'OVR_Platform.h')):
        raise Exception("SDK headers path '%s' doesn't contain 'OVR_Platform.h'" % sdk_headers_path)

    headers = parse_headers(sdk_headers_path)
    plan = make_codegen_plan(headers)
    output_path = target[0].abspath

    header_path = os.path.join(output_path, 'include', 'platform_sdk')
    source_path = os.path.join(output_path, 'src')

    files = []
    for class_name in plan['classes'].keys():
        file_base = camel_to_snake_case(class_name)
        files.append(env.File(os.path.join(header_path, file_base + '.h')))
        files.append(env.File(os.path.join(source_path, file_base + '.cpp')))

    env.Clean(target, files)
    env["godot_meta_toolkit_gen_dir"] = output_path
    return files, source


def scons_generate_bindings(target, source, env):
    headers = parse_headers(str(source[0]))
    plan = make_codegen_plan(headers)

    generate_code(plan, env["godot_meta_toolkit_gen_dir"])
    return None


def main():
    import argparse
    import json

    parser = argparse.ArgumentParser(
        description='Generates Meta Platform SDK bindings based on the native header files',
    )
    parser.add_argument('sdk_headers_path', type=str, help="The path to the Platform SDK's header files, for example: thirdparty/ovr_platform_sdk/Include")
    parser.add_argument('output_path', type=str, help="The path to output the generated code, for example: toolkit/gen")
    parser.add_argument('--dump-parsed-headers', type=str, metavar='PATH', help="Path to dump the parsed headers in JSON format")
    parser.add_argument('--dump-codegen-plan', type=str, metavar='PATH', help="Path to dump the codegen plan in JSON format")
    parser.add_argument('--update-docs-xml', type=str, metavar='PATH', help='Path to XML documentation to update with generated descriptions')
    parser.add_argument('--overwrite-docs', action='store_true', help='Overwrite existing documentation with autogenerated text')
    parser.add_argument('--verbose', action='store_true', default=False, help="Show extra messages and simple stats")

    args = parser.parse_args()

    # Do some sanity checks on the arguments.
    if not os.path.exists(args.sdk_headers_path) or not os.path.isdir(args.sdk_headers_path):
        raise Exception("SDK headers path '%s' doesn't exist or isn't a directory" % args.sdk_headers_path)
    if not os.path.exists(os.path.join(args.sdk_headers_path, 'OVR_Platform.h')):
        raise Exception("SDK headers path '%s' doesn't contain 'OVR_Platform.h'" % args.sdk_headers_path)
    if args.update_docs_xml:
        if not os.path.exists(args.update_docs_xml) or not os.path.isdir(args.update_docs_xml):
            raise Exception("Non-existent directory '%s' given for --update-docs-xml" % args.update_docs_xml)
    if not os.path.exists(args.output_path):
        os.makedirs(args.output_path)
    elif not os.path.isdir(args.output_path):
        raise Exception("Output path '%s' is not a directory" % args.output_path)

     # Parse the headers into an intermediate format.
    headers = parse_headers(args.sdk_headers_path)
    if args.dump_parsed_headers:
        with open(args.dump_parsed_headers, 'wt') as fd:
            json.dump(headers, fd, indent=4)

    # Transform that into a plan for the code we'll generate.
    plan = make_codegen_plan(headers, args.verbose)
    if args.dump_codegen_plan:
        with open(args.dump_codegen_plan, 'wt') as fd:
            json.dump(plan, fd, indent=4)
    if args.verbose:
        print("Classes:", len(plan['classes']))
        print("Singleton functions:", len(plan['classes']['MetaPlatformSDK']['functions']))
        print("Enums:", len(plan['enums']))

    # Generate the code.
    generate_code(plan, args.output_path)

    # Optionally, update XML docs.
    if args.update_docs_xml:
        update_docs_xml(plan, args.update_docs_xml, args.overwrite_docs)


if __name__ == '__main__': main()
