// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser;

import org.chromium.base.CalledByNative;
import org.chromium.base.ObserverList;
import org.chromium.chrome.browser.profiles.Profile;
import org.chromium.components.bookmarks.BookmarkId;

import java.util.ArrayList;
import java.util.List;

/**
 * Provides the communication channel for Android to fetch and manipulate the
 * bookmark model stored in native.
 */
public class BookmarksBridge {
    private final Profile mProfile;
    private long mNativeBookmarksBridge;
    private boolean mIsNativeBookmarkModelLoaded;
    private final List<DelayedBookmarkCallback> mDelayedBookmarkCallbacks =
            new ArrayList<DelayedBookmarkCallback>();
    private final ObserverList<BookmarkModelObserver> mObservers =
            new ObserverList<BookmarkModelObserver>();

    /**
     * Interface for callback object for fetching bookmarks and folder hierarchy.
     */
    public interface BookmarksCallback {
        /**
         * Callback method for fetching bookmarks for a folder and the folder hierarchy.
         * @param folderId The folder id to which the bookmarks belong.
         * @param bookmarksList List holding the fetched bookmarks and details.
         */
        @CalledByNative("BookmarksCallback")
        void onBookmarksAvailable(BookmarkId folderId, List<BookmarkItem> bookmarksList);

        /**
         * Callback method for fetching the folder hierarchy.
         * @param folderId The folder id to which the bookmarks belong.
         * @param bookmarksList List holding the fetched folder details.
         */
        @CalledByNative("BookmarksCallback")
        void onBookmarksFolderHierarchyAvailable(BookmarkId folderId,
                List<BookmarkItem> bookmarksList);
    }

    /**
     * Interface that provides listeners to be notified of changes to the bookmark model.
     */
    public interface BookmarkModelObserver {
        /**
         * Invoked when a node has moved.
         * @param oldParent The parent before the move.
         * @param oldIndex The index of the node in the old parent.
         * @param newParent The parent after the move.
         * @param newIndex The index of the node in the new parent.
         */
        void bookmarkNodeMoved(
                BookmarkItem oldParent, int oldIndex, BookmarkItem newParent, int newIndex);

        /**
         * Invoked when a node has been added.
         * @param parent The parent of the node being added.
         * @param index The index of the added node.
         */
        void bookmarkNodeAdded(BookmarkItem parent, int index);

        /**
         * Invoked when a node has been removed, the item may still be starred though.
         * @param parent The parent of the node that was removed.
         * @param oldIndex The index of the removed node in the parent before it was removed.
         * @param node The node that was removed.
         */
        void bookmarkNodeRemoved(BookmarkItem parent, int oldIndex, BookmarkItem node);

        /**
         * Invoked when the title or url of a node changes.
         * @param node The node being changed.
         */
        void bookmarkNodeChanged(BookmarkItem node);

        /**
         * Invoked when the children (just direct children, not descendants) of a node have been
         * reordered in some way, such as sorted.
         * @param node The node whose children are being reordered.
         */
        void bookmarkNodeChildrenReordered(BookmarkItem node);

        /**
         * Invoked before an extensive set of model changes is about to begin.  This tells UI
         * intensive observers to wait until the updates finish to update themselves. These methods
         * should only be used for imports and sync. Observers should still respond to
         * BookmarkNodeRemoved immediately, to avoid holding onto stale node references.
         */
        void extensiveBookmarkChangesBeginning();

        /**
         * Invoked after an extensive set of model changes has ended.  This tells observers to
         * update themselves if they were waiting for the update to finish.
         */
        void extensiveBookmarkChangesEnded();

        /**
         *  Called when there are changes to the bookmark model that don't trigger any of the other
         *  callback methods. For example, this is called when partner bookmarks change.
         */
        void bookmarkModelChanged();

        /**
         * Called when the native side of bookmark is loaded and now in usable state.
         */
        void bookmarkModelLoaded();
    }

    /**
     * Handler to fetch the bookmarks, titles, urls and folder hierarchy.
     * @param profile Profile instance corresponding to the active profile.
     */
    public BookmarksBridge(Profile profile) {
        mProfile = profile;
        mNativeBookmarksBridge = nativeInit(profile);
    }

    /**
     * Destroys this instance so no further calls can be executed.
     */
    public void destroy() {
        if (mNativeBookmarksBridge != 0) {
            nativeDestroy(mNativeBookmarksBridge);
            mNativeBookmarksBridge = 0;
            mIsNativeBookmarkModelLoaded = false;
            mDelayedBookmarkCallbacks.clear();
        }
        mObservers.clear();
    }

    /**
     * Add an observer to bookmark model changes.
     * @param observer The observer to be added.
     */
    public void addObserver(BookmarkModelObserver observer) {
        mObservers.addObserver(observer);
    }

