// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.service;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import com.lynx.animax.util.AnimaXLog;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Global service center that manages services across different scopes.
 * This is a singleton instance that provides centralized service management for the entire library.
 *
 * Services are organized by scopes.
 *
 * Registration constraints:
 * - Each service type can only have one registered instance per scope
 * - Attempting to register a second instance of the same service type in the same scope will fail
 * - The first registered instance will be preserved when registration conflicts occur
 * - After unregistering a service, a new instance of the same type can be registered
 *
 * Example usage:
 * <pre>
 * // Register a service
 * AnimaXServiceCenter.inst().registerService(IMyService.class, new MyServiceImpl());
 *
 * // Get a service
 * IMyService service = AnimaXServiceCenter.inst().getService(IMyService.class);
 *
 * // Unregister a service
 * AnimaXServiceCenter.inst().unregisterService(IMyService.class);
 * </pre>
 */
public class AnimaXServiceCenter {
  private static final String TAG = "AnimaXServiceCenter";
  private static volatile AnimaXServiceCenter sInstance = null;
  // TODO(aiyongbiao.rick): remove reflective registration after migration
  private static final String[] AUTO_REGISTER_SERVICE_CLASS_NAMES = {
      "com.lynx.animax.service.AnimaXFrescoImageService",
      "com.lynx.animax.monitor.common.AnimaXCommonMonitorService"};

  private final Map<ServiceScope, Map<Class<? extends IAnimaXService>, IAnimaXService>>
      mScopedServices = new ConcurrentHashMap<>();

  private AnimaXServiceCenter() {
    createScope(ServiceScope.DEFAULT);
    registerServicesReflective();
  }

  private void registerServicesReflective() {
    for (String className : AUTO_REGISTER_SERVICE_CLASS_NAMES) {
      try {
        Class<?> clazz = Class.forName(className);
        if (!IAutoRegisterAnimaXService.class.isAssignableFrom(clazz)) {
          AnimaXLog.w(TAG, "Class " + className + " does not implement IAutoRegisterAnimaXService");
          continue;
        }
        IAutoRegisterAnimaXService instance =
            (IAutoRegisterAnimaXService) clazz.getDeclaredConstructor().newInstance();
        registerServiceInner(
            instance.getAutoRegisterServiceScope(), instance.getServiceClass(), instance);
      } catch (Exception e) {
        AnimaXLog.e(TAG, "Failed to auto-register service " + className + ": " + e.toString());
      }
    }
  }

  /**
   * Gets the singleton instance of AnimaXServiceCenter.
   *
   * @return The global AnimaXServiceCenter instance
   */
  public static AnimaXServiceCenter inst() {
    if (sInstance == null) {
      synchronized (AnimaXServiceCenter.class) {
        if (sInstance == null) {
          sInstance = new AnimaXServiceCenter();
        }
      }
    }
    return sInstance;
  }

  /**
   * API for accessing services from DEFAULT scope.
   *
   * @param clazz The class of the service to retrieve
   * @return The service instance if found, null if either:
   *         - The service type is not registered
   *         - The scope is invalid
   *         - The provided class type is null
   */
  @Nullable
  public <T extends IAnimaXService> T getService(Class<T> clazz) {
    ServiceScope scope = ServiceScope.DEFAULT;
    return getService(scope, clazz);
  }

  /**
   * API for accessing services from any scope.
   *
   * @param scope The scope to look up the service in
   * @param clazz The class of the service to retrieve
   * @return The service instance if found, null if either:
   *         - The service type is not registered
   *         - The scope is invalid
   *         - The provided class type is null
   */
  @Nullable
  public <T extends IAnimaXService> T getService(ServiceScope scope, Class<T> clazz) {
    Map<Class<? extends IAnimaXService>, IAnimaXService> services = mScopedServices.get(scope);
    if (services != null && services.containsKey(clazz)) {
      return (T) services.get(clazz);
    } else {
      return null;
    }
  }

  /**
   * API for registering services in any scope.
   *
   * @param scope The scope to register the service in
   * @param clazz The class type to register the service as
   * @param instance The service instance to register
   * @return void. If the service type is already registered, logs error and returns silently
   */
  public <T extends IAnimaXService> void registerService(
      ServiceScope scope, @NonNull Class<T> clazz, @NonNull T instance) {
    registerServiceInner(scope, clazz, instance);
  }

  /**
   * Registers a service in the DEFAULT scope.
   *
   * @param clazz The class type to register the service as
   * @param instance The service instance to register
   * @return void. If the service type is already registered, logs error and returns silently
   */
  public <T extends IAnimaXService> void registerService(
      @NonNull Class<T> clazz, @NonNull T instance) {
    registerService(ServiceScope.DEFAULT, clazz, instance);
  }

  @SuppressWarnings("unchecked")
  private void registerServiceInner(
      ServiceScope scope, Class<? extends IAnimaXService> clazz, IAnimaXService instance) {
    Map<Class<? extends IAnimaXService>, IAnimaXService> services = mScopedServices.get(scope);
    if (services == null) {
      services = createScope(scope);
    }

    boolean containsClass = services.containsKey(clazz);
    if (!containsClass) {
      services.put(clazz, instance);
    }

    AnimaXLog.i(TAG,
        String.format("Register service %s with instance %s in scope %s, result: %s",
            clazz.getSimpleName(), instance, scope, !containsClass));
  }

  /**
   * Unregisters a service from the DEFAULT scope.
   *
   * @param clazz The class type of the service to unregister
   * @return void. If the service is not registered, method returns silently
   */
  public <T extends IAnimaXService> void unregisterService(@NonNull Class<T> clazz) {
    unregisterService(ServiceScope.DEFAULT, clazz);
  }

  /**
   * API for unregistering services from any scope.
   *
   * @param scope The scope to unregister the service from
   * @param clazz The class type of the service to unregister
   */
  public <T extends IAnimaXService> void unregisterService(
      ServiceScope scope, @NonNull Class<T> clazz) {
    Map<Class<? extends IAnimaXService>, IAnimaXService> services = mScopedServices.get(scope);
    if (services != null) {
      services.remove(clazz);
    }
  }

  /**
   * Unregisters all services from the DEFAULT scope.
   */
  public void unregisterAllServices() {
    unregisterAllServices(ServiceScope.DEFAULT);
  }

  /**
   * API for unregistering all services from any scope.
   *
   * @param scope The scope to unregister all services from
   */
  public void unregisterAllServices(ServiceScope scope) {
    Map<Class<? extends IAnimaXService>, IAnimaXService> services = mScopedServices.get(scope);
    if (services != null) {
      services.clear();
    }
  }

  /**
   * Creates a new service scope and adds it to the managed scopes.
   * Each scope maintains its own mapping of service types to service instances.
   *
   * @param scope The scope identifier to create
   * @return A new concurrent map for storing services in this scope
   */
  private Map<Class<? extends IAnimaXService>, IAnimaXService> createScope(ServiceScope scope) {
    Map<Class<? extends IAnimaXService>, IAnimaXService> services = new ConcurrentHashMap<>();
    mScopedServices.put(scope, services);
    return services;
  }
}
