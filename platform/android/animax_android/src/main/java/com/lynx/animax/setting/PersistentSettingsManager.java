// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.setting;

import android.content.Context;
import android.content.SharedPreferences;
import androidx.annotation.Keep;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import com.lynx.animax.service.IAnimaXSettingService;
import com.lynx.animax.util.AnimaX;
import com.lynx.animax.util.AnimaXLog;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

/**
 * A process-wide singleton backed by persistent storage (a well-known SharedPreferences
 * file), for settings that must stay stable for the whole process: once a value has been
 * observed by any consumer, all consumers of the same launch have to keep observing the
 * same value, even if the remote setting changes mid-process. It is deliberately NOT an
 * {@code IAnimaXSettingService}: the remote settings channel keeps flowing through that
 * interface as before, while this class provides the process-frozen local snapshot
 * underneath. It is internal to the AnimaX SDK — integrations are not expected to use it
 * directly. Retrieve it via {@link #inst()}.
 *
 * <p>Reading: {@link #getValueByKey} serves a snapshot of the file that is loaded at most
 * once per process and then frozen, so all readers of a launch observe identical values no
 * matter when they are created. Keys absent from the file yield empty values, which callers
 * treat as "use the default".
 *
 * <p>Writing: the SDK reconciles the remote settings (read through an
 * {@code IAnimaXSettingService}, e.g. the ability's service) with the frozen snapshot via
 * {@link #syncSetting}, which calls {@link #persistValue} / {@link #removeValue} to store
 * the remote value into, or delete a remotely-dropped key from, the file. The change takes
 * effect on the NEXT launch: the "freeze the current snapshot first, write afterwards"
 * ordering is built into this class, so callers cannot accidentally make a remote change
 * take effect mid-process. When no setting service is available, nothing is written.
 *
 * <p>The file is app-private storage (no permissions required) and is only shared within a
 * single process. This class intentionally depends on the Android framework only.
 */
@Keep
public class PersistentSettingsManager {
  /** SharedPreferences file that holds the persisted settings. */
  public static final String SETTINGS_SP_NAME = "animax_settings";

  private static final String TAG = "AnimaXPersistentSettings";

  private static volatile PersistentSettingsManager sInstance = null;

  /**
   * Process-wide frozen snapshot, loaded from disk at most once. {@code null} means "not
   * loaded yet" (AnimaX has not been initialized with an application context so far); reads
   * then return empty values and loading is retried on the next access.
   */
  private volatile Map<String, AnimaXSettingValue> mSnapshot = null;

  private final Object mSnapshotLock = new Object();

  /**
   * Keys whose remote value has been observed (a non-empty remote read) and reconciled in
   * this process. The write-back only affects the next launch, so re-syncing such a key is
   * pure overhead; these keys are never looked at again.
   */
  private final Set<String> mSyncedKeys =
      Collections.newSetFromMap(new ConcurrentHashMap<String, Boolean>());

  /**
   * How many times an empty remote read keeps being retried per key. The retry is passive —
   * it only happens when a new AnimaX instance triggers a sync — so the cap counts
   * opportunities, not wall-clock time: sparse instance creations stretch the retry period,
   * frequent ones bound it quickly. A locally-persisted value is only deleted once the
   * attempts run out (transient fetch misses must not wipe it); after that the key is given
   * up, so a remotely-unconfigured key costs no further remote reads for the rest of the
   * process.
   */
  @VisibleForTesting int maxEmptyReadAttempts = 10;

  /** Empty-remote-read attempt count per retrying key. */
  private final Map<String, int[]> mEmptyReadAttempts = new ConcurrentHashMap<>();

  /** Returns the process-wide singleton. */
  @NonNull
  public static PersistentSettingsManager inst() {
    PersistentSettingsManager instance = sInstance;
    if (instance == null) {
      synchronized (PersistentSettingsManager.class) {
        if (sInstance == null) {
          sInstance = new PersistentSettingsManager();
        }
        instance = sInstance;
      }
    }
    return instance;
  }

  private PersistentSettingsManager() {}