    /**
     * Remove an observer of bookmark model changes.
     * @param observer The observer to be removed.
     */
    public void removeObserver(BookmarkModelObserver observer) {
        mObservers.removeObserver(observer);
    }

    /**
     * @return Whether or not the underlying bookmark model is loaded.
     */
    public boolean isBookmarkModelLoaded() {
        return mIsNativeBookmarkModelLoaded;
    }

    /**
     * @return A BookmarkItem instance for the given BookmarkId.
     */
    public BookmarkItem getBookmarkById(BookmarkId id) {
        assert mIsNativeBookmarkModelLoaded;
        return nativeGetBookmarkByID(mNativeBookmarksBridge, id.getId(), id.getType());
    }

    /**
     * @return All the permanent nodes.
     */
    public List<BookmarkId> getPermanentNodeIDs() {
        assert mIsNativeBookmarkModelLoaded;
        List<BookmarkId> result = new ArrayList<BookmarkId>();
        nativeGetPermanentNodeIDs(mNativeBookmarksBridge, result);
        return result;
    }

    /**
     * Reads sub-folder IDs, sub-bookmark IDs, or both of the given folder.
     *
     * @param getFolders   Whether sub-folders should be returned.
     * @param getBookmarks Whether sub-bookmarks should be returned.
     * @return Child IDs of the given folder, with the specified type.
     */
    public List<BookmarkId> getChildIDs(BookmarkId id, boolean getFolders, boolean getBookmarks) {
        assert mIsNativeBookmarkModelLoaded;
        List<BookmarkId> result = new ArrayList<BookmarkId>();
        nativeGetChildIDs(mNativeBookmarksBridge,
                id.getId(),
                id.getType(),
                getFolders,
                getBookmarks,
                result);
        return result;
    }

    /**
     * @return All bookmark IDs ordered by descending creation date.
     */
    public List<BookmarkId> getAllBookmarkIDsOrderedByCreationDate() {
        assert mIsNativeBookmarkModelLoaded;
        List<BookmarkId> result = new ArrayList<BookmarkId>();
        nativeGetAllBookmarkIDsOrderedByCreationDate(mNativeBookmarksBridge, result);
        return result;
    }

    /**
     * Set title of the given bookmark.
     */
    public void setBookmarkTitle(BookmarkId id, String title) {
        assert mIsNativeBookmarkModelLoaded;
        nativeSetBookmarkTitle(mNativeBookmarksBridge, id.getId(), id.getType(), title);
    }

    /**
     * Set URL of the given bookmark.
     */
    public void setBookmarkUrl(BookmarkId id, String url) {
        assert mIsNativeBookmarkModelLoaded;
        nativeSetBookmarkUrl(mNativeBookmarksBridge, id.getId(), id.getType(), url);
    }

    /**
     * @return Whether the given bookmark exist in the current bookmark model, e.g., not deleted.
     */
    public boolean doesBookmarkExist(BookmarkId id) {
        assert mIsNativeBookmarkModelLoaded;
        return nativeDoesBookmarkExist(mNativeBookmarksBridge, id.getId(), id.getType());
    }

    /**
     * Fetches the bookmarks of the given folder. This is an always-synchronous version of another
     * getBookmarksForForder function.
     *
     * @param folderId The parent folder id.
     * @return Bookmarks of the given folder.
     */
    public List<BookmarkItem> getBookmarksForFolder(BookmarkId folderId) {
        assert mIsNativeBookmarkModelLoaded;
        List<BookmarkItem> result = new ArrayList<BookmarkItem>();
        nativeGetBookmarksForFolder(mNativeBookmarksBridge, folderId, null, result);
        return result;
    }

    /**
     * Fetches the bookmarks of the current folder. Callback will be
     * synchronous if the bookmark model is already loaded and async if it is loaded in the
     * background.
     * @param folderId The current folder id.
     * @param callback Instance of a callback object.
     */
    public void getBookmarksForFolder(BookmarkId folderId, BookmarksCallback callback) {
        if (mIsNativeBookmarkModelLoaded) {
            nativeGetBookmarksForFolder(mNativeBookmarksBridge, folderId, callback,
                    new ArrayList<BookmarkItem>());
        } else {
            mDelayedBookmarkCallbacks.add(new DelayedBookmarkCallback(folderId, callback,
                    DelayedBookmarkCallback.GET_BOOKMARKS_FOR_FOLDER, this));
        }
    }

