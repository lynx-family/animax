// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.property;

import static org.mockito.Mockito.mock;

import android.content.Context;
import android.graphics.SurfaceTexture;
import androidx.test.platform.app.InstrumentationRegistry;
import com.lynx.animax.ability.BaseAbility;
import com.lynx.animax.ability.NativeAbility;
import com.lynx.animax.listener.AnimaXErrorParam;
import com.lynx.animax.listener.AnimaXParam;
import com.lynx.animax.listener.AnimationListenerAdapter;
import com.lynx.animax.listener.IAnimationListener;
import com.lynx.animax.ui.AnimaXContext;
import com.lynx.animax.ui.AnimaXView;
import com.lynx.animax.util.DeviceUtil;

public class PropertyTestUtil {
  private static final String DYNAMIC_PROPERTY_JSON =
      "{ \"v\": \"4.8.0\", \"fr\": 60, \"ip\": 0, \"op\": 300, \"w\": 800, \"h\": 1000, \"nm\": "
      + "\"main 2\", \"ddd\": 1, \"assets\": [ { \"id\": \"image_0\", \"w\": 126, \"h\": 92, "
      + "\"u\": \"\", \"p\": \"\", \"e\": 1 }, { \"id\": \"comp_0\", \"nm\": \"comp\", \"fr\": 60, "
      + "\"layers\": [ { \"ddd\": 0, \"ind\": 1, \"ty\": 4, \"nm\": \"repeater\", \"sr\": 1, "
      + "\"ks\": { \"o\": { \"a\": 0, \"k\": 100, \"ix\": 11 }, \"r\": { \"a\": 0, \"k\": 0, "
      + "\"ix\": 10 }, \"p\": { \"a\": 0, \"k\": [400, 500, 0], \"ix\": 2, \"l\": 2 }, \"a\": { "
      + "\"a\": 0, \"k\": [0, 0, 0], \"ix\": 1, \"l\": 2 }, \"s\": { \"a\": 0, \"k\": [100, 100, "
      + "100], \"ix\": 6, \"l\": 2 } }, \"ao\": 0, \"ef\": [ { \"ty\": 25, \"nm\": \"shadow\", "
      + "\"np\": 8, \"ix\": 1, \"en\": 1, \"ef\": [ { \"ty\": 2, \"nm\": \"shadowcolor\", \"mn\": "
      + "\"ADBE Drop Shadow-0001\", \"ix\": 1, \"v\": { \"a\": 0, \"k\": [0, 0, 0, 1], \"ix\": 1 } "
      + "}, { \"ty\": 0, \"nm\": \"shadowopacity\", \"mn\": \"ADBE Drop Shadow-0002\", \"ix\": 2, "
      + "\"v\": { \"a\": 0, \"k\": 127.5, \"ix\": 2 } }, { \"ty\": 0, \"nm\": \"direction\", "
      + "\"mn\": \"ADBE Drop Shadow-0003\", \"ix\": 3, \"v\": { \"a\": 0, \"k\": 135, \"ix\": 3 } "
      + "}, { \"ty\": 0, \"nm\": \"distance\", \"mn\": \"ADBE Drop Shadow-0004\", \"ix\": 4, "
      + "\"v\": { \"a\": 0, \"k\": 40, \"ix\": 4 } }, { \"ty\": 0, \"nm\": \"shadowsoft\", \"mn\": "
      + "\"ADBE Drop Shadow-0005\", \"ix\": 5, \"v\": { \"a\": 0, \"k\": 0, \"ix\": 5 } }, { "
      + "\"ty\": 7, \"nm\": \"onlyshadow\", \"mn\": \"ADBE Drop Shadow-0006\", \"ix\": 6, \"v\": { "
      + "\"a\": 0, \"k\": 0, \"ix\": 6 } } ] } ], \"shapes\": [ { \"ty\": \"gr\", \"it\": [ { "
      + "\"ty\": \"sr\", \"sy\": 2, \"d\": 1, \"pt\": { \"a\": 0, \"k\": 5, \"ix\": 3 }, \"p\": { "
      + "\"a\": 0, \"k\": [0, 0], \"ix\": 4 }, \"r\": { \"a\": 0, \"k\": 113.708, \"ix\": 5 }, "
      + "\"or\": { \"a\": 0, \"k\": 90.138, \"ix\": 7 }, \"os\": { \"a\": 0, \"k\": 0, \"ix\": 9 "
      + "}, \"ix\": 1, \"nm\": \"path\", \"hd\": false }, { \"ty\": \"st\", \"c\": { \"a\": 0, "
      + "\"k\": [1, 1, 1, 1], \"ix\": 3 }, \"o\": { \"a\": 0, \"k\": 100, \"ix\": 4 }, \"w\": { "
      + "\"a\": 0, \"k\": 2, \"ix\": 5 }, \"lc\": 1, \"lj\": 1, \"ml\": 4, \"bm\": 0, \"nm\": "
      + "\"stroke\", \"hd\": false }, { \"ty\": \"fl\", \"c\": { \"a\": 0, \"k\": [0, "
      + "0.379036694765, 0.994913995266, 1], \"ix\": 4 }, \"o\": { \"a\": 0, \"k\": 100, \"ix\": 5 "
      + "}, \"r\": 1, \"bm\": 0, \"nm\": \"fill\", \"hd\": false }, { \"ty\": \"rp\", \"c\": { "
      + "\"a\": 0, \"k\": 3, \"ix\": 1 }, \"o\": { \"a\": 0, \"k\": 0, \"ix\": 2 }, \"m\": 1, "
      + "\"ix\": 4, \"tr\": { \"ty\": \"tr\", \"p\": { \"a\": 0, \"k\": [100, 0], \"ix\": 2 }, "
      + "\"a\": { \"a\": 0, \"k\": [0, 0], \"ix\": 1 }, \"s\": { \"a\": 0, \"k\": [100, 100], "
      + "\"ix\": 3 }, \"r\": { \"a\": 0, \"k\": 0, \"ix\": 4 }, \"so\": { \"a\": 0, \"k\": 100, "
      + "\"ix\": 5 }, \"eo\": { \"a\": 0, \"k\": 100, \"ix\": 6 }, \"nm\": \"transform\" }, "
      + "\"nm\": \"repeater\", \"hd\": false }, { \"ty\": \"tr\", \"p\": { \"a\": 0, \"k\": "
      + "[-230.812, -69.773], \"ix\": 2 }, \"a\": { \"a\": 0, \"k\": [0, 0], \"ix\": 1 }, \"s\": { "
      + "\"a\": 0, \"k\": [100, 100], \"ix\": 3 }, \"r\": { \"a\": 0, \"k\": 0, \"ix\": 6 }, "
      + "\"o\": { \"a\": 0, \"k\": 100, \"ix\": 7 }, \"sk\": { \"a\": 0, \"k\": 0, \"ix\": 4 }, "
      + "\"sa\": { \"a\": 0, \"k\": 0, \"ix\": 5 }, \"nm\": \"transform\" } ], \"nm\": "
      + "\"polystarcontent\", \"np\": 4, \"cix\": 2, \"bm\": 0, \"ix\": 1, \"hd\": false } ], "
      + "\"ip\": 0, \"op\": 300, \"st\": 0, \"ct\": 1, \"bm\": 0 }, { \"ddd\": 0, \"ind\": 2, "
      + "\"ty\": 4, \"nm\": \"ellipse\", \"sr\": 1, \"ks\": { \"o\": { \"a\": 0, \"k\": 100, "
      + "\"ix\": 11 }, \"r\": { \"a\": 0, \"k\": 0, \"ix\": 10 }, \"p\": { \"a\": 0, \"k\": [392, "
      + "500, 0], \"ix\": 2, \"l\": 2 }, \"a\": { \"a\": 0, \"k\": [0, 0, 0], \"ix\": 1, \"l\": 2 "
      + "}, \"s\": { \"a\": 0, \"k\": [100, 100, 100], \"ix\": 6, \"l\": 2 } }, \"ao\": 0, "
      + "\"shapes\": [ { \"ty\": \"gr\", \"it\": [ { \"d\": 1, \"ty\": \"el\", \"s\": { \"a\": 0, "
      + "\"k\": [154.984, 136.383], \"ix\": 2 }, \"p\": { \"a\": 0, \"k\": [0, 0], \"ix\": 3 }, "
      + "\"nm\": \"path\", \"hd\": false }, { \"ty\": \"st\", \"c\": { \"a\": 0, \"k\": [1, 1, 1, "
      + "1], \"ix\": 3 }, \"o\": { \"a\": 0, \"k\": 100, \"ix\": 4 }, \"w\": { \"a\": 0, \"k\": 2, "
      + "\"ix\": 5 }, \"lc\": 1, \"lj\": 1, \"ml\": 4, \"bm\": 0, \"nm\": \"stroke\", \"hd\": "
      + "false }, { \"ty\": \"fl\", \"c\": { \"a\": 0, \"k\": [0.089341133833, 0.994913995266, 0, "
      + "1], \"ix\": 4 }, \"o\": { \"a\": 0, \"k\": 100, \"ix\": 5 }, \"r\": 1, \"bm\": 0, \"nm\": "
      + "\"fill\", \"hd\": false }, { \"ty\": \"tr\", \"p\": { \"a\": 0, \"k\": [270.367, "
      + "-325.402], \"ix\": 2 }, \"a\": { \"a\": 0, \"k\": [0, 0], \"ix\": 1 }, \"s\": { \"a\": 0, "
      + "\"k\": [100, 100], \"ix\": 3 }, \"r\": { \"a\": 0, \"k\": 0, \"ix\": 6 }, \"o\": { \"a\": "
      + "0, \"k\": 100, \"ix\": 7 }, \"sk\": { \"a\": 0, \"k\": 0, \"ix\": 4 }, \"sa\": { \"a\": "
      + "0, \"k\": 0, \"ix\": 5 }, \"nm\": \"transform\" } ], \"nm\": \"ellipsecontent\", \"np\": "
      + "3, \"cix\": 2, \"bm\": 0, \"ix\": 1, \"hd\": false } ], \"ip\": 0, \"op\": 300, \"st\": "
      + "0, \"ct\": 1, \"bm\": 0 }, { \"ddd\": 0, \"ind\": 3, \"ty\": 0, \"nm\": \"comp3\", "
      + "\"refId\": \"comp_1\", \"sr\": 1, \"ks\": { \"o\": { \"a\": 0, \"k\": 100, \"ix\": 11 }, "
      + "\"r\": { \"a\": 0, \"k\": 0, \"ix\": 10 }, \"p\": { \"a\": 0, \"k\": [400, 500, 0], "
      + "\"ix\": 2, \"l\": 2 }, \"a\": { \"a\": 0, \"k\": [400, 500, 0], \"ix\": 1, \"l\": 2 }, "
      + "\"s\": { \"a\": 0, \"k\": [100, 100, 100], \"ix\": 6, \"l\": 2 } }, \"ao\": 0, \"w\": "
      + "800, \"h\": 1000, \"ip\": 0, \"op\": 300, \"st\": 0, \"bm\": 0 } ] }, { \"id\": "
      + "\"comp_1\", \"nm\": \"comp3\", \"fr\": 60, \"layers\": [ { \"ddd\": 0, \"ind\": 1, "
      + "\"ty\": 4, \"nm\": \"round\", \"sr\": 1, \"ks\": { \"o\": { \"a\": 0, \"k\": 100, \"ix\": "
      + "11 }, \"r\": { \"a\": 0, \"k\": 0, \"ix\": 10 }, \"p\": { \"a\": 0, \"k\": [366, 492, 0], "
      + "\"ix\": 2, \"l\": 2 }, \"a\": { \"a\": 0, \"k\": [0, 0, 0], \"ix\": 1, \"l\": 2 }, \"s\": "
      + "{ \"a\": 0, \"k\": [100, 100, 100], \"ix\": 6, \"l\": 2 } }, \"ao\": 0, \"shapes\": [ { "
      + "\"ty\": \"gr\", \"it\": [ { \"ty\": \"rc\", \"d\": 1, \"s\": { \"a\": 0, \"k\": [200.398, "
      + "102.406], \"ix\": 2 }, \"p\": { \"a\": 0, \"k\": [0, 0], \"ix\": 3 }, \"r\": { \"a\": 0, "
      + "\"k\": 20, \"ix\": 4 }, \"nm\": \"path\", \"hd\": false }, { \"ty\": \"st\", \"c\": { "
      + "\"a\": 0, \"k\": [1, 1, 1, 1], \"ix\": 3 }, \"o\": { \"a\": 0, \"k\": 100, \"ix\": 4 }, "
      + "\"w\": { \"a\": 0, \"k\": 2, \"ix\": 5 }, \"lc\": 1, \"lj\": 1, \"ml\": 4, \"bm\": 0, "
      + "\"nm\": \"stroke\", \"hd\": false }, { \"ty\": \"fl\", \"c\": { \"a\": 0, \"k\": "
      + "[0.994913995266, 0, 0.554883897305, 1], \"ix\": 4 }, \"o\": { \"a\": 0, \"k\": 100, "
      + "\"ix\": 5 }, \"r\": 1, \"bm\": 0, \"nm\": \"fill\", \"hd\": false }, { \"ty\": \"tr\", "
      + "\"p\": { \"a\": 0, \"k\": [28.199, -316.797], \"ix\": 2 }, \"a\": { \"a\": 0, \"k\": [0, "
      + "0], \"ix\": 1 }, \"s\": { \"a\": 0, \"k\": [100, 100], \"ix\": 3 }, \"r\": { \"a\": 0, "
      + "\"k\": 0, \"ix\": 6 }, \"o\": { \"a\": 0, \"k\": 100, \"ix\": 7 }, \"sk\": { \"a\": 0, "
      + "\"k\": 0, \"ix\": 4 }, \"sa\": { \"a\": 0, \"k\": 0, \"ix\": 5 }, \"nm\": \"transform\" } "
      + "], \"nm\": \"rectcontent\", \"np\": 3, \"cix\": 2, \"bm\": 0, \"ix\": 1, \"hd\": false } "
      + "], \"ip\": 0, \"op\": 300, \"st\": 0, \"ct\": 1, \"bm\": 0 }, { \"ddd\": 1, \"ind\": 2, "
      + "\"ty\": 4, \"nm\": \"polystar\", \"sr\": 1, \"ks\": { \"o\": { \"a\": 0, \"k\": 100, "
      + "\"ix\": 11 }, \"rx\": { \"a\": 0, \"k\": 0, \"ix\": 8 }, \"ry\": { \"a\": 0, \"k\": 33, "
      + "\"ix\": 9 }, \"rz\": { \"a\": 0, \"k\": 0, \"ix\": 10 }, \"or\": { \"a\": 0, \"k\": [0, "
      + "0, 0], \"ix\": 7 }, \"p\": { \"a\": 0, \"k\": [400, 500, 0], \"ix\": 2 }, \"a\": { \"a\": "
      + "0, \"k\": [0, 0, 0], \"ix\": 1 }, \"s\": { \"a\": 0, \"k\": [100, 100, 100], \"ix\": 6 } "
      + "}, \"ao\": 0, \"shapes\": [ { \"ty\": \"gr\", \"it\": [ { \"ty\": \"sr\", \"sy\": 1, "
      + "\"d\": 1, \"pt\": { \"a\": 0, \"k\": 5, \"ix\": 3 }, \"p\": { \"a\": 0, \"k\": [0, 0], "
      + "\"ix\": 4 }, \"r\": { \"a\": 0, \"k\": 143.131, \"ix\": 5 }, \"ir\": { \"a\": 0, \"k\": "
      + "45.385, \"ix\": 6 }, \"is\": { \"a\": 0, \"k\": 0, \"ix\": 8 }, \"or\": { \"a\": 0, "
      + "\"k\": 90.77, \"ix\": 7 }, \"os\": { \"a\": 0, \"k\": 0, \"ix\": 9 }, \"ix\": 1, \"nm\": "
      + "\"path\", \"hd\": false }, { \"ty\": \"st\", \"c\": { \"a\": 0, \"k\": [1, 1, 1, 1], "
      + "\"ix\": 3 }, \"o\": { \"a\": 0, \"k\": 100, \"ix\": 4 }, \"w\": { \"a\": 0, \"k\": 2, "
      + "\"ix\": 5 }, \"lc\": 1, \"lj\": 1, \"ml\": 4, \"bm\": 0, \"nm\": \"stroke\", \"hd\": "
      + "false }, { \"ty\": \"fl\", \"c\": { \"a\": 0, \"k\": [0, 0.695433971929, 0.994913976333, "
      + "1], \"ix\": 4 }, \"o\": { \"a\": 0, \"k\": 100, \"ix\": 5 }, \"r\": 1, \"bm\": 0, \"nm\": "
      + "\"fill\", \"hd\": false }, { \"ty\": \"tr\", \"p\": { \"a\": 0, \"k\": [-250.945, "
      + "-320.727], \"ix\": 2 }, \"a\": { \"a\": 0, \"k\": [0, 0], \"ix\": 1 }, \"s\": { \"a\": 0, "
      + "\"k\": [100, 100], \"ix\": 3 }, \"r\": { \"a\": 0, \"k\": 0, \"ix\": 6 }, \"o\": { \"a\": "
      + "0, \"k\": 100, \"ix\": 7 }, \"sk\": { \"a\": 0, \"k\": 0, \"ix\": 4 }, \"sa\": { \"a\": "
      + "0, \"k\": 0, \"ix\": 5 }, \"nm\": \"transform\" } ], \"nm\": \"polystarcontent\", \"np\": "
      + "3, \"cix\": 2, \"bm\": 0, \"ix\": 1, \"hd\": false } ], \"ip\": 0, \"op\": 300, \"st\": "
      + "0, \"ct\": 1, \"bm\": 0 }, { \"ddd\": 0, \"ind\": 3, \"ty\": 1, \"nm\": \"solid\", "
      + "\"sr\": 1, \"ks\": { \"o\": { \"a\": 0, \"k\": 100, \"ix\": 11 }, \"r\": { \"a\": 0, "
      + "\"k\": 0, \"ix\": 10 }, \"p\": { \"a\": 0, \"k\": [400, 500, 0], \"ix\": 2, \"l\": 2 }, "
      + "\"a\": { \"a\": 0, \"k\": [400, 500, 0], \"ix\": 1, \"l\": 2 }, \"s\": { \"a\": 0, \"k\": "
      + "[100, 100, 100], \"ix\": 6, \"l\": 2 } }, \"ao\": 0, \"sw\": 800, \"sh\": 1000, \"sc\": "
      + "\"#e0f86c\", \"ip\": 0, \"op\": 300, \"st\": 0, \"bm\": 0 } ] } ], \"layers\": [ { "
      + "\"ddd\": 0, \"ind\": 1, \"ty\": 2, \"nm\": \"image\", \"refId\": \"image_0\", \"sr\": 1, "
      + "\"ks\": { \"o\": { \"a\": 0, \"k\": 100, \"ix\": 11 }, \"r\": { \"a\": 0, \"k\": 0, "
      + "\"ix\": 10 }, \"p\": { \"a\": 0, \"k\": [158, 726, 0], \"ix\": 2, \"l\": 2 }, \"a\": { "
      + "\"a\": 0, \"k\": [63, 46, 0], \"ix\": 1, \"l\": 2 }, \"s\": { \"a\": 0, \"k\": [100, 100, "
      + "100], \"ix\": 6, \"l\": 2 } }, \"ao\": 0, \"ip\": 0, \"op\": 300, \"st\": 0, \"bm\": 0 }, "
      + "{ \"ddd\": 0, \"ind\": 3, \"ty\": 4, \"nm\": \"matte\", \"td\": 1, \"sr\": 1, \"ks\": { "
      + "\"o\": { \"a\": 0, \"k\": 100, \"ix\": 11 }, \"r\": { \"a\": 0, \"k\": 0, \"ix\": 10 }, "
      + "\"p\": { \"a\": 0, \"k\": [426, 500, 0], \"ix\": 2, \"l\": 2 }, \"a\": { \"a\": 0, \"k\": "
      + "[0, 0, 0], \"ix\": 1, \"l\": 2 }, \"s\": { \"a\": 0, \"k\": [100, 100, 100], \"ix\": 6, "
      + "\"l\": 2 } }, \"ao\": 0, \"shapes\": [ { \"ty\": \"gr\", \"it\": [ { \"ty\": \"rc\", "
      + "\"d\": 1, \"s\": { \"a\": 0, \"k\": [161.102, 88.75], \"ix\": 2 }, \"p\": { \"a\": 0, "
      + "\"k\": [0, 0], \"ix\": 3 }, \"r\": { \"a\": 0, \"k\": 0, \"ix\": 4 }, \"nm\": \"rect\", "
      + "\"hd\": false }, { \"ty\": \"st\", \"c\": { \"a\": 0, \"k\": [1, 1, 1, 1], \"ix\": 3 }, "
      + "\"o\": { \"a\": 0, \"k\": 100, \"ix\": 4 }, \"w\": { \"a\": 0, \"k\": 2, \"ix\": 5 }, "
      + "\"lc\": 1, \"lj\": 1, \"ml\": 4, \"bm\": 0, \"nm\": \"stroke\", \"hd\": false }, { "
      + "\"ty\": \"fl\", \"c\": { \"a\": 0, \"k\": [0, 0.695433971929, 0.994913976333, 1], \"ix\": "
      + "4 }, \"o\": { \"a\": 0, \"k\": 100, \"ix\": 5 }, \"r\": 1, \"bm\": 0, \"nm\": \"fill\", "
      + "\"hd\": false }, { \"ty\": \"tr\", \"p\": { \"a\": 0, \"k\": [180.551, -185.625], \"ix\": "
      + "2 }, \"a\": { \"a\": 0, \"k\": [0, 0], \"ix\": 1 }, \"s\": { \"a\": 0, \"k\": [100, 100], "
      + "\"ix\": 3 }, \"r\": { \"a\": 0, \"k\": 0, \"ix\": 6 }, \"o\": { \"a\": 0, \"k\": 100, "
      + "\"ix\": 7 }, \"sk\": { \"a\": 0, \"k\": 0, \"ix\": 4 }, \"sa\": { \"a\": 0, \"k\": 0, "
      + "\"ix\": 5 }, \"nm\": \"transform\" } ], \"nm\": \"rectcontent\", \"np\": 3, \"cix\": 2, "
      + "\"bm\": 0, \"ix\": 1, \"hd\": false } ], \"ip\": 0, \"op\": 300, \"st\": 0, \"ct\": 1, "
      + "\"bm\": 0 }, { \"ddd\": 0, \"ind\": 4, \"ty\": 4, \"nm\": \"ellipse\", \"tt\": 1, \"tp\": "
      + "3, \"sr\": 1, \"ks\": { \"o\": { \"a\": 0, \"k\": 100, \"ix\": 11 }, \"r\": { \"a\": 0, "
      + "\"k\": 0, \"ix\": 10 }, \"p\": { \"a\": 0, \"k\": [442, 636, 0], \"ix\": 2, \"l\": 2 }, "
      + "\"a\": { \"a\": 0, \"k\": [0, 0, 0], \"ix\": 1, \"l\": 2 }, \"s\": { \"a\": 0, \"k\": "
      + "[100, 100, 100], \"ix\": 6, \"l\": 2 } }, \"ao\": 0, \"shapes\": [ { \"ty\": \"gr\", "
      + "\"it\": [ { \"d\": 1, \"ty\": \"el\", \"s\": { \"a\": 0, \"k\": [154.984, 136.383], "
      + "\"ix\": 2 }, \"p\": { \"a\": 0, \"k\": [0, 0], \"ix\": 3 }, \"nm\": \"ellipath\", \"hd\": "
      + "false }, { \"ty\": \"st\", \"c\": { \"a\": 0, \"k\": [1, 1, 1, 1], \"ix\": 3 }, \"o\": { "
      + "\"a\": 0, \"k\": 100, \"ix\": 4 }, \"w\": { \"a\": 0, \"k\": 2, \"ix\": 5 }, \"lc\": 1, "
      + "\"lj\": 1, \"ml\": 4, \"bm\": 0, \"nm\": \"stroke\", \"hd\": false }, { \"ty\": \"fl\", "
      + "\"c\": { \"a\": 0, \"k\": [0.738961279392, 0, 0.994913995266, 1], \"ix\": 4 }, \"o\": { "
      + "\"a\": 0, \"k\": 100, \"ix\": 5 }, \"r\": 1, \"bm\": 0, \"nm\": \"fill\", \"hd\": false "
      + "}, { \"ty\": \"tr\", \"p\": { \"a\": 0, \"k\": [270.367, -323.402], \"ix\": 2 }, \"a\": { "
      + "\"a\": 0, \"k\": [0, 0], \"ix\": 1 }, \"s\": { \"a\": 0, \"k\": [100, 100], \"ix\": 3 }, "
      + "\"r\": { \"a\": 0, \"k\": 0, \"ix\": 6 }, \"o\": { \"a\": 0, \"k\": 100, \"ix\": 7 }, "
      + "\"sk\": { \"a\": 0, \"k\": 0, \"ix\": 4 }, \"sa\": { \"a\": 0, \"k\": 0, \"ix\": 5 }, "
      + "\"nm\": \"transform\" } ], \"nm\": \"ellipsecontent\", \"np\": 3, \"cix\": 2, \"bm\": 0, "
      + "\"ix\": 1, \"hd\": false } ], \"ip\": 0, \"op\": 300, \"st\": 0, \"ct\": 1, \"bm\": 0 }, "
      + "{ \"ddd\": 0, \"ind\": 5, \"ty\": 4, \"nm\": \"rect\", \"sr\": 1, \"ks\": { \"o\": { "
      + "\"a\": 0, \"k\": 100, \"ix\": 11 }, \"r\": { \"a\": 0, \"k\": 0, \"ix\": 10 }, \"p\": { "
      + "\"a\": 0, \"k\": [388, 332, 0], \"ix\": 2, \"l\": 2 }, \"a\": { \"a\": 0, \"k\": [0, 0, "
      + "0], \"ix\": 1, \"l\": 2 }, \"s\": { \"a\": 0, \"k\": [100, 100, 100], \"ix\": 6, \"l\": 2 "
      + "} }, \"ao\": 0, \"ef\": [ { \"ty\": 29, \"nm\": \"gaussianblur\", \"np\": 5, \"ix\": 1, "
      + "\"en\": 1, \"ef\": [ { \"ty\": 0, \"nm\": \"blurlevel\", \"ix\": 1, \"v\": { \"a\": 0, "
      + "\"k\": 15.6, \"ix\": 1 } }, { \"ty\": 7, \"nm\": \"blurdis\", \"ix\": 2, \"v\": { \"a\": "
      + "0, \"k\": 1, \"ix\": 2 } }, { \"ty\": 7, \"nm\": \"blurcorner\", \"ix\": 3, \"v\": { "
      + "\"a\": 0, \"k\": 1, \"ix\": 3 } } ] } ], \"shapes\": [ { \"ty\": \"gr\", \"it\": [ { "
      + "\"ty\": \"rc\", \"d\": 1, \"s\": { \"a\": 0, \"k\": [347.984, 120.336], \"ix\": 2 }, "
      + "\"p\": { \"a\": 0, \"k\": [0, 0], \"ix\": 3 }, \"r\": { \"a\": 0, \"k\": 0, \"ix\": 4 }, "
      + "\"nm\": \"path\", \"hd\": false }, { \"ty\": \"st\", \"c\": { \"a\": 0, \"k\": [1, 1, 1, "
      + "1], \"ix\": 3 }, \"o\": { \"a\": 0, \"k\": 100, \"ix\": 4 }, \"w\": { \"a\": 0, \"k\": 2, "
      + "\"ix\": 5 }, \"lc\": 1, \"lj\": 1, \"ml\": 4, \"bm\": 0, \"nm\": \"stroke\", \"hd\": "
      + "false }, { \"ty\": \"tr\", \"p\": { \"a\": 0, \"k\": [183.992, 348.168], \"ix\": 2 }, "
      + "\"a\": { \"a\": 0, \"k\": [0, 0], \"ix\": 1 }, \"s\": { \"a\": 0, \"k\": [100, 100], "
      + "\"ix\": 3 }, \"r\": { \"a\": 0, \"k\": 0, \"ix\": 6 }, \"o\": { \"a\": 0, \"k\": 100, "
      + "\"ix\": 7 }, \"sk\": { \"a\": 0, \"k\": 0, \"ix\": 4 }, \"sa\": { \"a\": 0, \"k\": 0, "
      + "\"ix\": 5 }, \"nm\": \"transform\" } ], \"nm\": \"rectcontent\", \"np\": 2, \"cix\": 2, "
      + "\"bm\": 0, \"ix\": 1, \"hd\": false }, { \"ty\": \"gf\", \"o\": { \"a\": 0, \"k\": 100, "
      + "\"ix\": 10 }, \"r\": 1, \"bm\": 0, \"g\": { \"p\": 3, \"k\": { \"a\": 0, \"k\": [0.483, "
      + "0.916, 0.044, 0.044, 0.752, 0.493, 0.331, 0.501, 1, 0.07, 0.618, 0.957], \"ix\": 9 } }, "
      + "\"s\": { \"a\": 0, \"k\": [0, 0], \"ix\": 5 }, \"e\": { \"a\": 0, \"k\": [306, 0], "
      + "\"ix\": 6 }, \"t\": 1, \"nm\": \"gradfill\", \"hd\": false } ], \"ip\": 0, \"op\": 300, "
      + "\"st\": 0, \"ct\": 1, \"bm\": 0 }, { \"ddd\": 0, \"ind\": 6, \"ty\": 0, \"nm\": "
      + "\"comp1\", \"refId\": \"comp_0\", \"sr\": 1, \"ks\": { \"o\": { \"a\": 0, \"k\": 100, "
      + "\"ix\": 11 }, \"r\": { \"a\": 0, \"k\": 0, \"ix\": 10 }, \"p\": { \"a\": 0, \"k\": [400, "
      + "500, 0], \"ix\": 2, \"l\": 2 }, \"a\": { \"a\": 0, \"k\": [400, 500, 0], \"ix\": 1, "
      + "\"l\": 2 }, \"s\": { \"a\": 0, \"k\": [100, 100, 100], \"ix\": 6, \"l\": 2 } }, \"ao\": "
      + "0, \"w\": 800, \"h\": 1000, \"ip\": 0, \"op\": 300, \"st\": 0, \"bm\": 0 } ], "
      + "\"markers\": [], \"props\": {} }";

