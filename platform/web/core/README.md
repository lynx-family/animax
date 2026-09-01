# @lynx-js/animax

Animax for web platform. `animax-view` is a high-performance animation component for the Web, consistent with the Lynx AnimaX framework.

## Installation

```bash
npm install @lynx-js/animax@latest
```

Video and Textra text layout are shipped as optional wasm side modules. If your animations use video layers or Textra text layout, install the companion packages with the same version as `@lynx-js/animax`:

```bash
npm install @lynx-js/animax@latest \
  @lynx-js/animax-video@latest \
  @lynx-js/animax-textra@latest
```

## Usage

### React

This React example loads optional runtime modules before mounting `<animax-view>`.

```tsx
import React, { useEffect, useRef, useState } from 'react';
import { AnimaXViewElement } from '@lynx-js/animax';
import type { AnimaXFontConfig, AnimaXViewProps } from '@lynx-js/animax';
import { AnimaXVideoModuleUrl } from '@lynx-js/animax-video';
import { AnimaXTextraModuleUrl } from '@lynx-js/animax-textra';

declare global {
  namespace JSX {
    interface IntrinsicElements {
      'animax-view': React.DetailedHTMLProps<
        React.HTMLAttributes<AnimaXViewElement>,
        AnimaXViewElement
      > &
        AnimaXViewProps;
    }
  }
}

const fontCdn =
  'https://lf3-static.bytednsdoc.com/obj/eden-cn/tzhd_ehtpuh/ljhwZthlaukjlkulzlp/fonts/';
const primaryFont = 'Noto Sans SC';
const fontConfig: AnimaXFontConfig = {
  defaultFamily: primaryFont,
  fonts: [
    { family: primaryFont, url: `${fontCdn}NotoSansSC-fallback.ttf` },
    { family: 'Noto Sans Thai', url: `${fontCdn}NotoSansThai-Regular.ttf` },
    {
      family: 'Noto Sans Bengali',
      url: `${fontCdn}NotoSansBengali-Regular.ttf`,
    },
    {
      family: 'Noto Sans Kannada',
      url: `${fontCdn}NotoSansKannada-Regular.ttf`,
    },
    {
      family: 'Noto Sans Gujarati',
      url: `${fontCdn}NotoSansGujarati-Regular.ttf`,
    },
    {
      family: 'Noto Sans Devanagari',
      url: `${fontCdn}NotoSansDevanagari-Regular.ttf`,
    },
    { family: 'Noto Sans Telugu', url: `${fontCdn}NotoSansTelugu-Regular.ttf` },
    {
      family: 'Noto Sans Malayalam',
      url: `${fontCdn}NotoSansMalayalam-Regular.ttf`,
    },
    { family: 'Noto Sans Oriya', url: `${fontCdn}NotoSansOriya-Regular.ttf` },
    { family: 'Noto Sans Arabic', url: `${fontCdn}NotoSansArabic-Regular.ttf` },
    { family: 'Noto Sans Hebrew', url: `${fontCdn}NotoSansHebrew-Regular.ttf` },
    { family: 'Noto Emoji', url: `${fontCdn}Noto-COLRv1.ttf` },
  ],
};

function configureAnimaXRuntimeFonts(): Promise<boolean> {
  return AnimaXViewElement.configureFonts(fontConfig);
}

export default function App() {
  const animRef = useRef<AnimaXViewElement>(null);
  const [runtimeReady, setRuntimeReady] = useState(false);
  const [runtimeError, setRuntimeError] = useState<string | null>(null);

  useEffect(() => {
    let canceled = false;
    const loadRuntime = async () => {
      const [fontsLoaded, videoLoaded, textraLoaded] = await Promise.all([
        configureAnimaXRuntimeFonts(),
        AnimaXViewElement.loadVideoModule(AnimaXVideoModuleUrl),
        AnimaXViewElement.loadTextraModule(AnimaXTextraModuleUrl),
      ]);
      if (canceled) return;

      const failed = [
        fontsLoaded ? null : 'fonts',
        videoLoaded ? null : 'video',
        textraLoaded ? null : 'textra',
      ].filter(Boolean);
      if (failed.length > 0) {
        setRuntimeError(`Failed to load AnimaX runtime: ${failed.join(', ')}`);
        return;
      }
      setRuntimeReady(true);
    };

    loadRuntime();
    return () => {
      canceled = true;
    };
  }, []);

  useEffect(() => {
    if (!runtimeReady) return;
    const element = animRef.current;
    if (!element) return;

    const handleReady = (e: any) => {
      console.log('Animation Ready:', e.detail);
      element.play();
    };

    element.addEventListener('ready', handleReady);
    return () => element.removeEventListener('ready', handleReady);
  }, [runtimeReady]);

  if (runtimeError) return <div>{runtimeError}</div>;
  if (!runtimeReady) return <div>Loading AnimaX runtime...</div>;

  return (
    <div style={{ width: 400, height: 400, border: '1px solid #ddd' }}>
      <animax-view
        ref={animRef}
        src="https://example.com/your-animation.json"
        width={400}
        height={400}
        loop={true}
        speed={1.0}
        objectfit="contain"
        style={{ display: 'block', width: '100%', height: '100%' }}
      />
    </div>
  );
}
```