    /**
     * Fetches the folder hierarchy of the given folder. Callback will be
     * synchronous if the bookmark model is already loaded and async if it is loaded in the
     * background.
     * @param folderId The current folder id.
     * @param callback Instance of a callback object.
     */
    public void getCurrentFolderHierarchy(BookmarkId folderId, BookmarksCallback callback) {
        if (mIsNativeBookmarkModelLoaded) {
            nativeGetCurrentFolderHierarchy(mNativeBookmarksBridge, folderId, callback,
                    new ArrayList<BookmarkItem>());
        } else {
            mDelayedBookmarkCallbacks.add(new DelayedBookmarkCallback(folderId, callback,
                    DelayedBookmarkCallback.GET_CURRENT_FOLDER_HIERARCHY, this));
        }
    }

    /**
     * Deletes a specified bookmark node.
     * @param bookmarkId The ID of the bookmark to be deleted.
     */
    public void deleteBookmark(BookmarkId bookmarkId) {
        nativeDeleteBookmark(mNativeBookmarksBridge, bookmarkId);
    }

    /**
     * Move the bookmark to the new index within same folder or to a different folder.
     * @param bookmarkId The id of the bookmark that is being moved.
     * @param newParentId The parent folder id.
     * @param index The new index for the bookmark.
     */
    public void moveBookmark(BookmarkId bookmarkId, BookmarkId newParentId, int index) {
        nativeMoveBookmark(mNativeBookmarksBridge, bookmarkId, newParentId, index);
    }

    /**
     * A bridge function to BookmarkModelFactory::GetForProfile.
     */
    public static long getNativeBookmarkModel(Profile profile) {
        return nativeGetNativeBookmarkModel(profile);
    }

    public static boolean isEditBookmarksEnabled() {
        return nativeIsEditBookmarksEnabled();
    }

    @CalledByNative
    private void bookmarkModelLoaded() {
        mIsNativeBookmarkModelLoaded = true;

        for (BookmarkModelObserver observer : mObservers) {
            observer.bookmarkModelLoaded();
        }

        if (!mDelayedBookmarkCallbacks.isEmpty()) {
            for (int i = 0; i < mDelayedBookmarkCallbacks.size(); i++) {
                mDelayedBookmarkCallbacks.get(i).callCallbackMethod();
            }
            mDelayedBookmarkCallbacks.clear();
        }
    }

    @CalledByNative
    private void bookmarkModelDeleted() {
        destroy();
    }

    @CalledByNative
    private void bookmarkNodeMoved(
            BookmarkItem oldParent, int oldIndex, BookmarkItem newParent, int newIndex) {
        for (BookmarkModelObserver observer : mObservers) {
            observer.bookmarkNodeMoved(oldParent, oldIndex, newParent, newIndex);
        }
    }

    @CalledByNative
    private void bookmarkNodeAdded(BookmarkItem parent, int index) {
        for (BookmarkModelObserver observer : mObservers) {
            observer.bookmarkNodeAdded(parent, index);
        }
    }

    @CalledByNative
    private void bookmarkNodeRemoved(BookmarkItem parent, int oldIndex, BookmarkItem node) {
        for (BookmarkModelObserver observer : mObservers) {
            observer.bookmarkNodeRemoved(parent, oldIndex, node);
        }
    }

    @CalledByNative
    private void bookmarkNodeChanged(BookmarkItem node) {
        for (BookmarkModelObserver observer : mObservers) {
            observer.bookmarkNodeChanged(node);
        }
    }

    @CalledByNative
    private void bookmarkNodeChildrenReordered(BookmarkItem node) {
        for (BookmarkModelObserver observer : mObservers) {
            observer.bookmarkNodeChildrenReordered(node);
        }
    }

    @CalledByNative
    private void extensiveBookmarkChangesBeginning() {
        for (BookmarkModelObserver observer : mObservers) {
            observer.extensiveBookmarkChangesBeginning();
        }
    }

    @CalledByNative
    private void extensiveBookmarkChangesEnded() {
        for (BookmarkModelObserver observer : mObservers) {
            observer.extensiveBookmarkChangesEnded();
        }
    }

    @CalledByNative
    private void bookmarkModelChanged() {
        for (BookmarkModelObserver observer : mObservers) {
            observer.bookmarkModelChanged();
        }
    }

    @CalledByNative
    private static BookmarkItem createBookmarkItem(long id, int type, String title, String url,
            boolean isFolder, long parentId, int parentIdType, boolean isEditable,
            boolean isManaged) {
        return new BookmarkItem(new BookmarkId(id, type), title, url, isFolder,
                new BookmarkId(parentId, parentIdType), isEditable, isManaged);
    }

    @CalledByNative
    private static void addToList(List<BookmarkItem> bookmarksList, BookmarkItem bookmark) {
        bookmarksList.add(bookmark);
    }

