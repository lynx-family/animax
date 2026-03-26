// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.service;

import static org.junit.Assert.*;
import static org.mockito.Mockito.*;

import org.junit.Before;
import org.junit.Test;

public class ServiceRegistryTest {
  private static class IMockAnimaXServiceA implements IAutoRegisterAnimaXService {
    @Override
    public Class<? extends IAnimaXService> getServiceClass() {
      return IMockAnimaXServiceA.class;
    }
  }
  private static class MockAnimaXServiceAImpl1 extends IMockAnimaXServiceA {}
  private static class MockAnimaXServiceAImpl2 extends IMockAnimaXServiceA {}

  private static class IMockAnimaXServiceB implements IAutoRegisterAnimaXService {
    @Override
    public Class<? extends IAnimaXService> getServiceClass() {
      return IMockAnimaXServiceB.class;
    }
  }
  private static class MockAnimaXServiceBImpl1 extends IMockAnimaXServiceB {}

  private ServiceRegistry registry;
  private static final ServiceScope TEST_SCOPE = ServiceScope.DEFAULT;

  @Before
  public void setUp() {
    registry = new ServiceRegistry(TEST_SCOPE);
  }

  @Test
  public void testRegisterAndGetService() {
    IMockAnimaXServiceA serviceImpl = new MockAnimaXServiceAImpl1();
    registry.registerService(IMockAnimaXServiceA.class, serviceImpl);

    IMockAnimaXServiceA retrievedService = registry.getService(IMockAnimaXServiceA.class);

    assertSame(serviceImpl, retrievedService);
  }

  @Test
  public void testUnregisterService() {
    IMockAnimaXServiceA serviceImpl = new MockAnimaXServiceAImpl1();
    registry.registerService(IMockAnimaXServiceA.class, serviceImpl);
    registry.unregisterService(IMockAnimaXServiceA.class);

    IMockAnimaXServiceA retrievedService = registry.getService(IMockAnimaXServiceA.class);

    assertNull(retrievedService);
  }

  @Test
  public void testUnregisterAllServices() {
    IMockAnimaXServiceA serviceA = new MockAnimaXServiceAImpl1();
    IMockAnimaXServiceB serviceB = new MockAnimaXServiceBImpl1();

    registry.registerService(IMockAnimaXServiceA.class, serviceA);
    registry.registerService(IMockAnimaXServiceB.class, serviceB);
    registry.unregisterAllServices();

    assertNull(registry.getService(IMockAnimaXServiceA.class));
    assertNull(registry.getService(IMockAnimaXServiceB.class));
  }

  @Test
  public void testRelease() {
    IMockAnimaXServiceA serviceImpl = new MockAnimaXServiceAImpl1();
    registry.registerService(IMockAnimaXServiceA.class, serviceImpl);
    registry.release();

    assertNull(registry.getService(IMockAnimaXServiceA.class));
  }

  @Test
  public void testGetServiceFallbackToGlobal() {
    // Given: Service is not registered locally but exists in global scope
    IMockAnimaXServiceA serviceImpl = new MockAnimaXServiceAImpl1();
    AnimaXServiceCenter.inst().registerService(TEST_SCOPE, IMockAnimaXServiceA.class, serviceImpl);

    // When: Getting service from registry
    IMockAnimaXServiceA retrievedService = registry.getService(IMockAnimaXServiceA.class);

    // Then: Should get service from global scope
    assertSame(serviceImpl, retrievedService);

    // Cleanup
    AnimaXServiceCenter.inst().unregisterService(TEST_SCOPE, IMockAnimaXServiceA.class);
  }

  @Test
  public void testLocalServicePrecedenceOverGlobal() {
    // Given: Service exists both locally and globally
    IMockAnimaXServiceA localService = new MockAnimaXServiceAImpl1();
    IMockAnimaXServiceA globalService = new MockAnimaXServiceAImpl2();

    AnimaXServiceCenter.inst().registerService(
        TEST_SCOPE, IMockAnimaXServiceA.class, globalService);
    registry.registerService(IMockAnimaXServiceA.class, localService);

    // When: Getting service
    IMockAnimaXServiceA retrievedService = registry.getService(IMockAnimaXServiceA.class);

    // Then: Should get local service
    assertSame(localService, retrievedService);
    assertNotSame(globalService, retrievedService);

    // Cleanup
    AnimaXServiceCenter.inst().unregisterService(TEST_SCOPE, IMockAnimaXServiceA.class);
  }

  @Test
  public void testRegisterSameServiceTwice() {
    // First registration
    IMockAnimaXServiceA firstImpl = new MockAnimaXServiceAImpl1();
    registry.registerService(IMockAnimaXServiceA.class, firstImpl);

    // Second registration with different instance of same type
    IMockAnimaXServiceA secondImpl = new MockAnimaXServiceAImpl2();
    registry.registerService(IMockAnimaXServiceA.class, secondImpl);

    // Get the registered service
    IMockAnimaXServiceA retrievedService = registry.getService(IMockAnimaXServiceA.class);

    assertSame(secondImpl, retrievedService);
    assertNotSame(firstImpl, retrievedService);
  }

  @Test
  public void testRegisterAfterUnregister() {
    // First registration
    IMockAnimaXServiceA firstImpl = new MockAnimaXServiceAImpl1();
    registry.registerService(IMockAnimaXServiceA.class, firstImpl);

    // Unregister
    registry.unregisterService(IMockAnimaXServiceA.class);

    // Should be able to register again after unregister
    IMockAnimaXServiceA secondImpl = new MockAnimaXServiceAImpl2();
    registry.registerService(IMockAnimaXServiceA.class, secondImpl);

    IMockAnimaXServiceA retrievedService = registry.getService(IMockAnimaXServiceA.class);

    assertSame(secondImpl, retrievedService);
  }
}
