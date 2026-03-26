// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.service;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RestrictTo;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Registry for managing instance-level services.
 * Each ServiceRegistry instance maintains its own service collection,
 * while also having access to global services through AnimaXServiceCenter.
 *
 * Example usage:
 * <pre>
 * ServiceRegistry registry = new ServiceRegistry(ServiceScope.DEFAULT);
 *
 * // Register a service
 * registry.registerService(IMyService.class, new MyServiceImpl());
 *
 * // Get a service
 * IMyService service = registry.getService(IMyService.class);
 *
 * // Unregister a service
 * registry.unregisterService(IMyService.class);
 * </pre>
 */
@RestrictTo(RestrictTo.Scope.LIBRARY_GROUP)
public class ServiceRegistry {
  private static final String TAG = "ServiceRegistry";

  private final Map<Class<? extends IAnimaXService>, IAnimaXService> mInstanceServices;
  private final ServiceScope mScope;

  /**
   * Creates a new ServiceRegistry instance with specified scope.
   *
   * @param scope The scope for global service lookup
   */
  public ServiceRegistry(ServiceScope scope) {
    mInstanceServices = new ConcurrentHashMap<>();
    mScope = scope;
  }

  /**
   * Gets a service of the specified type. First checks instance services,
   * then falls back to global services if not found.
   *
   * @param serviceClass The class of the service to retrieve
   * @return The service instance if found, null otherwise
   */
  @Nullable
  public <T extends IAnimaXService> T getService(Class<T> serviceClass) {
    // First check instance services
    IAnimaXService service = mInstanceServices.get(serviceClass);
    if (service != null) {
      return serviceClass.cast(service);
    }

    // Fall back to global services
    return AnimaXServiceCenter.inst().getService(mScope, serviceClass);
  }

  /**
   * Registers a service instance.
   *
   * @param clazz The class type to register the service as
   * @param instance The service instance to register
   */
  public <T extends IAnimaXService> void registerService(
      @NonNull Class<T> clazz, @NonNull T instance) {
    mInstanceServices.put(clazz, instance);
  }

  /**
   * Unregisters a service of the specified type.
   *
   * @param clazz The class type of the service to unregister
   * @return void. If the service is not registered, method returns silently
   */
  public <T extends IAnimaXService> void unregisterService(@NonNull Class<T> clazz) {
    mInstanceServices.remove(clazz);
  }

  /**
   * Unregisters all instance services.
   */
  public void unregisterAllServices() {
    mInstanceServices.clear();
  }

  /**
   * Releases resources held by this registry.
   * Should be called when the registry is no longer needed.
   */
  public void release() {
    unregisterAllServices();
  }
}