    @CalledByNative
    private static void addToBookmarkIdList(List<BookmarkId> bookmarkIdList, long id, int type) {
        bookmarkIdList.add(new BookmarkId(id, type));
    }

    @CalledByNative
    private static BookmarkId createBookmarkId(long id, int type) {
        return new BookmarkId(id, type);
    }

    private native BookmarkItem nativeGetBookmarkByID(long nativeBookmarksBridge, long id,
            int type);
    private native void nativeGetPermanentNodeIDs(long nativeBookmarksBridge,
            List<BookmarkId> bookmarksList);
    private native void nativeGetChildIDs(long nativeBookmarksBridge, long id, int type,
            boolean getFolders, boolean getBookmarks, List<BookmarkId> bookmarksList);
    private native void nativeGetAllBookmarkIDsOrderedByCreationDate(long nativeBookmarksBridge,
            List<BookmarkId> result);
    private native void nativeSetBookmarkTitle(long nativeBookmarksBridge, long id, int type,
            String title);
    private native void nativeSetBookmarkUrl(long nativeBookmarksBridge, long id, int type,
            String url);
    private native boolean nativeDoesBookmarkExist(long nativeBookmarksBridge, long id, int type);
    private native void nativeGetBookmarksForFolder(long nativeBookmarksBridge,
            BookmarkId folderId, BookmarksCallback callback,
            List<BookmarkItem> bookmarksList);
    private native void nativeGetCurrentFolderHierarchy(long nativeBookmarksBridge,
            BookmarkId folderId, BookmarksCallback callback,
            List<BookmarkItem> bookmarksList);
    private native void nativeDeleteBookmark(long nativeBookmarksBridge, BookmarkId bookmarkId);
    private native void nativeMoveBookmark(long nativeBookmarksBridge, BookmarkId bookmarkId,
            BookmarkId newParentId, int index);
    private static native long nativeGetNativeBookmarkModel(Profile profile);
    private native long nativeInit(Profile profile);
    private native void nativeDestroy(long nativeBookmarksBridge);
    private static native boolean nativeIsEditBookmarksEnabled();

    /**
     * Simple object representing the bookmark item.
     */
    public static class BookmarkItem {

        private final String mTitle;
        private final String mUrl;
        private final BookmarkId mId;
        private final boolean mIsFolder;
        private final BookmarkId mParentId;
        private final boolean mIsEditable;
        private final boolean mIsManaged;

        private BookmarkItem(BookmarkId id, String title, String url, boolean isFolder,
                BookmarkId parentId, boolean isEditable, boolean isManaged) {
            mId = id;
            mTitle = title;
            mUrl = url;
            mIsFolder = isFolder;
            mParentId = parentId;
            mIsEditable = isEditable;
            mIsManaged = isManaged;
        }

        /** @return Title of the bookmark item. */
        public String getTitle() {
            return mTitle;
        }

        /** @return Url of the bookmark item. */
        public String getUrl() {
            return mUrl;
        }

        /** @return Id of the bookmark item. */
        public BookmarkId getId() {
            return mId;
        }

        /** @return Whether item is a folder or a bookmark. */
        public boolean isFolder() {
            return mIsFolder;
        }

        /** @return Parent id of the bookmark item. */
        public BookmarkId getParentId() {
            return mParentId;
        }

        /** @return Whether this bookmark can be edited. */
        public boolean isEditable() {
            return mIsEditable;
        }

        /** @return Whether this is a managed bookmark. */
        public boolean isManaged() {
            return mIsManaged;
        }
    }

    /**
     * Details about callbacks that need to be called once the bookmark model has loaded.
     */
    private static class DelayedBookmarkCallback {

        private static final int GET_BOOKMARKS_FOR_FOLDER = 0;
        private static final int GET_CURRENT_FOLDER_HIERARCHY = 1;

        private final BookmarksCallback mCallback;
        private final BookmarkId mFolderId;
        private final int mCallbackMethod;
        private final BookmarksBridge mHandler;

        private DelayedBookmarkCallback(BookmarkId folderId, BookmarksCallback callback,
                int method, BookmarksBridge handler) {
            mFolderId = folderId;
            mCallback = callback;
            mCallbackMethod = method;
            mHandler = handler;
        }

        /**
         * Invoke the callback method.
         */
        private void callCallbackMethod() {
            switch (mCallbackMethod) {
                case GET_BOOKMARKS_FOR_FOLDER:
                    mHandler.getBookmarksForFolder(mFolderId, mCallback);
                    break;
                case GET_CURRENT_FOLDER_HIERARCHY:
                    mHandler.getCurrentFolderHierarchy(mFolderId, mCallback);
                    break;
                default:
                    assert false;
                    break;
            }
        }
    }
}