  /**
   * Serves a value from the frozen snapshot.
   *
   * @param key The settings key to retrieve
   * @return The snapshot value, or {@link AnimaXSettingValue#empty()} when the key is null,
   *         absent from the file, or the snapshot is not loadable yet (no application
   *         context). Absent keys stay empty for the rest of the process (the snapshot never
   *         changes once loaded).
   */
  public AnimaXSettingValue getValueByKey(String key) {
    if (key == null) {
      return AnimaXSettingValue.empty();
    }
    Map<String, AnimaXSettingValue> snapshot = ensureSnapshotLoaded();
    if (snapshot == null) {
      return AnimaXSettingValue.empty();
    }
    AnimaXSettingValue value = snapshot.get(key);
    return value != null ? value : AnimaXSettingValue.empty();
  }

  /**
   * Persists a value into the settings file, for the next launch. Freezes the current
   * snapshot first (if not frozen yet) and never touches the frozen memory, so the value
   * becomes visible to readers only after a restart. Safe to call repeatedly.
   *
   * @param key The settings key
   * @param value The value to persist
   */
  public void persistValue(@NonNull String key, @NonNull String value) {
    try {
      // Freeze first, write afterwards: keeps the value of this launch untouched.
      ensureSnapshotLoaded();
      Context context = AnimaX.inst().getAppContext();
      if (context == null) {
        AnimaXLog.w(TAG, "Application context not available, value not persisted");
        return;
      }
      context.getSharedPreferences(SETTINGS_SP_NAME, Context.MODE_PRIVATE)
          .edit()
          .putString(key, value)
          .apply();
      AnimaXLog.i(TAG, "Persisted value for next launch, key=" + key + ", value=" + value);
    } catch (Throwable t) {
      AnimaXLog.e(TAG, "Failed to persist value for key " + key + ": " + t);
    }
  }

  /**
   * Persists a remote collection value (stored as a string set) into the settings file, for
   * the next launch. Same freeze-then-write semantics as {@link #persistValue(String, String)}.
   *
   * @param key The settings key
   * @param value The collection value to persist
   */
  public void persistValue(@NonNull String key, @NonNull Collection<String> value) {
    try {
      // Freeze first, write afterwards: keeps the value of this launch untouched.
      ensureSnapshotLoaded();
      Context context = AnimaX.inst().getAppContext();
      if (context == null) {
        AnimaXLog.w(TAG, "Application context not available, value not persisted");
        return;
      }
      context.getSharedPreferences(SETTINGS_SP_NAME, Context.MODE_PRIVATE)
          .edit()
          .putStringSet(key, new HashSet<>(value))
          .apply();
      AnimaXLog.i(
          TAG, "Persisted collection for next launch, key=" + key + ", size=" + value.size());
    } catch (Throwable t) {
      AnimaXLog.e(TAG, "Failed to persist value for key " + key + ": " + t);
    }
  }

  /**
   * Removes a key from the settings file, for the next launch, so that a deleted remote setting
   * does not survive as a stale local value. Freezes the current snapshot first (if not frozen
   * yet) and never touches the frozen memory: readers of the current process keep seeing the
   * removed value. Removing an absent key is a no-op; safe to call repeatedly.
   *
   * <p><b>Prefer {@link #syncSetting} over calling this directly.</b> An empty remote read is
   * ambiguous (fetch pending vs. remotely deleted); syncSetting deletes a key only after its
   * empty-read budget is exhausted, while a direct removeValue bypasses that protection and
   * can wipe the last known good config on a transient fetch miss. Call this directly only
   * with out-of-band certainty that the key was removed on the remote side.
   *
   * @param key The settings key
   */
  public void removeValue(@NonNull String key) {
    try {
      // Freeze first, write afterwards: keeps the value of this launch untouched.
      ensureSnapshotLoaded();
      Context context = AnimaX.inst().getAppContext();
      if (context == null) {
        AnimaXLog.w(TAG, "Application context not available, key not removed");
        return;
      }
      context.getSharedPreferences(SETTINGS_SP_NAME, Context.MODE_PRIVATE)
          .edit()
          .remove(key)
          .apply();
      AnimaXLog.i(TAG, "Removed key for next launch, key=" + key);
    } catch (Throwable t) {
      AnimaXLog.e(TAG, "Failed to remove value for key " + key + ": " + t);
    }
  }