  public static AnimaXView createAnimaXView(IAnimationListener initListener, boolean autoplay) {
    return createAnimaXView(new NativeAbility(), initListener, autoplay);
  }

  private static AnimaXView createAnimaXView(
      BaseAbility ability, IAnimationListener initListener, boolean autoplay) {
    Context context = InstrumentationRegistry.getInstrumentation().getTargetContext();
    AnimaXContext animaXContext = new AnimaXContext.Builder(ability, context).build();
    AnimaXView animaXView = new AnimaXView(animaXContext);
    animaXView.onSurfaceTextureAvailable(mock(SurfaceTexture.class), 100, 100);
    animaXView.setJson(DYNAMIC_PROPERTY_JSON);

    animaXView.addAnimationListener(new AnimationListenerAdapter() {
      @Override
      public void onReady(AnimaXParam param) {
        if (initListener != null) {
          initListener.onReady(param);
        }
      }

      @Override
      public void onStart(AnimaXParam param) {
        if (initListener != null) {
          initListener.onStart(param);
        }
      }

      @Override
      public void onError(AnimaXErrorParam param) {
        if (initListener != null) {
          initListener.onError(param);
        }
      }
    });

    return animaXView;
  }

  public static boolean checkCapability() {
    return DeviceUtil.checkCapability(new NativeAbility());
  }