If your animation does not use video layers, you can skip `@lynx-js/animax-video` and `loadVideoModule`. If it does not need Textra text layout, you can skip `@lynx-js/animax-textra`, `loadTextraModule`, and custom font configuration.

## API Reference

The `animax-view` web component aligns its properties and capabilities with other platforms.

### Attributes & Properties

| Attribute | Type | Default | Description |
|-----------|------|---------|-------------|
| `backend` | `string` | `'webgl'` | Rendering backend (`'webgpu'` or `'webgl'`). |
| `src` | `string` | `''` | Animation resource URL (JSON or specific format). |
| `src-format` | `string` | `''` | Animation resource URL format containing `"%s"` placeholder (used with `src-polyfill`). |
| `src-polyfill` | `Record<string, string> \| string`| `''` | Asset ID → URL mapping. If `string`, will be parsed as JSON (invalid JSON falls back to `{}`). |
| `json` | `string` | `''` | Animation resource content as a JSON string. |
| `width` | `number` | - | Canvas pixel width. If not set, uses `offsetWidth * devicePixelRatio` (fallback `300 * DPR`). |
| `height` | `number` | - | Canvas pixel height. If not set, uses `offsetHeight * devicePixelRatio` (fallback `150 * DPR`). |
| `loop` | `boolean` | `false` | Loop playback. |
| `loop-count` | `number` | `1` | Number of times to loop (`0` = infinite). (Prefer not to use together with `loop`.) |
| `autoplay` | `boolean` | `true` | Auto play on load. |
| `speed` | `number` | `1.0` | Playback speed. |
| `start-frame`| `number` | `0` | Start frame index. |
| `end-frame` | `number` | `-1` | End frame index (`-1` means last frame). |
| `auto-reverse` | `boolean` | `false` | Reverse direction during repeat playback. |
| `progress` | `number` | `0` | Playback progress (range: `0` to `1`). |
| `objectfit` | `'contain' \| 'cover' \| 'center' \| 'fill' \| 'scale-down'` | `'contain'` | Object fit strategy. |
| `object-position`| `'center' \| 'left' \| 'right' \| 'top' \| 'bottom' \| 'top-left' \| 'top-right' \| 'bottom-left' \| 'bottom-right'` | `'center'` | Alignment of the content in the view (includes corner positions). |
| `fps-event-interval` | `number` | `0` | FPS event interval (ms). |
| `max-frame-rate` | `number` | `60` | Maximum frame rate for rendering. |
| `dynamic-resource` | `boolean` | `false` | Enable dynamic property/resource updates (may require manual play after `ready`). |
| `keeplastframe`| `boolean` | `true` | Keep the last frame visible after finish. |
| `capture-frame` | `boolean` | `false` | Retain the next rendered WebGL frame for capture. Set before the view initializes. This is not supported by WebGPU. |

### Methods

Invoke these methods on the DOM element ref (e.g., `animRef.current`).