  /**
   * Forces the process-wide snapshot to be loaded now, if it has not been loaded yet. The
   * load happens implicitly on the first read or persist anyway; integrations may call this
   * to freeze the snapshot at a well-defined early point.
   */
  public void preload() {
    ensureSnapshotLoaded();
  }

  /**
   * Reconciles a process-stable setting: the effective value always comes from the frozen
   * snapshot, while the remote value (read through the given {@link IAnimaXSettingService},
   * e.g. the ability's service backed by the integration's settings channel) is persisted
   * into / removed from the file when it disagrees, so the new value takes effect on the next
   * launch.
   *
   * <p>Empty remote reads are ambiguous: they may mean the server deleted the key, or simply
   * that the server response has not arrived yet (early calls in a process often see this).
   * An empty read is retried for up to {@link #maxEmptyReadAttempts} further sync
   * attempts: a non-empty delivery within that budget is persisted normally, and only a
   * budget-exhausted streak of empty reads deletes a locally-persisted value — a transient
   * fetch miss therefore never wipes the last known good config. After the budget the key is
   * given up and costs no further remote reads. Observing a non-empty remote value settles a
   * key immediately. Without a remote service nothing happens.
   *
   * @param remoteService The remote settings channel to compare against; {@code null} skips
   *        the sync entirely (an empty read would otherwise be indistinguishable from a
   *        remote deletion)
   * @param key The settings key to reconcile
   */
  public void syncSetting(@Nullable IAnimaXSettingService remoteService, @NonNull String key) {
    if (mSyncedKeys.contains(key)) {
      // A non-empty remote value was already observed, or the attempts ran out;
      // nothing to do.
      return;
    }
    // No remote service means there is no remote channel at all; an empty read then carries
    // no information (it is not a remote deletion), so leave the file untouched. The key is
    // NOT marked synced: the remote channel may become available later.
    if (remoteService == null) {
      return;
    }
    AnimaXSettingValue remote;
    try {
      remote = remoteService.getValueByKey(key);
    } catch (Throwable t) {
      AnimaXLog.e(TAG, "syncSetting fail for key " + key + ": " + t);
      return;
    }
    AnimaXSettingValue snapshot = getValueByKey(key);
    if (!isEmptySettingValue(remote)) {
      if (!settingValuesEqual(remote, snapshot)) {
        AnimaXLog.i(TAG,
            "SyncSetting: remote value differs, persisting for next launch, key=" + key
                + ", snapshot=" + snapshot + ", remote=" + remote);
        if (remote.isString()) {
          persistValue(key, remote.getStringOrEmpty());
        } else if (remote.isCollection()) {
          persistValue(key, remote.getCollectionOrEmpty());
        }
      } else {
        AnimaXLog.d(
            TAG, "SyncSetting: remote value matches snapshot, key=" + key + ", value=" + remote);
      }
      mEmptyReadAttempts.remove(key);
      mSyncedKeys.add(key);
    } else {
      int attempts = recordEmptyReadAttempt(key);
      if (attempts == 1) {
        AnimaXLog.d(TAG,
            "SyncSetting: remote reads empty (fetch pending or deleted), key=" + key
                + ", localSnapshot=" + snapshot);
      }
      if (attempts >= maxEmptyReadAttempts) {
        // Budget exhausted: the empty reads now look like a real remote deletion rather than a
        // fetch miss, so drop the stale local value. If the server delivers a value later in
        // this process anyway, the next launch picks it up from the reloaded file.
        AnimaXLog.i(TAG,
            "SyncSetting: empty reads exhausted budget, treating as remote"
                + " deletion, key=" + key + ", attempts=" + attempts);
        mEmptyReadAttempts.remove(key);
        mSyncedKeys.add(key);
        if (!isEmptySettingValue(snapshot)) {
          removeValue(key);
        }
      }
    }
  }

  /** An absent/null, empty-string, or empty-collection setting value. */
  private static boolean isEmptySettingValue(@Nullable AnimaXSettingValue value) {
    if (value == null) {
      return true;
    }
    if (value.isString()) {
      return value.getStringOrEmpty().isEmpty();
    }
    if (value.isCollection()) {
      return value.getCollectionOrEmpty().isEmpty();
    }
    return false;
  }

