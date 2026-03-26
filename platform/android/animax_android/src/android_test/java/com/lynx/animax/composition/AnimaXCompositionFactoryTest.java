// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.composition;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.*;

import androidx.test.platform.app.InstrumentationRegistry;
import com.lynx.animax.service.ServiceScope;
import com.lynx.animax.util.AnimaX;
import junit.framework.TestCase;
import org.junit.Before;
import org.junit.Test;
import org.mockito.ArgumentCaptor;

public class AnimaXCompositionFactoryTest extends TestCase {
  private static final String TEST_JSON =
      "{\"v\":\"5.6.9\",\"fr\":25,\"ip\":0,\"op\":200,\"w\":1000,\"h\":1000}";
  private static final String TEST_URI = "file:///test.json";

  private AnimaXCompositionFactory factory;
  private AnimaXCompositionListener mockListener;
  private ServiceScope testScope;

  @Before
  public void setUp() {
    AnimaX.inst().init(InstrumentationRegistry.getInstrumentation().getTargetContext());
    factory = AnimaXCompositionFactory.inst();
    mockListener = mock(AnimaXCompositionListener.class);
    testScope = ServiceScope.DEFAULT;
  }

  @Test
  public void testSingleton() {
    AnimaXCompositionFactory instance1 = AnimaXCompositionFactory.inst();
    AnimaXCompositionFactory instance2 = AnimaXCompositionFactory.inst();
    assertSame(instance1, instance2);
  }

  @Test
  public void testLoadJsonWithNullListener() {
    factory.loadJson(testScope, TEST_JSON, null);
  }

  @Test
  public void testLoadUriWithNullListener() {
    factory.loadUri(testScope, TEST_URI, null);
  }

  @Test
  public void testLoadJsonWithInvalidJson() {
    factory.loadJson(testScope, "invalid json", mockListener);
    verify(mockListener, timeout(1000).times(1)).onCompositionFailed(any(String.class));
  }

  @Test
  public void testLoadUriWithInvalidUri() {
    factory.loadUri(testScope, "invalid://uri", mockListener);
    verify(mockListener, timeout(1000).times(1)).onCompositionFailed(any(String.class));
  }

  @Test
  public void testLoadJsonSuccess() {
    factory.loadJson(testScope, TEST_JSON, mockListener);

    ArgumentCaptor<AnimaXComposition> compositionCaptor =
        ArgumentCaptor.forClass(AnimaXComposition.class);
    verify(mockListener, timeout(1000).times(1)).onCompositionReady(compositionCaptor.capture());

    AnimaXComposition composition = compositionCaptor.getValue();
    assertNotNull(composition);
    assertEquals(0, composition.getStartFrame());
    assertEquals(199, composition.getEndFrame());
  }

  @Test
  public void testLoadJsonWithEmptyString() {
    factory.loadJson(testScope, "", mockListener);
    verify(mockListener, timeout(1000).times(1)).onCompositionFailed(any(String.class));
  }

  @Test
  public void testLoadJsonWithNullString() {
    factory.loadJson(testScope, null, mockListener);
    verify(mockListener, timeout(1000).times(1)).onCompositionFailed(any(String.class));
  }

  @Test
  public void testLoadJsonWithMalformedJson() {
    String malformedJson = "{\"v\":\"5.6.9\",\"fr\":25,\"ip\":0,\"op\":200,\"w\":1000,\"h\":1000";
    factory.loadJson(testScope, malformedJson, mockListener);
    verify(mockListener, timeout(1000).times(1)).onCompositionFailed(any(String.class));
  }

  @Test
  public void testLoadUriWithEmptyUri() {
    factory.loadUri(testScope, "", mockListener);
    verify(mockListener, timeout(1000).times(1)).onCompositionFailed(any(String.class));
  }

  @Test
  public void testLoadUriWithNullUri() {
    factory.loadUri(testScope, null, mockListener);
    verify(mockListener, timeout(1000).times(1)).onCompositionFailed(any(String.class));
  }

  @Test
  public void testMultipleSimultaneousLoads() {
    factory.loadJson(testScope, TEST_JSON, mockListener);
    factory.loadJson(testScope, TEST_JSON, mockListener);
    factory.loadJson(testScope, TEST_JSON, mockListener);

    verify(mockListener, timeout(3000).times(3)).onCompositionReady(any(AnimaXComposition.class));
  }

  @Test
  public void testLoadJsonWithMixedCallbacks() {
    AnimaXCompositionListener secondListener = mock(AnimaXCompositionListener.class);

    factory.loadJson(testScope, TEST_JSON, mockListener);
    factory.loadJson(testScope, "invalid json", secondListener);

    verify(mockListener, timeout(1000).times(1)).onCompositionReady(any(AnimaXComposition.class));
    verify(secondListener, timeout(1000).times(1)).onCompositionFailed(any(String.class));
  }

  @Test
  public void testLoadComplexJson() {
    String complexJson = "{\"v\":\"5.6.9\",\"fr\":30,\"ip\":10,\"op\":300,\"w\":2000,\"h\":2000,"
        + "\"assets\":[],\"layers\":[],\"markers\":[]}";

    factory.loadJson(testScope, complexJson, mockListener);

    ArgumentCaptor<AnimaXComposition> compositionCaptor =
        ArgumentCaptor.forClass(AnimaXComposition.class);
    verify(mockListener, timeout(1000).times(1)).onCompositionReady(compositionCaptor.capture());

    AnimaXComposition composition = compositionCaptor.getValue();
    assertNotNull(composition);
    assertEquals(10, composition.getStartFrame());
    assertEquals(299, composition.getEndFrame());
  }

  @Test
  public void testMultipleServiceScopes() {
    ServiceScope scope1 = ServiceScope.of("scope1");
    ServiceScope scope2 = ServiceScope.of("scope2");

    AnimaXCompositionListener listener1 = mock(AnimaXCompositionListener.class);
    AnimaXCompositionListener listener2 = mock(AnimaXCompositionListener.class);

    factory.loadJson(scope1, TEST_JSON, listener1);
    factory.loadJson(scope2, TEST_JSON, listener2);

    verify(listener1, timeout(1000).times(1)).onCompositionReady(any(AnimaXComposition.class));
    verify(listener2, timeout(1000).times(1)).onCompositionReady(any(AnimaXComposition.class));
  }

  @Test
  public void testDefaultScopeCompatibility() {
    AnimaXCompositionListener listener = mock(AnimaXCompositionListener.class);

    factory.loadJson(TEST_JSON, listener);

    verify(listener, timeout(1000).times(1)).onCompositionReady(any(AnimaXComposition.class));
  }

  @Test
  public void testReleaseWithMultipleScopes() {
    ServiceScope scope1 = ServiceScope.of("scope1");

    factory.loadJson(scope1, TEST_JSON, mockListener);
    verify(mockListener, timeout(1000).times(1)).onCompositionReady(any(AnimaXComposition.class));

    factory.release();

    AnimaXCompositionListener newListener = mock(AnimaXCompositionListener.class);
    factory.loadJson(scope1, TEST_JSON, newListener);
    verify(mockListener, timeout(1000).times(1)).onCompositionReady(any(AnimaXComposition.class));
  }
}
