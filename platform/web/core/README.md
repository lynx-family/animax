# @lynx-js/animax

AnimaX for Web provides the `<animax-view>` custom element with WebGL and
WebGPU rendering backends.

## Install

```bash
npm install @lynx-js/animax
```

## Usage

Importing the package registers the custom element:

```typescript
import { AnimaXViewElement } from '@lynx-js/animax';

const view = document.createElement('animax-view') as AnimaXViewElement;
view.src = 'https://example.com/animation.json';
view.loop = true;
view.addEventListener('ready', () => view.play());
document.body.append(view);
```

The element supports JSON animations and common playback controls such as
`play`, `pause`, `resume`, `stop`, `seek`, and `playSegment`.

## Optional modules

Install the companion package at the same version when an animation needs
video layers or Textra text layout:

```bash
npm install @lynx-js/animax-video @lynx-js/animax-textra
```

```typescript
import { AnimaXViewElement } from '@lynx-js/animax';
import { AnimaXVideoModuleUrl } from '@lynx-js/animax-video';
import { AnimaXTextraModuleUrl } from '@lynx-js/animax-textra';

await AnimaXViewElement.loadVideoModule(AnimaXVideoModuleUrl);
await AnimaXViewElement.loadTextraModule(AnimaXTextraModuleUrl);
```

Load optional modules before creating `<animax-view>` elements that depend on
them.

## License

Apache-2.0