- `play()`: Start playback.
- `pause()`: Pause playback.
- `resume()`: Resume from pause.
- `stop()`: Stop playback.
- `reload()`: Reload animation.
- `captureFrame(type?, quality?)`: Return the retained WebGL frame as a `Blob`, then consume it. The default type is `image/png`. Returns `null` when capture is disabled or unavailable, or when using WebGPU.
- `requestFrameCapture()`: Request a fresh WebGL frame capture. A successful capture emits `frameavailable`.
- `seek(frame)`: Jump to specific frame.
- `playSegment(startFrame, endFrame)`: Play a specific range.
- `isAnimating()`: Return whether the animation is currently playing.
- `getDuration()`: Get animation duration in milliseconds.
- `getCurrentFrame()`: Get current frame number.
- `subscribeUpdateEvent(frame)`: Subscribe update callback for a specific frame.
- `unsubscribeUpdateEvent(frame)`: Unsubscribe update callback for a specific frame.
- `subscribeUpdateEvents(frames)`: Subscribe updates for multiple frames.
- `unsubscribeUpdateEvents(frames)`: Unsubscribe updates for multiple frames.
- `updateLayerProperty(type, layerName, value, callback?)`: Update dynamic layer properties.
- `setResourceProperty(type, resourceId, value, callback?)`: Update dynamic resources (images/fonts/videos).
- `updateTextByLayerName(layerName, newText, targetFrame?, callback?)`: Update text value.
- `updateTextSizeByLayerName(layerName, textSize, targetFrame?, callback?)`: Update text size.
- `updateTextColorByLayerName(layerName, textColor, targetFrame?, callback?)`: Update text color. Returns `false` if input color is invalid (`#RRGGBB[AA]` or `0xRRGGBB[AA]`).
- `updateImageById(imageId, newImageUrl)`: Update image resource URL by asset id.
- `updateVideoById(videoId, newVideoUrl)`: Update video resource URL by asset id.
- `updateFontByName(fontName, newFontPath)`: Update font path by font name.
- `getLayerBounds(layerName, boundsSpace)`: Get bounds of layer by name in specified space.

### Rendering Recovery & Frame Capture

`<animax-view>` recreates its player when the element is mounted again or its
WebGL context is restored. While rendering is temporarily unavailable because
of a context loss or canvas dimension change, it emits
`renderingunavailable` with the corresponding `reason`.

Frame capture is opt-in and WebGL-only. Enable `capture-frame` before the view
initializes, listen for `frameavailable`, and call `captureFrame()` to retain a
fallback image in your application. `captureFrame()` consumes the retained
frame; call `requestFrameCapture()` when another capture is needed.

```typescript
const view = document.createElement('animax-view');
view.setAttribute('capture-frame', 'true');
view.setAttribute('src', 'https://example.com/your-animation.json');

let fallbackFrame: Blob | null = null;
view.addEventListener('frameavailable', async () => {
  fallbackFrame = await view.captureFrame();
});
view.addEventListener('renderingunavailable', (event) => {
  console.log(event.detail.reason, fallbackFrame);
});

document.body.appendChild(view);
```

### Static Methods
Invoke these methods on AnimaXViewElement.
- `configureFonts(config)`: Configures the global wasm font registry. Use this for Textra and multi-language text layout. Call it once during app initialization, preferably before loading text-heavy animations. `fonts` is an ordered fallback list, and `defaultFamily` marks the configured default font.
  ```typescript
  const fontCdn =
    'https://lf3-static.bytednsdoc.com/obj/eden-cn/tzhd_ehtpuh/ljhwZthlaukjlkulzlp/fonts/';

  await AnimaXViewElement.configureFonts({
    defaultFamily: 'Noto Sans SC',
    fonts: [
      {
        family: 'Noto Sans SC',
        url: `${fontCdn}NotoSansSC-fallback.ttf`,
      },
      {
        family: 'Noto Sans Thai',
        url: `${fontCdn}NotoSansThai-Regular.ttf`,
      },
      {
        family: 'Noto Emoji',
        url: `${fontCdn}Noto-COLRv1.ttf`,
      },
    ],
  });
  ```
