# @byted-lynx/animax

Animax for web platform. `animax-view` is a high-performance animation component for the Web, consistent with the Lynx AnimaX framework.

## Installation

```bash
npm install @byted-lynx/animax
```

## Usage

### React

Here is a complete example using React.

1.  **Import the package** to register the web component.
2.  **Use `useRef`** to interact with the element's methods.
3.  **Add TypeScript declarations** for type safety.

```tsx
import React, { useRef, useEffect } from 'react';
import '@byted-lynx/animax'; // 1. Registers <animax-view>
import { AnimaXViewElement, AnimaXViewProps } from '@byted-lynx/animax';

// 2. TypeScript Declaration
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

export default function App() {
  const animRef = useRef<AnimaXViewElement>(null);

  useEffect(() => {
    const element = animRef.current;
    if (!element) return;

    // Listen for events
    const handleReady = (e: any) => {
      console.log('Animation Ready:', e.detail);
      element.play();
    };

    element.addEventListener('ready', handleReady);
    return () => element.removeEventListener('ready', handleReady);
  }, []);

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
        // Ensure the element is visible
        style={{ display: 'block', width: '100%', height: '100%' }}
      />
    </div>
  );
}
```

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

### Methods

Invoke these methods on the DOM element ref (e.g., `animRef.current`).

- `play()`: Start playback.
- `pause()`: Pause playback.
- `resume()`: Resume from pause.
- `stop()`: Stop playback.
- `reload()`: Reload animation.
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

### Events

Use `addEventListener` to listen to these events.

| Event Name | Description | Parameters (`event.detail`) |
|------------|-------------|-----------------------------|
| `ready` | Animation resource loaded. | `animationID`, `current`, `total`, `loopIndex` |
| `compositionready` | Composition is ready. | `animationID`, `current`, `total`, `loopIndex` |
| `firstframe` | First frame rendered. | `animationID`, `current`, `total`, `loopIndex` |
| `start` | Animation started. | `animationID`, `current`, `total`, `loopIndex` |
| `update` | Frame update event. | `animationID`, `current`, `total`, `loopIndex` |
| `completion` | Animation finished one cycle. | `animationID`, `current`, `total`, `loopIndex` |
| `repeat` | Animation repeated (loop tick). | `animationID`, `current`, `total`, `loopIndex` |
| `cancel` | Animation cancelled/stopped. | `animationID`, `current`, `total`, `loopIndex` |
| `taplayers` | Layer tap hit-test result. | `animationID`, `current`, `total`, `loopIndex`, `layerList` |
| `fps` | FPS report. | `animationID`, `current`, `total`, `loopIndex`, `fps`, `max_drop_rate` |
| `warning` | Warning occurred. | `code`, `data` |
| `error` | Error occurred. | `code`, `data` |

## Local Development

This repository contains the library and an example app.

### Setup

```bash
# Setup
source tools/envsetup.sh
lcm sync . --target tasm
source buildtools/emsdk/emsdk_env.sh
```


```bash
# 1. Install Library Dependencies
cd animax/platform/web
npm install

# 2. Install Example Dependencies
cd animax/example/web/example
npm install
```

### Build & Run

This is the current setup in `example/web/example/package.json`.

```json
"dependencies": {
  "@byted-lynx/animax": "file:../../../platform/web"
}
```

1.  **Build Library**:
```bash
cd animax/platform/web
npm run build
```

2.  **Dev Example**:
```bash
cd animax/example/web/example
# Changes in library are reflected after library rebuild + example reload
npm run dev
```

## Publish

This package is published to **bnpm** (ByteDance internal npm).

### Prerequisites

Before beginning the publish process, make sure you have first completed the [Setup](#setup) and [Build & Run](#build--run) steps appropriately for the project.

1.  **Set up the bnpm registry**:
    Make sure to check if there is an `.npmrc` file in the project directory when setting the registry, and modify it accordingly to avoid overriding the external config.
```bash
npm config set registry https://bnpm.byted.org
```
    
2.  **SSO Login**:
    bnpm supports SSO login. Password login is not supported, you must log in via SSO internally. Ensure your npm version is `npm@8`.
    
    *   **Local Machine**:
```bash
npx @bytedance-dev/bnpm@latest login --auth-type=sso
```

### Publishing Steps

Before each publish, ensure you have gone through the following checklist:

1.  **Update Version**: Update the `version` field in `package.json` according to semantic versioning. 
2.  **Update Changelog**: Add release notes for the new version in `CHANGELOG.md`.
3.  **Build and Publish**: Ensure the latest modifications are built by running `npm run build`, and then publish. Ensure the package name uses an internal scope (e.g., `@byted-lynx`). The `publishConfig` in `package.json` is already configured to publish to the bnpm registry.

```bash
# Change version manually in `package.json`
# Ensure the latest modifications are built by running `npm run build`
npm publish
```

## License

MIT