// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Multiply-included file, no traditional include guard.

#include <string>

#include "base/basictypes.h"
#include "base/process.h"
#include "build/build_config.h"
#include "chrome/common/common_param_traits.h"
#include "chrome/common/nacl_types.h"
#include "chrome/common/pnacl_types.h"
#include "content/public/common/common_param_traits.h"
#include "ipc/ipc_channel_handle.h"
#include "ipc/ipc_message_macros.h"
#include "ipc/ipc_platform_file.h"
#include "url/gurl.h"

#define IPC_MESSAGE_START NaClHostMsgStart

IPC_STRUCT_TRAITS_BEGIN(nacl::NaClLaunchParams)
  IPC_STRUCT_TRAITS_MEMBER(manifest_url)
  IPC_STRUCT_TRAITS_MEMBER(render_view_id)
  IPC_STRUCT_TRAITS_MEMBER(permission_bits)
  IPC_STRUCT_TRAITS_MEMBER(uses_irt)
  IPC_STRUCT_TRAITS_MEMBER(enable_dyncode_syscalls)
  IPC_STRUCT_TRAITS_MEMBER(enable_exception_handling)
IPC_STRUCT_TRAITS_END()

IPC_STRUCT_TRAITS_BEGIN(nacl::PnaclCacheInfo)
  IPC_STRUCT_TRAITS_MEMBER(pexe_url)
  IPC_STRUCT_TRAITS_MEMBER(abi_version)
  IPC_STRUCT_TRAITS_MEMBER(opt_level)
  IPC_STRUCT_TRAITS_MEMBER(last_modified)
  IPC_STRUCT_TRAITS_MEMBER(etag)
IPC_STRUCT_TRAITS_END()

// A renderer sends this to the browser process when it wants to start
// a new instance of the Native Client process. The browser will launch
// the process and return an IPC channel handle. This handle will only
// be valid if the NaCl IPC proxy is enabled.
IPC_SYNC_MESSAGE_CONTROL1_4(NaClHostMsg_LaunchNaCl,
                            nacl::NaClLaunchParams /* launch_params */,
                            nacl::FileDescriptor /* imc channel handle */,
                            IPC::ChannelHandle /* ipc_channel_handle */,
                            base::ProcessId /* plugin_pid */,
                            int /* plugin_child_id */)

// A renderer sends this to the browser process when it wants to
// open a file for from the Pnacl component directory.
IPC_SYNC_MESSAGE_CONTROL1_1(NaClHostMsg_GetReadonlyPnaclFD,
                            std::string /* name of requested PNaCl file */,
                            IPC::PlatformFileForTransit /* output file */)

// A renderer sends this to the browser process when it wants to
// create a temporary file.
IPC_SYNC_MESSAGE_CONTROL0_1(NaClHostMsg_NaClCreateTemporaryFile,
                            IPC::PlatformFileForTransit /* out file */)

// A renderer sends this to the browser to request a file descriptor for
// a translated nexe.
IPC_MESSAGE_CONTROL3(NaClHostMsg_NexeTempFileRequest,
                     int /* render_view_id */,
                     int /* instance */,
                     nacl::PnaclCacheInfo /* cache info */)

// The browser replies to a renderer's temp file request with output_file,
// which is either a writeable temp file to use for translation, or a
// read-only file containing the translated nexe from the cache.
IPC_MESSAGE_CONTROL3(NaClViewMsg_NexeTempFileReply,
                     int /* instance */,
                     bool /* is_cache_hit */,
                     IPC::PlatformFileForTransit /* output file */)

// A renderer sends this to the browser to report that its translation has
// finished and its temp file contains the translated nexe.
IPC_MESSAGE_CONTROL1(NaClHostMsg_ReportTranslationFinished,
                     int /* instance */)

// A renderer sends this to the browser process to report an error.
IPC_MESSAGE_CONTROL2(NaClHostMsg_NaClErrorStatus,
                     int /* render_view_id */,
                     int /* Error ID */)

// A renderer sends this to the browser process when it wants to
// open a NaCl executable file from an installed application directory.
IPC_SYNC_MESSAGE_CONTROL2_3(NaClHostMsg_OpenNaClExecutable,
                            int /* render_view_id */,
                            GURL /* URL of NaCl executable file */,
                            IPC::PlatformFileForTransit /* output file */,
                            uint64 /* file_token_lo */,
                            uint64 /* file_token_hi */)