  public static AnimaXValueParam createGenericValueParam(LayerPropertyType propertyType) {
    switch (propertyType) {
      case TRANSFORM_OPACITY:
      case OPACITY:
      case DROP_SHADOW_OPACITY:
        return AnimaXValueParam.fromNumber(50);
      case TRANSFORM_ANCHOR:
      case TRANSFORM_POSITION:
      case TRANSFORM_SCALE:
      case ELLIPSE_SIZE:
      case RECTANGLE_SIZE:
      case POSITION:
      case REPEATER_OFFSET:
        return AnimaXValueParam.fromCoordinate(10.0f, 20.0f);
      case TRANSFORM_ROTATION:
      case TRANSFORM_SKEW:
      case TRANSFORM_SKEW_ANGLE:
      case TRANSFORM_START_OPACITY:
      case TRANSFORM_END_OPACITY:
      case TRANSFORM_ROTATION_X:
      case TRANSFORM_ROTATION_Y:
      case TRANSFORM_ROTATION_Z:
      case TEXT_SIZE:
      case TEXT_TRACKING:
      case STROKE_WIDTH:
      case BLUR_RADIUS:
      case CORNER_RADIUS:
      case REPEATER_COPIES:
      case POLYSTAR_POINTS:
      case POLYSTAR_ROTATION:
      case POLYSTAR_INNER_RADIUS:
      case POLYSTAR_OUTER_RADIUS:
      case POLYSTAR_INNER_ROUNDED:
      case POLYSTAR_OUTER_ROUNDED:
      case DROP_SHADOW_DIRECTION:
      case DROP_SHADOW_DISTANCE:
      case DROP_SHADOW_RADIUS:
        return AnimaXValueParam.fromNumber(5.0f);
      case TEXT_VALUE:
        return AnimaXValueParam.fromString("Test Text");
      case TEXT_COLOR:
      case COLOR:
      case STROKE_COLOR:
      case DROP_SHADOW_COLOR:
        return AnimaXValueParam.fromColor(0xFF123456);
      case COLOR_FILTER:
        return AnimaXValueParam.fromColorFilter(0xFF654321, 0);
      default:
        return AnimaXValueParam.fromNumber(0f);
    }
  }