  private static boolean settingValuesEqual(
      @NonNull AnimaXSettingValue remote, @NonNull AnimaXSettingValue snapshot) {
    if (remote.isString() && snapshot.isString()) {
      return remote.getStringOrEmpty().equals(snapshot.getStringOrEmpty());
    }
    if (remote.isCollection() && snapshot.isCollection()) {
      Collection<String> remoteList = remote.getCollectionOrEmpty();
      Collection<String> snapshotList = snapshot.getCollectionOrEmpty();
      return remoteList.size() == snapshotList.size()
          && new HashSet<>(remoteList).containsAll(snapshotList);
    }
    return false;
  }

  /**
   * Counts one more empty remote read for the key and returns the number of attempts so far.
   * Uses only {@code Map#get}/{@code Map#put} guarded by a lock: Java-8 collection additions
   * such as {@code computeIfAbsent}/{@code putIfAbsent} are unavailable on the minSdk (21)
   * when invoked through the {@code Map} interface. Contention is irrelevant — this runs at
   * most a dozen times per key per process.
   */
  private int recordEmptyReadAttempt(String key) {
    synchronized (mEmptyReadAttempts) {
      int[] count = mEmptyReadAttempts.get(key);
      if (count == null) {
        count = new int[1];
        mEmptyReadAttempts.put(key, count);
      }
      count[0] += 1;
      return count[0];
    }
  }

  /**
   * Drops the frozen snapshot and the sync bookkeeping so the next access reloads them. For
   * tests only.
   */
  @VisibleForTesting
  public void resetForTest() {
    synchronized (mSnapshotLock) {
      mSnapshot = null;
    }
    mSyncedKeys.clear();
    mEmptyReadAttempts.clear();
  }

  @Nullable
  private Map<String, AnimaXSettingValue> ensureSnapshotLoaded() {
    Map<String, AnimaXSettingValue> snapshot = mSnapshot;
    if (snapshot != null) {
      return snapshot;
    }
    synchronized (mSnapshotLock) {
      if (mSnapshot == null) {
        Map<String, AnimaXSettingValue> loaded = loadSnapshotFromDisk();
        if (loaded != null) {
          mSnapshot = loaded;
        }
      }
      return mSnapshot;
    }
  }

  /**
   * Reads the whole settings file into the snapshot map.
   *
   * @return The loaded snapshot (an empty map when the file has no usable entries or cannot
   *         be read — both freeze the "loaded" state), or {@code null} when the application
   *         context is not available yet (loading stays pending and is retried later).
   */
  @Nullable
  private static Map<String, AnimaXSettingValue> loadSnapshotFromDisk() {
    Context context = AnimaX.inst().getAppContext();
    if (context == null) {
      AnimaXLog.w(TAG, "Application context not available yet, settings snapshot not loaded");
      return null;
    }
    try {
      SharedPreferences preferences =
          context.getSharedPreferences(SETTINGS_SP_NAME, Context.MODE_PRIVATE);
      Map<String, ?> all = preferences.getAll();
      Map<String, AnimaXSettingValue> snapshot = new ConcurrentHashMap<>(Math.max(1, all.size()));
      for (Map.Entry<String, ?> entry : all.entrySet()) {
        Object value = entry.getValue();
        if (value instanceof String) {
          snapshot.put(entry.getKey(), AnimaXSettingValue.fromString((String) value));
        } else if (value instanceof java.util.Set) {
          snapshot.put(entry.getKey(), toStringCollection((java.util.Set<?>) value));
        }
        // Other SharedPreferences types are not part of the contract and are ignored.
      }
      AnimaXLog.i(TAG, "Settings snapshot loaded, entry count: " + snapshot.size());
      return snapshot;
    } catch (Throwable t) {
      AnimaXLog.e(TAG, "Failed to load settings snapshot: " + t);
      return new ConcurrentHashMap<>(0);
    }
  }

  private static AnimaXSettingValue toStringCollection(java.util.Set<?> set) {
    java.util.ArrayList<String> values = new java.util.ArrayList<>(set.size());
    for (Object item : set) {
      if (item instanceof String) {
        values.add((String) item);
      }
    }
    return AnimaXSettingValue.fromCollection(values);
  }
}
