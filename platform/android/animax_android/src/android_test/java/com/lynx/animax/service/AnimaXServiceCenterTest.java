// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

package com.lynx.animax.service;

import static org.junit.Assert.*;

import org.junit.Before;
import org.junit.Test;

public class AnimaXServiceCenterTest {
  private static class IMockAnimaXServiceA implements IAnimaXService {}
  private static class MockAnimaXServiceAImpl1 extends IMockAnimaXServiceA {}
  private static class MockAnimaXServiceAImpl2 extends IMockAnimaXServiceA {}

  private static class IMockAnimaXServiceB implements IAnimaXService {}
  private static class MockAnimaXServiceBImpl1 extends IMockAnimaXServiceB {}

  private AnimaXServiceCenter serviceCenter;
  private IMockAnimaXServiceA mockDefaultService;
  private IMockAnimaXServiceB mockInternalService;

  private static final ServiceScope SCOPE_INTERNAL = ServiceScope.of("internal");

  @Before
  public void setUp() {
    serviceCenter = AnimaXServiceCenter.inst();
    mockDefaultService = new MockAnimaXServiceAImpl1();
    mockInternalService = new MockAnimaXServiceBImpl1();

    // Clear any previously registered services
    serviceCenter.unregisterAllServices(ServiceScope.DEFAULT);
    serviceCenter.unregisterAllServices(SCOPE_INTERNAL);
  }

  @Test
  public void testRegisterAndGetDefaultService() {
    serviceCenter.registerService(IMockAnimaXServiceA.class, mockDefaultService);

    IAnimaXService retrievedService =
        serviceCenter.getService(ServiceScope.DEFAULT, IMockAnimaXServiceA.class);

    assertSame(mockDefaultService, retrievedService);
  }

  @Test
  public void testRegisterAndGetInternalService() {
    serviceCenter.registerService(SCOPE_INTERNAL, IMockAnimaXServiceB.class, mockInternalService);

    IAnimaXService retrievedService =
        serviceCenter.getService(SCOPE_INTERNAL, IMockAnimaXServiceB.class);

    assertSame(mockInternalService, retrievedService);
  }

  @Test
  public void testUnregisterService() {
    serviceCenter.registerService(IMockAnimaXServiceA.class, mockDefaultService);
    serviceCenter.unregisterService(IMockAnimaXServiceA.class);

    IAnimaXService retrievedService =
        serviceCenter.getService(ServiceScope.DEFAULT, IMockAnimaXServiceA.class);

    assertNull(retrievedService);
  }

  @Test
  public void testUnregisterAllServices() {
    serviceCenter.registerService(IMockAnimaXServiceA.class, mockDefaultService);
    serviceCenter.registerService(SCOPE_INTERNAL, IMockAnimaXServiceB.class, mockInternalService);

    serviceCenter.unregisterAllServices(ServiceScope.DEFAULT);
    serviceCenter.unregisterAllServices(SCOPE_INTERNAL);

    assertNull(serviceCenter.getService(ServiceScope.DEFAULT, IMockAnimaXServiceA.class));
    assertNull(serviceCenter.getService(SCOPE_INTERNAL, IMockAnimaXServiceB.class));
  }

  @Test
  public void testGetNonExistentService() {
    IAnimaXService service =
        serviceCenter.getService(ServiceScope.DEFAULT, IMockAnimaXServiceA.class);
    assertNull(service);
  }

  @Test
  public void testRegisterSameServiceTwice() {
    // First registration
    IMockAnimaXServiceA firstService = new MockAnimaXServiceAImpl1();
    serviceCenter.registerService(IMockAnimaXServiceA.class, firstService);

    // Second registration with different implementation of same interface
    IMockAnimaXServiceA secondService = new MockAnimaXServiceAImpl2();
    serviceCenter.registerService(IMockAnimaXServiceA.class, secondService);

    // Get the registered service
    IAnimaXService retrievedService =
        serviceCenter.getService(ServiceScope.DEFAULT, IMockAnimaXServiceA.class);

    // Should still be the first registered instance
    assertSame(firstService, retrievedService);
    // Verify second service was not registered
    assertNotSame(secondService, retrievedService);
  }

  @Test
  public void testRegisterAfterUnregister() {
    // First registration
    IMockAnimaXServiceA firstService = new MockAnimaXServiceAImpl1();
    serviceCenter.registerService(IMockAnimaXServiceA.class, firstService);

    // Unregister
    serviceCenter.unregisterService(IMockAnimaXServiceA.class);

    // Should be able to register again after unregister
    IMockAnimaXServiceA secondService = new MockAnimaXServiceAImpl2();
    serviceCenter.registerService(IMockAnimaXServiceA.class, secondService);

    IAnimaXService retrievedService =
        serviceCenter.getService(ServiceScope.DEFAULT, IMockAnimaXServiceA.class);

    assertSame(secondService, retrievedService);
    assertNotSame(firstService, retrievedService);
  }

  @Test
  public void testRegisterSameServiceTypeInDifferentScopes() {
    // Register in DEFAULT scope
    IMockAnimaXServiceA defaultService = new MockAnimaXServiceAImpl1();
    serviceCenter.registerService(IMockAnimaXServiceA.class, defaultService);

    // Register same service type in INTERNAL scope
    IMockAnimaXServiceA internalService = new MockAnimaXServiceAImpl2();
    serviceCenter.registerService(SCOPE_INTERNAL, IMockAnimaXServiceA.class, internalService);

    // Both services should be retrievable from their respective scopes
    assertSame(
        defaultService, serviceCenter.getService(ServiceScope.DEFAULT, IMockAnimaXServiceA.class));
    assertSame(
        internalService, serviceCenter.getService(SCOPE_INTERNAL, IMockAnimaXServiceA.class));
  }

  @Test
  public void testRegisterDifferentServiceTypes() {
    IMockAnimaXServiceA serviceA = new MockAnimaXServiceAImpl1();
    IMockAnimaXServiceB serviceB = new MockAnimaXServiceBImpl1();

    serviceCenter.registerService(IMockAnimaXServiceA.class, serviceA);
    serviceCenter.registerService(IMockAnimaXServiceB.class, serviceB);

    assertSame(serviceA, serviceCenter.getService(ServiceScope.DEFAULT, IMockAnimaXServiceA.class));
    assertSame(serviceB, serviceCenter.getService(ServiceScope.DEFAULT, IMockAnimaXServiceB.class));
  }
}