- Font descriptors only accept `family` and `url`. `aliases`, `roles`, `fallbackPriority`, and `fallbackOrder` are not part of the Web API. The legacy default-font shortcut has been removed; use `configureFonts`, `loadFonts`, or `loadFont`.
- `loadFonts(fonts)`: Loads multiple font descriptors into the global wasm font registry. This is a shortcut for `configureFonts({ fonts })`.
- `loadFont(font)`: Loads one font descriptor into the global wasm font registry.
- `loadVideoModule(url)`: Asynchronously loads the video module from a URL. Returns a `Promise<boolean>` that resolves to `true` if the video module is loaded successfully; otherwise resolves to `false`.
  - **Recommended usage** — import from `@lynx-js/animax-video` package:
    ```typescript
    import { AnimaXViewElement } from '@lynx-js/animax';
    import { AnimaXVideoModuleUrl } from '@lynx-js/animax-video';

    // Call once during app initialization, before any <animax-view> elements
    await AnimaXViewElement.loadVideoModule(AnimaXVideoModuleUrl);
    ```
  - **Alternative usage** — pass a direct URL to the `.wasm` file:
    ```typescript
    import { AnimaXViewElement } from '@lynx-js/animax';

    // Pass any accessible URL pointing to animax-video.wasm
    await AnimaXViewElement.loadVideoModule('https://your-cdn.com/animax-video.wasm');
    ```
  - Required for playing video layers. Must be called before initializing the animax-view element if your animation contains video layers. **Note**: On the Web platform, video layers do **not** support ZIP-compressed video resources; use json files instead.
  - Call this once during app initialization (before any `<animax-view>` elements are created).
- `loadTextraModule(url)`: Asynchronously loads the Textra text layout module from a URL. Returns a `Promise<boolean>` that resolves to `true` if the Textra module is loaded successfully; otherwise resolves to `false`.
  - **Recommended usage** — import from `@lynx-js/animax-textra` package:
    ```typescript
    import { AnimaXViewElement } from '@lynx-js/animax';
    import { AnimaXTextraModuleUrl } from '@lynx-js/animax-textra';

    await AnimaXViewElement.loadTextraModule(AnimaXTextraModuleUrl);
    ```
  - Required for Textra text layout on Web. Keep `@lynx-js/animax-textra` at the same version as `@lynx-js/animax`.
  - For text-heavy or multi-language animations, call `configureFonts` before mounting `<animax-view>`.

### Events

Use `addEventListener` to listen to these events.

| Event Name | Description | Parameters (`event.detail`) |
|------------|-------------|-----------------------------|
| `ready` | Animation resource loaded. | `animationID`, `current`, `total`, `loopIndex` |
| `compositionready` | Composition is ready. | `animationID`, `current`, `total`, `loopIndex` |
| `firstframe` | First frame rendered. | `animationID`, `current`, `total`, `loopIndex` |
| `frameavailable` | A retained WebGL frame is ready for `captureFrame()`. | None |
| `renderingunavailable` | Rendering is temporarily unavailable while the view recovers. | `reason` (`context-lost` or `dimension-change`) |
| `start` | Animation started. | `animationID`, `current`, `total`, `loopIndex` |
| `update` | Frame update event. | `animationID`, `current`, `total`, `loopIndex` |
| `completion` | Animation finished one cycle. | `animationID`, `current`, `total`, `loopIndex` |
| `repeat` | Animation repeated (loop tick). | `animationID`, `current`, `total`, `loopIndex` |
| `cancel` | Animation cancelled/stopped. | `animationID`, `current`, `total`, `loopIndex` |
| `taplayers` | Layer tap hit-test result. | `animationID`, `current`, `total`, `loopIndex`, `layerList` |
| `fps` | FPS report. | `animationID`, `current`, `total`, `loopIndex`, `fps`, `max_drop_rate` |
| `warning` | Warning occurred. | `code`, `data` |
| `error` | Error occurred. | `code`, `data` |

## License

Apache-2.0
