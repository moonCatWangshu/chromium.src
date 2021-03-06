// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/browser/blob_holder.h"

#include <algorithm>
#include <utility>

#include "base/logging.h"
#include "content/public/browser/blob_handle.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_process_host.h"

namespace extensions {

namespace {

// Address to this variable used as the user data key.
const int kBlobHolderUserDataKey = 0;
}

// static
BlobHolder* BlobHolder::FromRenderProcessHost(
    content::RenderProcessHost* render_process_host) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  DCHECK(render_process_host);
  BlobHolder* existing = static_cast<BlobHolder*>(
      render_process_host->GetUserData(&kBlobHolderUserDataKey));

  if (existing)
    return existing;

  BlobHolder* new_instance = new BlobHolder(render_process_host);
  render_process_host->SetUserData(&kBlobHolderUserDataKey, new_instance);
  return new_instance;
}

BlobHolder::~BlobHolder() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
}

void BlobHolder::HoldBlobReference(scoped_ptr<content::BlobHandle> blob) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  DCHECK(!ContainsBlobHandle(blob.get()));

  std::string uuid = blob->GetUUID();
  held_blobs_.insert(make_pair(uuid, make_linked_ptr(blob.release())));
}

BlobHolder::BlobHolder(content::RenderProcessHost* render_process_host)
    : render_process_host_(render_process_host) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
}

bool BlobHolder::ContainsBlobHandle(content::BlobHandle* handle) const {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  for (BlobHandleMultimap::const_iterator it = held_blobs_.begin();
       it != held_blobs_.end();
       ++it) {
    if (it->second.get() == handle)
      return true;
  }

  return false;
}

void BlobHolder::DropBlobs(const std::vector<std::string>& blob_uuids) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  for (std::vector<std::string>::const_iterator uuid_it = blob_uuids.begin();
       uuid_it != blob_uuids.end();
       ++uuid_it) {
    BlobHandleMultimap::iterator it = held_blobs_.find(*uuid_it);

    if (it != held_blobs_.end()) {
      held_blobs_.erase(it);
    } else {
      DLOG(ERROR) << "Tried to release a Blob we don't have ownership to."
                  << "UUID: " << *uuid_it;
      render_process_host_->ReceivedBadMessage();
    }
  }
}

}  // namespace extensions
