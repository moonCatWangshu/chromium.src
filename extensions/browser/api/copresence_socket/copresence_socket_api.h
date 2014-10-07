// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_API_COPRESENCE_SOCKET_COPRESENCE_SOCKET_API_H_
#define CHROME_BROWSER_EXTENSIONS_API_COPRESENCE_SOCKET_COPRESENCE_SOCKET_API_H_

#include <map>
#include <string>
#include <vector>

#include "base/macros.h"
#include "base/memory/scoped_ptr.h"
#include "extensions/browser/api/api_resource.h"
#include "extensions/browser/api/api_resource_manager.h"
#include "extensions/browser/browser_context_keyed_api_factory.h"
#include "extensions/browser/extension_function.h"
#include "extensions/browser/extension_function_histogram_value.h"

namespace copresence_sockets {
class CopresenceSocket;
}

namespace net {
class IOBuffer;
}

namespace extensions {

class CopresencePeerResource;
class CopresenceSocketResource;

class CopresenceSocketFunction : public UIThreadExtensionFunction {
 public:
  CopresenceSocketFunction();

  void DispatchOnConnectedEvent(
      int peer_id,
      scoped_ptr<copresence_sockets::CopresenceSocket> socket);
  void DispatchOnReceiveEvent(int socket_id,
                              const scoped_refptr<net::IOBuffer>& buffer,
                              int size);

 protected:
  // ExtensionFunction overrides:
  virtual ExtensionFunction::ResponseAction Run() override;

  // Override this and do actual work here.
  virtual ExtensionFunction::ResponseAction Execute() = 0;

  // Takes ownership of peer.
  int AddPeer(CopresencePeerResource* peer);
  // Takes ownership of socket.
  int AddSocket(CopresenceSocketResource* socket);

  // Takes ownership of peer.
  void ReplacePeer(const std::string& extension_id,
                   int peer_id,
                   CopresencePeerResource* peer);

  CopresencePeerResource* GetPeer(int peer_id);
  CopresenceSocketResource* GetSocket(int socket_id);

  void RemovePeer(int peer_id);
  void RemoveSocket(int socket_id);

  virtual ~CopresenceSocketFunction();

 private:
  void Initialize();

  ApiResourceManager<CopresencePeerResource>* peers_manager_;
  ApiResourceManager<CopresenceSocketResource>* sockets_manager_;
};

class CopresenceSocketCreatePeerFunction : public CopresenceSocketFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("copresenceSocket.createPeer",
                             COPRESENCESOCKET_CREATEPEER);

 protected:
  virtual ~CopresenceSocketCreatePeerFunction() {}
  virtual ExtensionFunction::ResponseAction Execute() override;

 private:
  void OnCreated(int peer_id, const std::string& locator);
};

class CopresenceSocketDestroyPeerFunction : public CopresenceSocketFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("copresenceSocket.destroyPeer",
                             COPRESENCESOCKET_DESTROYPEER);

 protected:
  virtual ~CopresenceSocketDestroyPeerFunction() {}
  virtual ExtensionFunction::ResponseAction Execute() override;
};

class CopresenceSocketSendFunction : public CopresenceSocketFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("copresenceSocket.send", COPRESENCESOCKET_SEND);

 protected:
  virtual ~CopresenceSocketSendFunction() {}
  virtual ExtensionFunction::ResponseAction Execute() override;
};

class CopresenceSocketDisconnectFunction : public CopresenceSocketFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("copresenceSocket.disconnect",
                             COPRESENCESOCKET_DISCONNECT);

 protected:
  virtual ~CopresenceSocketDisconnectFunction() {}
  virtual ExtensionFunction::ResponseAction Execute() override;
};

}  // namespace extensions

#endif  // CHROME_BROWSER_EXTENSIONS_API_COPRESENCE_SOCKET_COPRESENCE_SOCKET_API_H_