  public static AnimaXValueCallback createGenericValueCallback(
      final LayerPropertyType propertyType) {
    return new AnimaXValueCallback() {
      @Override
      public AnimaXValueParam getValue(AnimaXFrameInfo frameInfo) {
        switch (propertyType) {
          case TRANSFORM_OPACITY:
          case OPACITY:
          case DROP_SHADOW_OPACITY:
            return AnimaXValueParam.fromNumber((int) (frameInfo.getOverallProgress() * 100));
          case TRANSFORM_ANCHOR:
          case TRANSFORM_POSITION:
          case TRANSFORM_SCALE:
          case ELLIPSE_SIZE:
          case RECTANGLE_SIZE:
          case POSITION:
          case REPEATER_OFFSET:
            return AnimaXValueParam.fromCoordinate(
                frameInfo.getOverallProgress(), frameInfo.getOverallProgress());
          case TRANSFORM_ROTATION:
          case TRANSFORM_SKEW:
          case TRANSFORM_SKEW_ANGLE:
          case TRANSFORM_START_OPACITY:
          case TRANSFORM_END_OPACITY:
          case TRANSFORM_ROTATION_X:
          case TRANSFORM_ROTATION_Y:
          case TRANSFORM_ROTATION_Z:
          case TEXT_SIZE:
          case TEXT_TRACKING:
          case STROKE_WIDTH:
          case BLUR_RADIUS:
          case CORNER_RADIUS:
          case REPEATER_COPIES:
          case POLYSTAR_POINTS:
          case POLYSTAR_ROTATION:
          case POLYSTAR_INNER_RADIUS:
          case POLYSTAR_OUTER_RADIUS:
          case POLYSTAR_INNER_ROUNDED:
          case POLYSTAR_OUTER_ROUNDED:
          case DROP_SHADOW_DIRECTION:
          case DROP_SHADOW_DISTANCE:
          case DROP_SHADOW_RADIUS:
            return AnimaXValueParam.fromNumber((float) frameInfo.getOverallProgress());
          case TEXT_VALUE:
            return AnimaXValueParam.fromString("Frame: " + frameInfo.getOverallProgress());
          case TEXT_COLOR:
          case COLOR:
          case STROKE_COLOR:
          case DROP_SHADOW_COLOR:
            int r = (int) (frameInfo.getOverallProgress() * 255);
            return AnimaXValueParam.fromColor(0xFF000000 | (r << 16));
          case COLOR_FILTER:
            return AnimaXValueParam.fromColor(0xFF00FF00);
          default:
            return AnimaXValueParam.fromNumber(0f);
        }
      }
    };
  }
}
